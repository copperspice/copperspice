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

#ifndef QLINECONTROL_P_H
#define QLINECONTROL_P_H

#include <qglobal.h>

#ifndef QT_NO_LINEEDIT

#include <qlineedit.h>
#include <qtextlayout.h>
#include <qstyleoption.h>
#include <qpointer.h>
#include <qclipboard.h>
#include <qinputmethod.h>
#include <qpoint.h>
#include <qcompleter.h>
#include <qthread.h>
#include <qplatformdefs.h>

#include <qinputcontrol_p.h>
#include <qtextengine_p.h>
#include <qwidget_p.h>

#ifdef DrawText
#  undef DrawText
#endif

class Q_GUI_EXPORT QLineControl : public QInputControl
{
   GUI_CS_OBJECT(QLineControl)

 public:
   QLineControl(const QString &txt = QString())
      : QInputControl(LineEdit),
        m_cursor(0), m_preeditCursor(0), m_cursorWidth(0), m_layoutDirection(Qt::LayoutDirectionAuto),
        m_hideCursor(false), m_separator(false), m_readOnly(false), m_dragEnabled(false),
        m_echoMode(0), m_textDirty(false), m_selDirty(false),
        m_validInput(true), m_blinkStatus(false), m_blinkPeriod(0), m_blinkTimer(0),
        m_deleteAllTimer(0), m_ascent(0), m_maxLength(32767), m_lastCursorPos(-1),
        m_tripleClickTimer(0), m_keyboardScheme(0), m_accessibleObject(nullptr),
        m_maskData(nullptr), m_modifiedState(0), m_undoState(0), m_selstart(0), m_selend(0),
        m_passwordEchoEditing(false), m_passwordEchoTimer(0),  m_passwordMaskDelay(-1)
   {
      init(txt);
   }

   ~QLineControl() {
      delete [] m_maskData;
   }

   void setAccessibleObject(QObject *object) {
      Q_ASSERT(object);
      m_accessibleObject = object;
   }

   QObject *accessibleObject() {
      if (m_accessibleObject) {
         return m_accessibleObject;
      }
      return parent();
   }

   int nextMaskBlank(int pos) {
      int c = findInMask(pos, true, false);
      m_separator |= (c != pos);
      return (c != -1 ?  c : m_maxLength);
   }

   int prevMaskBlank(int pos) {
      int c = findInMask(pos, false, false);
      m_separator |= (c != pos);
      return (c != -1 ? c : 0);
   }

   bool isUndoAvailable() const;
   bool isRedoAvailable() const;

   void clearUndo() {
      m_history.clear();
      m_modifiedState = 0;
      m_undoState     = 0;
   }

   bool isModified() const {
      return m_modifiedState != m_undoState;
   }

   void setModified(bool modified) {
      m_modifiedState = modified ? -1 : m_undoState;
   }

   bool allSelected() const {
      return !m_text.isEmpty() && m_selstart == 0 && m_selend == (int)m_text.length();
   }

   bool hasSelectedText() const {
      return !m_text.isEmpty() && m_selend > m_selstart;
   }

   int width() const {
      return qRound(m_textLayout.lineAt(0).width()) + 1;
   }

   int height() const {
      return qRound(m_textLayout.lineAt(0).height()) + 1;
   }

   int ascent() const {
      return m_ascent;
   }

   qreal naturalTextWidth() const {
      return m_textLayout.lineAt(0).naturalTextWidth();
   }

   void setSelection(int start, int length);

   QString selectedText() const {
      return hasSelectedText() ? m_text.mid(m_selstart, m_selend - m_selstart) : QString();
   }

   QString textBeforeSelection() const {
      return hasSelectedText() ? m_text.left(m_selstart) : QString();
   }
   QString textAfterSelection() const {
      return hasSelectedText() ? m_text.mid(m_selend) : QString();
   }

   int selectionStart() const {
      return hasSelectedText() ? m_selstart : -1;
   }

   int selectionEnd() const {
      return hasSelectedText() ? m_selend : -1;
   }

   bool inSelection(int x) const {
      if (m_selstart >= m_selend) {
         return false;
      }
      int pos = xToPos(x, QTextLine::CursorOnCharacter);
      return pos >= m_selstart && pos < m_selend;
   }

   void removeSelection() {
      int priorState = m_undoState;
      removeSelectedText();
      finishChange(priorState);
   }

   int start() const {
      return 0;
   }

