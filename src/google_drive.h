#ifndef GOOGLE_DRIVE_H
#define GOOGLE_DRIVE_H

#include "json.hpp"

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
        httplib::SSLClient cli(GAPI_URL);
#if defined(__APPLE__)
        cli.set_ca_cert_path("/etc/ssl/cert.pem");
#endif
        // Google Drive files list under root
        auto res = cli.Get(FILES_URL + "?q='root'%20in%20parents",
            { {"Authorization", "Bearer " + credentials.access_token } });
        if (res.error() == httplib::Error::Success) {
            nlohmann::json data = nlohmann::json::parse(res.value().body);
            for (const auto& el : data["files"]) {
                result.push_back(el.get<GFile>());
            }
        }
        return result;
    }
private:
    Credentials credentials;
};

#endif // GOOGLE_DRIVE_H