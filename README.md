# GoogleAuthenticator
A C++ code example demonstrating the authentication and authorization process for Google API services using OAuth 2.0.

It uses <a href="https://github.com/yhirose/cpp-httplib">cpp-httplib</a> library for HTTP requests and <a href="https://github.com/nlohmann/json">JSON</a> library for json data deserialization. Both are single header libraries.

### Usage

```
#include <iostream>
#include "client_secret.h"
#include "google_authenticator.h"
#include "httplib.h"
#include "constants.h"
#include "google_drive.h"

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cout << "Please provide client secrets json file path\n";
        return -1;
    }

    GoogleAuthenticator auth {
        ClientSecret::FromJson(argv[1]),
        {Scopes::DriveMetadataReadonly, Scopes::DrivePhotosReadonly}
    };

    auto credentials = auth.Authenticate();
    if (credentials.has_value()) {
        GDrive drive(credentials.value());
        std::vector<GFile> files = drive.GetFileList();
        for (const auto& f : files) {
            std::cout << f.name << "\n";
        }
    }
}
```
