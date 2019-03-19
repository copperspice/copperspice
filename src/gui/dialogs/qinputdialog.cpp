/***********************************************************************
*
* Copyright (c) 2012-2019 Barbara Geller
* Copyright (c) 2012-2019 Ansel Sermersheim
*
* Copyright (C) 2015 The Qt Company Ltd.
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
#include <qlabel.h>
#include <qlayout.h>
#include <qlineedit.h>
#include <qlistwidget.h>
#include <qpushbutton.h>
#include <qspinbox.h>
#include <qstackedlayout.h>
#include <qvalidator.h>
#include <qevent.h>
#include <qdialog_p.h>

QT_USE_NAMESPACE

static const QString &signalForMember(const QString &member)
{
   static const int NumCandidates = 4;

   static const QString candidateSignals[NumCandidates] = {
      "textValueSelected(QString)",
      "intValueSelected(int)",
      "doubleValueSelected(double)",
      "accepted()"
   };

   QString normalizedMember(QMetaObject::normalizedSignature(member));
   int i = 0;

   while (i < NumCandidates - 1) {
      if (QMetaObject::checkConnectArgs(candidateSignals[i], normalizedMember)) {
         break;
      }
      ++i;
   }

   return candidateSignals[i];
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
      connect(lineEdit(), SIGNAL(textChanged(const QString &)), this, SLOT(notifyTextChanged()));
      connect(this, SIGNAL(editingFinished()), this, SLOT(notifyTextChanged()));
   }

   GUI_CS_SIGNAL_1(Public, void textChanged(bool un_named_arg1))
   GUI_CS_SIGNAL_2(textChanged, un_named_arg1)

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
      connect(lineEdit(), SIGNAL(textChanged(const QString &)), this, SLOT(notifyTextChanged()));
      connect(this, SIGNAL(editingFinished()), this, SLOT(notifyTextChanged()));
   }

   GUI_CS_SIGNAL_1(Public, void textChanged(bool un_named_arg1))
   GUI_CS_SIGNAL_2(textChanged, un_named_arg1)

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
   void _q_currentRowChanged(const QModelIndex &newIndex, const QModelIndex &oldIndex);

   mutable QLabel *label;
   mutable QDialogButtonBox *buttonBox;
   mutable QLineEdit *lineEdit;
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
   : label(0), buttonBox(0), lineEdit(0), intSpinBox(0), doubleSpinBox(0), comboBox(0), listView(0),
     inputWidget(0), mainLayout(0)
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
   QObject::connect(buttonBox, SIGNAL(accepted()), q, SLOT(accept()));
   QObject::connect(buttonBox, SIGNAL(rejected()), q, SLOT(reject()));

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
   if (!lineEdit) {
      lineEdit = new QLineEdit(q);
#ifndef QT_NO_IM
      qt_widget_private(lineEdit)->inheritsInputMethodHints = 1;
#endif
      lineEdit->hide();
      QObject::connect(lineEdit, SIGNAL(textChanged(const QString &)), q, SLOT(_q_textChanged(const QString &)));
   }
}

void QInputDialogPrivate::ensureComboBox()
{
   Q_Q(QInputDialog);
   if (!comboBox) {
      comboBox = new QComboBox(q);
#ifndef QT_NO_IM
      qt_widget_private(comboBox)->inheritsInputMethodHints = 1;
#endif
      comboBox->hide();
      QObject::connect(comboBox, SIGNAL(editTextChanged(const QString &)), q, SLOT(_q_textChanged(const QString &)));
      QObject::connect(comboBox, SIGNAL(currentIndexChanged(const QString &)), q, SLOT(_q_textChanged(const QString &)));
   }
}

void QInputDialogPrivate::ensureListView()
{
   Q_Q(QInputDialog);
   if (!listView) {
      ensureComboBox();

      listView = new QListView(q);
      listView->hide();
      listView->setEditTriggers(QAbstractItemView::NoEditTriggers);
      listView->setSelectionMode(QAbstractItemView::SingleSelection);
      listView->setModel(comboBox->model());
      listView->setCurrentIndex(QModelIndex());

      QObject::connect(listView->selectionModel(), SIGNAL(currentRowChanged(const QModelIndex &, const QModelIndex &)),
                       q, SLOT(_q_currentRowChanged(const QModelIndex &, const QModelIndex &)));
   }
}

void QInputDialogPrivate::ensureIntSpinBox()
{
   Q_Q(QInputDialog);
   if (!intSpinBox) {
      intSpinBox = new QInputDialogSpinBox(q);
      intSpinBox->hide();
      QObject::connect(intSpinBox, SIGNAL(valueChanged(int)), q, SLOT(intValueChanged(int)));
   }
}

void QInputDialogPrivate::ensureDoubleSpinBox()
{
   Q_Q(QInputDialog);
   if (!doubleSpinBox) {
      doubleSpinBox = new QInputDialogDoubleSpinBox(q);
      doubleSpinBox->hide();
      QObject::connect(doubleSpinBox, SIGNAL(valueChanged(double)), q, SLOT(doubleValueChanged(double)));
   }
}

void QInputDialogPrivate::ensureEnabledConnection(QAbstractSpinBox *spinBox)
{
   if (spinBox) {
      QAbstractButton *okButton = buttonBox->button(QDialogButtonBox::Ok);
      QObject::connect(spinBox, SIGNAL(textChanged(bool)), okButton, SLOT(setEnabled(bool)), Qt::UniqueConnection);
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
         QObject::disconnect(spinBox, SIGNAL(textChanged(bool)), okButton, SLOT(setEnabled(bool)));
      }

      // connect new input widget and update enabled state of OK button
      QAbstractSpinBox *spinBox = qobject_cast<QAbstractSpinBox *>(widget);
      ensureEnabledConnection(spinBox);
      okButton->setEnabled(!spinBox || spinBox->hasAcceptableInput());
   }

   inputWidget = widget;

   // synchronize the text shown in the new text editor with the current
   // textValue
   if (widget == lineEdit) {
      lineEdit->setText(textValue);
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

void QInputDialogPrivate::_q_currentRowChanged(const QModelIndex &newIndex,
      const QModelIndex & /* oldIndex */)
{
   _q_textChanged(comboBox->model()->data(newIndex).toString());
   buttonBox->button(QDialogButtonBox::Ok)->setEnabled(true);
}

