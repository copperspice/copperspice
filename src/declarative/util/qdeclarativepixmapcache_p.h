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

#ifndef QDECLARATIVEPIXMAPCACHE_P_H
#define QDECLARATIVEPIXMAPCACHE_P_H

#include <QtCore/qcoreapplication.h>
#include <QtCore/qstring.h>
#include <QtGui/qpixmap.h>
#include <QtCore/qurl.h>

QT_BEGIN_NAMESPACE

class QDeclarativeEngine;
class QDeclarativePixmapData;

class Q_DECLARATIVE_EXPORT QDeclarativePixmap
{
   Q_DECLARE_TR_FUNCTIONS(QDeclarativePixmap)

 public:
   QDeclarativePixmap();
   QDeclarativePixmap(QDeclarativeEngine *, const QUrl &);
   QDeclarativePixmap(QDeclarativeEngine *, const QUrl &, const QSize &);
   ~QDeclarativePixmap();

   enum Status { Null, Ready, Error, Loading };

   enum Option {
      Asynchronous = 0x00000001,
      Cache        = 0x00000002
   };
   using Options = QFlags<Option>;

   bool isNull() const;
   bool isReady() const;
   bool isError() const;
   bool isLoading() const;

   Status status() const;
   QString error() const;
   const QUrl &url() const;
   const QSize &implicitSize() const;
   const QSize &requestSize() const;
   const QPixmap &pixmap() const;
   void setPixmap(const QPixmap &);

   QRect rect() const;
   int width() const;
   int height() const;
   inline operator const QPixmap &() const;

   void load(QDeclarativeEngine *, const QUrl &);
   void load(QDeclarativeEngine *, const QUrl &, QDeclarativePixmap::Options options);
   void load(QDeclarativeEngine *, const QUrl &, const QSize &);
   void load(QDeclarativeEngine *, const QUrl &, const QSize &, QDeclarativePixmap::Options options);

   void clear();
   void clear(QObject *);

   bool connectFinished(QObject *, const char *);
   bool connectFinished(QObject *, int);
   bool connectDownloadProgress(QObject *, const char *);
   bool connectDownloadProgress(QObject *, int);

   static void flushCache();

 private:
   Q_DISABLE_COPY(QDeclarativePixmap)
   QDeclarativePixmapData *d;
};

inline QDeclarativePixmap::operator const QPixmap &() const
{
   return pixmap();
}

Q_DECLARE_OPERATORS_FOR_FLAGS(QDeclarativePixmap::Options)

QT_END_NAMESPACE

#endif // QDECLARATIVEPIXMAPCACHE_H
