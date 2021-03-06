/////////////////////////////////////////////////////////////////////////////
// Name:        Server.h
// Purpose:     A server object
// Author:      Dave Page
// Created:     2007-02-13
// RCS-ID:      $Id: Server.h,v 1.3 2010/08/23 13:19:12 sachin Exp $
// Copyright:   (c) EnterpriseDB
// Licence:     BSD Licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _SERVER_H
#define _SERVER_H

#include "StackBuilder.h"

// wxWindows headers
#include <wx/wx.h>

enum ServerType
{
    SVR_POSTGRESQL = 0,
    SVR_ENTERPRISEDB
};

class Server : public wxClientData
{
public:
    long port;
    wxString description;
    long majorVer;
    long minorVer;
    int serverType;

    wxString installationPath;
    wxString dataDirectory;
    wxString serverVersion;
    wxString serviceId;
    wxString superuserName;
    wxString serviceAccount;
    wxString encoding;
    wxString locale;
    wxString platform;
};

#endif

