#ifndef GOOGLE_AUTHENTICATOR_H
#define GOOGLE_AUTHENTICATOR_H

#include "client_secret.h"

class GoogleAuthenticator {
public:
    GoogleAuthenticator(const ClientSecret& clientSecret,
                        const std::vector<std::string>& scopes)
        : secret(clientSecret)
        , scopes(scopes) { }

    [[nodiscard]]
    std::optional<Credentials> Authenticate();
private:
    [[nodiscard]]
    std::string ConstructAuthUrl() const;
    [[nodiscard]]
    std::string RunCodeReceiverServer() const;
    [[nodiscard]]
    std::optional<Credentials> SendAuthRequest(const std::string& code) const;
private:
    ClientSecret secret;
    std::vector<std::string> scopes;
};

#endif
