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

#ifndef QFILEICONPROVIDER_P_H
#define QFILEICONPROVIDER_P_H

#include <qstring.h>
#include <qicon.h>
#include <qstyle.h>

class QFileInfo;
class QFileIconProvider;

class QFileIconProviderPrivate
{
   Q_DECLARE_PUBLIC(QFileIconProvider)

 public:
   QFileIconProviderPrivate(QFileIconProvider *q);

   QIcon getIcon(QStyle::StandardPixmap name) const;
   QIcon getIcon(const QFileInfo &fi) const;

   QFileIconProvider *q_ptr;
   const QString homePath;
   QFileIconProvider::Options options;

 private:
   mutable QIcon file;
   mutable QIcon fileLink;
   mutable QIcon directory;
   mutable QIcon directoryLink;
   mutable QIcon harddisk;
   mutable QIcon floppy;
   mutable QIcon cdrom;
   mutable QIcon ram;
   mutable QIcon network;
   mutable QIcon computer;
   mutable QIcon desktop;
   mutable QIcon trashcan;
   mutable QIcon generic;
   mutable QIcon home;
};



#endif // QFILEICONPROVIDER_P_H
