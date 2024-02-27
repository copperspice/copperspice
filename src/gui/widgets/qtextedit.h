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

#ifndef QTEXTEDIT_H
#define QTEXTEDIT_H

#include <qabstractscrollarea.h>
#include <qtextdocument.h>
#include <qtextoption.h>
#include <qtextcursor.h>
#include <qtextformat.h>

#ifndef QT_NO_TEXTEDIT

class QStyleSheet;
class QTextDocument;
class QMenu;
class QTextEditPrivate;
class QMimeData;
class QPagedPaintDevice;

class Q_GUI_EXPORT QTextEdit : public QAbstractScrollArea
{
   GUI_CS_OBJECT(QTextEdit)

   GUI_CS_ENUM(AutoFormattingFlag)
   GUI_CS_FLAG(AutoFormattingFlag, AutoFormatting)

   GUI_CS_ENUM(LineWrapMode)

   GUI_CS_PROPERTY_READ(autoFormatting, autoFormatting)
   GUI_CS_PROPERTY_WRITE(autoFormatting, setAutoFormatting)

   GUI_CS_PROPERTY_READ(tabChangesFocus, tabChangesFocus)
   GUI_CS_PROPERTY_WRITE(tabChangesFocus, setTabChangesFocus)

   GUI_CS_PROPERTY_READ(documentTitle, documentTitle)
   GUI_CS_PROPERTY_WRITE(documentTitle, setDocumentTitle)

   GUI_CS_PROPERTY_READ(undoRedoEnabled, isUndoRedoEnabled)
   GUI_CS_PROPERTY_WRITE(undoRedoEnabled, setUndoRedoEnabled)

   GUI_CS_PROPERTY_READ(lineWrapMode, lineWrapMode)
   GUI_CS_PROPERTY_WRITE(lineWrapMode, setLineWrapMode)

   GUI_CS_PROPERTY_READ(lineWrapColumnOrWidth, lineWrapColumnOrWidth)
   GUI_CS_PROPERTY_WRITE(lineWrapColumnOrWidth, setLineWrapColumnOrWidth)

   GUI_CS_PROPERTY_READ(readOnly, isReadOnly)
   GUI_CS_PROPERTY_WRITE(readOnly, setReadOnly)

   // following 2 were qdoc_property
   GUI_CS_PROPERTY_READ(wordWrapMode, wordWrapMode)
   GUI_CS_PROPERTY_WRITE(wordWrapMode, setWordWrapMode)

#ifndef QT_NO_TEXTHTMLPARSER
   GUI_CS_PROPERTY_READ(html, toHtml)
   GUI_CS_PROPERTY_WRITE(html, setHtml)
   GUI_CS_PROPERTY_NOTIFY(html, textChanged)
   GUI_CS_PROPERTY_USER(html, true)
#endif

   GUI_CS_PROPERTY_READ(plainText, toPlainText)
   GUI_CS_PROPERTY_WRITE(plainText, setPlainText)
   GUI_CS_PROPERTY_DESIGNABLE(plainText, false)

   GUI_CS_PROPERTY_READ(overwriteMode, overwriteMode)
   GUI_CS_PROPERTY_WRITE(overwriteMode, setOverwriteMode)

   GUI_CS_PROPERTY_READ(tabStopWidth, tabStopWidth)
   GUI_CS_PROPERTY_WRITE(tabStopWidth, setTabStopWidth)

   GUI_CS_PROPERTY_READ(acceptRichText, acceptRichText)
   GUI_CS_PROPERTY_WRITE(acceptRichText, setAcceptRichText)

   GUI_CS_PROPERTY_READ(cursorWidth, cursorWidth)
   GUI_CS_PROPERTY_WRITE(cursorWidth, setCursorWidth)

   GUI_CS_PROPERTY_READ(textInteractionFlags, textInteractionFlags)
   GUI_CS_PROPERTY_WRITE(textInteractionFlags, setTextInteractionFlags)

   GUI_CS_PROPERTY_READ(document, document)
   GUI_CS_PROPERTY_WRITE(document, setDocument)
   GUI_CS_PROPERTY_DESIGNABLE(document, false)

   GUI_CS_PROPERTY_READ(placeholderText, placeholderText)
   GUI_CS_PROPERTY_WRITE(placeholderText, setPlaceholderText)

