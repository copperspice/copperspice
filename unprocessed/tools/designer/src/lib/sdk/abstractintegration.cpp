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

#include "abstractintegration.h"
#include "abstractformeditor.h"

#include <QtCore/QVariant>
#include <QtCore/QSharedPointer>

QT_BEGIN_NAMESPACE

// Add 'private' struct as a dynamic property.

static const char privatePropertyC[] = "_q_integrationprivate";

struct QDesignerIntegrationInterfacePrivate {
    QDesignerIntegrationInterfacePrivate() :
        headerSuffix(QLatin1String(".h")),
        headerLowercase(true) {}

    QString headerSuffix;
    bool headerLowercase;
};

typedef QSharedPointer<QDesignerIntegrationInterfacePrivate> QDesignerIntegrationInterfacePrivatePtr;

QT_END_NAMESPACE
Q_DECLARE_METATYPE(QT_PREPEND_NAMESPACE(QDesignerIntegrationInterfacePrivatePtr))
QT_BEGIN_NAMESPACE

static QDesignerIntegrationInterfacePrivatePtr integrationD(const QObject *o)
{
    const QVariant property = o->property(privatePropertyC);
    Q_ASSERT(qVariantCanConvert<QDesignerIntegrationInterfacePrivatePtr>(property));
    return qvariant_cast<QDesignerIntegrationInterfacePrivatePtr>(property);
}

QDesignerIntegrationInterface::QDesignerIntegrationInterface(QDesignerFormEditorInterface *core, QObject *parent)
    : QObject(parent),
      m_core(core)
{
    core->setIntegration(this);
    const QDesignerIntegrationInterfacePrivatePtr d(new QDesignerIntegrationInterfacePrivate);
    setProperty(privatePropertyC, qVariantFromValue<QDesignerIntegrationInterfacePrivatePtr>(d));
}

QString QDesignerIntegrationInterface::headerSuffix() const
{
    return integrationD(this)->headerSuffix;
}

void QDesignerIntegrationInterface::setHeaderSuffix(const QString &headerSuffix)
{
    integrationD(this)->headerSuffix = headerSuffix;
}

bool QDesignerIntegrationInterface::isHeaderLowercase() const
{
    return integrationD(this)->headerLowercase;
}

void QDesignerIntegrationInterface::setHeaderLowercase(bool headerLowercase)
{
    integrationD(this)->headerLowercase = headerLowercase;
}

QT_END_NAMESPACE
