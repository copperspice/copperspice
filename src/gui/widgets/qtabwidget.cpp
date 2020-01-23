/***********************************************************************
*
* Copyright (c) 2012-2020 Barbara Geller
* Copyright (c) 2012-2020 Ansel Sermersheim
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

#include <qtabwidget.h>

#ifndef QT_NO_TABWIDGET

#include <qwidget_p.h>
#include <qtabbar_p.h>
#include <qapplication.h>
#include <qbitmap.h>
#include <qdesktopwidget.h>
#include <qevent.h>
#include <qlayout.h>
#include <qstackedwidget.h>
#include <qstyle.h>
#include <qstyleoption.h>
#include <qstylepainter.h>
#include <qtabbar.h>
#include <qtoolbutton.h>



class QTabWidgetPrivate : public QWidgetPrivate
{
   Q_DECLARE_PUBLIC(QTabWidget)

 public:
   QTabWidgetPrivate();
   ~QTabWidgetPrivate();

   void updateTabBarPosition();
   void _q_showTab(int);
   void _q_removeTab(int);
   void _q_tabMoved(int from, int to);
   void init();

   QTabBar *tabs;
   QStackedWidget *stack;
   QRect panelRect;
   bool dirty;
   QTabWidget::TabPosition pos;
   QTabWidget::TabShape shape;
   int alignment;
   QWidget *leftCornerWidget;
   QWidget *rightCornerWidget;
};

QTabWidgetPrivate::QTabWidgetPrivate()
   : tabs(0), stack(0), dirty(true),
     pos(QTabWidget::North), shape(QTabWidget::Rounded),
     leftCornerWidget(0), rightCornerWidget(0)
{}

QTabWidgetPrivate::~QTabWidgetPrivate()
{}

void QTabWidgetPrivate::init()
{
   Q_Q(QTabWidget);

   stack = new QStackedWidget(q);
   stack->setObjectName(QLatin1String("qt_tabwidget_stackedwidget"));
   stack->setLineWidth(0);
   // hack so that QMacStyle::layoutSpacing() can detect tab widget pages
   stack->setSizePolicy(QSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred, QSizePolicy::TabWidget));

   QObject::connect(stack, SIGNAL(widgetRemoved(int)), q, SLOT(_q_removeTab(int)));
   QTabBar *tabBar = new QTabBar(q);
   tabBar->setObjectName(QLatin1String("qt_tabwidget_tabbar"));
   tabBar->setDrawBase(false);
   q->setTabBar(tabBar);

   q->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding,
         QSizePolicy::TabWidget));
#ifdef QT_KEYPAD_NAVIGATION
   if (QApplication::keypadNavigationEnabled()) {
      q->setFocusPolicy(Qt::NoFocus);
   } else
#endif
      q->setFocusPolicy(Qt::TabFocus);
   q->setFocusProxy(tabs);
   q->setTabPosition(static_cast<QTabWidget::TabPosition> (q->style()->styleHint(
            QStyle::SH_TabWidget_DefaultTabPosition, 0, q )));

}

bool QTabWidget::hasHeightForWidth() const
{
   Q_D(const QTabWidget);

   bool has = d->size_policy.hasHeightForWidth();
   if (!has && d->stack) {
      has = d->stack->hasHeightForWidth();
   }
   return has;
}

void QTabWidget::initStyleOption(QStyleOptionTabWidgetFrame *option) const
{
   if (!option) {
      return;
   }

   Q_D(const QTabWidget);
   option->initFrom(this);

   if (documentMode()) {
      option->lineWidth = 0;
   } else {
      option->lineWidth = style()->pixelMetric(QStyle::PM_DefaultFrameWidth, 0, this);
   }

   int exth = style()->pixelMetric(QStyle::PM_TabBarBaseHeight, 0, this);
   QSize t(0, d->stack->frameWidth());
   if (d->tabs->isVisibleTo(const_cast<QTabWidget *>(this))) {
      t = d->tabs->sizeHint();
      if (documentMode()) {
         if (tabPosition() == East || tabPosition() == West) {
            t.setHeight(height());
         } else {
            t.setWidth(width());
         }
      }
   }

   if (d->rightCornerWidget) {
      const QSize rightCornerSizeHint = d->rightCornerWidget->sizeHint();
      const QSize bounds(rightCornerSizeHint.width(), t.height() - exth);
      option->rightCornerWidgetSize = rightCornerSizeHint.boundedTo(bounds);
   } else {
      option->rightCornerWidgetSize = QSize(0, 0);
   }

   if (d->leftCornerWidget) {
      const QSize leftCornerSizeHint = d->leftCornerWidget->sizeHint();
      const QSize bounds(leftCornerSizeHint.width(), t.height() - exth);
      option->leftCornerWidgetSize = leftCornerSizeHint.boundedTo(bounds);
   } else {
      option->leftCornerWidgetSize = QSize(0, 0);
   }

   switch (d->pos) {
      case QTabWidget::North:
         option->shape = d->shape == QTabWidget::Rounded ? QTabBar::RoundedNorth
            : QTabBar::TriangularNorth;
         break;
      case QTabWidget::South:
         option->shape = d->shape == QTabWidget::Rounded ? QTabBar::RoundedSouth
            : QTabBar::TriangularSouth;
         break;
      case QTabWidget::West:
         option->shape = d->shape == QTabWidget::Rounded ? QTabBar::RoundedWest
            : QTabBar::TriangularWest;
         break;
      case QTabWidget::East:
         option->shape = d->shape == QTabWidget::Rounded ? QTabBar::RoundedEast
            : QTabBar::TriangularEast;
         break;
   }

   option->tabBarSize = t;

   QRect tbRect = tabBar()->geometry();
   QRect selectedTabRect = tabBar()->tabRect(tabBar()->currentIndex());

   option->tabBarRect = tbRect;
   selectedTabRect.moveTopLeft(selectedTabRect.topLeft() + tbRect.topLeft());
   option->selectedTabRect = selectedTabRect;
}

/*!
    Constructs a tabbed widget with parent \a parent.
*/
QTabWidget::QTabWidget(QWidget *parent)
   : QWidget(*new QTabWidgetPrivate, parent, 0)
{
   Q_D(QTabWidget);
   d->init();
}

