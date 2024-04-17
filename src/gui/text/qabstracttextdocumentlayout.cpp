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

#include <qabstracttextdocumentlayout.h>
#include <qtextformat.h>

#include <qtextdocument_p.h>
#include <qtextengine_p.h>
#include <qabstracttextdocumentlayout_p.h>

QAbstractTextDocumentLayoutPrivate::~QAbstractTextDocumentLayoutPrivate()
{
}

QTextObjectInterface::~QTextObjectInterface()
{
}

QAbstractTextDocumentLayout::QAbstractTextDocumentLayout(QTextDocument *document)
   : QObject(document), d_ptr(new QAbstractTextDocumentLayoutPrivate)
{
   d_ptr->q_ptr = this;
   Q_D(QAbstractTextDocumentLayout);

   d->setDocument(document);
}

QAbstractTextDocumentLayout::QAbstractTextDocumentLayout(QAbstractTextDocumentLayoutPrivate &dd,
   QTextDocument *document)
   : QObject(document), d_ptr(&dd)
{
   d_ptr->q_ptr = this;
   Q_D(QAbstractTextDocumentLayout);

   d->setDocument(document);
}

QAbstractTextDocumentLayout::~QAbstractTextDocumentLayout()
{
}

void QAbstractTextDocumentLayout::registerHandler(int objectType, QObject *component)
{
   Q_D(QAbstractTextDocumentLayout);

   QTextObjectInterface *iface = qobject_cast<QTextObjectInterface *>(component);
   if (!iface) {
      return;   // ### print error message on terminal?
   }

   connect(component, &QObject::destroyed, this, &QAbstractTextDocumentLayout::_q_handlerDestroyed);

   QTextObjectHandler h;
   h.iface = iface;
   h.component = component;
   d->handlers.insert(objectType, h);
}

void QAbstractTextDocumentLayout::unregisterHandler(int objectType, QObject *component)
{
   Q_D(QAbstractTextDocumentLayout);

   HandlerHash::iterator it = d->handlers.find(objectType);

   if (it != d->handlers.end() && (!component || component == it->component)) {
      if (component) {
         disconnect(component, &QObject::destroyed, this, &QAbstractTextDocumentLayout::_q_handlerDestroyed);
      }

      d->handlers.erase(it);
   }
}

QTextObjectInterface *QAbstractTextDocumentLayout::handlerForObject(int objectType) const
{
   Q_D(const QAbstractTextDocumentLayout);

   QTextObjectHandler handler = d->handlers.value(objectType);
   if (!handler.component) {
      return nullptr;
   }

   return handler.iface;
}

void QAbstractTextDocumentLayout::resizeInlineObject(QTextInlineObject item, int posInDocument,
   const QTextFormat &format)
{
   Q_D(QAbstractTextDocumentLayout);

   QTextCharFormat f = format.toCharFormat();
   Q_ASSERT(f.isValid());
   QTextObjectHandler handler = d->handlers.value(f.objectType());

   if (!handler.component) {
      return;
   }

   QSizeF s = handler.iface->intrinsicSize(document(), posInDocument, format);
   item.setWidth(s.width());
   item.setAscent(s.height());
   item.setDescent(0);
}

void QAbstractTextDocumentLayout::positionInlineObject(QTextInlineObject item, int posInDocument,
   const QTextFormat &format)
{
   (void) item;
   (void) posInDocument;
   (void) format;
}

void QAbstractTextDocumentLayout::drawInlineObject(QPainter *p, const QRectF &rect, QTextInlineObject item,
   int posInDocument, const QTextFormat &format)
{
   (void) item;

   Q_D(QAbstractTextDocumentLayout);

   QTextCharFormat f = format.toCharFormat();
   Q_ASSERT(f.isValid());
   QTextObjectHandler handler = d->handlers.value(f.objectType());
   if (!handler.component) {
      return;
   }

   handler.iface->drawObject(p, rect, document(), posInDocument, format);
}

void QAbstractTextDocumentLayoutPrivate::_q_handlerDestroyed(QObject *obj)
{
   HandlerHash::iterator it = handlers.begin();

   while (it != handlers.end())
      if ((*it).component == obj) {
         it = handlers.erase(it);
      } else {
         ++it;
      }
}

int QAbstractTextDocumentLayout::formatIndex(int pos)
{
   QTextDocumentPrivate *pieceTable = qobject_cast<QTextDocument *>(parent())->docHandle();
   return pieceTable->find(pos).value()->format;
}

QTextCharFormat QAbstractTextDocumentLayout::format(int pos)
{
   QTextDocumentPrivate *pieceTable = qobject_cast<QTextDocument *>(parent())->docHandle();
   int idx = pieceTable->find(pos).value()->format;
   return pieceTable->formatCollection()->charFormat(idx);
}

QTextDocument *QAbstractTextDocumentLayout::document() const
{
   Q_D(const QAbstractTextDocumentLayout);
   return d->document;
}

QString QAbstractTextDocumentLayout::anchorAt(const QPointF &pos) const
{
   int cursorPos = hitTest(pos, Qt::ExactHit);
   if (cursorPos == -1) {
      return QString();
   }

   // compensate for preedit in the hit text block
   QTextBlock block = document()->firstBlock();
   while (block.isValid()) {
      QRectF blockBr = blockBoundingRect(block);

      if (blockBr.contains(pos)) {
         QTextLayout *layout = block.layout();
         int relativeCursorPos = cursorPos - block.position();
         const int preeditLength = layout ? layout->preeditAreaText().length() : 0;

         if (preeditLength > 0 && relativeCursorPos > layout->preeditAreaPosition()) {
            cursorPos -= qMin(cursorPos - layout->preeditAreaPosition(), preeditLength);
         }
         break;
      }
      block = block.next();
   }

   QTextDocumentPrivate *pieceTable = qobject_cast<const QTextDocument *>(parent())->docHandle();
   QTextDocumentPrivate::FragmentIterator it = pieceTable->find(cursorPos);
   QTextCharFormat fmt = pieceTable->formatCollection()->charFormat(it->format);

   return fmt.anchorHref();
}

void QAbstractTextDocumentLayout::setPaintDevice(QPaintDevice *device)
{
   Q_D(QAbstractTextDocumentLayout);
   d->paintDevice = device;
}

QPaintDevice *QAbstractTextDocumentLayout::paintDevice() const
{
   Q_D(const QAbstractTextDocumentLayout);
   return d->paintDevice;
}

void QAbstractTextDocumentLayout::_q_handlerDestroyed(QObject *obj)
{
   Q_D(QAbstractTextDocumentLayout);
   d->_q_handlerDestroyed(obj);
}

int QAbstractTextDocumentLayout::_q_dynamicPageCountSlot()
{
   Q_D(QAbstractTextDocumentLayout);
   return d->_q_dynamicPageCountSlot();
}

QSizeF QAbstractTextDocumentLayout::_q_dynamicDocumentSizeSlot()
{
   Q_D(QAbstractTextDocumentLayout);
   return d->_q_dynamicDocumentSizeSlot();
}
