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

#include <qlayout.h>
#include <qapplication.h>
#include <qlayoutengine_p.h>
#include <qmenubar.h>
#include <qtoolbar.h>
#include <qsizegrip.h>
#include <qevent.h>
#include <qstyle.h>
#include <qvariant.h>
#include <qwidget_p.h>
#include <qlayout_p.h>
#include <qformlayout.h>

QT_BEGIN_NAMESPACE

static int menuBarHeightForWidth(QWidget *menubar, int w)
{
   if (menubar && !menubar->isHidden() && !menubar->isWindow()) {
      int result = menubar->heightForWidth(qMax(w, menubar->minimumWidth()));
      if (result == -1) {
         result = menubar->sizeHint().height();
      }
      const int min = qSmartMinSize(menubar).height();
      result = qBound(min, result, menubar->maximumSize().height());
      if (result != -1) {
         return result;
      }
   }
   return 0;
}

QLayout::QLayout(QWidget *parent)
   : QObject(parent), d_ptr(new QLayoutPrivate)
{
   d_ptr->q_ptr = this;

   if (!parent) {
      return;
   }
   parent->setLayout(this);
}

/*!
    Constructs a new child QLayout.

    This layout has to be inserted into another layout before geometry
    management will work.
*/
QLayout::QLayout()
   : QObject(0), d_ptr(new QLayoutPrivate)
{
   d_ptr->q_ptr = this;
}


/*! \internal
 */
QLayout::QLayout(QLayoutPrivate &dd, QLayout *lay, QWidget *w)
   : QObject(lay ? static_cast<QObject *>(lay) : static_cast<QObject *>(w)), d_ptr(&dd)
{
   d_ptr->q_ptr = this;
   Q_D(QLayout);

   if (lay) {
      lay->addItem(this);

   } else if (w) {
      if (w->layout()) {
         qWarning("QLayout: Attempting to add QLayout \"%s\" to %s \"%s\", which  already has a layout",
                  csPrintable(QObject::objectName()), csPrintable(w->metaObject()->className()), csPrintable(w->objectName()));

         setParent(0);

      } else {
         d->topLevel = true;
         w->d_func()->layout = this;

         QT_TRY {
            invalidate();
         } QT_CATCH(...) {
            w->d_func()->layout = 0;
            QT_RETHROW;
         }
      }
   }
}

QLayoutPrivate::QLayoutPrivate()
   : insideSpacing(-1), userLeftMargin(-1), userTopMargin(-1), userRightMargin(-1),
     userBottomMargin(-1), topLevel(false), enabled(true), activated(true), autoNewChild(false),
     constraint(QLayout::SetDefaultConstraint), menubar(0)
{
}

void QLayoutPrivate::getMargin(int *result, int userMargin, QStyle::PixelMetric pm) const
{
   if (!result) {
      return;
   }

   Q_Q(const QLayout);
   if (userMargin >= 0) {
      *result = userMargin;
   } else if (!topLevel) {
      *result = 0;
   } else if (QWidget *pw = q->parentWidget()) {
      *result = pw->style()->pixelMetric(pm, 0, pw);
   } else {
      *result = 0;
   }
}

// Static item factory functions that allow for hooking things in Designer

QLayoutPrivate::QWidgetItemFactoryMethod QLayoutPrivate::widgetItemFactoryMethod = 0;
QLayoutPrivate::QSpacerItemFactoryMethod QLayoutPrivate::spacerItemFactoryMethod = 0;

QWidgetItem *QLayoutPrivate::createWidgetItem(const QLayout *layout, QWidget *widget)
{
   if (widgetItemFactoryMethod)
      if (QWidgetItem *wi = (*widgetItemFactoryMethod)(layout, widget)) {
         return wi;
      }
   return new QWidgetItemV2(widget);
}

QSpacerItem *QLayoutPrivate::createSpacerItem(const QLayout *layout, int w, int h, QSizePolicy::Policy hPolicy,
      QSizePolicy::Policy vPolicy)
{
   if (spacerItemFactoryMethod)
      if (QSpacerItem *si = (*spacerItemFactoryMethod)(layout, w, h, hPolicy, vPolicy)) {
         return si;
      }
   return new QSpacerItem(w, h,  hPolicy, vPolicy);
}

/*!
    \fn void QLayout::addItem(QLayoutItem *item)

    Implemented in subclasses to add an \a item. How it is added is
    specific to each subclass.

    This function is not usually called in application code. To add a widget
    to a layout, use the addWidget() function; to add a child layout, use the
    addLayout() function provided by the relevant QLayout subclass.

    \bold{Note:} The ownership of \a item is transferred to the layout, and it's
    the layout's responsibility to delete it.

    \sa addWidget(), QBoxLayout::addLayout(), QGridLayout::addLayout()
*/

/*!
    Adds widget \a w to this layout in a manner specific to the
    layout. This function uses addItem().
*/
void QLayout::addWidget(QWidget *w)
{
   addChildWidget(w);
   addItem(QLayoutPrivate::createWidgetItem(this, w));
}



