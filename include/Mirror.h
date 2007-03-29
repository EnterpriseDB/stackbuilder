/////////////////////////////////////////////////////////////////////////////
// Name:        Mirror.h
// Purpose:     A mirror object
// Author:      Dave Page
// Created:     2007-02-13
// RCS-ID:      $Id: Mirror.h,v 1.4 2007/03/29 11:39:40 dpage Exp $
// Copyright:   (c) EnterpriseDB
// Licence:     BSD Licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _MIRROR_H
#define _MIRROR_H

#include "StackBuilder.h"

// wxWindows headers
#include <wx/wx.h>
#include <wx/treectrl.h>

class Mirror : public wxTreeItemData
{
public:
    Mirror() { port = 0; };

	bool IsValid();

	wxString country;
	wxString protocol;
	wxString hostname;
	wxString rootpath;
	long port;

	wxTreeItemId m_treeitem;
};

#endif