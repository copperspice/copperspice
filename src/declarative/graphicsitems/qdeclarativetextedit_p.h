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

#ifndef QDECLARATIVETEXTEDIT_P_H
#define QDECLARATIVETEXTEDIT_P_H

#include <qdeclarativetext_p.h>
#include <qdeclarativeimplicitsizeitem_p.h>
#include <QtGui/qtextdocument.h>
#include <QtGui/qtextoption.h>
#include <QtGui/qtextcursor.h>
#include <QtGui/qtextformat.h>

QT_BEGIN_NAMESPACE

class QDeclarativeTextEditPrivate;

class QDeclarativeTextEdit : public QDeclarativeImplicitSizePaintedItem
{
   DECL_CS_OBJECT(QDeclarativeTextEdit)

   CS_ENUM(VAlignment)
   CS_ENUM(HAlignment)
   CS_ENUM(TextFormat)
   CS_ENUM(WrapMode)
   CS_ENUM(SelectionMode)

   DECL_CS_PROPERTY_READ(text, text)
   DECL_CS_PROPERTY_WRITE(text, setText)
   DECL_CS_PROPERTY_NOTIFY(text, textChanged)
   DECL_CS_PROPERTY_READ(color, color)
   DECL_CS_PROPERTY_WRITE(color, setColor)
   DECL_CS_PROPERTY_NOTIFY(color, colorChanged)
   DECL_CS_PROPERTY_READ(selectionColor, selectionColor)
   DECL_CS_PROPERTY_WRITE(selectionColor, setSelectionColor)
   DECL_CS_PROPERTY_NOTIFY(selectionColor, selectionColorChanged)
   DECL_CS_PROPERTY_READ(selectedTextColor, selectedTextColor)
   DECL_CS_PROPERTY_WRITE(selectedTextColor, setSelectedTextColor)
   DECL_CS_PROPERTY_NOTIFY(selectedTextColor, selectedTextColorChanged)
   DECL_CS_PROPERTY_READ(font, font)
   DECL_CS_PROPERTY_WRITE(font, setFont)
   DECL_CS_PROPERTY_NOTIFY(font, fontChanged)
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
   DECL_CS_PROPERTY_READ(paintedWidth, paintedWidth)
   DECL_CS_PROPERTY_NOTIFY(paintedWidth, paintedSizeChanged)
   DECL_CS_PROPERTY_READ(paintedHeight, paintedHeight)
   DECL_CS_PROPERTY_NOTIFY(paintedHeight, paintedSizeChanged)
   DECL_CS_PROPERTY_READ(textFormat, textFormat)
   DECL_CS_PROPERTY_WRITE(textFormat, setTextFormat)
   DECL_CS_PROPERTY_NOTIFY(textFormat, textFormatChanged)
   DECL_CS_PROPERTY_READ(readOnly, isReadOnly)
   DECL_CS_PROPERTY_WRITE(readOnly, setReadOnly)
   DECL_CS_PROPERTY_NOTIFY(readOnly, readOnlyChanged)
   DECL_CS_PROPERTY_READ(cursorVisible, isCursorVisible)
   DECL_CS_PROPERTY_WRITE(cursorVisible, setCursorVisible)
   DECL_CS_PROPERTY_NOTIFY(cursorVisible, cursorVisibleChanged)
   DECL_CS_PROPERTY_READ(cursorPosition, cursorPosition)
   DECL_CS_PROPERTY_WRITE(cursorPosition, setCursorPosition)
   DECL_CS_PROPERTY_NOTIFY(cursorPosition, cursorPositionChanged)
   DECL_CS_PROPERTY_READ(cursorRectangle, cursorRectangle)
   DECL_CS_PROPERTY_NOTIFY(cursorRectangle, cursorRectangleChanged)
   DECL_CS_PROPERTY_READ(cursorDelegate, cursorDelegate)
   DECL_CS_PROPERTY_WRITE(cursorDelegate, setCursorDelegate)
   DECL_CS_PROPERTY_NOTIFY(cursorDelegate, cursorDelegateChanged)
   DECL_CS_PROPERTY_READ(selectionStart, selectionStart)
   DECL_CS_PROPERTY_NOTIFY(selectionStart, selectionStartChanged)
   DECL_CS_PROPERTY_READ(selectionEnd, selectionEnd)
   DECL_CS_PROPERTY_NOTIFY(selectionEnd, selectionEndChanged)
   DECL_CS_PROPERTY_READ(selectedText, selectedText)
   DECL_CS_PROPERTY_NOTIFY(selectedText, selectionChanged)
   DECL_CS_PROPERTY_READ(activeFocusOnPress, focusOnPress)
   DECL_CS_PROPERTY_WRITE(activeFocusOnPress, setFocusOnPress)
   DECL_CS_PROPERTY_NOTIFY(activeFocusOnPress, activeFocusOnPressChanged)
   DECL_CS_PROPERTY_READ(persistentSelection, persistentSelection)
   DECL_CS_PROPERTY_WRITE(persistentSelection, setPersistentSelection)
   DECL_CS_PROPERTY_NOTIFY(persistentSelection, persistentSelectionChanged)
   DECL_CS_PROPERTY_READ(textMargin, textMargin)
   DECL_CS_PROPERTY_WRITE(textMargin, setTextMargin)
   DECL_CS_PROPERTY_NOTIFY(textMargin, textMarginChanged)
   DECL_CS_PROPERTY_READ(inputMethodHints, inputMethodHints)
   DECL_CS_PROPERTY_WRITE(inputMethodHints, setInputMethodHints)
   DECL_CS_PROPERTY_READ(selectByMouse, selectByMouse)
   DECL_CS_PROPERTY_WRITE(selectByMouse, setSelectByMouse)
   DECL_CS_PROPERTY_NOTIFY(selectByMouse, selectByMouseChanged)
   DECL_CS_PROPERTY_READ(mouseSelectionMode, mouseSelectionMode)
   DECL_CS_PROPERTY_WRITE(mouseSelectionMode, setMouseSelectionMode)
   DECL_CS_PROPERTY_NOTIFY(mouseSelectionMode, mouseSelectionModeChanged)
   DECL_CS_PROPERTY_REVISION(mouseSelectionMode, 1)
   DECL_CS_PROPERTY_READ(canPaste, canPaste)
   DECL_CS_PROPERTY_NOTIFY(canPaste, canPasteChanged)
   DECL_CS_PROPERTY_REVISION(canPaste, 1)
   DECL_CS_PROPERTY_READ(inputMethodComposing, isInputMethodComposing)
   DECL_CS_PROPERTY_NOTIFY(inputMethodComposing, inputMethodComposingChanged)
   DECL_CS_PROPERTY_REVISION(inputMethodComposing, 1)