/*!
    Sets the alignment for widget \a w to \a alignment and returns
    true if \a w is found in this layout (not including child
    layouts); otherwise returns false.
*/
bool QLayout::setAlignment(QWidget *w, Qt::Alignment alignment)
{
   int i = 0;
   QLayoutItem *item = itemAt(i);
   while (item) {
      if (item->widget() == w) {
         item->setAlignment(alignment);
         invalidate();
         return true;
      }
      ++i;
      item = itemAt(i);
   }
   return false;
}

/*!
  \overload

  Sets the alignment for the layout \a l to \a alignment and
  returns true if \a l is found in this layout (not including child
  layouts); otherwise returns false.
*/
bool QLayout::setAlignment(QLayout *l, Qt::Alignment alignment)
{
   int i = 0;
   QLayoutItem *item = itemAt(i);
   while (item) {
      if (item->layout() == l) {
         item->setAlignment(alignment);
         invalidate();
         return true;
      }
      ++i;
      item = itemAt(i);
   }
   return false;
}

/*!
    \fn void QLayout::setAlignment(Qt::Alignment alignment)

    Sets the alignment of this item to \a alignment.

    \sa QLayoutItem::setAlignment()
*/

/*!
    \fn bool QLayout::isTopLevel() const

    Returns true if this layout is a top-level layout, i.e. not a
    child of another layout; otherwise returns false.
*/

/*!
    \property QLayout::margin
    \brief the width of the outside border of the layout
    \obsolete

    Use setContentsMargins() and getContentsMargins() instead.

    \sa contentsRect(), spacing
*/

/*!
    \obsolete
*/
int QLayout::margin() const
{
   int left, top, right, bottom;
   getContentsMargins(&left, &top, &right, &bottom);
   if (left == top && top == right && right == bottom) {
      return left;
   } else {
      return -1;
   }
}

/*!
    \property QLayout::spacing
    \brief the spacing between widgets inside the layout

    If no value is explicitly set, the layout's spacing is inherited
    from the parent layout, or from the style settings for the parent
    widget.

    For QGridLayout and QFormLayout, it is possible to set different horizontal and
    vertical spacings using \l{QGridLayout::}{setHorizontalSpacing()}
    and \l{QGridLayout::}{setVerticalSpacing()}. In that case,
    spacing() returns -1.

    \sa contentsRect(), getContentsMargins(), QStyle::layoutSpacing(),
        QStyle::pixelMetric()
*/

int QLayout::spacing() const
{
   if (const QBoxLayout *boxlayout = qobject_cast<const QBoxLayout *>(this)) {
      return boxlayout->spacing();
   } else if (const QGridLayout *gridlayout = qobject_cast<const QGridLayout *>(this)) {
      return gridlayout->spacing();
   } else if (const QFormLayout *formlayout = qobject_cast<const QFormLayout *>(this)) {
      return formlayout->spacing();
   } else {
      Q_D(const QLayout);
      if (d->insideSpacing >= 0) {
         return d->insideSpacing;
      } else {
         // arbitrarily prefer horizontal spacing to vertical spacing
         return qSmartSpacing(this, QStyle::PM_LayoutHorizontalSpacing);
      }
   }
}

/*!
    \obsolete
*/
void QLayout::setMargin(int margin)
{
   setContentsMargins(margin, margin, margin, margin);
}

void QLayout::setSpacing(int spacing)
{
   if (QBoxLayout *boxlayout = qobject_cast<QBoxLayout *>(this)) {
      boxlayout->setSpacing(spacing);
   } else if (QGridLayout *gridlayout = qobject_cast<QGridLayout *>(this)) {
      gridlayout->setSpacing(spacing);
   } else if (QFormLayout *formlayout = qobject_cast<QFormLayout *>(this)) {
      formlayout->setSpacing(spacing);
   } else {
      Q_D(QLayout);
      d->insideSpacing = spacing;
      invalidate();
   }
}

/*!
    \since 4.3

    Sets the \a left, \a top, \a right, and \a bottom margins to use
    around the layout.

    By default, QLayout uses the values provided by the style. On
    most platforms, the margin is 11 pixels in all directions.

    \sa getContentsMargins(), QStyle::pixelMetric(),
        {QStyle::}{PM_LayoutLeftMargin},
        {QStyle::}{PM_LayoutTopMargin},
        {QStyle::}{PM_LayoutRightMargin},
        {QStyle::}{PM_LayoutBottomMargin}
*/
void QLayout::setContentsMargins(int left, int top, int right, int bottom)
{
   Q_D(QLayout);

   if (d->userLeftMargin == left && d->userTopMargin == top &&
         d->userRightMargin == right && d->userBottomMargin == bottom) {
      return;
   }

   d->userLeftMargin = left;
   d->userTopMargin = top;
   d->userRightMargin = right;
   d->userBottomMargin = bottom;
   invalidate();
}

