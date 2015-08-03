/***********************************************************************
*
* Copyright (c) 2012-2015 Barbara Geller
* Copyright (c) 2012-2015 Ansel Sermersheim
* Copyright (c) 2012-2014 Digia Plc and/or its subsidiary(-ies).
* Copyright (c) 2008-2012 Nokia Corporation and/or its subsidiary(-ies).
* All rights reserved.
*
* This file is part of CopperSpice.
*
* CopperSpice is free software: you can redistribute it and/or 
* modify it under the terms of the GNU Lesser General Public License
* version 2.1 as published by the Free Software Foundation.
*
* CopperSpice is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
* Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public
* License along with CopperSpice.  If not, see 
* <http://www.gnu.org/licenses/>.
*
***********************************************************************/

#ifndef QDECLARATIVETEXTINPUT_P_H
#define QDECLARATIVETEXTINPUT_P_H

#include <qdeclarativetext_p.h>
#include <qdeclarativeimplicitsizeitem_p.h>
#include <QGraphicsSceneMouseEvent>
#include <QIntValidator>

#ifndef QT_NO_LINEEDIT

QT_BEGIN_NAMESPACE

class QDeclarativeTextInputPrivate;
class QValidator;

class QDeclarativeTextInput : public QDeclarativeImplicitSizePaintedItem
{
   DECL_CS_OBJECT(QDeclarativeTextInput)

   CS_ENUM(HAlignment)
   CS_ENUM(EchoMode)
   CS_ENUM(SelectionMode)

   CS_PROPERTY_READ(text, text)
   CS_PROPERTY_WRITE(text, setText)
   CS_PROPERTY_NOTIFY(text, textChanged)
   CS_PROPERTY_READ(color, color)
   CS_PROPERTY_WRITE(color, setColor)
   CS_PROPERTY_NOTIFY(color, colorChanged)
   CS_PROPERTY_READ(selectionColor, selectionColor)
   CS_PROPERTY_WRITE(selectionColor, setSelectionColor)
   CS_PROPERTY_NOTIFY(selectionColor, selectionColorChanged)
   CS_PROPERTY_READ(selectedTextColor, selectedTextColor)
   CS_PROPERTY_WRITE(selectedTextColor, setSelectedTextColor)
   CS_PROPERTY_NOTIFY(selectedTextColor, selectedTextColorChanged)
   CS_PROPERTY_READ(font, font)
   CS_PROPERTY_WRITE(font, setFont)
   CS_PROPERTY_NOTIFY(font, fontChanged)
   CS_PROPERTY_READ(horizontalAlignment, hAlign)
   CS_PROPERTY_WRITE(horizontalAlignment, setHAlign)
   CS_PROPERTY_RESET(horizontalAlignment, resetHAlign)
   CS_PROPERTY_NOTIFY(horizontalAlignment, horizontalAlignmentChanged)
   CS_PROPERTY_READ(readOnly, isReadOnly)
   CS_PROPERTY_WRITE(readOnly, setReadOnly)
   CS_PROPERTY_NOTIFY(readOnly, readOnlyChanged)
   CS_PROPERTY_READ(cursorVisible, isCursorVisible)
   CS_PROPERTY_WRITE(cursorVisible, setCursorVisible)
   CS_PROPERTY_NOTIFY(cursorVisible, cursorVisibleChanged)
   CS_PROPERTY_READ(cursorPosition, cursorPosition)
   CS_PROPERTY_WRITE(cursorPosition, setCursorPosition)
   CS_PROPERTY_NOTIFY(cursorPosition, cursorPositionChanged)
   CS_PROPERTY_READ(cursorRectangle, cursorRectangle)
   CS_PROPERTY_NOTIFY(cursorRectangle, cursorRectangleChanged)
   CS_PROPERTY_READ(*cursorDelegate, cursorDelegate)
   CS_PROPERTY_WRITE(*cursorDelegate, setCursorDelegate)
   CS_PROPERTY_NOTIFY(*cursorDelegate, cursorDelegateChanged)
   CS_PROPERTY_READ(selectionStart, selectionStart)
   CS_PROPERTY_NOTIFY(selectionStart, selectionStartChanged)
   CS_PROPERTY_READ(selectionEnd, selectionEnd)
   CS_PROPERTY_NOTIFY(selectionEnd, selectionEndChanged)
   CS_PROPERTY_READ(selectedText, selectedText)
   CS_PROPERTY_NOTIFY(selectedText, selectedTextChanged)

