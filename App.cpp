/////////////////////////////////////////////////////////////////////////////
// Name:        App.cpp
// Purpose:     An application object
// Author:      Dave Page
// Created:     2007-02-13
// RCS-ID:      $Id: App.cpp,v 1.42 2011/11/28 19:09:57 dpage Exp $
// Copyright:   (c) EnterpriseDB
// Licence:     BSD Licence
/////////////////////////////////////////////////////////////////////////////

#include "StackBuilder.h"

#ifdef __WXMAC__
#include <errno.h>

// We're going to use auto_ptr...
#include <memory>
using namespace std;
#endif

// wxWindows headers
#include <wx/wx.h>
#include <wx/busyinfo.h>
#include <wx/dir.h>
#include <wx/fileconf.h>
#include <wx/progdlg.h>
#include <wx/stream.h>
#include <wx/timer.h>
#include <wx/treectrl.h>
#include <wx/url.h>
#include <wx/wfstream.h>
#include <wx/zipstrm.h>
#include <wx/protocol/http.h>

#ifdef __WXMAC__
#include <wx/xml/xml.h>
#endif

#ifdef __WXMSW__
#include "Registry.h"
#endif

// Application headers
#include "App.h"
#include "AppList.h"
#include "MD5.h"
#include "Mirror.h"
#include "Server.h"
#include "ProxyDialog.h"
#include "CurlHandler.h"

App::App(AppList *applist, Server *server)
{
    m_applist = applist;
    m_server = server;
    sequence = 0;
    download = false;
    isDependency = false;
    downloaded = false;
    installed = false;
    m_tree = NULL;
};

bool App::IsValid()
{
    return (!id.IsEmpty() &&
            !name.IsEmpty() &&
            !version.IsEmpty() &&
            !category.IsEmpty() &&
            !format.IsEmpty() &&
            !checksum.IsEmpty() &&
            !(mirrorpath.IsEmpty() && alturl.IsEmpty()) &&
            !versionkey.IsEmpty());
}

bool App::IsInstalled()
{
    wxString ver;

#ifdef __WXMSW__
    pgRegKey::PGREGWOWMODE wowMode = pgRegKey::PGREG_WOW_DEFAULT;

    if (::wxIsPlatform64Bit())
    {
        if (platform.IsSameAs(wxT("windows")))
            wowMode = pgRegKey::PGREG_WOW32;
        else if(platform.IsSameAs(wxT("windows-x64")))
            wowMode = pgRegKey::PGREG_WOW64;
    }

    pgRegKey *key = pgRegKey::OpenRegKey(HKEY_LOCAL_MACHINE, versionkey.BeforeLast('\\'), pgRegKey::PGREG_READ, wowMode);

    if (key == NULL)
        return false;

    // Also consider the app not installed if the value exists but is empty
    key->QueryValue(versionkey.AfterLast('\\'), ver);

    delete key;
#else
    // Check for the registry
    if (!wxFile::Exists(REGISTRY_FILE))
        return false;

    wxFileInputStream fst(REGISTRY_FILE);
    wxFileConfig *cnf = new wxFileConfig(fst);

    if (!cnf->HasEntry(versionkey))
        return false;

    // Also consider the app not installed if the value exists but is empty
    ver = cnf->Read(versionkey, wxEmptyString);
    delete cnf;
#endif

    if (ver == wxEmptyString)
        return false;

    return true;
}

bool App::IsVersionInstalled()
{
    wxString ver;

#ifdef __WXMSW__
    pgRegKey::PGREGWOWMODE wowMode = pgRegKey::PGREG_WOW_DEFAULT;

    if (::wxIsPlatform64Bit())
    {
        if (platform.IsSameAs(wxT("windows")))
            wowMode = pgRegKey::PGREG_WOW32;
        else if(platform.IsSameAs(wxT("windows-x64")))
            wowMode = pgRegKey::PGREG_WOW64;
    }

    pgRegKey *key = pgRegKey::OpenRegKey(HKEY_LOCAL_MACHINE, versionkey.BeforeLast('\\'), pgRegKey::PGREG_READ, wowMode);

    if (key == NULL)
        return false;

    key->QueryValue(versionkey.AfterLast('\\'), ver);

    delete key;
#else
    // Check for the registry
    if (!wxFile::Exists(REGISTRY_FILE))
        return false;

    wxFileInputStream fst(REGISTRY_FILE);
    wxFileConfig *cnf = new wxFileConfig(fst);

    if (!cnf->HasEntry(versionkey))
        return false;

    // Also consider the app not installed if the value exists but is empty
    ver = cnf->Read(versionkey, wxEmptyString);
    delete cnf;
#endif

    if (ver != version)
        return false;

    return true;
}

