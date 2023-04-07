//#define CPPHTTPLIB_OPENSSL_SUPPORT
#include "httplib.h"
#include "constants.h"
#include "GoogleAuthenticator.h"

namespace {
const int port = 55599;
const std::string uri = std::string("http://localhost:").append(std::to_string(port));

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

std::string GoogleAuthenticator::ConstructAuthUrl() const {
    std::string scope(scopes[0]);
    for (const auto& s : scopes) {
        scope.append("+").append(s);
    }
	QueryParams queryParams{
		{"client_id", secret.client_id},
		{"redirect_uri", uri},
		{"response_type", "code"},
		{"scope", scope},
		{"access_type", "offline"},
	};

	std::string url = std::string(AUTH_URL).append(queryParams.url);
    return url;
}

std::string GoogleAuthenticator::RunCodeReceiverServer() const {
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
    svr.listen("0.0.0.0", port);
    return result;
}

std::optional<Credentials> GoogleAuthenticator::SendAuthRequest(const std::string& code) const {
    httplib::Params params{
		{"code", code},
		{"client_id", secret.client_id},
		{"client_secret", secret.client_secret},
		{"redirect_uri", uri},
		{"grant_type", "authorization_code"}
	};

	httplib::SSLClient cli(OAUTH_URL);
	// For MacOS
	//cli.set_ca_cert_path("/etc/ssl/cert.pem");
	httplib::Result res = cli.Post("/token", params);
	if (res.error() == httplib::Error::Success) {
		return std::optional<Credentials>(Credentials::FromJsonString(res.value().body));
	}
	return std::nullopt;
}

std::optional<Credentials> GoogleAuthenticator::Authenticate() {
    std::string url = ConstructAuthUrl();
	std::cout << "Please copy and paste the following URL into your browser.\n"
		<< url << '\n';

	std::string code = RunCodeReceiverServer();
    return SendAuthRequest(code);
}
