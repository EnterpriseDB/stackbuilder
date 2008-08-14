/////////////////////////////////////////////////////////////////////////////
// Name:        Config.h
// Purpose:     Configurable macros
// Author:      Dave Page
// Created:     2007-02-13
// RCS-ID:      $Id: Config.h,v 1.5 2008/08/14 10:36:28 dpage Exp $
// Copyright:   (c) EnterpriseDB
// Licence:     BSD Licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _CONFIG_H
#define _CONFIG_H

// The version number
#define VERSION_NUM 2,0,0
#define VERSION_STR "2.0.0"

// Download locations
#define DEFAULT_MIRROR_LIST_URL wxT("http://www.postgresql.org/mirrors.xml")
#define DEFAULT_APPLICATION_LIST_URL wxT("http://www.postgresql.org/applications-v2.xml")
#define DEFAULT_DOWNLOAD_COUNTER_URL wxT("http://wwwmaster.postgresql.org/redir")

// The registry file
#ifndef __WXMSW__
#define REGISTRY_FILE wxT("/etc/postgres-reg.ini")
#endif

#endif
