/////////////////////////////////////////////////////////////////////////////
// Name:        DownloadPage.h
// Purpose:     Download page of the wizard
// Author:      Dave Page
// Created:     2007-02-13
// RCS-ID:      $Id: DownloadPage.cpp,v 1.7 2007/04/13 10:32:27 dpage Exp $
// Copyright:   (c) EnterpriseDB
// Licence:     BSD Licence
/////////////////////////////////////////////////////////////////////////////

#include "StackBuilder.h"

// wxWindows headers
#include <wx/wx.h>
#include <wx/wizard.h>
#include <wx/button.h>
#include <wx/dir.h>
#include <wx/dirdlg.h>
#include <wx/stdpaths.h>
#include <wx/msw/registry.h>

// Application headers
#include "DownloadPage.h"
#include "AppList.h"
#include "MirrorList.h"

const int BTN_BROWSE=1001;

BEGIN_EVENT_TABLE(DownloadPage, wxWizardPageSimple)
    EVT_BUTTON(BTN_BROWSE,                  DownloadPage::OnBrowse)
    EVT_WIZARD_PAGE_CHANGING(wxID_ANY,		DownloadPage::OnWizardPageChanging)
END_EVENT_TABLE()

DownloadPage::DownloadPage(wxWizard *parent, AppList *applist, MirrorList *mirrorlist) 
	: wxWizardPageSimple(parent)
{
    m_applist = applist;
    m_mirrorlist = mirrorlist;

    wxBoxSizer *mainSizer = new wxBoxSizer(wxVERTICAL);

	mainSizer->Add(0, 10);

	wxStaticText *st = new wxStaticText(this, wxID_ANY, _("Select the download directory if required, and then click the Next button to begin downloading the packages you have selected."));
	st->Wrap(400);
    mainSizer->Add(st, 0, wxALL, 5);

	mainSizer->Add(0, 80);

    st = new wxStaticText(this, wxID_ANY, _("Download directory:"));
	st->Wrap(350);
    mainSizer->Add(st, 0, wxALL, 5);

    // Get the download path
	wxRegKey *key = new wxRegKey(wxT("HKEY_LOCAL_MACHINE\\Software\\PostgreSQL\\Stack Builder\\"));

    wxString path;
    wxStandardPaths sp;

	if (!key->Exists() || !key->HasValue(wxT("Download Path")))
		path = sp.GetTempDir();
    else
        key->QueryValue(wxT("Download Path"), path);

    delete key;

    // Add the path textbox and browse button
    wxBoxSizer *pathSizer = new wxBoxSizer(wxHORIZONTAL);

    m_path = new wxTextCtrl(this, wxID_ANY, path, wxDefaultPosition, wxSize(300, -1));
	pathSizer->Add(m_path, 0, wxALL | wxALIGN_CENTER, 2);

    m_browse = new wxButton(this, BTN_BROWSE, wxT("..."), wxDefaultPosition, wxSize(25, -1));
    pathSizer->Add(m_browse, 0, wxALL | wxALIGN_CENTER, 2);

    mainSizer->Add(pathSizer, 0, wxALL | wxALIGN_CENTER, 5);

    SetSizer(mainSizer);
    mainSizer->Fit(this);
}

void DownloadPage::OnBrowse(wxCommandEvent& WXUNUSED(event))
{
    wxDirDialog *dd = new wxDirDialog(this, _("Select a download directory"), m_path->GetValue());

    if (dd->ShowModal() != wxID_CANCEL)
        m_path->SetValue(dd->GetPath());

    delete dd;
}

void DownloadPage::OnWizardPageChanging(wxWizardEvent& event)
{
	// If we're going backwards, just bail out
	if (!event.GetDirection())
		return;

    if (!wxDir::Exists(m_path->GetValue()))
	{
        wxLogError(_("The download directory does not exist. Please select a valid directory."));
		event.Veto();
		return;
	}

    // Store the download location for next time
    wxRegKey *key = new wxRegKey(wxT("HKEY_LOCAL_MACHINE\\Software\\PostgreSQL\\Stack Builder\\"));
    key->SetValue(wxT("Download Path"), m_path->GetValue());
    delete key;

    if (!m_applist->DownloadFiles(m_path->GetValue(), m_mirrorlist->GetSelectedMirror()))
    {
    	event.Veto();
		return;
	}
}

