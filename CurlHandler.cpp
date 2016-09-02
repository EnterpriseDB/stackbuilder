/////////////////////////////////////////////////////////////////////////////
// Name:        CurlHandler.cpp
// Purpose:     PostgreSQL/EnterpriseDB Application Stack Builder
// Copyright:   (c) EnterpriseDB
// Licence:     BSD Licence
/////////////////////////////////////////////////////////////////////////////
#include "CurlHandler.h"
#include "StackBuilder.h"

#ifdef __WIN32__
static HANDLE g_eventHandle;
#endif

void LogMessage(wxString msg)
{

#ifdef __WIN32__
    if (!g_eventHandle)
        g_eventHandle = RegisterEventSource(0, wxT("Stackbuilder"));

    if (g_eventHandle)
    {
        LPCTSTR logStr = NULL;
        {
            logStr = _wcsdup(msg.wc_str());
            ReportEvent(g_eventHandle, EVENTLOG_INFORMATION_TYPE, 0, 0, NULL, 1, 0, &logStr, NULL);
        }
}

#else
    fprintf(stderr,"%s", (const char *) msg.mb_str());
#endif

}

static int GetSSLSupportStatus(wxString url)
{
    if (url.StartsWith(wxT("https://")) || url.StartsWith(wxT("ftps://")))
        return WITH_SSL_SUPPORT;
    else if (url.StartsWith(wxT("http://")) || url.StartsWith(wxT("ftp://")))
        return WITH_OUT_SSL_SUPPORT;
    else
        return INVALID_SSL_SUPPORT;
}

/* WriteCallback function is a callback function used for CURL to write the data.
 * We convert the user pointer to wxString and append the data.
*/
static size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
    if (userp != NULL)
        ((wxString *)userp)->Append(wxString::FromUTF8((const char*)contents), size * nmemb);

    return size * nmemb;
}

CurlHandler::CurlHandler() :
m_applicationListUrl(wxEmptyString), m_appListBuffer(wxEmptyString),
m_certFileExist(false), m_certFile(wxEmptyString)
{
    SetCertificateFile();

    /* Initialize CURL */
    curl_global_init(CURL_GLOBAL_ALL);

    m_curl = curl_easy_init();
}

CurlHandler::~CurlHandler()
{
    curl_easy_cleanup(m_curl);
    curl_global_cleanup();
}

void CurlHandler::SetCertificateFile()
{
#ifndef __WIN32__
    wxString certFolder = wxEmptyString;
    certFolder = wxPathOnly(wxStandardPaths::Get().GetExecutablePath()) + wxT("/../");
    wxFileName canonicalpath(certFolder);
    canonicalpath.Normalize(wxPATH_NORM_DOTS);


    if (g_certificateBundle != wxEmptyString)
        m_certFile = g_certificateBundle;
    else
#ifdef __linux__
        m_certFile = canonicalpath.GetPath() + wxT("/share/certs/ca-bundle.crt");
#else
        m_certFile = canonicalpath.GetPath() + wxT("/Resources/certs/ca-bundle.crt");
#endif
    if (wxFileExists(m_certFile))
        m_certFileExist = true;
#endif
}

