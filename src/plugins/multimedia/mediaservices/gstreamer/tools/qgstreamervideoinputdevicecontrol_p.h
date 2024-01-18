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

#ifndef QGSTREAMERVIDEOINPUTDEVICECONTROL_H
#define QGSTREAMERVIDEOINPUTDEVICECONTROL_H

#include <qvideodeviceselectorcontrol.h>
#include <qstringlist.h>

#include <gst/gst.h>
#include <qcamera.h>

class QGstreamerVideoInputDeviceControl : public QVideoDeviceSelectorControl
{
   CS_OBJECT(QGstreamerVideoInputDeviceControl)

 public:
   QGstreamerVideoInputDeviceControl(QObject *parent);
   QGstreamerVideoInputDeviceControl(GstElementFactory *factory, QObject *parent);

   ~QGstreamerVideoInputDeviceControl();

   int deviceCount() const override;

   QString deviceName(int index) const override;
   QString deviceDescription(int index) const override;

   int defaultDevice() const override;
   int selectedDevice() const override;

   static QString primaryCamera()   {
      return tr("Main camera");
   }
   static QString secondaryCamera() {
      return tr("Front camera");
   }

   CS_SLOT_1(Public, void setSelectedDevice(int index) override)
   CS_SLOT_2(setSelectedDevice)

 private:
   GstElementFactory *m_factory;

   int m_selectedDevice;
};

#endif