void QInputDialog::_q_textChanged(const QString &un_named_arg1)
{
   Q_D(QInputDialog);
   d->_q_textChanged(un_named_arg1);
}

void QInputDialog::_q_currentRowChanged(const QModelIndex &un_named_arg1, const QModelIndex &un_named_arg2)
{
   Q_D(QInputDialog);
   d->_q_currentRowChanged(un_named_arg1, un_named_arg2);
}


/*!
    \class QInputDialog
    \brief The QInputDialog class provides a simple convenience dialog to get a
    single value from the user.
    \ingroup standard-dialogs


    The input value can be a string, a number or an item from a list. A label
    must be set to tell the user what they should enter.

    Four static convenience functions are provided: getText(), getInt(),
    getDouble(), and getItem(). All the functions can be used in a similar way,
    for example:

    \snippet examples/dialogs/standarddialogs/dialog.cpp 3

    The \c ok variable is set to true if the user clicks \gui OK; otherwise it
    is set to false.

    \img inputdialogs.png Input Dialogs

    The \l{dialogs/standarddialogs}{Standard Dialogs} example shows how to use
    QInputDialog as well as other built-in Qt dialogs.

    \sa QMessageBox, {Standard Dialogs Example}
*/

/*!
    \enum QInputDialog::InputMode
    \since 4.5

    This enum describes the different modes of input that can be selected for
    the dialog.

    \value TextInput   Used to input text strings.
    \value IntInput    Used to input integers.
    \value DoubleInput Used to input floating point numbers with double
                       precision accuracy.

    \sa inputMode
*/

/*!
    \since 4.5

    Constructs a new input dialog with the given \a parent and window \a flags.
*/
QInputDialog::QInputDialog(QWidget *parent, Qt::WindowFlags flags)
   : QDialog(*new QInputDialogPrivate, parent, flags)
{
}