/*!
    \since 4.6

    Sets the \a margins to use around the layout.

    By default, QLayout uses the values provided by the style. On
    most platforms, the margin is 11 pixels in all directions.

    \sa contentsMargins()
*/
void QLayout::setContentsMargins(const QMargins &margins)
{
   setContentsMargins(margins.left(), margins.top(), margins.right(), margins.bottom());
}

/*!
    \since 4.3

    Extracts the left, top, right, and bottom margins used around the
    layout, and assigns them to *\a left, *\a top, *\a right, and *\a
    bottom (unless they are null pointers).

    By default, QLayout uses the values provided by the style. On
    most platforms, the margin is 11 pixels in all directions.

    \sa setContentsMargins(), QStyle::pixelMetric(),
        {QStyle::}{PM_LayoutLeftMargin},
        {QStyle::}{PM_LayoutTopMargin},
        {QStyle::}{PM_LayoutRightMargin},
        {QStyle::}{PM_LayoutBottomMargin}
*/
void QLayout::getContentsMargins(int *left, int *top, int *right, int *bottom) const
{
   Q_D(const QLayout);
   d->getMargin(left, d->userLeftMargin, QStyle::PM_LayoutLeftMargin);
   d->getMargin(top, d->userTopMargin, QStyle::PM_LayoutTopMargin);
   d->getMargin(right, d->userRightMargin, QStyle::PM_LayoutRightMargin);
   d->getMargin(bottom, d->userBottomMargin, QStyle::PM_LayoutBottomMargin);
}

/*!
    \since 4.6

    Returns the margins used around the layout.

    By default, QLayout uses the values provided by the style. On
    most platforms, the margin is 11 pixels in all directions.

    \sa setContentsMargins()
*/
QMargins QLayout::contentsMargins() const
{
   int left, top, right, bottom;
   getContentsMargins(&left, &top, &right, &bottom);
   return QMargins(left, top, right, bottom);
}

/*!
    \since 4.3

    Returns the layout's geometry() rectangle, but taking into account the
    contents margins.

    \sa setContentsMargins(), getContentsMargins()
*/
QRect QLayout::contentsRect() const
{
   Q_D(const QLayout);

   int left, top, right, bottom;
   getContentsMargins(&left, &top, &right, &bottom);

   return d->rect.adjusted(+left, +top, -right, -bottom);
}

/*!
    Returns the parent widget of this layout, or 0 if this layout is
    not installed on any widget.

    If the layout is a sub-layout, this function returns the parent
    widget of the parent layout.

    \sa parent()
*/
QWidget *QLayout::parentWidget() const
{
   Q_D(const QLayout);

   if (! d->topLevel) {
      if (parent()) {
         QLayout *parentLayout = qobject_cast<QLayout *>(parent());

         if (! parentLayout) {
            qWarning("QLayout::parentWidget: A layout can only have another layout as a parent.");
            return nullptr;
         }

         return parentLayout->parentWidget();

      } else {
         return nullptr;
      }

   } else {
      Q_ASSERT(parent() && parent()->isWidgetType());
      return static_cast<QWidget *>(parent());
   }
}

/*!
    \reimp
*/
bool QLayout::isEmpty() const
{
   int i = 0;
   QLayoutItem *item = itemAt(i);
   while (item) {
      if (!item->isEmpty()) {
         return false;
      }
      ++i;
      item = itemAt(i);
   }
   return true;
}

/*!
    \reimp
*/
void QLayout::setGeometry(const QRect &r)
{
   Q_D(QLayout);
   d->rect = r;
}

/*!
    \reimp
*/
QRect QLayout::geometry() const
{
   Q_D(const QLayout);
   return d->rect;
}

/*!
    \reimp
*/
void QLayout::invalidate()
{
   Q_D(QLayout);
   d->rect = QRect();
   update();
}

static bool removeWidgetRecursively(QLayoutItem *li, QWidget *w)
{
   QLayout *lay = li->layout();
   if (!lay) {
      return false;
   }
   int i = 0;
   QLayoutItem *child;
   while ((child = lay->itemAt(i))) {
      if (child->widget() == w) {
         delete lay->takeAt(i);
         lay->invalidate();
         return true;
      } else if (removeWidgetRecursively(child, w)) {
         return true;
      } else {
         ++i;
      }
   }
   return false;
}


void QLayoutPrivate::doResize(const QSize &r)
{
   Q_Q(QLayout);

   int mbh = menuBarHeightForWidth(menubar, r.width());

   QWidget *mw = q->parentWidget();

   QRect rect = mw->testAttribute(Qt::WA_LayoutOnEntireRect) ? mw->rect() : mw->contentsRect();
   rect.setTop(rect.top() + mbh);

   q->setGeometry(rect);

#ifndef QT_NO_MENUBAR
   if (menubar) {
      menubar->setGeometry(0, 0, r.width(), mbh);
   }

#endif
}

