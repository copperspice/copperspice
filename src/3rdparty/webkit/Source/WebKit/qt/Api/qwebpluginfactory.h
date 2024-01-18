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

#ifndef QWEBPLUGINFACTORY_H
#define QWEBPLUGINFACTORY_H

#include "qwebkitglobal.h"

#include <qobject.h>
#include <qstringlist.h>
#include <qstringfwd.h>

class QUrl;
class QWebPluginFactoryPrivate;

class QWEBKIT_EXPORT QWebPluginFactory : public QObject {
    WEB_CS_OBJECT(QWebPluginFactory)

public:
    struct QWEBKIT_EXPORT MimeType {
        QString name;
        QString description;
        QStringList fileExtensions;
        bool operator==(const MimeType& other) const;
        inline bool operator!=(const MimeType& other) const { return !operator==(other); }
    };

    struct Plugin {
        QString name;
        QString description;
        QList<MimeType> mimeTypes;
    };

    explicit QWebPluginFactory(QObject* parent = 0);
    virtual ~QWebPluginFactory();

    virtual QList<Plugin> plugins() const = 0;
    virtual void refreshPlugins();

    virtual QObject *create(const QString &mimeType, const QUrl &url, const QStringList& argumentNames,
          const QStringList &argumentValues) const = 0;

    enum Extension
    {};

    class ExtensionOption
    {};

    class ExtensionReturn
    {};

    virtual bool extension(Extension extension, const ExtensionOption* option = 0, ExtensionReturn* output = 0);
    virtual bool supportsExtension(Extension extension) const;

private:
    QWebPluginFactoryPrivate* d;
};

#endif
