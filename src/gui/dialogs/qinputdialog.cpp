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

#include <qinputdialog.h>

#ifndef QT_NO_INPUTDIALOG

#include <qapplication.h>
#include <qcombobox.h>
#include <qdialogbuttonbox.h>
#include <qevent.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qlineedit.h>
#include <qlistwidget.h>
#include <qplaintextedit.h>
#include <qpushbutton.h>
#include <qspinbox.h>
#include <qstackedlayout.h>
#include <qvalidator.h>

#include <qdialog_p.h>

enum CandidateSignal {
   TextValueSelectedSignal,
   IntValueSelectedSignal,
   DoubleValueSelectedSignal,

   NumCandidateSignals
};

static QString candidateSignal(int which)
{
   QString retval;

   switch (CandidateSignal(which)) {
      case TextValueSelectedSignal:
         retval = "textValueSelected(QString)";
         break;

      case IntValueSelectedSignal:
         retval = "intValueSelected(int)";
         break;

      case DoubleValueSelectedSignal:
         retval = "doubleValueSelected(double)";
         break;

      case NumCandidateSignals:
         [[fallthrough]];

      default:
         // error, may want to throw
         break;
   }

   return retval;
}

static QString signalForMember(const QString &member)
{
   QString normalizedMember(QMetaObject::normalizedSignature(member));

   for (int i = 0; i < NumCandidateSignals; ++i) {
      if (QMetaObject::checkConnectArgs(candidateSignal(i), normalizedMember)) {
         return candidateSignal(i);
      }
   }

   // use fit-all accepted signal
   return QString("accepted()");
}

/*
    These internal classes add extra validation to QSpinBox and QDoubleSpinBox by emitting
    textChanged(bool) after events that may potentially change the visible text. Return or
    Enter key presses are not propagated if the visible text is invalid. Instead, the visible
    text is modified to the last valid value.
*/
class QInputDialogSpinBox : public QSpinBox
{
   GUI_CS_OBJECT(QInputDialogSpinBox)

 public:
   QInputDialogSpinBox(QWidget *parent)
      : QSpinBox(parent) {
      connect(lineEdit(), &QLineEdit::textChanged,               this, &QInputDialogSpinBox::notifyTextChanged);
      connect(this,       &QInputDialogSpinBox::editingFinished, this, &QInputDialogSpinBox::notifyTextChanged);
   }

   GUI_CS_SIGNAL_1(Public, void textChanged(bool isChanged))
   GUI_CS_SIGNAL_2(textChanged, isChanged)

 private:
   GUI_CS_SLOT_1(Private, void notifyTextChanged())
   GUI_CS_SLOT_2(notifyTextChanged)

   void keyPressEvent(QKeyEvent *event) override {
      if ((event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter) && ! hasAcceptableInput()) {

#ifndef QT_NO_PROPERTIES
         setProperty("value", property("value"));
#endif

      } else {
         QSpinBox::keyPressEvent(event);
      }
      notifyTextChanged();
   }

   void mousePressEvent(QMouseEvent *event) override {
      QSpinBox::mousePressEvent(event);
      notifyTextChanged();
   }
};

void QInputDialogSpinBox::notifyTextChanged()
{
   emit textChanged(hasAcceptableInput());
}

class QInputDialogDoubleSpinBox : public QDoubleSpinBox
{
   GUI_CS_OBJECT(QInputDialogDoubleSpinBox)

 public:
   QInputDialogDoubleSpinBox(QWidget *parent = nullptr)
      : QDoubleSpinBox(parent) {
      connect(lineEdit(), &QLineEdit::textChanged,               this, &QInputDialogDoubleSpinBox::notifyTextChanged);
      connect(this,       &QInputDialogSpinBox::editingFinished, this, &QInputDialogDoubleSpinBox::notifyTextChanged);
   }

   GUI_CS_SIGNAL_1(Public, void textChanged(bool isChanged))
   GUI_CS_SIGNAL_2(textChanged, isChanged)

