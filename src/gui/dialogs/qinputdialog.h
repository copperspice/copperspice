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

#ifndef QINPUTDIALOG_H
#define QINPUTDIALOG_H

#include <qdialog.h>
#include <qstring.h>
#include <qlineedit.h>

#ifndef QT_NO_INPUTDIALOG

class QInputDialogPrivate;

class Q_GUI_EXPORT QInputDialog : public QDialog
{
   GUI_CS_OBJECT(QInputDialog)

   GUI_CS_PROPERTY_READ(inputMode, inputMode)
   GUI_CS_PROPERTY_WRITE(inputMode, setInputMode)
   GUI_CS_PROPERTY_READ(labelText, labelText)
   GUI_CS_PROPERTY_WRITE(labelText, setLabelText)
   GUI_CS_PROPERTY_READ(options, options)
   GUI_CS_PROPERTY_WRITE(options, setOptions)

   GUI_CS_PROPERTY_READ(textValue, textValue)
   GUI_CS_PROPERTY_WRITE(textValue, setTextValue)
   GUI_CS_PROPERTY_NOTIFY(textValue, textValueChanged)

   GUI_CS_PROPERTY_READ(intValue, intValue)
   GUI_CS_PROPERTY_WRITE(intValue, setIntValue)
   GUI_CS_PROPERTY_NOTIFY(intValue, intValueChanged)

   GUI_CS_PROPERTY_READ(doubleValue, doubleValue)
   GUI_CS_PROPERTY_WRITE(doubleValue, setDoubleValue)
   GUI_CS_PROPERTY_NOTIFY(doubleValue, doubleValueChanged)

   GUI_CS_PROPERTY_READ(textEchoMode, textEchoMode)
   GUI_CS_PROPERTY_WRITE(textEchoMode, setTextEchoMode)

   GUI_CS_PROPERTY_READ(comboBoxEditable, isComboBoxEditable)
   GUI_CS_PROPERTY_WRITE(comboBoxEditable, setComboBoxEditable)

   GUI_CS_PROPERTY_READ(comboBoxItems, comboBoxItems)
   GUI_CS_PROPERTY_WRITE(comboBoxItems, setComboBoxItems)

   GUI_CS_PROPERTY_READ(intMinimum, intMinimum)
   GUI_CS_PROPERTY_WRITE(intMinimum, setIntMinimum)

   GUI_CS_PROPERTY_READ(intMaximum, intMaximum)
   GUI_CS_PROPERTY_WRITE(intMaximum, setIntMaximum)

   GUI_CS_PROPERTY_READ(intStep, intStep)
   GUI_CS_PROPERTY_WRITE(intStep, setIntStep)
   GUI_CS_PROPERTY_READ(doubleMinimum, doubleMinimum)
   GUI_CS_PROPERTY_WRITE(doubleMinimum, setDoubleMinimum)
   GUI_CS_PROPERTY_READ(doubleMaximum, doubleMaximum)
   GUI_CS_PROPERTY_WRITE(doubleMaximum, setDoubleMaximum)
   GUI_CS_PROPERTY_READ(doubleDecimals, doubleDecimals)
   GUI_CS_PROPERTY_WRITE(doubleDecimals, setDoubleDecimals)
   GUI_CS_PROPERTY_READ(okButtonText, okButtonText)
   GUI_CS_PROPERTY_WRITE(okButtonText, setOkButtonText)
   GUI_CS_PROPERTY_READ(cancelButtonText, cancelButtonText)
   GUI_CS_PROPERTY_WRITE(cancelButtonText, setCancelButtonText)

 public:
   enum InputDialogOption {
      NoButtons                   = 0x00000001,
      UseListViewForComboBoxItems  = 0x00000002,
      UsePlainTextEditForTextInput = 0x00000004
   };

   using InputDialogOptions = QFlags<InputDialogOption>;

   enum InputMode {
      TextInput,
      IntInput,
      DoubleInput
   };

   QInputDialog(QWidget *parent = nullptr, Qt::WindowFlags flags = Qt::EmptyFlag);

   QInputDialog(const QInputDialog &) = delete;
   QInputDialog &operator=(const QInputDialog &) = delete;

   ~QInputDialog();

   void setInputMode(InputMode mode);
   InputMode inputMode() const;

   void setLabelText(const QString &text);
   QString labelText() const;

   void setOption(InputDialogOption option, bool on = true);
   bool testOption(InputDialogOption option) const;
   void setOptions(InputDialogOptions options);
   InputDialogOptions options() const;