/*!
    \since 4.5

    Destroys the input dialog.
*/
QInputDialog::~QInputDialog()
{
}

/*!
    \since 4.5

    \property QInputDialog::inputMode

    \brief the mode used for input

    This property help determines which widget is used for entering input into
    the dialog.
*/
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

/*!
    \since 4.5

    \property QInputDialog::labelText

    \brief the text to for the label to describe what needs to be input
*/
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

/*!
    \enum QInputDialog::InputDialogOption

    \since 4.5

    This enum specifies various options that affect the look and feel
    of an input dialog.

    \value NoButtons Don't display \gui{OK} and \gui{Cancel} buttons. (Useful for "live dialogs".)
    \value UseListViewForComboBoxItems Use a QListView rather than a non-editable QComboBox for
                                       displaying the items set with setComboBoxItems().

    \sa options, setOption(), testOption()
*/

/*!
    Sets the given \a option to be enabled if \a on is true;
    otherwise, clears the given \a option.

    \sa options, testOption()
*/
void QInputDialog::setOption(InputDialogOption option, bool on)
{
   Q_D(QInputDialog);
   if (!(d->opts & option) != !on) {
      setOptions(d->opts ^ option);
   }
}

/*!
    Returns true if the given \a option is enabled; otherwise, returns
    false.

    \sa options, setOption()
*/
bool QInputDialog::testOption(InputDialogOption option) const
{
   Q_D(const QInputDialog);
   return (d->opts & option) != 0;
}

/*!
    \property QInputDialog::options
    \brief the various options that affect the look and feel of the dialog
    \since 4.5

    By default, all options are disabled.

    \sa setOption(), testOption()
*/
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
}

QInputDialog::InputDialogOptions QInputDialog::options() const
{
   Q_D(const QInputDialog);
   return d->opts;
}

