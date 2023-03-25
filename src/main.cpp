#include <iostream>
#include "ClientSecret.h"
#include "GoogleAuthenticator.h"

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cout << "Please provide client secrets json file path\n";
        return -1;
    }
    auto secret = ClientSecret::FromJson(argv[1]);
    auto credentials = GoogleAuthenticator::Authenticate(secret);
    if (credentials) {
        std::cout << credentials->access_token << '\n';
    }
}