 public:
   GUI_CS_REGISTER_ENUM(
      enum LineWrapMode {
         NoWrap,
         WidgetWidth,
         FixedPixelWidth,
         FixedColumnWidth
      };
   )

   GUI_CS_REGISTER_ENUM(
      enum AutoFormattingFlag {
         AutoNone       = 0,
         AutoBulletList = 0x00000001,
         AutoAll        = 0xffffffff
      };
   )

   using AutoFormatting = QFlags<AutoFormattingFlag>;

   explicit QTextEdit(QWidget *parent = nullptr);
   explicit QTextEdit(const QString &text, QWidget *parent = nullptr);

   QTextEdit(const QTextEdit &) = delete;
   QTextEdit &operator=(const QTextEdit &) = delete;

   virtual ~QTextEdit();

   void setDocument(QTextDocument *document);
   QTextDocument *document() const;
   void setPlaceholderText(const QString &placeholderText);
   QString placeholderText() const;

   void setTextCursor(const QTextCursor &cursor);
   QTextCursor textCursor() const;

   bool isReadOnly() const;
   void setReadOnly(bool readOnly);

   void setTextInteractionFlags(Qt::TextInteractionFlags flags);
   Qt::TextInteractionFlags textInteractionFlags() const;

   qreal fontPointSize() const;
   QString fontFamily() const;
   int fontWeight() const;
   bool fontUnderline() const;
   bool fontItalic() const;
   QColor textColor() const;
   QColor textBackgroundColor() const;
   QFont currentFont() const;
   Qt::Alignment alignment() const;

   void mergeCurrentCharFormat(const QTextCharFormat &modifier);

   void setCurrentCharFormat(const QTextCharFormat &format);
   QTextCharFormat currentCharFormat() const;

   AutoFormatting autoFormatting() const;
   void setAutoFormatting(AutoFormatting features);

   bool tabChangesFocus() const;
   void setTabChangesFocus(bool enabled);

   inline void setDocumentTitle(const QString &title);
   inline QString documentTitle() const;

   inline bool isUndoRedoEnabled() const;
   inline void setUndoRedoEnabled(bool enabled);

   LineWrapMode lineWrapMode() const;
   void setLineWrapMode(LineWrapMode mode);

   int lineWrapColumnOrWidth() const;
   void setLineWrapColumnOrWidth(int width);

   QTextOption::WrapMode wordWrapMode() const;
   void setWordWrapMode(QTextOption::WrapMode policy);

   bool find(const QString &exp, QTextDocument::FindFlags options = QTextDocument::FindFlags());
   bool find(const QRegularExpression &exp, QTextDocument::FindFlags options = QTextDocument::FindFlags());

   QString toPlainText() const;

#ifndef QT_NO_TEXTHTMLPARSER
   QString toHtml() const;
#endif

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

   bool acceptRichText() const;
   void setAcceptRichText(bool accept);

   struct ExtraSelection {
      QTextCursor cursor;
      QTextCharFormat format;
   };

   void setExtraSelections(const QList<ExtraSelection> &selections);
   QList<ExtraSelection> extraSelections() const;

   void moveCursor(QTextCursor::MoveOperation operation, QTextCursor::MoveMode mode = QTextCursor::MoveAnchor);
   bool canPaste() const;

   void print(QPagedPaintDevice *printer) const;

   QVariant inputMethodQuery(Qt::InputMethodQuery property) const override;
   QVariant inputMethodQuery(Qt::InputMethodQuery query, QVariant argument) const;

   GUI_CS_SLOT_1(Public, void setFontPointSize(qreal size))
   GUI_CS_SLOT_2(setFontPointSize)

   GUI_CS_SLOT_1(Public, void setFontFamily(const QString &fontFamily))
   GUI_CS_SLOT_2(setFontFamily)

   GUI_CS_SLOT_1(Public, void setFontWeight(int weight))
   GUI_CS_SLOT_2(setFontWeight)

   GUI_CS_SLOT_1(Public, void setFontUnderline(bool underline))
   GUI_CS_SLOT_2(setFontUnderline)

   GUI_CS_SLOT_1(Public, void setFontItalic(bool italic))
   GUI_CS_SLOT_2(setFontItalic)

   GUI_CS_SLOT_1(Public, void setTextColor(const QColor &color))
   GUI_CS_SLOT_2(setTextColor)

