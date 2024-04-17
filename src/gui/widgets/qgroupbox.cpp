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

#include <qgroupbox.h>

#ifndef QT_NO_GROUPBOX

#include <qapplication.h>
#include <qbitmap.h>
#include <qdrawutil.h>
#include <qevent.h>
#include <qlayout.h>
#include <qradiobutton.h>
#include <qstyle.h>
#include <qstyleoption.h>
#include <qstylepainter.h>

#ifndef QT_NO_ACCESSIBILITY
#include <qaccessible.h>
#endif

#include <qwidget_p.h>
#include <qdebug.h>

class QGroupBoxPrivate : public QWidgetPrivate
{
   Q_DECLARE_PUBLIC(QGroupBox)

 public:
   void skip();
   void init();
   void calculateFrame();
   QString title;
   int align;

#ifndef QT_NO_SHORTCUT
   int shortcutId;
#endif

   void _q_fixFocus(Qt::FocusReason reason);
   void _q_setChildrenEnabled(bool b);
   void click();
   bool flat;
   bool checkable;
   bool checked;
   bool hover;
   bool overCheckBox;
   QStyle::SubControl pressedControl;
};


void QGroupBox::initStyleOption(QStyleOptionGroupBox *option) const
{
   if (!option) {
      return;
   }

   Q_D(const QGroupBox);
   option->initFrom(this);
   option->text = d->title;
   option->lineWidth = 1;
   option->midLineWidth = 0;
   option->textAlignment = Qt::Alignment(d->align);
   option->activeSubControls |= d->pressedControl;
   option->subControls = QStyle::SC_GroupBoxFrame;

   if (d->hover) {
      option->state |= QStyle::State_MouseOver;
   } else {
      option->state &= ~QStyle::State_MouseOver;
   }

   if (d->flat) {
      option->features |= QStyleOptionFrame::Flat;
   }

   if (d->checkable) {
      option->subControls |= QStyle::SC_GroupBoxCheckBox;
      option->state |= (d->checked ? QStyle::State_On : QStyle::State_Off);
      if ((d->pressedControl == QStyle::SC_GroupBoxCheckBox
            || d->pressedControl == QStyle::SC_GroupBoxLabel) && (d->hover || d->overCheckBox)) {
         option->state |= QStyle::State_Sunken;
      }
   }

   if (!option->palette.isBrushSet(isEnabled() ? QPalette::Active :
         QPalette::Disabled, QPalette::WindowText))
      option->textColor = QColor(style()->styleHint(QStyle::SH_GroupBox_TextLabelColor,
               option, this));

   if (!d->title.isEmpty()) {
      option->subControls |= QStyle::SC_GroupBoxLabel;
   }
}

void QGroupBoxPrivate::click()
{
   Q_Q(QGroupBox);

   QPointer<QGroupBox> guard(q);
   q->setChecked(!checked);
   if (!guard) {
      return;
   }
   emit q->clicked(checked);
}

QGroupBox::QGroupBox(QWidget *parent)
   : QWidget(*new QGroupBoxPrivate, parent, Qt::EmptyFlag)
{
   Q_D(QGroupBox);
   d->init();
}

QGroupBox::QGroupBox(const QString &title, QWidget *parent)
   : QWidget(*new QGroupBoxPrivate, parent, Qt::EmptyFlag)
{
   Q_D(QGroupBox);
   d->init();
   setTitle(title);
}

QGroupBox::~QGroupBox()
{
}

void QGroupBoxPrivate::init()
{
   Q_Q(QGroupBox);
   align = Qt::AlignLeft;
#ifndef QT_NO_SHORTCUT
   shortcutId = 0;
#endif
   flat = false;
   checkable = false;
   checked = true;
   hover = false;
   overCheckBox = false;
   pressedControl = QStyle::SC_None;
   calculateFrame();
   q->setSizePolicy(QSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred,
         QSizePolicy::GroupBox));
}

void QGroupBox::setTitle(const QString &title)
{
   Q_D(QGroupBox);
   if (d->title == title) {
      return;
   }

   d->title = title;
#ifndef QT_NO_SHORTCUT
   releaseShortcut(d->shortcutId);
   d->shortcutId = grabShortcut(QKeySequence::mnemonic(title));
#endif
   d->calculateFrame();

   update();
   updateGeometry();

#ifndef QT_NO_ACCESSIBILITY
   QAccessibleEvent event(this, QAccessible::NameChanged);
   QAccessible::updateAccessibility(&event);
#endif
}

