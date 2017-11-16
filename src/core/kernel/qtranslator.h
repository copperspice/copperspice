/***********************************************************************
*
* Copyright (c) 2012-2017 Barbara Geller
* Copyright (c) 2012-2017 Ansel Sermersheim
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

#ifndef QTRANSLATOR_H
#define QTRANSLATOR_H

#include <QtCore/qobject.h>
#include <QtCore/qbytearray.h>
#include <QScopedPointer>

#ifndef QT_NO_TRANSLATION

class QLocale;
class QTranslatorPrivate;

class Q_CORE_EXPORT QTranslator : public QObject
{
   CORE_CS_OBJECT(QTranslator)

 public:
   explicit QTranslator(QObject *parent = nullptr);
   ~QTranslator();

   virtual QString translate(const char *context, const char *sourceText, const char *disambiguation = 0, int n = -1) const;
   virtual bool isEmpty() const;

   bool load(const QString &filename, const QString &directory = QString(), const QString &search_delimiters = QString(),
                   const QString &suffix = QString());

   bool load(const QLocale &locale, const QString &filename, const QString &prefix = QString(),
                  const QString &directory = QString(), const QString &suffix = QString());

   bool load(const uchar *data, int len);

 protected:
   QScopedPointer<QTranslatorPrivate> d_ptr;

 private:
   Q_DISABLE_COPY(QTranslator)
   Q_DECLARE_PRIVATE(QTranslator)

};

#endif // QT_NO_TRANSLATION

#endif // QTRANSLATOR_H
