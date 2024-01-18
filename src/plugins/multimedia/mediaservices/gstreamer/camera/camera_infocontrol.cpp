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

#include <camera_infocontrol.h>

#include <qgstutils_p.h>

CameraBinInfoControl::CameraBinInfoControl(GstElementFactory *sourceFactory, QObject *parent)
   : QCameraInfoControl(parent), m_sourceFactory(sourceFactory)
{
   gst_object_ref(GST_OBJECT(m_sourceFactory));
}

CameraBinInfoControl::~CameraBinInfoControl()
{
   gst_object_unref(GST_OBJECT(m_sourceFactory));
}

QCamera::Position CameraBinInfoControl::cameraPosition(const QString &device) const
{
   return QGstUtils::cameraPosition(device, m_sourceFactory);
}

int CameraBinInfoControl::cameraOrientation(const QString &device) const
{
   return QGstUtils::cameraOrientation(device, m_sourceFactory);
}