/*!
    Destroys the tabbed widget.
*/
QTabWidget::~QTabWidget()
{
}

int QTabWidget::addTab(QWidget *child, const QString &label)
{
   return insertTab(-1, child, label);
}


/*!
    \fn int QTabWidget::addTab(QWidget *page, const QIcon &icon, const QString &label)
    \overload

    Adds a tab with the given \a page, \a icon, and \a label to the tab
    widget, and returns the index of the tab in the tab bar.

    This function is the same as addTab(), but with an additional \a
    icon.
*/
int QTabWidget::addTab(QWidget *child, const QIcon &icon, const QString &label)
{
   return insertTab(-1, child, icon, label);
}


int QTabWidget::insertTab(int index, QWidget *w, const QString &label)
{
   return insertTab(index, w, QIcon(), label);
}


/*!
    \fn int QTabWidget::insertTab(int index, QWidget *page, const QIcon& icon, const QString &label)
    \overload

    Inserts a tab with the given \a label, \a page, and \a icon into
    the tab widget at the specified \a index, and returns the index of the
    inserted tab in the tab bar.

    This function is the same as insertTab(), but with an additional
    \a icon.
*/
int QTabWidget::insertTab(int index, QWidget *w, const QIcon &icon, const QString &label)
{
   Q_D(QTabWidget);
   if (! w) {
      return -1;
   }
   index = d->stack->insertWidget(index, w);
   d->tabs->insertTab(index, icon, label);
   setUpLayout();
   tabInserted(index);

   return index;
}


void QTabWidget::setTabText(int index, const QString &label)
{
   Q_D(QTabWidget);
   d->tabs->setTabText(index, label);
   setUpLayout();
}


QString QTabWidget::tabText(int index) const
{
   Q_D(const QTabWidget);
   return d->tabs->tabText(index);
}


void QTabWidget::setTabIcon(int index, const QIcon &icon)
{
   Q_D(QTabWidget);
   d->tabs->setTabIcon(index, icon);
   setUpLayout();
}


QIcon QTabWidget::tabIcon(int index) const
{
   Q_D(const QTabWidget);
   return d->tabs->tabIcon(index);
}


bool QTabWidget::isTabEnabled(int index) const
{
   Q_D(const QTabWidget);
   return d->tabs->isTabEnabled(index);
}

