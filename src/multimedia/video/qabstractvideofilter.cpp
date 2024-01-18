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

#include <qabstractvideofilter.h>

class QAbstractVideoFilterPrivate
{
public:
    QAbstractVideoFilterPrivate() :
        active(true)
    { }

    bool active;
};

/*!
  \internal
 */
QVideoFilterRunnable::~QVideoFilterRunnable()
{
}

/*!
  Constructs a new QAbstractVideoFilter instance with parent object \a parent.
 */
QAbstractVideoFilter::QAbstractVideoFilter(QObject *parent) :
    QObject(parent),
    d_ptr(new QAbstractVideoFilterPrivate)
{
}

/*!
  \internal
 */
QAbstractVideoFilter::~QAbstractVideoFilter()
{
    delete d_ptr;
}

bool QAbstractVideoFilter::isActive() const
{
    Q_D(const QAbstractVideoFilter);
    return d->active;
}

void QAbstractVideoFilter::setActive(bool v)
{
    Q_D(QAbstractVideoFilter);
    if (d->active != v) {
        d->active = v;
        emit activeChanged();
    }
}