void QLayout::widgetEvent(QEvent *e)
{
   Q_D(QLayout);

   if (! d->enabled) {
      return;
   }

   switch (e->type()) {

      case QEvent::Resize:
         if (d->activated) {
            QResizeEvent *r = (QResizeEvent *)e;
            d->doResize(r->size());
         } else {
            activate();
         }
         break;

      case QEvent::ChildRemoved: {
         QChildEvent *c = static_cast<QChildEvent *>(e);

         if (c->child()->isWidgetType()) {
            QWidget *w = (QWidget *)c->child();

#ifndef QT_NO_MENUBAR
            if (w == d->menubar) {
               d->menubar = 0;
            }
#endif
            removeWidgetRecursively(this, w);
         }
      }

      break;

      case QEvent::LayoutRequest:
         if (static_cast<QWidget *>(parent())->isVisible()) {
            activate();
         }
         break;

      default:
         break;
   }
}

/*!
    \reimp
*/
void QLayout::childEvent(QChildEvent *e)
{
   Q_D(QLayout);

   if (! d->enabled) {
      return;
   }

   if (e->type() == QEvent::ChildRemoved) {
      int i = 0;
      QLayoutItem *item;

      while ((item = itemAt(i))) {

         if (item == static_cast<QLayout *>(e->child())) {
            takeAt(i);
            invalidate();
            break;

         } else {
            ++i;
         }
      }
   }
}

/*!
  \internal
  Also takes contentsMargins and menu bar into account.
*/
int QLayout::totalHeightForWidth(int w) const
{
   Q_D(const QLayout);
   int side = 0, top = 0;
   if (d->topLevel) {
      QWidget *parent = parentWidget();
      parent->ensurePolished();
      QWidgetPrivate *wd = parent->d_func();
      side += wd->leftmargin + wd->rightmargin;
      top += wd->topmargin + wd->bottommargin;
   }
   int h = heightForWidth(w - side) + top;
#ifndef QT_NO_MENUBAR
   h += menuBarHeightForWidth(d->menubar, w);
#endif
   return h;
}

/*!
  \internal
  Also takes contentsMargins and menu bar into account.
*/
QSize QLayout::totalMinimumSize() const
{
   Q_D(const QLayout);
   int side = 0, top = 0;
   if (d->topLevel) {
      QWidget *pw = parentWidget();
      pw->ensurePolished();
      QWidgetPrivate *wd = pw->d_func();
      side += wd->leftmargin + wd->rightmargin;
      top += wd->topmargin + wd->bottommargin;
   }

   QSize s = minimumSize();
#ifndef QT_NO_MENUBAR
   top += menuBarHeightForWidth(d->menubar, s.width() + side);
#endif
   return s + QSize(side, top);
}

/*!
  \internal
  Also takes contentsMargins and menu bar into account.
*/
QSize QLayout::totalSizeHint() const
{
   Q_D(const QLayout);
   int side = 0, top = 0;
   if (d->topLevel) {
      QWidget *pw = parentWidget();
      pw->ensurePolished();
      QWidgetPrivate *wd = pw->d_func();
      side += wd->leftmargin + wd->rightmargin;
      top += wd->topmargin + wd->bottommargin;
   }

   QSize s = sizeHint();
   if (hasHeightForWidth()) {
      s.setHeight(heightForWidth(s.width() + side));
   }
#ifndef QT_NO_MENUBAR
   top += menuBarHeightForWidth(d->menubar, s.width());
#endif
   return s + QSize(side, top);
}

/*!
  \internal
  Also takes contentsMargins and menu bar into account.
*/
QSize QLayout::totalMaximumSize() const
{
   Q_D(const QLayout);
   int side = 0, top = 0;
   if (d->topLevel) {
      QWidget *pw = parentWidget();
      pw->ensurePolished();
      QWidgetPrivate *wd = pw->d_func();
      side += wd->leftmargin + wd->rightmargin;
      top += wd->topmargin + wd->bottommargin;
   }

   QSize s = maximumSize();
#ifndef QT_NO_MENUBAR
   top += menuBarHeightForWidth(d->menubar, s.width());
#endif

   if (d->topLevel)
      s = QSize(qMin(s.width() + side, QLAYOUTSIZE_MAX),
                qMin(s.height() + top, QLAYOUTSIZE_MAX));
   return s;
}

/*!
  \internal
  Destroys the layout, deleting all child layouts.
  Geometry management stops when a top-level layout is deleted.

  The layout classes will probably be fatally confused if you delete
  a sublayout.
*/
QLayout::~QLayout()
{
   Q_D(QLayout);
   /*
     This function may be called during the QObject destructor,
     when the parent no longer is a QWidget.
   */
   if (d->topLevel && parent() && parent()->isWidgetType() &&
         ((QWidget *)parent())->layout() == this) {
      ((QWidget *)parent())->d_func()->layout = nullptr;
   }
}

