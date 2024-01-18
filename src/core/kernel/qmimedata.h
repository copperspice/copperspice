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

#ifndef QMIMEDATA_H
#define QMIMEDATA_H

#include <qvariant.h>
#include <qobject.h>
#include <qscopedpointer.h>

class QUrl;
class QMimeDataPrivate;

class Q_CORE_EXPORT QMimeData : public QObject
{
   CORE_CS_OBJECT(QMimeData)

 public:
   QMimeData();

   QMimeData(const QMimeData &) = delete;
   QMimeData &operator=(const QMimeData &) = delete;

   ~QMimeData();

   QList<QUrl> urls() const;
   void setUrls(const QList<QUrl> &urls);
   bool hasUrls() const;

   QString text() const;
   void setText(const QString &text);
   bool hasText() const;

   QString html() const;
   void setHtml(const QString &html);
   bool hasHtml() const;

   QVariant imageData() const;
   void setImageData(const QVariant &image);
   bool hasImage() const;

   QVariant colorData() const;
   void setColorData(const QVariant &color);
   bool hasColor() const;

   QByteArray data(const QString &mimeType) const;
   void setData(const QString &mimeType, const QByteArray &data);
   void removeFormat(const QString &mimeType);

   virtual bool hasFormat(const QString &mimeType) const;
   virtual QStringList formats() const;

   void clear();

 protected:
   virtual QVariant retrieveData(const QString &mimeType, QVariant::Type preferredType) const;
   QScopedPointer<QMimeDataPrivate> d_ptr;

 private:
   Q_DECLARE_PRIVATE(QMimeData)
};

#endif