   GUI_CS_SLOT_1(Private, void notifyTextChanged())
   GUI_CS_SLOT_2(notifyTextChanged)

 private:
   void keyPressEvent(QKeyEvent *event) override {
      if ((event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter) && !hasAcceptableInput()) {
#ifndef QT_NO_PROPERTIES
         setProperty("value", property("value"));
#endif
      } else {
         QDoubleSpinBox::keyPressEvent(event);
      }
      notifyTextChanged();
   }

   void mousePressEvent(QMouseEvent *event)  override {
      QDoubleSpinBox::mousePressEvent(event);
      notifyTextChanged();
   }
};

void QInputDialogDoubleSpinBox::notifyTextChanged()
{
   emit textChanged(hasAcceptableInput());
}

class QInputDialogPrivate : public QDialogPrivate
{
   Q_DECLARE_PUBLIC(QInputDialog)

 public:
   QInputDialogPrivate();

   void ensureLayout();
   void ensureLineEdit();
   void ensurePlainTextEdit();
   void ensureComboBox();
   void ensureListView();
   void ensureIntSpinBox();
   void ensureDoubleSpinBox();
   void ensureEnabledConnection(QAbstractSpinBox *spinBox);
   void setInputWidget(QWidget *widget);
   void chooseRightTextInputWidget();
   void setComboBoxText(const QString &text);
   void setListViewText(const QString &text);
   QString listViewText() const;

   void ensureLayout() const {
      const_cast<QInputDialogPrivate *>(this)->ensureLayout();
   }

   bool useComboBoxOrListView() const {
      return comboBox && comboBox->count() > 0;
   }

   void _q_textChanged(const QString &text);
   void _q_plainTextEditTextChanged();
   void _q_currentRowChanged(const QModelIndex &newIndex, const QModelIndex &oldIndex);

   mutable QLabel *label;
   mutable QDialogButtonBox *buttonBox;
   mutable QLineEdit *lineEdit;
   mutable QPlainTextEdit *plainTextEdit;
   mutable QSpinBox *intSpinBox;
   mutable QDoubleSpinBox *doubleSpinBox;
   mutable QComboBox *comboBox;
   mutable QListView *listView;
   mutable QWidget *inputWidget;
   mutable QVBoxLayout *mainLayout;
   QInputDialog::InputDialogOptions opts;
   QString textValue;
   QPointer<QObject> receiverToDisconnectOnClose;
   QString memberToDisconnectOnClose;
};

QInputDialogPrivate::QInputDialogPrivate()
   : label(nullptr), buttonBox(nullptr), lineEdit(nullptr), plainTextEdit(nullptr), intSpinBox(nullptr),
     doubleSpinBox(nullptr), comboBox(nullptr), listView(nullptr), inputWidget(nullptr), mainLayout(nullptr)
{
}

void QInputDialogPrivate::ensureLayout()
{
   Q_Q(QInputDialog);

   if (mainLayout) {
      return;
   }

   if (!inputWidget) {
      ensureLineEdit();
      inputWidget = lineEdit;
   }

   if (!label) {
      label = new QLabel(QInputDialog::tr("Enter a value:"), q);
   }

#ifndef QT_NO_SHORTCUT
   label->setBuddy(inputWidget);
#endif

   label->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);

   buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, q);
   QObject::connect(buttonBox, &QDialogButtonBox::accepted, q, &QInputDialog::accept);
   QObject::connect(buttonBox, &QDialogButtonBox::rejected, q, &QInputDialog::reject);

   mainLayout = new QVBoxLayout(q);
   mainLayout->setSizeConstraint(QLayout::SetMinAndMaxSize);
   mainLayout->addWidget(label);
   mainLayout->addWidget(inputWidget);
   mainLayout->addWidget(buttonBox);
   ensureEnabledConnection(qobject_cast<QAbstractSpinBox *>(inputWidget));
   inputWidget->show();
}

