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

#include <qdialogbuttonbox.h>

#include <qaction.h>
#include <qhash.h>
#include <qpushbutton.h>
#include <qstyle.h>
#include <qlayout.h>
#include <qdialog.h>
#include <qapplication.h>
#include <qplatform_dialoghelper.h>
#include <qplatform_theme.h>

#include <qwidget_p.h>
#include <qguiapplication_p.h>

class QDialogButtonBoxPrivate : public QWidgetPrivate
{
   Q_DECLARE_PUBLIC(QDialogButtonBox)

 public:
   QDialogButtonBoxPrivate(Qt::Orientation orient);

   QList<QAbstractButton *> buttonLists[QDialogButtonBox::NRoles];
   QHash<QPushButton *, QDialogButtonBox::StandardButton> standardButtonHash;

   Qt::Orientation orientation;
   QDialogButtonBox::ButtonLayout layoutPolicy;
   QBoxLayout *buttonLayout;
   bool internalRemove;
   bool center;

   void createStandardButtons(QDialogButtonBox::StandardButtons buttons);

   void layoutButtons();
   void initLayout();
   void resetLayout();

   QPushButton *createButton(QDialogButtonBox::StandardButton button, bool doLayout = true);
   void addButton(QAbstractButton *button, QDialogButtonBox::ButtonRole role, bool doLayout = true);
   void addButtonsToLayout(const QList<QAbstractButton *> &buttonList, bool reverse);
   void retranslateStrings();

   void _q_handleButtonDestroyed();
   void _q_handleButtonClicked();
};

QDialogButtonBoxPrivate::QDialogButtonBoxPrivate(Qt::Orientation orient)
   : orientation(orient), buttonLayout(nullptr), internalRemove(false), center(false)
{
}

void QDialogButtonBoxPrivate::initLayout()
{
   Q_Q(QDialogButtonBox);
   layoutPolicy = QDialogButtonBox::ButtonLayout(q->style()->styleHint(QStyle::SH_DialogButtonLayout, nullptr, q));

   bool createNewLayout = (buttonLayout == nullptr)
            || (orientation == Qt::Horizontal && qobject_cast<QVBoxLayout *>(buttonLayout) != nullptr)
            || (orientation == Qt::Vertical && qobject_cast<QHBoxLayout *>(buttonLayout)   != nullptr);

   if (createNewLayout) {
      delete buttonLayout;

      if (orientation == Qt::Horizontal) {
         buttonLayout = new QHBoxLayout(q);
      } else {
         buttonLayout = new QVBoxLayout(q);
      }
   }

   int left, top, right, bottom;
   setLayoutItemMargins(QStyle::SE_PushButtonLayoutItem);
   getLayoutItemMargins(&left, &top, &right, &bottom);
   buttonLayout->setContentsMargins(-left, -top, -right, -bottom);

   if (! q->testAttribute(Qt::WA_WState_OwnSizePolicy)) {
      QSizePolicy sp(QSizePolicy::Expanding, QSizePolicy::Fixed, QSizePolicy::ButtonBox);
      if (orientation == Qt::Vertical) {
         sp.transpose();
      }

      q->setSizePolicy(sp);
      q->setAttribute(Qt::WA_WState_OwnSizePolicy, false);
   }
}

void QDialogButtonBoxPrivate::resetLayout()
{
   // delete buttonLayout
   initLayout();
   layoutButtons();
}

void QDialogButtonBoxPrivate::addButtonsToLayout(const QList<QAbstractButton *> &buttonList,
   bool reverse)
{
   int start = reverse ? buttonList.count() - 1 : 0;
   int end   = reverse ? -1 : buttonList.count();
   int step  = reverse ? -1 : 1;

   for (int i = start; i != end; i += step) {
      QAbstractButton *button = buttonList.at(i);
      buttonLayout->addWidget(button);
      button->show();
   }
}

