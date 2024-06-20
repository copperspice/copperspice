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

#ifndef QGLOBAL_DEBUG_H
#define QGLOBAL_DEBUG_H

#if defined(CS_DISABLE_DEBUG)
# undef  QT_DEBUG
#else
# define QT_DEBUG
#endif

// ** (1) uncomment or pass any of the following defines in the CS build files

// #define CS_SHOW_DEBUG_CORE
// #define CS_SHOW_DEBUG_CORE_OSX
// #define CS_SHOW_DEBUG_CORE_IO
// #define CS_SHOW_DEBUG_CORE_PLUGIN
// #define CS_SHOW_DEBUG_CORE_SEMAPHORE
// #define CS_SHOW_DEBUG_CORE_STATEMACHINE

// #define CS_SHOW_DEBUG_GUI
// #define CS_SHOW_DEBUG_GUI_DPI
// #define CS_SHOW_DEBUG_GUI_GRAPHICSVIEW
// #define CS_SHOW_DEBUG_GUI_IMAGE
// #define CS_SHOW_DEBUG_GUI_OPENGL
// #define CS_SHOW_DEBUG_GUI_PAINTING
// #define CS_SHOW_DEBUG_GUI_STYLES
// #define CS_SHOW_DEBUG_GUI_TEXT
// #define CS_SHOW_DEBUG_GUI_WIDGETS

// #define CS_SHOW_DEBUG_MULTIMEDIA
// #define CS_SHOW_DEBUG_NETWORK
// #define CS_SHOW_DEBUG_OPENGL

// #define CS_SHOW_DEBUG_PLATFORM
// #define CS_SHOW_DEBUG_PLATFORM_MEDIA
// #define CS_SHOW_DEBUG_PLATFORM_PASTEBOARD
// #define CS_SHOW_DEBUG_PLATFORM_WINDOW

// #define CS_SHOW_DEBUG_SQL
// #define CS_SHOW_DEBUG_SVG
// #define CS_SHOW_DEBUG_XML


// ** (2) uncomment or pass the following define in the CS build files

// #define CS_DISABLE_ASSERT

#endif


