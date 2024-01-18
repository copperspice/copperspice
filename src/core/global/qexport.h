/***********************************************************************
*
* Copyright (c) 2012-2024 Barbara Geller
* Copyright (c) 2012-2024 Ansel Sermersheim
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

#ifndef QEXPORT_H
#define QEXPORT_H

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__)
#  define Q_IS_WIN
#endif


#if defined(QT_SHARED) && defined(QT_STATIC)
#  error "Both QT_SHARED and QT_STATIC were defined, please choose only one"
#endif

#if ! defined(QT_SHARED) && ! defined(QT_STATIC)
#  define QT_SHARED
#endif

#ifdef QT_STATIC
#  error "Currently unsupported"
#endif


#ifndef Q_DECL_EXPORT
#  if defined(Q_IS_WIN)
#    define Q_DECL_EXPORT    __declspec(dllexport)

#  elif defined(QT_VISIBILITY_AVAILABLE)
#    define Q_DECL_EXPORT    __attribute__((visibility("default")))
#    define Q_DECL_HIDDEN    __attribute__((visibility("hidden")))
#  endif

#  ifndef Q_DECL_EXPORT
#    define Q_DECL_EXPORT
#  endif

#endif

#ifdef _MSC_VER
#define Q_EXPORT_MAYBE

#else
#define Q_EXPORT_MAYBE   Q_DECL_EXPORT

#endif

#ifndef Q_DECL_IMPORT
#  if defined(Q_IS_WIN)
#    define Q_DECL_IMPORT    __declspec(dllimport)
#  else
#    define Q_DECL_IMPORT
#  endif
#endif

#ifndef Q_DECL_HIDDEN
#  define Q_DECL_HIDDEN
#endif

#if defined(Q_IS_WIN) && ! defined(QT_STATIC)        // create a DLL library

#    if defined(QT_BUILD_CORE_LIB)
#      define Q_CORE_EXPORT          Q_DECL_EXPORT
#    else
#      define Q_CORE_EXPORT          Q_DECL_IMPORT
#    endif

#    if defined(QT_BUILD_DBUS_LIB)
#      define Q_DBUS_EXPORT          Q_DECL_EXPORT
#    else
#      define Q_DBUS_EXPORT          Q_DECL_IMPORT
#    endif

#    if defined(QT_BUILD_GUI_LIB)
#      define Q_GUI_EXPORT           Q_DECL_EXPORT
#    else
#      define Q_GUI_EXPORT           Q_DECL_IMPORT
#    endif

#    if defined(QT_BUILD_MULTIMEDIA_LIB)
#      define Q_MULTIMEDIA_EXPORT    Q_DECL_EXPORT
#    else
#      define Q_MULTIMEDIA_EXPORT    Q_DECL_IMPORT
#    endif

#    if defined(QT_BUILD_NETWORK_LIB)
#      define Q_NETWORK_EXPORT       Q_DECL_EXPORT
#    else
#      define Q_NETWORK_EXPORT       Q_DECL_IMPORT
#    endif

#    if defined(QT_BUILD_OPENGL_LIB)
#      define Q_OPENGL_EXPORT        Q_DECL_EXPORT
#    else
#      define Q_OPENGL_EXPORT        Q_DECL_IMPORT
#    endif

#    if defined(QT_BUILD_SQL_LIB)
#      define Q_SQL_EXPORT           Q_DECL_EXPORT
#    else
#      define Q_SQL_EXPORT           Q_DECL_IMPORT
#    endif

#    if defined(QT_BUILD_SVG_LIB)
#      define Q_SVG_EXPORT           Q_DECL_EXPORT
#    else
#      define Q_SVG_EXPORT           Q_DECL_IMPORT
#    endif

#    if defined(QT_BUILD_SCRIPT_LIB)
#      define Q_SCRIPT_EXPORT        Q_DECL_EXPORT
#    else
#      define Q_SCRIPT_EXPORT        Q_DECL_IMPORT
#    endif

#    if defined(QT_BUILD_SCRIPTTOOLS_LIB)
#      define Q_SCRIPTTOOLS_EXPORT   Q_DECL_EXPORT
#    else
#      define Q_SCRIPTTOOLS_EXPORT   Q_DECL_IMPORT
#    endif

#    if defined(QT_BUILD_VULKAN_LIB)
#      define Q_VULKAN_EXPORT        Q_DECL_EXPORT
#    else
#      define Q_VULKAN_EXPORT        Q_DECL_IMPORT
#    endif

#    if defined(QT_BUILD_XML_LIB)
#      define Q_XML_EXPORT           Q_DECL_EXPORT
#    else
#      define Q_XML_EXPORT           Q_DECL_IMPORT
#    endif

#    if defined(QT_BUILD_XMLPATTERNS_LIB)
#      define Q_XMLPATTERNS_EXPORT   Q_DECL_EXPORT
#    else
#      define Q_XMLPATTERNS_EXPORT   Q_DECL_IMPORT
#    endif

#    if defined(QT_BUILD_DECLARATIVE_LIB)
#      define Q_DECLARATIVE_EXPORT   Q_DECL_EXPORT
#    else
#      define Q_DECLARATIVE_EXPORT   Q_DECL_IMPORT
#    endif

#    if defined(QT_BUILD_DBUS_LIB)
#      define Q_DBUS_EXPORT          Q_DECL_EXPORT
#    else
#      define Q_DBUS_EXPORT          Q_DECL_IMPORT
#    endif

#endif

#if ! defined(Q_CORE_EXPORT)

#  if ! defined(QT_STATIC)
#    define Q_CORE_EXPORT           Q_DECL_EXPORT
#    define Q_DBUS_EXPORT           Q_DECL_EXPORT
#    define Q_GUI_EXPORT            Q_DECL_EXPORT
#    define Q_SQL_EXPORT            Q_DECL_EXPORT
#    define Q_NETWORK_EXPORT        Q_DECL_EXPORT
#    define Q_SVG_EXPORT            Q_DECL_EXPORT
#    define Q_DECLARATIVE_EXPORT    Q_DECL_EXPORT
#    define Q_OPENGL_EXPORT         Q_DECL_EXPORT
#    define Q_MULTIMEDIA_EXPORT     Q_DECL_EXPORT
#    define Q_VULKAN_EXPORT         Q_DECL_EXPORT
#    define Q_XML_EXPORT            Q_DECL_EXPORT
#    define Q_XMLPATTERNS_EXPORT    Q_DECL_EXPORT
#    define Q_SCRIPT_EXPORT         Q_DECL_EXPORT
#    define Q_SCRIPTTOOLS_EXPORT    Q_DECL_EXPORT
#  else
#    define Q_CORE_EXPORT
#    define Q_DBUS_EXPORT
#    define Q_GUI_EXPORT
#    define Q_SQL_EXPORT
#    define Q_NETWORK_EXPORT
#    define Q_SVG_EXPORT
#    define Q_DECLARATIVE_EXPORT
#    define Q_OPENGL_EXPORT
#    define Q_MULTIMEDIA_EXPORT
#    define Q_VULKAN_EXPORT
#    define Q_XML_EXPORT
#    define Q_XMLPATTERNS_EXPORT
#    define Q_SCRIPT_EXPORT
#    define Q_SCRIPTTOOLS_EXPORT
#  endif

#endif

#ifdef Q_IS_WIN
#  undef Q_IS_WIN
#endif

#if defined(__cplusplus)
   Q_CORE_EXPORT bool qSharedBuild();
#endif

#endif
