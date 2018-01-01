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

#ifndef QPLAINTEXTEDIT_H
#define QPLAINTEXTEDIT_H

#include <QtGui/qtextedit.h>
#include <QtGui/qabstractscrollarea.h>
#include <QtGui/qtextdocument.h>
#include <QtGui/qtextoption.h>
#include <QtGui/qtextcursor.h>
#include <QtGui/qtextformat.h>
#include <QtGui/qabstracttextdocumentlayout.h>

#ifndef QT_NO_TEXTEDIT

QT_BEGIN_NAMESPACE

class QStyleSheet;
class QTextDocument;
class QMenu;
class QPlainTextEditPrivate;
class QMimeData;
class QPlainTextDocumentLayoutPrivate;

class Q_GUI_EXPORT QPlainTextEdit : public QAbstractScrollArea
{
   GUI_CS_OBJECT(QPlainTextEdit)
   Q_DECLARE_PRIVATE(QPlainTextEdit)

   GUI_CS_ENUM(LineWrapMode)

   GUI_CS_PROPERTY_READ(tabChangesFocus, tabChangesFocus)
   GUI_CS_PROPERTY_WRITE(tabChangesFocus, setTabChangesFocus)
   GUI_CS_PROPERTY_READ(documentTitle, documentTitle)
   GUI_CS_PROPERTY_WRITE(documentTitle, setDocumentTitle)
   GUI_CS_PROPERTY_READ(undoRedoEnabled, isUndoRedoEnabled)
   GUI_CS_PROPERTY_WRITE(undoRedoEnabled, setUndoRedoEnabled)
   GUI_CS_PROPERTY_READ(lineWrapMode, lineWrapMode)
   GUI_CS_PROPERTY_WRITE(lineWrapMode, setLineWrapMode)

   // following 2 were qdoc_property 1/5/2014
   GUI_CS_PROPERTY_READ(wordWrapMode, wordWrapMode)
   GUI_CS_PROPERTY_WRITE(wordWrapMode, setWordWrapMode)

   GUI_CS_PROPERTY_READ(readOnly, isReadOnly)
   GUI_CS_PROPERTY_WRITE(readOnly, setReadOnly)
   GUI_CS_PROPERTY_READ(plainText, toPlainText)
   GUI_CS_PROPERTY_WRITE(plainText, setPlainText)
   GUI_CS_PROPERTY_NOTIFY(plainText, textChanged)
   GUI_CS_PROPERTY_USER(plainText, true)
   GUI_CS_PROPERTY_READ(overwriteMode, overwriteMode)
   GUI_CS_PROPERTY_WRITE(overwriteMode, setOverwriteMode)
   GUI_CS_PROPERTY_READ(tabStopWidth, tabStopWidth)
   GUI_CS_PROPERTY_WRITE(tabStopWidth, setTabStopWidth)
   GUI_CS_PROPERTY_READ(cursorWidth, cursorWidth)
   GUI_CS_PROPERTY_WRITE(cursorWidth, setCursorWidth)
   GUI_CS_PROPERTY_READ(textInteractionFlags, textInteractionFlags)
   GUI_CS_PROPERTY_WRITE(textInteractionFlags, setTextInteractionFlags)
   GUI_CS_PROPERTY_READ(blockCount, blockCount)
   GUI_CS_PROPERTY_READ(maximumBlockCount, maximumBlockCount)
   GUI_CS_PROPERTY_WRITE(maximumBlockCount, setMaximumBlockCount)
   GUI_CS_PROPERTY_READ(backgroundVisible, backgroundVisible)
   GUI_CS_PROPERTY_WRITE(backgroundVisible, setBackgroundVisible)
   GUI_CS_PROPERTY_READ(centerOnScroll, centerOnScroll)
   GUI_CS_PROPERTY_WRITE(centerOnScroll, setCenterOnScroll)

 public:
   enum LineWrapMode {
      NoWrap,
      WidgetWidth
   };

   explicit QPlainTextEdit(QWidget *parent = nullptr);
   explicit QPlainTextEdit(const QString &text, QWidget *parent = nullptr);
   virtual ~QPlainTextEdit();

   void setDocument(QTextDocument *document);
   QTextDocument *document() const;

   void setTextCursor(const QTextCursor &cursor);
   QTextCursor textCursor() const;

   bool isReadOnly() const;
   void setReadOnly(bool ro);

   void setTextInteractionFlags(Qt::TextInteractionFlags flags);
   Qt::TextInteractionFlags textInteractionFlags() const;

   void mergeCurrentCharFormat(const QTextCharFormat &modifier);
   void setCurrentCharFormat(const QTextCharFormat &format);
   QTextCharFormat currentCharFormat() const;

   bool tabChangesFocus() const;
   void setTabChangesFocus(bool b);