void QTabWidget::setTabEnabled(int index, bool enable)
{
   Q_D(QTabWidget);
   d->tabs->setTabEnabled(index, enable);
   if (QWidget *widget = d->stack->widget(index)) {
      widget->setEnabled(enable);
   }
}

void QTabWidget::setCornerWidget(QWidget *widget, Qt::Corner corner)
{
   Q_D(QTabWidget);
   if (widget && widget->parentWidget() != this) {
      widget->setParent(this);
   }

   if (corner & Qt::TopRightCorner) {
      if (d->rightCornerWidget) {
         d->rightCornerWidget->hide();
      }
      d->rightCornerWidget = widget;
   } else {
      if (d->leftCornerWidget) {
         d->leftCornerWidget->hide();
      }
      d->leftCornerWidget = widget;
   }
   setUpLayout();
}

/*!
    Returns the widget shown in the \a corner of the tab widget or 0.
*/
QWidget *QTabWidget::cornerWidget(Qt::Corner corner) const
{
   Q_D(const QTabWidget);
   if (corner & Qt::TopRightCorner) {
      return d->rightCornerWidget;
   }
   return d->leftCornerWidget;
}

/*!
   Removes the tab at position \a index from this stack of widgets.
   The page widget itself is not deleted.

   \sa addTab(), insertTab()
*/
void QTabWidget::removeTab(int index)
{
   Q_D(QTabWidget);
   if (QWidget *w = d->stack->widget(index)) {
      d->stack->removeWidget(w);
   }
}

/*!
    Returns a pointer to the page currently being displayed by the tab
    dialog. The tab dialog does its best to make sure that this value
    is never 0 (but if you try hard enough, it can be).

    \sa currentIndex(), setCurrentWidget()
*/

QWidget *QTabWidget::currentWidget() const
{
   Q_D(const QTabWidget);
   return d->stack->currentWidget();
}

/*!
    Makes \a widget the current widget. The \a widget used must be a page in
    this tab widget.

    \sa addTab(), setCurrentIndex(), currentWidget()
 */
void QTabWidget::setCurrentWidget(QWidget *widget)
{
   Q_D(const QTabWidget);
   d->tabs->setCurrentIndex(indexOf(widget));
}


/*!
    \property QTabWidget::currentIndex
    \brief the index position of the current tab page

    The current index is -1 if there is no current widget.

    By default, this property contains a value of -1 because there are initially
    no tabs in the widget.
*/

int QTabWidget::currentIndex() const
{
   Q_D(const QTabWidget);
   return d->tabs->currentIndex();
}

void QTabWidget::setCurrentIndex(int index)
{
   Q_D(QTabWidget);
   d->tabs->setCurrentIndex(index);
}


/*!
    Returns the index position of the page occupied by the widget \a
    w, or -1 if the widget cannot be found.
*/
int QTabWidget::indexOf(QWidget *w) const
{
   Q_D(const QTabWidget);
   return d->stack->indexOf(w);
}


/*!
    \reimp
*/
void QTabWidget::resizeEvent(QResizeEvent *e)
{
   QWidget::resizeEvent(e);
   setUpLayout();
}


void QTabWidget::setTabBar(QTabBar *tb)
{
   Q_D(QTabWidget);
   Q_ASSERT(tb);

   if (tb->parentWidget() != this) {
      tb->setParent(this);
      tb->show();
   }
   delete d->tabs;
   d->tabs = tb;
   setFocusProxy(d->tabs);

   connect(d->tabs, SIGNAL(currentChanged(int)),      this, SLOT(_q_showTab(int)));
   connect(d->tabs, SIGNAL(tabMoved(int, int)),       this, SLOT(_q_tabMoved(int, int)));
   connect(d->tabs, SIGNAL(tabBarClicked(int)),       this, SLOT(tabBarClicked(int)));
   connect(d->tabs, SIGNAL(tabBarDoubleClicked(int)), this, SLOT(tabBarDoubleClicked(int)));


   if (d->tabs->tabsClosable()) {
      connect(d->tabs, SIGNAL(tabCloseRequested(int)), this, SLOT(tabCloseRequested(int)));
   }

   tb->setExpanding(!documentMode());
   setUpLayout();
}


/*!
    Returns the current QTabBar.

    \sa setTabBar()
*/
QTabBar *QTabWidget::tabBar() const
{
   Q_D(const QTabWidget);
   return d->tabs;
}

