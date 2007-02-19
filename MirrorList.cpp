/////////////////////////////////////////////////////////////////////////////
// Name:        MirrorList.cpp
// Purpose:     Maintains the list of mirrors
// Author:      Dave Page
// Created:     2007-02-13
// RCS-ID:      $Id: MirrorList.cpp,v 1.1 2007/02/19 09:57:00 dpage Exp $
// Copyright:   (c) EnterpriseDB
// Licence:     BSD Licence
/////////////////////////////////////////////////////////////////////////////

#include "StackBuilder.h"

// wxWindows headers
#include <wx/wx.h>
#include <wx/arrimpl.cpp>
#include <wx/treectrl.h>
#include <wx/url.h>
#include <wx/xml/xml.h>

// Application headers
#include "Mirror.h"
#include "MirrorList.h"

// Define the MirrorArray type
WX_DEFINE_OBJARRAY(MirrorArray);

bool MirrorList::LoadMirrorList()
{
    wxURL url(m_mirrorListUrl);
	wxURLError err = url.GetError();
    if (err != wxURL_NOERR)
	{
		wxString msg;
		switch (err)
		{
			case wxURL_SNTXERR:
				msg = _("Could not parse the URL.");
				break;
			case wxURL_NOPROTO:
				msg = _("Unsupported protocol specified.");
				break;
			case wxURL_NOHOST:
				msg = _("No hostname specified in URL.");
				break;
			case wxURL_NOPATH:
				msg = _("No path specified in URL.");
				break;
			case wxURL_CONNERR:
				msg = _("A connection error occurred.");
				break;
			case wxURL_PROTOERR:
				msg = _("A protocol error occurred.");
				break;
		}
		wxLogError(wxString::Format(_("Failed to open the mirror list: %s\n\nError: %s"), m_mirrorListUrl, msg));
        return false;
	}

    wxInputStream *ip = url.GetInputStream();

	if (!ip || !ip->IsOk())
	{
		wxLogError(wxString::Format(_("Failed to open the mirror list: %s\n\nError: The URL specified could not be opened."), m_mirrorListUrl));
		return false;
	}

	wxXmlDocument xml;
	if (!xml.Load(*ip))
	{
		wxLogError(wxString::Format(_("Failed to parse the mirror list: %s"), m_mirrorListUrl));
		return false;
	}

	// Clear the applist
	m_mirrors.Clear();

	// Iterate through the applications and build the list
	wxXmlNode *mirror, *properties;
	mirror = xml.GetRoot()->GetChildren();

	while (mirror) 
	{
		if (mirror->GetName() == wxT("mirror")) 
		{
			Mirror *newMirror = new Mirror();
			properties = mirror->GetChildren();

			while (properties)
			{
				if (properties->GetName() == wxT("country"))
					newMirror->country = properties->GetNodeContent();
				else if (properties->GetName() == wxT("protocol"))
					newMirror->protocol = properties->GetNodeContent();
				else if (properties->GetName() == wxT("hostname"))
					newMirror->hostname = properties->GetNodeContent();
				else if (properties->GetName() == wxT("path"))
					newMirror->path = properties->GetNodeContent();
				else if (properties->GetName() == wxT("port"))
					properties->GetNodeContent().ToLong(&newMirror->port);

				properties = properties->GetNext();
			}
			if (newMirror->IsValid())
				m_mirrors.Add(newMirror);
		}
		mirror = mirror->GetNext();
	}

	return true;
}

bool MirrorList::PopulateTreeCtrl(wxTreeCtrl *tree)
{
	wxTreeItemIdValue cookie;
	wxTreeItemId node, root, country, mirror;
	bool found;

	tree->DeleteAllItems();
	root = tree->AddRoot(_("Countries"), 0);

	for (unsigned int i=0; i<m_mirrors.GetCount(); i++)
	{
		found = false;

		node = tree->GetFirstChild(root, cookie);

		while (node)
		{
			if (tree->GetItemText(node) == m_mirrors[i].country)
			{
				country = node;
				found = true;
				break;
			}
			node = tree->GetNextChild(root, cookie);
		}

		if (!found)
			country = tree->AppendItem(root, m_mirrors[i].country, 0);

	    mirror = tree->AppendItem(country, m_mirrors[i].hostname, 0, -1, &m_mirrors[i]);
		tree->SortChildren(country);
	}

	tree->SortChildren(root);
	tree->Expand(root);

	return true;
}