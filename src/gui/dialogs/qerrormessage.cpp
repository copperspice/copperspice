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

#include <qerrormessage.h>

#ifndef QT_NO_ERRORMESSAGE

#include <qapplication.h>
#include <qcheckbox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qmessagebox.h>
#include <qmetaobject.h>
#include <qpixmap.h>
#include <qpushbutton.h>
#include <qset.h>
#include <qstringlist.h>
#include <qtextedit.h>
#include <qthread.h>

#include <qdialog_p.h>

#include <queue>
#include <stdio.h>
#include <stdlib.h>

class QErrorMessagePrivate : public QDialogPrivate
{
   Q_DECLARE_PUBLIC(QErrorMessage)

 public:
   QPushButton *ok;
   QCheckBox *again;
   QTextEdit *errors;
   QLabel *icon;

   std::queue<QPair<QString, QString>> pending;
   QSet<QString> doNotShow;
   QSet<QString> doNotShowType;
   QString currentMessage;
   QString currentType;

   bool isMessageToBeShown(const QString &message, const QString &type) const;
   bool nextPending();
   void retranslateStrings();
};

class QErrorMessageTextView : public QTextEdit
{
 public:
   QErrorMessageTextView(QWidget *parent)
      : QTextEdit(parent)
   {
      setReadOnly(true);
   }

   QSize minimumSizeHint() const override;
   QSize sizeHint() const override;
};

QSize QErrorMessageTextView::minimumSizeHint() const
{
   return QSize(50, 50);
}

QSize QErrorMessageTextView::sizeHint() const
{
   return QSize(250, 75);
}

QErrorMessage::QErrorMessage(QWidget *parent)
   : QDialog(*new QErrorMessagePrivate, parent)
{
   Q_D(QErrorMessage);
   d->icon   = new QLabel(this);
   d->errors = new QErrorMessageTextView(this);
   d->again  = new QCheckBox(this);
   d->ok     = new QPushButton(this);

   QGridLayout *grid = new QGridLayout(this);
   connect(d->ok, &QPushButton::clicked, this, &QErrorMessage::accept);

   grid->addWidget(d->icon,   0,  0, Qt::AlignTop);
   grid->addWidget(d->errors, 0,  1);
   grid->addWidget(d->again,  1,  1, Qt::AlignTop);
   grid->addWidget(d->ok,     2, 0, 1, 2, Qt::AlignCenter);

   grid->setColumnStretch(1, 42);
   grid->setRowStretch(0, 42);

#ifndef QT_NO_MESSAGEBOX
   d->icon->setPixmap(QMessageBox::standardIcon(QMessageBox::Information));
   d->icon->setAlignment(Qt::AlignHCenter | Qt::AlignTop);
#endif

   d->again->setChecked(true);
   d->ok->setFocus();
   d->retranslateStrings();
}

QErrorMessage::~QErrorMessage()
{
}

void QErrorMessage::done(int a)
{
   Q_D(QErrorMessage);

   if (! d->again->isChecked())  {
      if (d->currentType.isEmpty()) {

         if (! d->currentMessage.isEmpty()) {
            d->doNotShow.insert(d->currentMessage);
         }

      } else {
         d->doNotShowType.insert(d->currentType);
      }
   }

   d->currentMessage.clear();
   d->currentType.clear();

   if (! d->nextPending()) {
      QDialog::done(a);
   }
}

bool QErrorMessagePrivate::isMessageToBeShown(const QString &message, const QString &type) const
{
   if (message.isEmpty()) {
      // nothing to show
      return false;
   }

   if (type.isEmpty()) {

      if (doNotShow.contains(message)) {
         return false;
      }

   } else {

      if (doNotShowType.contains(type)) {
         return false;
      }
   }

   return true;
}

bool QErrorMessagePrivate::nextPending()
{
   while (! pending.empty()) {
      QPair<QString, QString> &pendingMessage = pending.front();

      QString message = std::move(pendingMessage.first);
      QString type    = std::move(pendingMessage.second);

      pending.pop();

      if (isMessageToBeShown(message, type)) {
#ifndef QT_NO_TEXTHTMLPARSER
         errors->setHtml(message);
#else
         errors->setPlainText(message);
#endif

         currentMessage = std::move(message);
         currentType    = std::move(type);
         return true;
      }
   }

   return false;
}

void QErrorMessage::showMessage(const QString &message)
{
   showMessage(message, QString());
}

void QErrorMessage::showMessage(const QString &message, const QString &type)
{
   Q_D(QErrorMessage);

   if (! d->isMessageToBeShown(message, type)) {
      return;
   }

   d->pending.push(qMakePair(message, type));

   if (! isVisible() && d->nextPending()) {
      show();
   }
}

void QErrorMessage::changeEvent(QEvent *e)
{
   Q_D(QErrorMessage);

   if (e->type() == QEvent::LanguageChange) {
      d->retranslateStrings();
   }

   QDialog::changeEvent(e);
}

void QErrorMessagePrivate::retranslateStrings()
{
   again->setText(QErrorMessage::tr("&Show this message again"));
   ok->setText(QErrorMessage::tr("&OK"));
}

#endif
