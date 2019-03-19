/*
 * Copyright (C) 2006, 2007, 2008 Apple Inc. All rights reserved.
 * Copyright (C) 2006 Samuel Weinig <sam.weinig@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 *
 */

#if defined(HAVE_CONFIG_H) && HAVE_CONFIG_H
#include "autotoolsconfig.h"
#endif

#include <wtf/Platform.h>

#if ! defined(QT_BUILD_SCRIPT_LIB) && OS(WINDOWS) && COMPILER(MSVC)

#if defined(BUILDING_JavaScriptCore) || defined(BUILDING_WTF)
#define JS_EXPORTDATA
#define JS_EXPORTCLASS   __declspec(dllexport)
#else
#define JS_EXPORTDATA
#define JS_EXPORTCLASS   __declspec(dllimport)
#endif

#else
#define JS_EXPORTDATA
#define JS_EXPORTCLASS

#endif

#if OS(WINDOWS)

// We need to define this before the first #include of stdlib.h or it won't contain rand_s.
#ifndef _CRT_RAND_S
#define _CRT_RAND_S
#endif

#endif

#if OS(FREEBSD) || OS(OPENBSD)
#define HAVE_PTHREAD_NP_H 1
#endif

/* FIXME: if all platforms have these, do they really need #defines? */
#define HAVE_STDINT_H 1

#define WTF_CHANGES 1

#ifdef __cplusplus
#undef new
#undef delete
#include <wtf/FastMalloc.h>
#endif

// this breaks compilation of <QFontDatabase>, so turn it off for now
// generates errors on wx on Windows, because these functions are used from wx headers.
#if !PLATFORM(QT) && !PLATFORM(WX)
#include <wtf/DisallowCType.h>
#endif

#if PLATFORM(CHROMIUM)
#if !defined(WTF_USE_V8)
#define WTF_USE_V8 1
#endif
#endif /* PLATFORM(CHROMIUM) */

#if !defined(WTF_USE_V8)
#define WTF_USE_V8 0
#endif /* !defined(WTF_USE_V8) */

/* Using V8 implies not using JSC and vice versa */
#define WTF_USE_JSC !WTF_USE_V8
