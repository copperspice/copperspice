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

#ifndef QTBRUSHMANAGER_H
#define QTBRUSHMANAGER_H

#include <QtDesigner/QDesignerBrushManagerInterface>
#include "formeditor_global.h"

#include <QtCore/QObject>
#include <QtCore/QMap>
#include <QtGui/QBrush>

QT_BEGIN_NAMESPACE

namespace qdesigner_internal {

class QtBrushManagerPrivate;

class QT_FORMEDITOR_EXPORT QtBrushManager : public QDesignerBrushManagerInterface
{
    Q_OBJECT
public:
    QtBrushManager(QObject *parent = 0);
    ~QtBrushManager();

    QBrush brush(const QString &name) const;
    QMap<QString, QBrush> brushes() const;
    QString currentBrush() const;

    QString addBrush(const QString &name, const QBrush &brush);
    void removeBrush(const QString &name);
    void setCurrentBrush(const QString &name);

    QPixmap brushPixmap(const QBrush &brush) const;

private:
    QScopedPointer<QtBrushManagerPrivate> d_ptr;
    Q_DECLARE_PRIVATE(QtBrushManager)
    Q_DISABLE_COPY(QtBrushManager)
};

}  // namespace qdesigner_internal

QT_END_NAMESPACE

#endif
