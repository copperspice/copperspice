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

#ifndef ABSTRACTMETADATABASE_H
#define ABSTRACTMETADATABASE_H

#include <QtDesigner/sdk_global.h>

#include <QtCore/QObject>
#include <QtCore/QList>
#include <QtCore/QHash>

QT_BEGIN_HEADER

QT_BEGIN_NAMESPACE

class QCursor;
class QWidget;

class QDesignerFormEditorInterface;

class QDesignerMetaDataBaseItemInterface
{
public:
    virtual ~QDesignerMetaDataBaseItemInterface() {}

    virtual QString name() const = 0;
    virtual void setName(const QString &name) = 0;
    
    virtual QList<QWidget*> tabOrder() const = 0;
    virtual void setTabOrder(const QList<QWidget*> &tabOrder) = 0;
  
    virtual bool enabled() const = 0;
    virtual void setEnabled(bool b) = 0;    
};


class QDESIGNER_SDK_EXPORT QDesignerMetaDataBaseInterface: public QObject
{
    Q_OBJECT
public:
    QDesignerMetaDataBaseInterface(QObject *parent = 0);
    virtual ~QDesignerMetaDataBaseInterface();

    virtual QDesignerMetaDataBaseItemInterface *item(QObject *object) const = 0;
    virtual void add(QObject *object) = 0;
    virtual void remove(QObject *object) = 0;

    virtual QList<QObject*> objects() const = 0;

    virtual QDesignerFormEditorInterface *core() const = 0;
    
Q_SIGNALS:
    void changed();
};

QT_END_NAMESPACE

QT_END_HEADER

#endif // ABSTRACTMETADATABASE_H
