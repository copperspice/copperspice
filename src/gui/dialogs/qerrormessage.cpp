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
#include <qdialog_p.h>
#include <qpixmap.h>
#include <qmetaobject.h>
#include <qthread.h>
#include <qqueue.h>
#include <qset.h>
#include <qnamespace.h>

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

   QQueue<QPair<QString, QString> > pending;
   QSet<QString> doNotShow;
   QSet<QString> doNotShowType;
   QString currentMessage;
   QString currentType;

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

/*!
    \class QErrorMessage

    \brief The QErrorMessage class provides an error message display dialog.

    \ingroup standard-dialog

    An error message widget consists of a text label and a checkbox. The
    checkbox lets the user control whether the same error message will be
    displayed again in the future, typically displaying the text,
    "Show this message again" translated into the appropriate local
    language.

    For production applications, the class can be used to display messages which
    the user only needs to see once. To use QErrorMessage like this, you create
    the dialog in the usual way, and show it by calling the showMessage() slot or
    connecting signals to it.

    The static qtHandler() function installs a message handler
    using qInstallMsgHandler() and creates a QErrorMessage that displays
    qDebug(), qWarning() and qFatal() messages. This is most useful in
    environments where no console is available to display warnings and
    error messages.

    In both cases QErrorMessage will queue pending messages and display
    them in order, with each new message being shown as soon as the user
    has accepted the previous message. Once the user has specified that a
    message is not to be shown again it is automatically skipped, and the
    dialog will show the next appropriate message in the queue.

    The \l{dialogs/standarddialogs}{Standard Dialogs} example shows
    how to use QErrorMessage as well as other built-in Qt dialogs.

    \img qerrormessage.png

    \sa QMessageBox, QStatusBar::showMessage(), {Standard Dialogs Example}
*/

static QErrorMessage *qtMessageHandler = 0;

static void deleteStaticcQErrorMessage() // post-routine
{
   if (qtMessageHandler) {
      delete qtMessageHandler;
      qtMessageHandler = 0;
   }
}

static bool metFatal = false;

static void jump(QtMsgType t, const char *m)
{
   if (!qtMessageHandler) {
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
   rich = QString::fromLatin1("<p><b>%1</b></p>").arg(rich);
   rich += Qt::convertFromPlainText(QLatin1String(m), Qt::WhiteSpaceNormal);

   // ### work around text engine quirk
   if (rich.endsWith(QLatin1String("</p>"))) {
      rich.chop(4);
   }

   if (!metFatal) {
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

   QGridLayout *grid = new QGridLayout(this);

   d->icon   = new QLabel(this);
   d->errors = new QErrorMessageTextView(this);
   d->again  = new QCheckBox(this);
   d->ok     = new QPushButton(this);

#ifndef QT_NO_MESSAGEBOX
   d->icon->setPixmap(QMessageBox::standardIcon(QMessageBox::Information));
   d->icon->setAlignment(Qt::AlignHCenter | Qt::AlignTop);
#endif

   grid->addWidget(d->icon,   0,  0, Qt::AlignTop);
   grid->addWidget(d->errors, 0,  1);
   grid->addWidget(d->again,  1,  1, Qt::AlignTop);
   grid->addWidget(d->ok,     2, 0, 1, 2, Qt::AlignCenter);

   grid->setColumnStretch(1, 42);
   grid->setRowStretch(0, 42);

   connect(d->ok, SIGNAL(clicked()), this, SLOT(accept()));

   d->again->setChecked(true);
   d->ok->setFocus();
   d->retranslateStrings();
}


/*!
    Destroys the error message dialog.
*/

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
   if (!d->again->isChecked() && !d->currentMessage.isEmpty() && d->currentType.isEmpty()) {
      d->doNotShow.insert(d->currentMessage);
   }
   if (!d->again->isChecked() && !d->currentType.isEmpty()) {
      d->doNotShowType.insert(d->currentType);
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


/*!
    Returns a pointer to a QErrorMessage object that outputs the
    default Qt messages. This function creates such an object, if there
    isn't one already.
*/

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

bool QErrorMessagePrivate::nextPending()
{
   while (!pending.isEmpty()) {
      QPair<QString, QString> pendingMessage = pending.dequeue();
      QString message = pendingMessage.first;
      QString type = pendingMessage.second;
      if (!message.isEmpty() && ((type.isEmpty() && !doNotShow.contains(message)) || (!type.isEmpty() &&
                                 !doNotShowType.contains(type)))) {
#ifndef QT_NO_TEXTHTMLPARSER
         errors->setHtml(message);
#else
         errors->setPlainText(message);
#endif
         currentMessage = message;
         currentType = type;
         return true;
      }
   }
   return false;
}


/*!
    Shows the given message, \a message, and returns immediately. If the user
    has requested for the message not to be shown again, this function does
    nothing.

    Normally, the message is displayed immediately. However, if there are
    pending messages, it will be queued to be displayed later.
*/

void QErrorMessage::showMessage(const QString &message)
{
   Q_D(QErrorMessage);
   if (d->doNotShow.contains(message)) {
      return;
   }
   d->pending.enqueue(qMakePair(message, QString()));
   if (!isVisible() && d->nextPending()) {
      show();
   }
}

void QErrorMessage::showMessage(const QString &message, const QString &type)
{
   Q_D(QErrorMessage);
   if (d->doNotShow.contains(message) && d->doNotShowType.contains(type)) {
      return;
   }
   d->pending.push_back(qMakePair(message, type));
   if (!isVisible() && d->nextPending()) {
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

#endif // QT_NO_ERRORMESSAGE
