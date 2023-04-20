#ifndef REQUEST_HANDLER_H
#define REQUEST_HANDLER_H

#include "httplib.h"

class RequestHandler {
public:
    static httplib::Result PostRequest(const std::string& host,
                                       const std::string& url,
                                       const httplib::Params& params);
};

#endif // REQUEST_HANDLER_H