   int end() const {
      return m_text.length();
   }

#ifndef QT_NO_CLIPBOARD
   void copy(QClipboard::Mode mode = QClipboard::Clipboard) const;
   void paste(QClipboard::Mode mode = QClipboard::Clipboard);
#endif

   int cursor() const {
      return m_cursor;
   }
   int preeditCursor() const {
      return m_preeditCursor;
   }

   int cursorWidth() const {
      return m_cursorWidth;
   }
   void setCursorWidth(int value) {
      m_cursorWidth = value;
   }

   Qt::CursorMoveStyle cursorMoveStyle() const {
      return m_textLayout.cursorMoveStyle();
   }

   void setCursorMoveStyle(Qt::CursorMoveStyle style) {
      m_textLayout.setCursorMoveStyle(style);
   }

   void moveCursor(int pos, bool mark = false);
   void cursorForward(bool mark, int steps) {
      int c = m_cursor;
      if (steps > 0) {
         while (steps--)
            c = cursorMoveStyle() == Qt::VisualMoveStyle ? m_textLayout.rightCursorPosition(c)
               : m_textLayout.nextCursorPosition(c);
      } else if (steps < 0) {
         while (steps++)
            c = cursorMoveStyle() == Qt::VisualMoveStyle ? m_textLayout.leftCursorPosition(c)
               : m_textLayout.previousCursorPosition(c);
      }
      moveCursor(c, mark);
   }

   void cursorWordForward(bool mark) {
      moveCursor(m_textLayout.nextCursorPosition(m_cursor, QTextLayout::SkipWords), mark);
   }

   void cursorWordBackward(bool mark) {
      moveCursor(m_textLayout.previousCursorPosition(m_cursor, QTextLayout::SkipWords), mark);
   }

   void home(bool mark) {
      moveCursor(0, mark);
   }

   void end(bool mark) {
      moveCursor(text().length(), mark);
   }

   int xToPos(int x, QTextLine::CursorPosition = QTextLine::CursorBetweenCharacters) const;
   QRect cursorRect() const;

   qreal cursorToX(int cursor) const {
      return m_textLayout.lineAt(0).cursorToX(cursor);
   }

   qreal cursorToX() const {
      int cursor = m_cursor;
      if (m_preeditCursor != -1) {
         cursor += m_preeditCursor;
      }
      return cursorToX(cursor);
   }

   bool isReadOnly() const {
      return m_readOnly;
   }

   void setReadOnly(bool enable);

   QString text() const {
      QString res = m_maskData ? stripString(m_text) : m_text;
      return (res.isEmpty() ? QString("") : res);
   }

   void setText(const QString &txt) {
#ifndef QT_NO_IM
      if (composeMode()) {
         QGuiApplication::inputMethod()->reset();
      }
#endif
      internalSetText(txt, -1, false);
   }

   void commitPreedit();

   QString displayText() const {
      return m_textLayout.text();
   }

   void backspace();
   void del();
   void deselect() {
      internalDeselect();
      finishChange();
   }

   void selectAll() {
      m_selstart = 0;
      m_selend   = 0;
      m_cursor   = 0;

      moveCursor(m_text.length(), true);
   }

   void insert(const QString &);
   void clear();

   void undo();

   void redo() {
      internalRedo();
      finishChange();
   }
   void selectWordAtPos(int);

   uint echoMode() const {
      return m_echoMode;
   }
   void setEchoMode(uint mode) {
      cancelPasswordEchoTimer();
      m_echoMode = mode;
      m_passwordEchoEditing = false;
      updateDisplayText();
   }

   int maxLength() const {
      return m_maxLength;
   }

   void setMaxLength(int maxLength) {
      if (m_maskData) {
         return;
      }
      m_maxLength = maxLength;
      setText(m_text);
   }

#ifndef QT_NO_VALIDATOR
   const QValidator *validator() const {
      return m_validator;
   }

   void setValidator(const QValidator *v) {
      m_validator = const_cast<QValidator *>(v);
   }
#endif

#ifndef QT_NO_COMPLETER
   QCompleter *completer() const {
      return m_completer;
   }

   /* Note that you must set the widget for the completer separately */
   void setCompleter(const QCompleter *c) {
      m_completer = const_cast<QCompleter *>(c);
   }

   void complete(int key);
#endif

   int cursorPosition() const {
      return m_cursor;
   }

   void setCursorPosition(int pos) {
      if (pos <= m_text.length()) {
         moveCursor(qMax(0, pos));
      }
   }

   bool hasAcceptableInput() const {
      return hasAcceptableInput(m_text);
   }

   bool fixup();

