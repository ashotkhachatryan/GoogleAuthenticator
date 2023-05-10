#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <string>

struct Scopes {
    inline static std::string DriveMetadataReadonly = "https://www.googleapis.com/auth/drive.metadata.readonly";
    inline static std::string DrivePhotosReadonly = "https://www.googleapis.com/auth/drive.photos.readonly";
    inline static std::string Drive = "https://www.googleapis.com/auth/drive";
};

static const std::string AUTH_URL     = "https://accounts.google.com/o/oauth2/v2/auth";
static const std::string GDRIVE_SCOPE = "https://www.googleapis.com/auth/drive.metadata.readonly";
static const std::string OAUTH_URL    = "oauth2.googleapis.com";
static const std::string GAPI_URL     = "www.googleapis.com";
static const std::string FILES_URL    = "/drive/v3/files";
static const std::string UPLOAD_URL   = "/upload/drive/v3/files?uploadType=multipart";

#endif