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

#ifndef QFILEICONPROVIDER_P_H
#define QFILEICONPROVIDER_P_H

#include <QtCore/qstring.h>
#include <QtGui/qicon.h>
#include <QtGui/qstyle.h>

QT_BEGIN_NAMESPACE

class QFileInfo;
class QFileIconProvider;

class QFileIconProviderPrivate
{
   Q_DECLARE_PUBLIC(QFileIconProvider)

 public:
   QFileIconProviderPrivate();
   void setUseCustomDirectoryIcons(bool enable);
   QIcon getIcon(QStyle::StandardPixmap name) const;

#ifdef Q_OS_WIN
   QIcon getWinIcon(const QFileInfo &fi) const;
#elif defined(Q_OS_MAC)
   QIcon getMacIcon(const QFileInfo &fi) const;
#endif

   QFileIconProvider *q_ptr;
   const QString homePath;

 private:
   bool useCustomDirectoryIcons;
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

QT_END_NAMESPACE

#endif // QFILEICONPROVIDER_P_H