void QInputDialogPrivate::ensureLineEdit()
{
   Q_Q(QInputDialog);

   if (! lineEdit) {
      lineEdit = new QLineEdit(q);

#ifndef QT_NO_IM
      qt_widget_private(lineEdit)->inheritsInputMethodHints = 1;
#endif

      lineEdit->hide();
      QObject::connect(lineEdit, &QLineEdit::textChanged, q, &QInputDialog::_q_textChanged);
   }
}

void QInputDialogPrivate::ensurePlainTextEdit()
{
   Q_Q(QInputDialog);

   if (! plainTextEdit) {
      plainTextEdit = new QPlainTextEdit(q);
      plainTextEdit->setLineWrapMode(QPlainTextEdit::NoWrap);

#ifndef QT_NO_IM
      qt_widget_private(plainTextEdit)->inheritsInputMethodHints = 1;
#endif

      plainTextEdit->hide();
      QObject::connect(plainTextEdit, &QPlainTextEdit::textChanged, q, &QInputDialog::_q_plainTextEditTextChanged);
   }
}

void QInputDialogPrivate::ensureComboBox()
{
   Q_Q(QInputDialog);

   if (! comboBox) {
      comboBox = new QComboBox(q);

#ifndef QT_NO_IM
      qt_widget_private(comboBox)->inheritsInputMethodHints = 1;
#endif

      comboBox->hide();
      QObject::connect(comboBox, &QComboBox::editTextChanged, q, &QInputDialog::_q_textChanged);
      QObject::connect(comboBox, cs_mp_cast<const QString &>(&QComboBox::currentIndexChanged), q, &QInputDialog::_q_textChanged);
   }
}

void QInputDialogPrivate::ensureListView()
{
   Q_Q(QInputDialog);

   if (! listView) {
      ensureComboBox();

      listView = new QListView(q);
      listView->hide();
      listView->setEditTriggers(QAbstractItemView::NoEditTriggers);
      listView->setSelectionMode(QAbstractItemView::SingleSelection);
      listView->setModel(comboBox->model());
      listView->setCurrentIndex(QModelIndex());

      QObject::connect(listView->selectionModel(), &QItemSelectionModel::currentRowChanged, q, &QInputDialog::_q_currentRowChanged);
   }
}

void QInputDialogPrivate::ensureIntSpinBox()
{
   Q_Q(QInputDialog);

   if (! intSpinBox) {
      intSpinBox = new QInputDialogSpinBox(q);
      intSpinBox->hide();

      QObject::connect(intSpinBox, cs_mp_cast<int>(&QSpinBox::valueChanged), q, &QInputDialog::intValueChanged);
   }
}

void QInputDialogPrivate::ensureDoubleSpinBox()
{
   Q_Q(QInputDialog);

   if (! doubleSpinBox) {
      doubleSpinBox = new QInputDialogDoubleSpinBox(q);
      doubleSpinBox->hide();

      QObject::connect(doubleSpinBox, cs_mp_cast<double>(&QDoubleSpinBox::valueChanged), q, &QInputDialog::doubleValueChanged);
   }
}

void QInputDialogPrivate::ensureEnabledConnection(QAbstractSpinBox *spinBox)
{
   if (spinBox != nullptr) {
      QAbstractButton *okButton = buttonBox->button(QDialogButtonBox::Ok);

      QInputDialogSpinBox *tmp1 = dynamic_cast<QInputDialogSpinBox *>(spinBox);

      if (tmp1 != nullptr) {
         QObject::connect(tmp1, &QInputDialogSpinBox::textChanged, okButton, &QAbstractButton::setEnabled, Qt::UniqueConnection);

      } else {
         QInputDialogDoubleSpinBox *tmp2 = dynamic_cast<QInputDialogDoubleSpinBox *>(spinBox);

         if (tmp2 != nullptr) {
            QObject::connect(tmp2, &QInputDialogDoubleSpinBox::textChanged, okButton, &QAbstractButton::setEnabled, Qt::UniqueConnection);
         }
      }
   }
}