void QDialogButtonBoxPrivate::layoutButtons()
{
   Q_Q(QDialogButtonBox);
   const int MacGap = 36 - 8;    // 8 is the default gap between a widget and a spacer item

   for (int i = buttonLayout->count() - 1; i >= 0; --i) {
      QLayoutItem *item = buttonLayout->takeAt(i);

      if (QWidget *widget = item->widget()) {
         widget->hide();
      }

      delete item;
   }

   int tmpPolicy = layoutPolicy;

   static constexpr const int MaxRoleCount = 5;

   static const int ModalRoles[MaxRoleCount] = {
      QPlatformDialogHelper::AcceptRole,
      QPlatformDialogHelper::RejectRole,
      QPlatformDialogHelper::DestructiveRole,
      QPlatformDialogHelper::YesRole,
      QPlatformDialogHelper::NoRole
   };

   if (tmpPolicy == QDialogButtonBox::MacLayout) {
      bool hasModalButton = false;

      for (int i = 0; i < MaxRoleCount; ++i) {
         if (! buttonLists[ModalRoles[i]].isEmpty()) {
            hasModalButton = true;
            break;
         }
      }

      if (! hasModalButton) {
         tmpPolicy = 4;   // Mac modeless
      }
   }

   const int *currentLayout = QPlatformDialogHelper::buttonLayout(
         orientation, static_cast<QPlatformDialogHelper::ButtonLayout>(tmpPolicy));

   if (center) {
      buttonLayout->addStretch();
   }

   const QList<QAbstractButton *> &acceptRoleList = buttonLists[QPlatformDialogHelper::AcceptRole];

   while (*currentLayout != QPlatformDialogHelper::EOL) {
      int role = (*currentLayout & ~QPlatformDialogHelper::Reverse);
      bool reverse = (*currentLayout & QPlatformDialogHelper::Reverse);

      switch (role) {
         case QPlatformDialogHelper::Stretch:
            if (! center) {
               buttonLayout->addStretch();
            }

            break;

         case QPlatformDialogHelper::AcceptRole: {
            if (acceptRoleList.isEmpty()) {
               break;
            }

            // Only the first one
            QAbstractButton *button = acceptRoleList.first();

            buttonLayout->addWidget(button);
            button->show();
         }
         break;

         case QPlatformDialogHelper::AlternateRole: {
            if (acceptRoleList.size() < 2) {
               break;
            }

            QList<QAbstractButton *> list = acceptRoleList;
            list.removeFirst();
            addButtonsToLayout(list, reverse);
         }
         break;

         case QPlatformDialogHelper::DestructiveRole: {
            const QList<QAbstractButton *> &list = buttonLists[role];

            /*
                Mac: Insert a gap on the left of the destructive
                buttons to ensure that they don't get too close to
                the help and action buttons (but only if there are
                some buttons to the left of the destructive buttons
                (and the stretch, whence buttonLayout->count() > 1
                and not 0)).
            */
            if (tmpPolicy == QDialogButtonBox::MacLayout
               && !list.isEmpty() && buttonLayout->count() > 1) {
               buttonLayout->addSpacing(MacGap);
            }

            addButtonsToLayout(list, reverse);

            // Insert a gap between the destructive buttons and the
            // accept and reject buttons.

            if (tmpPolicy == QDialogButtonBox::MacLayout && !list.isEmpty()) {
               buttonLayout->addSpacing(MacGap);
            }
         }
         break;

         case QPlatformDialogHelper::RejectRole:
         case QPlatformDialogHelper::ActionRole:
         case QPlatformDialogHelper::HelpRole:
         case QPlatformDialogHelper::YesRole:
         case QPlatformDialogHelper::NoRole:
         case QPlatformDialogHelper::ApplyRole:
         case QPlatformDialogHelper::ResetRole:
            addButtonsToLayout(buttonLists[role], reverse);
      }

      ++currentLayout;
   }

   QWidget *lastWidget = nullptr;
   q->setFocusProxy(nullptr);

   for (int i = 0; i < buttonLayout->count(); ++i) {
      QLayoutItem *item = buttonLayout->itemAt(i);

      if (QWidget *widget = item->widget()) {
         if (lastWidget) {
            QWidget::setTabOrder(lastWidget, widget);
         } else {
            q->setFocusProxy(widget);
         }

         lastWidget = widget;
      }
   }

   if (center) {
      buttonLayout->addStretch();
   }
}

