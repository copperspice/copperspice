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

#ifndef QPLAINTEXTEDIT_H
#define QPLAINTEXTEDIT_H

#include <qtextedit.h>

#include <qabstractscrollarea.h>
#include <qabstracttextdocumentlayout.h>
#include <qtextdocument.h>
#include <qtextoption.h>
#include <qtextcursor.h>
#include <qtextformat.h>

#ifndef QT_NO_TEXTEDIT

class QStyleSheet;
class QTextDocument;
class QMenu;
class QPlainTextEditPrivate;
class QMimeData;
class QPagedPaintDevice;
class QPlainTextDocumentLayoutPrivate;

class Q_GUI_EXPORT QPlainTextEdit : public QAbstractScrollArea
{
   GUI_CS_OBJECT(QPlainTextEdit)

   GUI_CS_ENUM(LineWrapMode)

   GUI_CS_PROPERTY_READ(tabChangesFocus, tabChangesFocus)
   GUI_CS_PROPERTY_WRITE(tabChangesFocus, setTabChangesFocus)

   GUI_CS_PROPERTY_READ(documentTitle, documentTitle)
   GUI_CS_PROPERTY_WRITE(documentTitle, setDocumentTitle)

   GUI_CS_PROPERTY_READ(undoRedoEnabled, isUndoRedoEnabled)
   GUI_CS_PROPERTY_WRITE(undoRedoEnabled, setUndoRedoEnabled)

   GUI_CS_PROPERTY_READ(lineWrapMode, lineWrapMode)
   GUI_CS_PROPERTY_WRITE(lineWrapMode, setLineWrapMode)

   // following were qdoc_property
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

   GUI_CS_PROPERTY_READ(placeholderText, placeholderText )
   GUI_CS_PROPERTY_WRITE(placeholderText, setPlaceholderText)

 public:
   GUI_CS_REGISTER_ENUM(
      enum LineWrapMode {
         NoWrap,
         WidgetWidth
      };
   )

   explicit QPlainTextEdit(QWidget *parent = nullptr);
   explicit QPlainTextEdit(const QString &text, QWidget *parent = nullptr);

   QPlainTextEdit(const QPlainTextEdit &) = delete;
   QPlainTextEdit &operator=(const QPlainTextEdit &) = delete;

   virtual ~QPlainTextEdit();

   void setDocument(QTextDocument *document);
   QTextDocument *document() const;
   void setPlaceholderText(const QString &placeholderText);
   QString placeholderText() const;

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
   void setTabChangesFocus(bool enable);

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

   void setCenterOnScroll(bool enable);
   bool centerOnScroll() const;

   bool find(const QString &exp, QTextDocument::FindFlags options = QTextDocument::FindFlags());

   bool find(const QRegularExpression &exp, QTextDocument::FindFlags options = QTextDocument::FindFlags());

   QString toPlainText() const {
      return document()->toPlainText();
   }

   void ensureCursorVisible();

   virtual QVariant loadResource(int type, const QUrl &name);

#ifndef QT_NO_CONTEXTMENU
   QMenu *createStandardContextMenu();
   QMenu *createStandardContextMenu(const QPoint &position);
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
   void print(QPagedPaintDevice *printer) const;
   int blockCount() const;

   QVariant inputMethodQuery(Qt::InputMethodQuery property) const override;
   QVariant inputMethodQuery(Qt::InputMethodQuery query, QVariant argument) const;

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

   GUI_CS_SLOT_1(Public, void zoomIn(int range = 1))
   GUI_CS_SLOT_2(zoomIn)

   GUI_CS_SLOT_1(Public, void zoomOut(int range = 1))
   GUI_CS_SLOT_2(zoomOut)

   GUI_CS_SIGNAL_1(Public, void textChanged())
   GUI_CS_SIGNAL_2(textChanged)

   GUI_CS_SIGNAL_1(Public, void undoAvailable(bool status))
   GUI_CS_SIGNAL_2(undoAvailable, status)