   QString inputMask() const {
      QString mask;

      if (m_maskData) {
         mask = m_inputMask;
         if (m_blank != QLatin1Char(' ')) {
            mask += QLatin1Char(';');
            mask += m_blank;
         }
      }
      return mask;
   }

   void setInputMask(const QString &mask) {
      parseInputMask(mask);
      if (m_maskData) {
         moveCursor(nextMaskBlank(0));
      }
   }

   // input methods
#ifndef QT_NO_IM
   bool composeMode() const {
      return !m_textLayout.preeditAreaText().isEmpty();
   }

   void setPreeditArea(int cursor, const QString &text) {
      m_textLayout.setPreeditArea(cursor, text);
   }
#endif

   QString preeditAreaText() const {
      return m_textLayout.preeditAreaText();
   }

   void updatePasswordEchoEditing(bool editing);

   bool passwordEchoEditing() const {

      if (m_passwordEchoTimer != 0) {
         return true;
      }

      return m_passwordEchoEditing ;
   }

   QChar passwordCharacter() const {
      return m_passwordCharacter;
   }

   void setPasswordCharacter(const QChar &character) {
      m_passwordCharacter = character;
      updateDisplayText();
   }

   int passwordMaskDelay() const {
      return m_passwordMaskDelay;
   }
   void setPasswordMaskDelay(int delay) {
      m_passwordMaskDelay = delay;
   }

   Qt::LayoutDirection layoutDirection() const {
      if (m_layoutDirection == Qt::LayoutDirectionAuto) {
         if (m_text.isEmpty()) {
            return QGuiApplication::inputMethod()->inputDirection();
         }

         if (QTextEngine::isRightToLeft(m_text)) {
            return Qt::RightToLeft;
         } else {
            return Qt::LeftToRight;
         }
      }

      return m_layoutDirection;
   }

   void setLayoutDirection(Qt::LayoutDirection direction) {
      if (direction != m_layoutDirection) {
         m_layoutDirection = direction;
         updateDisplayText();
      }
   }

   void setFont(const QFont &font) {
      m_textLayout.setFont(font);
      updateDisplayText();
   }

   void processInputMethodEvent(QInputMethodEvent *event);

   void processKeyEvent(QKeyEvent *event);

   int cursorBlinkPeriod() const {
      return m_blinkPeriod;
   }

   void setCursorBlinkPeriod(int msec);
   void resetCursorBlinkTimer();

   bool cursorBlinkStatus() const {
      return m_blinkStatus;
   }

   QString cancelText() const {
      return m_cancelText;
   }
   void setCancelText(const QString &text) {
      m_cancelText = text;
   }

   const QPalette &palette() const {
      return m_palette;
   }
   void setPalette(const QPalette &p) {
      m_palette = p;
   }

   enum DrawFlags {
      DrawText = 0x01,
      DrawSelections = 0x02,
      DrawCursor = 0x04,
      DrawAll = DrawText | DrawSelections | DrawCursor
   };
   void draw(QPainter *, const QPoint &, const QRect &, int flags = DrawAll);

#ifndef QT_NO_SHORTCUT
   void processShortcutOverrideEvent(QKeyEvent *ke);
#endif

   QTextLayout *textLayout() const {

      return &m_textLayout;
   }

   GUI_CS_SIGNAL_1(Public, void cursorPositionChanged(int oldpos, int newpos))
   GUI_CS_SIGNAL_2(cursorPositionChanged, oldpos, newpos)

   GUI_CS_SIGNAL_1(Public, void selectionChanged())
   GUI_CS_SIGNAL_2(selectionChanged)

   GUI_CS_SIGNAL_1(Public, void displayTextChanged(const QString &text))
   GUI_CS_SIGNAL_2(displayTextChanged, text)

   GUI_CS_SIGNAL_1(Public, void textChanged(const QString &text))
   GUI_CS_SIGNAL_2(textChanged, text)

   GUI_CS_SIGNAL_1(Public, void textEdited(const QString &text))
   GUI_CS_SIGNAL_2(textEdited, text)

   GUI_CS_SIGNAL_1(Public, void resetInputContext())
   GUI_CS_SIGNAL_2(resetInputContext)

   GUI_CS_SIGNAL_1(Public, void updateMicroFocus())
   GUI_CS_SIGNAL_2(updateMicroFocus)

   GUI_CS_SIGNAL_1(Public, void accepted())
   GUI_CS_SIGNAL_2(accepted)

   GUI_CS_SIGNAL_1(Public, void editingFinished())
   GUI_CS_SIGNAL_2(editingFinished)

