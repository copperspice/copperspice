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

#include "eglnullwsscreen.h"
#include "eglnullwswindowsurface.h"
#include "eglnullwsscreenplugin.h"

#include <QHash>
#include <QDebug>

namespace
{
    class EGLNullWSScreenSurfaceFunctions : public QGLScreenSurfaceFunctions
    {
    public:
        virtual bool createNativeWindow(QWidget *, EGLNativeWindowType *native)
            { *native = 0; return true; }
    };
}

EGLNullWSScreen::EGLNullWSScreen(int displayId) : QGLScreen(displayId) {}

EGLNullWSScreen::~EGLNullWSScreen() {}

bool EGLNullWSScreen::initDevice()
{
    setSurfaceFunctions(new EGLNullWSScreenSurfaceFunctions);
    return true;
}

static const QHash<QString, QImage::Format> formatDictionary()
{
    QHash<QString, QImage::Format> dictionary;
    dictionary["rgb32"]     = QImage::Format_RGB32;
    dictionary["argb32"]    = QImage::Format_ARGB32;
    dictionary["rgb16"]     = QImage::Format_RGB16;
    dictionary["rgb666"]    = QImage::Format_RGB666;
    dictionary["rgb555"]    = QImage::Format_RGB555;
    dictionary["rgb888"]    = QImage::Format_RGB888;
    dictionary["rgb444"]    = QImage::Format_RGB444;
    return dictionary;
}

static int depthForFormat(QImage::Format format)
{
    switch (format) {
    case QImage::Format_RGB32:  return 32;
    case QImage::Format_ARGB32: return 32;
    case QImage::Format_RGB16:  return 16;
    case QImage::Format_RGB666: return 24;
    case QImage::Format_RGB555: return 16;
    case QImage::Format_RGB888: return 24;
    case QImage::Format_RGB444: return 16;
    default:
        Q_ASSERT_X(false, "EGLNullWSScreen", "Unknown format");
        return -1;
    }
}

static void printHelp(const QHash<QString, QImage::Format> &formatDictionary)
{
    QByteArray formatsBuf;
    QTextStream(&formatsBuf) << QStringList(formatDictionary.keys()).join(", ");
    qWarning(
        "%s: Valid options are:\n"
        "size=WIDTHxHEIGHT   Screen size reported by this driver\n"
        "format=FORMAT       Screen format, where FORMAT is one of the following:\n"
        "                      %s\n",
        PluginName,
        formatsBuf.constData());
}

bool EGLNullWSScreen::connect(const QString &displaySpec)
{
    const QStringList args = displaySpec.section(':', 1).split(':', QString::SkipEmptyParts);
    const QHash<QString, QImage::Format> formatDict = formatDictionary();
    Q_FOREACH(const QString arg, args) {
        const QString optionName = arg.section('=', 0, 0);
        const QString optionArg = arg.section('=', 1);
        if (optionName == QLatin1String("size")) {
            w = optionArg.section('x', 0, 0).toInt();
            h = optionArg.section('x', 1, 1).toInt();
        } else if (optionName == QLatin1String("format")) {
            if (formatDict.contains(optionArg))
                setPixelFormat(formatDict.value(optionArg));
            else
                printHelp(formatDict);
        } else {
            printHelp(formatDict);
        }
    }

    if (w == 0 || h == 0) {
        w = 640;
        h = 480;
        qWarning("%s: Using default screen size %dx%d", PluginName, w, h);
    }
    dw = w;
    dh = h;

    if (pixelFormat() == QImage::Format_Invalid) {
        qWarning("%s: Using default screen format argb32", PluginName);
        setPixelFormat(QImage::Format_ARGB32);
    }
    d = depthForFormat(pixelFormat());

    static const int Dpi = 120;
    static const qreal ScalingFactor = static_cast<qreal>(25.4) / Dpi;
    physWidth = qRound(dw * ScalingFactor);
    physHeight = qRound(dh * ScalingFactor);

    return true;
}

void EGLNullWSScreen::disconnect() {}

void EGLNullWSScreen::shutdownDevice() {}

void EGLNullWSScreen::setMode(int /*width*/, int /*height*/, int /*depth*/) {}

void EGLNullWSScreen::blank(bool /*on*/) {}

void EGLNullWSScreen::exposeRegion(QRegion /*r*/, int /*changing*/) {}

QWSWindowSurface* EGLNullWSScreen::createSurface(QWidget *widget) const
{
    if (qobject_cast<QGLWidget*>(widget)) {
        return new EGLNullWSWindowSurface(widget);
    } else {
        qWarning("%s: Creating non-GL surface", PluginName);
        return QScreen::createSurface(widget);
    }
}

QWSWindowSurface* EGLNullWSScreen::createSurface(const QString &key) const
{
    if (key == QLatin1String("eglnullws")) {
        return new EGLNullWSWindowSurface;
    } else {
        qWarning("%s: Creating non-GL surface", PluginName);
        return QScreen::createSurface(key);
    }
}
