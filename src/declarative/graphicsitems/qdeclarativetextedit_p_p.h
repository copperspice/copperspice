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

#ifndef QDECLARATIVETEXTEDIT_P_P_H
#define QDECLARATIVETEXTEDIT_P_P_H

#include <qdeclarativeitem.h>
#include <qdeclarativeimplicitsizeitem_p_p.h>
#include <qdeclarative.h>

QT_BEGIN_NAMESPACE
class QTextLayout;
class QTextDocument;
class QTextControl;
class QDeclarativeTextEditPrivate : public QDeclarativeImplicitSizePaintedItemPrivate
{
   Q_DECLARE_PUBLIC(QDeclarativeTextEdit)

 public:
   QDeclarativeTextEditPrivate()
      : color("black"), hAlign(QDeclarativeTextEdit::AlignLeft), vAlign(QDeclarativeTextEdit::AlignTop),
        imgDirty(true), dirty(false), richText(false), cursorVisible(false), focusOnPress(true),
        showInputPanelOnFocus(true), clickCausedFocus(false), persistentSelection(true), requireImplicitWidth(false),
        hAlignImplicit(true), rightToLeftText(false), textMargin(0.0), lastSelectionStart(0), lastSelectionEnd(0),
        cursorComponent(0), cursor(0), format(QDeclarativeTextEdit::AutoText), document(0),
        wrapMode(QDeclarativeTextEdit::NoWrap),
        mouseSelectionMode(QDeclarativeTextEdit::SelectCharacters), lineCount(0), selectByMouse(false), canPaste(false),
        yoff(0) {
   }

   void init();

   void updateDefaultTextOption();
   void relayoutDocument();
   void updateSelection();
   bool determineHorizontalAlignment();
   bool setHAlign(QDeclarativeTextEdit::HAlignment, bool forceAlign = false);
   void mirrorChange();
   qreal implicitWidth() const;
   void focusChanged(bool);

   QString text;
   QFont font;
   QFont sourceFont;
   QColor  color;
   QColor  selectionColor;
   QColor  selectedTextColor;
   QString style;
   QColor  styleColor;
   QPixmap imgCache;
   QPixmap imgStyleCache;
   QDeclarativeTextEdit::HAlignment hAlign;
   QDeclarativeTextEdit::VAlignment vAlign;
   bool imgDirty : 1;
   bool dirty : 1;
   bool richText : 1;
   bool cursorVisible : 1;
   bool focusOnPress : 1;
   bool showInputPanelOnFocus : 1;
   bool clickCausedFocus : 1;
   bool persistentSelection : 1;
   bool requireImplicitWidth: 1;
   bool hAlignImplicit: 1;
   bool rightToLeftText: 1;
   qreal textMargin;
   int lastSelectionStart;
   int lastSelectionEnd;
   QDeclarativeComponent *cursorComponent;
   QDeclarativeItem *cursor;
   QDeclarativeTextEdit::TextFormat format;
   QTextDocument *document;
   QTextControl *control;
   QDeclarativeTextEdit::WrapMode wrapMode;
   QDeclarativeTextEdit::SelectionMode mouseSelectionMode;
   int lineCount;
   bool selectByMouse;
   bool canPaste;
   int yoff;
   QSize paintedSize;
};

QT_END_NAMESPACE
#endif
