/////////////////////////////////////////////////////////////////////////////
// Name:        DownloadPage.h
// Purpose:     Download page of the wizard
// Author:      Dave Page
// Created:     2007-02-13
// RCS-ID:      $Id: DownloadPage.h,v 1.1 2007/02/19 09:57:00 dpage Exp $
// Copyright:   (c) EnterpriseDB
// Licence:     BSD Licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _DOWNLOADPAGE_H
#define _DOWNLOADPAGE_H

#include "StackBuilder.h"

// wxWindows headers
#include <wx/wx.h>
#include <wx/wizard.h>

class DownloadPage : public wxWizardPageSimple
{
public:
    DownloadPage(wxWizard *parent);
    virtual bool TransferDataFromWindow();

private:

};

#endif