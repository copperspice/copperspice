/***********************************************************************
*
* Copyright (c) 2012-2018 Barbara Geller
* Copyright (c) 2012-2018 Ansel Sermersheim
* Copyright (c) 2012-2016 Digia Plc and/or its subsidiary(-ies).
* Copyright (c) 2008-2012 Nokia Corporation and/or its subsidiary(-ies).
* All rights reserved.
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
* <http://www.gnu.org/licenses/>.
*
***********************************************************************/

#ifndef QVFBHDR_H
#define QVFBHDR_H

#include <QtGui/qcolor.h>
#include <QtGui/qwindowdefs.h>
#include <QtCore/qrect.h>

QT_BEGIN_NAMESPACE

#ifndef QT_QWS_TEMP_DIR
#  define QT_QWS_TEMP_DIR QLatin1String("/tmp")
#endif

#ifdef QT_PRIVATE_QWS
#define QT_VFB_DATADIR(DISPLAY)       QString::fromLatin1("%1/qtembedded-%2-%3") \
                                      .arg(QT_QWS_TEMP_DIR).arg(getuid()).arg(DISPLAY)
#define QT_VFB_MOUSE_PIPE(DISPLAY)    QT_VFB_DATADIR(DISPLAY) \
                                      .append(QLatin1String("/qtvfb_mouse"))
#define QT_VFB_KEYBOARD_PIPE(DISPLAY) QT_VFB_DATADIR(DISPLAY) \
                                      .append(QLatin1String("/qtvfb_keyboard"))
#define QT_VFB_MAP(DISPLAY)           QT_VFB_DATADIR(DISPLAY) \
                                      .append(QLatin1String("/qtvfb_map"))
#define QT_VFB_SOUND_PIPE(DISPLAY)    QT_VFB_DATADIR(DISPLAY) \
                                      .append(QLatin1String("/qt_soundserver"))
#define QTE_PIPE(DISPLAY)             QT_VFB_DATADIR(DISPLAY) \
                                      .append(QLatin1String("/QtEmbedded"))
#define QTE_PIPE_QVFB(DISPLAY)        QTE_PIPE(DISPLAY)
#else
#define QT_VFB_DATADIR(DISPLAY)       QString::fromLatin1("%1/qtembedded-%2") \
                                      .arg(QT_QWS_TEMP_DIR).arg(DISPLAY)
#define QT_VFB_MOUSE_PIPE(DISPLAY)    QString::fromLatin1("%1/.qtvfb_mouse-%2") \
                                      .arg(QT_QWS_TEMP_DIR).arg(DISPLAY)
#define QT_VFB_KEYBOARD_PIPE(DISPLAY) QString::fromLatin1("%1/.qtvfb_keyboard-%2") \
                                      .arg(QT_QWS_TEMP_DIR).arg(DISPLAY)
#define QT_VFB_MAP(DISPLAY)           QString::fromLatin1("%1/.qtvfb_map-%2") \
                                      .arg(QT_QWS_TEMP_DIR).arg(DISPLAY)
#define QT_VFB_SOUND_PIPE(DISPLAY)    QString::fromLatin1("%1/.qt_soundserver-%2") \
                                      .arg(QT_QWS_TEMP_DIR).arg(DISPLAY)
#define QTE_PIPE(DISPLAY)             QT_VFB_DATADIR(DISPLAY) \
                                      .append(QLatin1String("/QtEmbedded-%1")).arg(DISPLAY)
#define QTE_PIPE_QVFB(DISPLAY)        QTE_PIPE(DISPLAY)
#endif

struct QVFbHeader {
   int width;
   int height;
   int depth;
   int linestep;
   int dataoffset;
   QRect update;
   bool dirty;
   int  numcols;
   QRgb clut[256];
   int viewerVersion;
   int serverVersion;
   int brightness; // since 4.4.0
   WId windowId; // since 4.5.0
};

struct QVFbKeyData {
   unsigned int keycode;
   Qt::KeyboardModifiers modifiers;
   unsigned short int unicode;
   bool press;
   bool repeat;
};

QT_END_NAMESPACE

#endif // QVFBHDR_H
