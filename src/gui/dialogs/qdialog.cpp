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

#include <qdialog.h>
#include <qevent.h>
#include <qdesktopwidget.h>
#include <qpushbutton.h>
#include <qapplication.h>
#include <qlayout.h>
#include <qsizegrip.h>
#include <qwhatsthis.h>
#include <qmenu.h>
#include <qcursor.h>
#include <qdialog_p.h>

#ifndef QT_NO_ACCESSIBILITY
#include <qaccessible.h>
#endif

#if defined(Q_WS_X11)
#include <qt_x11_p.h>
#endif

#ifndef SPI_GETSNAPTODEFBUTTON
#define SPI_GETSNAPTODEFBUTTON  95
#endif

QT_BEGIN_NAMESPACE

QDialog::QDialog(QWidget *parent, Qt::WindowFlags f)
   : QWidget(*new QDialogPrivate, parent, f | ((f & Qt::WindowType_Mask) == 0 ? Qt::Dialog : Qt::WindowType(0)))
{
}


/*!
  \overload
  \internal
*/
QDialog::QDialog(QDialogPrivate &dd, QWidget *parent, Qt::WindowFlags f)
   : QWidget(dd, parent, f | ((f & Qt::WindowType_Mask) == 0 ? Qt::Dialog : Qt::WindowType(0)))
{
}

/*!
  Destroys the QDialog, deleting all its children.
*/

QDialog::~QDialog()
{
   QT_TRY {
      // Need to hide() here, as our (to-be) overridden hide()
      // will not be called in ~QWidget.
      hide();
   } QT_CATCH(...) {
      // we're in the destructor - just swallow the exception
   }
}

/*!
  \internal
  This function is called by the push button \a pushButton when it
  becomes the default button. If \a pushButton is 0, the dialogs
  default default button becomes the default button. This is what a
  push button calls when it loses focus.
*/
void QDialogPrivate::setDefault(QPushButton *pushButton)
{
   Q_Q(QDialog);
   bool hasMain = false;
   QList<QPushButton *> list = q->findChildren<QPushButton *>();
   for (int i = 0; i < list.size(); ++i) {
      QPushButton *pb = list.at(i);
      if (pb->window() == q) {
         if (pb == mainDef) {
            hasMain = true;
         }
         if (pb != pushButton) {
            pb->setDefault(false);
         }
      }
   }
   if (!pushButton && hasMain) {
      mainDef->setDefault(true);
   }
   if (!hasMain) {
      mainDef = pushButton;
   }
}

/*!
  \internal
  This function sets the default default push button to \a pushButton.
  This function is called by QPushButton::setDefault().
*/
void QDialogPrivate::setMainDefault(QPushButton *pushButton)
{
   mainDef = 0;
   setDefault(pushButton);
}

/*!
  \internal
  Hides the default button indicator. Called when non auto-default
  push button get focus.
 */
void QDialogPrivate::hideDefault()
{
   Q_Q(QDialog);
   QList<QPushButton *> list = q->findChildren<QPushButton *>();
   for (int i = 0; i < list.size(); ++i) {
      list.at(i)->setDefault(false);
   }
}

void QDialogPrivate::resetModalitySetByOpen()
{
   Q_Q(QDialog);

   if (resetModalityTo != -1 && !q->testAttribute(Qt::WA_SetWindowModality)) {
      // open() changed the window modality and the user didn't touch it afterwards; restore it
      q->setWindowModality(Qt::WindowModality(resetModalityTo));
      q->setAttribute(Qt::WA_SetWindowModality, wasModalitySet);

#ifdef Q_OS_MAC
      Q_ASSERT(resetModalityTo != Qt::WindowModal);
      q->setParent(q->parentWidget(), Qt::Dialog);
#endif

   }

   resetModalityTo = -1;
}

int QDialog::result() const
{
   Q_D(const QDialog);
   return d->rescode;
}

void QDialog::setResult(int r)
{
   Q_D(QDialog);
   d->rescode = r;
}