/*!
    Ensures that the selected tab's page is visible and appropriately
    sized.
*/

void QTabWidgetPrivate::_q_showTab(int index)
{
   Q_Q(QTabWidget);
   if (index < stack->count() && index >= 0) {
      stack->setCurrentIndex(index);
   }
   emit q->currentChanged(index);
}

void QTabWidgetPrivate::_q_removeTab(int index)
{
   Q_Q(QTabWidget);
   tabs->removeTab(index);
   q->setUpLayout();
   q->tabRemoved(index);
}

void QTabWidgetPrivate::_q_tabMoved(int from, int to)
{
   stack->blockSignals(true);
   QWidget *w = stack->widget(from);
   stack->removeWidget(w);
   stack->insertWidget(to, w);
   stack->blockSignals(false);
}

/*
    Set up the layout.
    Get subrect from the current style, and set the geometry for the
    stack widget, tab bar and corner widgets.
*/
void QTabWidget::setUpLayout(bool onlyCheck)
{
   Q_D(QTabWidget);
   if (onlyCheck && !d->dirty) {
      return;   // nothing to do
   }

   QStyleOptionTabWidgetFrame option;
   initStyleOption(&option);

   // this must be done immediately, because QWidgetItem relies on it (even if !isVisible())
   d->setLayoutItemMargins(QStyle::SE_TabWidgetLayoutItem, &option);

   if (!isVisible()) {
      d->dirty = true;
      return; // we'll do it later
   }

   QRect tabRect = style()->subElementRect(QStyle::SE_TabWidgetTabBar, &option, this);
   d->panelRect = style()->subElementRect(QStyle::SE_TabWidgetTabPane, &option, this);
   QRect contentsRect = style()->subElementRect(QStyle::SE_TabWidgetTabContents, &option, this);
   QRect leftCornerRect = style()->subElementRect(QStyle::SE_TabWidgetLeftCorner, &option, this);
   QRect rightCornerRect = style()->subElementRect(QStyle::SE_TabWidgetRightCorner, &option, this);

   d->tabs->setGeometry(tabRect);
   d->stack->setGeometry(contentsRect);
   if (d->leftCornerWidget) {
      d->leftCornerWidget->setGeometry(leftCornerRect);
   }
   if (d->rightCornerWidget) {
      d->rightCornerWidget->setGeometry(rightCornerRect);
   }

   if (!onlyCheck) {
      update();
   }
   updateGeometry();
}

/*!
    \internal
*/
static inline QSize basicSize(
   bool horizontal, const QSize &lc, const QSize &rc, const QSize &s, const QSize &t)
{
   return horizontal
      ? QSize(qMax(s.width(), t.width() + rc.width() + lc.width()),
         s.height() + (qMax(rc.height(), qMax(lc.height(), t.height()))))
      : QSize(s.width() + (qMax(rc.width(), qMax(lc.width(), t.width()))),
         qMax(s.height(), t.height() + rc.height() + lc.height()));
}

/*!
    \reimp
*/
QSize QTabWidget::sizeHint() const
{
   Q_D(const QTabWidget);

   QSize lc(0, 0), rc(0, 0);
   QStyleOptionTabWidgetFrame opt;
   initStyleOption(&opt);
   opt.state = QStyle::State_None;

   if (d->leftCornerWidget) {
      lc = d->leftCornerWidget->sizeHint();
   }
   if (d->rightCornerWidget) {
      rc = d->rightCornerWidget->sizeHint();
   }

   if (!d->dirty) {
      QTabWidget *that = const_cast<QTabWidget *>(this);
      that->setUpLayout(true);
   }

   QSize s(d->stack->sizeHint());
   QSize t(d->tabs->sizeHint());

   if (usesScrollButtons()) {
      t = t.boundedTo(QSize(200, 200));
   } else {
      t = t.boundedTo(QApplication::desktop()->size());
   }

   QSize sz = basicSize(d->pos == North || d->pos == South, lc, rc, s, t);

   return style()->sizeFromContents(QStyle::CT_TabWidget, &opt, sz, this)
      .expandedTo(QApplication::globalStrut());
}


