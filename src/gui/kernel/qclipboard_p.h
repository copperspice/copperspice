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

#ifndef QCLIPBOARD_P_H
#define QCLIPBOARD_P_H

#include <qmime.h>
#include <qclipboard.h>
#include <qstringlist.h>

QT_BEGIN_NAMESPACE

class QClipboardPrivate;

class QMimeDataWrapper : public QMimeSource
{
 public:
   QMimeDataWrapper() {}

   QString format(int n) const override;
   QByteArray encodedData(const QString &format) const override;

   mutable QStringList m_formats;
   const QMimeData *data;
};

class QMimeSourceWrapper : public QMimeData
{
 public:
   QMimeSourceWrapper(QClipboardPrivate *priv, QClipboard::Mode m);
   ~QMimeSourceWrapper();

   bool hasFormat(const QString &mimetype) const override;
   QStringList formats() const override;

 protected:
   QVariant retrieveData(const QString &mimetype, QVariant::Type) const override;

 private:
   QClipboardPrivate *d;
   QClipboard::Mode mode;
   QMimeSource *source;
};


class QClipboardPrivate
{

 public:
   QClipboardPrivate() {
      for (int i = 0; i <= QClipboard::LastMode; ++i) {
         compat_data[i] = 0;
         wrapper[i] = new QMimeDataWrapper();
      }
   }

   virtual ~QClipboardPrivate() {
      for (int i = 0; i <= QClipboard::LastMode; ++i) {
         delete wrapper[i];
         delete compat_data[i];
      }
   }

   mutable QMimeDataWrapper *wrapper[QClipboard::LastMode + 1];
   mutable QMimeSource *compat_data[QClipboard::LastMode + 1];
};

inline QMimeSourceWrapper::QMimeSourceWrapper(QClipboardPrivate *priv, QClipboard::Mode m)
   : QMimeData()
{
   d = priv;
   mode = m;
   source = d->compat_data[mode];
}

inline QMimeSourceWrapper::~QMimeSourceWrapper()
{
   if (d->compat_data[mode] == source) {
      d->compat_data[mode] = 0;
   }
   delete source;
}

QT_END_NAMESPACE

#endif // QCLIPBOARD_P_H
