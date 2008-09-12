/////////////////////////////////////////////////////////////////////////////
// Name:        Wizard.cpp
// Purpose:     The StackBuilder Wizard
// Author:      Dave Page
// Created:     2007-02-13
// RCS-ID:      $Id: Wizard.h,v 1.5 2008/09/12 10:47:00 dpage Exp $
// Copyright:   (c) EnterpriseDB
// Licence:     BSD Licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WIZARD_H
#define _WIZARD_H

#include "StackBuilder.h"

// wxWindows headers
#include <wx/wx.h>
#include <wx/wizard.h>

class AppList;
class MirrorList;

class Wizard : public wxWizard
{
public:
    Wizard(wxFrame *frame, wxBitmap bitmap, const wxString &applicationlisturl, const wxString &mirrorlisturl, const wxString &title);
    wxWizardPage *GetFirstPage() const { return m_page1; }

private:
    wxWizardPageSimple *m_page1, *m_page2, *m_page3, *m_page4, *m_page5, *m_page6;
    AppList *m_applist;
    MirrorList *m_mirrorlist;
};

#endif

