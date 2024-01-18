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

#ifndef QMEDIARESOURCESET_P_H
#define QMEDIARESOURCESET_P_H

#include <qobject.h>

#define QMediaPlayerResourceSetInterface_iid "com.copperspice.CS.mediaPlayerResourceSet/1.0"

class Q_MULTIMEDIA_EXPORT QMediaPlayerResourceSetInterface : public QObject
{
   MULTI_CS_OBJECT(QMediaPlayerResourceSetInterface)

 public:
   virtual bool isVideoEnabled() const = 0;
   virtual bool isGranted() const = 0;
   virtual bool isAvailable() const = 0;

   virtual void acquire() = 0;
   virtual void release() = 0;
   virtual void setVideoEnabled(bool enabled) = 0;

   static QString iid();

   MULTI_CS_SIGNAL_1(Public, void resourcesGranted())
   MULTI_CS_SIGNAL_2(resourcesGranted)
   MULTI_CS_SIGNAL_1(Public, void resourcesLost())
   MULTI_CS_SIGNAL_2(resourcesLost)
   MULTI_CS_SIGNAL_1(Public, void resourcesDenied())
   MULTI_CS_SIGNAL_2(resourcesDenied)
   MULTI_CS_SIGNAL_1(Public, void resourcesReleased())
   MULTI_CS_SIGNAL_2(resourcesReleased)
   MULTI_CS_SIGNAL_1(Public, void availabilityChanged(bool available))
   MULTI_CS_SIGNAL_2(availabilityChanged, available)

 protected:
   QMediaPlayerResourceSetInterface(QObject *parent = nullptr);
};

#endif
