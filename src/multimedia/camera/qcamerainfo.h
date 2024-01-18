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

#ifndef QCAMERAINFO_H
#define QCAMERAINFO_H

#include <qcamera.h>
#include <qsharedpointer.h>

class QCameraInfoPrivate;

class Q_MULTIMEDIA_EXPORT QCameraInfo
{
 public:
   explicit QCameraInfo(const QString &name = QString());
   explicit QCameraInfo(const QCamera &camera);
   QCameraInfo(const QCameraInfo &other);

   ~QCameraInfo();

   QCameraInfo &operator=(const QCameraInfo &other);
   bool operator==(const QCameraInfo &other) const;
   inline bool operator!=(const QCameraInfo &other) const;

   bool isNull() const;

   QString deviceName() const;
   QString description() const;
   QCamera::Position position() const;
   int orientation() const;

   static QCameraInfo defaultCamera();
   static QList<QCameraInfo> availableCameras(QCamera::Position position = QCamera::UnspecifiedPosition);

 private:
   QSharedPointer<QCameraInfoPrivate> d;
};

bool QCameraInfo::operator!=(const QCameraInfo &other) const
{
   return !operator==(other);
}

Q_MULTIMEDIA_EXPORT QDebug operator<<(QDebug, const QCameraInfo &);

#endif
