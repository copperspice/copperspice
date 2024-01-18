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

#ifndef QCAMERAVIEWFINDER_H
#define QCAMERAVIEWFINDER_H

#include <qstringlist.h>
#include <qpair.h>
#include <qsize.h>
#include <qpoint.h>
#include <qrect.h>

#include <qmediacontrol.h>
#include <qmediaobject.h>
#include <qmediaservice.h>
#include <qvideowidget.h>

class QCamera;
class QCameraViewfinderPrivate;

class Q_MULTIMEDIA_EXPORT QCameraViewfinder : public QVideoWidget
{
   MULTI_CS_OBJECT(QCameraViewfinder)

 public:
   explicit QCameraViewfinder(QWidget *parent = nullptr);

   QCameraViewfinder(const QCameraViewfinder &) = delete;
   QCameraViewfinder &operator=(const QCameraViewfinder &) = delete;

   ~QCameraViewfinder();

   QMediaObject *mediaObject() const override;

 protected:
   bool setMediaObject(QMediaObject *object) override;

 private:
   Q_DECLARE_PRIVATE(QCameraViewfinder)
};

#endif