bool App::WorksWithDB()
{
    wxString tmpversion;

    if (!m_server)
        return true;

    switch (m_server->serverType)
    {
        case SVR_POSTGRESQL:
            tmpversion = pgversion.Trim();
            break;

        case SVR_ENTERPRISEDB:
            tmpversion = edbversion.Trim();
            break;

        default:
            tmpversion = wxEmptyString;
    }

    if (tmpversion == wxEmptyString)
        return true;

    long appMajor = 0, appMinor = 0;
    tmpversion.BeforeFirst('.').ToLong(&appMajor);
    if (appMajor < 10)
        tmpversion.AfterFirst('.').ToLong(&appMinor);

    if (tmpversion.EndsWith(wxT("+")))
        return (m_server->majorVer > appMajor || (m_server->majorVer == appMajor && m_server->minorVer >= appMinor));
    else
        return (m_server->majorVer == appMajor && m_server->minorVer == appMinor);

    return false;
}

bool App::WorksWithPlatform()
{
#ifndef __WXMSW__
    if (platform != STACKBUILDER_PLATFORM)
        return false;
#else
    // For remote server - we will have empty string for the platform in the dummyServer
    // We will allow 32 bit and 64 bit applications to be downloaded and installed on 64 bit windows
    // But, 64 bit applications will be allowed to get installed/downloaded on 32 bit windows.
    if (m_server && m_server->platform == wxEmptyString)
    {
        if (platform == wxT("windows") || (::wxIsPlatform64Bit() && platform == wxT("windows-x64")))
            return true;
        return false;
    }

    if (m_server && m_server->platform != platform)
    {
        // Check if SBP is running on 64 bit platform and
        // server's architecture (platform - windows/windows-x64) can match with the secondaryplatform.
        // StackBuilder - itself can be 32/64 bit and will works for both 32 bit and 64 bit PostgreSQL server.
        if (!::wxIsPlatform64Bit() || secondaryplatform != m_server->platform)
           return false;
    }
#endif
    return true;
}

wxString App::GetInstalledVersion()
{
    wxString ver;

#ifdef __WXMSW__
    pgRegKey::PGREGWOWMODE wowMode = pgRegKey::PGREG_WOW_DEFAULT;

    if (::wxIsPlatform64Bit())
    {
        if (platform.IsSameAs(wxT("windows")))
            wowMode = pgRegKey::PGREG_WOW32;
        else if(platform.IsSameAs(wxT("windows-x64")))
            wowMode = pgRegKey::PGREG_WOW64;
    }

    pgRegKey *key = pgRegKey::OpenRegKey(HKEY_LOCAL_MACHINE, versionkey.BeforeLast('\\'), pgRegKey::PGREG_READ, wowMode);

    if (key == NULL)
        return wxEmptyString;

    key->QueryValue(versionkey.AfterLast('\\'), ver);
    delete key;
#else
    // Check for the registry
    if (!wxFile::Exists(REGISTRY_FILE))
        return wxEmptyString;

    wxFileInputStream fst(REGISTRY_FILE);
    wxFileConfig *cnf = new wxFileConfig(fst);

    if (!cnf->HasEntry(versionkey))
        return wxEmptyString;

    ver = cnf->Read(versionkey, wxEmptyString);
    delete cnf;
#endif
    return ver;
}

