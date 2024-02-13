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

/*****************************************************
** Copyright (c) 2013 Laszlo Papp <lpapp@kde.org>
*****************************************************/

#ifndef QCOMMANDLINEOPTION_H
#define QCOMMANDLINEOPTION_H

#include <qshareddata.h>
#include <qstringlist.h>

class QCommandLineOptionPrivate;

class Q_CORE_EXPORT QCommandLineOption
{
 public:
   explicit QCommandLineOption(const QString &name, const QString &description = QString(),
         const QString &valueName = QString(), const QString &defaultValue = QString());

   explicit QCommandLineOption(const QStringList &names, const QString &description = QString(),
         const QString &valueName = QString(), const QString &defaultValue = QString());

   QCommandLineOption(const QCommandLineOption &other);

   ~QCommandLineOption();

   QCommandLineOption &operator=(const QCommandLineOption &other);

   QCommandLineOption &operator=(QCommandLineOption && other) {
      qSwap(d, other.d);
      return *this;
   }

   void swap(QCommandLineOption &other) {
      qSwap(d, other.d);
   }

   QStringList names() const;

   void setValueName(const QString &name);
   QString valueName() const;

   void setDescription(const QString &description);
   QString description() const;

   void setDefaultValue(const QString &defaultValue);
   void setDefaultValues(const QStringList &defaultValues);
   QStringList defaultValues() const;

 private:
   QSharedDataPointer<QCommandLineOptionPrivate> d;
};

#endif