void QInputDialogPrivate::setInputWidget(QWidget *widget)
{
   Q_ASSERT(widget);

   if (inputWidget == widget) {
      return;
   }

   if (mainLayout) {
      Q_ASSERT(inputWidget);
      mainLayout->removeWidget(inputWidget);
      inputWidget->hide();
      mainLayout->insertWidget(1, widget);
      widget->show();

      // disconnect old input widget
      QAbstractButton *okButton = buttonBox->button(QDialogButtonBox::Ok);

      if (QAbstractSpinBox *spinBox = qobject_cast<QAbstractSpinBox *>(inputWidget)) {

         QInputDialogSpinBox *tmp1 = dynamic_cast<QInputDialogSpinBox *>(spinBox);

         if (tmp1 != nullptr) {
            QObject::disconnect(tmp1, &QInputDialogSpinBox::textChanged, okButton, &QAbstractButton::setEnabled);

         } else {
            QInputDialogDoubleSpinBox *tmp2 = dynamic_cast<QInputDialogDoubleSpinBox *>(spinBox);

            if (tmp2 != nullptr) {
               QObject::disconnect(tmp2, &QInputDialogDoubleSpinBox::textChanged, okButton, &QAbstractButton::setEnabled);
            }
         }
      }

      QAbstractSpinBox *spinBox = qobject_cast<QAbstractSpinBox *>(widget);

      // connect new input widget and update enabled state of OK button
      ensureEnabledConnection(spinBox);
      okButton->setEnabled(! spinBox || spinBox->hasAcceptableInput());
   }

   inputWidget = widget;

   // synchronize the text shown in the new text editor with the current textValue
   if (widget == lineEdit) {
      lineEdit->setText(textValue);

   } else if (widget == plainTextEdit) {
      plainTextEdit->setPlainText(textValue);

   } else if (widget == comboBox) {
      setComboBoxText(textValue);

   } else if (widget == listView) {
      setListViewText(textValue);
      ensureLayout();
      buttonBox->button(QDialogButtonBox::Ok)->setEnabled(listView->selectionModel()->hasSelection());
   }
}

void QInputDialogPrivate::chooseRightTextInputWidget()
{
   QWidget *widget;

   if (useComboBoxOrListView()) {
      if ((opts & QInputDialog::UseListViewForComboBoxItems) && !comboBox->isEditable()) {
         ensureListView();
         widget = listView;
      } else {
         widget = comboBox;
      }

   } else if (opts & QInputDialog::UsePlainTextEditForTextInput) {
      ensurePlainTextEdit();
      widget = plainTextEdit;

   } else {
      ensureLineEdit();
      widget = lineEdit;
   }

   setInputWidget(widget);

   if (inputWidget == comboBox) {
      _q_textChanged(comboBox->currentText());
   } else if (inputWidget == listView) {
      _q_textChanged(listViewText());
   }
}

void QInputDialogPrivate::setComboBoxText(const QString &text)
{
   int index = comboBox->findText(text);
   if (index != -1) {
      comboBox->setCurrentIndex(index);
   } else if (comboBox->isEditable()) {
      comboBox->setEditText(text);
   }
}

void QInputDialogPrivate::setListViewText(const QString &text)
{
   int row = comboBox->findText(text);
   if (row != -1) {
      QModelIndex index(comboBox->model()->index(row, 0));
      listView->selectionModel()->setCurrentIndex(index, QItemSelectionModel::Clear
         | QItemSelectionModel::SelectCurrent);
   }
}

QString QInputDialogPrivate::listViewText() const
{
   if (listView->selectionModel()->hasSelection()) {
      int row = listView->selectionModel()->selectedRows().value(0).row();
      return comboBox->itemText(row);
   } else {
      return QString();
   }
}

void QInputDialogPrivate::_q_textChanged(const QString &text)
{
   Q_Q(QInputDialog);
   if (textValue != text) {
      textValue = text;
      emit q->textValueChanged(text);
   }
}

