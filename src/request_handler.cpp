#include "request_handler.h"

httplib::Result
RequestHandler::PostRequest(const std::string& host,
                            const std::string& url,
                            const httplib::Params& params)
{
    httplib::SSLClient cli(host);
#if defined(__APPLE__)
    cli.set_ca_cert_path("/etc/ssl/cert.pem");
#endif
    return cli.Post(url, params);
}

httplib::Result
RequestHandler::GetRequest(const std::string& host,
                           const std::string& url,
                           const httplib::Headers& headers)
{
    httplib::SSLClient cli(host);
#if defined(__APPLE__)
    cli.set_ca_cert_path("/etc/ssl/cert.pem");
#endif
    return cli.Get(url, headers);
}