   void setTextValue(const QString &text);
   QString textValue() const;

   void setTextEchoMode(QLineEdit::EchoMode mode);
   QLineEdit::EchoMode textEchoMode() const;

   void setComboBoxEditable(bool editable);
   bool isComboBoxEditable() const;

   void setComboBoxItems(const QStringList &items);
   QStringList comboBoxItems() const;

   void setIntValue(int value);
   int intValue() const;

   void setIntMinimum(int min);
   int intMinimum() const;

   void setIntMaximum(int max);
   int intMaximum() const;

   void setIntRange(int min, int max);

   void setIntStep(int step);
   int intStep() const;

   void setDoubleValue(double value);
   double doubleValue() const;

   void setDoubleMinimum(double min);
   double doubleMinimum() const;

   void setDoubleMaximum(double max);
   double doubleMaximum() const;

   void setDoubleRange(double min, double max);

   void setDoubleDecimals(int decimals);
   int doubleDecimals() const;

   void setOkButtonText(const QString &text);
   QString okButtonText() const;

   void setCancelButtonText(const QString &text);
   QString cancelButtonText() const;

   using QDialog::open;
   void open(QObject *receiver, const QString &member);

   QSize minimumSizeHint() const override;
   QSize sizeHint() const override;

   void setVisible(bool visible) override;

   static QString getText(QWidget *parent, const QString &title, const QString &label,
      QLineEdit::EchoMode echoMode = QLineEdit::Normal, const QString &text = QString(), bool *ok = nullptr,
      Qt::WindowFlags flags = Qt::EmptyFlag, Qt::InputMethodHints inputMethodHints = Qt::ImhNone);

   static QString getMultiLineText(QWidget *parent, const QString &title, const QString &label,
      const QString &text = QString(), bool *ok = nullptr,
      Qt::WindowFlags flags = Qt::EmptyFlag, Qt::InputMethodHints inputMethodHints = Qt::ImhNone);

   static QString getItem(QWidget *parent, const QString &title, const QString &label,
      const QStringList &items, int current = 0, bool editable = true, bool *ok = nullptr,
      Qt::WindowFlags flags = Qt::EmptyFlag, Qt::InputMethodHints inputMethodHints = Qt::ImhNone);

   static int getInt(QWidget *parent, const QString &title, const QString &label, int value = 0,
      int minValue = -2147483647, int maxValue = 2147483647, int step = 1, bool *ok = nullptr,
      Qt::WindowFlags flags = Qt::EmptyFlag);

   static double getDouble(QWidget *parent, const QString &title, const QString &label, double value = 0,
      double minValue = -2147483647, double maxValue = 2147483647, int decimals = 1,
      bool *ok = nullptr, Qt::WindowFlags flags = Qt::EmptyFlag);

   GUI_CS_SIGNAL_1(Public, void textValueChanged(const QString &text))
   GUI_CS_SIGNAL_2(textValueChanged, text)

   GUI_CS_SIGNAL_1(Public, void textValueSelected(const QString &text))
   GUI_CS_SIGNAL_2(textValueSelected, text)

   GUI_CS_SIGNAL_1(Public, void intValueChanged(int value))
   GUI_CS_SIGNAL_2(intValueChanged, value)

   GUI_CS_SIGNAL_1(Public, void intValueSelected(int value))
   GUI_CS_SIGNAL_2(intValueSelected, value)

   GUI_CS_SIGNAL_1(Public, void doubleValueChanged(double value))
   GUI_CS_SIGNAL_2(doubleValueChanged, value)

   GUI_CS_SIGNAL_1(Public, void doubleValueSelected(double value))
   GUI_CS_SIGNAL_2(doubleValueSelected, value)

   void done(int result) override;

 private:
   Q_DECLARE_PRIVATE(QInputDialog)

   GUI_CS_SLOT_1(Private, void _q_textChanged(const QString &text))
   GUI_CS_SLOT_2(_q_textChanged)

   GUI_CS_SLOT_1(Private, void _q_plainTextEditTextChanged())
   GUI_CS_SLOT_2(_q_plainTextEditTextChanged)

   GUI_CS_SLOT_1(Private, void _q_currentRowChanged(const QModelIndex &newIndex, const QModelIndex &oldIndex))
   GUI_CS_SLOT_2(_q_currentRowChanged)
};

Q_DECLARE_OPERATORS_FOR_FLAGS(QInputDialog::InputDialogOptions)

#endif // QT_NO_INPUTDIALOG

#endif