void QInputDialogPrivate::_q_plainTextEditTextChanged()
{
   Q_Q(QInputDialog);
   QString text = plainTextEdit->toPlainText();
   if (textValue != text) {
      textValue = text;
      emit q->textValueChanged(text);
   }
}
void QInputDialogPrivate::_q_currentRowChanged(const QModelIndex &newIndex,
   const QModelIndex & /* oldIndex */)
{
   _q_textChanged(comboBox->model()->data(newIndex).toString());
   buttonBox->button(QDialogButtonBox::Ok)->setEnabled(true);
}


QInputDialog::QInputDialog(QWidget *parent, Qt::WindowFlags flags)
   : QDialog(*new QInputDialogPrivate, parent, flags)
{
}

QInputDialog::~QInputDialog()
{
}

void QInputDialog::setInputMode(InputMode mode)
{
   Q_D(QInputDialog);

   QWidget *widget;

   /*
       Warning: Some functions in QInputDialog rely on implementation details
       of the code below. Look for the comments that accompany the calls to
       setInputMode() throughout this file before you change the code below.
   */

   switch (mode) {
      case IntInput:
         d->ensureIntSpinBox();
         widget = d->intSpinBox;
         break;
      case DoubleInput:
         d->ensureDoubleSpinBox();
         widget = d->doubleSpinBox;
         break;
      default:
         Q_ASSERT(mode == TextInput);
         d->chooseRightTextInputWidget();
         return;
   }

   d->setInputWidget(widget);
}

QInputDialog::InputMode QInputDialog::inputMode() const
{
   Q_D(const QInputDialog);

   if (d->inputWidget) {
      if (d->inputWidget == d->intSpinBox) {
         return IntInput;
      } else if (d->inputWidget == d->doubleSpinBox) {
         return DoubleInput;
      }
   }

   return TextInput;
}

void QInputDialog::setLabelText(const QString &text)
{
   Q_D(QInputDialog);
   if (!d->label) {
      d->label = new QLabel(text, this);
   } else {
      d->label->setText(text);
   }
}

QString QInputDialog::labelText() const
{
   Q_D(const QInputDialog);
   d->ensureLayout();
   return d->label->text();
}

void QInputDialog::setOption(InputDialogOption option, bool on)
{
   Q_D(QInputDialog);
   if (!(d->opts & option) != !on) {
      setOptions(d->opts ^ option);
   }
}

bool QInputDialog::testOption(InputDialogOption option) const
{
   Q_D(const QInputDialog);
   return (d->opts & option) != 0;
}

void QInputDialog::setOptions(InputDialogOptions options)
{
   Q_D(QInputDialog);

   InputDialogOptions changed = (options ^ d->opts);
   if (!changed) {
      return;
   }

   d->opts = options;
   d->ensureLayout();

   if (changed & NoButtons) {
      d->buttonBox->setVisible(!(options & NoButtons));
   }

   if ((changed & UseListViewForComboBoxItems) && inputMode() == TextInput) {
      d->chooseRightTextInputWidget();
   }

   if ((changed & UsePlainTextEditForTextInput) && inputMode() == TextInput) {
      d->chooseRightTextInputWidget();
   }
}

QInputDialog::InputDialogOptions QInputDialog::options() const
{
   Q_D(const QInputDialog);
   return d->opts;
}

void QInputDialog::setTextValue(const QString &text)
{
   Q_D(QInputDialog);

   setInputMode(TextInput);

   if (d->inputWidget == d->lineEdit) {
      d->lineEdit->setText(text);

   } else if (d->inputWidget == d->plainTextEdit) {
      d->plainTextEdit->setPlainText(text);

   } else if (d->inputWidget == d->comboBox) {
      d->setComboBoxText(text);

   } else {
      d->setListViewText(text);
   }
}

QString QInputDialog::textValue() const
{
   Q_D(const QInputDialog);
   return d->textValue;
}

