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

#ifndef QMEDIAAVAILABILITYCONTROL_H
#define QMEDIAAVAILABILITYCONTROL_H

#include <qstring.h>
#include <qmediacontrol.h>
#include <qmediaobject.h>
#include <qmultimedia.h>

class Q_MULTIMEDIA_EXPORT QMediaAvailabilityControl : public QMediaControl
{
   MULTI_CS_OBJECT(QMediaAvailabilityControl)

 public:
   ~QMediaAvailabilityControl();
   virtual QMultimedia::AvailabilityStatus availability() const = 0;

   MULTI_CS_SIGNAL_1(Public, void availabilityChanged(QMultimedia::AvailabilityStatus availability))
   MULTI_CS_SIGNAL_2(availabilityChanged, availability)

 protected:
   explicit QMediaAvailabilityControl(QObject *parent = nullptr);
};

#define QMediaAvailabilityControl_iid "com.copperspice.CS.mediaAvailabilityControl/1.0"
CS_DECLARE_INTERFACE(QMediaAvailabilityControl, QMediaAvailabilityControl_iid)

#endif
