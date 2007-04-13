/////////////////////////////////////////////////////////////////////////////
// Name:        AppSelectionPage.cpp
// Purpose:     Application selection page of the wizard
// Author:      Dave Page
// Created:     2007-02-13
// RCS-ID:      $Id: AppSelectionPage.cpp,v 1.6 2007/04/13 14:17:08 dpage Exp $
// Copyright:   (c) EnterpriseDB
// Licence:     BSD Licence
/////////////////////////////////////////////////////////////////////////////

#include "StackBuilder.h"

// wxWindows headers
#include <wx/wx.h>
#include <wx/busyinfo.h>
#include <wx/imaglist.h>
#include <wx/wizard.h>
#include <wx/treectrl.h>

// Application headers
#include "AppSelectionPage.h"
#include "App.h"
#include "AppList.h"
#include "MirrorList.h"
#include "MirrorSelectionPage.h"
#include "images/bullet.xpm"
#include "images/checked.xpm"
#include "images/disabled.xpm"
#include "images/unchecked.xpm"

BEGIN_EVENT_TABLE(AppTreeCtrl, wxTreeCtrl)
	EVT_LEFT_DOWN(							AppTreeCtrl::OnLeftClick)
END_EVENT_TABLE()

BEGIN_EVENT_TABLE(AppSelectionPage, wxWizardPageSimple)
	EVT_TREE_SEL_CHANGED(wxID_ANY,			AppSelectionPage::OnTreeItemSelected)
	EVT_TREE_ITEM_ACTIVATED(wxID_ANY,       AppSelectionPage::OnTreeItemActivated)
    EVT_WIZARD_PAGE_CHANGING(wxID_ANY,		AppSelectionPage::OnWizardPageChanging)
END_EVENT_TABLE()


AppTreeCtrl::AppTreeCtrl(wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style)
:wxTreeCtrl(parent, id, pos, size, style)
{
}

void AppTreeCtrl::OnLeftClick(wxMouseEvent &evt)
{
	wxTreeItemId node = GetSelection();

	if (node && node == HitTest(evt.GetPosition()))
	{
		App *app = (App *)GetItemData(node);

		if (app)
		{
			if (GetItemImage(node) == 0)
			{
				SetItemImage(node, 1);
				app->SelectForDownload(true, false);
			}
			else if (GetItemImage(node) == 1)
			{
				if (app->IsSelectedAsDependency())
				{
					if (wxMessageBox(_("This application was automatically selected because another seleciton is dependent upon it.\nIf you do not install this application others may not install or work correctly.\n\nAre you sure you wish to continue?"), _("Deselect application"), wxYES_NO | wxICON_EXCLAMATION) == wxNO)
						return;
				}
				SetItemImage(node, 0);
				app->SelectForDownload(false, false);
			}
		}
	}
	evt.Skip();
}


AppSelectionPage::AppSelectionPage(wxWizard *parent, AppList *applist, MirrorList *mirrorlist) 
	: wxWizardPageSimple(parent)
{
	m_applist = applist;
	m_mirrorlist = mirrorlist;

    wxBoxSizer *mainSizer = new wxBoxSizer(wxVERTICAL);

	mainSizer->Add(0, 10);

	wxStaticText *st = new wxStaticText(this, wxID_ANY, _("Please select the applications you would like to install."));
	st->Wrap(350);
    mainSizer->Add(st, 0, wxALL, 5);

    m_apptree = new AppTreeCtrl(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTR_DEFAULT_STYLE);
	m_treeimages = new wxImageList(16, 16, true, 4);
	m_treeimages->Add(wxIcon(unchecked_xpm));
	m_treeimages->Add(wxIcon(checked_xpm));
	m_treeimages->Add(wxIcon(disabled_xpm));
	m_treeimages->Add(wxIcon(bullet_xpm));
	m_apptree->SetImageList(m_treeimages);
	mainSizer->Add(m_apptree, 4, wxALL | wxEXPAND, 5);

	mainSizer->Add(0, 10);

	m_description = new wxTextCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_READONLY | wxTE_MULTILINE);
    m_description->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNFACE));
    mainSizer->Add(m_description, 1, wxALL | wxEXPAND, 5);

    SetSizer(mainSizer);
    mainSizer->Fit(this);
}

void AppSelectionPage::OnWizardPageChanging(wxWizardEvent& event)
{
	// If we're going backwards, clear the tree view
	if (!event.GetDirection())
	{
		m_applist->DeleteAllItems();
		m_apptree->DeleteAllItems();
		return;
	}

	if (!m_applist->HaveDownloads())
	{
		wxLogError(_("You must select at least one package to install before you continue."));
		event.Veto();
		return;
	}

    m_mirrorlist->SetTree(((MirrorSelectionPage *)GetNext())->GetTreeCtrl());
	m_applist->RankDownloads();

	// Get the mirror list, parse it and build the tree
	bool retval;
	{
		wxWindowDisabler disableAll;
		wxBusyInfo info(_("Downloading mirror list..."));
		wxTheApp->Yield();
		retval = m_mirrorlist->LoadMirrorList();
	}

	if (!retval)
	{
		event.Veto();
		return;
	}

	retval = m_mirrorlist->PopulateTreeCtrl();

	if (!retval)
	{
		event.Veto();
        return;
	}

    return;
}

void AppSelectionPage::OnTreeItemSelected(wxTreeEvent &evt)
{
	wxTreeItemId node = evt.GetItem();

	App *app = (App *)m_apptree->GetItemData(node);

	if (app)
		m_description->SetValue(app->description);
	else
		m_description->SetValue(wxEmptyString);
}

void AppSelectionPage::OnTreeItemActivated(wxTreeEvent &evt)
{
	wxTreeItemId node = m_apptree->GetSelection();

	if (!node)
		return;

	App *app = (App *)m_apptree->GetItemData(node);

	if (app)
	{
		if (m_apptree->GetItemImage(node) == 0)
		{
			m_apptree->SetItemImage(node, 1);
			app->SelectForDownload(true, false);
		}
		else if (m_apptree->GetItemImage(node) == 1)
		{
			if (app->IsSelectedAsDependency())
			{
				if (wxMessageBox(_("This application was automatically selected because another seleciton is dependent upon it.\nIf you do not install this application others may not install or work correctly.\n\nAre you sure you wish to continue?"), _("Deselect application"), wxYES_NO | wxICON_EXCLAMATION) == wxNO)
					return;
			}
			m_apptree->SetItemImage(node, 0);
			app->SelectForDownload(false, false);
		}
	}
}

