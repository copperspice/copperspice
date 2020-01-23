/***********************************************************************
*
* Copyright (c) 2012-2020 Barbara Geller
* Copyright (c) 2012-2020 Ansel Sermersheim
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

#ifndef QABSTRACTVIDEOFILTER_H
#define QABSTRACTVIDEOFILTER_H

#include <qobject.h>
#include <qvideoframe.h>
#include <qvideosurfaceformat.h>

class QAbstractVideoFilterPrivate;

class Q_MULTIMEDIA_EXPORT QVideoFilterRunnable
{
public:
    enum RunFlag {
        LastInChain = 0x01
    };
    using RunFlags = QFlags<RunFlag>;

    virtual ~QVideoFilterRunnable();
    virtual QVideoFrame run(QVideoFrame *input, const QVideoSurfaceFormat &surfaceFormat, RunFlags flags) = 0;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(QVideoFilterRunnable::RunFlags)

class Q_MULTIMEDIA_EXPORT QAbstractVideoFilter : public QObject
{
    MULTI_CS_OBJECT(QAbstractVideoFilter)

    MULTI_CS_PROPERTY_READ(active, isActive)
    MULTI_CS_PROPERTY_WRITE(active, setActive)
    MULTI_CS_PROPERTY_NOTIFY(active, activeChanged)

public:
    explicit QAbstractVideoFilter(QObject *parent = nullptr);
    ~QAbstractVideoFilter();

    bool isActive() const;
    void setActive(bool v);

    virtual QVideoFilterRunnable *createFilterRunnable() = 0;

    MULTI_CS_SIGNAL_1(Public, void activeChanged())
    MULTI_CS_SIGNAL_2(activeChanged)

private:
    Q_DECLARE_PRIVATE(QAbstractVideoFilter)
    Q_DISABLE_COPY(QAbstractVideoFilter)

    QAbstractVideoFilterPrivate *d_ptr;
};

#endif
