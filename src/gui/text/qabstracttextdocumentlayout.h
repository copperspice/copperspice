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

#ifndef QABSTRACTTEXTDOCUMENTLAYOUT_H
#define QABSTRACTTEXTDOCUMENTLAYOUT_H

#include <qobject.h>
#include <qpalette.h>
#include <qscopedpointer.h>
#include <qtextlayout.h>
#include <qtextdocument.h>
#include <qtextcursor.h>
#include <qtextblock.h>

class QAbstractTextDocumentLayoutPrivate;
class QTextObjectInterface;
class QTextFrame;

class Q_GUI_EXPORT QAbstractTextDocumentLayout : public QObject
{
   GUI_CS_OBJECT(QAbstractTextDocumentLayout)
   Q_DECLARE_PRIVATE(QAbstractTextDocumentLayout)

 public:
   explicit QAbstractTextDocumentLayout(QTextDocument *doc);
   ~QAbstractTextDocumentLayout();

   struct Selection {
      QTextCursor cursor;
      QTextCharFormat format;
   };

   struct PaintContext {
      PaintContext()
         : cursorPosition(-1) {
      }
      int cursorPosition;
      QPalette palette;
      QRectF clip;
      QVector<Selection> selections;
   };

   virtual void draw(QPainter *painter, const PaintContext &context) = 0;
   virtual int hitTest(const QPointF &point, Qt::HitTestAccuracy accuracy) const = 0;
   QString anchorAt(const QPointF &pos) const;

   virtual int pageCount() const = 0;
   virtual QSizeF documentSize() const = 0;

   virtual QRectF frameBoundingRect(QTextFrame *frame) const = 0;
   virtual QRectF blockBoundingRect(const QTextBlock &block) const = 0;

   void setPaintDevice(QPaintDevice *device);
   QPaintDevice *paintDevice() const;

   QTextDocument *document() const;

   void registerHandler(int objectType, QObject *component);
   void unregisterHandler(int objectType, QObject *component = nullptr);
   QTextObjectInterface *handlerForObject(int objectType) const;

   GUI_CS_SIGNAL_1(Public, void update(const QRectF &rect = QRectF(0.0, 0.0, 1000000000.0, 1000000000.0) ))
   GUI_CS_SIGNAL_2(update, rect)

   GUI_CS_SIGNAL_1(Public, void updateBlock(const QTextBlock &block))
   GUI_CS_SIGNAL_2(updateBlock, block)

   GUI_CS_SIGNAL_1(Public, void documentSizeChanged(const QSizeF &newSize))
   GUI_CS_SIGNAL_2(documentSizeChanged, newSize)

   GUI_CS_SIGNAL_1(Public, void pageCountChanged(int newPages))
   GUI_CS_SIGNAL_2(pageCountChanged, newPages)

 protected:
   QAbstractTextDocumentLayout(QAbstractTextDocumentLayoutPrivate &, QTextDocument *);

   virtual void documentChanged(int pos, int charsRemoved, int charsAdded) = 0;

   virtual void resizeInlineObject(QTextInlineObject item, int posInDocument, const QTextFormat &format);
   virtual void positionInlineObject(QTextInlineObject item, int posInDocument, const QTextFormat &format);
   virtual void drawInlineObject(QPainter *painter, const QRectF &rect, QTextInlineObject object, int posInDocument,
      const QTextFormat &format);

   int formatIndex(int pos);
   QTextCharFormat format(int pos);

   QScopedPointer<QAbstractTextDocumentLayoutPrivate> d_ptr;

 private:
   friend class QTextControl;
   friend class QTextDocument;
   friend class QTextDocumentPrivate;
   friend class QTextEngine;
   friend class QTextLayout;
   friend class QTextLine;

   GUI_CS_SLOT_1(Private, void _q_handlerDestroyed(QObject *obj))
   GUI_CS_SLOT_2(_q_handlerDestroyed)

   GUI_CS_SLOT_1(Private, int _q_dynamicPageCountSlot())
   GUI_CS_SLOT_2(_q_dynamicPageCountSlot)

   GUI_CS_SLOT_1(Private, QSizeF _q_dynamicDocumentSizeSlot())
   GUI_CS_SLOT_2(_q_dynamicDocumentSizeSlot)
};

class Q_GUI_EXPORT QTextObjectInterface
{
 public:
   virtual ~QTextObjectInterface();

   virtual QSizeF intrinsicSize(QTextDocument *doc, int posInDocument, const QTextFormat &format) = 0;
   virtual void drawObject(QPainter *painter, const QRectF &rect, QTextDocument *doc, int posInDocument,
      const QTextFormat &format) = 0;
};

CS_DECLARE_INTERFACE(QTextObjectInterface, "com.copperspice.QTextObjectInterface")

#endif // QABSTRACTTEXTDOCUMENTLAYOUT_H
