/***********************************************************************
*
* Copyright (c) 2012-2018 Barbara Geller
* Copyright (c) 2012-2018 Ansel Sermersheim
* Copyright (c) 2012-2016 Digia Plc and/or its subsidiary(-ies).
* Copyright (c) 2008-2012 Nokia Corporation and/or its subsidiary(-ies).
* All rights reserved.
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
* <http://www.gnu.org/licenses/>.
*
***********************************************************************/

#include <qerrormessage.h>

#ifndef QT_NO_ERRORMESSAGE

#include <qapplication.h>
#include <qcheckbox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qmessagebox.h>
#include <qpushbutton.h>
#include <qstringlist.h>
#include <qtextedit.h>
#include <qpixmap.h>
#include <qmetaobject.h>
#include <qthread.h>
#include <qset.h>

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
      : QTextEdit(parent) {
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






static QErrorMessage *qtMessageHandler = 0;

static void deleteStaticcQErrorMessage() // post-routine
{
   if (qtMessageHandler) {
      delete qtMessageHandler;
      qtMessageHandler = 0;
   }
}

static bool metFatal = false;

static void jump(QtMsgType t, QStringView msg)
{
   if (! qtMessageHandler) {
      return;
   }

   QString rich;

   switch (t) {
      case QtDebugMsg:
      default:
         rich = QErrorMessage::tr("Debug Message:");
         break;

      case QtWarningMsg:
         rich = QErrorMessage::tr("Warning:");
         break;

      case QtFatalMsg:
         rich = QErrorMessage::tr("Fatal Error:");
   }

   rich  = QString("<p><b>%1</b></p>").formatArg(rich);
   rich += Qt::convertFromPlainText(msg, Qt::WhiteSpaceNormal);

   // ### work around text engine quirk
   if (rich.endsWith("</p>")) {
      rich.chop(4);
   }

   if (! metFatal) {
      if (QThread::currentThread() == qApp->thread()) {
         qtMessageHandler->showMessage(rich);
      } else {
         QMetaObject::invokeMethod(qtMessageHandler, "showMessage", Qt::QueuedConnection, Q_ARG(const QString &, rich));
      }

      metFatal = (t == QtFatalMsg);
   }
}


/*!
    Constructs and installs an error handler window with the given parent.
*/

QErrorMessage::QErrorMessage(QWidget *parent)
   : QDialog(*new QErrorMessagePrivate, parent)
{
   Q_D(QErrorMessage);
   d->icon   = new QLabel(this);
   d->errors = new QErrorMessageTextView(this);
   d->again  = new QCheckBox(this);
   d->ok     = new QPushButton(this);

   QGridLayout *grid = new QGridLayout(this);
   connect(d->ok, SIGNAL(clicked()), this, SLOT(accept()));


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
   if (this == qtMessageHandler) {
      qtMessageHandler = 0;
      QtMsgHandler tmp = qInstallMsgHandler(0);

      // in case someone else has later stuck in another...
      if (tmp != jump) {
         qInstallMsgHandler(tmp);
      }
   }
}

void QErrorMessage::done(int a)
{
   Q_D(QErrorMessage);

   if (! d->again->isChecked())  {
      if (d->currentType.isEmpty()) {
         if (!d->currentMessage.isEmpty()) {
            d->doNotShow.insert(d->currentMessage);
         }
      } else {
         d->doNotShowType.insert(d->currentType);
      }
   }

   d->currentMessage.clear();
   d->currentType.clear();

   if (!d->nextPending()) {
      QDialog::done(a);
      if (this == qtMessageHandler && metFatal) {
         exit(1);
      }
   }
}


QErrorMessage *QErrorMessage::qtHandler()
{
   if (!qtMessageHandler) {
      qtMessageHandler = new QErrorMessage(0);
      qAddPostRoutine(deleteStaticcQErrorMessage); // clean up
      qtMessageHandler->setWindowTitle(QApplication::applicationName());
      qInstallMsgHandler(jump);
   }
   return qtMessageHandler;
}


/*! \internal */

bool QErrorMessagePrivate::isMessageToBeShown(const QString &message, const QString &type) const
{
   return !message.isEmpty()
      && (type.isEmpty() ? !doNotShow.contains(message) : !doNotShowType.contains(type));
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

/*!
    \reimp
*/
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
