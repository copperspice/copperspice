/***********************************************************************
*
* Copyright (c) 2012-2020 Barbara Geller
* Copyright (c) 2012-2020 Ansel Sermersheim
*
* Copyright (c) 2015 The Qt Company Ltd.
* Copyright (c) 2012-2016 Digia Plc and/or its subsidiary(-ies).
* Copyright (c) 2008-2012 Nokia Corporation and/or its subsidiary(-ies).
*
* This file is part of CopperSpice.
*
* CopperSpice is free software. You can redistribute it and/or
* modify it under the terms of the GNU Lesser General Public License
* version 2.1 as published by the Free Software Foundation.
*
* CopperSpice is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
*
* https://www.gnu.org/licenses/
*
***********************************************************************/

#ifndef QTWINDOWS_ADDITIONAL_H
#define QTWINDOWS_ADDITIONAL_H

#include <qglobal.h>
#include <qt_windows.h>

#ifndef WM_THEMECHANGED
#    define WM_THEMECHANGED 0x031A
#endif

#ifndef WM_DWMCOMPOSITIONCHANGED
#    define WM_DWMCOMPOSITIONCHANGED 0x31E
#endif

#ifndef GWL_HWNDPARENT
#    define GWL_HWNDPARENT (-8)
#endif

/* Complement the definitions and declarations missing
 * when using MinGW or older Windows SDKs. */

#if defined(Q_CC_MINGW)
#    if !defined(ULW_ALPHA)
#        define ULW_ALPHA 0x00000002
#        define LWA_ALPHA 0x00000002
#    endif // !defined(ULW_ALPHA)
#    define SPI_GETFONTSMOOTHINGTYPE 0x200A
#    define FE_FONTSMOOTHINGCLEARTYPE 0x0002
#    define CLEARTYPE_QUALITY       5
#    define SPI_GETDROPSHADOW 0x1024
#    define COLOR_MENUHILIGHT 29
#    define COLOR_MENUBAR     30
#    define CF_DIBV5 17

#if !defined(CO_E_NOT_SUPPORTED)
#define CO_E_NOT_SUPPORTED               _HRESULT_TYPEDEF_(0x80004021L)
#endif

#define IFMETHOD HRESULT STDMETHODCALLTYPE
#define IFACEMETHODIMP STDMETHODIMP
#define IFACEMETHODIMP_(type) STDMETHODIMP_(type)

// For accessibility:
#ifdef __cplusplus
#define EXTERN_C extern "C"
#else
#define EXTERN_C extern
#endif

#define CHILDID_SELF 0
#define WM_GETOBJECT 0x003D

#ifndef SHGFI_ADDOVERLAYS // Shell structures for icons.
typedef struct _SHSTOCKICONINFO {
   DWORD cbSize;
   HICON hIcon;
   int   iSysImageIndex;
   int   iIcon;
   WCHAR szPath[MAX_PATH];
} SHSTOCKICONINFO;

#  define SIID_SHIELD 77
#  define SHGFI_ADDOVERLAYS 0x20
#  define SHGFI_OVERLAYINDEX 0x40
#endif // SIID_SHIELD

#if !defined(__MINGW64_VERSION_MAJOR)

#define STATE_SYSTEM_HASPOPUP 0x40000000
#define STATE_SYSTEM_PROTECTED 0x20000000

typedef struct tagUPDATELAYEREDWINDOWINFO {
   DWORD               cbSize;
   HDC                 hdcDst;
   const POINT         *pptDst;
   const SIZE          *psize;
   HDC                 hdcSrc;
   const POINT         *pptSrc;
   COLORREF            crKey;
   const BLENDFUNCTION *pblend;
   DWORD               dwFlags;
   const RECT          *prcDirty;
} UPDATELAYEREDWINDOWINFO, *PUPDATELAYEREDWINDOWINFO;

#endif // if !defined(__MINGW64_VERSION_MAJOR)

// OpenGL Pixelformat flags.
#define PFD_SUPPORT_DIRECTDRAW      0x00002000
#define PFD_DIRECT3D_ACCELERATED    0x00004000
#define PFD_SUPPORT_COMPOSITION     0x00008000

// IME.
#define IMR_CONFIRMRECONVERTSTRING      0x0005

#ifndef MAPVK_VK_TO_CHAR
#  define MAPVK_VK_TO_CHAR 2
#endif

#endif // if defined(Q_CC_MINGW)

/* Touch is supported from Windows 7 onwards and data structures
 * are present in the Windows SDK's, but not in older MSVC Express
 * versions. */

#if defined(Q_CC_MINGW) || !defined(TOUCHEVENTF_MOVE)

#define WM_TOUCH 0x0240

typedef struct tagTOUCHINPUT {
   LONG x;
   LONG y;
   HANDLE hSource;
   DWORD dwID;
   DWORD dwFlags;
   DWORD dwMask;
   DWORD dwTime;
   ULONG_PTR dwExtraInfo;
   DWORD cxContact;
   DWORD cyContact;
} TOUCHINPUT, *PTOUCHINPUT;
typedef TOUCHINPUT const *PCTOUCHINPUT;

#    define TOUCHEVENTF_MOVE 0x0001
#    define TOUCHEVENTF_DOWN 0x0002
#    define TOUCHEVENTF_UP 0x0004
#    define TOUCHEVENTF_INRANGE 0x0008
#    define TOUCHEVENTF_PRIMARY 0x0010
#    define TOUCHEVENTF_NOCOALESCE 0x0020
#    define TOUCHEVENTF_PALM 0x0080
#    define TOUCHINPUTMASKF_CONTACTAREA 0x0004
#    define TOUCHINPUTMASKF_EXTRAINFO 0x0002

#endif // if defined(Q_CC_MINGW) || !defined(TOUCHEVENTF_MOVE)

#ifndef WM_GESTURE
#  define WM_GESTURE 0x0119
#endif

#endif