void QInputDialog::setTextEchoMode(QLineEdit::EchoMode mode)
{
   Q_D(QInputDialog);
   d->ensureLineEdit();
   d->lineEdit->setEchoMode(mode);
}

QLineEdit::EchoMode QInputDialog::textEchoMode() const
{
   Q_D(const QInputDialog);
   if (d->lineEdit) {
      return d->lineEdit->echoMode();
   } else {
      return QLineEdit::Normal;
   }
}


void QInputDialog::setComboBoxEditable(bool editable)
{
   Q_D(QInputDialog);
   d->ensureComboBox();
   d->comboBox->setEditable(editable);
   if (inputMode() == TextInput) {
      d->chooseRightTextInputWidget();
   }
}

bool QInputDialog::isComboBoxEditable() const
{
   Q_D(const QInputDialog);
   if (d->comboBox) {
      return d->comboBox->isEditable();
   } else {
      return false;
   }
}

void QInputDialog::setComboBoxItems(const QStringList &items)
{
   Q_D(QInputDialog);

   d->ensureComboBox();
   d->comboBox->blockSignals(true);
   d->comboBox->clear();
   d->comboBox->addItems(items);
   d->comboBox->blockSignals(false);

   if (inputMode() == TextInput) {
      d->chooseRightTextInputWidget();
   }
}

QStringList QInputDialog::comboBoxItems() const
{
   Q_D(const QInputDialog);
   QStringList result;
   if (d->comboBox) {
      const int count = d->comboBox->count();
      for (int i = 0; i < count; ++i) {
         result.append(d->comboBox->itemText(i));
      }
   }
   return result;
}


void QInputDialog::setIntValue(int value)
{
   Q_D(QInputDialog);
   setInputMode(IntInput);
   d->intSpinBox->setValue(value);
}

int QInputDialog::intValue() const
{
   Q_D(const QInputDialog);
   if (d->intSpinBox) {
      return d->intSpinBox->value();
   } else {
      return 0;
   }
}


void QInputDialog::setIntMinimum(int min)
{
   Q_D(QInputDialog);
   d->ensureIntSpinBox();
   d->intSpinBox->setMinimum(min);
}

int QInputDialog::intMinimum() const
{
   Q_D(const QInputDialog);
   if (d->intSpinBox) {
      return d->intSpinBox->minimum();
   } else {
      return 0;
   }
}


void QInputDialog::setIntMaximum(int max)
{
   Q_D(QInputDialog);
   d->ensureIntSpinBox();
   d->intSpinBox->setMaximum(max);
}

int QInputDialog::intMaximum() const
{
   Q_D(const QInputDialog);
   if (d->intSpinBox) {
      return d->intSpinBox->maximum();
   } else {
      return 99;
   }
}

void QInputDialog::setIntRange(int min, int max)
{
   Q_D(QInputDialog);
   d->ensureIntSpinBox();
   d->intSpinBox->setRange(min, max);
}

void QInputDialog::setIntStep(int step)
{
   Q_D(QInputDialog);
   d->ensureIntSpinBox();
   d->intSpinBox->setSingleStep(step);
}

int QInputDialog::intStep() const
{
   Q_D(const QInputDialog);
   if (d->intSpinBox) {
      return d->intSpinBox->singleStep();
   } else {
      return 1;
   }
}

void QInputDialog::setDoubleValue(double value)
{
   Q_D(QInputDialog);
   setInputMode(DoubleInput);
   d->doubleSpinBox->setValue(value);
}

double QInputDialog::doubleValue() const
{
   Q_D(const QInputDialog);
   if (d->doubleSpinBox) {
      return d->doubleSpinBox->value();
   } else {
      return 0.0;
   }
}

void QInputDialog::setDoubleMinimum(double min)
{
   Q_D(QInputDialog);
   d->ensureDoubleSpinBox();
   d->doubleSpinBox->setMinimum(min);
}

