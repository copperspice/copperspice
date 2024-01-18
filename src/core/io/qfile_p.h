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

#ifndef QFILE_P_H
#define QFILE_P_H

#include <qfiledevice_p.h>

class QTemporaryFile;

class QFilePrivate : public QFileDevicePrivate
{
 protected:
   QFilePrivate();
   ~QFilePrivate();

   bool openExternalFile(int flags, int fd, QFile::FileHandleFlags handleFlags);
   bool openExternalFile(int flags, FILE *fh, QFile::FileHandleFlags handleFlags);

   QAbstractFileEngine *engine() const override;

   QString fileName;

 private:
   Q_DECLARE_PUBLIC(QFile)

   static QFile::EncoderFn encoder;
   static QFile::DecoderFn decoder;

   friend class QTemporaryFile;
};

#endif