QPushButton *QDialogButtonBoxPrivate::createButton(QDialogButtonBox::StandardButton sbutton, bool doLayout)
{
   Q_Q(QDialogButtonBox);

   int icon = 0;

   switch (sbutton) {
      case QDialogButtonBox::Ok:
         icon = QStyle::SP_DialogOkButton;
         break;

      case QDialogButtonBox::Save:
         icon = QStyle::SP_DialogSaveButton;
         break;

      case QDialogButtonBox::Open:
         icon = QStyle::SP_DialogOpenButton;
         break;

      case QDialogButtonBox::Cancel:
         icon = QStyle::SP_DialogCancelButton;
         break;

      case QDialogButtonBox::Close:
         icon = QStyle::SP_DialogCloseButton;
         break;

      case QDialogButtonBox::Apply:
         icon = QStyle::SP_DialogApplyButton;
         break;

      case QDialogButtonBox::Reset:
         icon = QStyle::SP_DialogResetButton;
         break;

      case QDialogButtonBox::Help:
         icon = QStyle::SP_DialogHelpButton;
         break;

      case QDialogButtonBox::Discard:
         icon = QStyle::SP_DialogDiscardButton;
         break;

      case QDialogButtonBox::Yes:
         icon = QStyle::SP_DialogYesButton;
         break;

      case QDialogButtonBox::No:
         icon = QStyle::SP_DialogNoButton;
         break;

      case QDialogButtonBox::YesToAll:
      case QDialogButtonBox::NoToAll:
      case QDialogButtonBox::SaveAll:
      case QDialogButtonBox::Abort:
      case QDialogButtonBox::Retry:
      case QDialogButtonBox::Ignore:
      case QDialogButtonBox::RestoreDefaults:
         break;

      case QDialogButtonBox::NoButton:
         return nullptr;
   }

   QPushButton *button = new QPushButton(QGuiApplicationPrivate::platformTheme()->standardButtonText(sbutton), q);
   QStyle *style = q->style();

   if (style->styleHint(QStyle::SH_DialogButtonBox_ButtonsHaveIcons, nullptr, q) && icon != 0) {
      button->setIcon(style->standardIcon(QStyle::StandardPixmap(icon), nullptr, q));
   }

   if (style != QApplication::style()) {
      // Propagate style
      button->setStyle(style);
   }

   standardButtonHash.insert(button, sbutton);
   QPlatformDialogHelper::ButtonRole role =
         QPlatformDialogHelper::buttonRole(static_cast<QPlatformDialogHelper::StandardButton>(sbutton));

   if (role != QPlatformDialogHelper::InvalidRole) {
      addButton(button, static_cast<QDialogButtonBox::ButtonRole>(role), doLayout);
   } else {
      qWarning("QDialogButtonBox::createButton() Invalid ButtonRole, button will not be added");
   }

   return button;
}

void QDialogButtonBoxPrivate::addButton(QAbstractButton *button, QDialogButtonBox::ButtonRole role, bool doLayout)
{
   Q_Q(QDialogButtonBox);

   QObject::connect(button, &QAbstractButton::clicked,   q, &QDialogButtonBox::_q_handleButtonClicked);
   QObject::connect(button, &QAbstractButton::destroyed, q, &QDialogButtonBox::_q_handleButtonDestroyed);
   buttonLists[role].append(button);

   if (doLayout) {
      layoutButtons();
   }
}

void QDialogButtonBoxPrivate::createStandardButtons(QDialogButtonBox::StandardButtons buttons)
{
   uint i = QDialogButtonBox::FirstButton;

   while (i <= QDialogButtonBox::LastButton) {
      if (i & buttons) {
         createButton(QDialogButtonBox::StandardButton(i), false);
      }

      i = i << 1;
   }

   layoutButtons();
}

void QDialogButtonBoxPrivate::retranslateStrings()
{
   QHash<QPushButton *, QDialogButtonBox::StandardButton>::iterator it = standardButtonHash.begin();

   while (it != standardButtonHash.end()) {

      const QString text = QGuiApplicationPrivate::platformTheme()->standardButtonText(it.value());
      if (! text.isEmpty()) {
         it.key()->setText(text);
      }

      ++it;
   }
}

QDialogButtonBox::QDialogButtonBox(QWidget *parent)
   : QWidget(*new QDialogButtonBoxPrivate(Qt::Horizontal), parent, Qt::EmptyFlag)
{
   d_func()->initLayout();
}

QDialogButtonBox::QDialogButtonBox(Qt::Orientation orientation, QWidget *parent)
   : QWidget(*new QDialogButtonBoxPrivate(orientation), parent, Qt::EmptyFlag)
{
   d_func()->initLayout();
}

QDialogButtonBox::QDialogButtonBox(StandardButtons buttons, QWidget *parent)
   : QWidget(*new QDialogButtonBoxPrivate(Qt::Horizontal), parent, Qt::EmptyFlag)
{
   d_func()->initLayout();
   d_func()->createStandardButtons(buttons);
}

QDialogButtonBox::QDialogButtonBox(StandardButtons buttons, Qt::Orientation orientation, QWidget *parent)
   : QWidget(*new QDialogButtonBoxPrivate(orientation), parent, Qt::EmptyFlag)
{
   d_func()->initLayout();
   d_func()->createStandardButtons(buttons);
}

