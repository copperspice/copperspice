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

#ifndef QVIDEOSURFACEOUTPUT_P_H
#define QVIDEOSURFACEOUTPUT_P_H

#include <qmediabindableinterface.h>
#include <qsharedpointer.h>
#include <qpointer.h>

class QAbstractVideoSurface;
class QVideoRendererControl;

class QVideoSurfaceOutput : public QObject, public QMediaBindableInterface
{
   MULTI_CS_OBJECT_MULTIPLE(QVideoSurfaceOutput, QObject)

   CS_INTERFACES(QMediaBindableInterface)

 public:
   QVideoSurfaceOutput(QObject *parent = nullptr);
   ~QVideoSurfaceOutput();

   QMediaObject *mediaObject() const override;

   void setVideoSurface(QAbstractVideoSurface *surface);

 protected:
   bool setMediaObject(QMediaObject *object) override;

 private:
   QPointer<QAbstractVideoSurface> m_surface;
   QPointer<QVideoRendererControl> m_control;
   QPointer<QMediaService> m_service;
   QPointer<QMediaObject> m_object;
};

#endif