/*!
    \reimp

    Returns a suitable minimum size for the tab widget.
*/
QSize QTabWidget::minimumSizeHint() const
{
   Q_D(const QTabWidget);
   QSize lc(0, 0), rc(0, 0);

   if (d->leftCornerWidget) {
      lc = d->leftCornerWidget->minimumSizeHint();
   }
   if (d->rightCornerWidget) {
      rc = d->rightCornerWidget->minimumSizeHint();
   }

   if (!d->dirty) {
      QTabWidget *that = const_cast<QTabWidget *>(this);
      that->setUpLayout(true);
   }
   QSize s(d->stack->minimumSizeHint());
   QSize t(d->tabs->minimumSizeHint());

   QSize sz = basicSize(d->pos == North || d->pos == South, lc, rc, s, t);

   QStyleOptionTabWidgetFrame opt;
   initStyleOption(&opt);
   opt.palette = palette();
   opt.state = QStyle::State_None;
   return style()->sizeFromContents(QStyle::CT_TabWidget, &opt, sz, this)
      .expandedTo(QApplication::globalStrut());
}


int QTabWidget::heightForWidth(int width) const
{
   Q_D(const QTabWidget);

   QStyleOptionTabWidgetFrame opt;
   initStyleOption(&opt);
   opt.state = QStyle::State_None;

   QSize zero(0, 0);
   const QSize padding = style()->sizeFromContents(QStyle::CT_TabWidget, &opt,
         zero, this).expandedTo(QApplication::globalStrut());

   QSize lc(0, 0), rc(0, 0);

   if (d->leftCornerWidget) {
      lc = d->leftCornerWidget->sizeHint();
   }

   if (d->rightCornerWidget) {
      rc = d->rightCornerWidget->sizeHint();
   }

   if (!d->dirty) {
      QTabWidget *that = const_cast<QTabWidget *>(this);
      that->setUpLayout(true);
   }

   QSize t(d->tabs->sizeHint());

   if (usesScrollButtons()) {
      t = t.boundedTo(QSize(200, 200));
   } else {
      t = t.boundedTo(QApplication::desktop()->size());
   }

   const bool tabIsHorizontal = (d->pos == North || d->pos == South);
   const int contentsWidth = width - padding.width();
   int stackWidth = contentsWidth;

   if (!tabIsHorizontal) {
      stackWidth -= qMax(t.width(), qMax(lc.width(), rc.width()));
   }

   int stackHeight = d->stack->heightForWidth(stackWidth);
   QSize s(stackWidth, stackHeight);

   QSize contentSize = basicSize(tabIsHorizontal, lc, rc, s, t);
   return (contentSize + padding).expandedTo(QApplication::globalStrut()).height();
}


/*!
    \reimp
 */
void QTabWidget::showEvent(QShowEvent *)
{
   setUpLayout();
}

void QTabWidgetPrivate::updateTabBarPosition()
{
   Q_Q(QTabWidget);
   switch (pos) {
      case QTabWidget::North:
         tabs->setShape(shape == QTabWidget::Rounded ? QTabBar::RoundedNorth
            : QTabBar::TriangularNorth);
         break;
      case QTabWidget::South:
         tabs->setShape(shape == QTabWidget::Rounded ? QTabBar::RoundedSouth
            : QTabBar::TriangularSouth);
         break;
      case QTabWidget::West:
         tabs->setShape(shape == QTabWidget::Rounded ? QTabBar::RoundedWest
            : QTabBar::TriangularWest);
         break;
      case QTabWidget::East:
         tabs->setShape(shape == QTabWidget::Rounded ? QTabBar::RoundedEast
            : QTabBar::TriangularEast);
         break;
   }
   q->setUpLayout();
}

/*!
    \property QTabWidget::tabPosition
    \brief the position of the tabs in this tab widget

    Possible values for this property are described by the TabPosition
    enum.

    By default, this property is set to \l North.

    \sa TabPosition
*/
QTabWidget::TabPosition QTabWidget::tabPosition() const
{
   Q_D(const QTabWidget);
   return d->pos;
}

void QTabWidget::setTabPosition(TabPosition pos)
{
   Q_D(QTabWidget);
   if (d->pos == pos) {
      return;
   }
   d->pos = pos;
   d->updateTabBarPosition();
}

/*!
    \property QTabWidget::tabsClosable
    \brief whether close buttons are automatically added to each tab.

    \since 4.5

    \sa QTabBar::tabsClosable()
*/
bool QTabWidget::tabsClosable() const
{
   return tabBar()->tabsClosable();
}