void QDialog::open()
{
   Q_D(QDialog);

   Qt::WindowModality modality = windowModality();
   if (modality != Qt::WindowModal) {
      d->resetModalityTo = modality;
      d->wasModalitySet = testAttribute(Qt::WA_SetWindowModality);
      setWindowModality(Qt::WindowModal);
      setAttribute(Qt::WA_SetWindowModality, false);
#ifdef Q_OS_MAC
      setParent(parentWidget(), Qt::Sheet);
#endif
   }

   setResult(0);
   show();
}

/*!
    Shows the dialog as a \l{QDialog#Modal Dialogs}{modal dialog},
    blocking until the user closes it. The function returns a \l
    DialogCode result.

    If the dialog is \l{Qt::ApplicationModal}{application modal}, users cannot
    interact with any other window in the same application until they close
    the dialog. If the dialog is \l{Qt::ApplicationModal}{window modal}, only
    interaction with the parent window is blocked while the dialog is open.
    By default, the dialog is application modal.

    \sa open(), show(), result(), setWindowModality()
*/

int QDialog::exec()
{
   Q_D(QDialog);

   if (d->eventLoop) {
      qWarning("QDialog::exec: Recursive call detected");
      return -1;
   }

   bool deleteOnClose = testAttribute(Qt::WA_DeleteOnClose);
   setAttribute(Qt::WA_DeleteOnClose, false);

   d->resetModalitySetByOpen();

   bool wasShowModal = testAttribute(Qt::WA_ShowModal);
   setAttribute(Qt::WA_ShowModal, true);
   setResult(0);

   show();

#ifdef Q_OS_MAC
   d->mac_nativeDialogModalHelp();
#endif

   QEventLoop eventLoop;
   d->eventLoop = &eventLoop;
   QPointer<QDialog> guard = this;

   (void) eventLoop.exec(QEventLoop::DialogExec);

   if (guard.isNull()) {
      return QDialog::Rejected;
   }
   d->eventLoop = 0;

   setAttribute(Qt::WA_ShowModal, wasShowModal);

   int res = result();
   if (deleteOnClose) {
      delete this;
   }

   return res;
}


/*!
  Closes the dialog and sets its result code to \a r. If this dialog
  is shown with exec(), done() causes the local event loop to finish,
  and exec() to return \a r.

  As with QWidget::close(), done() deletes the dialog if the
  Qt::WA_DeleteOnClose flag is set. If the dialog is the application's
  main widget, the application terminates. If the dialog is the
  last window closed, the QApplication::lastWindowClosed() signal is
  emitted.

  \sa accept(), reject(), QApplication::activeWindow(), QApplication::quit()
*/

void QDialog::done(int r)
{
   Q_D(QDialog);
   hide();
   setResult(r);

   d->close_helper(QWidgetPrivate::CloseNoEvent);
   d->resetModalitySetByOpen();

   emit finished(r);
   if (r == Accepted) {
      emit accepted();
   } else if (r == Rejected) {
      emit rejected();
   }
}

/*!
  Hides the modal dialog and sets the result code to \c Accepted.

  \sa reject() done()
*/

void QDialog::accept()
{
   done(Accepted);
}

/*!
  Hides the modal dialog and sets the result code to \c Rejected.

  \sa accept() done()
*/

void QDialog::reject()
{
   done(Rejected);
}

/*! \reimp */
bool QDialog::eventFilter(QObject *o, QEvent *e)
{
   return QWidget::eventFilter(o, e);
}

/*****************************************************************************
  Event handlers
 *****************************************************************************/

#ifndef QT_NO_CONTEXTMENU
/*! \reimp */
void QDialog::contextMenuEvent(QContextMenuEvent *e)
{
#if defined(QT_NO_WHATSTHIS) || defined(QT_NO_MENU)
   Q_UNUSED(e);
#else
   QWidget *w = childAt(e->pos());
   if (!w) {
      w = rect().contains(e->pos()) ? this : 0;
      if (!w) {
         return;
      }
   }
   while (w && w->whatsThis().size() == 0 && !w->testAttribute(Qt::WA_CustomWhatsThis)) {
      w = w->isWindow() ? 0 : w->parentWidget();
   }
   if (w) {
      QWeakPointer<QMenu> p = new QMenu(this);
      QAction *wt = p.data()->addAction(tr("What's This?"));
      if (p.data()->exec(e->globalPos()) == wt) {
         QHelpEvent e(QEvent::WhatsThis, w->rect().center(),
                      w->mapToGlobal(w->rect().center()));
         QApplication::sendEvent(w, &e);
      }
      delete p.data();
   }
#endif
}
#endif // QT_NO_CONTEXTMENU

