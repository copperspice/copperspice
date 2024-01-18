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

#include <qcheckbox.h>
#include <qapplication.h>
#include <qbitmap.h>
#include <qicon.h>
#include <qstylepainter.h>
#include <qstyle.h>
#include <qstyleoption.h>
#include <qevent.h>
#include <qabstractbutton_p.h>

class QCheckBoxPrivate : public QAbstractButtonPrivate
{
   Q_DECLARE_PUBLIC(QCheckBox)

 public:
   QCheckBoxPrivate()
      : QAbstractButtonPrivate(QSizePolicy::CheckBox), tristate(false), noChange(false),
        hovering(true), publishedState(Qt::Unchecked)
   {
   }

   uint tristate : 1;
   uint noChange : 1;
   uint hovering : 1;
   uint publishedState : 2;

   void init();
};

void QCheckBoxPrivate::init()
{
   Q_Q(QCheckBox);

   q->setCheckable(true);
   q->setMouseTracking(true);
   q->setForegroundRole(QPalette::WindowText);
   setLayoutItemMargins(QStyle::SE_CheckBoxLayoutItem);
}

void QCheckBox::initStyleOption(QStyleOptionButton *option) const
{
   if (! option) {
      return;
   }

   Q_D(const QCheckBox);

   option->initFrom(this);

   if (d->down) {
      option->state |= QStyle::State_Sunken;
   }

   if (d->tristate && d->noChange) {
      option->state |= QStyle::State_NoChange;
   } else {
      option->state |= d->checked ? QStyle::State_On : QStyle::State_Off;
   }

   if (testAttribute(Qt::WA_Hover) && underMouse()) {
      if (d->hovering) {
         option->state |= QStyle::State_MouseOver;
      } else {
         option->state &= ~QStyle::State_MouseOver;
      }
   }

   option->text = d->text;
   option->icon = d->icon;
   option->iconSize = iconSize();
}

QCheckBox::QCheckBox(QWidget *parent)
   : QAbstractButton (*new QCheckBoxPrivate, parent)
{
   Q_D(QCheckBox);
   d->init();
}

QCheckBox::QCheckBox(const QString &text, QWidget *parent)
   : QAbstractButton (*new QCheckBoxPrivate, parent)
{
   Q_D(QCheckBox);

   d->init();
   setText(text);
}

QCheckBox::~QCheckBox()
{
}

void QCheckBox::setTristate(bool y)
{
   Q_D(QCheckBox);
   d->tristate = y;
}

bool QCheckBox::isTristate() const
{
   Q_D(const QCheckBox);
   return d->tristate;
}

Qt::CheckState QCheckBox::checkState() const
{
   Q_D(const QCheckBox);
   if (d->tristate &&  d->noChange) {
      return Qt::PartiallyChecked;
   }
   return d->checked ? Qt::Checked : Qt::Unchecked;
}

void QCheckBox::setCheckState(Qt::CheckState state)
{
   Q_D(QCheckBox);

   if (state == Qt::PartiallyChecked) {
      d->tristate = true;
      d->noChange = true;
   } else {
      d->noChange = false;
   }

   d->blockRefresh = true;
   setChecked(state != Qt::Unchecked);
   d->blockRefresh = false;
   d->refresh();

   if ((uint)state != d->publishedState) {
      d->publishedState = state;
      emit stateChanged(state);
   }
}

QSize QCheckBox::sizeHint() const
{
   Q_D(const QCheckBox);

   if (d->sizeHint.isValid()) {
      return d->sizeHint;
   }

   ensurePolished();
   QFontMetrics fm = fontMetrics();
   QStyleOptionButton opt;
   initStyleOption(&opt);

   QSize sz = style()->itemTextRect(fm, QRect(), Qt::TextShowMnemonic, false, text()).size();

   if (! opt.icon.isNull()) {
      sz = QSize(sz.width() + opt.iconSize.width() + 4, qMax(sz.height(), opt.iconSize.height()));
   }

   d->sizeHint = (style()->sizeFromContents(QStyle::CT_CheckBox, &opt, sz, this)
         .expandedTo(QApplication::globalStrut()));

   return d->sizeHint;
}

QSize QCheckBox::minimumSizeHint() const
{
   return sizeHint();
}

void QCheckBox::paintEvent(QPaintEvent *)
{
   QStylePainter p(this);
   QStyleOptionButton opt;

   initStyleOption(&opt);
   p.drawControl(QStyle::CE_CheckBox, opt);
}

void QCheckBox::mouseMoveEvent(QMouseEvent *e)
{
   Q_D(QCheckBox);

   if (testAttribute(Qt::WA_Hover)) {
      bool hit = false;
      if (underMouse()) {
         hit = hitButton(e->pos());
      }

      if (hit != d->hovering) {
         update(rect());
         d->hovering = hit;
      }
   }

   QAbstractButton::mouseMoveEvent(e);
}


bool QCheckBox::hitButton(const QPoint &pos) const
{
   QStyleOptionButton opt;
   initStyleOption(&opt);

   return style()->subElementRect(QStyle::SE_CheckBoxClickRect, &opt, this).contains(pos);
}

void QCheckBox::checkStateSet()
{
   Q_D(QCheckBox);
   d->noChange = false;

   Qt::CheckState state = checkState();

   if ((uint)state != d->publishedState) {
      d->publishedState = state;
      emit stateChanged(state);
   }
}

void QCheckBox::nextCheckState()
{
   Q_D(QCheckBox);

   if (d->tristate) {
      setCheckState((Qt::CheckState)((checkState() + 1) % 3));
   } else {
      QAbstractButton::nextCheckState();
      QCheckBox::checkStateSet();
   }
}

bool QCheckBox::event(QEvent *e)
{
   Q_D(QCheckBox);

   if (e->type() == QEvent::StyleChange
#ifdef Q_OS_DARWIN
      || e->type() == QEvent::MacSizeChange
#endif

   ) {
      d->setLayoutItemMargins(QStyle::SE_CheckBoxLayoutItem);
   }

   return QAbstractButton::event(e);
}