   CS_PROPERTY_READ(maximumLength, maxLength)
   CS_PROPERTY_WRITE(maximumLength, setMaxLength)
   CS_PROPERTY_NOTIFY(maximumLength, maximumLengthChanged)
#ifndef QT_NO_VALIDATOR
   CS_PROPERTY_READ(validator, validator)
   CS_PROPERTY_WRITE(validator, setValidator)
   CS_PROPERTY_NOTIFY(validator, validatorChanged)
#endif
   CS_PROPERTY_READ(inputMask, inputMask)
   CS_PROPERTY_WRITE(inputMask, setInputMask)
   CS_PROPERTY_NOTIFY(inputMask, inputMaskChanged)
   CS_PROPERTY_READ(inputMethodHints, imHints)
   CS_PROPERTY_WRITE(inputMethodHints, setIMHints)

   CS_PROPERTY_READ(acceptableInput, hasAcceptableInput)
   CS_PROPERTY_NOTIFY(acceptableInput, acceptableInputChanged)
   CS_PROPERTY_READ(echoMode, echoMode)
   CS_PROPERTY_WRITE(echoMode, setEchoMode)
   CS_PROPERTY_NOTIFY(echoMode, echoModeChanged)
   CS_PROPERTY_READ(activeFocusOnPress, focusOnPress)
   CS_PROPERTY_WRITE(activeFocusOnPress, setFocusOnPress)
   CS_PROPERTY_NOTIFY(activeFocusOnPress, activeFocusOnPressChanged)
   CS_PROPERTY_READ(passwordCharacter, passwordCharacter)
   CS_PROPERTY_WRITE(passwordCharacter, setPasswordCharacter)
   CS_PROPERTY_NOTIFY(passwordCharacter, passwordCharacterChanged)
   CS_PROPERTY_READ(displayText, displayText)
   CS_PROPERTY_NOTIFY(displayText, displayTextChanged)
   CS_PROPERTY_READ(autoScroll, autoScroll)
   CS_PROPERTY_WRITE(autoScroll, setAutoScroll)
   CS_PROPERTY_NOTIFY(autoScroll, autoScrollChanged)
   CS_PROPERTY_READ(selectByMouse, selectByMouse)
   CS_PROPERTY_WRITE(selectByMouse, setSelectByMouse)
   CS_PROPERTY_NOTIFY(selectByMouse, selectByMouseChanged)
   CS_PROPERTY_READ(mouseSelectionMode, mouseSelectionMode)
   CS_PROPERTY_WRITE(mouseSelectionMode, setMouseSelectionMode)
   CS_PROPERTY_NOTIFY(mouseSelectionMode, mouseSelectionModeChanged)
   CS_PROPERTY_REVISION(mouseSelectionMode, 1)
   CS_PROPERTY_READ(canPaste, canPaste)
   CS_PROPERTY_NOTIFY(canPaste, canPasteChanged)
   CS_PROPERTY_REVISION(canPaste, 1)
   CS_PROPERTY_READ(inputMethodComposing, isInputMethodComposing)
   CS_PROPERTY_NOTIFY(inputMethodComposing, inputMethodComposingChanged)
   CS_PROPERTY_REVISION(inputMethodComposing, 1)

 public:
   QDeclarativeTextInput(QDeclarativeItem *parent = 0);
   ~QDeclarativeTextInput();

   enum EchoMode {//To match QLineEdit::EchoMode
      Normal,
      NoEcho,
      Password,
      PasswordEchoOnEdit
   };

   enum HAlignment {
      AlignLeft = Qt::AlignLeft,
      AlignRight = Qt::AlignRight,
      AlignHCenter = Qt::AlignHCenter
   };

   enum SelectionMode {
      SelectCharacters,
      SelectWords
   };

   enum CursorPosition {
      CursorBetweenCharacters,
      CursorOnCharacter
   };

   //Auxilliary functions needed to control the TextInput from QML
   CS_INVOKABLE_METHOD_1(Public, int positionAt(int x) const)
   CS_INVOKABLE_METHOD_OVERLOAD(positionAt, (int))

   CS_INVOKABLE_METHOD_1(Public, int positionAt(int x, CursorPosition position) const)
   CS_INVOKABLE_METHOD_OVERLOAD(positionAt, (int, CursorPosition))
   CS_REVISION_OVERLOAD(positionAt, 1, (int, CursorPosition))

   CS_INVOKABLE_METHOD_1(Public, QRectF positionToRectangle(int pos) const)
   CS_INVOKABLE_METHOD_2(positionToRectangle)

   CS_INVOKABLE_METHOD_1(Public, void moveCursorSelection(int pos))
   CS_INVOKABLE_METHOD_OVERLOAD(moveCursorSelection, (int))