/*!
    This function is called from \c addLayout() or \c insertLayout() functions in
    subclasses to add layout \a l as a sub-layout.

    The only scenario in which you need to call it directly is if you
    implement a custom layout that supports nested layouts.

    \sa QBoxLayout::addLayout(), QBoxLayout::insertLayout(), QGridLayout::addLayout()
*/
void QLayout::addChildLayout(QLayout *l)
{
   if (l->parent()) {
      qWarning("QLayout::addChildLayout: layout \"%s\" already has a parent", csPrintable(l->objectName()));
      return;
   }
   l->setParent(this);

   if (QWidget *mw = parentWidget()) {
      l->d_func()->reparentChildWidgets(mw);
   }

}

/*!
   \internal
 */
bool QLayout::adoptLayout(QLayout *layout)
{
   const bool ok = !layout->parent();
   addChildLayout(layout);
   return ok;
}

#ifdef QT_DEBUG
static bool layoutDebug()
{
   static int checked_env = -1;
   if (checked_env == -1) {
      checked_env = !!qgetenv("QT_LAYOUT_DEBUG").toInt();
   }

   return checked_env;
}
#endif

void QLayoutPrivate::reparentChildWidgets(QWidget *mw)
{
   Q_Q(QLayout);
   int n =  q->count();

#ifndef QT_NO_MENUBAR
   if (menubar && menubar->parentWidget() != mw) {
      menubar->setParent(mw);
   }
#endif

   bool mwVisible = mw && mw->isVisible();
   for (int i = 0; i < n; ++i) {

      QLayoutItem *item = q->itemAt(i);

      if (QWidget *w = item->widget()) {
         QWidget *pw = w->parentWidget();

#ifdef QT_DEBUG
         if (pw && pw != mw && layoutDebug()) {
            qWarning("QLayout::addChildLayout() Widget %s \"%s\" in wrong parent, moved to correct parent",
                     csPrintable(w->metaObject()->className()), csPrintable(w->objectName()));
         }
#endif
         bool needShow = mwVisible && !(w->isHidden() && w->testAttribute(Qt::WA_WState_ExplicitShowHide));
         if (pw != mw) {
            w->setParent(mw);
         }
         if (needShow) {
            QMetaObject::invokeMethod(w, "_q_showIfNotHidden", Qt::QueuedConnection);   //show later
         }

      } else if (QLayout *l = item->layout()) {
         l->d_func()->reparentChildWidgets(mw);
      }
   }
}

/*!
    This function is called from \c addWidget() functions in
    subclasses to add \a w as a managed widget of a layout.

    If \a w is already managed by a layout, this function will give a warning
    and remove \a w from that layout. This function must therefore be
    called before adding \a w to the layout's data structure.
*/
void QLayout::addChildWidget(QWidget *w)
{
   QWidget *mw = parentWidget();
   QWidget *pw = w->parentWidget();

   //Qt::WA_LaidOut is never reset. It only means that the widget at some point has been in a layout.
   if (pw && w->testAttribute(Qt::WA_LaidOut)) {
      QLayout *l = pw->layout();

      if (l && removeWidgetRecursively(l, w)) {

#ifdef QT_DEBUG
         if (layoutDebug()) {
            qWarning("QLayout::addChildWidget: %s \"%s\" is already in a layout; moved to new layout",
                     csPrintable(w->metaObject()->className()), csPrintable(w->objectName()));
         }
#endif
      }
   }

   if (pw && mw && pw != mw) {

#ifdef QT_DEBUG
      if (layoutDebug())
         qWarning("QLayout::addChildWidget: %s \"%s\" in wrong parent; moved to correct parent",
                  csPrintable(w->metaObject()->className()), csPrintable(w->objectName()));
#endif
      pw = 0;
   }

   bool needShow = mw && mw->isVisible() && !(w->isHidden() && w->testAttribute(Qt::WA_WState_ExplicitShowHide));

   if (! pw && mw) {
      w->setParent(mw);
   }

   w->setAttribute(Qt::WA_LaidOut);
   if (needShow) {
      // show later
      QMetaObject::invokeMethod(w, "_q_showIfNotHidden", Qt::QueuedConnection);
   }
}


/*!
    Tells the geometry manager to place the menu bar \a widget at the
    top of parentWidget(), outside QWidget::contentsMargins(). All
    child widgets are placed below the bottom edge of the menu bar.
*/
void QLayout::setMenuBar(QWidget *widget)
{
   Q_D(QLayout);

   if (widget) {
      addChildWidget(widget);
   }

   d->menubar = widget;
}

/*!
    Returns the menu bar set for this layout, or 0 if no menu bar is
    set.
*/

QWidget *QLayout::menuBar() const
{
   Q_D(const QLayout);
   return d->menubar;
}


/*!
    Returns the minimum size of this layout. This is the smallest
    size that the layout can have while still respecting the
    specifications.

    The returned value doesn't include the space required by
    QWidget::setContentsMargins() or menuBar().

    The default implementation allows unlimited resizing.
*/
QSize QLayout::minimumSize() const
{
   return QSize(0, 0);
}