void App::SelectForDownload(bool select, bool isdep)
{
    // If this item doesn't have a checkbox image, we cannot select it.
    if (!m_tree || m_tree->GetItemImage(m_treeitem) >= 2)
        return;

    // Deselect item if required.
    if (!select)
    {
        download = false;
        isDependency = false;
        m_tree->SetItemImage(m_treeitem, 0);
        return;
    }
    else
    {
        // If this item is already selected, bail out to avoid
        // the possiblity of hitting an infinite loop.
        if (download)
            return;

        download = true;
        if (isdep)
            isDependency = true;

        m_tree->SetItemImage(m_treeitem, 1);

        // Select all dependencies
        for (unsigned int x=0; x < dependencies.Count(); x++)
        {
            for (unsigned int y=0; y < m_applist->Count(); y++)
            {
                if (m_applist->GetItem(y)->id == dependencies[x] && !m_applist->GetItem(y)->IsInstalled())
                    m_applist->GetItem(y)->SelectForDownload(true, true);
            }
        }
    }
}

// Figure out what order to download and install
int App::RankDependencies(int rank, unsigned int depth)
{
    // Only recurse if we've not gone too deep
    if (depth < m_applist->Count())
    {
        // Iterate through the dependencies, setting the sequence as required
        for (unsigned int i=0; i<dependencies.GetCount(); i++)
        {
            App *dep = m_applist->GetItem(dependencies[i]);
            if (!dep->IsSelectedForDownload() || dep->sequence > 0)
                continue;

            depth++;
            rank = dep->RankDependencies(rank, depth);
        }
    }

    depth--;

    // The sequence might already be set if we've gone through a
    // circular dependency. In that case, don't reset it.
    if (!sequence)
    {
        sequence = rank;
        return sequence + 1;
    }
    else
        return rank;
}

bool App::Download(const wxString& downloadPath, const Mirror *mirror)
{
    // Ensure we only log the download once
    bool haveLogged = false;

    if (!CheckFilename(downloadPath))
        return false;

    // If this file has already been downloaded, don't bother getting it again.
    // GetFilename would have set this flag if it found a matching filename and
    // the checksum was correct.
    if (downloaded)
        return true;

    wxString downloadFilename = wxEmptyString;
#ifndef __WIN32__
    downloadFilename = downloadPath + wxT("/") + file.GetFullName();
#else
    downloadFilename = downloadPath + wxT("\\") + file.GetFullName();
#endif

    wxString theUrl;
    if (alturl.IsEmpty())
        theUrl = wxString::Format(wxT("%s://%s%s%s/%s"),
        mirror->protocol.c_str(),
        mirror->hostname.c_str(),
        (mirror->port == 0 ? wxEmptyString : wxString::Format(wxT(":%d"), mirror->port).c_str()),
        mirror->rootpath.c_str(),
        mirrorpath.c_str());
    else
        theUrl = alturl;


    DownloadThread *dt = new DownloadThread();
    if (dt == NULL)
    {
        wxLogError(wxString::Format(_("Failed to create new DownloadThread")));
        return false;
    }

    wxString packageName = file.GetFullName();
    wxString newURL = dt->GetDownloadPackageUrl(theUrl, packageName);
    if (newURL == wxEmptyString)
        return false;

    dt->SetDownloadUrl(newURL);
    dt->SetDownloadFileName(downloadFilename);

    wxProgressDialog *pd = new wxProgressDialog(wxString::Format(_("Downloading %s"), wxFileName(mirrorpath).GetFullName().c_str()),
        _("Connecting to server..."),
        100,
        NULL,
        wxPD_APP_MODAL | wxPD_SMOOTH | wxPD_CAN_ABORT | wxPD_ELAPSED_TIME);
    pd->SetSize(500, -1);
    pd->CentreOnParent();
    pd->Show();

    if (dt->Create() != wxTHREAD_NO_ERROR)
    {
        wxLogError(wxString::Format(_("Failed to create Download Thread ")));
        return false;
    }
    if (dt->Run() != wxTHREAD_NO_ERROR)
    {
        wxLogError(wxString::Format(_("Couldn't initalise the downloading process")));
        return false;
    }

    bool abort = false;
    wxStopWatch sw;
    long updateTime = 100;
    long speed = 0;
    int kbDownloaded;
    int kbFileSize;
    wxString msg;

    do
    {
        if (sw.Time() >= updateTime)
        {
            /* Calculate the download speed */
            if (updateTime >= 1000)
                speed = round((dt->GetTotalFileSize() / KBSIZE) / (updateTime / 1000));

            /* Set the next time to update the progress dialog */
            updateTime += 100;

            kbFileSize = (int)dt->GetTotalFileSize() / KBSIZE;
            kbDownloaded = (int)dt->GetTotalDownloadSize() / KBSIZE;

            // To ignore initial few 0 values which is not updated
            if ((kbFileSize == 0) || (kbDownloaded == 0))
                continue;

            if (kbFileSize > kbDownloaded)
                msg = wxString::Format(_("Downloaded %d KB of  %d KB (%d KB/Sec)"), kbDownloaded, kbFileSize, speed);
            else
            {
                msg = wxString::Format(_("Downloaded %6.0lf KB (%d KB/Sec)"), kbDownloaded, speed);
                break;
            }

            if (!pd->Pulse(msg, &abort))
            {
                wxRemoveFile(file.GetFullPath());
                pd->Show(false);
                delete pd;
                if (dt && dt->TestDestroy())
                {
                    delete dt;
                    dt = NULL;
                    return false;
                }
                return false;

            }
        }
    } while (1);

    pd->Show(false);
    delete pd;

    // Having downloaded the file, now verify the checksum
    wxString tmpsum;

    {
        wxBusyInfo wait(wxString::Format(_("Verifying checksum for: %s"), file.GetFullName().c_str()));
        tmpsum = md5sum(file.GetFullPath());
    }

    if (tmpsum.Lower() == checksum.Lower())
    {
        downloaded = true;
        return true;
    }
    else
    {
        downloaded = false;
        wxLogError(_("Checksum verification failed for: %s! Deleting file..."), file.GetFullName().c_str());
        wxRemoveFile(file.GetFullPath());
        return false;
    }

    return true;
}

