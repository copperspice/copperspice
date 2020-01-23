/***********************************************************************
*
* Copyright (c) 2012-2020 Barbara Geller
* Copyright (c) 2012-2020 Ansel Sermersheim
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

#ifndef QFILEICONPROVIDER_H
#define QFILEICONPROVIDER_H

#include <QtCore/qfileinfo.h>
#include <QtCore/qscopedpointer.h>
#include <QtGui/qicon.h>

class QFileIconProviderPrivate;

class Q_GUI_EXPORT QFileIconProvider
{

 public:
   enum Option {
      DontUseCustomDirectoryIcons = 0x00000001
   };
   using Options = QFlags<Option>;

   QFileIconProvider();
   virtual ~QFileIconProvider();
   enum IconType { Computer, Desktop, Trashcan, Network, Drive, Folder, File };
   virtual QIcon icon(IconType type) const;
   virtual QIcon icon(const QFileInfo &info) const;
   virtual QString type(const QFileInfo &info) const;

   void setOptions(Options options);
   Options options() const;
 private:
   Q_DECLARE_PRIVATE(QFileIconProvider)
   QScopedPointer<QFileIconProviderPrivate> d_ptr;
   Q_DISABLE_COPY(QFileIconProvider)

};

Q_DECLARE_OPERATORS_FOR_FLAGS(QFileIconProvider::Options)
#endif