   GUI_CS_SIGNAL_1(Public, void updateNeeded(const QRect &rect))
   GUI_CS_SIGNAL_2(updateNeeded, rect)

#ifdef QT_KEYPAD_NAVIGATION
   GUI_CS_SIGNAL_1(Public, void editFocusChange(bool isFocusChanged))
   GUI_CS_SIGNAL_2(editFocusChange, isFocusChanged)
#endif

 protected:
   void timerEvent(QTimerEvent *event) override;

 private:
   void init(const QString &txt);
   void removeSelectedText();
   void internalSetText(const QString &txt, int pos = -1, bool edited = true);
   void updateDisplayText(bool forceUpdate = false);

   void internalInsert(const QString &s);
   void internalDelete(bool wasBackspace = false);
   void internalRemove(int pos);

   inline void internalDeselect() {
      m_selDirty |= (m_selend > m_selstart);
      m_selstart = m_selend = 0;
   }

   void internalUndo(int until = -1);
   void internalRedo();

   QString m_text;
   QPalette m_palette;
   int m_cursor;
   int m_preeditCursor;
   int m_cursorWidth;
   Qt::LayoutDirection m_layoutDirection;
   uint m_hideCursor : 1; // used to hide the m_cursor inside preedit areas
   uint m_separator : 1;
   uint m_readOnly : 1;
   uint m_dragEnabled : 1;
   uint m_echoMode : 2;
   uint m_textDirty : 1;
   uint m_selDirty : 1;
   uint m_validInput : 1;
   uint m_blinkStatus : 1;
   int m_blinkPeriod; // 0 for non-blinking cursor
   int m_blinkTimer;
   int m_deleteAllTimer;
   int m_ascent;
   int m_maxLength;
   int m_lastCursorPos;

   QList<int> m_transactions;
   QPoint m_tripleClick;
   int m_tripleClickTimer;
   QString m_cancelText;

   int m_keyboardScheme;

   // accessibility events are sent for this object
   QObject *m_accessibleObject;

   void emitCursorPositionChanged();

   bool finishChange(int validateFromState = -1, bool update = false, bool edited = true);

#ifndef QT_NO_VALIDATOR
   QPointer<QValidator> m_validator;
#endif

   QPointer<QCompleter> m_completer;

#ifndef QT_NO_COMPLETER
   bool advanceToEnabledItem(int dir);
#endif

   struct MaskInputData {
      enum Casemode { NoCaseMode, Upper, Lower };
      QChar maskChar; // either the separator char or the inputmask
      bool separator;
      Casemode caseMode;
   };

   QString m_inputMask;
   QChar m_blank;
   MaskInputData *m_maskData;

   // undo/redo handling
   enum CommandType { Separator, Insert, Remove, Delete, RemoveSelection, DeleteSelection, SetSelection };
   struct Command {

      inline Command(CommandType t, int p, QChar c, int ss, int se) : type(t), uc(c), pos(p), selStart(ss), selEnd(se) {}
      uint type : 4;
      QChar uc;
      int pos, selStart, selEnd;
   };
   int m_modifiedState;
   int m_undoState;
   QVector<Command> m_history;
   void addCommand(const Command &cmd);

   inline void separate() {
      m_separator = true;
   }

   // selection
   int m_selstart;
   int m_selend;

   // masking
   void parseInputMask(const QString &maskFields);
   bool isValidInput(QChar key, QChar mask) const;
   bool hasAcceptableInput(const QString &text) const;
   QString maskString(uint pos, const QString &str, bool clear = false) const;
   QString clearString(uint pos, uint len) const;
   QString stripString(const QString &str) const;
   int findInMask(int pos, bool forward, bool findSeparator, QChar searchChar = QChar()) const;

   // complex text layout
   mutable QTextLayout m_textLayout;

   bool m_passwordEchoEditing;
   QChar m_passwordCharacter;

   int m_passwordEchoTimer;
   int m_passwordMaskDelay;

   void cancelPasswordEchoTimer() {

      if (m_passwordEchoTimer != 0) {
         killTimer(m_passwordEchoTimer);
         m_passwordEchoTimer = 0;
      }
   }

   int redoTextLayout() const;

   GUI_CS_SLOT_1(Private, void _q_clipboardChanged())
   GUI_CS_SLOT_2(_q_clipboardChanged)

   GUI_CS_SLOT_1(Private, void _q_deleteSelected())
   GUI_CS_SLOT_2(_q_deleteSelected)

 private:
    friend class QLineEdit;
};

#endif // QT_NO_LINEEDIT

#endif