 public:
   QDeclarativeTextEdit(QDeclarativeItem *parent = 0);

   enum HAlignment {
      AlignLeft = Qt::AlignLeft,
      AlignRight = Qt::AlignRight,
      AlignHCenter = Qt::AlignHCenter,
      AlignJustify = Qt::AlignJustify // ### VERSIONING: Only in QtQuick 1.1
   };

   enum VAlignment {
      AlignTop = Qt::AlignTop,
      AlignBottom = Qt::AlignBottom,
      AlignVCenter = Qt::AlignVCenter
   };

   enum TextFormat {
      PlainText = Qt::PlainText,
      RichText = Qt::RichText,
      AutoText = Qt::AutoText
   };

   enum WrapMode { NoWrap = QTextOption::NoWrap,
                   WordWrap = QTextOption::WordWrap,
                   WrapAnywhere = QTextOption::WrapAnywhere,
                   WrapAtWordBoundaryOrAnywhere = QTextOption::WrapAtWordBoundaryOrAnywhere, // COMPAT
                   Wrap = QTextOption::WrapAtWordBoundaryOrAnywhere
                 };

   enum SelectionMode {
      SelectCharacters,
      SelectWords
   };

   CS_INVOKABLE_METHOD_1(Public, void openSoftwareInputPanel())
   CS_INVOKABLE_METHOD_2(openSoftwareInputPanel)

   CS_INVOKABLE_METHOD_1(Public, void closeSoftwareInputPanel())
   CS_INVOKABLE_METHOD_2(closeSoftwareInputPanel)

   CS_INVOKABLE_METHOD_1(Public, QRectF positionToRectangle(int) const)
   CS_INVOKABLE_METHOD_2(positionToRectangle)

   CS_INVOKABLE_METHOD_1(Public, int positionAt(int x, int y) const)
   CS_INVOKABLE_METHOD_2(positionAt)

   CS_INVOKABLE_METHOD_1(Public, void moveCursorSelection(int pos))
   CS_INVOKABLE_METHOD_2(moveCursorSelection)

   CS_INVOKABLE_METHOD_1(Public, void moveCursorSelection(int pos, SelectionMode mode) )
   CS_INVOKABLE_METHOD_2(moveCursorSelection)
   CS_REVISION(moveCursorSelection, 1)

   QString text() const;
   void setText(const QString &);

   TextFormat textFormat() const;
   void setTextFormat(TextFormat format);

   QFont font() const;
   void setFont(const QFont &font);

   QColor color() const;
   void setColor(const QColor &c);

   QColor selectionColor() const;
   void setSelectionColor(const QColor &c);

