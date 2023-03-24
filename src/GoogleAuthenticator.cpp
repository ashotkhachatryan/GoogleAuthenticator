//#define CPPHTTPLIB_OPENSSL_SUPPORT
#include "httplib.h"
#include "constants.h"
#include "GoogleAuthenticator.h"

struct QueryParams {
    std::string url;
    QueryParams(const std::initializer_list<std::pair<std::string, std::string>>& params) {
        for (int index = 0; const auto & p : params) {
            index == 0 ? url.append("?") : url.append("&");
            url.append(p.first).append("=").append(p.second);
            index++;
        }
    }
};

std::unique_ptr<Credentials> GoogleAuthenticator::Authenticate(const ClientSecret& clientSecret) {
        QueryParams queryParams {
            {"client_id", clientSecret.client_id},
            {"redirect_uri", clientSecret.redirect_uris[0]},
            {"response_type", "code"},
            {"scope", GDRIVE_SCOPE},
            {"access_type", "offline"},
        };

        std::string url(AUTH_URL);
        std::cout << "Please copy and paste the following URL into your browser.\n"
                  << url.append(queryParams.url);

        std::cin.get();
        std::cout << "CODE IS: ";
        std::string code;
        std::cin >> code;

        httplib::Params params {
            {"code", code},
            {"client_id", clientSecret.client_id},
            {"client_secret", clientSecret.client_secret},
            {"redirect_uri", clientSecret.redirect_uris[0]},
            {"grant_type", "authorization_code"}
        };

        httplib::SSLClient cli(OAUTH_URL);
        // For MacOS
        //cli.set_ca_cert_path("/etc/ssl/cert.pem");
        httplib::Result res = cli.Post("/token", params);
        if (res.error() == httplib::Error::Success) {
            return std::make_unique<Credentials>(Credentials::FromJsonString(res.value().body));
        }
        return nullptr;
}