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

#include <qcameraviewfinder.h>

#include <qcamera.h>
#include <qdebug.h>
#include <qvideodeviceselectorcontrol.h>

#include <qmediaobject_p.h>
#include <qvideowidget_p.h>

class QCameraViewfinderPrivate : public QVideoWidgetPrivate
{
 public:
   QCameraViewfinderPrivate()
      : QVideoWidgetPrivate() {
   }

 private:
   Q_DECLARE_NON_CONST_PUBLIC(QCameraViewfinder)
};

QCameraViewfinder::QCameraViewfinder(QWidget *parent)
   : QVideoWidget(*new QCameraViewfinderPrivate, parent)
{
}

QCameraViewfinder::~QCameraViewfinder()
{
}

QMediaObject *QCameraViewfinder::mediaObject() const
{
   return QVideoWidget::mediaObject();
}

bool QCameraViewfinder::setMediaObject(QMediaObject *object)
{
   return QVideoWidget::setMediaObject(object);
}