QDialogButtonBox::~QDialogButtonBox()
{
}

Qt::Orientation QDialogButtonBox::orientation() const
{
   return d_func()->orientation;
}

void QDialogButtonBox::setOrientation(Qt::Orientation orientation)
{
   Q_D(QDialogButtonBox);
   if (orientation == d->orientation) {
      return;
   }

   d->orientation = orientation;
   d->resetLayout();
}

void QDialogButtonBox::clear()
{
   Q_D(QDialogButtonBox);

   // Remove the created standard buttons, they should be in the other lists, which will do the deletion
   d->standardButtonHash.clear();

   for (int i = 0; i < NRoles; ++i) {
      QList<QAbstractButton *> &list = d->buttonLists[i];

      while (list.count()) {
         QAbstractButton *button = list.takeAt(0);
         QObject::disconnect(button, &QAbstractButton::destroyed, this, &QDialogButtonBox::_q_handleButtonDestroyed);
         delete button;
      }
   }
}

QList<QAbstractButton *> QDialogButtonBox::buttons() const
{
   Q_D(const QDialogButtonBox);
   QList<QAbstractButton *> finalList;

   for (int i = 0; i < NRoles; ++i) {
      const QList<QAbstractButton *> &list = d->buttonLists[i];

      for (int j = 0; j < list.count(); ++j) {
         finalList.append(list.at(j));
      }
   }

   return finalList;
}

QDialogButtonBox::ButtonRole QDialogButtonBox::buttonRole(QAbstractButton *button) const
{
   Q_D(const QDialogButtonBox);

   for (int i = 0; i < NRoles; ++i) {
      const QList<QAbstractButton *> &list = d->buttonLists[i];

      for (int j = 0; j < list.count(); ++j) {
         if (list.at(j) == button) {
            return ButtonRole(i);
         }
      }
   }

   return InvalidRole;
}

void QDialogButtonBox::removeButton(QAbstractButton *button)
{
   Q_D(QDialogButtonBox);

   if (! button) {
      return;
   }

   // Remove it from the standard button hash first and then from the roles
   if (QPushButton *pushButton = qobject_cast<QPushButton *>(button)) {
      d->standardButtonHash.remove(pushButton);
   }

   for (int i = 0; i < NRoles; ++i) {
      QList<QAbstractButton *> &list = d->buttonLists[i];

      for (int j = 0; j < list.count(); ++j) {
         if (list.at(j) == button) {
            list.takeAt(j);

            if (! d->internalRemove) {
               disconnect(button, &QAbstractButton::clicked,   this, &QDialogButtonBox::_q_handleButtonClicked);
               disconnect(button, &QAbstractButton::destroyed, this, &QDialogButtonBox::_q_handleButtonDestroyed);
            }
            break;
         }
      }
   }

   if (! d->internalRemove) {
      button->setParent(nullptr);
   }
}

void QDialogButtonBox::addButton(QAbstractButton *button, ButtonRole role)
{
   Q_D(QDialogButtonBox);

   if (role <= InvalidRole || role >= NRoles) {
      qWarning("QDialogButtonBox::addButton() Invalid ButtonRole, button will not be added");
      return;
   }

   removeButton(button);
   button->setParent(this);
   d->addButton(button, role);
}

QPushButton *QDialogButtonBox::addButton(const QString &text, ButtonRole role)
{
   Q_D(QDialogButtonBox);

   if (role <= InvalidRole || role >= NRoles) {
      qWarning("QDialogButtonBox::addButton() Invalid ButtonRole, button will not be added");
      return nullptr;
   }

   QPushButton *button = new QPushButton(text, this);
   d->addButton(button, role);

   return button;
}

QPushButton *QDialogButtonBox::addButton(StandardButton button)
{
   Q_D(QDialogButtonBox);
   return d->createButton(button);
}

void QDialogButtonBox::setStandardButtons(StandardButtons buttons)
{
   Q_D(QDialogButtonBox);

   // clear the old standard buttons, then recreate them
   qDeleteAll(d->standardButtonHash.keys());
   d->standardButtonHash.clear();

   d->createStandardButtons(buttons);
}

QDialogButtonBox::StandardButtons QDialogButtonBox::standardButtons() const
{
   Q_D(const QDialogButtonBox);
   StandardButtons standardButtons = NoButton;

   for (auto item : d->standardButtonHash) {
      standardButtons |= item;
   }

   return standardButtons;
}

