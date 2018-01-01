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

#ifndef QPRINTERINFO_P_H
#define QPRINTERINFO_P_H

#include <QtCore/qglobal.h>

#ifndef QT_NO_PRINTER

#include <QtCore/qlist.h>

QT_BEGIN_NAMESPACE

class QPrinterInfoPrivate
{
 public:
   QPrinterInfoPrivate(const QString &name = QString()) :
      name(name), isDefault(false)

#if (defined(Q_OS_UNIX) && !defined(Q_OS_MAC)) || defined(Q_WS_QPA)
#if !defined(QT_NO_CUPS)
      , cupsPrinterIndex(0), hasPaperSizes(false)
#endif
#endif

   {}
   ~QPrinterInfoPrivate() {
   }

   static QPrinterInfoPrivate shared_null;

   QString name;
   bool isDefault;

#if (defined(Q_OS_UNIX) && !defined(Q_OS_MAC)) || defined(Q_WS_QPA)
#if !defined(QT_NO_CUPS)
   int cupsPrinterIndex;
   mutable bool hasPaperSizes;
   mutable QList<QPrinter::PaperSize> paperSizes;
#endif
#endif
};


class QPrinterInfoPrivateDeleter
{
 public:
   static inline void cleanup(QPrinterInfoPrivate *d) {
      if (d != &QPrinterInfoPrivate::shared_null) {
         delete d;
      }
   }
};

QT_END_NAMESPACE

#endif // QT_NO_PRINTER

#endif // QPRINTERINFO_P_H
