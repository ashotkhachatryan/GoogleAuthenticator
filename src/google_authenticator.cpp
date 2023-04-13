#include <filesystem>

#include "httplib.h"
#include "constants.h"
#include "google_authenticator.h"

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
#define WINDOWS
#endif

#if defined(WINDOWS)
#include "Windows.h"
#include "shellapi.h"
#include <shlobj.h>
#endif

using namespace std;

std::string GoogleAuthenticator::ConvertParamsToString(const ParamsType& params) const {
    if (params.size() == 0)
        return {};
    string url{'?'};
    for (int i = 0; i < params.size(); ++i) {
        if (i != 0)
            url.append("&");
        url.append(params[i].first).append("=").append(params[i].second);
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

std::optional<Credentials> GoogleAuthenticator::SendAuthRequest(const std::string& code) const {
    httplib::Params params{
        {"code",          code},
        {"client_id",     secret.client_id},
        {"client_secret", secret.client_secret},
        {"redirect_uri",  uri},
        {"grant_type",    "authorization_code"}
    };
    httplib::SSLClient cli(OAUTH_URL);
#if defined(__APPLE__)
    cli.set_ca_cert_path("/etc/ssl/cert.pem");
#endif
    httplib::Result res = cli.Post("/token", params);
    if (res.error() == httplib::Error::Success) {
        std::string json_body = res.value().body;
        StoreCredentials(json_body);
        return std::optional<Credentials>(Credentials::FromJsonString(json_body));
    }
    return std::nullopt;
}

std::optional<Credentials> GoogleAuthenticator::Authenticate() {
    std::string url = ConstructAuthUrl();
#if defined(WINDOWS)
    ShellExecute(0, 0, url.c_str(), 0, 0, SW_SHOW);
#else
    std::cout << "Please copy and paste the following URL into your browser.\n"
              << url << '\n';
#endif
    std::string code = RunCodeReceiverServer();
    return SendAuthRequest(code);
}

void GoogleAuthenticator::StoreCredentials(const std::string& data) const {
    std::string documentsPath;
#if defined(WINDOWS)
    char myDocuments[MAX_PATH];
    HRESULT result = SHGetFolderPath(NULL, CSIDL_PERSONAL, NULL, SHGFP_TYPE_CURRENT, myDocuments);
    documentsPath = std::string(myDocuments);
#endif

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