/*!
    Returns the maximum size of this layout. This is the largest size
    that the layout can have while still respecting the
    specifications.

    The returned value doesn't include the space required by
    QWidget::setContentsMargins() or menuBar().

    The default implementation allows unlimited resizing.
*/
QSize QLayout::maximumSize() const
{
   return QSize(QLAYOUTSIZE_MAX, QLAYOUTSIZE_MAX);
}

/*!
    Returns whether this layout can make use of more space than
    sizeHint(). A value of Qt::Vertical or Qt::Horizontal means that
    it wants to grow in only one dimension, whereas Qt::Vertical |
    Qt::Horizontal means that it wants to grow in both dimensions.

    The default implementation returns Qt::Horizontal | Qt::Vertical.
    Subclasses reimplement it to return a meaningful value based on
    their child widgets's \l{QSizePolicy}{size policies}.

    \sa sizeHint()
*/
Qt::Orientations QLayout::expandingDirections() const
{
   return Qt::Horizontal | Qt::Vertical;
}

void QLayout::activateRecursiveHelper(QLayoutItem *item)
{
   item->invalidate();
   QLayout *layout = item->layout();
   if (layout) {
      QLayoutItem *child;
      int i = 0;
      while ((child = layout->itemAt(i++))) {
         activateRecursiveHelper(child);
      }
      layout->d_func()->activated = true;
   }
}

/*!
  Updates the layout for parentWidget().

  You should generally not need to call this because it is
  automatically called at the most appropriate times.

  \sa activate(), invalidate()
*/

void QLayout::update()
{
   QLayout *layout = this;

   while (layout && layout->d_func()->activated) {
      layout->d_func()->activated = false;

      if (layout->d_func()->topLevel) {
         Q_ASSERT(layout->parent()->isWidgetType());

         QWidget *mw = static_cast<QWidget *>(layout->parent());
         QApplication::postEvent(mw, new QEvent(QEvent::LayoutRequest));
         break;
      }
      layout = static_cast<QLayout *>(layout->parent());
   }
}

/*!
    Redoes the layout for parentWidget() if necessary.

    You should generally not need to call this because it is
    automatically called at the most appropriate times. It returns
    true if the layout was redone.

    \sa update(), QWidget::updateGeometry()
*/
bool QLayout::activate()
{
   Q_D(QLayout);
   if (!d->enabled || ! parent()) {
      return false;
   }
   if (!d->topLevel) {
      return static_cast<QLayout *>(parent())->activate();
   }
   if (d->activated) {
      return false;
   }

   QWidget *mw = static_cast<QWidget *>(parent());
   if (mw == 0) {
      qWarning("QLayout::activate: %s \"%s\" does not have a main widget",
               csPrintable(QObject::metaObject()->className()), csPrintable(QObject::objectName()));
      return false;
   }
   activateRecursiveHelper(this);

   QWidgetPrivate *md = mw->d_func();
   uint explMin = md->extra ? md->extra->explicitMinSize : 0;
   uint explMax = md->extra ? md->extra->explicitMaxSize : 0;

   switch (d->constraint) {
      case SetFixedSize:
         // will trigger resize
         mw->setFixedSize(totalSizeHint());
         break;
      case SetMinimumSize:
         mw->setMinimumSize(totalMinimumSize());
         break;
      case SetMaximumSize:
         mw->setMaximumSize(totalMaximumSize());
         break;
      case SetMinAndMaxSize:
         mw->setMinimumSize(totalMinimumSize());
         mw->setMaximumSize(totalMaximumSize());
         break;
      case SetDefaultConstraint: {
         bool widthSet = explMin & Qt::Horizontal;
         bool heightSet = explMin & Qt::Vertical;
         if (mw->isWindow()) {
            QSize ms = totalMinimumSize();
            if (widthSet) {
               ms.setWidth(mw->minimumSize().width());
            }
            if (heightSet) {
               ms.setHeight(mw->minimumSize().height());
            }
            if ((!heightSet || !widthSet) && hasHeightForWidth()) {
               int h = minimumHeightForWidth(ms.width());
               if (h > ms.height()) {
                  if (!heightSet) {
                     ms.setHeight(0);
                  }
                  if (!widthSet) {
                     ms.setWidth(0);
                  }
               }
            }
            mw->setMinimumSize(ms);
         } else if (!widthSet || !heightSet) {
            QSize ms = mw->minimumSize();
            if (!widthSet) {
               ms.setWidth(0);
            }
            if (!heightSet) {
               ms.setHeight(0);
            }
            mw->setMinimumSize(ms);
         }
         break;
      }
      case SetNoConstraint:
         break;
   }

   d->doResize(mw->size());

   if (md->extra) {
      md->extra->explicitMinSize = explMin;
      md->extra->explicitMaxSize = explMax;
   }
   // ideally only if sizeHint() or sizePolicy() has changed
   mw->updateGeometry();
   return true;
}

/*!
    \fn QLayoutItem *QLayout::itemAt(int index) const

    Must be implemented in subclasses to return the layout item at \a
    index. If there is no such item, the function must return 0.
    Items are numbered consecutively from 0. If an item is deleted, other items will be renumbered.

    This function can be used to iterate over a layout. The following
    code will draw a rectangle for each layout item in the layout structure of the widget.

    \snippet doc/src/snippets/code/src_gui_kernel_qlayout.cpp 0

    \sa count(), takeAt()
*/

