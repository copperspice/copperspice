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

#ifndef QTRANSLATOR_H
#define QTRANSLATOR_H

#include <qbytearray.h>
#include <qobject.h>
#include <qscopedpointer.h>

class QLocale;
class QTranslatorPrivate;

class Q_CORE_EXPORT QTranslator : public QObject
{
   CORE_CS_OBJECT(QTranslator)

 public:
   explicit QTranslator(QObject *parent = nullptr);

   QTranslator(const QTranslator &) = delete;
   QTranslator &operator=(const QTranslator &) = delete;

   ~QTranslator();

   virtual QString translate(const char *context, const char *text, const char *comment = nullptr,
         std::optional<int> numArg = std::optional<int>()) const;

   virtual QString translate(const QString &context, const QString &text, const QString &comment = QString(),
         std::optional<int> numArg = std::optional<int>()) const;

   virtual bool isEmpty() const;

   static QString replacePercentN(QString text, int numArg);

   bool load(const QString &filename, const QString &directory = QString(), const QString &search_delimiters = QString(),
         const QString &suffix = QString());

   bool load(const QLocale &locale, const QString &filename, const QString &prefix = QString(),
         const QString &directory = QString(), const QString &suffix = QString());

   bool load(const uchar *data, int len, const QString &directory = QString());

 protected:
   QScopedPointer<QTranslatorPrivate> d_ptr;

 private:
   Q_DECLARE_PRIVATE(QTranslator)
};

#endif
