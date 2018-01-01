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

#ifndef QABSTRACTTEXTDOCUMENTLAYOUT_P_H
#define QABSTRACTTEXTDOCUMENTLAYOUT_P_H

#include <QtCore/qhash.h>

QT_BEGIN_NAMESPACE

struct QTextObjectHandler {
   QTextObjectHandler() : iface(0) {}
   QTextObjectInterface *iface;
   QPointer<QObject> component;
};
typedef QHash<int, QTextObjectHandler> HandlerHash;

class QAbstractTextDocumentLayoutPrivate
{

 public:
   Q_DECLARE_PUBLIC(QAbstractTextDocumentLayout)

   inline QAbstractTextDocumentLayoutPrivate()
      : paintDevice(0) {}

   virtual ~QAbstractTextDocumentLayoutPrivate() {}

   inline void setDocument(QTextDocument *doc) {
      document = doc;
      docPrivate = 0;
      if (doc) {
         docPrivate = doc->docHandle();
      }
   }

   inline int _q_dynamicPageCountSlot() const {
      return q_func()->pageCount();
   }
   inline QSizeF _q_dynamicDocumentSizeSlot() const {
      return q_func()->documentSize();
   }

   HandlerHash handlers;

   void _q_handlerDestroyed(QObject *obj);
   QPaintDevice *paintDevice;

   QTextDocument *document;
   QTextDocumentPrivate *docPrivate;

 protected:
   QAbstractTextDocumentLayout *q_ptr;

};

QT_END_NAMESPACE

#endif // QABSTRACTTEXTDOCUMENTLAYOUT_P_H