/*! \reimp */
void QDialog::keyPressEvent(QKeyEvent *e)
{
   //   Calls reject() if Escape is pressed. Simulates a button
   //   click for the default button if Enter is pressed. Move focus
   //   for the arrow keys. Ignore the rest.
#ifdef Q_OS_MAC
   if (e->modifiers() == Qt::ControlModifier && e->key() == Qt::Key_Period) {
      reject();
   } else
#endif
      if (!e->modifiers() || (e->modifiers() & Qt::KeypadModifier && e->key() == Qt::Key_Enter)) {
         switch (e->key()) {
            case Qt::Key_Enter:
            case Qt::Key_Return: {
               QList<QPushButton *> list = findChildren<QPushButton *>();
               for (int i = 0; i < list.size(); ++i) {
                  QPushButton *pb = list.at(i);
                  if (pb->isDefault() && pb->isVisible()) {
                     if (pb->isEnabled()) {
                        pb->click();
                     }
                     return;
                  }
               }
            }
            break;
            case Qt::Key_Escape:
               reject();
               break;
            default:
               e->ignore();
               return;
         }
      } else {
         e->ignore();
      }
}

/*! \reimp */
void QDialog::closeEvent(QCloseEvent *e)
{
#ifndef QT_NO_WHATSTHIS
   if (isModal() && QWhatsThis::inWhatsThisMode()) {
      QWhatsThis::leaveWhatsThisMode();
   }
#endif
   if (isVisible()) {
      QPointer<QObject> that = this;
      reject();
      if (that && isVisible()) {
         e->ignore();
      }
   } else {
      e->accept();
   }
}

/*****************************************************************************
  Geometry management.
 *****************************************************************************/

/*! \reimp
*/

void QDialog::setVisible(bool visible)
{
   Q_D(QDialog);
   if (visible) {
      if (testAttribute(Qt::WA_WState_ExplicitShowHide) && !testAttribute(Qt::WA_WState_Hidden)) {
         return;
      }

      if (!testAttribute(Qt::WA_Moved)) {
         Qt::WindowStates state = windowState();
         adjustPosition(parentWidget());
         setAttribute(Qt::WA_Moved, false); // not really an explicit position
         if (state != windowState()) {
            setWindowState(state);
         }
      }
      QWidget::setVisible(visible);
      showExtension(d->doShowExtension);
      QWidget *fw = window()->focusWidget();
      if (!fw) {
         fw = this;
      }

      /*
        The following block is to handle a special case, and does not
        really follow propper logic in concern of autoDefault and TAB
        order. However, it's here to ease usage for the users. If a
        dialog has a default QPushButton, and first widget in the TAB
        order also is a QPushButton, then we give focus to the main
        default QPushButton. This simplifies code for the developers,
        and actually catches most cases... If not, then they simply
        have to use [widget*]->setFocus() themselves...
      */
      if (d->mainDef && fw->focusPolicy() == Qt::NoFocus) {
         QWidget *first = fw;
         while ((first = first->nextInFocusChain()) != fw && first->focusPolicy() == Qt::NoFocus)
            ;
         if (first != d->mainDef && qobject_cast<QPushButton *>(first)) {
            d->mainDef->setFocus();
         }
      }
      if (!d->mainDef && isWindow()) {
         QWidget *w = fw;
         while ((w = w->nextInFocusChain()) != fw) {
            QPushButton *pb = qobject_cast<QPushButton *>(w);
            if (pb && pb->autoDefault() && pb->focusPolicy() != Qt::NoFocus) {
               pb->setDefault(true);
               break;
            }
         }
      }
      if (fw && !fw->hasFocus()) {
         QFocusEvent e(QEvent::FocusIn, Qt::TabFocusReason);
         QApplication::sendEvent(fw, &e);
      }

#ifndef QT_NO_ACCESSIBILITY
      QAccessible::updateAccessibility(this, 0, QAccessible::DialogStart);
#endif

   } else {
      if (testAttribute(Qt::WA_WState_ExplicitShowHide) && testAttribute(Qt::WA_WState_Hidden)) {
         return;
      }

#ifndef QT_NO_ACCESSIBILITY
      if (isVisible()) {
         QAccessible::updateAccessibility(this, 0, QAccessible::DialogEnd);
      }
#endif

      // Reimplemented to exit a modal event loop when the dialog is hidden.
      QWidget::setVisible(visible);
      if (d->eventLoop) {
         d->eventLoop->exit();
      }
   }
#ifdef Q_OS_WIN
   if (d->mainDef && isActiveWindow()) {
      BOOL snapToDefault = false;
      if (SystemParametersInfo(SPI_GETSNAPTODEFBUTTON, 0, &snapToDefault, 0)) {
         if (snapToDefault) {
            QCursor::setPos(d->mainDef->mapToGlobal(d->mainDef->rect().center()));
         }
      }
   }
#endif
}

