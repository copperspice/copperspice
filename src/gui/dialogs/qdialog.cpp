/***********************************************************************
*
* Copyright (c) 2012-2023 Barbara Geller
* Copyright (c) 2012-2023 Ansel Sermersheim
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

#include <qdialog.h>

#include <qapplication.h>
#include <qcolordialog.h>
#include <qcursor.h>
#include <qdesktopwidget.h>
#include <qerrormessage.h>
#include <qfontdialog.h>
#include <qfiledialog.h>
#include <qevent.h>
#include <qlayout.h>
#include <qplatform_theme.h>
#include <qpushbutton.h>
#include <qmenu.h>
#include <qmessagebox.h>
#include <qsizegrip.h>
#include <qwhatsthis.h>

#include <qguiapplication_p.h>
#include <qdialog_p.h>

#ifndef QT_NO_ACCESSIBILITY
#include <qaccessible.h>
#endif

static inline int themeDialogType(const QDialog *dialog)
{
#ifndef QT_NO_FILEDIALOG
   if (qobject_cast<const QFileDialog *>(dialog)) {
      return QPlatformTheme::FileDialog;
   }
#endif
#ifndef QT_NO_COLORDIALOG
   if (qobject_cast<const QColorDialog *>(dialog)) {
      return QPlatformTheme::ColorDialog;
   }
#endif
#ifndef QT_NO_FONTDIALOG
   if (qobject_cast<const QFontDialog *>(dialog)) {
      return QPlatformTheme::FontDialog;
   }
#endif
#ifndef QT_NO_MESSAGEBOX
   if (qobject_cast<const QMessageBox *>(dialog)) {
      return QPlatformTheme::MessageDialog;
   }
#endif
#ifndef QT_NO_ERRORMESSAGE
   if (qobject_cast<const QErrorMessage *>(dialog)) {
      return QPlatformTheme::MessageDialog;
   }
#endif
   return -1;
}

QPlatformDialogHelper *QDialogPrivate::platformHelper() const
{
   // Delayed creation of the platform, ensuring that
   // that qobject_cast<> on the dialog works in the plugin.

   if (! m_platformHelperCreated) {
      m_platformHelperCreated = true;
      QDialogPrivate *ncThis = const_cast<QDialogPrivate *>(this);
      QDialog *dialog = ncThis->q_func();
      const int type = themeDialogType(dialog);

      if (type >= 0) {
         m_platformHelper = QGuiApplicationPrivate::platformTheme()
            ->createPlatformDialogHelper(static_cast<QPlatformTheme::DialogType>(type));

         if (m_platformHelper) {
            QObject::connect(m_platformHelper, &QPlatformDialogHelper::accept, dialog, &QDialog::accept);
            QObject::connect(m_platformHelper, &QPlatformDialogHelper::reject, dialog, &QDialog::reject);

            ncThis->initHelper(m_platformHelper);
         }
      }
   }
   return m_platformHelper;
}

bool QDialogPrivate::canBeNativeDialog() const
{
   QDialogPrivate *ncThis = const_cast<QDialogPrivate *>(this);
   QDialog *dialog = ncThis->q_func();
   const int type = themeDialogType(dialog);

   if (type >= 0) {
      return QGuiApplicationPrivate::platformTheme()->usePlatformNativeDialog(static_cast<QPlatformTheme::DialogType>(type));
   }

   return false;
}

QWindow *QDialogPrivate::parentWindow() const
{
   if (const QWidget *parent = q_func()->nativeParentWidget()) {
      return parent->windowHandle();
   }
   return nullptr;
}

bool QDialogPrivate::setNativeDialogVisible(bool visible)
{
   if (QPlatformDialogHelper *helper = platformHelper()) {
      if (visible) {
         Q_Q(QDialog);
         helperPrepareShow(helper);
         nativeDialogInUse = helper->show(q->windowFlags(), q->windowModality(), parentWindow());
      } else if (nativeDialogInUse) {
         helper->hide();
      }
   }
   return nativeDialogInUse;
}

QVariant QDialogPrivate::styleHint(QPlatformDialogHelper::StyleHint hint) const
{
   if (const QPlatformDialogHelper *helper = platformHelper()) {
      return helper->styleHint(hint);
   }
   return QPlatformDialogHelper::defaultStyleHint(hint);
}

void QDialogPrivate::deletePlatformHelper()
{
   delete m_platformHelper;
   m_platformHelper = nullptr;
   m_platformHelperCreated = false;
   nativeDialogInUse = false;
}

QDialog::QDialog(QWidget *parent, Qt::WindowFlags flags)
   : QWidget(*new QDialogPrivate, parent, flags | ((flags & Qt::WindowType_Mask) == 0 ? Qt::Dialog : Qt::WindowType(0)))
{
}

// internal
QDialog::QDialog(QDialogPrivate &dd, QWidget *parent, Qt::WindowFlags flags)
   : QWidget(dd, parent, flags | ((flags & Qt::WindowType_Mask) == 0 ? Qt::Dialog : Qt::WindowType(0)))
{
}

QDialog::~QDialog()
{
   try {
      // must call hide() as the overridden hide() will not be called in ~QWidget
      hide();

   } catch (...) {
      // do nothing
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
   mainDef = nullptr;
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

#ifdef Q_OS_DARWIN
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

#ifdef Q_OS_DARWIN
      setParent(parentWidget(), Qt::Sheet);
#endif
   }

   setResult(0);
   show();
}

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

   QPointer<QDialog> guard = this;
   if (d->nativeDialogInUse) {
      d->platformHelper()->exec();
   } else {
      QEventLoop eventLoop;
      d->eventLoop = &eventLoop;

      (void) eventLoop.exec(QEventLoop::DialogExec);
   }

   if (guard.isNull()) {
      return QDialog::Rejected;
   }
   d->eventLoop = nullptr;

   setAttribute(Qt::WA_ShowModal, wasShowModal);

   int res = result();

   if (d->nativeDialogInUse) {
      d->helperDone(static_cast<QDialog::DialogCode>(res), d->platformHelper());
   }

   if (deleteOnClose) {
      delete this;
   }

   return res;
}

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

void QDialog::accept()
{
   done(Accepted);
}

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
   (void) e;

#else
   QWidget *w = childAt(e->pos());
   if (!w) {
      w = rect().contains(e->pos()) ? this : nullptr;
      if (!w) {
         return;
      }
   }
   while (w && w->whatsThis().size() == 0 && !w->testAttribute(Qt::WA_CustomWhatsThis)) {
      w = w->isWindow() ? nullptr : w->parentWidget();
   }

   if (w) {
      QPointer<QMenu> p = new QMenu(this);
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

   if (e->matches(QKeySequence::Cancel)) {
      reject();

   } else

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

void QDialog::setVisible(bool visible)
{
   Q_D(QDialog);

   if (! testAttribute(Qt::WA_DontShowOnScreen) && d->canBeNativeDialog() && d->setNativeDialogVisible(visible)) {
      return;
   }

   if (visible) {
      if (testAttribute(Qt::WA_WState_ExplicitShowHide) && ! testAttribute(Qt::WA_WState_Hidden)) {
         return;
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
      QAccessibleEvent event(this, QAccessible::DialogStart);
      QAccessible::updateAccessibility(&event);
#endif

   } else {
      if (testAttribute(Qt::WA_WState_ExplicitShowHide) && testAttribute(Qt::WA_WState_Hidden)) {
         return;
      }

#ifndef QT_NO_ACCESSIBILITY
      if (isVisible()) {
         QAccessibleEvent event(this, QAccessible::DialogEnd);
         QAccessible::updateAccessibility(&event);
      }
#endif

      // Reimplemented to exit a modal event loop when the dialog is hidden.
      QWidget::setVisible(visible);

      if (d->eventLoop) {
         d->eventLoop->exit();
      }

   }

   const QPlatformTheme *theme = QGuiApplicationPrivate::platformTheme();

   if (d->mainDef && isActiveWindow()
      && theme->themeHint(QPlatformTheme::DialogSnapToDefaultButton).toBool()) {
      QCursor::setPos(d->mainDef->mapToGlobal(d->mainDef->rect().center()));
   }


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
   if (const QPlatformTheme *theme = QGuiApplicationPrivate::platformTheme())
      if (theme->themeHint(QPlatformTheme::WindowAutoPlacement).toBool()) {
         return;
      }


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
      // Use pos() if the widget is embedded into a native window
      QPoint pp;

      if (w->windowHandle() && w->windowHandle()->property("_q_embedded_native_parent_handle").value<WId>()) {
         pp = w->pos();
      } else {
         pp = w->mapToGlobal(QPoint(0, 0));
      }

      p = QPoint(pp.x() + w->width() / 2, pp.y() + w->height() / 2);

   } else {
      // p = middle of the desktop
      p = QPoint(desk.x() + desk.width() / 2, desk.y() + desk.height() / 2);
   }

   // p = origin of this
   p = QPoint(p.x() - width() / 2 - extraw, p.y() - height() / 2 - extrah);

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

   if (scrn >= 0) {
      if (QWindow *window = windowHandle()) {
         window->setScreen(QGuiApplication::screens().at(scrn));
      }
   }

   move(p);
}


void QDialog::setOrientation(Qt::Orientation orientation)
{
   Q_D(QDialog);
   d->orientation = orientation;
}

Qt::Orientation QDialog::orientation() const
{
   Q_D(const QDialog);
   return d->orientation;
}

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


QWidget *QDialog::extension() const
{
   Q_D(const QDialog);
   return d->extension;
}


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

      if (d->orientation == Qt::Horizontal)
         return QSize(QWidget::sizeHint().width(),
               qMax(QWidget::sizeHint().height(), d->extension->sizeHint().height()));
      else
         return QSize(qMax(QWidget::sizeHint().width(), d->extension->sizeHint().width()),
               QWidget::sizeHint().height());
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
         d->resizer = nullptr;
      }
   }

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


