/***********************************************************************
*
* Copyright (c) 2012-2018 Barbara Geller
* Copyright (c) 2012-2018 Ansel Sermersheim
* Copyright (c) 2012-2016 Digia Plc and/or its subsidiary(-ies).
* Copyright (c) 2008-2012 Nokia Corporation and/or its subsidiary(-ies).
* All rights reserved.
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
* <http://www.gnu.org/licenses/>.
*
***********************************************************************/

#ifndef QPICTUREFORMATPLUGIN_H
#define QPICTUREFORMATPLUGIN_H

#include <qplugin.h>
#include <qfactoryinterface.h>

#if ! defined(QT_NO_PICTURE)

class QPicture;
class QImage;
class QString;
class QStringList;

struct Q_GUI_EXPORT QPictureFormatInterface : public QFactoryInterface {
   virtual bool loadPicture(const QString &format, const QString &filename, QPicture *) = 0;
   virtual bool savePicture(const QString &format, const QString &filename, const QPicture &) = 0;
   virtual bool installIOHandler(const QString &format) = 0;
};

#define QPictureFormatInterface_iid "com.copperspice.QPictureFormatInterface"
CS_DECLARE_INTERFACE(QPictureFormatInterface, QPictureFormatInterface_iid)

class Q_GUI_EXPORT QPictureFormatPlugin : public QObject, public QPictureFormatInterface
{
   GUI_CS_OBJECT(QPictureFormatPlugin)
   CS_INTERFACES(QPictureFormatInterface, QFactoryInterface)

 public:
   explicit QPictureFormatPlugin(QObject *parent = nullptr);
   ~QPictureFormatPlugin();

   bool loadPicture(const QString &format, const QString &filename, QPicture *pic) override;
   bool savePicture(const QString &format, const QString &filename, const QPicture &pic) override;
};

#endif


#endif