bool App::CheckFilename(const wxString& downloadPath)
{
    wxString thePath;
    if (!alturl.IsEmpty())
        thePath = alturl;
    else
        thePath = mirrorpath;

    // NOTE: If we're using the alturl here, the protocol will
    //       become the drive, and the hostname etc. part of
    //       the path in the wxFileName. That's not a problem
    //       now, but might need to be fixed if this code is
    //       hacked in the future.
    wxFileName svrFile(thePath);

#ifndef __WXMAC__
    file = downloadPath + wxT("/") + id + wxT(".") + format;
#else
    // On Mac, installers are always bundles, so they must be zipped for transport
    file = downloadPath + wxT("/") + id + wxT(".") + format + wxT(".zip");
#endif

    if (file.FileExists())
    {
        // Check the file to see if it's checksum matches ours. If
        // if does, keep the filename and set the 'Downloaded' flag
        wxString tmpsum;

        {
            wxBusyInfo wait(wxString::Format(_("Checking existing file: %s"), file.GetFullName().c_str()));
            tmpsum = md5sum(file.GetFullPath());
        }

        if (tmpsum.Lower() == checksum.Lower())
        {
            downloaded = true;
            return true;
        }

        // OK, the file is no good, so rename it
        wxDateTime now = wxDateTime::Now();
        if (!wxRenameFile(file.GetFullPath(), file.GetFullPath() + wxT("-") + now.Format(wxT("%Y%m%d%H%M%S"))))
        {
            wxLogError(_("Failed to rename the file\n\n%s\n\nto\n\n%s-%s"), file.GetFullPath().c_str(), file.GetFullPath().c_str(), now.Format(wxT("%Y%m%d%H%M%S")).c_str());
            return false;
        }
    }

    downloaded = false;
    return true;
}

