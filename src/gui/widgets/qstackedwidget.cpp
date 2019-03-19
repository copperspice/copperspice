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

#include <qstackedwidget.h>

#ifndef QT_NO_STACKEDWIDGET

#include <qstackedlayout.h>
#include <qevent.h>
#include <qframe_p.h>

QT_BEGIN_NAMESPACE

/**
   QStackedLayout does not support height for width (simply because it does not reimplement
   heightForWidth() and hasHeightForWidth()). That is not possible to fix without breaking
   binary compatibility. (QLayout is subject to multiple inheritance).
   However, we can fix QStackedWidget by simply using a modified version of QStackedLayout
   that reimplements the hfw-related functions:
 */
class QStackedLayoutHFW : public QStackedLayout
{
 public:
   QStackedLayoutHFW(QWidget *parent = nullptr) : QStackedLayout(parent) {}

   bool hasHeightForWidth() const  override;
   int heightForWidth(int width) const  override;
};

bool QStackedLayoutHFW::hasHeightForWidth() const
{
   const int n = count();

   for (int i = 0; i < n; ++i) {
      if (QLayoutItem *item = itemAt(i)) {
         if (item->hasHeightForWidth()) {
            return true;
         }
      }
   }
   return false;
}

int QStackedLayoutHFW::heightForWidth(int width) const
{
   const int n = count();

   int hfw = 0;
   for (int i = 0; i < n; ++i) {
      if (QLayoutItem *item = itemAt(i)) {
         if (QWidget *w = item->widget()) {
            hfw = qMax(hfw, w->heightForWidth(width));
         }
      }
   }

   hfw = qMax(hfw, minimumSize().height());
   return hfw;
}


class QStackedWidgetPrivate : public QFramePrivate
{
   Q_DECLARE_PUBLIC(QStackedWidget)
 public:
   QStackedWidgetPrivate(): layout(0) {}
   QStackedLayoutHFW *layout;
   bool blockChildAdd;
};

/*!
    \class QStackedWidget
    \brief The QStackedWidget class provides a stack of widgets where
    only one widget is visible at a time.

    \ingroup organizers
    \ingroup geomanagement


    QStackedWidget can be used to create a user interface similar to
    the one provided by QTabWidget. It is a convenience layout widget
    built on top of the QStackedLayout class.

    Like QStackedLayout, QStackedWidget can be constructed and
    populated with a number of child widgets ("pages"):

    \snippet doc/src/snippets/qstackedwidget/main.cpp 0
    \snippet doc/src/snippets/qstackedwidget/main.cpp 2
    \snippet doc/src/snippets/qstackedwidget/main.cpp 3

    QStackedWidget provides no intrinsic means for the user to switch
    page. This is typically done through a QComboBox or a QListWidget
    that stores the titles of the QStackedWidget's pages. For
    example:

    \snippet doc/src/snippets/qstackedwidget/main.cpp 1

    When populating a stacked widget, the widgets are added to an
    internal list. The indexOf() function returns the index of a
    widget in that list. The widgets can either be added to the end of
    the list using the addWidget() function, or inserted at a given
    index using the insertWidget() function. The removeWidget()
    function removes a widget from the stacked widget. The number of
    widgets contained in the stacked widget, can
    be obtained using the count() function.

    The widget() function returns the widget at a given index
    position. The index of the widget that is shown on screen is given
    by currentIndex() and can be changed using setCurrentIndex(). In a
    similar manner, the currently shown widget can be retrieved using
    the currentWidget() function, and altered using the
    setCurrentWidget() function.

    Whenever the current widget in the stacked widget changes or a
    widget is removed from the stacked widget, the currentChanged()
    and widgetRemoved() signals are emitted respectively.

    \sa QStackedLayout, QTabWidget, {Config Dialog Example}
*/

/*!
    \fn void QStackedWidget::currentChanged(int index)

    This signal is emitted whenever the current widget changes.

    The parameter holds the \a index of the new current widget, or -1
    if there isn't a new one (for example, if there are no widgets in
    the QStackedWidget).

    \sa currentWidget(), setCurrentWidget()
*/

/*!
    \fn void QStackedWidget::widgetRemoved(int index)

    This signal is emitted whenever a widget is removed. The widget's
    \a index is passed as parameter.

    \sa removeWidget()
*/

