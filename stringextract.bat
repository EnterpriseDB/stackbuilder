@echo off
REM ///////////////////////////////////////////////////////////////////////////
REM // Name:        stringextract.bat
REM // Purpose:     Extract strings for translation
REM // Author:      Dave Page
REM // Created:     2007-02-13
REM // RCS-ID:      $Id: stringextract.bat,v 1.1 2007/05/01 11:17:09 dpage Exp $
REM // Copyright:   (c) EnterpriseDB
REM // Licence:     BSD Licence
REM ///////////////////////////////////////////////////////////////////////////

copy StackBuilder-release.pot StackBuilder.pot

xgettext -k_ -k__ -j -s -o StackBuilder.pot *.cpp
xgettext -k_ -k__ -j -s -o StackBuilder.pot include/*.h

