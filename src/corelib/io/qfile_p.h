/***********************************************************************
*
* Copyright (c) 2012-2014 Barbara Geller
* Copyright (c) 2012-2014 Ansel Sermersheim
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

#ifndef QFILE_P_H
#define QFILE_P_H

#include "qfiledevice_p.h"

QT_BEGIN_NAMESPACE

class QTemporaryFile;

class QFilePrivate : public QFileDevicePrivate
{
   Q_DECLARE_PUBLIC(QFile)
   friend class QTemporaryFile;

 protected:
   QFilePrivate();
   ~QFilePrivate();

   bool openExternalFile(int flags, int fd, QFile::FileHandleFlags handleFlags);
   bool openExternalFile(int flags, FILE *fh, QFile::FileHandleFlags handleFlags);

   virtual QAbstractFileEngine *engine() const;

   QString fileName;

 private:
   static QFile::EncoderFn encoder;
   static QFile::DecoderFn decoder;
};

QT_END_NAMESPACE

#endif // QFILE_P_H
