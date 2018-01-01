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

#ifndef ABSTRACTINTEGRATION_H
#define ABSTRACTINTEGRATION_H

#include <QtDesigner/sdk_global.h>

#include <QtCore/QObject>
#include <QtCore/QString>

QT_BEGIN_HEADER

QT_BEGIN_NAMESPACE

class QDesignerFormEditorInterface;

class QDESIGNER_SDK_EXPORT QDesignerIntegrationInterface: public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString headerSuffix READ headerSuffix WRITE setHeaderSuffix)
    Q_PROPERTY(bool headerLowercase READ isHeaderLowercase WRITE setHeaderLowercase)

public:
    QDesignerIntegrationInterface(QDesignerFormEditorInterface *core, QObject *parent = 0);

    inline QDesignerFormEditorInterface *core() const;

    virtual QWidget *containerWindow(QWidget *widget) const = 0;

    QString headerSuffix() const;
    void setHeaderSuffix(const QString &headerSuffix);

    bool isHeaderLowercase() const;
    void setHeaderLowercase(bool headerLowerCase);

private:
    QDesignerFormEditorInterface *m_core;
};

inline QDesignerFormEditorInterface *QDesignerIntegrationInterface::core() const
{ return m_core; }

QT_END_NAMESPACE

QT_END_HEADER

#endif // ABSTRACTINTEGRATION_H
