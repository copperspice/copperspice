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

#ifndef QTEXTDOCUMENTLAYOUT_P_H
#define QTEXTDOCUMENTLAYOUT_P_H

#include <qabstracttextdocumentlayout.h>
#include <qtextoption.h>
#include <qtextobject.h>


class QTextListFormat;
class QTextTableCell;
class QTextDocumentLayoutPrivate;

class QTextDocumentLayout : public QAbstractTextDocumentLayout
{
   GUI_CS_OBJECT(QTextDocumentLayout)
   Q_DECLARE_PRIVATE(QTextDocumentLayout)

   GUI_CS_PROPERTY_READ(cursorWidth, cursorWidth)
   GUI_CS_PROPERTY_WRITE(cursorWidth, setCursorWidth)
   GUI_CS_PROPERTY_READ(idealWidth, idealWidth)
   GUI_CS_PROPERTY_READ(contentHasAlignment, contentHasAlignment)

 public:
   explicit QTextDocumentLayout(QTextDocument *doc);

   // from the abstract layout
   void draw(QPainter *painter, const PaintContext &context) override;
   int hitTest(const QPointF &point, Qt::HitTestAccuracy accuracy) const override;

   int pageCount() const override;
   QSizeF documentSize() const override;

   void setCursorWidth(int width);
   int cursorWidth() const;

   // internal, to support the ugly FixedColumnWidth wordwrap mode in QTextEdit
   void setFixedColumnWidth(int width);

   // internal for QTextEdit's NoWrap mode
   void setViewport(const QRectF &viewport);

   QRectF frameBoundingRect(QTextFrame *frame) const override;
   QRectF blockBoundingRect(const QTextBlock &block) const override;
   QRectF tableBoundingRect(QTextTable *table) const;
   QRectF tableCellBoundingRect(QTextTable *table, const QTextTableCell &cell) const;

   // ####
   int layoutStatus() const;
   int dynamicPageCount() const;
   QSizeF dynamicDocumentSize() const;
   void ensureLayouted(qreal);

   qreal idealWidth() const;

   bool contentHasAlignment() const;

 protected:
   void documentChanged(int from, int oldLength, int length) override;
   void resizeInlineObject(QTextInlineObject item, int posInDocument, const QTextFormat &format) override;
   void positionInlineObject(QTextInlineObject item, int posInDocument, const QTextFormat &format) override;

   void drawInlineObject(QPainter *p, const QRectF &rect, QTextInlineObject item,
      int posInDocument, const QTextFormat &format) override;

   void timerEvent(QTimerEvent *e) override;

 private:
   QRectF doLayout(int from, int oldLength, int length);
   void layoutFinished();
};


#endif // QTEXTDOCUMENTLAYOUT_P_H
