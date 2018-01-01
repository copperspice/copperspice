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

/****************************************************************************
**
** Copyright (C) 2012 David Faure <faure@kde.org>
** Contact: http://www.qt-project.org/legal
**
****************************************************************************/

#ifndef QSAVEFILE_H
#define QSAVEFILE_H

#include <QtCore/qfiledevice.h>
#include <QtCore/qstring.h>

#ifdef open
#error qsavefile.h must be included before any header file that defines open
#endif

QT_BEGIN_NAMESPACE

class QAbstractFileEngine;
class QSaveFilePrivate;

class Q_CORE_EXPORT QSaveFile : public QFileDevice
{
   CORE_CS_OBJECT(QSaveFile)
   Q_DECLARE_PRIVATE(QSaveFile)

 public:

   explicit QSaveFile(const QString &name);
   explicit QSaveFile(QObject *parent = nullptr);
   explicit QSaveFile(const QString &name, QObject *parent);
   ~QSaveFile();

   QString fileName() const override;
   void setFileName(const QString &name);

   bool open(OpenMode flags) override;
   bool commit();

   void cancelWriting();

   void setDirectWriteFallback(bool enabled);
   bool directWriteFallback() const;

 protected:
   qint64 writeData(const char *data, qint64 len) override;

 private:
   void close() override;
   Q_DISABLE_COPY(QSaveFile)
};

QT_END_NAMESPACE

#endif // QSAVEFILE_H