QPushButton *QDialogButtonBox::button(StandardButton which) const
{
   Q_D(const QDialogButtonBox);
   return d->standardButtonHash.key(which);
}

QDialogButtonBox::StandardButton QDialogButtonBox::standardButton(QAbstractButton *button) const
{
   Q_D(const QDialogButtonBox);
   return d->standardButtonHash.value(static_cast<QPushButton *>(button));
}

void QDialogButtonBoxPrivate::_q_handleButtonClicked()
{
   Q_Q(QDialogButtonBox);

   if (QAbstractButton *button = dynamic_cast<QAbstractButton *>(q->sender())) {

      const QDialogButtonBox::ButtonRole buttonRole = q->buttonRole(button);

      QPointer<QDialogButtonBox> guard(q);
      emit q->clicked(button);

      if (! guard) {
         return;
      }

      switch (buttonRole) {
         case QDialogButtonBox::AcceptRole:
         case QDialogButtonBox::YesRole:
            emit q->accepted();
            break;

         case QDialogButtonBox::RejectRole:
         case QDialogButtonBox::NoRole:
            emit q->rejected();
            break;

         case QDialogButtonBox::HelpRole:
            emit q->helpRequested();
            break;

         default:
            break;
      }
   }
}

void QDialogButtonBoxPrivate::_q_handleButtonDestroyed()
{
   Q_Q(QDialogButtonBox);

   QObject *object = q->sender();

   if (object != nullptr) {
      // remove from the standard button hash first and then from the roles
      for (auto iter = standardButtonHash.cbegin(); iter != standardButtonHash.cend(); ++iter) {
         if (iter.key() == object) {
            standardButtonHash.erase(iter);
            break;
         }
      }

      for (QList<QAbstractButton *> &list : buttonLists) {
         for (auto iter = list.cbegin(); iter != list.cend(); ++iter) {
            if (*iter == object) {
               list.erase(iter);
               break;
            }
         }
      }
   }
}

void QDialogButtonBox::setCenterButtons(bool center)
{
   Q_D(QDialogButtonBox);

   if (d->center != center) {
      d->center = center;
      d->resetLayout();
   }
}

bool QDialogButtonBox::centerButtons() const
{
   Q_D(const QDialogButtonBox);
   return d->center;
}

void QDialogButtonBox::changeEvent(QEvent *event)
{
   typedef QHash<QPushButton *, QDialogButtonBox::StandardButton> StandardButtonHash;

   Q_D(QDialogButtonBox);

   switch (event->type()) {
      case QEvent::StyleChange:
         // Propagate style

         if (! d->standardButtonHash.empty()) {
            QStyle *newStyle = style();
            const StandardButtonHash::iterator end = d->standardButtonHash.end();

            for (StandardButtonHash::iterator it = d->standardButtonHash.begin(); it != end; ++it) {
               it.key()->setStyle(newStyle);
            }
         }

#ifdef Q_OS_DARWIN
         [[fallthrough]];

      case QEvent::MacSizeChange:
#endif
         d->resetLayout();
         QWidget::changeEvent(event);
         break;

      default:
         QWidget::changeEvent(event);
         break;
   }
}

bool QDialogButtonBox::event(QEvent *event)
{
   Q_D(QDialogButtonBox);

   if (event->type() == QEvent::Show) {
      QList<QAbstractButton *> acceptRoleList = d->buttonLists[AcceptRole];
      QPushButton *firstAcceptButton = acceptRoleList.isEmpty() ? nullptr : qobject_cast<QPushButton *>(acceptRoleList.at(0));
      bool hasDefault = false;

      QWidget *dialog = nullptr;
      QWidget *p = this;

      while (p && !p->isWindow()) {
         p = p->parentWidget();
         if ((dialog = qobject_cast<QDialog *>(p))) {
            break;
         }
      }

      for (QPushButton *pb : (dialog ? dialog : this)->findChildren<QPushButton *>()) {
         if (pb->isDefault() && pb != firstAcceptButton) {
            hasDefault = true;
            break;
         }
      }

      if (! hasDefault && firstAcceptButton) {
         firstAcceptButton->setDefault(true);
      }

   } else if (event->type() == QEvent::LanguageChange) {
      d->retranslateStrings();
   }

   return QWidget::event(event);
}

void QDialogButtonBox::_q_handleButtonClicked()
{
   Q_D(QDialogButtonBox);
   d->_q_handleButtonClicked();
}

void QDialogButtonBox::_q_handleButtonDestroyed()
{
   Q_D(QDialogButtonBox);
   d->_q_handleButtonDestroyed();
}

