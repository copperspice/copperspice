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

#include "qdeclarativepropertyvalueinterceptor.h"

#include "qdeclarative.h"

QT_BEGIN_NAMESPACE

/*!
    \class QDeclarativePropertyValueInterceptor
    \brief The QDeclarativePropertyValueInterceptor class is inherited by property interceptors such as Behavior.
    \internal

    This class intercepts property writes, allowing for custom handling. For example, Behavior uses this
    interception to provide a default animation for all changes to a property's value.
 */

/*!
    Constructs a QDeclarativePropertyValueInterceptor.
*/
QDeclarativePropertyValueInterceptor::QDeclarativePropertyValueInterceptor()
{
}

QDeclarativePropertyValueInterceptor::~QDeclarativePropertyValueInterceptor()
{
}

/*!
    \fn void QDeclarativePropertyValueInterceptor::setTarget(const QDeclarativeProperty &property)
    Set the target \a property for the value interceptor.  This method will
    be called by the QML engine when assigning a value interceptor.
*/

/*!
    \fn void QDeclarativePropertyValueInterceptor::write(const QVariant &value)
    This method will be called when a new \a value is assigned to the property being intercepted.
*/

QT_END_NAMESPACE
