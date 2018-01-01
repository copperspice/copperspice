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

#ifndef QFILEICONPROVIDER_H
#define QFILEICONPROVIDER_H

#include <QtCore/qfileinfo.h>
#include <QtCore/qscopedpointer.h>
#include <QtGui/qicon.h>

QT_BEGIN_NAMESPACE

#ifndef QT_NO_FILEICONPROVIDER

class QFileIconProviderPrivate;

class Q_GUI_EXPORT QFileIconProvider
{

 public:
   QFileIconProvider();
   virtual ~QFileIconProvider();
   enum IconType { Computer, Desktop, Trashcan, Network, Drive, Folder, File };
   virtual QIcon icon(IconType type) const;
   virtual QIcon icon(const QFileInfo &info) const;
   virtual QString type(const QFileInfo &info) const;

 private:
   Q_DECLARE_PRIVATE(QFileIconProvider)
   QScopedPointer<QFileIconProviderPrivate> d_ptr;
   Q_DISABLE_COPY(QFileIconProvider)
   friend class QFileDialog;
};

#endif // QT_NO_FILEICONPROVIDER

QT_END_NAMESPACE

#endif // QFILEICONPROVIDER_H

