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

#ifndef PHONON_PLATFORMPLUGIN_H
#define PHONON_PLATFORMPLUGIN_H

#include <QtCore/QObject>
#include <QtCore/QStringList>
#include <QtCore/QPair>
#include "phonon_export.h"
#include "objectdescription.h"

QT_BEGIN_NAMESPACE

#ifndef QT_NO_PHONON_PLATFORMPLUGIN

class QUrl;
class QObject;
class QIcon;

namespace Phonon
{
class AbstractMediaStream;

class PlatformPlugin
{
    public:
        virtual ~PlatformPlugin() {}

        /**
         * Creates a AbstractMediaStream object that provides the data for the given \p url. On KDE
         * this uses KIO.
         */
        virtual AbstractMediaStream *createMediaStream(const QUrl &url, QObject *parent) = 0;

        /**
         * Returns the icon for the given icon name.
         */
        virtual QIcon icon(const QString &name) const = 0;

        /**
         * Shows a notification popup
         */
        virtual void notification(const char *notificationName, const QString &text,
                const QStringList &actions = QStringList(), QObject *receiver = 0,
                const char *actionSlot = 0) const = 0;

        /**
         * Returns the name of the application. For most Qt application this is
         * QCoreApplication::applicationName(), but for KDE this is overridden by KAboutData.
         */
        virtual QString applicationName() const = 0;

        /**
         * Creates a backend object. This way the platform can decide the backend preference.
         */
        virtual QObject *createBackend() = 0;

        /**
         * Using the library loader of the platform, loads a given backend.
         */
        virtual QObject *createBackend(const QString &library, const QString &version) = 0;

        /**
         * Tries to check whether the default backend supports a given MIME type without loading the
         * actual backend library. On KDE this reads the MIME type list from the .desktop file of
         * the backend.
         */
        virtual bool isMimeTypeAvailable(const QString &mimeType) const = 0;

        /**
         * Saves the volume for the given output.
         */
        virtual void saveVolume(const QString &outputName, qreal volume) = 0;

        /**
         * Loads the volume for the given output.
         */
        virtual qreal loadVolume(const QString &outputName) const = 0;

        virtual QList<int> objectDescriptionIndexes(ObjectDescriptionType type) const = 0;
        virtual QHash<QByteArray, QVariant> objectDescriptionProperties(ObjectDescriptionType type, int index) const = 0;

        /**
         * Returns a list of (driver, handle) pairs for the given AudioOutputDevice description.
         */
        virtual QList<QPair<QByteArray, QString> > deviceAccessListFor(const Phonon::AudioOutputDevice &) const { return QList<QPair<QByteArray, QString> >(); }
};
} // namespace Phonon

CS_DECLARE_INTERFACE(Phonon::PlatformPlugin, "3PlatformPlugin.phonon.kde.org")

#endif //QT_NO_PHONON_PLATFORMPLUGIN

QT_END_NAMESPACE

#endif // PHONON_PLATFORMPLUGIN_H