void QTabWidget::setTabsClosable(bool closeable)
{
   if (tabsClosable() == closeable) {
      return;
   }

   tabBar()->setTabsClosable(closeable);
   if (closeable) {
      connect(tabBar(), SIGNAL(tabCloseRequested(int)), this, SLOT(tabCloseRequested(int)));
   } else {
      disconnect(tabBar(), SIGNAL(tabCloseRequested(int)), this, SLOT(tabCloseRequested(int)));
   }
   setUpLayout();
}

bool QTabWidget::isMovable() const
{
   return tabBar()->isMovable();
}

void QTabWidget::setMovable(bool movable)
{
   tabBar()->setMovable(movable);
}


QTabWidget::TabShape QTabWidget::tabShape() const
{
   Q_D(const QTabWidget);
   return d->shape;
}

void QTabWidget::setTabShape(TabShape s)
{
   Q_D(QTabWidget);
   if (d->shape == s) {
      return;
   }
   d->shape = s;
   d->updateTabBarPosition();
}

/*!
    \reimp
 */
bool QTabWidget::event(QEvent *ev)
{
   if (ev->type() == QEvent::LayoutRequest) {
      setUpLayout();
   }
   return QWidget::event(ev);
}

/*!
    \reimp
 */
void QTabWidget::changeEvent(QEvent *ev)
{
   if (ev->type() == QEvent::StyleChange
#ifdef Q_OS_DARWIN
      || ev->type() == QEvent::MacSizeChange
#endif
   ) {
      setUpLayout();
   }
   QWidget::changeEvent(ev);
}


/*!
    \reimp
 */
void QTabWidget::keyPressEvent(QKeyEvent *e)
{
   Q_D(QTabWidget);
   if (((e->key() == Qt::Key_Tab || e->key() == Qt::Key_Backtab) &&
         count() > 1 && e->modifiers() & Qt::ControlModifier)
#ifdef QT_KEYPAD_NAVIGATION
      || QApplication::keypadNavigationEnabled() && (e->key() == Qt::Key_Left || e->key() == Qt::Key_Right) && count() > 1
#endif
   ) {
      int pageCount = d->tabs->count();
      int page = currentIndex();
      int dx = (e->key() == Qt::Key_Backtab || e->modifiers() & Qt::ShiftModifier) ? -1 : 1;

#ifdef QT_KEYPAD_NAVIGATION
      if (QApplication::keypadNavigationEnabled() && (e->key() == Qt::Key_Left || e->key() == Qt::Key_Right)) {
         dx = e->key() == (isRightToLeft() ? Qt::Key_Right : Qt::Key_Left) ? -1 : 1;
      }
#endif

      for (int pass = 0; pass < pageCount; ++pass) {
         page += dx;
         if (page < 0
#ifdef QT_KEYPAD_NAVIGATION
            && !e->isAutoRepeat()
#endif
         ) {
            page = count() - 1;
         } else if (page >= pageCount
#ifdef QT_KEYPAD_NAVIGATION
            && !e->isAutoRepeat()
#endif
         ) {
            page = 0;
         }
         if (d->tabs->isTabEnabled(page)) {
            setCurrentIndex(page);
            break;
         }
      }

      if (!QApplication::focusWidget()) {
         d->tabs->setFocus();
      }

   } else {
      e->ignore();
   }
}

QWidget *QTabWidget::widget(int index) const
{
   Q_D(const QTabWidget);
   return d->stack->widget(index);
}


int QTabWidget::count() const
{
   Q_D(const QTabWidget);
   return d->tabs->count();
}

#ifndef QT_NO_TOOLTIP
/*!
    Sets the tab tool tip for the page at position \a index to \a tip.

    \sa  tabToolTip()
*/
void QTabWidget::setTabToolTip(int index, const QString &tip)
{
   Q_D(QTabWidget);
   d->tabs->setTabToolTip(index, tip);
}

/*!
    Returns the tab tool tip for the page at position \a index or
    an empty string if no tool tip has been set.

    \sa setTabToolTip()
*/
QString QTabWidget::tabToolTip(int index) const
{
   Q_D(const QTabWidget);
   return d->tabs->tabToolTip(index);
}
#endif // QT_NO_TOOLTIP

