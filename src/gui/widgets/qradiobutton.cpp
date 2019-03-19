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

QT_BEGIN_NAMESPACE

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

/*!
    \class QRadioButton
    \brief The QRadioButton widget provides a radio button with a text label.

    \ingroup basicwidgets


    A QRadioButton is an option button that can be switched on (checked) or
    off (unchecked). Radio buttons typically present the user with a "one
    of many" choice. In a group of radio buttons only one radio button at
    a time can be checked; if the user selects another button, the
    previously selected button is switched off.

    Radio buttons are autoExclusive by default. If auto-exclusive is
    enabled, radio buttons that belong to the same parent widget
    behave as if they were part of the same exclusive button group. If
    you need multiple exclusive button groups for radio buttons that
    belong to the same parent widget, put them into a QButtonGroup.

    Whenever a button is switched on or off it emits the toggled() signal.
    Connect to this signal if you want to trigger an action each time the
    button changes state. Use isChecked() to see if a particular button is
    selected.

    Just like QPushButton, a radio button displays text, and
    optionally a small icon. The icon is set with setIcon(). The text
    can be set in the constructor or with setText(). A shortcut key
    can be specified by preceding the preferred character with an
    ampersand in the text. For example:

    \snippet doc/src/snippets/code/src_gui_widgets_qradiobutton.cpp 0

    In this example the shortcut is \e{Alt+c}. See the \l
    {QShortcut#mnemonic}{QShortcut} documentation for details (to
    display an actual ampersand, use '&&').

    Important inherited members: text(), setText(), text(),
    setDown(), isDown(), autoRepeat(), group(), setAutoRepeat(),
    toggle(), pressed(), released(), clicked(), and toggled().

    \table 100%
    \row \o \inlineimage plastique-radiobutton.png Screenshot of a Plastique radio button
         \o A radio button shown in the \l{Plastique Style Widget Gallery}{Plastique widget style}.
    \row \o \inlineimage windows-radiobutton.png Screenshot of a Windows XP radio button
         \o A radio button shown in the \l{Windows XP Style Widget Gallery}{Windows XP widget style}.
    \row \o \inlineimage macintosh-radiobutton.png Screenshot of a Macintosh radio button
         \o A radio button shown in the \l{Macintosh Style Widget Gallery}{Macintosh widget style}.
    \endtable

    \sa QPushButton, QToolButton, QCheckBox, {fowler}{GUI Design Handbook: Radio Button},
        {Group Box Example}
*/


/*!
    Constructs a radio button with the given \a parent, but with no text or
    pixmap.

    The \a parent argument is passed on to the QAbstractButton constructor.
*/

QRadioButton::QRadioButton(QWidget *parent)
   : QAbstractButton(*new QRadioButtonPrivate, parent)
{
   Q_D(QRadioButton);
   d->init();
}

/*!
    Constructs a radio button with the given \a parent and a \a text string.

    The \a parent argument is passed on to the QAbstractButton constructor.
*/

QRadioButton::QRadioButton(const QString &text, QWidget *parent)
   : QAbstractButton(*new QRadioButtonPrivate, parent)
{
   Q_D(QRadioButton);
   d->init();
   setText(text);
}

/*!
    Initialize \a option with the values from this QRadioButton. This method is useful
    for subclasses when they need a QStyleOptionButton, but don't want to fill
    in all the information themselves.

    \sa QStyleOption::initFrom()
*/
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