/*!\reimp */
void QDialog::showEvent(QShowEvent *event)
{
   if (!event->spontaneous() && !testAttribute(Qt::WA_Moved)) {
      Qt::WindowStates  state = windowState();
      adjustPosition(parentWidget());
      setAttribute(Qt::WA_Moved, false); // not really an explicit position
      if (state != windowState()) {
         setWindowState(state);
      }
   }
}

/*! \internal */
void QDialog::adjustPosition(QWidget *w)
{
#ifdef Q_WS_X11
   // if the WM advertises that it will place the windows properly for us, let it do it :)
   if (X11->isSupportedByWM(ATOM(_NET_WM_FULL_PLACEMENT))) {
      return;
   }
#endif

   QPoint p(0, 0);
   int extraw = 0, extrah = 0, scrn = 0;
   if (w) {
      w = w->window();
   }
   QRect desk;
   if (w) {
      scrn = QApplication::desktop()->screenNumber(w);
   } else if (QApplication::desktop()->isVirtualDesktop()) {
      scrn = QApplication::desktop()->screenNumber(QCursor::pos());
   } else {
      scrn = QApplication::desktop()->screenNumber(this);
   }
   desk = QApplication::desktop()->availableGeometry(scrn);

   QWidgetList list = QApplication::topLevelWidgets();
   for (int i = 0; (extraw == 0 || extrah == 0) && i < list.size(); ++i) {
      QWidget *current = list.at(i);
      if (current->isVisible()) {
         int framew = current->geometry().x() - current->x();
         int frameh = current->geometry().y() - current->y();

         extraw = qMax(extraw, framew);
         extrah = qMax(extrah, frameh);
      }
   }

   // sanity check for decoration frames. With embedding, we
   // might get extraordinary values
   if (extraw == 0 || extrah == 0 || extraw >= 10 || extrah >= 40) {
      extrah = 40;
      extraw = 10;
   }


   if (w) {
      // Use mapToGlobal rather than geometry() in case w might
      // be embedded in another application
      QPoint pp = w->mapToGlobal(QPoint(0, 0));
      p = QPoint(pp.x() + w->width() / 2,
                 pp.y() + w->height() / 2);
   } else {
      // p = middle of the desktop
      p = QPoint(desk.x() + desk.width() / 2, desk.y() + desk.height() / 2);
   }

   // p = origin of this
   p = QPoint(p.x() - width() / 2 - extraw,
              p.y() - height() / 2 - extrah);


   if (p.x() + extraw + width() > desk.x() + desk.width()) {
      p.setX(desk.x() + desk.width() - width() - extraw);
   }
   if (p.x() < desk.x()) {
      p.setX(desk.x());
   }

   if (p.y() + extrah + height() > desk.y() + desk.height()) {
      p.setY(desk.y() + desk.height() - height() - extrah);
   }
   if (p.y() < desk.y()) {
      p.setY(desk.y());
   }

   move(p);
}