#ifndef QT_NO_WHATSTHIS
void QTabWidget::setTabWhatsThis(int index, const QString &text)
{
   Q_D(QTabWidget);
   d->tabs->setTabWhatsThis(index, text);
}


QString QTabWidget::tabWhatsThis(int index) const
{
   Q_D(const QTabWidget);
   return d->tabs->tabWhatsThis(index);
}
#endif // QT_NO_WHATSTHIS

void QTabWidget::tabInserted(int index)
{

}

void QTabWidget::tabRemoved(int index)
{

}

/*!
    \fn void QTabWidget::paintEvent(QPaintEvent *event)

    Paints the tab widget's tab bar in response to the paint \a event.
*/
void QTabWidget::paintEvent(QPaintEvent *)
{
   Q_D(QTabWidget);
   if (documentMode()) {
      QStylePainter p(this, tabBar());
      if (QWidget *w = cornerWidget(Qt::TopLeftCorner)) {
         QStyleOptionTabBarBase opt;
         QTabBarPrivate::initStyleBaseOption(&opt, tabBar(), w->size());
         opt.rect.moveLeft(w->x() + opt.rect.x());
         opt.rect.moveTop(w->y() + opt.rect.y());
         p.drawPrimitive(QStyle::PE_FrameTabBarBase, opt);
      }
      if (QWidget *w = cornerWidget(Qt::TopRightCorner)) {
         QStyleOptionTabBarBase opt;
         QTabBarPrivate::initStyleBaseOption(&opt, tabBar(), w->size());
         opt.rect.moveLeft(w->x() + opt.rect.x());
         opt.rect.moveTop(w->y() + opt.rect.y());
         p.drawPrimitive(QStyle::PE_FrameTabBarBase, opt);
      }
      return;
   }
   QStylePainter p(this);

   QStyleOptionTabWidgetFrame opt;
   initStyleOption(&opt);
   opt.rect = d->panelRect;
   p.drawPrimitive(QStyle::PE_FrameTabWidget, opt);
}

/*!
    \property QTabWidget::iconSize
    \brief The size for icons in the tab bar
    \since 4.2

    The default value is style-dependent. This is the maximum size
    that the icons will have. Icons are not scaled up if they are of
    smaller size.

    \sa QTabBar::iconSize
*/
QSize QTabWidget::iconSize() const
{
   return d_func()->tabs->iconSize();
}

void QTabWidget::setIconSize(const QSize &size)
{
   d_func()->tabs->setIconSize(size);
}

Qt::TextElideMode QTabWidget::elideMode() const
{
   return d_func()->tabs->elideMode();
}

void QTabWidget::setElideMode(Qt::TextElideMode mode)
{
   d_func()->tabs->setElideMode(mode);
}

bool QTabWidget::usesScrollButtons() const
{
   return d_func()->tabs->usesScrollButtons();
}

void QTabWidget::setUsesScrollButtons(bool useButtons)
{
   d_func()->tabs->setUsesScrollButtons(useButtons);
}

bool QTabWidget::documentMode() const
{
   Q_D(const QTabWidget);
   return d->tabs->documentMode();
}

void QTabWidget::setDocumentMode(bool enabled)
{
   Q_D(QTabWidget);
   d->tabs->setDocumentMode(enabled);
   d->tabs->setExpanding(!enabled);
   d->tabs->setDrawBase(enabled);
   setUpLayout();
}

bool QTabWidget::tabBarAutoHide() const
{
   Q_D(const QTabWidget);
   return d->tabs->autoHide();
}

void QTabWidget::setTabBarAutoHide(bool enabled)
{
   Q_D(QTabWidget);
   return d->tabs->setAutoHide(enabled);
}

void QTabWidget::clear()
{
   // ### optimize by introduce QStackedLayout::clear()
   while (count()) {
      removeTab(0);
   }
}

void QTabWidget::_q_showTab(int un_named_arg1)
{
   Q_D(QTabWidget);
   d->_q_showTab(un_named_arg1);
}

void QTabWidget::_q_removeTab(int un_named_arg1)
{
   Q_D(QTabWidget);
   d->_q_removeTab(un_named_arg1);
}

void QTabWidget::_q_tabMoved(int un_named_arg1, int un_named_arg2)
{
   Q_D(QTabWidget);
   d->_q_tabMoved(un_named_arg1, un_named_arg2);
}


#endif //QT_NO_TABWIDGET