QString QGroupBox::title() const
{
   Q_D(const QGroupBox);
   return d->title;
}

Qt::Alignment QGroupBox::alignment() const
{
   Q_D(const QGroupBox);
   return QFlag(d->align);
}

void QGroupBox::setAlignment(int alignment)
{
   Q_D(QGroupBox);
   d->align = alignment;
   updateGeometry();
   update();
}

void QGroupBox::resizeEvent(QResizeEvent *e)
{
   QWidget::resizeEvent(e);
}

void QGroupBox::paintEvent(QPaintEvent *)
{
   QStylePainter paint(this);
   QStyleOptionGroupBox option;
   initStyleOption(&option);
   paint.drawComplexControl(QStyle::CC_GroupBox, option);
}

bool QGroupBox::event(QEvent *e)
{
   Q_D(QGroupBox);

#ifndef QT_NO_SHORTCUT
   if (e->type() == QEvent::Shortcut) {
      QShortcutEvent *se = static_cast<QShortcutEvent *>(e);
      if (se->shortcutId() == d->shortcutId) {
         if (!isCheckable()) {
            d->_q_fixFocus(Qt::ShortcutFocusReason);
         } else {
            d->click();
            setFocus(Qt::ShortcutFocusReason);
         }
         return true;
      }
   }
#endif

   QStyleOptionGroupBox box;
   initStyleOption(&box);
   switch (e->type()) {


      case QEvent::HoverEnter:
      case QEvent::HoverMove: {
         QStyle::SubControl control = style()->hitTestComplexControl(QStyle::CC_GroupBox, &box,
               static_cast<QHoverEvent *>(e)->pos(),
               this);
         bool oldHover = d->hover;
         d->hover = d->checkable && (control == QStyle::SC_GroupBoxLabel || control == QStyle::SC_GroupBoxCheckBox);
         if (oldHover != d->hover) {
            QRect rect = style()->subControlRect(QStyle::CC_GroupBox, &box, QStyle::SC_GroupBoxCheckBox, this)
               | style()->subControlRect(QStyle::CC_GroupBox, &box, QStyle::SC_GroupBoxLabel, this);
            update(rect);
         }
         return true;
      }
      case QEvent::HoverLeave:
         d->hover = false;
         if (d->checkable) {
            QRect rect = style()->subControlRect(QStyle::CC_GroupBox, &box, QStyle::SC_GroupBoxCheckBox, this)
               | style()->subControlRect(QStyle::CC_GroupBox, &box, QStyle::SC_GroupBoxLabel, this);
            update(rect);
         }
         return true;

      case QEvent::KeyPress: {
         QKeyEvent *k = static_cast<QKeyEvent *>(e);
         if (!k->isAutoRepeat() && (k->key() == Qt::Key_Select || k->key() == Qt::Key_Space)) {
            d->pressedControl = QStyle::SC_GroupBoxCheckBox;
            update(style()->subControlRect(QStyle::CC_GroupBox, &box, QStyle::SC_GroupBoxCheckBox, this));
            return true;
         }
         break;
      }

      case QEvent::KeyRelease: {
         QKeyEvent *k = static_cast<QKeyEvent *>(e);
         if (!k->isAutoRepeat() && (k->key() == Qt::Key_Select || k->key() == Qt::Key_Space)) {
            bool toggle = (d->pressedControl == QStyle::SC_GroupBoxLabel
                  || d->pressedControl == QStyle::SC_GroupBoxCheckBox);
            d->pressedControl = QStyle::SC_None;
            if (toggle) {
               d->click();
            }
            return true;
         }
         break;
      }

      default:
         break;
   }
   return QWidget::event(e);
}

void QGroupBox::childEvent(QChildEvent *c)
{
   Q_D(QGroupBox);
   if (c->type() != QEvent::ChildAdded || !c->child()->isWidgetType()) {
      return;
   }

   QWidget *w = (QWidget *)c->child();
   if (w->isWindow()) {
      return;
   }

   if (d->checkable) {
      if (d->checked) {
         if (!w->testAttribute(Qt::WA_ForceDisabled)) {
            w->setEnabled(true);
         }
      } else {
         if (w->isEnabled()) {
            w->setEnabled(false);
            w->setAttribute(Qt::WA_ForceDisabled, false);
         }
      }
   }
}

