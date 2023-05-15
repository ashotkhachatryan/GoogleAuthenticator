#ifndef GOOGLE_DRIVE_H
#define GOOGLE_DRIVE_H

#include <filesystem>

#include "json.hpp"
#include "request_handler.h"
#include "mime_type.h"
#include "constants.h"
#include "client_secret.h"

namespace {
    std::vector<std::string> split(const std::string& str, char delimeter)
    {
        std::vector<std::string> result;
        size_t begin = 0;
        size_t end = str.find(delimeter, 0);
        while (end != -1) {
            std::string substr = str.substr(begin, end - begin);
            result.push_back(substr);

            begin = end + 1;
            end = begin;

            end = str.find(delimeter, end);
        }
        result.push_back(str.substr(begin, str.size() - begin));
        return result;
    }
}

struct GFile {
    std::string kind;
    std::string mimeType;
    std::string id;
    std::string name;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE(GFile,
        kind, mimeType, id, name)
};

class GDrive {
public:
    GDrive(const Credentials& c);
    std::vector<GFile> GetFileList(const std::string& location = "root",
                                   const std::string& nextPageToken = "");
    int UploadFile(const std::string& filePath, const std::string& parent = "/root");
    std::optional<std::string> GetFileId(const std::string& path);

private:
    std::optional<std::string> GetFileId(const std::string& fileName, const std::string& parent);
    std::vector<std::string> SplitPath(const std::string& path);

private:
    Credentials credentials;
};

#endif // GOOGLE_DRIVE_H