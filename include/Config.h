/////////////////////////////////////////////////////////////////////////////
// Name:        Config.h
// Purpose:     Configurable macros
// Author:      Dave Page
// Created:     2007-02-13
// RCS-ID:      $Id: Config.h,v 1.9 2011/11/28 19:09:57 dpage Exp $
// Copyright:   (c) EnterpriseDB
// Licence:     BSD Licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _CONFIG_H
#define _CONFIG_H

// The version number
#define VERSION_NUM 3,0,0
#define VERSION_STR "3.0.0"

// Download locations
#define DEFAULT_MIRROR_LIST_URL wxT("http://www.postgresql.org/mirrors.xml")
#define DEFAULT_APPLICATION_LIST_URL wxT("http://www.postgresql.org/applications-v2.xml")

// The registry file
#ifndef __WXMSW__
#define REGISTRY_FILE wxT("/etc/postgres-reg.ini")
#endif

#endif