   QColor selectedTextColor() const;
   void setSelectedTextColor(const QColor &c);

   HAlignment hAlign() const;
   void setHAlign(HAlignment align);
   void resetHAlign();
   HAlignment effectiveHAlign() const;

   VAlignment vAlign() const;
   void setVAlign(VAlignment align);

   WrapMode wrapMode() const;
   void setWrapMode(WrapMode w);

   int lineCount() const;

   bool isCursorVisible() const;
   void setCursorVisible(bool on);

   int cursorPosition() const;
   void setCursorPosition(int pos);

   QDeclarativeComponent *cursorDelegate() const;
   void setCursorDelegate(QDeclarativeComponent *);

   int selectionStart() const;
   int selectionEnd() const;

   QString selectedText() const;

   bool focusOnPress() const;
   void setFocusOnPress(bool on);

   bool persistentSelection() const;
   void setPersistentSelection(bool on);

   qreal textMargin() const;
   void setTextMargin(qreal margin);

   bool selectByMouse() const;
   void setSelectByMouse(bool);

   SelectionMode mouseSelectionMode() const;
   void setMouseSelectionMode(SelectionMode mode);

   bool canPaste() const;

   virtual void componentComplete();

   void setReadOnly(bool);
   bool isReadOnly() const;

   void setTextInteractionFlags(Qt::TextInteractionFlags flags);
   Qt::TextInteractionFlags textInteractionFlags() const;

   QRect cursorRectangle() const;

   QVariant inputMethodQuery(Qt::InputMethodQuery property) const;

   qreal paintedWidth() const;
   qreal paintedHeight() const;


   QRectF boundingRect() const;

   bool isInputMethodComposing() const;

   DECL_CS_SIGNAL_1(Public, void textChanged(const QString &un_named_arg1))
   DECL_CS_SIGNAL_2(textChanged, un_named_arg1)
   DECL_CS_SIGNAL_1(Public, void paintedSizeChanged())
   DECL_CS_SIGNAL_2(paintedSizeChanged)
   DECL_CS_SIGNAL_1(Public, void cursorPositionChanged())
   DECL_CS_SIGNAL_2(cursorPositionChanged)
   DECL_CS_SIGNAL_1(Public, void cursorRectangleChanged())
   DECL_CS_SIGNAL_2(cursorRectangleChanged)
   DECL_CS_SIGNAL_1(Public, void selectionStartChanged())
   DECL_CS_SIGNAL_2(selectionStartChanged)
   DECL_CS_SIGNAL_1(Public, void selectionEndChanged())
   DECL_CS_SIGNAL_2(selectionEndChanged)
   DECL_CS_SIGNAL_1(Public, void selectionChanged())
   DECL_CS_SIGNAL_2(selectionChanged)
   DECL_CS_SIGNAL_1(Public, void colorChanged(const QColor &color))
   DECL_CS_SIGNAL_2(colorChanged, color)
   DECL_CS_SIGNAL_1(Public, void selectionColorChanged(const QColor &color))
   DECL_CS_SIGNAL_2(selectionColorChanged, color)
   DECL_CS_SIGNAL_1(Public, void selectedTextColorChanged(const QColor &color))
   DECL_CS_SIGNAL_2(selectedTextColorChanged, color)
   DECL_CS_SIGNAL_1(Public, void fontChanged(const QFont &font))
   DECL_CS_SIGNAL_2(fontChanged, font)
   DECL_CS_SIGNAL_1(Public, void horizontalAlignmentChanged(HAlignment alignment))
   DECL_CS_SIGNAL_2(horizontalAlignmentChanged, alignment)
   DECL_CS_SIGNAL_1(Public, void verticalAlignmentChanged(VAlignment alignment))
   DECL_CS_SIGNAL_2(verticalAlignmentChanged, alignment)
   DECL_CS_SIGNAL_1(Public, void wrapModeChanged())
   DECL_CS_SIGNAL_2(wrapModeChanged)
   DECL_CS_SIGNAL_1(Public, void lineCountChanged())
   DECL_CS_SIGNAL_2(lineCountChanged)
   DECL_CS_SIGNAL_1(Public, void textFormatChanged(TextFormat textFormat))
   DECL_CS_SIGNAL_2(textFormatChanged, textFormat)
   DECL_CS_SIGNAL_1(Public, void readOnlyChanged(bool isReadOnly))
   DECL_CS_SIGNAL_2(readOnlyChanged, isReadOnly)
   DECL_CS_SIGNAL_1(Public, void cursorVisibleChanged(bool isCursorVisible))
   DECL_CS_SIGNAL_2(cursorVisibleChanged, isCursorVisible)
   DECL_CS_SIGNAL_1(Public, void cursorDelegateChanged())
   DECL_CS_SIGNAL_2(cursorDelegateChanged)
   DECL_CS_SIGNAL_1(Public, void activeFocusOnPressChanged(bool activeFocusOnPressed))
   DECL_CS_SIGNAL_2(activeFocusOnPressChanged, activeFocusOnPressed)
   DECL_CS_SIGNAL_1(Public, void persistentSelectionChanged(bool isPersistentSelection))
   DECL_CS_SIGNAL_2(persistentSelectionChanged, isPersistentSelection)
   DECL_CS_SIGNAL_1(Public, void textMarginChanged(qreal textMargin))
   DECL_CS_SIGNAL_2(textMarginChanged, textMargin)
   DECL_CS_SIGNAL_1(Public, void selectByMouseChanged(bool selectByMouse))
   DECL_CS_SIGNAL_2(selectByMouseChanged, selectByMouse)

