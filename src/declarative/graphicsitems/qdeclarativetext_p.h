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

#ifndef QDECLARATIVETEXT_P_H
#define QDECLARATIVETEXT_P_H

#include <QtGui/qtextoption.h>
#include <qdeclarativeimplicitsizeitem_p.h>
#include <qdeclarativeglobal_p.h>

QT_BEGIN_NAMESPACE

class QDeclarativeTextPrivate;

class Q_DECLARATIVE_PRIVATE_EXPORT QDeclarativeText : public QDeclarativeImplicitSizeItem
{
   DECL_CS_OBJECT(QDeclarativeText)

   DECL_CS_ENUM(HAlignment)
   DECL_CS_ENUM(VAlignment)
   DECL_CS_ENUM(TextStyle)
   DECL_CS_ENUM(TextFormat)
   DECL_CS_ENUM(TextElideMode)
   DECL_CS_ENUM(WrapMode)
   DECL_CS_ENUM(LineHeightMode)

   DECL_CS_PROPERTY_READ(text, text)
   DECL_CS_PROPERTY_WRITE(text, setText)
   DECL_CS_PROPERTY_NOTIFY(text, textChanged)
   DECL_CS_PROPERTY_READ(font, font)
   DECL_CS_PROPERTY_WRITE(font, setFont)
   DECL_CS_PROPERTY_NOTIFY(font, fontChanged)
   DECL_CS_PROPERTY_READ(color, color)
   DECL_CS_PROPERTY_WRITE(color, setColor)
   DECL_CS_PROPERTY_NOTIFY(color, colorChanged)
   DECL_CS_PROPERTY_READ(style, style)
   DECL_CS_PROPERTY_WRITE(style, setStyle)
   DECL_CS_PROPERTY_NOTIFY(style, styleChanged)
   DECL_CS_PROPERTY_READ(styleColor, styleColor)
   DECL_CS_PROPERTY_WRITE(styleColor, setStyleColor)
   DECL_CS_PROPERTY_NOTIFY(styleColor, styleColorChanged)
   DECL_CS_PROPERTY_READ(horizontalAlignment, hAlign)
   DECL_CS_PROPERTY_WRITE(horizontalAlignment, setHAlign)
   DECL_CS_PROPERTY_RESET(horizontalAlignment, resetHAlign)
   DECL_CS_PROPERTY_NOTIFY(horizontalAlignment, horizontalAlignmentChanged)
   DECL_CS_PROPERTY_READ(verticalAlignment, vAlign)
   DECL_CS_PROPERTY_WRITE(verticalAlignment, setVAlign)
   DECL_CS_PROPERTY_NOTIFY(verticalAlignment, verticalAlignmentChanged)
   DECL_CS_PROPERTY_READ(wrapMode, wrapMode)
   DECL_CS_PROPERTY_WRITE(wrapMode, setWrapMode)
   DECL_CS_PROPERTY_NOTIFY(wrapMode, wrapModeChanged)
   DECL_CS_PROPERTY_READ(lineCount, lineCount)
   DECL_CS_PROPERTY_NOTIFY(lineCount, lineCountChanged)
   DECL_CS_PROPERTY_REVISION(lineCount, 1)
   DECL_CS_PROPERTY_READ(truncated, truncated)
   DECL_CS_PROPERTY_NOTIFY(truncated, truncatedChanged)
   DECL_CS_PROPERTY_REVISION(truncated, 1)
   DECL_CS_PROPERTY_READ(maximumLineCount, maximumLineCount)
   DECL_CS_PROPERTY_WRITE(maximumLineCount, setMaximumLineCount)
   DECL_CS_PROPERTY_NOTIFY(maximumLineCount, maximumLineCountChanged)
   DECL_CS_PROPERTY_RESET(maximumLineCount, resetMaximumLineCount)
   DECL_CS_PROPERTY_REVISION(maximumLineCount, 1)

