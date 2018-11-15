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

#include <qprogressdialog.h>

#ifndef QT_NO_PROGRESSDIALOG

#include <qapplication.h>
#include <qcursor.h>
#include <qdrawutil.h>
#include <qelapsedtimer.h>

#include <qlabel.h>
#include <qprogressbar.h>
#include <qpushbutton.h>
#include <qpainter.h>
#include <qstyle.h>
#include <qshortcut.h>
#include <QTimer>
#include <QVBoxLayout>

#include <qdialog_p.h>

#include <limits.h>

// If the operation is expected to take this long (as predicted by progress time),
// show the progress dialog
static const int defaultShowTime = 4000;

// wait at least the minWaitTime long before attempting to make a prediction
static const int minWaitTime = 50;


QProgressDialog::QProgressDialog(QWidget *parent, Qt::WindowFlags f)
   : QDialog(parent, f)
{
   useDefaultCancelText = true;
   init(QString(), QString(), 0, 100);
}

QProgressDialog::QProgressDialog(const QString &labelText, const QString &cancelButtonText,
   int minimum, int maximum, QWidget *parent, Qt::WindowFlags f)
   : QDialog(parent, f)
{
   init(labelText, cancelButtonText, minimum, maximum);
}

QProgressDialog::~QProgressDialog()
{
}

void QProgressDialog::init(const QString &labelText, const QString &cancelText, int min, int max)
{
   shown_once   = false;
   m_autoClose  = true;
   m_autoReset  = true;
   forceHide    = false;

   // broom - gone?   m_centerCancelPB  = false;
   // broom - gone?   cancellation_flag = false;

   //
   m_label = new QLabel(labelText, this);

   int alignL = style()->styleHint(QStyle::SH_ProgressDialog_TextLabelAlignment, 0, this);
   m_label->setAlignment(Qt::Alignment(alignL));

   //
   m_progressBar = new QProgressBar(this);
   m_progressBar->setRange(min, max);

   // broom - gone?   c  m_progressBar->setSizePolicy( QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);

   // broom - gone?   cm_cancelButton =  new QPushButton();
   // broom - gone?   cm_centerCancelPB = style()->styleHint(QStyle::SH_ProgressDialog_CenterCancelButton, 0, this);

   if (useDefaultCancelText) {
      retranslateStrings();

   } else {
      setCancelButtonText(cancelText);

   }

   // broom - gone?   cconnect(m_cancelButton, SIGNAL(clicked()),  this, SLOT(canceled()));
   QObject::connect(this,  SIGNAL(canceled()), this, SLOT(cancel()));

   //
   // broom - gone?   cm_layout = new QVBoxLayout();
   // broom - gone?   cm_layout->addWidget(m_label);
   // broom - gone?   cm_layout->addWidget(m_progressBar);
   // broom - gone?   cm_layout->addWidget(m_cancelButton);
   // broom - gone?   cm_layout->setSpacing(9);

   // broom - gone?   csetCancelButtonAlignment();

   // broom - gone?   csetWindowTitle(tr("Progress Bar"));
   // broom - gone?   csetLayout(m_layout);

   starttime.start();

   // broom - gone?   cshowTime   = defaultShowTime;
   forceTimer = new QTimer(this);
   forceTimer->start(showTime);

   QObject::connect(forceTimer, SIGNAL(timeout()), this, SLOT(forceShow()));
}

void QProgressDialog::cancel()
{
   forceHide = true;
   reset();

   forceHide = false;
   cancellation_flag = true;
}

void QProgressDialog::layout()
{
   int sp  = style()->pixelMetric(QStyle::PM_DefaultLayoutSpacing);
   int mtb = style()->pixelMetric(QStyle::PM_DefaultTopLevelMargin);
   int mlr = qMin(width() / 10, mtb);

   const bool centered = bool(style()->styleHint(QStyle::SH_ProgressDialog_CenterCancelButton, 0, this));

   int additionalSpacing = 0;
   QSize cs = m_cancelButton ? m_cancelButton->sizeHint() : QSize(0, 0);
   QSize bh = m_progressBar->sizeHint();
   int cspc;
   int lh = 0;

   // Find spacing and sizes that fit.  It is important that a progress
   // dialog can be made very small if the user demands it so.
   for (int attempt = 5; attempt--;) {
      cspc = m_cancelButton ? cs.height() + sp : 0;
      lh = qMax(0, height() - mtb - bh.height() - sp - cspc);

      if (lh < height() / 4) {
         // Getting cramped
         sp  /= 2;
         mtb /= 2;

         if (m_cancelButton) {
            cs.setHeight(qMax(4, cs.height() - sp - 2));
         }
         bh.setHeight(qMax(4, bh.height() - sp - 1));

      } else {
         break;
      }
   }

   if (m_cancelButton) {
      m_cancelButton->setGeometry(
         centered ? width() / 2 - cs.width() / 2 : width() - mlr - cs.width(),
         height() - mtb - cs.height(), cs.width(), cs.height());
   }

   if (m_label) {
      m_label->setGeometry(mlr, additionalSpacing, width() - mlr * 2, lh);
   }

   m_progressBar->setGeometry(mlr, lh + sp + additionalSpacing, width() - mlr * 2, bh.height());
}

