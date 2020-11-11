/////////////////////////////////////////////////////////////////////////////
// Name:        CurlHandler.h
// Purpose:     libCurl ssl support functions for download file
// Author:      Paresh More
// Created:     2016-06-10
// Copyright:   (c) EnterpriseDB
// Licence:     BSD Licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _CURLHANDLER_H
#define _CURLHANDLER_H

#include <wx/wx.h>
#include <wx/url.h>
#include <curl/curl.h>
#include <wx/stdpaths.h>
#include <wx/sstream.h>
#include <wx/filefn.h>
#include <wx/thread.h>
#include <wx/filename.h>

#define URLSIZE 1024

#define WITH_SSL_SUPPORT      0
#define WITH_OUT_SSL_SUPPORT  1
#define INVALID_SSL_SUPPORT   2

#define CURL_DOWNLOAD_TIMEOUT   60L // Seconds
#define CURL_DOWNLOAD_MIN_BYTES 30L // Bytes

class CurlHandler
{
public:
        CurlHandler();
        ~CurlHandler();

        wxString GetRedirectedURL(wxString oldURL, bool ignoreInsecureError);
        wxString GetApplicationList() const { return m_appListBuffer; }

        bool ParseApplicationList();
        void SetApplicationUrl(const wxString &url) { m_applicationListUrl = url; }

private:
        bool VerifyCertificate(wxString url, bool ignoreInsecureError);
        void SetCertificateFile();

private:
        wxString m_applicationListUrl;
        wxString m_appListBuffer;
        wxString m_certFile;

        bool m_certFileExist;
        CURL *m_curl;
};


class DownloadThread : public wxThread
{
public:
        DownloadThread();
        ~DownloadThread(void);
        virtual void *Entry();

        wxString GetDownloadPackageUrl(wxString downloadUrl, wxString packageName);
        void SetDownloadUrl(const wxString &url) { m_downloadUrl = url; }
        void SetDownloadFileName(const wxString &file) { m_downloadFileName = file; }
        void SetDownloadInProgress(const bool &status) { m_isDownloadInProgress = status; }

        friend int DownloadInfo(void *p, curl_off_t dltotal, curl_off_t dlnow, curl_off_t ultotal, curl_off_t ulnow);
        double GetTotalFileSize() const { return m_totalFileSize; }
        double GetTotalDownloadSize() const { return m_totalDownloadSize; }
        bool GetDownloadInProgress() const { return m_isDownloadInProgress; }

private:
        wxString m_downloadUrl;
        wxString m_downloadFileName;

        double m_totalFileSize;
        double m_totalDownloadSize;
        bool m_isDownloadInProgress;
};

#endif