   DECL_CS_PROPERTY_READ(textFormat, textFormat)
   DECL_CS_PROPERTY_WRITE(textFormat, setTextFormat)
   DECL_CS_PROPERTY_NOTIFY(textFormat, textFormatChanged)
   DECL_CS_PROPERTY_READ(elide, elideMode)
   DECL_CS_PROPERTY_WRITE(elide, setElideMode)
   DECL_CS_PROPERTY_NOTIFY(elide, elideModeChanged)//### elideMode?
   DECL_CS_PROPERTY_READ(paintedWidth, paintedWidth)
   DECL_CS_PROPERTY_NOTIFY(paintedWidth, paintedSizeChanged)
   DECL_CS_PROPERTY_READ(paintedHeight, paintedHeight)
   DECL_CS_PROPERTY_NOTIFY(paintedHeight, paintedSizeChanged)
   DECL_CS_PROPERTY_READ(lineHeight, lineHeight)
   DECL_CS_PROPERTY_WRITE(lineHeight, setLineHeight)
   DECL_CS_PROPERTY_NOTIFY(lineHeight, lineHeightChanged)
   DECL_CS_PROPERTY_REVISION(lineHeight, 1)
   DECL_CS_PROPERTY_READ(lineHeightMode, lineHeightMode)
   DECL_CS_PROPERTY_WRITE(lineHeightMode, setLineHeightMode)
   DECL_CS_PROPERTY_NOTIFY(lineHeightMode, lineHeightModeChanged)
   DECL_CS_PROPERTY_REVISION(lineHeightMode, 1)

 public:
   QDeclarativeText(QDeclarativeItem *parent = 0);
   ~QDeclarativeText();

   enum HAlignment { AlignLeft = Qt::AlignLeft,
                     AlignRight = Qt::AlignRight,
                     AlignHCenter = Qt::AlignHCenter,
                     AlignJustify = Qt::AlignJustify
                   }; // ### VERSIONING: Only in QtQuick 1.1
   enum VAlignment { AlignTop = Qt::AlignTop,
                     AlignBottom = Qt::AlignBottom,
                     AlignVCenter = Qt::AlignVCenter
                   };
   enum TextStyle { Normal,
                    Outline,
                    Raised,
                    Sunken
                  };
   enum TextFormat { PlainText = Qt::PlainText,
                     RichText = Qt::RichText,
                     AutoText = Qt::AutoText,
                     StyledText = 4
                   };
   enum TextElideMode { ElideLeft = Qt::ElideLeft,
                        ElideRight = Qt::ElideRight,
                        ElideMiddle = Qt::ElideMiddle,
                        ElideNone = Qt::ElideNone
                      };

   enum WrapMode { NoWrap = QTextOption::NoWrap,
                   WordWrap = QTextOption::WordWrap,
                   WrapAnywhere = QTextOption::WrapAnywhere,
                   WrapAtWordBoundaryOrAnywhere = QTextOption::WrapAtWordBoundaryOrAnywhere, // COMPAT
                   Wrap = QTextOption::WrapAtWordBoundaryOrAnywhere
                 };

   enum LineHeightMode { ProportionalHeight, FixedHeight };

   QString text() const;
   void setText(const QString &);

   QFont font() const;
   void setFont(const QFont &font);

   QColor color() const;
   void setColor(const QColor &c);

   TextStyle style() const;
   void setStyle(TextStyle style);

   QColor styleColor() const;
   void setStyleColor(const QColor &c);

   HAlignment hAlign() const;
   void setHAlign(HAlignment align);
   void resetHAlign();
   HAlignment effectiveHAlign() const;

   VAlignment vAlign() const;
   void setVAlign(VAlignment align);

   WrapMode wrapMode() const;
   void setWrapMode(WrapMode w);

   int lineCount() const;
   bool truncated() const;

   int maximumLineCount() const;
   void setMaximumLineCount(int lines);
   void resetMaximumLineCount();

   TextFormat textFormat() const;
   void setTextFormat(TextFormat format);

   TextElideMode elideMode() const;
   void setElideMode(TextElideMode);

   qreal lineHeight() const;
   void setLineHeight(qreal lineHeight);

