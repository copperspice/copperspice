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

#ifndef AVFSTORAGE_H
#define AVFSTORAGE_H

#include <qdir.h>
#include <qcamera.h>

class AVFStorageLocation
{
 public:
   AVFStorageLocation();
   ~AVFStorageLocation();

   QString generateFileName(const QString &requestedName, QCamera::CaptureMode mode,
                            const QString &prefix, const QString &ext) const;

   QDir defaultDir(QCamera::CaptureMode mode) const;
   QString generateFileName(const QString &prefix, const QDir &dir, const QString &ext) const;

 private:
   mutable QMap<QString, int> m_lastUsedIndex;
};

#endif
