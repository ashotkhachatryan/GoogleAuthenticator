#include "google_drive.h"

GDrive::GDrive(const Credentials &c)
    : credentials(c)
{
}

std::vector<GFile> GDrive::GetFileList(const std::string &location,
                                       const std::string &nextPageToken)
{
    std::stringstream query;
    query << FILES_URL << "?q='" << location << "'%20in%20parents";
    if (!nextPageToken.empty())
        query << "&pageToken=" << nextPageToken;

    auto res = RequestHandler::GetRequest(GAPI_URL, query.str(),
                                          {{"Authorization", "Bearer " + credentials.access_token}});

    std::vector<GFile> result;
    if (res.error() == httplib::Error::Success)
    {
        nlohmann::json data = nlohmann::json::parse(res->body);
        for (const auto &el : data["files"])
        {
            result.push_back(el.get<GFile>());
        }

        if (data.find("nextPageToken") != data.end())
        {
            std::string token = data["nextPageToken"].get<std::string>();
            auto nextResult = GetFileList(location, token);
            result.insert(result.end(), nextResult.begin(), nextResult.end());
        }
    }
    else
    {
        std::cerr << "GetFileList failed: " << res.error() << std::endl;
    }
    return result;
}

int GDrive::UploadFile(const std::string &filePath, const std::string &parent)
{
    std::ifstream file_stream(filePath, std::ios::binary);
    if (!file_stream)
    {
        std::cerr << "Failed to open file" << std::endl;
        return 1;
    }
    std::string fileContent((std::istreambuf_iterator<char>(file_stream)),
                            std::istreambuf_iterator<char>());

    std::string fileName{std::filesystem::path(filePath).filename().string()};
    httplib::MultipartFormData media{
        .name = "file",
        .content = fileContent,
        .filename = fileName,
        .content_type = MIMEType::get(fileName)};
    std::string folderId = GetFileId(parent).value();
    httplib::MultipartFormData metadata{
        .name = "metadata",
        .content = nlohmann::json{{"name", fileName}, {"parents", {folderId}}}.dump(),
        .filename = "",
        .content_type = "application/json; charset=UTF-8"};

    httplib::MultipartFormDataItems items = {metadata, media};
    httplib::Headers headers = {{"Authorization", "Bearer " + credentials.access_token}};

    httplib::SSLClient client(GAPI_URL);
#if defined(__APPLE__)
    client.set_ca_cert_path("/etc/ssl/cert.pem");
#endif
    auto res = client.Post(UPLOAD_URL, headers, items);
    if (res)
    {
        std::cout << "Status code: " << res->status << std::endl;
        std::cout << "Response body: " << res->body << std::endl;
    }
    else
    {
        std::cerr << "Failed to send request: " << res.error() << std::endl;
        return 1;
    }
    return 0;
}

std::optional<std::string> GDrive::GetFileId(const std::string &path)
{
    auto pathVec = SplitPath(path);
    if (pathVec.size() == 0)
    {
        std::cerr << "Failed to parse the given path.\n";
        return std::nullopt;
    }

    std::string parent = "root";
    for (const auto &p : pathVec)
    {
        if (p == "root")
            continue;
        auto fileId = GetFileId(p, parent);
        if (fileId.has_value())
            parent = fileId.value();
        else
        {
            return std::nullopt;
        }
    }
    return parent;
}

std::optional<std::string> GDrive::GetFileId(const std::string &fileName, const std::string &parent)
{
    std::vector<GFile> files = GetFileList(parent);
    auto it = std::find_if(files.begin(), files.end(), [&](GFile f)
                           { return f.name == fileName; });
    if (it != files.end())
        return it->id;
    return std::nullopt;
}

std::vector<std::string> GDrive::SplitPath(const std::string &path)
{
    std::string str = path;
    if (str.size() == 0 || str[0] != '/')
        return {};
    if (str.back() == '/')
        str = str.substr(0, str.size() - 1);
    auto v = split(str.substr(1), '/');
    if (v.size() == 0 || v[0] != "root")
        return {};
    return v;
}