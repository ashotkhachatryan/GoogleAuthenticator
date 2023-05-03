#include <filesystem>

#include "httplib.h"
#include "constants.h"
#include "system_utilities.h"
#include "google_authenticator.h"
#include "request_handler.h"

std::string GoogleAuthenticator::ConvertParamsToString(const Params& params) const {
    if (params.size() == 0)
        return {};
    std::string url{'?'};

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
    std::string paramsUrl = ConvertParamsToString({
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
    auto res = RequestHandler::PostRequest(OAUTH_URL, "/token", params);
    if (res.error() == httplib::Error::Success) {
        return Credentials::FromJsonString(res->body);
    }
    else {
        std::cerr << "TokenRequest failed: " << res.error() << std::endl;
    }
    return std::nullopt;
}

httplib::Params GoogleAuthenticator::GetParams(std::string token, bool refresh) const {
    httplib::Params params {
        {"client_id", secret.client_id},
        {"client_secret", secret.client_secret},
        {"grant_type", refresh ? "refresh_token" : "authorization_code"},
    };
    if (refresh) {
        params.insert({ "refresh_token", token });
    }
    else {
        params.insert({ "code", token });
        params.insert({ "redirect_uri", uri });
    }
    return params;
}

std::optional<Credentials> GoogleAuthenticator::Authenticate() {
    httplib::Params params;
    std::string refreshToken;

    auto credentials = ReadCredentials();

    if (credentials.has_value()) {
        if (IsTokenValid(credentials.value()))
            return credentials;

        refreshToken = credentials.value().refresh_token;
        params = GetParams(refreshToken, true);
    }
    else {
        std::string url = ConstructAuthUrl();
        SystemUtilities::OpenUrlInBrowser(url);
        std::string code = RunCodeReceiverServer();
        params = GetParams(code);
    }

    auto cred = TokenRequest(params);

    if (cred.has_value()) {
        nlohmann::json j = cred.value();
        if (!refreshToken.empty())
            cred.value().refresh_token = refreshToken;
        StoreCredentials(j.dump(2));
    }

    return cred;
}

void GoogleAuthenticator::StoreCredentials(const std::string& data) const {
    std::string documentsPath = SystemUtilities::GetDocumentsPath();
    if (!documentsPath.empty())
    {
        std::filesystem::path dirPath = std::filesystem::path(documentsPath).append(dirName);
        if (!std::filesystem::exists(dirPath)) {
            std::filesystem::create_directory(dirPath);
        }
        std::filesystem::path filePath = dirPath.append(fileName);
        std::ofstream ofs(filePath);
        if (ofs)
            ofs << data;
        ofs.close();
    }
}

std::optional<Credentials> GoogleAuthenticator::ReadCredentials() const {
    std::string documentsPath = SystemUtilities::GetDocumentsPath();
    if (!documentsPath.empty()) {
        std::filesystem::path dirPath = std::filesystem::path(documentsPath).append(dirName);
        auto filePath = dirPath.append(fileName);
        if (std::filesystem::exists(filePath)) {
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
    httplib::Params params{
        {"access_token", credentials.access_token}
    };
    auto res = RequestHandler::PostRequest(OAUTH_URL, "/tokeninfo", params);
    if (res.error() == httplib::Error::Success) {
        return res->body;
    }
    else {
        std::cerr << "GetTokenInfo failed: " << res.error() << std::endl;
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