void QProgressDialog::disconnectOnClose()
{
   if (receiverToDisconnectOnClose) {
      QObject::disconnect(this, SIGNAL(canceled()), receiverToDisconnectOnClose, memberToDisconnectOnClose);
      receiverToDisconnectOnClose = 0;
   }

   memberToDisconnectOnClose.clear();
}

QString QProgressDialog::labelText() const
{
   if (m_label) {
      return m_label->text();
   }

   return QString();
}

void QProgressDialog::reset()
{

#ifndef QT_NO_CURSOR
   if (value() >= 0) {
      if (parentWidget()) {
         parentWidget()->setCursor(parentCursor);
      }
   }
#endif

   if (m_autoClose || forceHide) {
      hide();
   }

   m_progressBar->reset();

   cancellation_flag = false;
   shown_once        = false;
   setValue_called   = false;
   forceTimer->stop();

   /*
       we could disconnect the user slot provided to open() here but unfortunately reset() is
       usually called before the slot has been invoked.
       reset() is itself invoked when canceled() is emitted.
   */

   if (receiverToDisconnectOnClose) {
      QMetaObject::invokeMethod(this, "disconnectOnClose", Qt::QueuedConnection);
   }
}

void QProgressDialog::retranslateStrings()
{
   if (useDefaultCancelText) {
      setCancelButtonText(QProgressDialog::tr("Cancel"));
   }
}

void QProgressDialog::setCancelButton(QPushButton *newButton)
{
   if (m_cancelButton == newButton) {
      if (newButton) {
         qWarning("QProgressDialog::setCancelButton: Attempt to set the same button twice");
      }

      return;
   }

   delete m_cancelButton;
   m_cancelButton = newButton;

   if (m_cancelButton) {
      // BROOM  gone ?  m_layout->insertWidget(CANCEL_BUTTON_INDEX, m_cancelButton);

      connect(m_cancelButton, SIGNAL(clicked()), this, SLOT(canceled()));

#ifndef QT_NO_SHORTCUT
      escapeShortcut = new QShortcut(QKeySequence::Cancel, this, SLOT(canceled()));
#endif


   } else {

#ifndef QT_NO_SHORTCUT
      delete escapeShortcut;
      escapeShortcut = 0;
#endif

   }

   adoptChildWidget(newButton);
}

/* broom - may not be used
void QProgressDialog::setCancelButtonAlignment()
{
   Qt::Alignment alignPB;

   if (m_centerCancelPB) {
      alignPB = Qt::AlignHCenter;

   }  else {
      alignPB = Qt::AlignRight;

   }

   if (m_cancelButton) {
      m_layout->itemAt(CANCEL_BUTTON_INDEX)->setAlignment(alignPB);
   }
}

void QProgressDialog::setCancelButtonCentered(bool value)
{
   m_centerCancelPB = value;
   setCancelButtonAlignment();
}

*/

void QProgressDialog::setLabel(QLabel *newLabel)
{
   if (newLabel == m_label) {
      if (newLabel) {
         qWarning("QProgressDialog::setLabel: Attempt to set the same label twice");
      }

      return;
   }

   delete m_label;
   m_label = newLabel;

   adoptChildWidget(newLabel);
}

void QProgressDialog::setCancelButtonText(const QString &cancelButtonText)
{
   useDefaultCancelText = false;
   setCancelButtonText(cancelButtonText);

   if (! cancelButtonText.isEmpty()) {

      if (m_cancelButton) {
         m_cancelButton->setText(cancelButtonText);

      } else {
         setCancelButton(new QPushButton(cancelButtonText, this));

      }

   } else {
      setCancelButton(0);

   }

   ensureSizeIsAtLeastSizeHint();
}

void QProgressDialog::setBar(QProgressBar *newBar)
{
   if (! m_progressBar) {
      qWarning("QProgressDialog::setBar() Can not set a null progress bar");
      return;
   }


   if (newBar == m_progressBar) {
      qWarning("QProgressDialog::setBar: Attempt to set the same progress bar twice");
      return;
   }

   delete m_progressBar;
   m_progressBar = newBar;

   adoptChildWidget(newBar);
}

void QProgressDialog::setLabelText(const QString &text)
{
   if (m_label) {
      m_label->setText(text);
      ensureSizeIsAtLeastSizeHint();
   }
}