bool App::Install()
{
    // Have we already installed this package in this session?
    if (installed)
        return true;

    wxString cmd, args;

    // Install or upgrade?
    if (IsInstalled())
    {
        if (!upgradeoptions.IsEmpty())
            args = SubstituteFlags(upgradeoptions);
    }
    else
    {
        if (!installoptions.IsEmpty())
            args = SubstituteFlags(installoptions);
    }

#ifdef __WXMSW__
    // MSI or EXE?
    if (format.Lower() == wxT("msi"))
        cmd = wxT("msiexec /i \"") + file.GetFullPath() + wxT("\" ") + args;
    else
        cmd = wxT("\"") + file.GetFullPath() + wxT("\" ") + args;
#else
#ifdef __WXMAC__
    // Unpack the downloaded file

    // Create a directory to put it in. This is a little ugly, but
    // wxWidgets only allows us to create a temp file
    wxString macTmpPath = wxFileName::CreateTempFileName(wxT("/tmp/"));
    wxRemoveFile(macTmpPath);
    wxMkdir(macTmpPath);

    // Prepare to extract
    auto_ptr<wxZipEntry> entry;

    wxFFileInputStream in(file.GetFullPath());
    wxZipInputStream zip(in);

    while (entry.reset(zip.GetNextEntry()), entry.get() != NULL)
    {
        // Get the filename
        if (entry->GetName().StartsWith(wxT("__MACOSX")))
            continue;

        if (entry->GetName().EndsWith(wxT("/")))
        {
            if (!wxMkdir(macTmpPath + wxT("/") + entry->GetName()))
            {
                wxLogError(_("Failed to create the installer appbundle directory: %s/%s"), macTmpPath.c_str(), entry->GetName().c_str());
                return false;
            }
        }
        else
        {
            // Must be an actual file, so extract it
            wxFFileOutputStream out(macTmpPath + wxT("/") + entry->GetName());
            if (!out.IsOk())
            {
                wxLogError(_("Failed to write the installer appbundle file: %s/%s"), macTmpPath.c_str(), entry->GetName().c_str());
                return false;
            }
            zip >> out;
            out.Close();
            chmod(wxString::Format(wxT("%s/%s"), macTmpPath.c_str(), entry->GetName().c_str()).ToAscii(), entry->GetMode());
        }

    }

	// Find the bundle. This will be the first .app, .pkg or .mpkg found in the temp dir
	wxString bundle, filespec;
	wxDir(dir);

	if (format.Lower() == wxT("app"))
		filespec = wxT("*.app");
	else if (format.Lower() == wxT("pkg"))
		filespec = wxT("*.pkg");
	else if (format.Lower() == wxT("mpkg"))
		filespec = wxT("*.mpkg");

	dir.Open(macTmpPath);
	dir.GetFirst(&bundle, filespec, wxDIR_DIRS);

    // If this is a pkg or mpkg, just throw it at open. Note that you cannot pass arguments
    // to this type of installer
    wxString installer = macTmpPath + wxT("/") + bundle;

    if (format.Lower() == wxT("pkg") || format.Lower() == wxT("mpkg"))
        cmd = wxT("open -W \"") + installer + wxT("\"");
    else
    {
        // On the Mac, having unpacked an appbundle, we must read the
        // CFBundleExecutable value from installer.app/Contents/Info.plist
        // and then execute that.
        wxString exe = GetBundleExecutable(installer);

        if (exe.IsEmpty())
        {
            wxLogError(_("Failed to read the CFBundleExecutable value from the appbundle description: %s/Contents/Info.plist"), installer.c_str());
            return false;
        }

        cmd = wxT("\"") + installer + wxT("/Contents/MacOS/") + exe + wxT("\" ") + args;
    }

#else
    // Everything is executed directly on *nix, as we cannot support RPM/DEB in any
    // non-distro specific way.
    chmod(file.GetFullPath().ToAscii(), S_IRUSR | S_IWUSR | S_IXUSR);
    cmd = wxT("\"") + file.GetFullPath() + wxT("\" ") + args;
#endif
#endif

    // Now run the installation
    if (cmd.IsEmpty())
        return false;

#ifdef __WXMSW__
    long retval = wxExecute(cmd.Trim(), wxEXEC_SYNC);
#else
    // wxExecute can sometime fail to call wait() for some reason, leading to zombies
    long retval = ExecProcess(cmd.Trim());
#endif

#ifdef __WXMAC__
    ExecProcess(wxT("rm -rf ") + macTmpPath);
#endif

    if (retval == 0) // Installed OK
    {
        installed = true;
        return true;
    }
    else
    {
        int response = wxMessageBox(wxString::Format(_("The installation of %s returned an error.\n\n Click on the OK button to ignore this error and continue with any remaining installations, or click Cancel to abort the remaining installations.\n\nNote that ignoring this error may result in failure of any later installations that depend on this one."), this->name.c_str()),
                                                     _("Installation error"),
                                                     wxOK | wxCANCEL | wxICON_EXCLAMATION);

        if (response == wxCANCEL) // User cancelled the installations
            return false;
        else // User skipped this installation
        {
            m_applist->IncrementErrorCount();
            installed = true;
            return true;
        }
    }
}

