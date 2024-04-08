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

#ifndef QTEXTCONTROL_P_H
#define QTEXTCONTROL_P_H

#include <qtextdocument.h>
#include <qtextoption.h>
#include <qtextcursor.h>
#include <qtextformat.h>
#include <qtextedit.h>
#include <qmenu.h>
#include <qrect.h>
#include <qabstracttextdocumentlayout.h>
#include <qtextdocumentfragment.h>
#include <qclipboard.h>
#include <qmimedata.h>
#include <qinputcontrol_p.h>

class QAbstractScrollArea;
class QMenu;
class QStyleSheet;
class QTextDocument;
class QTextControlPrivate;

class QEvent;
class QTimerEvent;
class QPagedPaintDevice;

class Q_GUI_EXPORT QTextControl : public QInputControl
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

   QTextControl(const QTextControl &) = delete;
   QTextControl &operator=(const QTextControl &) = delete;

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

   bool find(const QString &exp, QTextDocument::FindFlags options = Qt::EmptyFlag);

   bool find(const QRegularExpression &exp, QTextDocument::FindFlags options = Qt::EmptyFlag);
   QString toPlainText() const;

#ifndef QT_NO_TEXTHTMLPARSER
   QString toHtml() const;
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

   virtual QString anchorAt(const QPointF &pos) const;
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

   bool isPreediting();
   void print(QPagedPaintDevice *printer) const;

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

   GUI_CS_SIGNAL_1(Public, void undoAvailable(bool status))
   GUI_CS_SIGNAL_2(undoAvailable, status)

   GUI_CS_SIGNAL_1(Public, void redoAvailable(bool status))
   GUI_CS_SIGNAL_2(redoAvailable, status)

   GUI_CS_SIGNAL_1(Public, void currentCharFormatChanged(const QTextCharFormat &format))
   GUI_CS_SIGNAL_2(currentCharFormatChanged, format)

   GUI_CS_SIGNAL_1(Public, void copyAvailable(bool status))
   GUI_CS_SIGNAL_2(copyAvailable, status)

   GUI_CS_SIGNAL_1(Public, void selectionChanged())
   GUI_CS_SIGNAL_2(selectionChanged)

   GUI_CS_SIGNAL_1(Public, void cursorPositionChanged())
   GUI_CS_SIGNAL_2(cursorPositionChanged)

   // control signals
   GUI_CS_SIGNAL_1(Public, void updateRequest(const QRectF &rect = QRectF()))
   GUI_CS_SIGNAL_2(updateRequest, rect)

   GUI_CS_SIGNAL_1(Public, void documentSizeChanged(const QSizeF &size))
   GUI_CS_SIGNAL_2(documentSizeChanged, size)

   GUI_CS_SIGNAL_1(Public, void blockCountChanged(int newBlockCount))
   GUI_CS_SIGNAL_2(blockCountChanged, newBlockCount)

   GUI_CS_SIGNAL_1(Public, void visibilityRequest(const QRectF &rect))
   GUI_CS_SIGNAL_2(visibilityRequest, rect)

   GUI_CS_SIGNAL_1(Public, void microFocusChanged())
   GUI_CS_SIGNAL_2(microFocusChanged)

   GUI_CS_SIGNAL_1(Public, void linkActivated(const QString &link))
   GUI_CS_SIGNAL_2(linkActivated, link)

   GUI_CS_SIGNAL_1(Public, void linkHovered(const QString &anchor))
   GUI_CS_SIGNAL_2(linkHovered, anchor)

   GUI_CS_SIGNAL_1(Public, void modificationChanged(bool m))
   GUI_CS_SIGNAL_2(modificationChanged, m)

   // control properties
   QPalette palette() const;
   void setPalette(const QPalette &pal);

   virtual void processEvent(QEvent *e, const QMatrix &matrix, QWidget *contextWidget = nullptr);
   void processEvent(QEvent *e, const QPointF &coordinateOffset = QPointF(), QWidget *contextWidget = nullptr);

   // control methods
   void drawContents(QPainter *painter, const QRectF &rect = QRectF(), QWidget *widget = nullptr);

   void setFocus(bool focus, Qt::FocusReason = Qt::OtherFocusReason);

   virtual QVariant inputMethodQuery(Qt::InputMethodQuery property, QVariant argument) const;

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
   GUI_CS_SLOT_1(Private, void _q_updateCurrentCharFormatAndSelection())
   GUI_CS_SLOT_2(_q_updateCurrentCharFormatAndSelection)

   GUI_CS_SLOT_1(Private, void _q_emitCursorPosChanged(const QTextCursor &cursor))
   GUI_CS_SLOT_2(_q_emitCursorPosChanged)

   GUI_CS_SLOT_1(Private, void _q_deleteSelected())
   GUI_CS_SLOT_2(_q_deleteSelected)

   GUI_CS_SLOT_1(Private, void _q_copyLink())
   GUI_CS_SLOT_2(_q_copyLink)

   GUI_CS_SLOT_1(Private, void _q_updateBlock(const QTextBlock &block))
   GUI_CS_SLOT_2(_q_updateBlock)

   GUI_CS_SLOT_1(Private, void _q_documentLayoutChanged())
   GUI_CS_SLOT_2(_q_documentLayoutChanged)

   GUI_CS_SLOT_1(Private, void _q_contentsChanged(int arg1, int arg2, int arg3))
   GUI_CS_SLOT_2(_q_contentsChanged)
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
   QTextEditMimeData(const QTextDocumentFragment &aFragment)
      : fragment(aFragment)
   { }

   QStringList formats() const override;

 protected:
   QVariant retrieveData(const QString &mimeType, QVariant::Type type) const override;

 private:
   void setup() const;

   mutable QTextDocumentFragment fragment;
};



#endif // QTEXTCONTROL_H
