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

#ifndef QTEXTCONTROL_P_H
#define QTEXTCONTROL_P_H

#include <QtGui/qtextdocument.h>
#include <QtGui/qtextoption.h>
#include <QtGui/qtextcursor.h>
#include <QtGui/qtextformat.h>
#include <QtGui/qtextedit.h>
#include <QtGui/qmenu.h>
#include <QtCore/qrect.h>
#include <QtGui/qabstracttextdocumentlayout.h>
#include <QtGui/qtextdocumentfragment.h>
#include <QtGui/qclipboard.h>
#include <qtextblock.h>
#include <QScopedPointer>

QT_BEGIN_NAMESPACE

class QStyleSheet;
class QTextDocument;
class QMenu;
class QTextControlPrivate;
class QMimeData;
class QAbstractScrollArea;
class QEvent;
class QTimerEvent;

class Q_GUI_EXPORT QTextControl : public QObject
{
   GUI_CS_OBJECT(QTextControl)
   Q_DECLARE_PRIVATE(QTextControl)

#ifndef QT_NO_TEXTHTMLPARSER
   GUI_CS_PROPERTY_READ(html, toHtml)
   GUI_CS_PROPERTY_WRITE(html, setHtml)
   GUI_CS_PROPERTY_NOTIFY(html, textChanged)
   GUI_CS_PROPERTY_USER(html, true)
#endif

   GUI_CS_PROPERTY_READ(overwriteMode, overwriteMode)
   GUI_CS_PROPERTY_WRITE(overwriteMode, setOverwriteMode)
   GUI_CS_PROPERTY_READ(acceptRichText, acceptRichText)
   GUI_CS_PROPERTY_WRITE(acceptRichText, setAcceptRichText)
   GUI_CS_PROPERTY_READ(cursorWidth, cursorWidth)
   GUI_CS_PROPERTY_WRITE(cursorWidth, setCursorWidth)
   GUI_CS_PROPERTY_READ(textInteractionFlags, textInteractionFlags)
   GUI_CS_PROPERTY_WRITE(textInteractionFlags, setTextInteractionFlags)
   GUI_CS_PROPERTY_READ(openExternalLinks, openExternalLinks)
   GUI_CS_PROPERTY_WRITE(openExternalLinks, setOpenExternalLinks)
   GUI_CS_PROPERTY_READ(ignoreUnusedNavigationEvents, ignoreUnusedNavigationEvents)
   GUI_CS_PROPERTY_WRITE(ignoreUnusedNavigationEvents, setIgnoreUnusedNavigationEvents)

 public:
   explicit QTextControl(QObject *parent = nullptr);
   explicit QTextControl(const QString &text, QObject *parent = nullptr);
   explicit QTextControl(QTextDocument *doc, QObject *parent = nullptr);
   virtual ~QTextControl();

   void setDocument(QTextDocument *document);
   QTextDocument *document() const;

   void setTextCursor(const QTextCursor &cursor);
   QTextCursor textCursor() const;

   void setTextInteractionFlags(Qt::TextInteractionFlags flags);
   Qt::TextInteractionFlags textInteractionFlags() const;

   void mergeCurrentCharFormat(const QTextCharFormat &modifier);

   void setCurrentCharFormat(const QTextCharFormat &format);
   QTextCharFormat currentCharFormat() const;

   bool find(const QString &exp, QTextDocument::FindFlags options = 0);

   inline QString toPlainText() const {
      return document()->toPlainText();
   }

#ifndef QT_NO_TEXTHTMLPARSER
   inline QString toHtml() const;
#endif

   virtual void ensureCursorVisible();

   virtual QVariant loadResource(int type, const QUrl &name);

#ifndef QT_NO_CONTEXTMENU
   QMenu *createStandardContextMenu(const QPointF &pos, QWidget *parent);
#endif

   QTextCursor cursorForPosition(const QPointF &pos) const;
   QRectF cursorRect(const QTextCursor &cursor) const;
   QRectF cursorRect() const;
   QRectF selectionRect(const QTextCursor &cursor) const;
   QRectF selectionRect() const;

   QString anchorAt(const QPointF &pos) const;
   QPointF anchorPosition(const QString &name) const;

   QString anchorAtCursor() const;

   bool overwriteMode() const;
   void setOverwriteMode(bool overwrite);

   int cursorWidth() const;
   void setCursorWidth(int width);

   bool acceptRichText() const;
   void setAcceptRichText(bool accept);

#ifndef QT_NO_TEXTEDIT
   void setExtraSelections(const QList<QTextEdit::ExtraSelection> &selections);
   QList<QTextEdit::ExtraSelection> extraSelections() const;
#endif

