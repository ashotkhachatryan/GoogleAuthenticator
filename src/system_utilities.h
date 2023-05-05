#ifndef SYSTEM_UTILITIES_H
#define SYSTEM_UTILITIES_H

#include <string>

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
#define __WINDOWS__
#endif

#if defined(__WINDOWS__)
#include "Windows.h"
#include "shellapi.h"
#include <shlobj.h>
#elif defined(__APPLE__)
#include <CoreFoundation/CFBundle.h>
#include <ApplicationServices/ApplicationServices.h>
#include <CoreFoundation/CoreFoundation.h>
#elif defined(__linux__)
#endif

class SystemUtilities
{
public:
    static void OpenUrlInBrowser(const std::string &urlStr)
    {
#if defined(__WINDOWS__)
        ShellExecute(0, 0, urlStr.c_str(), 0, 0, SW_SHOW);
#elif defined(__APPLE__)
        CFURLRef url = CFURLCreateWithBytes(
            NULL,                    // allocator
            (UInt8 *)urlStr.c_str(), // URLBytes
            urlStr.length(),         // length
            kCFStringEncodingASCII,  // encoding
            NULL                     // baseURL
        );
        LSOpenCFURLRef(url, 0);
        CFRelease(url);
#elif defined(__linux__)
        system(("xdg-open '" + urlStr + "'").c_str());
#endif
    }
    static std::string GetDocumentsPath()
    {
        std::string documentsPath;
#if defined(__WINDOWS__)
        char myDocuments[MAX_PATH];
        HRESULT result = SHGetFolderPath(NULL, CSIDL_PERSONAL, NULL, SHGFP_TYPE_CURRENT, myDocuments);
        documentsPath = std::string(myDocuments);
#elif defined(__APPLE__) || defined(__linux__)
        // TODO this needs to be improved
        const char *homeDir = getenv("HOME");
        if (homeDir)
        {
            documentsPath = std::string(homeDir).append("/Documents");
        }
#endif
        return documentsPath;
    }
};

#endif // SYSTEM_UTILITIES_H