   CS_INVOKABLE_METHOD_1(Public, void moveCursorSelection(int pos, SelectionMode mode))
   CS_INVOKABLE_METHOD_OVERLOAD(moveCursorSelection, (int, SelectionMode))
   CS_REVISION_OVERLOAD(moveCursorSelection, 1, (int, SelectionMode))

   CS_INVOKABLE_METHOD_1(Public, void openSoftwareInputPanel())
   CS_INVOKABLE_METHOD_2(openSoftwareInputPanel)

   CS_INVOKABLE_METHOD_1(Public, void closeSoftwareInputPanel())
   CS_INVOKABLE_METHOD_2(closeSoftwareInputPanel)

   QString text() const;
   void setText(const QString &);

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

   bool isReadOnly() const;
   void setReadOnly(bool);

   bool isCursorVisible() const;
   void setCursorVisible(bool on);

   int cursorPosition() const;
   void setCursorPosition(int cp);

   QRect cursorRectangle() const;

   int selectionStart() const;
   int selectionEnd() const;

   QString selectedText() const;

   int maxLength() const;
   void setMaxLength(int ml);

#ifndef QT_NO_VALIDATOR
   QValidator *validator() const;
   void setValidator(QValidator *v);
#endif

   QString inputMask() const;
   void setInputMask(const QString &im);

   EchoMode echoMode() const;
   void setEchoMode(EchoMode echo);

   QString passwordCharacter() const;
   void setPasswordCharacter(const QString &str);

   QString displayText() const;

   QDeclarativeComponent *cursorDelegate() const;
   void setCursorDelegate(QDeclarativeComponent *);

   bool focusOnPress() const;
   void setFocusOnPress(bool);

   bool autoScroll() const;
   void setAutoScroll(bool);

   bool selectByMouse() const;
   void setSelectByMouse(bool);

   SelectionMode mouseSelectionMode() const;
   void setMouseSelectionMode(SelectionMode mode);

   bool hasAcceptableInput() const;

   void drawContents(QPainter *p, const QRect &r);
   QVariant inputMethodQuery(Qt::InputMethodQuery property) const;

   QRectF boundingRect() const;
   bool canPaste() const;

   bool isInputMethodComposing() const;

   Qt::InputMethodHints imHints() const;
   void setIMHints(Qt::InputMethodHints hints);

   CS_SIGNAL_1(Public, void textChanged())
   CS_SIGNAL_2(textChanged)
   CS_SIGNAL_1(Public, void cursorPositionChanged())
   CS_SIGNAL_2(cursorPositionChanged)
   CS_SIGNAL_1(Public, void cursorRectangleChanged())
   CS_SIGNAL_2(cursorRectangleChanged)
   CS_SIGNAL_1(Public, void selectionStartChanged())
   CS_SIGNAL_2(selectionStartChanged)
   CS_SIGNAL_1(Public, void selectionEndChanged())
   CS_SIGNAL_2(selectionEndChanged)
   CS_SIGNAL_1(Public, void selectedTextChanged())
   CS_SIGNAL_2(selectedTextChanged)
   CS_SIGNAL_1(Public, void accepted())
   CS_SIGNAL_2(accepted)
   CS_SIGNAL_1(Public, void acceptableInputChanged())
   CS_SIGNAL_2(acceptableInputChanged)
   CS_SIGNAL_1(Public, void colorChanged(const QColor &color))
   CS_SIGNAL_2(colorChanged, color)
   CS_SIGNAL_1(Public, void selectionColorChanged(const QColor &color))
   CS_SIGNAL_2(selectionColorChanged, color)
   CS_SIGNAL_1(Public, void selectedTextColorChanged(const QColor &color))
   CS_SIGNAL_2(selectedTextColorChanged, color)
   CS_SIGNAL_1(Public, void fontChanged(const QFont &font))
   CS_SIGNAL_2(fontChanged, font)
   CS_SIGNAL_1(Public, void horizontalAlignmentChanged(HAlignment alignment))
   CS_SIGNAL_2(horizontalAlignmentChanged, alignment)
   CS_SIGNAL_1(Public, void readOnlyChanged(bool isReadOnly))
   CS_SIGNAL_2(readOnlyChanged, isReadOnly)
   CS_SIGNAL_1(Public, void cursorVisibleChanged(bool isCursorVisible))
   CS_SIGNAL_2(cursorVisibleChanged, isCursorVisible)
   CS_SIGNAL_1(Public, void cursorDelegateChanged())
   CS_SIGNAL_2(cursorDelegateChanged)
   CS_SIGNAL_1(Public, void maximumLengthChanged(int maximumLength))
   CS_SIGNAL_2(maximumLengthChanged, maximumLength)
   CS_SIGNAL_1(Public, void validatorChanged())
   CS_SIGNAL_2(validatorChanged)
   CS_SIGNAL_1(Public, void inputMaskChanged(const QString &inputMask))
   CS_SIGNAL_2(inputMaskChanged, inputMask)
   CS_SIGNAL_1(Public, void echoModeChanged(EchoMode echoMode))
   CS_SIGNAL_2(echoModeChanged, echoMode)
   CS_SIGNAL_1(Public, void passwordCharacterChanged())
   CS_SIGNAL_2(passwordCharacterChanged)
   CS_SIGNAL_1(Public, void displayTextChanged())
   CS_SIGNAL_2(displayTextChanged)
   CS_SIGNAL_1(Public, void activeFocusOnPressChanged(bool activeFocusOnPress))
   CS_SIGNAL_2(activeFocusOnPressChanged, activeFocusOnPress)
   CS_SIGNAL_1(Public, void autoScrollChanged(bool autoScroll))
   CS_SIGNAL_2(autoScrollChanged, autoScroll)
   CS_SIGNAL_1(Public, void selectByMouseChanged(bool selectByMouse))
   CS_SIGNAL_2(selectByMouseChanged, selectByMouse)