/*!
    \fn QLayoutItem *QLayout::takeAt(int index)

    Must be implemented in subclasses to remove the layout item at \a
    index from the layout, and return the item. If there is no such
    item, the function must do nothing and return 0.  Items are numbered
    consecutively from 0. If an item is removed, other items will be
    renumbered.

    The following code fragment shows a safe way to remove all items
    from a layout:

    \snippet doc/src/snippets/code/src_gui_kernel_qlayout.cpp 1

    \sa itemAt(), count()
*/

/*!
    \fn int *QLayout::count() const

    Must be implemented in subclasses to return the number of items
    in the layout.

    \sa itemAt()
*/

/*!
    Searches for widget \a widget in this layout (not including child
    layouts).

    Returns the index of \a widget, or -1 if \a widget is not found.

    The default implementation iterates over all items using itemAt()
*/
int QLayout::indexOf(QWidget *widget) const
{
   int i = 0;
   QLayoutItem *item = itemAt(i);
   while (item) {
      if (item->widget() == widget) {
         return i;
      }
      ++i;
      item = itemAt(i);
   }
   return -1;
}

/*!
    \enum QLayout::SizeConstraint

    The possible values are:

    \value SetDefaultConstraint The main widget's minimum size is set
                    to minimumSize(), unless the widget already has
                    a minimum size.

    \value SetFixedSize The main widget's size is set to sizeHint(); it
                    cannot be resized at all.
    \value SetMinimumSize  The main widget's minimum size is set to
                    minimumSize(); it cannot be smaller.

    \value SetMaximumSize  The main widget's maximum size is set to
                    maximumSize(); it cannot be larger.

    \value SetMinAndMaxSize  The main widget's minimum size is set to
                    minimumSize() and its maximum size is set to
                    maximumSize().

    \value SetNoConstraint  The widget is not constrained.

    \omitvalue Auto
    \omitvalue FreeResize
    \omitvalue Minimum
    \omitvalue Fixed

    \sa setSizeConstraint()
*/

/*!
    \property QLayout::sizeConstraint
    \brief the resize mode of the layout

    The default mode is \l {QLayout::SetDefaultConstraint}
    {SetDefaultConstraint}.
*/
void QLayout::setSizeConstraint(SizeConstraint constraint)
{
   Q_D(QLayout);
   if (constraint == d->constraint) {
      return;
   }

   d->constraint = constraint;
   invalidate();
}

QLayout::SizeConstraint QLayout::sizeConstraint() const
{
   Q_D(const QLayout);
   return d->constraint;
}

/*!
    Returns the rectangle that should be covered when the geometry of
    this layout is set to \a r, provided that this layout supports
    setAlignment().

    The result is derived from sizeHint() and expanding(). It is never
    larger than \a r.
*/
QRect QLayout::alignmentRect(const QRect &r) const
{
   QSize s = sizeHint();
   Qt::Alignment a = alignment();

   /*
     This is a hack to obtain the real maximum size, not
     QSize(QLAYOUTSIZE_MAX, QLAYOUTSIZE_MAX), the value consistently
     returned by QLayoutItems that have an alignment.
   */
   QLayout *that = const_cast<QLayout *>(this);
   that->setAlignment(0);
   QSize ms = that->maximumSize();
   that->setAlignment(a);

   if ((expandingDirections() & Qt::Horizontal) ||
         !(a & Qt::AlignHorizontal_Mask)) {
      s.setWidth(qMin(r.width(), ms.width()));
   }
   if ((expandingDirections() & Qt::Vertical) ||
         !(a & Qt::AlignVertical_Mask)) {
      s.setHeight(qMin(r.height(), ms.height()));
   } else if (hasHeightForWidth()) {
      int hfw = heightForWidth(s.width());
      if (hfw < s.height()) {
         s.setHeight(qMin(hfw, ms.height()));
      }
   }

   s = s.boundedTo(r.size());
   int x = r.x();
   int y = r.y();

   if (a & Qt::AlignBottom) {
      y += (r.height() - s.height());
   } else if (!(a & Qt::AlignTop)) {
      y += (r.height() - s.height()) / 2;
   }

   QWidget *parent = parentWidget();
   a = QStyle::visualAlignment(parent ? parent->layoutDirection() : QApplication::layoutDirection(), a);
   if (a & Qt::AlignRight) {
      x += (r.width() - s.width());
   } else if (!(a & Qt::AlignLeft)) {
      x += (r.width() - s.width()) / 2;
   }

   return QRect(x, y, s.width(), s.height());
}

