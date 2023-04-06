#include <iostream>
#include "ClientSecret.h"
#include "GoogleAuthenticator.h"
#include "httplib.h"
#include "constants.h"

struct Scopes {
    inline static std::string DriveMetadataReadonly = "https://www.googleapis.com/auth/drive.metadata.readonly";
    inline static std::string DrivePhotosReadonly   = "https://www.googleapis.com/auth/drive.photos.readonly";
};

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cout << "Please provide client secrets json file path\n";
        return -1;
    }
    auto secret = ClientSecret::FromJson(argv[1]);
    std::vector<std::string> scopes {Scopes::DriveMetadataReadonly, Scopes::DrivePhotosReadonly};
    auto credentials = GoogleAuthenticator::Authenticate(secret, scopes);
    if (credentials) {
        httplib::SSLClient cli(GAPI_URL);
        // Google Drive files list under root
        auto res = cli.Get(FILES_URL + "?q='root'%20in%20parents",
                           {{"Authorization", "Bearer " + credentials->access_token }});
        if (res.error() == httplib::Error::Success) {
            std::cout << res.value().body << std::endl;
        }
    }
}
