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

#ifndef QABSTRACTSPINBOX_H
#define QABSTRACTSPINBOX_H

#include <qvalidator.h>
#include <qwidget.h>

#ifndef QT_NO_SPINBOX

class QLineEdit;
class QAbstractSpinBoxPrivate;
class QStyleOptionSpinBox;

class Q_GUI_EXPORT QAbstractSpinBox : public QWidget
{
   GUI_CS_OBJECT(QAbstractSpinBox)

   GUI_CS_ENUM(ButtonSymbols)
   GUI_CS_ENUM(CorrectionMode)

   GUI_CS_PROPERTY_READ(wrapping, wrapping)
   GUI_CS_PROPERTY_WRITE(wrapping, setWrapping)

   GUI_CS_PROPERTY_READ(frame, hasFrame)
   GUI_CS_PROPERTY_WRITE(frame, setFrame)

   GUI_CS_PROPERTY_READ(alignment, alignment)
   GUI_CS_PROPERTY_WRITE(alignment, setAlignment)

   GUI_CS_PROPERTY_READ(readOnly, isReadOnly)
   GUI_CS_PROPERTY_WRITE(readOnly, setReadOnly)

   GUI_CS_PROPERTY_READ(buttonSymbols, buttonSymbols)
   GUI_CS_PROPERTY_WRITE(buttonSymbols, setButtonSymbols)

   GUI_CS_PROPERTY_READ(specialValueText, specialValueText)
   GUI_CS_PROPERTY_WRITE(specialValueText, setSpecialValueText)

   GUI_CS_PROPERTY_READ(text, text)

   GUI_CS_PROPERTY_READ(accelerated, isAccelerated)
   GUI_CS_PROPERTY_WRITE(accelerated, setAccelerated)

   GUI_CS_PROPERTY_READ(correctionMode, correctionMode)
   GUI_CS_PROPERTY_WRITE(correctionMode, setCorrectionMode)

   GUI_CS_PROPERTY_READ(acceptableInput, hasAcceptableInput)

   GUI_CS_PROPERTY_READ(keyboardTracking, keyboardTracking)
   GUI_CS_PROPERTY_WRITE(keyboardTracking, setKeyboardTracking)

 public:
   enum StepEnabledFlag {
      StepNone        = 0x00,
      StepUpEnabled   = 0x01,
      StepDownEnabled = 0x02
   };
   using StepEnabled = QFlags<StepEnabledFlag>;

   GUI_CS_REGISTER_ENUM(
      enum ButtonSymbols  {
         UpDownArrows,
         PlusMinus,
         NoButtons
      };
   )

   GUI_CS_REGISTER_ENUM(
      enum CorrectionMode {
         CorrectToPreviousValue,
         CorrectToNearestValue
      };
   )

   explicit QAbstractSpinBox(QWidget *parent = nullptr);

   QAbstractSpinBox(const QAbstractSpinBox &) = delete;
   QAbstractSpinBox &operator=(const QAbstractSpinBox &) = delete;

   ~QAbstractSpinBox();

   bool hasAcceptableInput() const;
   QString text() const;

   void setButtonSymbols(ButtonSymbols symbols);
   ButtonSymbols buttonSymbols() const;

   void setCorrectionMode(CorrectionMode cm);
   CorrectionMode correctionMode() const;

   QString specialValueText() const;
   void setSpecialValueText(const QString &txt);

   void setWrapping(bool wrapping);
   bool wrapping() const;

   void setReadOnly(bool enable);
   bool isReadOnly() const;

   void setKeyboardTracking(bool enable);
   bool keyboardTracking() const;

   void setAlignment(Qt::Alignment flag);
   Qt::Alignment alignment() const;

   void setFrame(bool enable);
   bool hasFrame() const;

   void setAccelerated(bool on);
   bool isAccelerated() const;

   void setGroupSeparatorShown(bool shown);
   bool isGroupSeparatorShown() const;
   QSize sizeHint() const override;
   QSize minimumSizeHint() const override;
   void interpretText();
   bool event(QEvent *event) override;

   QVariant inputMethodQuery(Qt::InputMethodQuery query) const override;

   virtual QValidator::State validate(QString &input, int &pos) const;
   virtual void fixup(QString &input) const;

   virtual void stepBy(int steps);

   GUI_CS_SLOT_1(Public, void stepUp())
   GUI_CS_SLOT_2(stepUp)

   GUI_CS_SLOT_1(Public, void stepDown())
   GUI_CS_SLOT_2(stepDown)

   GUI_CS_SLOT_1(Public, void selectAll())
   GUI_CS_SLOT_2(selectAll)

   GUI_CS_SLOT_1(Public, virtual void clear())
   GUI_CS_SLOT_2(clear)

   GUI_CS_SIGNAL_1(Public, void editingFinished())
   GUI_CS_SIGNAL_2(editingFinished)

 protected:
   void resizeEvent(QResizeEvent *event) override;
   void keyPressEvent(QKeyEvent *event) override;
   void keyReleaseEvent(QKeyEvent *event) override;

#ifndef QT_NO_WHEELEVENT
   void wheelEvent(QWheelEvent *event) override;
#endif

   void focusInEvent(QFocusEvent *event) override;
   void focusOutEvent(QFocusEvent *event) override;
   void contextMenuEvent(QContextMenuEvent *event) override;
   void changeEvent(QEvent *event) override;
   void closeEvent(QCloseEvent *event) override;
   void hideEvent(QHideEvent *event) override;
   void mousePressEvent(QMouseEvent *event) override;
   void mouseReleaseEvent(QMouseEvent *event) override;
   void mouseMoveEvent(QMouseEvent *event) override;
   void timerEvent(QTimerEvent *event) override;
   void paintEvent(QPaintEvent *event) override;
   void showEvent(QShowEvent *event) override;
   void initStyleOption(QStyleOptionSpinBox *option) const;

   QLineEdit *lineEdit() const;
   void setLineEdit(QLineEdit *lineEdit);

   virtual StepEnabled stepEnabled() const;

   QAbstractSpinBox(QAbstractSpinBoxPrivate &dd, QWidget *parent = nullptr);

 private:
   Q_DECLARE_PRIVATE(QAbstractSpinBox)

   GUI_CS_SLOT_1(Private, void _q_editorTextChanged(const QString &text))
   GUI_CS_SLOT_2(_q_editorTextChanged)

   GUI_CS_SLOT_1(Private, void _q_editorCursorPositionChanged(int oldpos, int newpos))
   GUI_CS_SLOT_2(_q_editorCursorPositionChanged)

   friend class QAccessibleAbstractSpinBox;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(QAbstractSpinBox::StepEnabled)

#endif // QT_NO_SPINBOX

#endif