   inline void setDocumentTitle(const QString &title);
   inline QString documentTitle() const;

   inline bool isUndoRedoEnabled() const;
   inline void setUndoRedoEnabled(bool enable);

   inline void setMaximumBlockCount(int maximum);
   inline int maximumBlockCount() const;

   LineWrapMode lineWrapMode() const;
   void setLineWrapMode(LineWrapMode mode);

   QTextOption::WrapMode wordWrapMode() const;
   void setWordWrapMode(QTextOption::WrapMode policy);

   void setBackgroundVisible(bool visible);
   bool backgroundVisible() const;

   void setCenterOnScroll(bool enabled);
   bool centerOnScroll() const;

   bool find(const QString &exp, QTextDocument::FindFlags options = 0);

   inline QString toPlainText() const;

   void ensureCursorVisible();

   virtual QVariant loadResource(int type, const QUrl &name);

#ifndef QT_NO_CONTEXTMENU
   QMenu *createStandardContextMenu();
#endif

   QTextCursor cursorForPosition(const QPoint &pos) const;
   QRect cursorRect(const QTextCursor &cursor) const;
   QRect cursorRect() const;

   QString anchorAt(const QPoint &pos) const;

   bool overwriteMode() const;
   void setOverwriteMode(bool overwrite);

   int tabStopWidth() const;
   void setTabStopWidth(int width);

   int cursorWidth() const;
   void setCursorWidth(int width);

   void setExtraSelections(const QList<QTextEdit::ExtraSelection> &selections);
   QList<QTextEdit::ExtraSelection> extraSelections() const;

   void moveCursor(QTextCursor::MoveOperation operation, QTextCursor::MoveMode mode = QTextCursor::MoveAnchor);

   bool canPaste() const;
   int blockCount() const;

#ifndef QT_NO_PRINTER
   void print(QPrinter *printer) const;
#endif

   GUI_CS_SLOT_1(Public, void setPlainText(const QString &text))
   GUI_CS_SLOT_2(setPlainText)

#ifndef QT_NO_CLIPBOARD
   GUI_CS_SLOT_1(Public, void cut())
   GUI_CS_SLOT_2(cut)
   GUI_CS_SLOT_1(Public, void copy())
   GUI_CS_SLOT_2(copy)
   GUI_CS_SLOT_1(Public, void paste())
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

   GUI_CS_SLOT_1(Public, void appendPlainText(const QString &text))
   GUI_CS_SLOT_2(appendPlainText)
   GUI_CS_SLOT_1(Public, void appendHtml(const QString &html))
   GUI_CS_SLOT_2(appendHtml)

   GUI_CS_SLOT_1(Public, void centerCursor())
   GUI_CS_SLOT_2(centerCursor)

   GUI_CS_SIGNAL_1(Public, void textChanged())
   GUI_CS_SIGNAL_2(textChanged)
   GUI_CS_SIGNAL_1(Public, void undoAvailable(bool b))
   GUI_CS_SIGNAL_2(undoAvailable, b)
   GUI_CS_SIGNAL_1(Public, void redoAvailable(bool b))
   GUI_CS_SIGNAL_2(redoAvailable, b)
   GUI_CS_SIGNAL_1(Public, void copyAvailable(bool b))
   GUI_CS_SIGNAL_2(copyAvailable, b)
   GUI_CS_SIGNAL_1(Public, void selectionChanged())
   GUI_CS_SIGNAL_2(selectionChanged)
   GUI_CS_SIGNAL_1(Public, void cursorPositionChanged())
   GUI_CS_SIGNAL_2(cursorPositionChanged)

   GUI_CS_SIGNAL_1(Public, void updateRequest(const QRect &rect, int dy))
   GUI_CS_SIGNAL_2(updateRequest, rect, dy)
   GUI_CS_SIGNAL_1(Public, void blockCountChanged(int newBlockCount))
   GUI_CS_SIGNAL_2(blockCountChanged, newBlockCount)
   GUI_CS_SIGNAL_1(Public, void modificationChanged(bool un_named_arg1))
   GUI_CS_SIGNAL_2(modificationChanged, un_named_arg1)

 protected:
   bool event(QEvent *e) override;
   void timerEvent(QTimerEvent *e) override;
   void keyPressEvent(QKeyEvent *e) override;
   void keyReleaseEvent(QKeyEvent *e) override;
   void resizeEvent(QResizeEvent *e) override;
   void paintEvent(QPaintEvent *e) override;
   void mousePressEvent(QMouseEvent *e) override;
   void mouseMoveEvent(QMouseEvent *e) override;
   void mouseReleaseEvent(QMouseEvent *e) override;
   void mouseDoubleClickEvent(QMouseEvent *e) override;
   bool focusNextPrevChild(bool next) override;

#ifndef QT_NO_CONTEXTMENU
   void contextMenuEvent(QContextMenuEvent *e) override;
#endif

#ifndef QT_NO_DRAGANDDROP
   void dragEnterEvent(QDragEnterEvent *e) override;
   void dragLeaveEvent(QDragLeaveEvent *e) override;
   void dragMoveEvent(QDragMoveEvent *e) override;
   void dropEvent(QDropEvent *e) override;
#endif