wxString App::SubstituteFlags(const wxString &options)
{
    wxString retval = options;

    // Do we have any server options?
    if (!m_server)
    {
        retval.Replace(wxT("$LOCALE"), wxEmptyString); // Locale first, otherwise LOCAL will match!
        retval.Replace(wxT("$LOCAL"), wxT("0"));
        retval.Replace(wxT("$PATH"), wxEmptyString);
        retval.Replace(wxT("$DATA"), wxEmptyString);
        retval.Replace(wxT("$VERSION"), wxEmptyString);
        retval.Replace(wxT("$PORT"), wxEmptyString);
        retval.Replace(wxT("$SERVICE"), wxEmptyString);
        retval.Replace(wxT("$ACCOUNT"), wxEmptyString);
        retval.Replace(wxT("$SUPER"), wxEmptyString);
        retval.Replace(wxT("$ENCODING"), wxEmptyString);
    }
    else
    {
        retval.Replace(wxT("$LOCALE"), m_server->locale); // Locale first, otherwise LOCAL will match!
        retval.Replace(wxT("$LOCAL"), wxT("1"));
        retval.Replace(wxT("$PATH"), m_server->installationPath);
        retval.Replace(wxT("$DATA"), m_server->dataDirectory);
        retval.Replace(wxT("$VERSION"), m_server->serverVersion);
        retval.Replace(wxT("$PORT"), wxString::Format(wxT("%ld"), m_server->port));
        retval.Replace(wxT("$SERVICE"), m_server->serviceId);
        retval.Replace(wxT("$ACCOUNT"), m_server->serviceAccount);
        retval.Replace(wxT("$SUPER"), m_server->superuserName);
        retval.Replace(wxT("$ENCODING"), m_server->encoding);
    }

    return retval;
}

#ifdef __WXMAC__
wxString App::GetBundleExecutable(const wxString &bundle)
{
    wxFileInputStream ip(bundle + wxT("/Contents/Info.plist"));

    if (!ip || !ip.IsOk())
        return wxEmptyString;

    wxXmlDocument xml;
    if (!xml.Load(ip))
        return wxEmptyString;

    // Iterate through the applications and build the list
    wxXmlNode *dict, *properties;
    dict = xml.GetRoot()->GetChildren();
    bool next = false;

    while (dict)
    {
        if (dict->GetName() == wxT("dict"))
        {
            properties = dict->GetChildren();

            while (properties)
            {
                // Look for the key with the value CFBundleExecutable
                // When we've found it, the next string should be our value
                if (!next)
                {
                    if (properties->GetName() == wxT("key"))
                    {
                        if (properties->GetNodeContent().Lower() == wxT("cfbundleexecutable"))
                            next = true;
                    }
                }
                else
                {
                    if (properties->GetName() == wxT("string"))
                    {
                        return properties->GetNodeContent();
                    }
                }

                properties = properties->GetNext();
            }
        }

        dict = dict->GetNext();
    }

    return wxEmptyString;
}
#endif

#ifndef __WXMSW__
int App::ExecProcess(const wxString &cmd)
{
    wxString res;
    FILE *f=popen(cmd.ToAscii(), "r");

    if (f)
    {
        char buffer[KBSIZE];
        int cnt;
	int rc;

        while ((cnt = fread(buffer, 1, KBSIZE, f)) > 0)
        {
            buffer[cnt] = 0;
            res += wxString::FromAscii(buffer);
        }

	rc = pclose(f);

        // Check for a non-zero return code, or an error string. BitRock
        // installers don't always exit with a non-zero when they error.
        if (rc || res.Lower().Matches(wxT("*error*")))
            wxLogError(res);

        return rc;
    }
    return -1;
}
#endif

bool App::IsRequired()
{
    for (unsigned int y=0; y < m_applist->Count(); y++)
    {
        App *app = (App *) m_applist->GetItem(y);
        if (app->IsSelectedForDownload() && app->dependencies.Index(id) != wxNOT_FOUND)
             return true;
    }
    return false;
}