   CS_SIGNAL_1(Public, void mouseSelectionModeChanged(SelectionMode mode))
   CS_SIGNAL_2(mouseSelectionModeChanged, mode)
   CS_REVISION(mouseSelectionModeChanged, 1)

   CS_SIGNAL_1(Public, void canPasteChanged())
   CS_SIGNAL_2(canPasteChanged)
   CS_REVISION(canPasteChanged, 1)

   CS_SIGNAL_1(Public, void inputMethodComposingChanged())
   CS_SIGNAL_2(inputMethodComposingChanged)
   CS_REVISION(inputMethodComposingChanged, 1)

 public :
   CS_SLOT_1(Public, void selectAll())
   CS_SLOT_2(selectAll)

   CS_SLOT_1(Public, void selectWord())
   CS_SLOT_2(selectWord)

   CS_SLOT_1(Public, void select(int start, int end))
   CS_SLOT_2(select)

   CS_SLOT_1(Public, void deselect())
   CS_SLOT_2(deselect)
   CS_REVISION(deselect, 1)

   CS_SLOT_1(Public, bool isRightToLeft(int start, int end))
   CS_SLOT_2(isRightToLeft)
   CS_REVISION(isRightToLeft, 1)

#ifndef QT_NO_CLIPBOARD
   CS_SLOT_1(Public, void cut())
   CS_SLOT_2(cut)
   CS_SLOT_1(Public, void copy())
   CS_SLOT_2(copy)
   CS_SLOT_1(Public, void paste())
   CS_SLOT_2(paste)
#endif

 protected:
   virtual void geometryChanged(const QRectF &newGeometry, const QRectF &oldGeometry);

   void mousePressEvent(QGraphicsSceneMouseEvent *event);
   void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
   void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
   void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event);
   bool sceneEvent(QEvent *event);
   void keyPressEvent(QKeyEvent *ev);
   void inputMethodEvent(QInputMethodEvent *);
   bool event(QEvent *e);
   void focusInEvent(QFocusEvent *event);

 private :
   CS_SLOT_1(Private, void updateSize(bool needsRedraw = true))
   CS_SLOT_2(updateSize)
   CS_SLOT_1(Private, void q_textChanged())
   CS_SLOT_2(q_textChanged)
   CS_SLOT_1(Private, void selectionChanged())
   CS_SLOT_2(selectionChanged)
   CS_SLOT_1(Private, void createCursor())
   CS_SLOT_2(createCursor)
   CS_SLOT_1(Private, void cursorPosChanged())
   CS_SLOT_2(cursorPosChanged)
   CS_SLOT_1(Private, void updateCursorRectangle())
   CS_SLOT_2(updateCursorRectangle)
   CS_SLOT_1(Private, void updateRect(const QRect &r = QRect()))
   CS_SLOT_2(updateRect)
   CS_SLOT_1(Private, void q_canPasteChanged())
   CS_SLOT_2(q_canPasteChanged)

 private:
   Q_DECLARE_PRIVATE_D(QGraphicsItem::d_ptr.data(), QDeclarativeTextInput)
};

QT_END_NAMESPACE

QML_DECLARE_TYPE(QDeclarativeTextInput)
#ifndef QT_NO_VALIDATOR
QML_DECLARE_TYPE(QValidator)
QML_DECLARE_TYPE(QIntValidator)
QML_DECLARE_TYPE(QDoubleValidator)
QML_DECLARE_TYPE(QRegExpValidator)
#endif

#endif // QT_NO_LINEEDIT

#endif // QDECLARATIVETEXTINPUT_H
