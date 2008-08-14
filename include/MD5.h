/////////////////////////////////////////////////////////////////////////////
// Name:        MD5.h
// Purpose:     MD5 functions
// Author:      Colin Plumb, adapted/stolen by Dave Page
// Created:     2007-02-13
// RCS-ID:      $Id: MD5.h,v 1.2 2008/08/14 15:54:08 dpage Exp $
// Copyright:   (c) EnterpriseDB
// Licence:     BSD Licence
//
// Notes:       This code implements the MD5 message-digest algorithm.
//              The algorithm is due to Ron Rivest.    This code was
//              written by Colin Plumb in 1993, no copyright is claimed.
/////////////////////////////////////////////////////////////////////////////

#ifndef _MD5_H
#define _MD5_H

#include "StackBuilder.h"

// wxWindows headers
#include <wx/wx.h>

typedef unsigned int uint32;

struct MD5Context {
        uint32 buf[4];
        uint32 bits[2];
        unsigned char in[64];
};

void MD5Init(MD5Context *ctx);
void MD5Update(MD5Context *ctx, unsigned char *buf, unsigned len);
void MD5Final(unsigned char digest[16], struct MD5Context *ctx);
void MD5Transform(uint32 buf[4], uint32 in[16]);
wxString md5sum(const wxString& filename);

/*
 * This is needed to make RSAREF happy on some MS-DOS compilers.
 */
typedef struct MD5Context MD5_CTX;

#endif

