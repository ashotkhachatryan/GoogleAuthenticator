#include <iostream>
#include "client_secret.h"
#include "google_authenticator.h"
#include "httplib.h"
#include "constants.h"

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cout << "Please provide client secrets json file path\n";
        return -1;
    }

    GoogleAuthenticator auth {
        ClientSecret::FromJson(argv[1]),
        {Scopes::DriveMetadataReadonly, Scopes::DrivePhotosReadonly}
    };

    auto credentials = auth.Authenticate();
    if (credentials.has_value()) {
        httplib::SSLClient cli(GAPI_URL);
        //cli.set_ca_cert_path("/etc/ssl/cert.pem");
        // Google Drive files list under root
        auto res = cli.Get(FILES_URL + "?q='root'%20in%20parents",
                           {{"Authorization", "Bearer " + credentials->access_token }});
        if (res.error() == httplib::Error::Success) {
            std::cout << res.value().body << std::endl;
        }
    }
}