void QGroupBoxPrivate::_q_fixFocus(Qt::FocusReason reason)
{
   Q_Q(QGroupBox);
   QWidget *fw = q->focusWidget();

   if (! fw || fw == q) {
      QWidget *best      = nullptr;
      QWidget *candidate = nullptr;
      QWidget *w = q;

      while ((w = w->nextInFocusChain()) != q) {
         if (q->isAncestorOf(w) && (w->focusPolicy() & Qt::TabFocus) == Qt::TabFocus && w->isVisibleTo(q)) {
            if (!best && qobject_cast<QRadioButton *>(w) && ((QRadioButton *)w)->isChecked())
               // we prefer a checked radio button or a widget that
               // already has focus, if there is one
            {
               best = w;
            } else if (!candidate)
               // but we'll accept anything that takes focus
            {
               candidate = w;
            }
         }
      }
      if (best) {
         fw = best;
      } else if (candidate) {
         fw = candidate;
      }
   }
   if (fw) {
      fw->setFocus(reason);
   }
}

void QGroupBoxPrivate::calculateFrame()
{
   Q_Q(QGroupBox);
   QStyleOptionGroupBox box;
   q->initStyleOption(&box);
   QRect contentsRect = q->style()->subControlRect(QStyle::CC_GroupBox, &box, QStyle::SC_GroupBoxContents, q);
   q->setContentsMargins(contentsRect.left() - box.rect.left(), contentsRect.top() - box.rect.top(),
      box.rect.right() - contentsRect.right(), box.rect.bottom() - contentsRect.bottom());
   setLayoutItemMargins(QStyle::SE_GroupBoxLayoutItem, &box);
}

void QGroupBox::focusInEvent(QFocusEvent *fe)
{
   // note no call to super
   Q_D(QGroupBox);
   if (focusPolicy() == Qt::NoFocus) {
      d->_q_fixFocus(fe->reason());
   } else {
      QWidget::focusInEvent(fe);
   }
}

QSize QGroupBox::minimumSizeHint() const
{
   Q_D(const QGroupBox);
   QStyleOptionGroupBox option;
   initStyleOption(&option);

   QFontMetrics metrics(fontMetrics());

   int baseWidth = metrics.width(d->title) + metrics.width(QLatin1Char(' '));
   int baseHeight = metrics.height();
   if (d->checkable) {
      baseWidth += style()->pixelMetric(QStyle::PM_IndicatorWidth);
      baseWidth += style()->pixelMetric(QStyle::PM_CheckBoxLabelSpacing);
      baseHeight = qMax(baseHeight, style()->pixelMetric(QStyle::PM_IndicatorHeight));
   }

   QSize size = style()->sizeFromContents(QStyle::CT_GroupBox, &option, QSize(baseWidth, baseHeight), this);
   return size.expandedTo(QWidget::minimumSizeHint());
}

bool QGroupBox::isFlat() const
{
   Q_D(const QGroupBox);
   return d->flat;
}

void QGroupBox::setFlat(bool b)
{
   Q_D(QGroupBox);
   if (d->flat == b) {
      return;
   }
   d->flat = b;
   updateGeometry();
   update();
}


void QGroupBox::setCheckable(bool checkable)
{
   Q_D(QGroupBox);

   bool wasCheckable = d->checkable;
   d->checkable = checkable;

   if (checkable) {
      setChecked(true);
      if (!wasCheckable) {
         setFocusPolicy(Qt::StrongFocus);
         d->_q_setChildrenEnabled(true);
         updateGeometry();
      }
   } else {
      if (wasCheckable) {
         setFocusPolicy(Qt::NoFocus);
         d->_q_setChildrenEnabled(true);
         updateGeometry();
      }
      d->_q_setChildrenEnabled(true);
   }

   if (wasCheckable != checkable) {
      d->calculateFrame();
      update();
   }
}

bool QGroupBox::isCheckable() const
{
   Q_D(const QGroupBox);
   return d->checkable;
}


bool QGroupBox::isChecked() const
{
   Q_D(const QGroupBox);
   return d->checkable && d->checked;
}

