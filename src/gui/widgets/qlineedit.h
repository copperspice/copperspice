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

#ifndef QLINEEDIT_H
#define QLINEEDIT_H

#include <qframe.h>
#include <qtextcursor.h>
#include <qstring.h>
#include <qmargins.h>

#ifndef QT_NO_LINEEDIT

class QValidator;
class QMenu;
class QLineEditPrivate;
class QCompleter;
class QStyleOptionFrame;
class QAbstractSpinBox;
class QDateTimeEdit;
class QIcon;
class QToolButton;

class Q_GUI_EXPORT QLineEdit : public QWidget
{
   GUI_CS_OBJECT(QLineEdit)

   GUI_CS_ENUM(ActionPosition)
   GUI_CS_ENUM(EchoMode)

   GUI_CS_PROPERTY_READ(inputMask, inputMask)
   GUI_CS_PROPERTY_WRITE(inputMask, setInputMask)

   GUI_CS_PROPERTY_READ(text, text)
   GUI_CS_PROPERTY_WRITE(text, setText)
   GUI_CS_PROPERTY_NOTIFY(text, textChanged)
   GUI_CS_PROPERTY_USER(text, true)

   GUI_CS_PROPERTY_READ(maxLength, maxLength)
   GUI_CS_PROPERTY_WRITE(maxLength, setMaxLength)

   GUI_CS_PROPERTY_READ(frame, hasFrame)
   GUI_CS_PROPERTY_WRITE(frame, setFrame)

   GUI_CS_PROPERTY_READ(echoMode, echoMode)
   GUI_CS_PROPERTY_WRITE(echoMode, setEchoMode)

   GUI_CS_PROPERTY_READ(displayText, displayText)

   GUI_CS_PROPERTY_READ(cursorPosition, cursorPosition)
   GUI_CS_PROPERTY_WRITE(cursorPosition, setCursorPosition)

   GUI_CS_PROPERTY_READ(alignment, alignment)
   GUI_CS_PROPERTY_WRITE(alignment, setAlignment)

   GUI_CS_PROPERTY_READ(modified, isModified)
   GUI_CS_PROPERTY_WRITE(modified, setModified)
   GUI_CS_PROPERTY_DESIGNABLE(modified, false)

   GUI_CS_PROPERTY_READ(hasSelectedText, hasSelectedText)

   GUI_CS_PROPERTY_READ(selectedText, selectedText)

   GUI_CS_PROPERTY_READ(dragEnabled, dragEnabled)
   GUI_CS_PROPERTY_WRITE(dragEnabled, setDragEnabled)

   GUI_CS_PROPERTY_READ(readOnly, isReadOnly)
   GUI_CS_PROPERTY_WRITE(readOnly, setReadOnly)

   GUI_CS_PROPERTY_READ(undoAvailable, isUndoAvailable)
   GUI_CS_PROPERTY_READ(redoAvailable, isRedoAvailable)

   GUI_CS_PROPERTY_READ(acceptableInput, hasAcceptableInput)
   GUI_CS_PROPERTY_READ(placeholderText, placeholderText)
   GUI_CS_PROPERTY_WRITE(placeholderText, setPlaceholderText)

   GUI_CS_PROPERTY_READ(cursorMoveStyle, cursorMoveStyle)
   GUI_CS_PROPERTY_WRITE(cursorMoveStyle, setCursorMoveStyle)

   GUI_CS_PROPERTY_READ(clearButtonEnabled, isClearButtonEnabled)
   GUI_CS_PROPERTY_WRITE(clearButtonEnabled, setClearButtonEnabled)

 public:
   enum ActionPosition {
      LeadingPosition,
      TrailingPosition
   };

   GUI_CS_REGISTER_ENUM(
      enum EchoMode {
         Normal,
         NoEcho,
         Password,
         PasswordEchoOnEdit
      };
   )

   explicit QLineEdit(QWidget *parent = nullptr);
   explicit QLineEdit(const QString &contents, QWidget *parent = nullptr);

   QLineEdit(const QLineEdit &) = delete;
   QLineEdit &operator=(const QLineEdit &) = delete;

   ~QLineEdit();

   QString text() const;

   QString displayText() const;

   QString placeholderText() const;
   void setPlaceholderText(const QString &);

   int maxLength() const;
   void setMaxLength(int);

   void setFrame(bool);
   bool hasFrame() const;

   void setClearButtonEnabled(bool enable);
   bool isClearButtonEnabled() const;

   EchoMode echoMode() const;
   void setEchoMode(EchoMode);

   bool isReadOnly() const;
   void setReadOnly(bool);

#ifndef QT_NO_VALIDATOR
   void setValidator(const QValidator *validator);
   const QValidator *validator() const;
#endif

#ifndef QT_NO_COMPLETER
   void setCompleter(QCompleter *completer);
   QCompleter *completer() const;
#endif

   QSize sizeHint() const override;
   QSize minimumSizeHint() const override;

   int cursorPosition() const;
   void setCursorPosition(int);
   int cursorPositionAt(const QPoint &pos);

   void setAlignment(Qt::Alignment flag);
   Qt::Alignment alignment() const;

   void cursorForward(bool mark, int steps = 1);
   void cursorBackward(bool mark, int steps = 1);
   void cursorWordForward(bool mark);
   void cursorWordBackward(bool mark);
   void backspace();
   void del();
   void home(bool mark);
   void end(bool mark);

   bool isModified() const;
   void setModified(bool);

   void setSelection(int start, int length);
   bool hasSelectedText() const;
   QString selectedText() const;
   int selectionStart() const;

   bool isUndoAvailable() const;
   bool isRedoAvailable() const;

   void setDragEnabled(bool enable);
   bool dragEnabled() const;

