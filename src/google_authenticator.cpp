#include <filesystem>

#include "httplib.h"
#include "constants.h"
#include "system_utilities.h"
#include "google_authenticator.h"

using namespace std;

std::string GoogleAuthenticator::ConvertParamsToString(const Params& params) const {
    if (params.size() == 0)
        return {};
    string url{'?'};

    for (auto p : params) {
        if (p != *params.begin()) {
            url.append("&");
        }
        url.append(p.first).append("=").append(p.second);
    }
    return url;
}

std::string GoogleAuthenticator::ConstructAuthUrl() const {
    std::string scope(scopes[0]);
    for (const auto& s : scopes) {
        scope.append("+").append(s);
    }
    string paramsUrl = ConvertParamsToString({
        {"client_id",     secret.client_id},
        {"redirect_uri",  uri},
        {"response_type", "code"},
        {"scope",         scope},
        {"access_type",   "offline"},
    });
    std::string url = std::string(AUTH_URL).append(paramsUrl);
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

std::optional<Credentials> GoogleAuthenticator::TokenRequest(const httplib::Params& params) const {
    httplib::SSLClient cli(OAUTH_URL);
#if defined(__APPLE__)
    cli.set_ca_cert_path("/etc/ssl/cert.pem");
#endif
    auto res = cli.Post("/token", params);
    if (res.error() == httplib::Error::Success) {
        std::string json_body = res.value().body;
        return Credentials::FromJsonString(json_body);
    }
    return std::nullopt;
}

std::optional<Credentials> GoogleAuthenticator::Authenticate() {
    auto credentials = ReadCredentials();
    if (credentials.has_value()) {
        if (!IsTokenValid(credentials.value()))
        {
            httplib::Params params{
                {"client_id", secret.client_id},
                {"client_secret", secret.client_secret},
                {"refresh_token", credentials.value().refresh_token},
                {"grant_type", "refresh_token"}
            };
            auto cred = TokenRequest(params);

            if (cred.has_value())
            {
                nlohmann::json j = cred.value();
                StoreCredentials(j.dump(2));
                cred.value().refresh_token = credentials.value().refresh_token;
            }

            return cred;
        }
        return credentials;
    }
    else {
        std::string url = ConstructAuthUrl();
        SystemUtilities::OpenUrlInBrowser(url);
        std::string code = RunCodeReceiverServer();
        httplib::Params params{
            {"code",          code},
            {"client_id",     secret.client_id},
            {"client_secret", secret.client_secret},
            {"redirect_uri",  uri},
            {"grant_type",    "authorization_code"}
        };
        auto cr = TokenRequest(params);
        if (cr.has_value())
        {
            nlohmann::json j = cr.value();
            StoreCredentials(j.dump(2));
        }
        return cr;
    }
}

void GoogleAuthenticator::StoreCredentials(const std::string& data) const {
    std::string documentsPath = SystemUtilities::GetDocumentsPath();
    if (!documentsPath.empty())
    {
        filesystem::path dirPath = filesystem::path(documentsPath).append(dirName);
        if (!filesystem::exists(dirPath)) {
            filesystem::create_directory(dirPath);
        }
        filesystem::path filePath = dirPath.append(fileName);
        std::ofstream ofs(filePath);
        if (ofs)
            ofs << data;
        ofs.close();
    }
}

std::optional<Credentials> GoogleAuthenticator::ReadCredentials() const {
    std::string documentsPath = SystemUtilities::GetDocumentsPath();
    if (!documentsPath.empty()) {
        filesystem::path dirPath = filesystem::path(documentsPath).append(dirName);
        filesystem::path filePath = dirPath.append(fileName);
        if (filesystem::exists(filePath)) {
            std::ifstream ifs(filePath);
            std::string jsonStr((std::istreambuf_iterator<char>(ifs)),
                                 std::istreambuf_iterator<char>());
            ifs.close();

            return std::optional<Credentials>(Credentials::FromJsonString(jsonStr));
        }
    }
    return std::nullopt;
}

std::string GoogleAuthenticator::GetTokenInfo(const Credentials& credentials) const {
    httplib::SSLClient cli(OAUTH_URL);
#if defined(__APPLE__)
    cli.set_ca_cert_path("/etc/ssl/cert.pem");
#endif
    httplib::Params params{
        {"access_token", credentials.access_token}
    };
    auto res = cli.Post("/tokeninfo", params);
    if (res.error() != httplib::Error::Success) {
        std::cout << "ERROR: " << res.error() << std::endl;
    }
    else {
        return res.value().body;
    }
    return std::string();
}

bool GoogleAuthenticator::IsTokenValid(const Credentials& credentials) const {
    std::string info = GetTokenInfo(credentials);
    if (!info.empty()) {
        nlohmann::json j = nlohmann::json::parse(info);
        if (j.find("error") != j.end()) {
            return false;
        }
    }
    return true;
}