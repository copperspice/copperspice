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

#ifndef QPICTUREFORMATPLUGIN_H
#define QPICTUREFORMATPLUGIN_H

#include <qplugin.h>
#include <qfactoryinterface.h>
#include <qstringfwd.h>

#if ! defined(QT_NO_PICTURE)

class QPicture;
class QImage;
class QStringList;

#define QPictureFormatInterface_ID "com.copperspice.CS.PictureFormatInterface"

class Q_GUI_EXPORT QPictureFormatPlugin : public QObject
{
   GUI_CS_OBJECT(QPictureFormatPlugin)

 public:
   explicit QPictureFormatPlugin(QObject *parent = nullptr);
   ~QPictureFormatPlugin();

   virtual bool loadPicture(const QString &format, const QString &fileName, QPicture *picture);
   virtual bool savePicture(const QString &format, const QString &fileName, const QPicture &picture);
   virtual bool installIOHandler(const QString &format) = 0;
};

#endif

#endif