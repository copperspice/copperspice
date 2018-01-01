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

#ifndef CONTAINER_H
#define CONTAINER_H

#include <QtDesigner/extension.h>
#include <QtCore/QObject>

QT_BEGIN_HEADER

QT_BEGIN_NAMESPACE

class QWidget;

class QDesignerContainerExtension
{
public:
    virtual ~QDesignerContainerExtension() {}

    virtual int count() const = 0;
    virtual QWidget *widget(int index) const = 0;

    virtual int currentIndex() const = 0;
    virtual void setCurrentIndex(int index) = 0;

    virtual void addWidget(QWidget *widget) = 0;
    virtual void insertWidget(int index, QWidget *widget) = 0;
    virtual void remove(int index) = 0;
};
Q_DECLARE_EXTENSION_INTERFACE(QDesignerContainerExtension, "com.trolltech.Qt.Designer.Container")

QT_END_NAMESPACE

QT_END_HEADER

#endif // CONTAINER_H