   void setTextWidth(qreal width);
   qreal textWidth() const;
   QSizeF size() const;

   void setOpenExternalLinks(bool open);
   bool openExternalLinks() const;

   void setIgnoreUnusedNavigationEvents(bool ignore);
   bool ignoreUnusedNavigationEvents() const;

   void moveCursor(QTextCursor::MoveOperation op, QTextCursor::MoveMode mode = QTextCursor::MoveAnchor);

   bool canPaste() const;

   void setCursorIsFocusIndicator(bool b);
   bool cursorIsFocusIndicator() const;

   void setDragEnabled(bool enabled);
   bool isDragEnabled() const;

   bool isWordSelectionEnabled() const;
   void setWordSelectionEnabled(bool enabled);

#ifndef QT_NO_PRINTER
   void print(QPrinter *printer) const;
#endif

   virtual int hitTest(const QPointF &point, Qt::HitTestAccuracy accuracy) const;
   virtual QRectF blockBoundingRect(const QTextBlock &block) const;
   QAbstractTextDocumentLayout::PaintContext getPaintContext(QWidget *widget) const;

   GUI_CS_SLOT_1(Public, void setPlainText(const QString &text))
   GUI_CS_SLOT_2(setPlainText)

   GUI_CS_SLOT_1(Public, void setHtml(const QString &text))
   GUI_CS_SLOT_2(setHtml)

#ifndef QT_NO_CLIPBOARD
   GUI_CS_SLOT_1(Public, void cut())
   GUI_CS_SLOT_2(cut)

   GUI_CS_SLOT_1(Public, void copy())
   GUI_CS_SLOT_2(copy)

   GUI_CS_SLOT_1(Public, void paste(QClipboard::Mode mode = QClipboard::Clipboard))
   GUI_CS_SLOT_2(paste)
#endif

   GUI_CS_SLOT_1(Public, void undo())
   GUI_CS_SLOT_2(undo)

   GUI_CS_SLOT_1(Public, void redo())
   GUI_CS_SLOT_2(redo)

   GUI_CS_SLOT_1(Public, void clear())
   GUI_CS_SLOT_2(clear)
   GUI_CS_SLOT_1(Public, void selectAll())
   GUI_CS_SLOT_2(selectAll)

   GUI_CS_SLOT_1(Public, void insertPlainText(const QString &text))
   GUI_CS_SLOT_2(insertPlainText)

#ifndef QT_NO_TEXTHTMLPARSER
   GUI_CS_SLOT_1(Public, void insertHtml(const QString &text))
   GUI_CS_SLOT_2(insertHtml)
#endif

   GUI_CS_SLOT_1(Public, void append(const QString &text))
   GUI_CS_SLOT_2(append)
   GUI_CS_SLOT_1(Public, void appendHtml(const QString &html))
   GUI_CS_SLOT_2(appendHtml)
   GUI_CS_SLOT_1(Public, void appendPlainText(const QString &text))
   GUI_CS_SLOT_2(appendPlainText)

   GUI_CS_SLOT_1(Public, void adjustSize())
   GUI_CS_SLOT_2(adjustSize)

   GUI_CS_SIGNAL_1(Public, void textChanged())
   GUI_CS_SIGNAL_2(textChanged)
   GUI_CS_SIGNAL_1(Public, void undoAvailable(bool b))
   GUI_CS_SIGNAL_2(undoAvailable, b)
   GUI_CS_SIGNAL_1(Public, void redoAvailable(bool b))
   GUI_CS_SIGNAL_2(redoAvailable, b)
   GUI_CS_SIGNAL_1(Public, void currentCharFormatChanged(const QTextCharFormat &format))
   GUI_CS_SIGNAL_2(currentCharFormatChanged, format)
   GUI_CS_SIGNAL_1(Public, void copyAvailable(bool b))
   GUI_CS_SIGNAL_2(copyAvailable, b)
   GUI_CS_SIGNAL_1(Public, void selectionChanged())
   GUI_CS_SIGNAL_2(selectionChanged)
   GUI_CS_SIGNAL_1(Public, void cursorPositionChanged())
   GUI_CS_SIGNAL_2(cursorPositionChanged)

   // control signals
   GUI_CS_SIGNAL_1(Public, void updateRequest(const QRectF &rect = QRectF()))
   GUI_CS_SIGNAL_2(updateRequest, rect)