/*!
    \obsolete

    If \a orientation is Qt::Horizontal, the extension will be displayed
    to the right of the dialog's main area. If \a orientation is
    Qt::Vertical, the extension will be displayed below the dialog's main
    area.

    Instead of using this functionality, we recommend that you simply call
    show() or hide() on the part of the dialog that you want to use as an
    extension. See the \l{Extension Example} for details.

    \sa setExtension()
*/
void QDialog::setOrientation(Qt::Orientation orientation)
{
   Q_D(QDialog);
   d->orientation = orientation;
}

/*!
    \obsolete

    Returns the dialog's extension orientation.

    Instead of using this functionality, we recommend that you simply call
    show() or hide() on the part of the dialog that you want to use as an
    extension. See the \l{Extension Example} for details.

    \sa extension()
*/
Qt::Orientation QDialog::orientation() const
{
   Q_D(const QDialog);
   return d->orientation;
}

/*!
    \obsolete

    Sets the widget, \a extension, to be the dialog's extension,
    deleting any previous extension. The dialog takes ownership of the
    extension. Note that if 0 is passed any existing extension will be
    deleted. This function must only be called while the dialog is hidden.

    Instead of using this functionality, we recommend that you simply call
    show() or hide() on the part of the dialog that you want to use as an
    extension. See the \l{Extension Example} for details.

    \sa showExtension(), setOrientation()
*/
void QDialog::setExtension(QWidget *extension)
{
   Q_D(QDialog);
   delete d->extension;
   d->extension = extension;

   if (!extension) {
      return;
   }

   if (extension->parentWidget() != this) {
      extension->setParent(this);
   }
   extension->hide();
}

/*!
    \obsolete

    Returns the dialog's extension or 0 if no extension has been
    defined.

    Instead of using this functionality, we recommend that you simply call
    show() or hide() on the part of the dialog that you want to use as an
    extension. See the \l{Extension Example} for details.

    \sa showExtension(), setOrientation()
*/
QWidget *QDialog::extension() const
{
   Q_D(const QDialog);
   return d->extension;
}


/*!
    \obsolete

    If \a showIt is true, the dialog's extension is shown; otherwise the
    extension is hidden.

    Instead of using this functionality, we recommend that you simply call
    show() or hide() on the part of the dialog that you want to use as an
    extension. See the \l{Extension Example} for details.

    \sa show(), setExtension(), setOrientation()
*/
void QDialog::showExtension(bool showIt)
{
   Q_D(QDialog);
   d->doShowExtension = showIt;
   if (!d->extension) {
      return;
   }
   if (!testAttribute(Qt::WA_WState_Visible)) {
      return;
   }
   if (d->extension->isVisible() == showIt) {
      return;
   }

   if (showIt) {
      d->size = size();
      d->min = minimumSize();
      d->max = maximumSize();
      if (layout()) {
         layout()->setEnabled(false);
      }
      QSize s(d->extension->sizeHint()
              .expandedTo(d->extension->minimumSize())
              .boundedTo(d->extension->maximumSize()));
      if (d->orientation == Qt::Horizontal) {
         int h = qMax(height(), s.height());
         d->extension->setGeometry(width(), 0, s.width(), h);
         setFixedSize(width() + s.width(), h);
      } else {
         int w = qMax(width(), s.width());
         d->extension->setGeometry(0, height(), w, s.height());
         setFixedSize(w, height() + s.height());
      }
      d->extension->show();
#ifndef QT_NO_SIZEGRIP
      const bool sizeGripEnabled = isSizeGripEnabled();
      setSizeGripEnabled(false);
      d->sizeGripEnabled = sizeGripEnabled;
#endif
   } else {
      d->extension->hide();
      // workaround for CDE window manager that won't shrink with (-1,-1)
      setMinimumSize(d->min.expandedTo(QSize(1, 1)));
      setMaximumSize(d->max);
      resize(d->size);
      if (layout()) {
         layout()->setEnabled(true);
      }
#ifndef QT_NO_SIZEGRIP
      setSizeGripEnabled(d->sizeGripEnabled);
#endif
   }
}


