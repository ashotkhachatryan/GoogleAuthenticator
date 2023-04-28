#ifndef GOOGLE_DRIVE_H
#define GOOGLE_DRIVE_H

#include <filesystem>

#include "json.hpp"
#include "request_handler.h"
#include "mime_type.h"

struct GFile {
    std::string kind;
    std::string mimeType;
    std::string id;
    std::string name;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE(GFile,
        kind, mimeType, id, name)
};

class GDrive {
public:
    GDrive(const Credentials& c)
        : credentials(c) {
    }

    std::vector<GFile> GetFileList(const std::string& location = "root") {
        std::vector<GFile> result;
        httplib::Headers headers = { {"Authorization", "Bearer " + credentials.access_token } };
        auto res = RequestHandler::GetRequest(GAPI_URL, FILES_URL + "?q='root'%20in%20parents", headers);
        if (res.error() == httplib::Error::Success) {
            nlohmann::json data = nlohmann::json::parse(res->body);
            for (const auto& el : data["files"]) {
                result.push_back(el.get<GFile>());
            }
        }
        else {
            std::cerr << "GetFileList failed: " << res.error() << std::endl;
        }
        return result;
    }

    int UploadFile(const std::string& filePath, const std::string& parent = "root") {
        std::ifstream file_stream(filePath, std::ios::binary);
        if (!file_stream) {
            std::cerr << "Failed to open file" << std::endl;
            return 1;
        }
        std::string fileContent((std::istreambuf_iterator<char>(file_stream)),
                                 std::istreambuf_iterator<char>());

        std::string fileName{ std::filesystem::path(filePath).filename().string() };
        httplib::MultipartFormData media {
            .name = "file",
            .content = fileContent,
            .filename = fileName,
            .content_type = MIMEType::get(fileName)
        };
        std::string folderId = GetFileId(parent).value();
        httplib::MultipartFormData metadata {
            .name = "metadata",
            .content = nlohmann::json{ {"name", fileName}, {"parents", {folderId} }}.dump(),
            .filename = "",
            .content_type = "application/json; charset=UTF-8"
        };

        httplib::MultipartFormDataItems items = { metadata, media };
        httplib::Headers headers = { {"Authorization", "Bearer " + credentials.access_token} };

        httplib::SSLClient client(GAPI_URL);
#if defined(__APPLE__)
    client.set_ca_cert_path("/etc/ssl/cert.pem");
#endif
        auto res = client.Post(UPLOAD_URL, headers, items);
        if (res) {
            std::cout << "Status code: " << res->status << std::endl;
            std::cout << "Response body: " << res->body << std::endl;
        } else {
            std::cerr << "Failed to send request: " << res.error() << std::endl;
            return 1;
        }
        return 0;
    }

    std::optional<std::string> GetFileId(const std::string& fileName, const std::string& parent = "root") {
        std::vector<GFile> files = GetFileList(parent);
        auto it = std::find_if(files.begin(), files.end(), [&](GFile f) {
            return f.name == fileName;
        });
        if (it != files.end())
            return it->id;
        return std::nullopt;
    }
private:
    Credentials credentials;
};

#endif // GOOGLE_DRIVE_H