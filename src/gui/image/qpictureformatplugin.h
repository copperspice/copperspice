/***********************************************************************
*
* Copyright (c) 2012-2016 Barbara Geller
* Copyright (c) 2012-2016 Ansel Sermersheim
* Copyright (c) 2012-2014 Digia Plc and/or its subsidiary(-ies).
* Copyright (c) 2008-2012 Nokia Corporation and/or its subsidiary(-ies).
* All rights reserved.
*
* This file is part of CopperSpice.
*
* CopperSpice is free software: you can redistribute it and/or 
* modify it under the terms of the GNU Lesser General Public License
* version 2.1 as published by the Free Software Foundation.
*
* CopperSpice is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
* Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public
* License along with CopperSpice.  If not, see 
* <http://www.gnu.org/licenses/>.
*
***********************************************************************/

#ifndef QPICTUREFORMATPLUGIN_H
#define QPICTUREFORMATPLUGIN_H

#include <QtCore/qplugin.h>
#include <QtCore/qfactoryinterface.h>

QT_BEGIN_NAMESPACE

#if ! defined(QT_NO_PICTURE)

class QPicture;
class QImage;
class QString;
class QStringList;

struct Q_GUI_EXPORT QPictureFormatInterface : public QFactoryInterface {
   virtual bool loadPicture(const QString &format, const QString &filename, QPicture *) = 0;
   virtual bool savePicture(const QString &format, const QString &filename, const QPicture &) = 0;

   virtual bool installIOHandler(const QString &) = 0;
};

#define QPictureFormatInterface_iid "com.copperspice.QPictureFormatInterface"
CS_DECLARE_INTERFACE(QPictureFormatInterface, QPictureFormatInterface_iid)


class Q_GUI_EXPORT QPictureFormatPlugin : public QObject, public QPictureFormatInterface
{
   GUI_CS_OBJECT(QPictureFormatPlugin)
   CS_INTERFACES(QPictureFormatInterface, QFactoryInterface)

 public:
   explicit QPictureFormatPlugin(QObject *parent = 0);
   ~QPictureFormatPlugin();

   virtual QStringList keys() const = 0;
   virtual bool loadPicture(const QString &format, const QString &filename, QPicture *pic);
   virtual bool savePicture(const QString &format, const QString &filename, const QPicture &pic);
   virtual bool installIOHandler(const QString &format) = 0;

};

#endif // QT_NO_PICTURE

QT_END_NAMESPACE

#endif // QPICTUREFORMATPLUGIN_H
