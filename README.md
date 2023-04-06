# GoogleAuthenticator
A C++ code example demonstrating the authentication and authorization process for Google API services using OAuth 2.0.

It uses <a href="https://github.com/yhirose/cpp-httplib">cpp-httplib</a> library for HTTP requests and <a href="https://github.com/nlohmann/json">JSON</a> library for json data deserialization. Both are single header libraries.

### Usage

```
#include "ClientSecret.h"
#include "GoogleAuthenticator.h"

int main(int argc, char** argv) {
    // 1. ClientSecret object is created based on json credentials.
    auto secret = ClientSecret::FromJson("client_secret.json");
    
    // 2. The Authenticate function is responsible for constructing
    //    an authentication URL that must be opened in a browser.
    //    Once the user successfully authenticates themselves, the
    //    library will receive an authentication code, which it will
    //    use to make an authorization request to obtain additional credentials.
    auto credentials = GoogleAuthenticator::Authenticate(secret);
    if (credentials) {
        // Credentials can be used to make further requests
        
        // Get list of Google Drive files under root
        httplib::SSLClient cli(GAPI_URL);
        auto res = cli.Get(FILES_URL + "?q='root'%20in%20parents",
                           {{"Authorization", "Bearer " + credentials->access_token }});
        if (res.error() == httplib::Error::Success) {
            std::cout << res.value().body << std::endl;
        }
    }
}
```