   void setCursorMoveStyle(Qt::CursorMoveStyle style);
   Qt::CursorMoveStyle cursorMoveStyle() const;

   QString inputMask() const;
   void setInputMask(const QString &inputMask);
   bool hasAcceptableInput() const;

   void setTextMargins(int left, int top, int right, int bottom);
   void setTextMargins(const QMargins &margins);
   void getTextMargins(int *left, int *top, int *right, int *bottom) const;
   QMargins textMargins() const;

   using QWidget::addAction;
   void addAction(QAction *action, ActionPosition position);
   QAction *addAction(const QIcon &icon, ActionPosition position);

   void deselect();
   void insert(const QString &newText);

#ifndef QT_NO_CONTEXTMENU
   QMenu *createStandardContextMenu();
#endif

   GUI_CS_SLOT_1(Public, void setText(const QString &str))
   GUI_CS_SLOT_2(setText)

   GUI_CS_SLOT_1(Public, void clear())
   GUI_CS_SLOT_2(clear)

   GUI_CS_SLOT_1(Public, void selectAll())
   GUI_CS_SLOT_2(selectAll)

   GUI_CS_SLOT_1(Public, void undo())
   GUI_CS_SLOT_2(undo)

   GUI_CS_SLOT_1(Public, void redo())
   GUI_CS_SLOT_2(redo)

#ifndef QT_NO_CLIPBOARD
   GUI_CS_SLOT_1(Public, void cut())
   GUI_CS_SLOT_2(cut)

   GUI_CS_SLOT_1(Public, void copy() const)
   GUI_CS_SLOT_2(copy)

   GUI_CS_SLOT_1(Public, void paste())
   GUI_CS_SLOT_2(paste)
#endif

   GUI_CS_SIGNAL_1(Public, void textChanged(const QString &newText))
   GUI_CS_SIGNAL_2(textChanged, newText)

   GUI_CS_SIGNAL_1(Public, void textEdited(const QString &newText))
   GUI_CS_SIGNAL_2(textEdited, newText)

   GUI_CS_SIGNAL_1(Public, void cursorPositionChanged(int oldValue, int newValue))
   GUI_CS_SIGNAL_2(cursorPositionChanged, oldValue, newValue)

   GUI_CS_SIGNAL_1(Public, void returnPressed())
   GUI_CS_SIGNAL_2(returnPressed)

   GUI_CS_SIGNAL_1(Public, void editingFinished())
   GUI_CS_SIGNAL_2(editingFinished)

   GUI_CS_SIGNAL_1(Public, void selectionChanged())
   GUI_CS_SIGNAL_2(selectionChanged)

   QVariant inputMethodQuery(Qt::InputMethodQuery query) const override;
   bool event(QEvent *event) override;

 protected:
   void mousePressEvent(QMouseEvent *event) override;
   void mouseMoveEvent(QMouseEvent *event) override;
   void mouseReleaseEvent(QMouseEvent *event) override;
   void mouseDoubleClickEvent(QMouseEvent *event) override;
   void keyPressEvent(QKeyEvent *event) override;
   void focusInEvent(QFocusEvent *event) override;
   void focusOutEvent(QFocusEvent *event) override;
   void paintEvent(QPaintEvent *event) override;

#ifndef QT_NO_DRAGANDDROP
   void dragEnterEvent(QDragEnterEvent *event) override;
   void dragMoveEvent(QDragMoveEvent *event) override;
   void dragLeaveEvent(QDragLeaveEvent *event) override;
   void dropEvent(QDropEvent *event) override;
#endif

   void changeEvent(QEvent *event) override;

#ifndef QT_NO_CONTEXTMENU
   void contextMenuEvent(QContextMenuEvent *event) override;
#endif

   void inputMethodEvent(QInputMethodEvent *event) override;
   void initStyleOption(QStyleOptionFrame *option) const;

 protected:
   QRect cursorRect() const;

 private:
   friend class QAbstractSpinBox;
   friend class QAccessibleLineEdit;

#ifdef QT_KEYPAD_NAVIGATION
   friend class QDateTimeEdit;
#endif

   Q_DECLARE_PRIVATE(QLineEdit)

   GUI_CS_SLOT_1(Private, void _q_handleWindowActivate())
   GUI_CS_SLOT_2(_q_handleWindowActivate)

   GUI_CS_SLOT_1(Private, void _q_textEdited(const QString &newText))
   GUI_CS_SLOT_2(_q_textEdited)

   GUI_CS_SLOT_1(Private, void _q_cursorPositionChanged(int oldValue, int newValue))
   GUI_CS_SLOT_2(_q_cursorPositionChanged)

#ifndef QT_NO_COMPLETER
   GUI_CS_SLOT_1(Private, void _q_completionHighlighted(const QString &text))
   GUI_CS_SLOT_2(_q_completionHighlighted)

#endif

#ifdef QT_KEYPAD_NAVIGATION
   GUI_CS_SLOT_1(Private, void _q_editFocusChange(bool isFocusChanged))
   GUI_CS_SLOT_2(_q_editFocusChange)
#endif

   GUI_CS_SLOT_1(Private, void _q_selectionChanged())
   GUI_CS_SLOT_2(_q_selectionChanged)

   GUI_CS_SLOT_1(Private, void _q_updateNeeded(const QRect &rect))
   GUI_CS_SLOT_2(_q_updateNeeded)

   GUI_CS_SLOT_1(Private, void _q_textChanged(const QString &newText))
   GUI_CS_SLOT_2(_q_textChanged)

   GUI_CS_SLOT_1(Private, void _q_clearButtonClicked())
   GUI_CS_SLOT_2(_q_clearButtonClicked)
};

#endif // QT_NO_LINEEDIT

#endif // QLINEEDIT_H