double QInputDialog::doubleMinimum() const
{
   Q_D(const QInputDialog);
   if (d->doubleSpinBox) {
      return d->doubleSpinBox->minimum();
   } else {
      return 0.0;
   }
}

void QInputDialog::setDoubleMaximum(double max)
{
   Q_D(QInputDialog);
   d->ensureDoubleSpinBox();
   d->doubleSpinBox->setMaximum(max);
}

double QInputDialog::doubleMaximum() const
{
   Q_D(const QInputDialog);
   if (d->doubleSpinBox) {
      return d->doubleSpinBox->maximum();
   } else {
      return 99.99;
   }
}


void QInputDialog::setDoubleRange(double min, double max)
{
   Q_D(QInputDialog);
   d->ensureDoubleSpinBox();
   d->doubleSpinBox->setRange(min, max);
}


void QInputDialog::setDoubleDecimals(int decimals)
{
   Q_D(QInputDialog);

   d->ensureDoubleSpinBox();
   d->doubleSpinBox->setDecimals(decimals);
}

int QInputDialog::doubleDecimals() const
{
   Q_D(const QInputDialog);

   if (d->doubleSpinBox) {
      return d->doubleSpinBox->decimals();
   } else {
      return 2;
   }
}

void QInputDialog::setOkButtonText(const QString &text)
{
   Q_D(const QInputDialog);
   d->ensureLayout();
   d->buttonBox->button(QDialogButtonBox::Ok)->setText(text);
}

QString QInputDialog::okButtonText() const
{
   Q_D(const QInputDialog);
   d->ensureLayout();
   return d->buttonBox->button(QDialogButtonBox::Ok)->text();
}


void QInputDialog::setCancelButtonText(const QString &text)
{
   Q_D(const QInputDialog);
   d->ensureLayout();
   d->buttonBox->button(QDialogButtonBox::Cancel)->setText(text);
}

QString QInputDialog::cancelButtonText() const
{
   Q_D(const QInputDialog);
   d->ensureLayout();
   return d->buttonBox->button(QDialogButtonBox::Cancel)->text();
}

void QInputDialog::open(QObject *receiver, const QString &member)
{
   Q_D(QInputDialog);

   connect(this, signalForMember(member), receiver, member);
   d->receiverToDisconnectOnClose = receiver;
   d->memberToDisconnectOnClose = member;

   QDialog::open();
}

QSize QInputDialog::minimumSizeHint() const
{
   Q_D(const QInputDialog);
   d->ensureLayout();
   return QDialog::minimumSizeHint();
}

QSize QInputDialog::sizeHint() const
{
   Q_D(const QInputDialog);
   d->ensureLayout();
   return QDialog::sizeHint();
}

void QInputDialog::setVisible(bool visible)
{
   Q_D(const QInputDialog);
   if (visible) {
      d->ensureLayout();
      d->inputWidget->setFocus();

      if (d->inputWidget == d->lineEdit) {
         d->lineEdit->selectAll();

      } else if (d->inputWidget == d->plainTextEdit) {
         d->plainTextEdit->selectAll();

      } else if (d->inputWidget == d->intSpinBox) {
         d->intSpinBox->selectAll();

      } else if (d->inputWidget == d->doubleSpinBox) {
         d->doubleSpinBox->selectAll();
      }
   }
   QDialog::setVisible(visible);
}

void QInputDialog::done(int result)
{
   Q_D(QInputDialog);
   QDialog::done(result);

   if (result) {
      InputMode mode = inputMode();

      switch (mode) {
         case DoubleInput:
            emit doubleValueSelected(doubleValue());
            break;
         case IntInput:
            emit intValueSelected(intValue());
            break;
         default:
            Q_ASSERT(mode == TextInput);
            emit textValueSelected(textValue());
      }
   }

   if (d->receiverToDisconnectOnClose) {
      disconnect(this, signalForMember(d->memberToDisconnectOnClose), d->receiverToDisconnectOnClose,
         d->memberToDisconnectOnClose);

      d->receiverToDisconnectOnClose = nullptr;
   }

   d->memberToDisconnectOnClose.clear();
}

