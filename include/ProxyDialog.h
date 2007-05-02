/////////////////////////////////////////////////////////////////////////////
// Name:        ProxyDialog.h
// Purpose:     Proxy server configuration dialog
// Author:      Dave Page
// Created:     2007-05-02
// RCS-ID:      $Id: ProxyDialog.h,v 1.1 2007/05/02 12:44:43 dpage Exp $
// Copyright:   (c) EnterpriseDB
// Licence:     BSD Licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _PROXYDIALOG_H
#define _PROXYDIALOG_H

#include "StackBuilder.h"

// wxWindows headers
#include <wx/wx.h>
#include <wx/button.h>
#include <wx/textctrl.h>

class ProxyDialog : public wxDialog
{
public:
    ProxyDialog(wxWindow *parent, const wxString& title);
    static wxString GetProxy(const wxString &protocol);
  
private:
    void OnOK(wxCommandEvent& event);
    void OnCancel(wxCommandEvent& event);

    wxButton *m_ok, *m_cancel;
    wxTextCtrl *m_http_host, *m_http_port, *m_ftp_host, *m_ftp_port;

    DECLARE_EVENT_TABLE()
};

#endif