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

#ifndef QMEDIACONTAINERCONTROL_H
#define QMEDIACONTAINERCONTROL_H

#include <qstring.h>
#include <qmediacontrol.h>

class Q_MULTIMEDIA_EXPORT QMediaContainerControl : public QMediaControl
{
    MULTI_CS_OBJECT(QMediaContainerControl)

public:
    virtual ~QMediaContainerControl();

    virtual QStringList supportedContainers() const = 0;
    virtual QString containerFormat() const = 0;
    virtual void setContainerFormat(const QString &format) = 0;

    virtual QString containerDescription(const QString &format) const = 0;

protected:
    explicit QMediaContainerControl(QObject *parent = nullptr);
};

#define QMediaContainerControl_iid "com.copperspice.CS.mediaContainerControl/1.0"
CS_DECLARE_INTERFACE(QMediaContainerControl, QMediaContainerControl_iid)

#endif