QString QInputDialog::getText(QWidget *parent, const QString &title, const QString &label,
   QLineEdit::EchoMode mode, const QString &text, bool *ok,
   Qt::WindowFlags flags, Qt::InputMethodHints inputMethodHints)
{
   QInputDialog dialog(parent, flags);

   dialog.setWindowTitle(title);
   dialog.setLabelText(label);
   dialog.setTextValue(text);
   dialog.setTextEchoMode(mode);
   dialog.setInputMethodHints(inputMethodHints);

   const int result = dialog.exec();

   if (ok) {
      *ok = (result != 0);
   }

   if (result) {
      return dialog.textValue();
   } else {
      return QString();
   }
}

QString QInputDialog::getMultiLineText(QWidget *parent, const QString &title, const QString &label,
   const QString &text, bool *ok, Qt::WindowFlags flags, Qt::InputMethodHints inputMethodHints)
{
   QInputDialog dialog(parent, flags);

   dialog.setOptions(QInputDialog::UsePlainTextEditForTextInput);
   dialog.setWindowTitle(title);
   dialog.setLabelText(label);
   dialog.setTextValue(text);
   dialog.setInputMethodHints(inputMethodHints);

   const int ret = dialog.exec();

   if (ok) {
      *ok = !!ret;
   }

   if (ret) {
      return dialog.textValue();
   } else {
      return QString();
   }
}

int QInputDialog::getInt(QWidget *parent, const QString &title, const QString &label, int value,
   int min, int max, int step, bool *ok, Qt::WindowFlags flags)
{
   QInputDialog dialog(parent, flags);

   dialog.setWindowTitle(title);
   dialog.setLabelText(label);
   dialog.setIntRange(min, max);
   dialog.setIntValue(value);
   dialog.setIntStep(step);

   int ret = dialog.exec();

   if (ok) {
      *ok = !!ret;
   }

   if (ret) {
      return dialog.intValue();
   } else {
      return value;
   }
}

double QInputDialog::getDouble(QWidget *parent, const QString &title, const QString &label,
   double value, double min, double max, int decimals, bool *ok, Qt::WindowFlags flags)
{
   QInputDialog dialog(parent, flags);

   dialog.setWindowTitle(title);
   dialog.setLabelText(label);
   dialog.setDoubleDecimals(decimals);
   dialog.setDoubleRange(min, max);
   dialog.setDoubleValue(value);

   const int ret = dialog.exec();

   if (ok) {
      *ok = !!ret;
   }

   if (ret) {
      return dialog.doubleValue();
   } else {
      return value;
   }
}

QString QInputDialog::getItem(QWidget *parent, const QString &title, const QString &label,
   const QStringList &items, int current, bool editable, bool *ok,
   Qt::WindowFlags flags, Qt::InputMethodHints inputMethodHints)
{
   QString text(items.value(current));

   QInputDialog dialog(parent, flags);

   dialog.setWindowTitle(title);
   dialog.setLabelText(label);
   dialog.setComboBoxItems(items);
   dialog.setTextValue(text);
   dialog.setComboBoxEditable(editable);
   dialog.setInputMethodHints(inputMethodHints);

   const int ret = dialog.exec();

   if (ok) {
      *ok = !!ret;
   }

   if (ret) {
      return dialog.textValue();
   } else {
      return text;
   }
}

void QInputDialog::_q_textChanged(const QString &text)
{
   Q_D(QInputDialog);
   d->_q_textChanged(text);
}

void QInputDialog::_q_plainTextEditTextChanged()
{
   Q_D(QInputDialog);
   d->_q_plainTextEditTextChanged();
}

void QInputDialog::_q_currentRowChanged(const QModelIndex &newIndex, const QModelIndex &oldIndex)
{
   Q_D(QInputDialog);
   d->_q_currentRowChanged(newIndex, oldIndex);
}

#endif
