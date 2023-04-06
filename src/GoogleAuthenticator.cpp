//#define CPPHTTPLIB_OPENSSL_SUPPORT
#include "httplib.h"
#include "constants.h"
#include "GoogleAuthenticator.h"

namespace {
std::string runServerAndWait() {
    httplib::Server svr;
    std::string result;
    svr.Get("/", [&svr, &result](const httplib::Request &req, httplib::Response &res) {
        auto it = req.params.find("code");
        if (it != req.params.end()) {
            result = it->second;
            res.set_content("The code has been received successfully. You may now close this window.", "text/plain");
            svr.stop();
        }
    });
    svr.listen("0.0.0.0", 55599);
    return result;
}

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
}

std::unique_ptr<Credentials> GoogleAuthenticator::Authenticate(const ClientSecret& clientSecret,
                                                               const std::vector<std::string>& scopes) {
    if (scopes.size() == 0)
        // This needs to throw an exception
        return nullptr;

    std::string scope(scopes[0]);
    for (const auto& s : scopes) {
        scope.append("+").append(s);
    }
	QueryParams queryParams{
		{"client_id", clientSecret.client_id},
		{"redirect_uri", "http://localhost:55599/"},
		{"response_type", "code"},
		{"scope", scope},
		{"access_type", "offline"},
	};

	std::string url(AUTH_URL);
	std::cout << "Please copy and paste the following URL into your browser.\n"
		<< url.append(queryParams.url) << '\n';

	std::string code = runServerAndWait();

	httplib::Params params{
		{"code", code},
		{"client_id", clientSecret.client_id},
		{"client_secret", clientSecret.client_secret},
		{"redirect_uri", "http://localhost:55599/"},
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
