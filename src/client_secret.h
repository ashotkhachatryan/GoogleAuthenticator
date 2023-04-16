#ifndef CLIENT_SECRET_H
#define CLIENT_SECRET_H

#include <fstream>
#include <string>
#include <vector>

#include "json.hpp"

struct ClientSecret
{
    std::string client_id;
    std::string project_id;
    std::string auth_uri;
    std::string token_uri;
    std::string auth_provider_x509_cert_url;
    std::string client_secret;
    std::vector<std::string> redirect_uris;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE(ClientSecret,
                                   client_id,
                                   project_id,
                                   auth_uri,
                                   token_uri,
                                   auth_provider_x509_cert_url,
                                   client_secret,
                                   redirect_uris)

    static ClientSecret FromJson(const std::string& path) {
        ClientSecret secret;

        std::ifstream f(path);
        nlohmann::json j = nlohmann::json::parse(f);
        secret = j["installed"].get<ClientSecret>();
        
        return secret;
    }
};

struct Credentials {
    std::string access_token;
    int expires_in;
    std::string refresh_token;
    std::string scope;
    std::string token_type;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(Credentials,
                                   access_token,
                                   expires_in,
                                   refresh_token,
                                   scope,
                                   token_type)

    static Credentials FromJsonString(const std::string& json) {
        auto j = nlohmann::json::parse(json);
        return j.get<Credentials>();
    }
};

#endif