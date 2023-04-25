#ifndef GOOGLE_DRIVE_H
#define GOOGLE_DRIVE_H

#include "json.hpp"
#include "request_handler.h"

using namespace httplib;
using json = nlohmann::json;

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
            nlohmann::json data = nlohmann::json::parse(res.value().body);
            for (const auto& el : data["files"]) {
                result.push_back(el.get<GFile>());
            }
        }
        return result;
    }

    int UploadFile(const std::string& filePath, const std::string& fileName, const std::string& fileType) {
        std::ifstream file_stream(filePath + fileName, std::ios::binary);
        if (!file_stream) {
            std::cerr << "Failed to open file" << std::endl;
            return 1;
        }
        std::string fileContent((std::istreambuf_iterator<char>(file_stream)),
                                 std::istreambuf_iterator<char>());

        MultipartFormData media {
            .name = "file",
            .content = fileContent,
            .filename = fileName,
            .content_type = fileType
        };

        MultipartFormData metadata {
            .name = "metadata",
            .content = json{ {"name", fileName} }.dump(),
            .filename = "",
            .content_type = "application/json; charset=UTF-8"
        };

        httplib::MultipartFormDataItems items = { metadata, media };
        Headers headers = { {"Authorization", "Bearer " + credentials.access_token} };

        httplib::SSLClient client(GAPI_URL);
#if defined(__APPLE__)
    client.set_ca_cert_path("/etc/ssl/cert.pem");
#endif
        auto res = client.Post("/upload/drive/v3/files?uploadType=multipart", headers, items);
        if (res) {
            std::cout << "Status code: " << res->status << std::endl;
            std::cout << "Response body: " << res->body << std::endl;
        } else {
            std::cerr << "Failed to send request: " << res.error() << std::endl;
            return 1;
        }
        return 0;
    }
private:
    Credentials credentials;
};

#endif // GOOGLE_DRIVE_H