   void focusInEvent(QFocusEvent *e) override;
   void focusOutEvent(QFocusEvent *e) override;
   void showEvent(QShowEvent *) override;
   void changeEvent(QEvent *e) override;

#ifndef QT_NO_WHEELEVENT
   void wheelEvent(QWheelEvent *e) override;
#endif

   QMimeData *createMimeDataFromSelection() const;
   bool canInsertFromMimeData(const QMimeData *source) const;
   void insertFromMimeData(const QMimeData *source);

   void inputMethodEvent(QInputMethodEvent *) override;
   QVariant inputMethodQuery(Qt::InputMethodQuery property) const override;

   QPlainTextEdit(QPlainTextEditPrivate &dd, QWidget *parent);

   void scrollContentsBy(int dx, int dy) override;

   QTextBlock firstVisibleBlock() const;
   QPointF contentOffset() const;
   QRectF blockBoundingRect(const QTextBlock &block) const;
   QRectF blockBoundingGeometry(const QTextBlock &block) const;
   QAbstractTextDocumentLayout::PaintContext getPaintContext() const;

 private:
   Q_DISABLE_COPY(QPlainTextEdit)

   GUI_CS_SLOT_1(Private, void _q_repaintContents(const QRectF &r))
   GUI_CS_SLOT_2(_q_repaintContents)

   GUI_CS_SLOT_1(Private, void _q_adjustScrollbars())
   GUI_CS_SLOT_2(_q_adjustScrollbars)

   GUI_CS_SLOT_1(Private, void _q_verticalScrollbarActionTriggered(int un_named_arg1))
   GUI_CS_SLOT_2(_q_verticalScrollbarActionTriggered)

   GUI_CS_SLOT_1(Private, void _q_cursorPositionChanged())
   GUI_CS_SLOT_2(_q_cursorPositionChanged)

   friend class QPlainTextEditControl;
};

class Q_GUI_EXPORT QPlainTextDocumentLayout : public QAbstractTextDocumentLayout
{
   GUI_CS_OBJECT(QPlainTextDocumentLayout)
   Q_DECLARE_PRIVATE(QPlainTextDocumentLayout)

   GUI_CS_PROPERTY_READ(cursorWidth, cursorWidth)
   GUI_CS_PROPERTY_WRITE(cursorWidth, setCursorWidth)

 public:
   QPlainTextDocumentLayout(QTextDocument *document);
   ~QPlainTextDocumentLayout();

   void draw(QPainter *, const PaintContext &) override;
   int hitTest(const QPointF &, Qt::HitTestAccuracy ) const override;

   int pageCount() const override;
   QSizeF documentSize() const override;

   QRectF frameBoundingRect(QTextFrame *) const override;
   QRectF blockBoundingRect(const QTextBlock &block) const override;

   void ensureBlockLayout(const QTextBlock &block) const;

   void setCursorWidth(int width);
   int cursorWidth() const;

   void requestUpdate();

 protected:
   void documentChanged(int from, int /*charsRemoved*/, int charsAdded) override;

 private:
   void setTextWidth(qreal newWidth);
   qreal textWidth() const;
   void layoutBlock(const QTextBlock &block);
   qreal blockWidth(const QTextBlock &block);

   QPlainTextDocumentLayoutPrivate *priv() const;

   friend class QPlainTextEdit;
   friend class QPlainTextEditPrivate;
};

void QPlainTextEdit::setDocumentTitle(const QString &title)
{
   document()->setMetaInformation(QTextDocument::DocumentTitle, title);
}

QString QPlainTextEdit::documentTitle() const
{
   return document()->metaInformation(QTextDocument::DocumentTitle);
}

bool QPlainTextEdit::isUndoRedoEnabled() const
{
   return document()->isUndoRedoEnabled();
}

void QPlainTextEdit::setUndoRedoEnabled(bool enable)
{
   document()->setUndoRedoEnabled(enable);
}

void QPlainTextEdit::setMaximumBlockCount(int maximum)
{
   document()->setMaximumBlockCount(maximum);
}

int QPlainTextEdit::maximumBlockCount() const
{
   return document()->maximumBlockCount();
}

QString QPlainTextEdit::toPlainText() const
{
   return document()->toPlainText();
}


QT_END_NAMESPACE

#endif // QT_NO_TEXTEDIT

#endif // QPLAINTEXTEDIT_H