   DECL_CS_SIGNAL_1(Public, void mouseSelectionModeChanged(SelectionMode mode))
   DECL_CS_SIGNAL_2(mouseSelectionModeChanged, mode)
   CS_REVISION(mouseSelectionModeChanged, mode, 1)

   DECL_CS_SIGNAL_1(Public, void linkActivated(const QString &link))
   DECL_CS_SIGNAL_2(linkActivated, link)
   CS_REVISION(linkActivated, link, 1)

   DECL_CS_SIGNAL_1(Public, void canPasteChanged())
   DECL_CS_SIGNAL_2(canPasteChanged)
   CS_REVISION(canPasteChanged, 1)

   DECL_CS_SIGNAL_1(Public, void inputMethodComposingChanged())
   DECL_CS_SIGNAL_2(inputMethodComposingChanged)
   CS_REVISION(inputMethodComposingChanged, 1)

   DECL_CS_SLOT_1(Public, void selectAll())
   DECL_CS_SLOT_2(selectAll)
   DECL_CS_SLOT_1(Public, void selectWord())
   DECL_CS_SLOT_2(selectWord)
   DECL_CS_SLOT_1(Public, void select(int start, int end))
   DECL_CS_SLOT_2(select)

   DECL_CS_SLOT_1(Public, void deselect())
   DECL_CS_SLOT_2(deselect)
   CS_REVISION(deselect, 1)

   DECL_CS_SLOT_1(Public, bool isRightToLeft(int start, int end))
   DECL_CS_SLOT_2(isRightToLeft)
   CS_REVISION(isRightToLeft, 1)

#ifndef QT_NO_CLIPBOARD
   DECL_CS_SLOT_1(Public, void cut())
   DECL_CS_SLOT_2(cut)
   DECL_CS_SLOT_1(Public, void copy())
   DECL_CS_SLOT_2(copy)
   DECL_CS_SLOT_1(Public, void paste())
   DECL_CS_SLOT_2(paste)
#endif

 protected:
   virtual void geometryChanged(const QRectF &newGeometry, const QRectF &oldGeometry);

   bool event(QEvent *);
   void keyPressEvent(QKeyEvent *);
   void keyReleaseEvent(QKeyEvent *);
   void focusInEvent(QFocusEvent *event);

   // mouse filter?
   void mousePressEvent(QGraphicsSceneMouseEvent *event);
   void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
   void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event);
   void mouseMoveEvent(QGraphicsSceneMouseEvent *event);

   void inputMethodEvent(QInputMethodEvent *e);

   void drawContents(QPainter *, const QRect &);

 private :
   DECL_CS_SLOT_1(Private, void updateImgCache(const QRectF &rect))
   DECL_CS_SLOT_2(updateImgCache)
   DECL_CS_SLOT_1(Private, void q_textChanged())
   DECL_CS_SLOT_2(q_textChanged)
   DECL_CS_SLOT_1(Private, void updateSelectionMarkers())
   DECL_CS_SLOT_2(updateSelectionMarkers)
   DECL_CS_SLOT_1(Private, void moveCursorDelegate())
   DECL_CS_SLOT_2(moveCursorDelegate)
   DECL_CS_SLOT_1(Private, void loadCursorDelegate())
   DECL_CS_SLOT_2(loadCursorDelegate)
   DECL_CS_SLOT_1(Private, void q_canPasteChanged())
   DECL_CS_SLOT_2(q_canPasteChanged)

   void updateSize();
   void updateTotalLines();

   Q_DISABLE_COPY(QDeclarativeTextEdit)
   Q_DECLARE_PRIVATE_D(QGraphicsItem::d_ptr.data(), QDeclarativeTextEdit)
};

QT_END_NAMESPACE

QML_DECLARE_TYPE(QDeclarativeTextEdit)

#endif