   GUI_CS_SIGNAL_1(Public, void redoAvailable(bool status))
   GUI_CS_SIGNAL_2(redoAvailable, status)

   GUI_CS_SIGNAL_1(Public, void copyAvailable(bool status))
   GUI_CS_SIGNAL_2(copyAvailable, status)

   GUI_CS_SIGNAL_1(Public, void selectionChanged())
   GUI_CS_SIGNAL_2(selectionChanged)

   GUI_CS_SIGNAL_1(Public, void cursorPositionChanged())
   GUI_CS_SIGNAL_2(cursorPositionChanged)

   GUI_CS_SIGNAL_1(Public, void updateRequest(const QRect &rect, int dy))
   GUI_CS_SIGNAL_2(updateRequest, rect, dy)

   GUI_CS_SIGNAL_1(Public, void blockCountChanged(int newBlockCount))
   GUI_CS_SIGNAL_2(blockCountChanged, newBlockCount)

   GUI_CS_SIGNAL_1(Public, void modificationChanged(bool changed))
   GUI_CS_SIGNAL_2(modificationChanged, changed)

 protected:
   bool event(QEvent *event) override;
   void timerEvent(QTimerEvent *event) override;
   void keyPressEvent(QKeyEvent *event) override;
   void keyReleaseEvent(QKeyEvent *event) override;
   void resizeEvent(QResizeEvent *event) override;
   void paintEvent(QPaintEvent *event) override;
   void mousePressEvent(QMouseEvent *event) override;
   void mouseMoveEvent(QMouseEvent *event) override;
   void mouseReleaseEvent(QMouseEvent *event) override;
   void mouseDoubleClickEvent(QMouseEvent *event) override;
   bool focusNextPrevChild(bool next) override;

#ifndef QT_NO_CONTEXTMENU
   void contextMenuEvent(QContextMenuEvent *event) override;
#endif

#ifndef QT_NO_DRAGANDDROP
   void dragEnterEvent(QDragEnterEvent *event) override;
   void dragLeaveEvent(QDragLeaveEvent *event) override;
   void dragMoveEvent(QDragMoveEvent *event) override;
   void dropEvent(QDropEvent *event) override;
#endif

   void focusInEvent(QFocusEvent *event) override;
   void focusOutEvent(QFocusEvent *event) override;
   void showEvent(QShowEvent *event) override;
   void changeEvent(QEvent *event) override;

#ifndef QT_NO_WHEELEVENT
   void wheelEvent(QWheelEvent *event) override;
#endif

   QMimeData *createMimeDataFromSelection() const;
   bool canInsertFromMimeData(const QMimeData *source) const;
   void insertFromMimeData(const QMimeData *source);

   void inputMethodEvent(QInputMethodEvent *event) override;

   QPlainTextEdit(QPlainTextEditPrivate &dd, QWidget *parent);

   void scrollContentsBy(int dx, int dy) override;
   virtual void doSetTextCursor(const QTextCursor &cursor);

   QTextBlock firstVisibleBlock() const;
   QPointF contentOffset() const;
   QRectF blockBoundingRect(const QTextBlock &block) const;
   QRectF blockBoundingGeometry(const QTextBlock &block) const;
   QAbstractTextDocumentLayout::PaintContext getPaintContext() const;

   void zoomInF(float range);

 private:
   Q_DECLARE_PRIVATE(QPlainTextEdit)

   GUI_CS_SLOT_1(Private, void _q_repaintContents(const QRectF &r))
   GUI_CS_SLOT_2(_q_repaintContents)

   GUI_CS_SLOT_1(Private, void _q_adjustScrollbars())
   GUI_CS_SLOT_2(_q_adjustScrollbars)

   GUI_CS_SLOT_1(Private, void _q_verticalScrollbarActionTriggered(int action))
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
   void documentChanged(int from, int charsRemoved, int charsAdded) override;

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

#endif // QT_NO_TEXTEDIT

#endif