/*!
    Constructs a QStackedWidget with the given \a parent.

    \sa addWidget(), insertWidget()
*/
QStackedWidget::QStackedWidget(QWidget *parent)
   : QFrame(*new QStackedWidgetPrivate, parent)
{
   Q_D(QStackedWidget);
   d->layout = new QStackedLayoutHFW(this);

   connect(d->layout, SIGNAL(widgetRemoved(int)),  this, SLOT(widgetRemoved(int)));
   connect(d->layout, SIGNAL(currentChanged(int)), this, SLOT(currentChanged(int)));
}

/*!
    Destroys this stacked widget, and frees any allocated resources.
*/
QStackedWidget::~QStackedWidget()
{
}

/*!
    Appends the given \a widget to the QStackedWidget and returns the
    index position. Ownership of \a widget is passed on to the
    QStackedWidget.

    If the QStackedWidget is empty before this function is called,
    \a widget becomes the current widget.

    \sa insertWidget(), removeWidget(), setCurrentWidget()
*/
int QStackedWidget::addWidget(QWidget *widget)
{
   return d_func()->layout->addWidget(widget);
}

/*!
    Inserts the given \a widget at the given \a index in the
    QStackedWidget. Ownership of \a widget is passed on to the
    QStackedWidget. If \a index is out of range, the \a widget is
    appended (in which case it is the actual index of the \a widget
    that is returned).

    If the QStackedWidget was empty before this function is called,
    the given \a widget becomes the current widget.

    Inserting a new widget at an index less than or equal to the current index
    will increment the current index, but keep the current widget.

    \sa addWidget(), removeWidget(), setCurrentWidget()
*/
int QStackedWidget::insertWidget(int index, QWidget *widget)
{
   return d_func()->layout->insertWidget(index, widget);
}

/*!
    Removes \a widget from the QStackedWidget. i.e., \a widget is \e
    not deleted but simply removed from the stacked layout, causing it
    to be hidden.

    \bold{Note:} Ownership of \a widget reverts to the application.

    \sa addWidget(), insertWidget(), currentWidget()
*/
void QStackedWidget::removeWidget(QWidget *widget)
{
   d_func()->layout->removeWidget(widget);
}

/*!
    \property QStackedWidget::currentIndex
    \brief the index position of the widget that is visible

    The current index is -1 if there is no current widget.

    By default, this property contains a value of -1 because the stack
    is initially empty.

    \sa currentWidget(), indexOf()
*/

void QStackedWidget::setCurrentIndex(int index)
{
   d_func()->layout->setCurrentIndex(index);
}

int QStackedWidget::currentIndex() const
{
   return d_func()->layout->currentIndex();
}

/*!
    Returns the current widget, or 0 if there are no child widgets.

    \sa currentIndex(), setCurrentWidget()
*/
QWidget *QStackedWidget::currentWidget() const
{
   return d_func()->layout->currentWidget();
}


/*!
    \fn void QStackedWidget::setCurrentWidget(QWidget *widget)

    Sets the current widget to be the specified \a widget. The new
    current widget must already be contained in this stacked widget.

    \sa currentWidget(), setCurrentIndex()
 */
void QStackedWidget::setCurrentWidget(QWidget *widget)
{
   Q_D(QStackedWidget);
   if (d->layout->indexOf(widget) == -1) {
      qWarning("QStackedWidget::setCurrentWidget: widget %p not contained in stack", widget);
      return;
   }
   d->layout->setCurrentWidget(widget);
}

/*!
    Returns the index of the given \a widget, or -1 if the given \a
    widget is not a child of the QStackedWidget.

    \sa currentIndex(), widget()
*/
int QStackedWidget::indexOf(QWidget *widget) const
{
   return d_func()->layout->indexOf(widget);
}

/*!
    Returns the widget at the given \a index, or 0 if there is no such
    widget.

    \sa currentWidget(), indexOf()
*/
QWidget *QStackedWidget::widget(int index) const
{
   return d_func()->layout->widget(index);
}

/*!
    \property QStackedWidget::count
    \brief the number of widgets contained by this stacked widget

    By default, this property contains a value of 0.

    \sa currentIndex(), widget()
*/
int QStackedWidget::count() const
{
   return d_func()->layout->count();
}

/*! \reimp */
bool QStackedWidget::event(QEvent *e)
{
   return QFrame::event(e);
}

QT_END_NAMESPACE

#endif // QT_NO_STACKEDWIDGET