/*!
    \since 4.5

    \property QInputDialog::textValue

    \brief the text value for the input dialog

    This property is only relevant when the input dialog is used in
    TextInput mode.
*/
void QInputDialog::setTextValue(const QString &text)
{
   Q_D(QInputDialog);

   setInputMode(TextInput);
   if (d->inputWidget == d->lineEdit) {
      d->lineEdit->setText(text);
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

/*!
    \since 4.5

    \property QInputDialog::textEchoMode

    \brief the echo mode for the text value

    This property is only relevant when the input dialog is used in
    TextInput mode.
*/
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

/*!
    \since 4.5

    \property QInputDialog::comboBoxEditable

    \brief whether or not the combo box is used in the input dialog is editable
*/
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

/*!
    \since 4.5

    \property QInputDialog::comboBoxItems

    \brief the items used in the combobox for the input dialog
*/
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

/*!
    \property QInputDialog::intValue
    \since 4.5
    \brief the current integer value accepted as input

    This property is only relevant when the input dialog is used in
    IntInput mode.
*/
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

/*!
    \property QInputDialog::intMinimum
    \since 4.5
    \brief the minimum integer value accepted as input

    This property is only relevant when the input dialog is used in
    IntInput mode.
*/
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

/*!
    \property QInputDialog::intMaximum
    \since 4.5
    \brief the maximum integer value accepted as input

    This property is only relevant when the input dialog is used in
    IntInput mode.
*/
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

/*!
    Sets the range of integer values accepted by the dialog when used in
    IntInput mode, with minimum and maximum values specified by \a min and
    \a max respectively.
*/
void QInputDialog::setIntRange(int min, int max)
{
   Q_D(QInputDialog);
   d->ensureIntSpinBox();
   d->intSpinBox->setRange(min, max);
}

/*!
    \property QInputDialog::intStep
    \since 4.5
    \brief the step by which the integer value is increased and decreased

    This property is only relevant when the input dialog is used in
    IntInput mode.
*/
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

/*!
    \property QInputDialog::doubleValue
    \since 4.5
    \brief the current double precision floating point value accepted as input

    This property is only relevant when the input dialog is used in
    DoubleInput mode.
*/
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

/*!
    \property QInputDialog::doubleMinimum
    \since 4.5
    \brief the minimum double precision floating point value accepted as input

    This property is only relevant when the input dialog is used in
    DoubleInput mode.
*/
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

/*!
    \property QInputDialog::doubleMaximum
    \since 4.5
    \brief the maximum double precision floating point value accepted as input

    This property is only relevant when the input dialog is used in
    DoubleInput mode.
*/
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

/*!
    Sets the range of double precision floating point values accepted by the
    dialog when used in DoubleInput mode, with minimum and maximum values
    specified by \a min and \a max respectively.
*/
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

/*!
    \since 4.5

    \property QInputDialog::okButtonText

    \brief the text for the button used to accept the entry in the dialog
*/
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

/*!
    \since 4.5

    \property QInputDialog::cancelButtonText
    \brief the text for the button used to cancel the dialog
*/
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

/*!
    \since 4.5
    \overload

    This function connects one of its signals to the slot specified by \a receiver
    and \a member. The specific signal depends on the arguments that are specified
    in \a member. These are:

    \list
      \o textValueSelected() if \a member has a QString for its first argument.
      \o intValueSelected() if \a member has an int for its first argument.
      \o doubleValueSelected() if \a member has a double for its first argument.
      \o accepted() if \a member has NO arguments.
    \endlist

    The signal will be disconnected from the slot when the dialog is closed.
*/
void QInputDialog::open(QObject *receiver, const QString &member)
{
   Q_D(QInputDialog);

   connect(this, signalForMember(member), receiver, member);
   d->receiverToDisconnectOnClose = receiver;
   d->memberToDisconnectOnClose = member;

   QDialog::open();
}

/*!
    \reimp
*/
QSize QInputDialog::minimumSizeHint() const
{
   Q_D(const QInputDialog);
   d->ensureLayout();
   return QDialog::minimumSizeHint();
}

/*!
    \reimp
*/
QSize QInputDialog::sizeHint() const
{
   Q_D(const QInputDialog);
   d->ensureLayout();
   return QDialog::sizeHint();
}

/*!
    \reimp
*/
void QInputDialog::setVisible(bool visible)
{
   Q_D(const QInputDialog);
   if (visible) {
      d->ensureLayout();
      d->inputWidget->setFocus();
      if (d->inputWidget == d->lineEdit) {
         d->lineEdit->selectAll();
      } else if (d->inputWidget == d->intSpinBox) {
         d->intSpinBox->selectAll();
      } else if (d->inputWidget == d->doubleSpinBox) {
         d->doubleSpinBox->selectAll();
      }
   }
   QDialog::setVisible(visible);
}

/*!
  Closes the dialog and sets its result code to \a result. If this dialog
  is shown with exec(), done() causes the local event loop to finish,
  and exec() to return \a result.

  \sa QDialog::done()
*/
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

      d->receiverToDisconnectOnClose = 0;
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

   int ret = dialog.exec();
   if (ok) {
      *ok = !!ret;
   }
   if (ret) {
      return dialog.textValue();
   } else {
      return QString();
   }
}

/*!
    \internal
*/
// ### Qt5: Use only the version above.
QString QInputDialog::getText(QWidget *parent, const QString &title, const QString &label,
                  QLineEdit::EchoMode mode, const QString &text, bool *ok, Qt::WindowFlags flags)
{
   return getText(parent, title, label, mode, text, ok, flags, Qt::ImhNone);
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




   int ret = dialog.exec();

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

   int ret = dialog.exec();
   if (ok) {
      *ok = !!ret;
   }
   if (ret) {
      return dialog.textValue();
   } else {
      return text;
   }
}

/*!
    \internal
*/
// ### Qt5: Use only the version above.
QString QInputDialog::getItem(QWidget *parent, const QString &title, const QString &label,
                              const QStringList &items, int current, bool editable, bool *ok,
                              Qt::WindowFlags flags)
{
   return getItem(parent, title, label, items, current, editable, ok, flags, Qt::ImhNone);
}

/*!
    \obsolete

    Use getInt() instead.
*/
int QInputDialog::getInteger(QWidget *parent, const QString &title, const QString &label,
                             int value, int min, int max, int step, bool *ok,
                             Qt::WindowFlags flags)
{
   return getInt(parent, title, label, value, min, max, step, ok, flags);
}


#endif
