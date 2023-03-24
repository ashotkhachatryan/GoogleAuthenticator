#ifndef GOOGLE_AUTHENTICATOR_H
#define GOOGLE_AUTHENTICATOR_H

#include "ClientSecret.h"

class GoogleAuthenticator {
public:
    [[nodiscard]]
    static std::unique_ptr<Credentials> Authenticate(const ClientSecret& secret);
};

#endif