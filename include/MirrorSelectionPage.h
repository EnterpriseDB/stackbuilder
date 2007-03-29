/////////////////////////////////////////////////////////////////////////////
// Name:        MirrorSelectionPage.h
// Purpose:     Mirror selection page of the wizard
// Author:      Dave Page
// Created:     2007-02-13
// RCS-ID:      $Id: MirrorSelectionPage.h,v 1.3 2007/03/29 11:39:40 dpage Exp $
// Copyright:   (c) EnterpriseDB
// Licence:     BSD Licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _MIRRORSELECTIONPAGE_H
#define _MIRRORSELECTIONPAGE_H

#include "StackBuilder.h"

// wxWindows headers
#include <wx/wx.h>
#include <wx/wizard.h>
#include <wx/treectrl.h>

class MirrorList;

class MirrorSelectionPage : public wxWizardPageSimple
{
public:
    MirrorSelectionPage(wxWizard *parent, MirrorList *mirrorlist);
    virtual void OnWizardPageChanging(wxWizardEvent& event);
	wxTreeCtrl *GetTreeCtrl() { return m_mirrortree; };

private:
	wxTreeCtrl *m_mirrortree;
	wxImageList *m_treeimages;
	MirrorList *m_mirrorlist;
	
	DECLARE_EVENT_TABLE()
};

#endif