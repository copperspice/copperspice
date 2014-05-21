/***********************************************************************
*
* Copyright (c) 2012-2014 Barbara Geller
* Copyright (c) 2012-2014 Ansel Sermersheim
* Copyright (c) 2012-2014 Digia Plc and/or its subsidiary(-ies).
* Copyright (c) 2008-2012 Nokia Corporation and/or its subsidiary(-ies).
* All rights reserved.
*
* This file is part of CopperSpice.
*
* CopperSpice is free software: you can redistribute it and/or 
* modify it under the terms of the GNU Lesser General Public License
* version 2.1 as published by the Free Software Foundation.
*
* CopperSpice is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
* Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public
* License along with CopperSpice.  If not, see 
* <http://www.gnu.org/licenses/>.
*
***********************************************************************/

#ifndef QTEXTDOCUMENTLAYOUT_P_H
#define QTEXTDOCUMENTLAYOUT_P_H

#include "QtGui/qabstracttextdocumentlayout.h"
#include "QtGui/qtextoption.h"
#include "QtGui/qtextobject.h"

QT_BEGIN_NAMESPACE

class QTextListFormat;

class QTextDocumentLayoutPrivate;

class QTextDocumentLayout : public QAbstractTextDocumentLayout
{
    CS_OBJECT(QTextDocumentLayout)
    Q_DECLARE_PRIVATE(QTextDocumentLayout)
    
    GUI_CS_PROPERTY_READ(cursorWidth, cursorWidth)
    GUI_CS_PROPERTY_WRITE(cursorWidth, setCursorWidth)
    GUI_CS_PROPERTY_READ(idealWidth, idealWidth)
    GUI_CS_PROPERTY_READ(contentHasAlignment, contentHasAlignment)

public:
    explicit QTextDocumentLayout(QTextDocument *doc);

    // from the abstract layout
    void draw(QPainter *painter, const PaintContext &context);
    int hitTest(const QPointF &point, Qt::HitTestAccuracy accuracy) const;

    int pageCount() const;
    QSizeF documentSize() const;

    void setCursorWidth(int width);
    int cursorWidth() const;

    // internal, to support the ugly FixedColumnWidth wordwrap mode in QTextEdit
    void setFixedColumnWidth(int width);

    // internal for QTextEdit's NoWrap mode
    void setViewport(const QRectF &viewport);

    virtual QRectF frameBoundingRect(QTextFrame *frame) const;
    virtual QRectF blockBoundingRect(const QTextBlock &block) const;

    // ####
    int layoutStatus() const;
    int dynamicPageCount() const;
    QSizeF dynamicDocumentSize() const;
    void ensureLayouted(qreal);

    qreal idealWidth() const;

    bool contentHasAlignment() const;

protected:
    void documentChanged(int from, int oldLength, int length);
    void resizeInlineObject(QTextInlineObject item, int posInDocument, const QTextFormat &format);
    void positionInlineObject(QTextInlineObject item, int posInDocument, const QTextFormat &format);
    void drawInlineObject(QPainter *p, const QRectF &rect, QTextInlineObject item,
                          int posInDocument, const QTextFormat &format);
    virtual void timerEvent(QTimerEvent *e);
private:
    QRectF doLayout(int from, int oldLength, int length);
    void layoutFinished();
};

QT_END_NAMESPACE

#endif // QTEXTDOCUMENTLAYOUT_P_H
