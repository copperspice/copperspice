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
   void _q_showTab(int index);
   void _q_removeTab(int index);
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
   : tabs(nullptr), stack(nullptr), dirty(true),
     pos(QTabWidget::North), shape(QTabWidget::Rounded),
     leftCornerWidget(nullptr), rightCornerWidget(nullptr)
{ }

QTabWidgetPrivate::~QTabWidgetPrivate()
{
}

void QTabWidgetPrivate::init()
{
   Q_Q(QTabWidget);

   stack = new QStackedWidget(q);
   stack->setObjectName(QLatin1String("qt_tabwidget_stackedwidget"));
   stack->setLineWidth(0);

   // do this so QMacStyle::layoutSpacing() can detect tab widget pages
   stack->setSizePolicy(QSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred, QSizePolicy::TabWidget));

   QObject::connect(stack, &QStackedWidget::widgetRemoved, q, &QTabWidget::_q_removeTab);

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
            QStyle::SH_TabWidget_DefaultTabPosition, nullptr, q )));
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
   if (! option) {
      return;
   }

   Q_D(const QTabWidget);
   option->initFrom(this);

   if (documentMode()) {
      option->lineWidth = 0;
   } else {
      option->lineWidth = style()->pixelMetric(QStyle::PM_DefaultFrameWidth, nullptr, this);
   }

   int exth = style()->pixelMetric(QStyle::PM_TabBarBaseHeight, nullptr, this);
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

QTabWidget::QTabWidget(QWidget *parent)
   : QWidget(*new QTabWidgetPrivate, parent, Qt::EmptyFlag)
{
   Q_D(QTabWidget);
   d->init();
}

QTabWidget::~QTabWidget()
{
}

int QTabWidget::addTab(QWidget *child, const QString &label)
{
   return insertTab(-1, child, label);
}

int QTabWidget::addTab(QWidget *child, const QIcon &icon, const QString &label)
{
   return insertTab(-1, child, icon, label);
}

int QTabWidget::insertTab(int index, QWidget *w, const QString &label)
{
   return insertTab(index, w, QIcon(), label);
}

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

QWidget *QTabWidget::cornerWidget(Qt::Corner corner) const
{
   Q_D(const QTabWidget);
   if (corner & Qt::TopRightCorner) {
      return d->rightCornerWidget;
   }
   return d->leftCornerWidget;
}

void QTabWidget::removeTab(int index)
{
   Q_D(QTabWidget);
   if (QWidget *w = d->stack->widget(index)) {
      d->stack->removeWidget(w);
   }
}

QWidget *QTabWidget::currentWidget() const
{
   Q_D(const QTabWidget);
   return d->stack->currentWidget();
}

void QTabWidget::setCurrentWidget(QWidget *widget)
{
   Q_D(const QTabWidget);
   d->tabs->setCurrentIndex(indexOf(widget));
}

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

int QTabWidget::indexOf(QWidget *w) const
{
   Q_D(const QTabWidget);
   return d->stack->indexOf(w);
}

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

   connect(d->tabs, &QTabBar::currentChanged,      this, &QTabWidget::_q_showTab);
   connect(d->tabs, &QTabBar::tabMoved,            this, &QTabWidget::_q_tabMoved);
   connect(d->tabs, &QTabBar::tabBarClicked,       this, &QTabWidget::tabBarClicked);
   connect(d->tabs, &QTabBar::tabBarDoubleClicked, this, &QTabWidget::tabBarDoubleClicked);


   if (d->tabs->tabsClosable()) {
      connect(d->tabs, &QTabBar::tabCloseRequested, this, &QTabWidget::tabCloseRequested);
   }

   tb->setExpanding(!documentMode());
   setUpLayout();
}

QTabBar *QTabWidget::tabBar() const
{
   Q_D(const QTabWidget);
   return d->tabs;
}

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

static inline QSize basicSize(
   bool horizontal, const QSize &lc, const QSize &rc, const QSize &s, const QSize &t)
{
   return horizontal
      ? QSize(qMax(s.width(), t.width() + rc.width() + lc.width()),
         s.height() + (qMax(rc.height(), qMax(lc.height(), t.height()))))
      : QSize(s.width() + (qMax(rc.width(), qMax(lc.width(), t.width()))),
         qMax(s.height(), t.height() + rc.height() + lc.height()));
}

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
      connect(tabBar(), &QTabBar::tabCloseRequested,    this, &QTabWidget::tabCloseRequested);
   } else {
      disconnect(tabBar(), &QTabBar::tabCloseRequested, this, &QTabWidget::tabCloseRequested);
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

bool QTabWidget::event(QEvent *ev)
{
   if (ev->type() == QEvent::LayoutRequest) {
      setUpLayout();
   }
   return QWidget::event(ev);
}

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
void QTabWidget::setTabToolTip(int index, const QString &tip)
{
   Q_D(QTabWidget);
   d->tabs->setTabToolTip(index, tip);
}

QString QTabWidget::tabToolTip(int index) const
{
   Q_D(const QTabWidget);
   return d->tabs->tabToolTip(index);
}
#endif

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
#endif

void QTabWidget::tabInserted(int index)
{
   (void) index;
}

void QTabWidget::tabRemoved(int index)
{
   (void) index;
}

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

void QTabWidget::_q_showTab(int index)
{
   Q_D(QTabWidget);
   d->_q_showTab(index);
}

void QTabWidget::_q_removeTab(int index)
{
   Q_D(QTabWidget);
   d->_q_removeTab(index);
}

void QTabWidget::_q_tabMoved(int from, int to)
{
   Q_D(QTabWidget);
   d->_q_tabMoved(from, to);
}

#endif //QT_NO_TABWIDGET
