#include <iostream>
#include "ClientSecret.h"
#include "GoogleAuthenticator.h"

int main() {
    auto secret = ClientSecret::FromJson(R"");
    auto credentials = GoogleAuthenticator::Authenticate(secret);
    if (credentials) {
        std::cout << credentials->access_token << '\n';
    }
}