void QGroupBox::setChecked(bool b)
{
   Q_D(QGroupBox);

   if (d->checkable && b != d->checked) {
      update();
      d->checked = b;
      d->_q_setChildrenEnabled(b);

#ifndef QT_NO_ACCESSIBILITY
      QAccessible::State st;
      st.checked = true;
      QAccessibleStateChangeEvent e(this, st);
      QAccessible::updateAccessibility(&e);
#endif
      emit toggled(b);
   }
}

void QGroupBoxPrivate::_q_setChildrenEnabled(bool b)
{
   Q_Q(QGroupBox);

   QObjectList childList = q->children();

   for (int i = 0; i < childList.size(); ++i) {
      QObject *o = childList.at(i);

      if (o->isWidgetType()) {
         QWidget *w = static_cast<QWidget *>(o);
         if (b) {
            if (!w->testAttribute(Qt::WA_ForceDisabled)) {
               w->setEnabled(true);
            }
         } else {
            if (w->isEnabled()) {
               w->setEnabled(false);
               w->setAttribute(Qt::WA_ForceDisabled, false);
            }
         }
      }
   }
}

void QGroupBox::changeEvent(QEvent *ev)
{
   Q_D(QGroupBox);

   if (ev->type() == QEvent::EnabledChange) {
      if (d->checkable && isEnabled()) {
         // we are being enabled - disable children
         if (!d->checked) {
            d->_q_setChildrenEnabled(false);
         }
      }

   } else if (ev->type() == QEvent::FontChange

#ifdef Q_OS_DARWIN
      || ev->type() == QEvent::MacSizeChange
#endif

      || ev->type() == QEvent::StyleChange) {
      d->calculateFrame();
   }
   QWidget::changeEvent(ev);
}

void QGroupBox::mousePressEvent(QMouseEvent *event)
{
   if (event->button() != Qt::LeftButton) {
      event->ignore();
      return;
   }

   Q_D(QGroupBox);

   QStyleOptionGroupBox box;
   initStyleOption(&box);

   d->pressedControl = style()->hitTestComplexControl(QStyle::CC_GroupBox, &box,
         event->pos(), this);

   if (d->checkable && (d->pressedControl & (QStyle::SC_GroupBoxCheckBox | QStyle::SC_GroupBoxLabel))) {
      d->overCheckBox = true;
      update(style()->subControlRect(QStyle::CC_GroupBox, &box, QStyle::SC_GroupBoxCheckBox, this));
   }
}

void QGroupBox::mouseMoveEvent(QMouseEvent *event)
{
   Q_D(QGroupBox);
   QStyleOptionGroupBox box;
   initStyleOption(&box);
   QStyle::SubControl pressed = style()->hitTestComplexControl(QStyle::CC_GroupBox, &box, event->pos(), this);

   bool oldOverCheckBox = d->overCheckBox;
   d->overCheckBox = (pressed == QStyle::SC_GroupBoxCheckBox || pressed == QStyle::SC_GroupBoxLabel);

   if (d->checkable && (d->pressedControl  == QStyle::SC_GroupBoxCheckBox || d->pressedControl == QStyle::SC_GroupBoxLabel)
      && (d->overCheckBox != oldOverCheckBox)) {
      update(style()->subControlRect(QStyle::CC_GroupBox, &box, QStyle::SC_GroupBoxCheckBox, this));
   }
}

void QGroupBox::mouseReleaseEvent(QMouseEvent *event)
{
   if (event->button() != Qt::LeftButton) {
      event->ignore();
      return;
   }

   Q_D(QGroupBox);
   if (!d->overCheckBox) {
      event->ignore();
      return;
   }
   QStyleOptionGroupBox box;
   initStyleOption(&box);
   QStyle::SubControl released = style()->hitTestComplexControl(QStyle::CC_GroupBox, &box,
         event->pos(), this);
   bool toggle = d->checkable && (released == QStyle::SC_GroupBoxLabel
         || released == QStyle::SC_GroupBoxCheckBox);
   d->pressedControl = QStyle::SC_None;
   d->overCheckBox = false;
   if (toggle) {
      d->click();
   } else if (d->checkable) {
      update(style()->subControlRect(QStyle::CC_GroupBox, &box, QStyle::SC_GroupBoxCheckBox, this));
   }
}

void QGroupBox::_q_setChildrenEnabled(bool b)
{
   Q_D(QGroupBox);
   d->_q_setChildrenEnabled(b);
}

#endif //QT_NO_GROUPBOX
