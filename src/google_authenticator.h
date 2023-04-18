#ifndef GOOGLE_AUTHENTICATOR_H
#define GOOGLE_AUTHENTICATOR_H

#include "client_secret.h"

class GoogleAuthenticator {
public:
    using Params = std::multimap<std::string, std::string>;
    GoogleAuthenticator(const ClientSecret& clientSecret,
                        const std::vector<std::string>& scopes)
        : secret(clientSecret)
        , scopes(scopes) { }

    [[nodiscard]] std::optional<Credentials> Authenticate();
private:
    [[nodiscard]] std::string ConstructAuthUrl() const;
    [[nodiscard]] std::string RunCodeReceiverServer() const;
    std::string ConvertParamsToString(const Params& params) const;
    void StoreCredentials(const std::string& data) const;
    std::optional<Credentials> ReadCredentials() const;
    std::string GetTokenInfo(const Credentials& credentials) const;
    std::optional<Credentials> TokenRequest(const Params& params) const;
    bool IsTokenValid(const Credentials& credentials) const;
    Params GetParams(std::string token, bool refresh = false) const;

private:
    ClientSecret secret;
    std::vector<std::string> scopes;

    static const int port = 55599;
    inline static const std::string uri = std::string("http://localhost:").append(std::to_string(port));
    inline static const std::string fileName = "credentials.json";
    inline static const std::string dirName = "GAuth";
};

#endif