/*
 * VerifyCertificate would be used by catlog and package download
 * for Catlog ignoreInsecureError = always false ( it would always abort for https->http
 * for packagedownload ignoreInsecureError = true (it would also abort but this is special case
 * if http->http is made, internally it would first try https->http,so this case we are ignoring
 * warning msg is shown and confirmation is required from user.
*/
bool CurlHandler::VerifyCertificate(wxString newUrl,bool ignoreInsecureError)
{
#ifndef __WIN32__
    if (!m_certFileExist)
    {
        wxLogError(wxString::Format(_("Failed to open certificate file : %s"), m_certFile.c_str()));
        return false;
    }
#endif

    if (!m_curl)
    {
        wxLogError(wxString::Format(_("Failed to perform curl_easy_init")));
        return false;
    }

    CURLcode curl_res;
    /* Reset CURL to default state */
    curl_easy_reset(m_curl);

#ifndef __WIN32__
    curl_easy_setopt(m_curl, CURLOPT_CAINFO, (char*)m_certFile.char_str(wxConvUTF8));
#endif
    curl_easy_setopt(m_curl, CURLOPT_URL,(const char *)newUrl.mb_str());
    curl_easy_setopt(m_curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(m_curl, CURLOPT_WRITEDATA, NULL);

    curl_easy_setopt(m_curl, CURLOPT_SSL_VERIFYPEER, 1L);
    curl_easy_setopt(m_curl, CURLOPT_SSL_VERIFYHOST, 1L);

    curl_easy_setopt(m_curl, CURLOPT_VERBOSE, 0L);
    curl_easy_setopt(m_curl, CURLOPT_CERTINFO, 1L);
    char cErrbuf[CURL_ERROR_SIZE] = { 0 };
    /* provide a buffer to store errors in */
    curl_easy_setopt(m_curl, CURLOPT_ERRORBUFFER, cErrbuf);

    curl_res = curl_easy_perform(m_curl);
    wxString msg = wxEmptyString;

    long response_code;
    if (curl_res != CURLE_OK)
    {
        curl_res = curl_easy_getinfo(m_curl, CURLINFO_SSL_VERIFYRESULT, &response_code);
        switch (curl_res)
        {
            case CURLE_FAILED_INIT:
                msg = _("Could not parse the URL.");
                break;
            case CURLE_COULDNT_RESOLVE_PROXY:
                if(ignoreInsecureError)
                    return true;
                else
                    msg = _("Invalid URL");
                break;
            case CURLE_UNSUPPORTED_PROTOCOL:
                msg = _("Unsupported protocol specified.");
                break;
            case CURLE_COULDNT_RESOLVE_HOST:
                if(ignoreInsecureError)
                    return true;
                else
                    msg = _("No hostname specified in URL.");
                break;
            case CURLE_SSL_CACERT_BADFILE:
                msg = wxString::Format(_("Invalid Certificate Authority bundle at %s"), m_certFile.c_str());
                break;
            case CURLE_COULDNT_CONNECT:
                msg = _("A connection error occurred.");
                break;
            default:
                msg = wxString::FromUTF8(cErrbuf);
                wxString warnmsg = wxString::Format(_("A certificate verification problem was encountered whilst accessing %s\n%s\nThis means that the source of the download cannot be verified. It is recommended that you do not continue with the download as it may be coming from a site that is pretending to be the intended download site and may contain viruses or malware.\n\nDo you wish to continue ?"),m_applicationListUrl.c_str(),msg.c_str());
                if (wxMessageBox(warnmsg, _("Warning"), wxYES_NO | wxICON_QUESTION) == wxNO)
                    return false;
                else
                    return true;
        }
        wxLogError(wxString::Format(_("Error occurred during certification verification for the url '%s'\n\nError: %s"), m_applicationListUrl.c_str(), msg.c_str()));
        return false;
    }

       return true;
}

/*
 * GetRedirectURL would be used by catlog and package download
 * for Catlog ignoreInsecureError = always false ( it would always abort for https->http
 * for packagedownload ignoreInsecureError = true (it would also abort but this is special case
 * if http->http is made, internally it would first try https->http,so this case we are ignoring
 * warning msg is shown and confirmation is required from user.
*/
wxString CurlHandler::GetRedirectedURL(wxString oldURL, bool ignoreInsecureError)
{
    wxString retURL = wxEmptyString;

#ifndef __WIN32__
    if (!m_certFileExist)
    {
        wxLogError(wxString::Format(_("Failed to open certificate file : %s"), m_certFile.c_str()));
        return retURL;
    }
#endif
    CURLcode res;
    char *location=NULL;
    long response_code;
    wxString newURL = oldURL;

    /* Check if Valid certificate */
    if (!VerifyCertificate(newURL,ignoreInsecureError))
        return retURL;

    if (!m_curl)
    {
        wxLogError(wxString::Format(_("Failed to perform curl_easy_init")));
        return retURL;
    }
    /* Reset CURL to default state */
    curl_easy_reset(m_curl);

#ifndef __WIN32__
    curl_easy_setopt(m_curl, CURLOPT_CAINFO, (char*)m_certFile.char_str(wxConvUTF8));
#endif
    curl_easy_setopt(m_curl, CURLOPT_URL, (const char *)oldURL.mb_str());
    curl_easy_setopt(m_curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(m_curl, CURLOPT_WRITEDATA, NULL);
    char cErrbuf[CURL_ERROR_SIZE] = { 0 };
    /* provide a buffer to store errors in */
    curl_easy_setopt(m_curl, CURLOPT_ERRORBUFFER, cErrbuf);
    wxString msg = wxEmptyString;

    res = curl_easy_perform(m_curl);
    if (res != CURLE_OK)
    {
        if(!ignoreInsecureError)
        {
            msg = wxString::FromUTF8(cErrbuf);
            wxLogError(wxString::Format(_("Couldn't access the URL '%s'.\n\nERROR: %s"), oldURL.c_str(), msg.c_str()));
        }
        return retURL;
    }

    res = curl_easy_getinfo(m_curl, CURLINFO_RESPONSE_CODE, &response_code);
    if (res != CURLE_OK)
    {
        /* Invalid url */
        if(!ignoreInsecureError)
        {
            msg = wxString::FromUTF8(cErrbuf);
                        wxLogError(wxString::Format(_("Couldn't access the URL '%s'.\n\nERROR: %s"), oldURL.c_str(), msg.c_str()));
        }
        return retURL;
    }

    if ((res == CURLE_OK) && ((response_code / 100) != 3))
    {
        /* a redirect implies a 3xx response code
         * Not redirecting
        */
        return oldURL;
    }

    res = curl_easy_getinfo(m_curl, CURLINFO_REDIRECT_URL, &location);
    if (res != CURLE_OK)
    {
        /* Invalid redirect url */
        if(!ignoreInsecureError)
        {
            msg = wxString::FromUTF8(cErrbuf);
            wxLogError(wxString::Format(_("Couldn't Redirect Invalid URL '%s'.\n\nERROR: %s"), oldURL.c_str(), msg.c_str()));
        }
        return retURL;
    }

    if (location)
    {
        /* This is the new absolute URL that you could redirect to, even if
         * the Location: response header may have been a relative URL.
        */
        wxString redirectedURL(location, wxConvUTF8);

        /* Debug print message is required */
                LogMessage(wxString::Format(_("\nRequested URL - %s is getting redirected to - %s\n"),oldURL.c_str(),redirectedURL.c_str()));

        /* Below logic is used to check the protocol we only
         * support redirect between equal security levels, or to a
         * higher one - e.g. http -> http, http -> https, https -> https.
        */
        int OldProtcol = GetSSLSupportStatus(oldURL);
        int redirectProtocol = GetSSLSupportStatus(redirectedURL);
        if (OldProtcol == WITH_SSL_SUPPORT && redirectProtocol == WITH_OUT_SSL_SUPPORT)
        {
            if (!ignoreInsecureError)
            wxLogError(wxString::Format(_("ERROR: The website attempted to redirect the download request from a secure connection to an insecure connection. The download cannot continue.")));
            return retURL;
        }
        else
        {
            newURL = redirectedURL;
            long redirectCount = 0;
            res = curl_easy_getinfo(m_curl, CURLINFO_REDIRECT_COUNT, &redirectCount);
            /* If redirected count is greater than 0 then the specified url has redirection. */
            if ((int)redirectCount > 0)
                newURL = GetRedirectedURL(redirectedURL, ignoreInsecureError);
        }
    }
    return newURL;
}

bool CurlHandler::ParseApplicationList()
{
#ifndef __WIN32__
    if (!m_certFileExist)
    {
        wxLogError(wxString::Format(_("Failed to open certificate file : %s"), m_certFile.c_str()));
        return false;
    }
#endif

    wxString url = GetRedirectedURL(m_applicationListUrl,false);
    if (url == wxEmptyString)
        return false;

    CURLcode curl_res;
    if (!m_curl)
    {
        wxLogError(wxString::Format(_("Failed to perform curl_easy_init")));
        return false;
    }
    /* Reset CURL to default state */
    curl_easy_reset(m_curl);

#ifndef __WIN32__
    curl_easy_setopt(m_curl, CURLOPT_CAINFO, (char*)m_certFile.char_str(wxConvUTF8));
#endif

    curl_easy_setopt(m_curl, CURLOPT_URL, (char*)url.char_str(wxConvUTF8));
    curl_easy_setopt(m_curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(m_curl, CURLOPT_WRITEDATA, &m_appListBuffer);

    curl_res = curl_easy_perform(m_curl);

    if (curl_res != CURLE_OK)
        return false;

    return true;
}


int DownloadInfo(void *p, curl_off_t dltotal, curl_off_t dlnow, curl_off_t ultotal, curl_off_t ulnow)
{
    DownloadThread *thread = static_cast<DownloadThread *>(p);

    if (thread)
    {
        thread->m_totalFileSize = dltotal;
        thread->m_totalDownloadSize = dlnow;
    }
    return 0;
}

static int ProgressFunction(void *ptr, double dltotal, double dlnow, double ultotal, double ulnow)
{
    return DownloadInfo(ptr, (curl_off_t)dltotal, (curl_off_t)dlnow,
        (curl_off_t)ultotal, (curl_off_t)ulnow);
}

static size_t WriteFunctionToFile(void *ptr, size_t size, size_t nmemb, FILE *stream)
{
    return fwrite(ptr, size, nmemb, stream);
}

static size_t ReadFunctionFromFile(void *ptr, size_t size, size_t nmemb, FILE *stream)
{
    return fread(ptr, size, nmemb, stream);
}


DownloadThread::DownloadThread():
m_downloadUrl(wxEmptyString), m_downloadFileName(wxEmptyString),
m_totalFileSize(0), m_totalDownloadSize(0)
{
}

DownloadThread::~DownloadThread(void)
{
}

wxString DownloadThread::GetDownloadPackageUrl(wxString downloadUrl, wxString packageName)
{
    int packUrlProtocolType, orgUrlProtocolType;
    wxString packageUrl = downloadUrl;
    wxString retUrl = wxEmptyString;

    if (downloadUrl == wxEmptyString)
    {
        wxLogError(wxString::Format(_("Error: URL is empty")));
        return retUrl;
    }

    if (packageName == wxEmptyString)
    {
        wxLogError(wxString::Format(_("Error: Package name is blank")));
        return retUrl;
    }

    bool ignoreInsecureError = false;
    orgUrlProtocolType = GetSSLSupportStatus(downloadUrl);

    /* First convert this url to https url if http, ignoreInsecureError should be ignored for https to ->http only
     * That means first try with https and check if availiable if yes continue else we should skip the error
     * and try with http now
    */
    if (orgUrlProtocolType == WITH_OUT_SSL_SUPPORT)
    {
        packageUrl.Replace(wxT("http://"), wxT("https://"));
        ignoreInsecureError = true;
    }

    CurlHandler curlObj;
    curlObj.SetApplicationUrl(packageUrl);
    wxString newUrl = curlObj.GetRedirectedURL(packageUrl, ignoreInsecureError);
    if (newUrl == wxEmptyString) /* This means https does not worked, now try with http */
    {
        /* If original URL is https and it fails no need to try again for http */
        if (orgUrlProtocolType == WITH_SSL_SUPPORT)
            return retUrl;
        packageUrl = downloadUrl;
        packUrlProtocolType = GetSSLSupportStatus(packageUrl);
        if (packUrlProtocolType == WITH_OUT_SSL_SUPPORT)
        {
            wxString msg = wxString::Format(_("The package '%s' cannot be downloaded using a secure connection.\n The source of the download cannot be verified through a digital certificate, and the packages's validity can only be verified by StackBuilder's internal checksum mechanism and the signature on the package, if present.\n\nDo you wish to continue ?"), packageName.c_str());
            if (wxMessageBox(msg, _("Warning"), wxYES_NO | wxICON_QUESTION) == wxNO)
                return retUrl;
        }
        newUrl = curlObj.GetRedirectedURL(packageUrl, ignoreInsecureError);
        if (newUrl == wxEmptyString)
            return retUrl;
    }


    return newUrl;
}

void *DownloadThread::Entry()
{

#ifndef __WIN32__
    wxString certFile = wxEmptyString;

    if (g_certificateBundle != wxEmptyString)
        certFile = g_certificateBundle;
    else
#ifdef __linux__
    certFile = wxPathOnly(wxStandardPaths::Get().GetExecutablePath()) + wxT("/../share/certs/ca-bundle.crt");
#else
    certFile = wxPathOnly(wxStandardPaths::Get().GetExecutablePath()) + wxT("/../Resources/certs/ca-bundle.crt");
#endif

    if (!wxFileExists(certFile))
    {
        wxLogError(wxString::Format(_("Failed to open certificate file : %s"), certFile.c_str()));
        return NULL;
    }
#endif

    CURL *curl;
    FILE *fP;
    curl = curl_easy_init();
    if (!curl)
    {
        wxLogError(wxString::Format(_("Failed to perform Download Thread - curl_easy_init")));
        return NULL;
        }

    fP = fopen((const char *)m_downloadFileName.mb_str(), "wb");
    if (!fP)
    {
        wxLogError(wxString::Format(_("Error: Cannot write file %s"), m_downloadFileName.c_str()));
        return NULL;
    }

    curl_easy_setopt(curl, CURLOPT_URL, (const char *)m_downloadUrl.mb_str());
#ifndef __WIN32__
    curl_easy_setopt(curl, CURLOPT_CAINFO, (char*)certFile.char_str(wxConvUTF8));
#endif
    curl_easy_setopt(curl, CURLOPT_PROGRESSFUNCTION, ProgressFunction);
    /* pass the struct pointer into the progress function */
    curl_easy_setopt(curl, CURLOPT_PROGRESSDATA, (void *)this);


#if LIBCURL_VERSION_NUM >= 0x072000
    /* xferinfo was introduced in 7.32.0, no earlier libcurl versions will
     * compile as they won't have the symbols around.
     * If built with a newer libcurl, but running with an older libcurl:
     * curl_easy_setopt() will fail in run-time trying to set the new
     * callback, making the older callback get used.
     * New libcurls will prefer the new callback and instead use that one even
     * if both callbacks are set.
    */

    curl_easy_setopt(curl, CURLOPT_XFERINFOFUNCTION, DownloadInfo);
    /* pass the struct pointer into the xferinfo function, note that this is
     * an alias to CURLOPT_PROGRESSDATA
    */
    curl_easy_setopt(curl, CURLOPT_XFERINFODATA, (void *)this);
#endif

    /* disable progress meter, set to 0L to enable and disable debug output */
    curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0L);

    /* write the page body to this file handle */
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, fP);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteFunctionToFile);
    curl_easy_setopt(curl, CURLOPT_READFUNCTION, ReadFunctionFromFile);
    curl_easy_perform(curl);

    fclose(fP);
    curl_easy_cleanup(curl);

    return NULL;
}