   GUI_CS_SIGNAL_1(Public, void documentSizeChanged(const QSizeF &un_named_arg1))
   GUI_CS_SIGNAL_2(documentSizeChanged, un_named_arg1)

   GUI_CS_SIGNAL_1(Public, void blockCountChanged(int newBlockCount))
   GUI_CS_SIGNAL_2(blockCountChanged, newBlockCount)

   GUI_CS_SIGNAL_1(Public, void visibilityRequest(const QRectF &rect))
   GUI_CS_SIGNAL_2(visibilityRequest, rect)

   GUI_CS_SIGNAL_1(Public, void microFocusChanged())
   GUI_CS_SIGNAL_2(microFocusChanged)

   GUI_CS_SIGNAL_1(Public, void linkActivated(const QString &link))
   GUI_CS_SIGNAL_2(linkActivated, link)

   GUI_CS_SIGNAL_1(Public, void linkHovered(const QString &un_named_arg1))
   GUI_CS_SIGNAL_2(linkHovered, un_named_arg1)

   GUI_CS_SIGNAL_1(Public, void modificationChanged(bool m))
   GUI_CS_SIGNAL_2(modificationChanged, m)

   // control properties
   QPalette palette() const;
   void setPalette(const QPalette &pal);

   virtual void processEvent(QEvent *e, const QMatrix &matrix, QWidget *contextWidget = 0);
   void processEvent(QEvent *e, const QPointF &coordinateOffset = QPointF(), QWidget *contextWidget = 0);

   // control methods
   void drawContents(QPainter *painter, const QRectF &rect = QRectF(), QWidget *widget = 0);

   void setFocus(bool focus, Qt::FocusReason = Qt::OtherFocusReason);

   virtual QVariant inputMethodQuery(Qt::InputMethodQuery property) const;

   virtual QMimeData *createMimeDataFromSelection() const;
   virtual bool canInsertFromMimeData(const QMimeData *source) const;
   virtual void insertFromMimeData(const QMimeData *source);

   bool setFocusToAnchor(const QTextCursor &newCursor);
   bool setFocusToNextOrPreviousAnchor(bool next);
   bool findNextPrevAnchor(const QTextCursor &from, bool next, QTextCursor &newAnchor);

 protected:
   void timerEvent(QTimerEvent *e) override;
   bool event(QEvent *e) override;

   QScopedPointer<QTextControlPrivate> d_ptr;
 
 private:
   Q_DISABLE_COPY(QTextControl)

   GUI_CS_SLOT_1(Private, void _q_updateCurrentCharFormatAndSelection())
   GUI_CS_SLOT_2(_q_updateCurrentCharFormatAndSelection)

   GUI_CS_SLOT_1(Private, void _q_emitCursorPosChanged(const QTextCursor &un_named_arg1))
   GUI_CS_SLOT_2(_q_emitCursorPosChanged)

   GUI_CS_SLOT_1(Private, void _q_deleteSelected())
   GUI_CS_SLOT_2(_q_deleteSelected)

   GUI_CS_SLOT_1(Private, void _q_copyLink())
   GUI_CS_SLOT_2(_q_copyLink)

   GUI_CS_SLOT_1(Private, void _q_updateBlock(const QTextBlock &un_named_arg1))
   GUI_CS_SLOT_2(_q_updateBlock)

   GUI_CS_SLOT_1(Private, void _q_documentLayoutChanged())
   GUI_CS_SLOT_2(_q_documentLayoutChanged)
 
};


#ifndef QT_NO_CONTEXTMENU
class QUnicodeControlCharacterMenu : public QMenu
{
   GUI_CS_OBJECT(QUnicodeControlCharacterMenu)

 public:
   QUnicodeControlCharacterMenu(QObject *editWidget, QWidget *parent);

 private :
   GUI_CS_SLOT_1(Private, void menuActionTriggered())
   GUI_CS_SLOT_2(menuActionTriggered)

   QObject *editWidget;
};
#endif // QT_NO_CONTEXTMENU


// also used by QLabel
class QTextEditMimeData : public QMimeData
{
 public:
   inline QTextEditMimeData(const QTextDocumentFragment &aFragment) : fragment(aFragment) {}

   virtual QStringList formats() const override;

 protected:
   virtual QVariant retrieveData(const QString &mimeType, QVariant::Type type) const override;

 private:
   void setup() const;

   mutable QTextDocumentFragment fragment;
};


#ifndef QT_NO_TEXTHTMLPARSER
QString QTextControl::toHtml() const
{
   return document()->toHtml();
}
#endif

QT_END_NAMESPACE

#endif // QTEXTCONTROL_H