   LineHeightMode lineHeightMode() const;
   void setLineHeightMode(LineHeightMode);

   void paint(QPainter *, const QStyleOptionGraphicsItem *, QWidget *);

   virtual void componentComplete();

   int resourcesLoading() const; // mainly for testing

   qreal paintedWidth() const;
   qreal paintedHeight() const;

   QRectF boundingRect() const;

   DECL_CS_SIGNAL_1(Public, void textChanged(const QString &text))
   DECL_CS_SIGNAL_2(textChanged, text)
   DECL_CS_SIGNAL_1(Public, void linkActivated(const QString &link))
   DECL_CS_SIGNAL_2(linkActivated, link)
   DECL_CS_SIGNAL_1(Public, void fontChanged(const QFont &font))
   DECL_CS_SIGNAL_2(fontChanged, font)
   DECL_CS_SIGNAL_1(Public, void colorChanged(const QColor &color))
   DECL_CS_SIGNAL_2(colorChanged, color)
   DECL_CS_SIGNAL_1(Public, void styleChanged(TextStyle style))
   DECL_CS_SIGNAL_2(styleChanged, style)
   DECL_CS_SIGNAL_1(Public, void styleColorChanged(const QColor &color))
   DECL_CS_SIGNAL_2(styleColorChanged, color)
   DECL_CS_SIGNAL_1(Public, void horizontalAlignmentChanged(HAlignment alignment))
   DECL_CS_SIGNAL_2(horizontalAlignmentChanged, alignment)
   DECL_CS_SIGNAL_1(Public, void verticalAlignmentChanged(VAlignment alignment))
   DECL_CS_SIGNAL_2(verticalAlignmentChanged, alignment)
   DECL_CS_SIGNAL_1(Public, void wrapModeChanged())
   DECL_CS_SIGNAL_2(wrapModeChanged)

   DECL_CS_SIGNAL_1(Public, void lineCountChanged())
   DECL_CS_SIGNAL_2(lineCountChanged)
   DECL_CS_REVISION(lineCountChanged, 1)

   DECL_CS_SIGNAL_1(Public, void truncatedChanged())
   DECL_CS_SIGNAL_2(truncatedChanged)
   DECL_CS_REVISION(truncatedChanged, 1)

   DECL_CS_SIGNAL_1(Public, void maximumLineCountChanged())
   DECL_CS_SIGNAL_2(maximumLineCountChanged)
   DECL_CS_REVISION(maximumLineCountChanged, 1)

   DECL_CS_SIGNAL_1(Public, void textFormatChanged(TextFormat textFormat))
   DECL_CS_SIGNAL_2(textFormatChanged, textFormat)
   DECL_CS_SIGNAL_1(Public, void elideModeChanged(TextElideMode mode))
   DECL_CS_SIGNAL_2(elideModeChanged, mode)
   DECL_CS_SIGNAL_1(Public, void paintedSizeChanged())
   DECL_CS_SIGNAL_2(paintedSizeChanged)

   DECL_CS_SIGNAL_1(Public, void lineHeightChanged(qreal lineHeight))
   DECL_CS_SIGNAL_2(lineHeightChanged, lineHeight)
   DECL_CS_REVISION(lineHeightChanged, 1)

   DECL_CS_SIGNAL_1(Public, void lineHeightModeChanged(LineHeightMode mode))
   DECL_CS_SIGNAL_2(lineHeightModeChanged, mode)
   DECL_CS_REVISION(lineHeightModeChanged, 1)

 protected:
   void mousePressEvent(QGraphicsSceneMouseEvent *event);
   void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
   virtual void geometryChanged(const QRectF &newGeometry, const QRectF &oldGeometry);

 private:
   Q_DISABLE_COPY(QDeclarativeText)
   Q_DECLARE_PRIVATE_D(QGraphicsItem::d_ptr.data(), QDeclarativeText)
};

QT_END_NAMESPACE

QML_DECLARE_TYPE(QDeclarativeText)


#endif
