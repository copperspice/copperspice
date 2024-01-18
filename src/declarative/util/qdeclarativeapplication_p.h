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

#ifndef QDECLARATIVEAPPLICATION_P_H
#define QDECLARATIVEAPPLICATION_P_H

#include <QtCore/QObject>
#include <qdeclarative.h>
#include <qdeclarativeglobal_p.h>

QT_BEGIN_NAMESPACE

class QDeclarativeApplicationPrivate;

class Q_DECLARATIVE_PRIVATE_EXPORT QDeclarativeApplication : public QObject
{
   DECL_CS_OBJECT(QDeclarativeApplication)
   DECL_CS_PROPERTY_READ(active, active)
   DECL_CS_PROPERTY_NOTIFY(active, activeChanged)
   DECL_CS_PROPERTY_READ(layoutDirection, layoutDirection)
   DECL_CS_PROPERTY_NOTIFY(layoutDirection, layoutDirectionChanged)

 public:
   explicit QDeclarativeApplication(QObject *parent = nullptr);
   virtual ~QDeclarativeApplication();
   bool active() const;
   Qt::LayoutDirection layoutDirection() const;

   DECL_CS_SIGNAL_1(Public, void activeChanged())
   DECL_CS_SIGNAL_2(activeChanged)
   DECL_CS_SIGNAL_1(Public, void layoutDirectionChanged())
   DECL_CS_SIGNAL_2(layoutDirectionChanged)

 protected:
   bool eventFilter(QObject *obj, QEvent *event);

 private:
   Q_DISABLE_COPY(QDeclarativeApplication)
   Q_DECLARE_PRIVATE(QDeclarativeApplication)
};

QT_END_NAMESPACE

QML_DECLARE_TYPE(QDeclarativeApplication)

#endif // QDECLARATIVEAPPLICATION_P_H
