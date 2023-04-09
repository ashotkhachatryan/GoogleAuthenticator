#ifndef GOOGLE_AUTHENTICATOR_H
#define GOOGLE_AUTHENTICATOR_H

#include "client_secret.h"

class GoogleAuthenticator {
public:
    using ParamsType = std::vector<std::pair<std::string, std::string>>;
    GoogleAuthenticator(const ClientSecret& clientSecret,
                        const std::vector<std::string>& scopes)
        : secret(clientSecret)
        , scopes(scopes) { }

    [[nodiscard]] std::optional<Credentials> Authenticate();
private:
    [[nodiscard]] std::string ConstructAuthUrl() const;
    [[nodiscard]] std::string RunCodeReceiverServer() const;
    [[nodiscard]] std::optional<Credentials> SendAuthRequest(const std::string& code) const;
    std::string ConvertParamsToString(const ParamsType& params) const;
private:
    ClientSecret secret;
    std::vector<std::string> scopes;

    static const int port = 55599;
    inline static const std::string uri = std::string("http://localhost:").append(std::to_string(port));
};

#endif
