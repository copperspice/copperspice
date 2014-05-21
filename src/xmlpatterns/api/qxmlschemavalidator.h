/***********************************************************************
*
* Copyright (c) 2012-2014 Barbara Geller
* Copyright (c) 2012-2014 Ansel Sermersheim
* Copyright (c) 2012-2014 Digia Plc and/or its subsidiary(-ies).
* Copyright (c) 2008-2012 Nokia Corporation and/or its subsidiary(-ies).
* All rights reserved.
*
* This file is part of CopperSpice.
*
* CopperSpice is free software: you can redistribute it and/or 
* modify it under the terms of the GNU Lesser General Public License
* version 2.1 as published by the Free Software Foundation.
*
* CopperSpice is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
* Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public
* License along with CopperSpice.  If not, see 
* <http://www.gnu.org/licenses/>.
*
***********************************************************************/

#ifndef QXMLSCHEMAVALIDATOR_H
#define QXMLSCHEMAVALIDATOR_H

#include <QtCore/QUrl>
#include <QtXmlPatterns/QXmlNamePool>

QT_BEGIN_NAMESPACE

class QAbstractMessageHandler;
class QAbstractUriResolver;
class QIODevice;
class QNetworkAccessManager;
class QUrl;
class QXmlNamePool;
class QXmlSchema;
class QXmlSchemaValidatorPrivate;

class Q_XMLPATTERNS_EXPORT QXmlSchemaValidator
{
    public:
        QXmlSchemaValidator();
        QXmlSchemaValidator(const QXmlSchema &schema);
        ~QXmlSchemaValidator();

        void setSchema(const QXmlSchema &schema);

        bool validate(const QUrl &source) const;
        bool validate(QIODevice *source, const QUrl &documentUri = QUrl()) const;
        bool validate(const QByteArray &data, const QUrl &documentUri = QUrl()) const;

        QXmlNamePool namePool() const;
        QXmlSchema schema() const;

        void setMessageHandler(QAbstractMessageHandler *handler);
        QAbstractMessageHandler *messageHandler() const;

        void setUriResolver(const QAbstractUriResolver *resolver);
        const QAbstractUriResolver *uriResolver() const;

        void setNetworkAccessManager(QNetworkAccessManager *networkmanager);
        QNetworkAccessManager *networkAccessManager() const;

    private:
        QXmlSchemaValidatorPrivate* const d;

        Q_DISABLE_COPY(QXmlSchemaValidator)
};

QT_END_NAMESPACE

#endif
