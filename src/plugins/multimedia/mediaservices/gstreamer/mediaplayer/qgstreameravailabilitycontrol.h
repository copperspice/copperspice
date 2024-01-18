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

#ifndef QGSTREAMERAVAILABILITYCONTROL_H
#define QGSTREAMERAVAILABILITYCONTROL_H

#include <QObject>
#include <qmediaavailabilitycontrol.h>

class QMediaPlayerResourceSetInterface;

class QGStreamerAvailabilityControl : public QMediaAvailabilityControl
{
   CS_OBJECT(QGStreamerAvailabilityControl)

 public:
   QGStreamerAvailabilityControl(QMediaPlayerResourceSetInterface *resources, QObject *parent = nullptr);
   QMultimedia::AvailabilityStatus availability() const override;

 private:
   QMediaPlayerResourceSetInterface *m_resources;

   CS_SLOT_1(Private, void handleAvailabilityChanged())
   CS_SLOT_2(handleAvailabilityChanged)
};

#endif
