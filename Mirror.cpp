/////////////////////////////////////////////////////////////////////////////
// Name:        Mirror.cpp
// Purpose:     A mirror object
// Author:      Dave Page
// Created:     2007-02-13
// RCS-ID:      $Id: Mirror.cpp,v 1.3 2008/06/11 10:58:04 dpage Exp $
// Copyright:   (c) EnterpriseDB
// Licence:     BSD Licence
/////////////////////////////////////////////////////////////////////////////

#include "StackBuilder.h"

// wxWindows headers
#include <wx/wx.h>
#include <wx/treectrl.h>

// Application headers
#include "Mirror.h"

bool Mirror::IsValid()
{
    if (protocol != wxT("ftp") && protocol != wxT("http"))
        return false;

    if (country == wxEmptyString)
        return false;

    if (hostname == wxEmptyString)
        return false;

    return true;
}