/*! \reimp */
QSize QDialog::sizeHint() const
{
   Q_D(const QDialog);

   if (d->extension) {

      if (d->orientation == Qt::Horizontal) {
         return QSize(QWidget::sizeHint().width(), qMax(QWidget::sizeHint().height(), d->extension->sizeHint().height()));
      } else {
         return QSize(qMax(QWidget::sizeHint().width(), d->extension->sizeHint().width()), QWidget::sizeHint().height());
      }
   }

   return QWidget::sizeHint();
}


/*! \reimp */
QSize QDialog::minimumSizeHint() const
{
   Q_D(const QDialog);
   if (d->extension) {
      if (d->orientation == Qt::Horizontal)
         return QSize(QWidget::minimumSizeHint().width(),
                      qMax(QWidget::minimumSizeHint().height(), d->extension->minimumSizeHint().height()));
      else
         return QSize(qMax(QWidget::minimumSizeHint().width(), d->extension->minimumSizeHint().width()),
                      QWidget::minimumSizeHint().height());
   }

   return QWidget::minimumSizeHint();
}

/*!
    \property QDialog::modal
    \brief whether show() should pop up the dialog as modal or modeless

    By default, this property is false and show() pops up the dialog
    as modeless. Setting his property to true is equivalent to setting
    QWidget::windowModality to Qt::ApplicationModal.

    exec() ignores the value of this property and always pops up the
    dialog as modal.

    \sa QWidget::windowModality, show(), exec()
*/

void QDialog::setModal(bool modal)
{
   setAttribute(Qt::WA_ShowModal, modal);
}


bool QDialog::isSizeGripEnabled() const
{
#ifndef QT_NO_SIZEGRIP
   Q_D(const QDialog);
   return !!d->resizer;
#else
   return false;
#endif
}


void QDialog::setSizeGripEnabled(bool enabled)
{
#ifdef QT_NO_SIZEGRIP
   Q_UNUSED(enabled);
#else
   Q_D(QDialog);
#ifndef QT_NO_SIZEGRIP
   d->sizeGripEnabled = enabled;
   if (enabled && d->doShowExtension) {
      return;
   }
#endif
   if (!enabled != !d->resizer) {
      if (enabled) {
         d->resizer = new QSizeGrip(this);
         // adjustSize() processes all events, which is suboptimal
         d->resizer->resize(d->resizer->sizeHint());
         if (isRightToLeft()) {
            d->resizer->move(rect().bottomLeft() - d->resizer->rect().bottomLeft());
         } else {
            d->resizer->move(rect().bottomRight() - d->resizer->rect().bottomRight());
         }
         d->resizer->raise();
         d->resizer->show();
      } else {
         delete d->resizer;
         d->resizer = 0;
      }
   }
#endif //QT_NO_SIZEGRIP
}



/*! \reimp */
void QDialog::resizeEvent(QResizeEvent *)
{
#ifndef QT_NO_SIZEGRIP
   Q_D(QDialog);
   if (d->resizer) {
      if (isRightToLeft()) {
         d->resizer->move(rect().bottomLeft() - d->resizer->rect().bottomLeft());
      } else {
         d->resizer->move(rect().bottomRight() - d->resizer->rect().bottomRight());
      }
      d->resizer->raise();
   }
#endif
}

/*! \fn void QDialog::finished(int result)
    \since 4.1

    This signal is emitted when the dialog's \a result code has been
    set, either by the user or by calling done(), accept(), or
    reject().

    Note that this signal is \e not emitted when hiding the dialog
    with hide() or setVisible(false). This includes deleting the
    dialog while it is visible.

    \sa accepted(), rejected()
*/

/*! \fn void QDialog::accepted()
    \since 4.1

    This signal is emitted when the dialog has been accepted either by
    the user or by calling accept() or done() with the
    QDialog::Accepted argument.

    Note that this signal is \e not emitted when hiding the dialog
    with hide() or setVisible(false). This includes deleting the
    dialog while it is visible.

    \sa finished(), rejected()
*/

/*! \fn void QDialog::rejected()
    \since 4.1

    This signal is emitted when the dialog has been rejected either by
    the user or by calling reject() or done() with the
    QDialog::Rejected argument.

    Note that this signal is \e not emitted when hiding the dialog
    with hide() or setVisible(false). This includes deleting the
    dialog while it is visible.

    \sa finished(), accepted()
*/

QT_END_NAMESPACE

