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

#include <qradiobutton.h>
#include <qapplication.h>
#include <qbitmap.h>
#include <qbuttongroup.h>
#include <qstylepainter.h>
#include <qstyle.h>
#include <qstyleoption.h>
#include <qevent.h>
#include <qabstractbutton_p.h>



class QRadioButtonPrivate : public QAbstractButtonPrivate
{
   Q_DECLARE_PUBLIC(QRadioButton)

 public:
   QRadioButtonPrivate() : QAbstractButtonPrivate(QSizePolicy::RadioButton), hovering(true) {}
   void init();
   uint hovering : 1;
};

/*
    Initializes the radio button.
*/
void QRadioButtonPrivate::init()
{
   Q_Q(QRadioButton);
   q->setCheckable(true);
   q->setAutoExclusive(true);
   q->setMouseTracking(true);
   q->setForegroundRole(QPalette::WindowText);
   setLayoutItemMargins(QStyle::SE_RadioButtonLayoutItem);
}


QRadioButton::QRadioButton(QWidget *parent)
   : QAbstractButton(*new QRadioButtonPrivate, parent)
{
   Q_D(QRadioButton);
   d->init();
}

QRadioButton::~QRadioButton()
{
}
QRadioButton::QRadioButton(const QString &text, QWidget *parent)
   : QAbstractButton(*new QRadioButtonPrivate, parent)
{
   Q_D(QRadioButton);
   d->init();
   setText(text);
}


void QRadioButton::initStyleOption(QStyleOptionButton *option) const
{
   if (!option) {
      return;
   }
   Q_D(const QRadioButton);
   option->initFrom(this);
   option->text = d->text;
   option->icon = d->icon;
   option->iconSize = iconSize();
   if (d->down) {
      option->state |= QStyle::State_Sunken;
   }
   option->state |= (d->checked) ? QStyle::State_On : QStyle::State_Off;
   if (testAttribute(Qt::WA_Hover) && underMouse()) {
      if (d->hovering) {
         option->state |= QStyle::State_MouseOver;
      } else {
         option->state &= ~QStyle::State_MouseOver;
      }
   }
}

/*!
    \reimp
*/
QSize QRadioButton::sizeHint() const
{
   Q_D(const QRadioButton);
   if (d->sizeHint.isValid()) {
      return d->sizeHint;
   }
   ensurePolished();
   QStyleOptionButton opt;
   initStyleOption(&opt);
   QSize sz = style()->itemTextRect(fontMetrics(), QRect(), Qt::TextShowMnemonic,
         false, text()).size();
   if (!opt.icon.isNull()) {
      sz = QSize(sz.width() + opt.iconSize.width() + 4, qMax(sz.height(), opt.iconSize.height()));
   }
   d->sizeHint = (style()->sizeFromContents(QStyle::CT_RadioButton, &opt, sz, this).
         expandedTo(QApplication::globalStrut()));
   return d->sizeHint;
}

/*!
    \reimp
    \since 4.8
*/
QSize QRadioButton::minimumSizeHint() const
{
   return sizeHint();
}

/*!
    \reimp
*/
bool QRadioButton::hitButton(const QPoint &pos) const
{
   QStyleOptionButton opt;
   initStyleOption(&opt);
   return style()->subElementRect(QStyle::SE_RadioButtonClickRect, &opt, this).contains(pos);
}

/*!
    \reimp
*/
void QRadioButton::mouseMoveEvent(QMouseEvent *e)
{
   Q_D(QRadioButton);
   if (testAttribute(Qt::WA_Hover)) {
      bool hit = false;
      if (underMouse()) {
         hit = hitButton(e->pos());
      }

      if (hit != d->hovering) {
         update();
         d->hovering = hit;
      }
   }

   QAbstractButton::mouseMoveEvent(e);
}

/*!\reimp
 */
void QRadioButton::paintEvent(QPaintEvent *)
{
   QStylePainter p(this);
   QStyleOptionButton opt;
   initStyleOption(&opt);
   p.drawControl(QStyle::CE_RadioButton, opt);
}

/*! \reimp */
bool QRadioButton::event(QEvent *e)
{
   Q_D(QRadioButton);
   if (e->type() == QEvent::StyleChange
#ifdef Q_OS_MAC
      || e->type() == QEvent::MacSizeChange
#endif
   ) {
      d->setLayoutItemMargins(QStyle::SE_RadioButtonLayoutItem);
   }
   return QAbstractButton::event(e);
}

QT_END_NAMESPACE