void QProgressDialog::adoptChildWidget(QWidget *control)
{
   if (control) {
      if (control->parentWidget() == this)  {
         control->hide();
      } else {
         control->setParent(this, 0);
      }
   }

   ensureSizeIsAtLeastSizeHint();

   if (control) {
      control->show();
   }
}
QSize QProgressDialog::sizeHint() const
{
   QSize sh = m_label ? m_label->sizeHint() : QSize(0, 0);
   QSize bh = m_progressBar->sizeHint();

   int margin  = style()->pixelMetric(QStyle::PM_DefaultTopLevelMargin);
   int spacing = style()->pixelMetric(QStyle::PM_DefaultLayoutSpacing);
   int h = margin * 2 + bh.height() + sh.height() + spacing;

   if (m_cancelButton) {
      h += m_cancelButton->sizeHint().height() + spacing;
   }

   return QSize(qMax(200, sh.width() + 2 * margin), h);
}

void QProgressDialog::ensureSizeIsAtLeastSizeHint()
{

   QSize size = sizeHint();
   if (isVisible()) {
      size = size.expandedTo(this->size());
   }

   resize(size);
}

int QProgressDialog::maximum() const
{
   return m_progressBar->maximum();
}

void QProgressDialog::setMaximum(int maximum)
{
   return m_progressBar->setMaximum(maximum);
}

int QProgressDialog::minimum() const
{
   return m_progressBar->minimum();
}

void QProgressDialog::setMinimum(int minimum)
{
   m_progressBar->setMinimum(minimum);
}

void QProgressDialog::setRange(int minimum, int maximum)
{
   m_progressBar->setRange(minimum, maximum);
}

bool QProgressDialog::wasCanceled() const
{
   return cancellation_flag;
}

int QProgressDialog::value() const
{
   return m_progressBar->value();
}

void QProgressDialog::setValue(int progress)
{
   if (setValue_called && progress == m_progressBar->value()) {
      return;
   }

   m_progressBar->setValue(progress);

   if (shown_once) {
      if (isModal()) {
         QApplication::processEvents();
      }

   } else {

      if ((!setValue_called && progress == 0) || progress == minimum()) {
         starttime.start();
         forceTimer->start(showTime);
         setValue_called = true;
         return;

      } else {
         setValue_called = true;

         bool need_show;
         int elapsed = starttime.elapsed();

         if (elapsed >= showTime) {
            need_show = true;

         } else {
            if (elapsed > minWaitTime) {
               int estimate;
               int totalSteps = maximum() - minimum();
               int myprogress = progress - minimum();

               if (myprogress == 0) {
                  myprogress = 1;
               }

               if ((totalSteps - myprogress) >= INT_MAX / elapsed)  {
                  estimate = (totalSteps - myprogress) / myprogress * elapsed;
               }  else  {
                  estimate = elapsed * (totalSteps - myprogress) / myprogress;
               }

               need_show = (estimate >= showTime);

            } else {
               need_show = false;
            }
         }

         if (need_show) {
            ensureSizeIsAtLeastSizeHint();
            show();
            shown_once = true;
         }
      }
   }

   if (progress == m_progressBar->maximum() && m_autoReset)  {
      reset();
   }
}

void QProgressDialog::resizeEvent(QResizeEvent *)
{
   layout();
}

void QProgressDialog::changeEvent(QEvent *ev)
{
   if (ev->type() == QEvent::StyleChange) {
      layout();

   } else if (ev->type() == QEvent::LanguageChange) {
      retranslateStrings();

   }

   QDialog::changeEvent(ev);
}

void QProgressDialog::setMinimumDuration(int ms)
{
   showTime = ms;

   if (m_progressBar->value() == m_progressBar->minimum()) {
      forceTimer->stop();
      forceTimer->start(ms);
   }
}

int QProgressDialog::minimumDuration() const
{
   return showTime;
}

void QProgressDialog::closeEvent(QCloseEvent *e)
{
   emit canceled();
   QDialog::closeEvent(e);
}

void QProgressDialog::setAutoReset(bool b)
{
   m_autoReset = b;
}

bool QProgressDialog::autoReset() const
{
   return m_autoReset;
}

void QProgressDialog::setAutoClose(bool close)
{
   m_autoClose = close;
}

bool QProgressDialog::autoClose() const
{
   return m_autoClose;
}

void QProgressDialog::showEvent(QShowEvent *e)
{
   QDialog::showEvent(e);
   ensureSizeIsAtLeastSizeHint();
   forceTimer->stop();
}

void QProgressDialog::forceShow()
{
   forceTimer->stop();
   if (shown_once || cancellation_flag)  {
      return;
   }

   show();
   shown_once = true;
}

void QProgressDialog::open(QObject *receiver, const QString &member)
{
   connect(this, SIGNAL(canceled()), receiver, member);

   receiverToDisconnectOnClose = receiver;
   memberToDisconnectOnClose   = member;

   QDialog::open();
}



#endif // QT_NO_PROGRESSDIALOG
