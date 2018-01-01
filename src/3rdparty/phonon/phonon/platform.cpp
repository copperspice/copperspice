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

/********************************************************
**  Copyright (C) 2005-2006 Matthias Kretz <kretz@kde.org
********************************************************/

#include "platform_p.h"
#include "platformplugin.h"
#include "factory_p.h"
#include <QtCore/QCoreApplication>
#include <QtCore/QUrl>
#include <QtGui/QIcon>
#include <QtGui/QStyle>
#include <QtGui/QApplication>

QT_BEGIN_NAMESPACE

namespace Phonon
{

void Platform::saveVolume(const QString &outputName, qreal volume)
{
#ifndef QT_NO_PHONON_PLATFORMPLUGIN
    PlatformPlugin *f = Factory::platformPlugin();
    if (f) {
        f->saveVolume(outputName, volume);
    }
#else
    Q_UNUSED(outputName);
    Q_UNUSED(volume);
#endif //QT_NO_PHONON_PLATFORMPLUGIN
}

qreal Platform::loadVolume(const QString &outputName)
{
#ifndef QT_NO_PHONON_PLATFORMPLUGIN
    const PlatformPlugin *f = Factory::platformPlugin();
    if (f) {
        return f->loadVolume(outputName);
    }
#else
    Q_UNUSED(outputName);
#endif //QT_NO_PHONON_PLATFORMPLUGIN
    return 1.0;
}

AbstractMediaStream *Platform::createMediaStream(const QUrl &url, QObject *parent)
{
#ifndef QT_NO_PHONON_PLATFORMPLUGIN
    PlatformPlugin *f = Factory::platformPlugin();
    if (f) {
        return f->createMediaStream(url, parent);
    }
#else
    Q_UNUSED(url);
    Q_UNUSED(parent);
#endif //QT_NO_PHONON_PLATFORMPLUGIN
    return 0;
}

QIcon Platform::icon(const QString &name, QStyle *style)
{
    QIcon ret;
#ifndef QT_NO_PHONON_PLATFORMPLUGIN
    if (const PlatformPlugin *f = Factory::platformPlugin()) {
        ret = f->icon(name);
    }
#endif //QT_NO_PHONON_PLATFORMPLUGIN
    if (ret.isNull()) {
        if (!style) {
            style = QApplication::style();
        }
        if (name == QLatin1String("player-volume")) {
            ret = style->standardPixmap(QStyle::SP_MediaVolume);
        } else if (name == QLatin1String("player-volume-muted")) {
            ret = style->standardPixmap(QStyle::SP_MediaVolumeMuted);
        }
    }

    return ret;
}

void Platform::notification(const char *notificationName, const QString &text,
        const QStringList &actions, QObject *receiver,
        const char *actionSlot)
{
#ifndef QT_NO_PHONON_PLATFORMPLUGIN
    const PlatformPlugin *f = Factory::platformPlugin();
    if (f) {
        f->notification(notificationName, text, actions, receiver, actionSlot);
    }
#else
    Q_UNUSED(notificationName);
    Q_UNUSED(text);
    Q_UNUSED(actions);
    Q_UNUSED(receiver);
    Q_UNUSED(actionSlot);
#endif //QT_NO_PHONON_PLATFORMPLUGIN
}

QString Platform::applicationName()
{
#ifndef QT_NO_PHONON_PLATFORMPLUGIN
    const PlatformPlugin *f = Factory::platformPlugin();
    if (f) {
        return f->applicationName();
    }
#endif //QT_NO_PHONON_PLATFORMPLUGIN
    QString ret = QCoreApplication::applicationName();
    if (ret.isEmpty())
        ret = QCoreApplication::applicationFilePath();
    return ret;
}

QList<QPair<QByteArray, QString> > Platform::deviceAccessListFor(const Phonon::AudioOutputDevice &deviceDesc)
{
#ifndef QT_NO_PHONON_PLATFORMPLUGIN
    const PlatformPlugin *f = Factory::platformPlugin();
    if (f) {
        return f->deviceAccessListFor(deviceDesc);
    }
#endif //QT_NO_PHONON_PLATFORMPLUGIN
    return QList<QPair<QByteArray, QString> >();
}

} // namespace Phonon

QT_END_NAMESPACE