/*!
    Removes the widget \a widget from the layout. After this call, it
    is the caller's responsibility to give the widget a reasonable
    geometry or to put the widget back into a layout.

    \bold{Note:} The ownership of \a widget remains the same as
    when it was added.

    \sa removeItem(), QWidget::setGeometry(), addWidget()
*/
void QLayout::removeWidget(QWidget *widget)
{
   int i = 0;
   QLayoutItem *child;
   while ((child = itemAt(i))) {
      if (child->widget() == widget) {
         delete takeAt(i);
         invalidate();
      } else {
         ++i;
      }
   }
}

/*!
    Removes the layout item \a item from the layout. It is the
    caller's responsibility to delete the item.

    Notice that \a item can be a layout (since QLayout inherits
    QLayoutItem).

    \sa removeWidget(), addItem()
*/
void QLayout::removeItem(QLayoutItem *item)
{
   int i = 0;
   QLayoutItem *child;
   while ((child = itemAt(i))) {
      if (child == item) {
         takeAt(i);
         invalidate();
      } else {
         ++i;
      }
   }
}

/*!
    Enables this layout if \a enable is true, otherwise disables it.

    An enabled layout adjusts dynamically to changes; a disabled
    layout acts as if it did not exist.

    By default all layouts are enabled.

    \sa isEnabled()
*/
void QLayout::setEnabled(bool enable)
{
   Q_D(QLayout);
   d->enabled = enable;
}

/*!
    Returns true if the layout is enabled; otherwise returns false.

    \sa setEnabled()
*/
bool QLayout::isEnabled() const
{
   Q_D(const QLayout);
   return d->enabled;
}

/*!
    Returns a size that satisfies all size constraints on \a widget,
    including heightForWidth() and that is as close as possible to \a
    size.
*/

QSize QLayout::closestAcceptableSize(const QWidget *widget, const QSize &size)
{
   QSize result = size.boundedTo(qSmartMaxSize(widget));
   result = result.expandedTo(qSmartMinSize(widget));
   QLayout *l = widget->layout();
   if (l && l->hasHeightForWidth() && result.height() < l->minimumHeightForWidth(result.width()) ) {
      QSize current = widget->size();
      int currentHfw =  l->minimumHeightForWidth(current.width());
      int newHfw = l->minimumHeightForWidth(result.width());
      if (current.height() < currentHfw || currentHfw == newHfw) {
         //handle the constant hfw case and the vertical-only case, as well as the
         // current-size-is-not-correct case
         result.setHeight(newHfw);
      } else {
         // binary search; assume hfw is decreasing ###

         int maxw = qMax(widget->width(), result.width());
         int maxh = qMax(widget->height(), result.height());
         int minw = qMin(widget->width(), result.width());
         int minh = qMin(widget->height(), result.height());

         int minhfw = l->minimumHeightForWidth(minw);
         int maxhfw = l->minimumHeightForWidth(maxw);
         while (minw < maxw) {
            if (minhfw > maxh) { //assume decreasing
               minw = maxw - (maxw - minw) / 2;
               minhfw = l->minimumHeightForWidth(minw);
            } else if (maxhfw < minh ) { //assume decreasing
               maxw = minw + (maxw - minw) / 2;
               maxhfw = l->minimumHeightForWidth(maxw);
            } else  {
               break;
            }
         }
         result = result.expandedTo(QSize(minw, minhfw));
      }
   }
   return result;
}

/*!
    \fn void QLayout::setResizeMode(SizeConstraint constraint)

    Use setSizeConstraint(\a constraint) instead.
*/

/*!
    \fn QLayout::SizeConstraint QLayout::resizeMode() const

    Use sizeConstraint() instead.
*/

void QSizePolicy::setControlType(ControlType type)
{
   /*
       The control type is a flag type, with values 0x1, 0x2, 0x4, 0x8, 0x10,
       etc. In memory, we pack it onto the available bits (CTSize) in
       setControlType(), and unpack it here.

       Example:

           0x00000001 maps to 0x00000000
           0x00000002 maps to 0x00000200
           0x00000004 maps to 0x00000400
           0x00000008 maps to 0x00000600
           etc.
   */

   int i = 0;
   while (true) {
      if (type & (0x1 << i)) {
         data = (data & ~CTMask) | (i << CTShift);
         return;
      }
      ++i;
   }
}

QSizePolicy::ControlType QSizePolicy::controlType() const
{
   return QSizePolicy::ControlType(0x1 << ((data & CTMask) >> CTShift));
}

#ifndef QT_NO_DATASTREAM
/*!
    \relates QSizePolicy
    \since 4.2

    Writes the size \a policy to the data stream \a stream.

    \sa \link datastreamformat.html Format of the QDataStream operators \endlink
*/
QDataStream &operator<<(QDataStream &stream, const QSizePolicy &policy)
{
   return stream << policy.data;
}

/*!
    \relates QSizePolicy
    \since 4.2

    Reads the size \a policy from the data stream \a stream.

    \sa \link datastreamformat.html Format of the QDataStream operators \endlink
*/
QDataStream &operator>>(QDataStream &stream, QSizePolicy &policy)
{
   return stream >> policy.data;
}
#endif // QT_NO_DATASTREAM

QT_END_NAMESPACE