   GUI_CS_SLOT_1(Public, void setTextBackgroundColor(const QColor &color))
   GUI_CS_SLOT_2(setTextBackgroundColor)

   GUI_CS_SLOT_1(Public, void setCurrentFont(const QFont &font))
   GUI_CS_SLOT_2(setCurrentFont)

   GUI_CS_SLOT_1(Public, void setAlignment(Qt::Alignment align))
   GUI_CS_SLOT_2(setAlignment)

   GUI_CS_SLOT_1(Public, void setPlainText(const QString &text))
   GUI_CS_SLOT_2(setPlainText)

   GUI_CS_SLOT_1(Public, void setText(const QString &text))
   GUI_CS_SLOT_2(setText)

#ifndef QT_NO_TEXTHTMLPARSER
   GUI_CS_SLOT_1(Public, void setHtml(const QString &text))
   GUI_CS_SLOT_2(setHtml)

   GUI_CS_SLOT_1(Public, void insertHtml(const QString &text))
   GUI_CS_SLOT_2(insertHtml)
#endif

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

   GUI_CS_SLOT_1(Public, void append(const QString &text))
   GUI_CS_SLOT_2(append)

   GUI_CS_SLOT_1(Public, void scrollToAnchor(const QString &name))
   GUI_CS_SLOT_2(scrollToAnchor)

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

   GUI_CS_SIGNAL_1(Public, void currentCharFormatChanged(const QTextCharFormat &format))
   GUI_CS_SIGNAL_2(currentCharFormatChanged, format)

   GUI_CS_SIGNAL_1(Public, void copyAvailable(bool status))
   GUI_CS_SIGNAL_2(copyAvailable, status)

   GUI_CS_SIGNAL_1(Public, void selectionChanged())
   GUI_CS_SIGNAL_2(selectionChanged)

   GUI_CS_SIGNAL_1(Public, void cursorPositionChanged())
   GUI_CS_SIGNAL_2(cursorPositionChanged)

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

#ifndef QT_NO_WHEELEVENT
   void wheelEvent(QWheelEvent *event) override;
#endif

   void focusInEvent(QFocusEvent *event) override;
   void focusOutEvent(QFocusEvent *event) override;
   void showEvent(QShowEvent *event) override;
   void changeEvent(QEvent *event) override;

   virtual QMimeData *createMimeDataFromSelection() const;
   virtual bool canInsertFromMimeData(const QMimeData *source) const;
   virtual void insertFromMimeData(const QMimeData *source);

   void inputMethodEvent(QInputMethodEvent *event) override;
   QTextEdit(QTextEditPrivate &dd, QWidget *parent);

   void scrollContentsBy(int dx, int dy) override;
   virtual void doSetTextCursor(const QTextCursor &cursor);

   void zoomInF(float range);

 private:
   GUI_CS_SLOT_1(Private, void _q_repaintContents(const QRectF &r))
   GUI_CS_SLOT_2(_q_repaintContents)

   GUI_CS_SLOT_1(Private, void _q_currentCharFormatChanged(const QTextCharFormat &textFormat))
   GUI_CS_SLOT_2(_q_currentCharFormatChanged)

   GUI_CS_SLOT_1(Private, void _q_adjustScrollbars())
   GUI_CS_SLOT_2(_q_adjustScrollbars)

   GUI_CS_SLOT_1(Private, void _q_ensureVisible(const QRectF &rectF))
   GUI_CS_SLOT_2(_q_ensureVisible)

   GUI_CS_SLOT_1(Private, void _q_cursorPositionChanged())
   GUI_CS_SLOT_2(_q_cursorPositionChanged)

   Q_DECLARE_PRIVATE(QTextEdit)

   friend class QTextEditControl;
   friend class QTextDocument;
   friend class QTextControl;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(QTextEdit::AutoFormatting)

void QTextEdit::setDocumentTitle(const QString &title)
{
   document()->setMetaInformation(QTextDocument::DocumentTitle, title);
}

QString QTextEdit::documentTitle() const
{
   return document()->metaInformation(QTextDocument::DocumentTitle);
}

bool QTextEdit::isUndoRedoEnabled() const
{
   return document()->isUndoRedoEnabled();
}

void QTextEdit::setUndoRedoEnabled(bool enabled)
{
   document()->setUndoRedoEnabled(enabled);
}

#endif // QT_NO_TEXTEDIT

#endif // QTEXTEDIT_H
