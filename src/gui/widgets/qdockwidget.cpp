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

#include <qdockwidget.h>

#ifndef QT_NO_DOCKWIDGET

#include <qalgorithms.h>
#include <qaction.h>
#include <qapplication.h>
#include <qdebug.h>
#include <qdesktopwidget.h>
#include <qdrawutil.h>
#include <qevent.h>
#include <qfontmetrics.h>
#include <qmainwindow.h>
#include <qrubberband.h>
#include <qstylepainter.h>
#include <qscreen.h>
#include <qtoolbutton.h>
#include <qwindow.h>

#include <qwidgetresizehandler_p.h>
#include <qdockwidget_p.h>
#include <qmainwindowlayout_p.h>

extern QString cs_internal_parseWindowTitle(const QString &, const QWidget *); // qwidget.cpp

// qmainwindow.cpp
extern QMainWindowLayout *qt_mainwindow_layout(const QMainWindow *window);

static inline QMainWindowLayout *qt_mainwindow_layout_from_dock(const QDockWidget *dock)
{
   const QWidget *p = dock->parentWidget();

   while (p) {
      const QMainWindow *window = qobject_cast<const QMainWindow *>(p);

      if (window) {
         return qt_mainwindow_layout(window);
      }

      p = p->parentWidget();
   }

   return nullptr;
}

static inline bool hasFeature(const QDockWidgetPrivate *priv, QDockWidget::DockWidgetFeature feature)
{
   return (priv->features & feature) == feature;
}

static inline bool hasFeature(const QDockWidget *dockwidget, QDockWidget::DockWidgetFeature feature)
{
   return (dockwidget->features() & feature) == feature;
}

/*
    A Dock Window:

    [+] is the float button
    [X] is the close button

    +-------------------------------+
    | Dock Window Title       [+][X]|
    +-------------------------------+
    |                               |
    | place to put the single       |
    | QDockWidget child (this space |
    | does not yet have a name)     |
    |                               |
    |                               |
    |                               |
    |                               |
    |                               |
    |                               |
    |                               |
    |                               |
    |                               |
    +-------------------------------+

*/

class QDockWidgetTitleButton : public QAbstractButton
{
   GUI_CS_OBJECT(QDockWidgetTitleButton)

 public:
   QDockWidgetTitleButton(QDockWidget *dockWidget);

   QSize sizeHint() const override;
   QSize minimumSizeHint() const override {
      return sizeHint();
   }

   void enterEvent(QEvent *event) override;
   void leaveEvent(QEvent *event) override;
   void paintEvent(QPaintEvent *event) override;
};

QDockWidgetTitleButton::QDockWidgetTitleButton(QDockWidget *dockWidget)
   : QAbstractButton(dockWidget)
{
   setFocusPolicy(Qt::NoFocus);
}

QSize QDockWidgetTitleButton::sizeHint() const
{
   ensurePolished();

   int size = 2 * style()->pixelMetric(QStyle::PM_DockWidgetTitleBarButtonMargin, nullptr, this);

   if (!icon().isNull()) {
      int iconSize = style()->pixelMetric(QStyle::PM_SmallIconSize, nullptr, this);
      QSize sz = icon().actualSize(QSize(iconSize, iconSize));
      size += qMax(sz.width(), sz.height());
   }

   return QSize(size, size);
}

void QDockWidgetTitleButton::enterEvent(QEvent *event)
{
   if (isEnabled()) {
      update();
   }
   QAbstractButton::enterEvent(event);
}

void QDockWidgetTitleButton::leaveEvent(QEvent *event)
{
   if (isEnabled()) {
      update();
   }
   QAbstractButton::leaveEvent(event);
}

void QDockWidgetTitleButton::paintEvent(QPaintEvent *)
{
   QPainter p(this);

   QStyleOptionToolButton opt;
   opt.initFrom(this);
   opt.state |= QStyle::State_AutoRaise;

   if (style()->styleHint(QStyle::SH_DockWidget_ButtonsHaveFrame, nullptr, this)) {
      if (isEnabled() && underMouse() && !isChecked() && !isDown()) {
         opt.state |= QStyle::State_Raised;
      }
      if (isChecked()) {
         opt.state |= QStyle::State_On;
      }
      if (isDown()) {
         opt.state |= QStyle::State_Sunken;
      }
      style()->drawPrimitive(QStyle::PE_PanelButtonTool, &opt, &p, this);
   }

   opt.icon = icon();
   opt.subControls = Qt::EmptyFlag;
   opt.activeSubControls = Qt::EmptyFlag;
   opt.features = QStyleOptionToolButton::None;
   opt.arrowType = Qt::NoArrow;
   int size = style()->pixelMetric(QStyle::PM_SmallIconSize, nullptr, this);
   opt.iconSize = QSize(size, size);
   style()->drawComplexControl(QStyle::CC_ToolButton, &opt, &p, this);
}

QDockWidgetLayout::QDockWidgetLayout(QWidget *parent)
   : QLayout(parent), verticalTitleBar(false), item_list(RoleCount, nullptr)
{
}

QDockWidgetLayout::~QDockWidgetLayout()
{
   qDeleteAll(item_list);
}

bool QDockWidgetLayout::nativeWindowDeco() const
{
   bool floating = parentWidget()->isWindow();

   if (!floating && qobject_cast<QDockWidgetGroupWindow *>(parentWidget()->parentWidget())) {
      return wmSupportsNativeWindowDeco();
   }

   return nativeWindowDeco(floating);
}

bool QDockWidgetLayout::wmSupportsNativeWindowDeco()
{
   static const bool xcb = !QGuiApplication::platformName().compare(QLatin1String("xcb"), Qt::CaseInsensitive);
   return ! xcb;
}

bool QDockWidgetLayout::nativeWindowDeco(bool floating) const
{
   return wmSupportsNativeWindowDeco() && floating && item_list.at(QDockWidgetLayout::TitleBar) == nullptr;
}

void QDockWidgetLayout::addItem(QLayoutItem *)
{
   qWarning("QDockWidgetLayout::addItem(): Use QDockWidgetLayout::setWidget()");
   return;
}

QLayoutItem *QDockWidgetLayout::itemAt(int index) const
{
   int cnt = 0;
   for (int i = 0; i < item_list.count(); ++i) {
      QLayoutItem *item = item_list.at(i);
      if (item == nullptr) {
         continue;
      }

      if (index == cnt++) {
         return item;
      }
   }

   return nullptr;
}

QLayoutItem *QDockWidgetLayout::takeAt(int index)
{
   int j = 0;
   for (int i = 0; i < item_list.count(); ++i) {
      QLayoutItem *item = item_list.at(i);
      if (item == nullptr) {
         continue;
      }

      if (index == j) {
         item_list[i] = nullptr;
         invalidate();

         return item;
      }

      ++j;
   }

   return nullptr;
}

int QDockWidgetLayout::count() const
{
   int result = 0;

   for (int i = 0; i < item_list.count(); ++i) {
      if (item_list.at(i)) {
         ++result;
      }
   }

   return result;
}

QSize QDockWidgetLayout::sizeFromContent(const QSize &content, bool floating) const
{
   QSize result = content;

   if (verticalTitleBar) {
      result.setHeight(qMax(result.height(), minimumTitleWidth()));
      result.setWidth(qMax(content.width(), 0));

   } else {
      result.setHeight(qMax(result.height(), 0));
      result.setWidth(qMax(content.width(), minimumTitleWidth()));
   }

   QDockWidget *w = qobject_cast<QDockWidget *>(parentWidget());
   const bool nativeDeco = nativeWindowDeco(floating);

   int fw = floating && ! nativeDeco
            ? w->style()->pixelMetric(QStyle::PM_DockWidgetFrameWidth, nullptr, w) : 0;

   const int th = titleHeight();
   if (!nativeDeco) {
      if (verticalTitleBar) {
         result += QSize(th + 2 * fw, 2 * fw);
      } else {
         result += QSize(2 * fw, th + 2 * fw);
      }
   }

   result.setHeight(qMin(result.height(), (int) QWIDGETSIZE_MAX));
   result.setWidth(qMin(result.width(), (int) QWIDGETSIZE_MAX));

   if (content.width() < 0) {
      result.setWidth(-1);
   }
   if (content.height() < 0) {
      result.setHeight(-1);
   }

   int left, top, right, bottom;
   w->getContentsMargins(&left, &top, &right, &bottom);

   // we need to substract the contents margin (it will be added by the caller)
   QSize min = w->minimumSize() - QSize(left + right, top + bottom);
   QSize max = w->maximumSize() - QSize(left + right, top + bottom);

   /* A floating dockwidget will automatically get its minimumSize set to the layout's
      minimum size + deco. We're *not* interested in this, we only take minimumSize()
      into account if the user set it herself. Otherwise we end up expanding the result
      of a calculation for a non-floating dock widget to a floating dock widget's
      minimum size + window decorations. */

   uint explicitMin = 0;
   uint explicitMax = 0;

   if (w->d_func()->extra != nullptr) {
      explicitMin = w->d_func()->extra->explicitMinSize;
      explicitMax = w->d_func()->extra->explicitMaxSize;
   }

   if (!(explicitMin & Qt::Horizontal) || min.width() == 0) {
      min.setWidth(-1);
   }

   if (!(explicitMin & Qt::Vertical) || min.height() == 0) {
      min.setHeight(-1);
   }

   if (!(explicitMax & Qt::Horizontal)) {
      max.setWidth(QWIDGETSIZE_MAX);
   }

   if (!(explicitMax & Qt::Vertical)) {
      max.setHeight(QWIDGETSIZE_MAX);
   }

   return result.boundedTo(max).expandedTo(min);
}

QSize QDockWidgetLayout::sizeHint() const
{
   QDockWidget *w = qobject_cast<QDockWidget *>(parentWidget());

   QSize content(-1, -1);
   if (item_list[Content] != nullptr) {
      content = item_list[Content]->sizeHint();
   }

   return sizeFromContent(content, w->isFloating());
}

QSize QDockWidgetLayout::maximumSize() const
{
   if (item_list[Content] != nullptr) {
      const QSize content = item_list[Content]->maximumSize();
      return sizeFromContent(content, parentWidget()->isWindow());
   } else {
      return parentWidget()->maximumSize();
   }
}

QSize QDockWidgetLayout::minimumSize() const
{
   QDockWidget *w = qobject_cast<QDockWidget *>(parentWidget());

   QSize content(0, 0);

   if (item_list[Content] != nullptr) {
      content = item_list[Content]->minimumSize();
   }

   return sizeFromContent(content, w->isFloating());
}

QWidget *QDockWidgetLayout::widgetForRole(Role r) const
{
   QLayoutItem *item = item_list.at(r);
   return item == nullptr ? nullptr : item->widget();
}

QLayoutItem *QDockWidgetLayout::itemForRole(Role r) const
{
   return item_list.at(r);
}

void QDockWidgetLayout::setWidgetForRole(Role r, QWidget *w)
{
   QWidget *old = widgetForRole(r);
   if (old != nullptr) {
      old->hide();
      removeWidget(old);
   }

   if (w != nullptr) {
      addChildWidget(w);
      item_list[r] = new QWidgetItemV2(w);
      w->show();
   } else {
      item_list[r] = nullptr;
   }

   invalidate();
}

static inline int pick(bool vertical, const QSize &size)
{
   return vertical ? size.height() : size.width();
}

static inline int perp(bool vertical, const QSize &size)
{
   return vertical ? size.width() : size.height();
}

int QDockWidgetLayout::minimumTitleWidth() const
{
   QDockWidget *q = qobject_cast<QDockWidget *>(parentWidget());

   if (QWidget *title = widgetForRole(TitleBar)) {
      return pick(verticalTitleBar, title->minimumSizeHint());
   }

   QSize closeSize(0, 0);
   QSize floatSize(0, 0);
   if (hasFeature(q, QDockWidget::DockWidgetClosable)) {
      if (QLayoutItem *item = item_list[CloseButton]) {
         closeSize = item->widget()->sizeHint();
      }
   }
   if (hasFeature(q, QDockWidget::DockWidgetFloatable)) {
      if (QLayoutItem *item = item_list[FloatButton]) {
         floatSize = item->widget()->sizeHint();
      }
   }

   int titleHeight = this->titleHeight();

   int mw = q->style()->pixelMetric(QStyle::PM_DockWidgetTitleMargin, nullptr, q);
   int fw = q->style()->pixelMetric(QStyle::PM_DockWidgetFrameWidth, nullptr, q);

   return pick(verticalTitleBar, closeSize)
      + pick(verticalTitleBar, floatSize)
      + titleHeight + 2 * fw + 3 * mw;
}

int QDockWidgetLayout::titleHeight() const
{
   QDockWidget *q = qobject_cast<QDockWidget *>(parentWidget());

   if (QWidget *title = widgetForRole(TitleBar)) {
      return perp(verticalTitleBar, title->sizeHint());
   }

   QSize closeSize(0, 0);
   QSize floatSize(0, 0);
   if (QLayoutItem *item = item_list[CloseButton]) {
      closeSize = item->widget()->sizeHint();
   }
   if (QLayoutItem *item = item_list[FloatButton]) {
      floatSize = item->widget()->sizeHint();
   }

   int buttonHeight = qMax(perp(verticalTitleBar, closeSize),
         perp(verticalTitleBar, floatSize));

   QFontMetrics titleFontMetrics = q->fontMetrics();

   int mw = q->style()->pixelMetric(QStyle::PM_DockWidgetTitleMargin, nullptr, q);

   return qMax(buttonHeight + 2, titleFontMetrics.height() + 2 * mw);
}

void QDockWidgetLayout::setGeometry(const QRect &geometry)
{
   QDockWidget *q = qobject_cast<QDockWidget *>(parentWidget());

   bool nativeDeco = nativeWindowDeco();

   int fw = q->isFloating() && !nativeDeco
      ? q->style()->pixelMetric(QStyle::PM_DockWidgetFrameWidth, nullptr, q)
      : 0;

   if (nativeDeco) {
      if (QLayoutItem *item = item_list[Content]) {
         item->setGeometry(geometry);
      }
   } else {
      int titleHeight = this->titleHeight();

      if (verticalTitleBar) {
         _titleArea = QRect(QPoint(fw, fw),
               QSize(titleHeight, geometry.height() - (fw * 2)));
      } else {
         _titleArea = QRect(QPoint(fw, fw),
               QSize(geometry.width() - (fw * 2), titleHeight));
      }

      if (QLayoutItem *item = item_list[TitleBar]) {
         item->setGeometry(_titleArea);

      } else {
         QStyleOptionDockWidget opt;
         q->initStyleOption(&opt);

         if (QLayoutItem *item = item_list[CloseButton]) {
            if (!item->isEmpty()) {
               QRect r = q->style()->subElementRect(QStyle::SE_DockWidgetCloseButton, &opt, q);

               if (!r.isNull()) {
                  item->setGeometry(r);
               }
            }
         }

         if (QLayoutItem *item = item_list[FloatButton]) {
            if (!item->isEmpty()) {
               QRect r = q->style()->subElementRect(QStyle::SE_DockWidgetFloatButton,&opt, q);
               if (!r.isNull()) {
                  item->setGeometry(r);
               }
            }
         }
      }

      if (QLayoutItem *item = item_list[Content]) {
         QRect r = geometry;
         if (verticalTitleBar) {
            r.setLeft(_titleArea.right() + 1);
            r.adjust(0, fw, -fw, -fw);
         } else {
            r.setTop(_titleArea.bottom() + 1);
            r.adjust(fw, 0, -fw, -fw);
         }
         item->setGeometry(r);
      }
   }
}

void QDockWidgetLayout::setVerticalTitleBar(bool b)
{
   if (b == verticalTitleBar) {
      return;
   }
   verticalTitleBar = b;
   invalidate();
   parentWidget()->update();
}

QDockWidgetItem::QDockWidgetItem(QDockWidget *dockWidget)
   : QWidgetItem(dockWidget)
{
}

QSize QDockWidgetItem::minimumSize() const
{
   QSize widgetMin(0, 0);
   if (QLayoutItem *item = dockWidgetChildItem()) {
      widgetMin = item->minimumSize();
   }
   return dockWidgetLayout()->sizeFromContent(widgetMin, false);
}

QSize QDockWidgetItem::maximumSize() const
{
   if (QLayoutItem *item = dockWidgetChildItem()) {
      return dockWidgetLayout()->sizeFromContent(item->maximumSize(), false);
   } else {
      return QSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX);
   }
}

QSize QDockWidgetItem::sizeHint() const
{
   if (QLayoutItem *item = dockWidgetChildItem()) {
      return dockWidgetLayout()->sizeFromContent(item->sizeHint(), false);
   } else {
      return QWidgetItem::sizeHint();
   }
}

void QDockWidgetPrivate::init()
{
   Q_Q(QDockWidget);

   QDockWidgetLayout *layout = new QDockWidgetLayout(q);
   layout->setSizeConstraint(QLayout::SetMinAndMaxSize);

   QAbstractButton *button1 = new QDockWidgetTitleButton(q);
   button1->setObjectName("qt_dockwidget_floatbutton");
   layout->setWidgetForRole(QDockWidgetLayout::FloatButton, button1);

   QAbstractButton *button2 = new QDockWidgetTitleButton(q);
   button2->setObjectName("qt_dockwidget_closebutton");
   layout->setWidgetForRole(QDockWidgetLayout::CloseButton, button2);

   QObject::connect(button1, &QAbstractButton::clicked, q, &QDockWidget::_q_toggleTopLevel);
   QObject::connect(button2, &QAbstractButton::clicked, q, &QDockWidget::close);

#ifndef QT_NO_ACTION
   toggleViewAction = new QAction(q);
   toggleViewAction->setCheckable(true);

   fixedWindowTitle = cs_internal_parseWindowTitle(q->windowTitle(), q);
   toggleViewAction->setText(fixedWindowTitle);

   QObject::connect(toggleViewAction, &QAction::triggered, q, &QDockWidget::_q_toggleView);
#endif

   updateButtons();
}

void QDockWidget::initStyleOption(QStyleOptionDockWidget *option) const
{
   Q_D(const QDockWidget);

   if (! option) {
      return;
   }

   QDockWidgetLayout *dwlayout = qobject_cast<QDockWidgetLayout *>(layout());

   QDockWidgetGroupWindow *floatingTab = qobject_cast<QDockWidgetGroupWindow *>(parent());

   // If we are in a floating tab, init from the parent because the attributes and the geometry
   // of the title bar should be taken from the floating window.
   option->initFrom(floatingTab && !isFloating() ? parentWidget() : this);
   option->rect  = dwlayout->titleArea();
   option->title = d->fixedWindowTitle;

   option->closable  = hasFeature(this, QDockWidget::DockWidgetClosable);
   option->movable   = hasFeature(this, QDockWidget::DockWidgetMovable);
   option->floatable = hasFeature(this, QDockWidget::DockWidgetFloatable);

   QDockWidgetLayout *l = qobject_cast<QDockWidgetLayout *>(layout());
   option->verticalTitleBar = l->verticalTitleBar;
}

void QDockWidgetPrivate::_q_toggleView(bool b)
{
   Q_Q(QDockWidget);

   if (b == q->isHidden()) {
      if (b) {
         q->show();
      } else {
         q->close();
      }
   }
}

void QDockWidgetPrivate::updateButtons()
{
   Q_Q(QDockWidget);
   QDockWidgetLayout *dwLayout = qobject_cast<QDockWidgetLayout *>(layout);

   QStyleOptionDockWidget opt;
   q->initStyleOption(&opt);

   bool customTitleBar = dwLayout->widgetForRole(QDockWidgetLayout::TitleBar) != nullptr;
   bool nativeDeco = dwLayout->nativeWindowDeco();
   bool hideButtons = nativeDeco || customTitleBar;

   bool canClose = hasFeature(this, QDockWidget::DockWidgetClosable);
   bool canFloat = hasFeature(this, QDockWidget::DockWidgetFloatable);

   QAbstractButton *button
      = qobject_cast<QAbstractButton *>(dwLayout->widgetForRole(QDockWidgetLayout::FloatButton));
   button->setIcon(q->style()->standardIcon(QStyle::SP_TitleBarNormalButton, &opt, q));
   button->setVisible(canFloat && !hideButtons);

#ifndef QT_NO_ACCESSIBILITY
   //: Accessible name for button undocking a dock widget (floating state)
   button->setAccessibleName(QDockWidget::tr("Float"));
   button->setAccessibleDescription(QDockWidget::tr("Undocks and re-attaches the dock widget"));
#endif

   button = qobject_cast <QAbstractButton *>(dwLayout->widgetForRole(QDockWidgetLayout::CloseButton));
   button->setIcon(q->style()->standardIcon(QStyle::SP_TitleBarCloseButton, &opt, q));
   button->setVisible(canClose && !hideButtons);

#ifndef QT_NO_ACCESSIBILITY
   //: Accessible name for button closing a dock widget
   button->setAccessibleName(QDockWidget::tr("Close"));
   button->setAccessibleDescription(QDockWidget::tr("Closes the dock widget"));
#endif

   q->setAttribute(Qt::WA_ContentsPropagated, (canFloat || canClose) && !hideButtons);
   layout->invalidate();
}

void QDockWidgetPrivate::_q_toggleTopLevel()
{
   Q_Q(QDockWidget);
   q->setFloating(!q->isFloating());
}

void QDockWidgetPrivate::initDrag(const QPoint &pos, bool nca)
{
   Q_Q(QDockWidget);

   if (state != nullptr) {
      return;
   }

   QMainWindowLayout *layout = qt_mainwindow_layout_from_dock(q);
   Q_ASSERT(layout != nullptr);

   if (layout->pluggingWidget != nullptr) {
      // the main window is animating a docking operation
      return;
   }

   state = new QDockWidgetPrivate::DragState;
   state->pressPos = pos;
   state->dragging = false;
   state->widgetItem = nullptr;
   state->ownWidgetItem = false;
   state->nca = nca;
   state->ctrlDrag = false;
}

void QDockWidgetPrivate::startDrag(bool group)
{
   Q_Q(QDockWidget);

   if (state == nullptr || state->dragging) {
      return;
   }

   QMainWindowLayout *layout = qt_mainwindow_layout_from_dock(q);
   Q_ASSERT(layout != nullptr);

   state->widgetItem = layout->unplug(q, group);

   if (state->widgetItem == nullptr) {

      QDockWidgetGroupWindow *floatingTab = qobject_cast<QDockWidgetGroupWindow *>(q->parent());

      if (floatingTab && !q->isFloating()) {
         state->widgetItem = new QDockWidgetGroupWindowItem(floatingTab);
      } else {
         state->widgetItem = new QDockWidgetItem(q);
      }

      state->ownWidgetItem = true;
   }

   if (state->ctrlDrag) {
      layout->restore();
   }

   state->dragging = true;
}

void QDockWidgetPrivate::endDrag(bool abort)
{
   Q_Q(QDockWidget);
   Q_ASSERT(state != nullptr);

   q->releaseMouse();

   if (state->dragging) {
      QMainWindowLayout *mwLayout = qt_mainwindow_layout_from_dock(q);
      Q_ASSERT(mwLayout != nullptr);

      if (abort || !mwLayout->plug(state->widgetItem)) {
         if (hasFeature(this, QDockWidget::DockWidgetFloatable)) {
            if (state->ownWidgetItem) {
               delete state->widgetItem;
               state->widgetItem = nullptr;
            }

            mwLayout->restore();

            QDockWidgetLayout *dwLayout = qobject_cast<QDockWidgetLayout *>(layout);

            if (!dwLayout->nativeWindowDeco()) {
               // get rid of the X11BypassWindowManager window flag and activate the resizer
               Qt::WindowFlags flags = q->windowFlags();
               flags &= ~Qt::X11BypassWindowManagerHint;
               q->setWindowFlags(flags);
               setResizerActive(q->isFloating());
               q->show();
            } else {
               setResizerActive(false);
            }
            if (q->isFloating()) { // Might not be floating when dragging a QDockWidgetGroupWindow
               undockedGeometry = q->geometry();
            }
            q->activateWindow();
         } else {
            mwLayout->revert(state->widgetItem);
         }
      }
   }

   delete state;
   state = nullptr;
}

void QDockWidgetPrivate::setResizerActive(bool active)
{
   Q_Q(QDockWidget);
   if (active && !resizer) {
      resizer = new QWidgetResizeHandler(q);
      resizer->setMovingEnabled(false);
   }
   if (resizer) {
      resizer->setActive(QWidgetResizeHandler::Resize, active);
   }
}

bool QDockWidgetPrivate::isAnimating() const
{
   Q_Q(const QDockWidget);

   QMainWindowLayout *mainWinLayout = qt_mainwindow_layout_from_dock(q);

   if (mainWinLayout == nullptr) {
      return false;
   }

   return (const void *)mainWinLayout->pluggingWidget == (const void *)q;
}

bool QDockWidgetPrivate::mousePressEvent(QMouseEvent *event)
{
#if !defined(QT_NO_MAINWINDOW)
   Q_Q(QDockWidget);

   QDockWidgetLayout *dwLayout = qobject_cast<QDockWidgetLayout *>(layout);

   if (!dwLayout->nativeWindowDeco()) {
      QRect titleArea = dwLayout->titleArea();

      QDockWidgetGroupWindow *floatingTab = qobject_cast<QDockWidgetGroupWindow *>(q->parent());

      // check if the tool window is movable... do nothing if it
      // is not (but allow moving if the window is floating)

      if (event->button() != Qt::LeftButton ||
            ! titleArea.contains(event->pos()) ||
            (! hasFeature(this, QDockWidget::DockWidgetMovable) && ! q->isFloating()) ||
            (qobject_cast<QMainWindow *>(q->parent()) == nullptr && ! floatingTab) || isAnimating() || state != nullptr) {
         return false;
      }

      initDrag(event->pos(), false);

      if (state) {
         state->ctrlDrag = hasFeature(this, QDockWidget::DockWidgetFloatable) && event->modifiers() & Qt::ControlModifier;
      }

      return true;
   }

#endif // !defined(QT_NO_MAINWINDOW)
   return false;
}

bool QDockWidgetPrivate::mouseDoubleClickEvent(QMouseEvent *event)
{
   QDockWidgetLayout *dwLayout = qobject_cast<QDockWidgetLayout *>(layout);

   if (!dwLayout->nativeWindowDeco()) {
      QRect titleArea = dwLayout->titleArea();

      if (event->button() == Qt::LeftButton && titleArea.contains(event->pos()) &&
         hasFeature(this, QDockWidget::DockWidgetFloatable)) {
         _q_toggleTopLevel();
         return true;
      }
   }
   return false;
}

bool QDockWidgetPrivate::mouseMoveEvent(QMouseEvent *event)
{
   bool ret = false;

#if ! defined(QT_NO_MAINWINDOW)
   Q_Q(QDockWidget);

   if (!state) {
      return ret;
   }

   QDockWidgetLayout *dwlayout = qobject_cast<QDockWidgetLayout *>(layout);
   QMainWindowLayout *mwlayout = qt_mainwindow_layout_from_dock(q);

   if (! dwlayout->nativeWindowDeco()) {
      if (! state->dragging && mwlayout->pluggingWidget == nullptr
         && (event->pos() - state->pressPos).manhattanLength() > QApplication::startDragDistance()) {
         startDrag();


         q->grabMouse();

         ret = true;
      }
   }

   if (state->dragging && !state->nca) {
      QPoint pos = event->globalPos() - state->pressPos;
      QDockWidgetGroupWindow *floatingTab = qobject_cast<QDockWidgetGroupWindow *>(q->parent());

      if (floatingTab && !q->isFloating()) {
         floatingTab->move(pos);
      } else {
         q->move(pos);
      }

      if (state && !state->ctrlDrag) {
         mwlayout->hover(state->widgetItem, event->globalPos());
      }

      ret = true;
   }
#endif // !defined(QT_NO_MAINWINDOW)

   return ret;
}

bool QDockWidgetPrivate::mouseReleaseEvent(QMouseEvent *event)
{
#if !defined(QT_NO_MAINWINDOW)

   if (event->button() == Qt::LeftButton && state && !state->nca) {
      endDrag();
      return true; //filter out the event
   }

#endif
   return false;
}

void QDockWidgetPrivate::nonClientAreaMouseEvent(QMouseEvent *event)
{
   Q_Q(QDockWidget);

   int fw = q->style()->pixelMetric(QStyle::PM_DockWidgetFrameWidth, nullptr, q);

   QWidget *tl     = q->topLevelWidget();
   QRect geo       = tl->geometry();
   QRect titleRect = tl->frameGeometry();

   titleRect.setLeft(geo.left());
   titleRect.setRight(geo.right());
   titleRect.setBottom(geo.top() - 1);
   titleRect.adjust(0, fw, 0, 0);

   switch (event->type()) {
      case QEvent::NonClientAreaMouseButtonPress:
         if (!titleRect.contains(event->globalPos())) {
            break;
         }

         if (state != nullptr) {
            break;
         }

         if (qobject_cast<QMainWindow *>(q->parent()) == nullptr &&
                  qobject_cast<QDockWidgetGroupWindow *>(q->parent()) == nullptr) {
            break;
         }

         if (isAnimating()) {
            break;
         }

         initDrag(event->pos(), true);
         if (state == nullptr) {
            break;
         }

         state->ctrlDrag = event->modifiers() & Qt::ControlModifier;

         startDrag();
         break;

      case QEvent::NonClientAreaMouseMove:
         if (state == nullptr || ! state->dragging) {
            break;
         }

#ifndef Q_OS_DARWIN
         if (state->nca) {
            endDrag();
         }
#endif
         break;
      case QEvent::NonClientAreaMouseButtonRelease:

#ifdef Q_OS_DARWIN
         if (state) {
            endDrag();
         }
#endif
         break;

      case QEvent::NonClientAreaMouseButtonDblClick:
         _q_toggleTopLevel();
         break;

      default:
         break;
   }
}

void QDockWidgetPrivate::moveEvent(QMoveEvent *event)
{
   Q_Q(QDockWidget);

   if (state == nullptr || !state->dragging || !state->nca) {
      return;
   }

   if (! q->isWindow() && qobject_cast<QDockWidgetGroupWindow *>(q->parent()) == nullptr) {
      return;
   }

   // When the native window frame is being dragged, all we get is these mouse  move events
   if (state->ctrlDrag) {
      return;
   }

   QMainWindowLayout *layout = qt_mainwindow_layout_from_dock(q);
   Q_ASSERT(layout != nullptr);

   QPoint globalMousePos = event->pos() + state->pressPos;
   layout->hover(state->widgetItem, globalMousePos);
}

void QDockWidgetPrivate::unplug(const QRect &rect)
{
   Q_Q(QDockWidget);
   QRect r = rect;

   r.moveTopLeft(q->mapToGlobal(QPoint(0, 0)));
   QDockWidgetLayout *dwLayout = qobject_cast<QDockWidgetLayout *>(layout);

   if (dwLayout->nativeWindowDeco(true)) {
      r.adjust(0, dwLayout->titleHeight(), 0, 0);
   }

   setWindowState(true, true, r);
}

void QDockWidgetPrivate::plug(const QRect &rect)
{
   setWindowState(false, false, rect);
}

void QDockWidgetPrivate::setWindowState(bool floating, bool unplug, const QRect &rect)
{
   Q_Q(QDockWidget);

   if (! floating && q->parent()) {
      QMainWindowLayout *mwlayout = qt_mainwindow_layout_from_dock(q);

      if (mwlayout && mwlayout->dockWidgetArea(q) == Qt::NoDockWidgetArea
         && ! qobject_cast<QDockWidgetGroupWindow *>(q->parent())) {
         return;   // this dockwidget can't be redocked
      }
   }

   bool wasFloating = q->isFloating();

   if (wasFloating) { // Prevent repetitive unplugging from nested invocations (QTBUG-42818)
      unplug = false;
   }

   bool hidden = q->isHidden();

   if (q->isVisible()) {
      q->hide();
   }

   Qt::WindowFlags flags = floating ? Qt::Tool : Qt::Widget;

   QDockWidgetLayout *dwLayout = qobject_cast<QDockWidgetLayout *>(layout);
   const bool nativeDeco = dwLayout->nativeWindowDeco(floating);

   if (nativeDeco) {
      flags |= Qt::CustomizeWindowHint | Qt::WindowTitleHint;
      if (hasFeature(this, QDockWidget::DockWidgetClosable)) {
         flags |= Qt::WindowCloseButtonHint;
      }
   } else {
      flags |= Qt::FramelessWindowHint;
   }

   if (unplug) {
      flags |= Qt::X11BypassWindowManagerHint;
   }

   q->setWindowFlags(flags);

   if (!rect.isNull()) {
      q->setGeometry(rect);
   }

   updateButtons();

   if (! hidden) {
      q->show();
   }

   if (floating != wasFloating) {
      emit q->topLevelChanged(floating);

      if (! floating && q->parent()) {
         QMainWindowLayout *mwlayout = qt_mainwindow_layout_from_dock(q);

         if (mwlayout) {
            emit q->dockLocationChanged(mwlayout->dockWidgetArea(q));
         }
      }
   }

   setResizerActive(!unplug && floating && !nativeDeco);
}

QDockWidget::QDockWidget(QWidget *parent, Qt::WindowFlags flags)
   : QWidget(*new QDockWidgetPrivate, parent, flags)
{
   Q_D(QDockWidget);
   d->init();
}

QDockWidget::QDockWidget(const QString &title, QWidget *parent, Qt::WindowFlags flags)
   : QWidget(*new QDockWidgetPrivate, parent, flags)
{
   Q_D(QDockWidget);
   d->init();
   setWindowTitle(title);
}

QDockWidget::~QDockWidget()
{
}

QWidget *QDockWidget::widget() const
{
   QDockWidgetLayout *layout = qobject_cast<QDockWidgetLayout *>(this->layout());
   return layout->widgetForRole(QDockWidgetLayout::Content);
}

void QDockWidget::setWidget(QWidget *widget)
{
   QDockWidgetLayout *layout = qobject_cast<QDockWidgetLayout *>(this->layout());
   layout->setWidgetForRole(QDockWidgetLayout::Content, widget);
}

void QDockWidget::setFeatures(QDockWidget::DockWidgetFeatures features)
{
   Q_D(QDockWidget);

   features &= DockWidgetFeatureMask;
   if (d->features == features) {
      return;
   }

   const bool closableChanged = (d->features ^ features) & DockWidgetClosable;
   d->features = features;
   QDockWidgetLayout *layout
      = qobject_cast<QDockWidgetLayout *>(this->layout());
   layout->setVerticalTitleBar(features & DockWidgetVerticalTitleBar);
   d->updateButtons();
   d->toggleViewAction->setEnabled((d->features & DockWidgetClosable) == DockWidgetClosable);
   emit featuresChanged(d->features);
   update();

   if (closableChanged && layout->nativeWindowDeco()) {
      QDockWidgetGroupWindow *floatingTab = qobject_cast<QDockWidgetGroupWindow *>(parent());

      if (floatingTab && !isFloating()) {
         floatingTab->adjustFlags();
      } else {
         d->setWindowState(true /*floating*/, true /*unplug*/);
      }
   }
}

QDockWidget::DockWidgetFeatures QDockWidget::features() const
{
   Q_D(const QDockWidget);
   return d->features;
}


void QDockWidget::setFloating(bool floating)
{
   Q_D(QDockWidget);

   // the initial click of a double-click may have started a drag...
   if (d->state != nullptr) {
      d->endDrag(true);
   }

   QRect r = d->undockedGeometry;
   // Keep position when undocking for the first time.
   if (floating && isVisible() && !r.isValid()) {
      r = QRect(mapToGlobal(QPoint(0, 0)), size());
   }

   d->setWindowState(floating, false, floating ? r : QRect());

   if (floating && r.isNull()) {
      if (x() < 0 || y() < 0) { //may happen if we have been hidden
         move(QPoint());
      }
      setAttribute(Qt::WA_Moved, false); //we want it at the default position
   }
}

void QDockWidget::setAllowedAreas(Qt::DockWidgetAreas areas)
{
   Q_D(QDockWidget);
   areas &= Qt::DockWidgetArea_Mask;
   if (areas == d->allowedAreas) {
      return;
   }
   d->allowedAreas = areas;
   emit allowedAreasChanged(d->allowedAreas);
}

Qt::DockWidgetAreas QDockWidget::allowedAreas() const
{
   Q_D(const QDockWidget);
   return d->allowedAreas;
}

void QDockWidget::changeEvent(QEvent *event)
{
   Q_D(QDockWidget);
   QDockWidgetLayout *layout = qobject_cast<QDockWidgetLayout *>(this->layout());

   switch (event->type()) {
      case QEvent::ModifiedChange:
      case QEvent::WindowTitleChange:
         update(layout->titleArea());

#ifndef QT_NO_ACTION
         d->fixedWindowTitle = cs_internal_parseWindowTitle(windowTitle(), this);
         d->toggleViewAction->setText(d->fixedWindowTitle);
#endif

#ifndef QT_NO_TABBAR
         {
            if (QMainWindowLayout *winLayout = qt_mainwindow_layout_from_dock(this)) {
               if (QDockAreaLayoutInfo *info = winLayout->layoutState.dockAreaLayout.info(this)) {
                  info->updateTabBar();
               }
            }
         }
#endif

         break;

      default:
         break;
   }
   QWidget::changeEvent(event);
}

void QDockWidget::closeEvent(QCloseEvent *event)
{
   Q_D(QDockWidget);
   if (d->state) {
      d->endDrag(true);
   }
   QWidget::closeEvent(event);
}

void QDockWidget::paintEvent(QPaintEvent *event)
{
   (void) event;

   QDockWidgetLayout *layout = qobject_cast<QDockWidgetLayout *>(this->layout());
   bool customTitleBar = layout->widgetForRole(QDockWidgetLayout::TitleBar) != nullptr;
   bool nativeDeco = layout->nativeWindowDeco();

   if (!nativeDeco && !customTitleBar) {
      QStylePainter p(this);

      // ### Add PixelMetric to change spacers, so style may show border
      // when not floating.

      if (isFloating()) {
         QStyleOptionFrame framOpt;
         framOpt.initFrom(this);
         p.drawPrimitive(QStyle::PE_FrameDockWidget, framOpt);
      }

      // Title must be painted after the frame, since the areas overlap, and
      // the title may wish to extend out to all sides (eg. XP style)
      QStyleOptionDockWidget titleOpt;
      initStyleOption(&titleOpt);
      p.drawControl(QStyle::CE_DockWidgetTitle, titleOpt);
   }
}

bool QDockWidget::event(QEvent *event)
{
   Q_D(QDockWidget);

   QMainWindow *win = qobject_cast<QMainWindow *>(parentWidget());
   QMainWindowLayout *layout = qt_mainwindow_layout_from_dock(this);

   switch (event->type()) {

#ifndef QT_NO_ACTION
      case QEvent::Hide:
         if (layout != nullptr) {
            layout->keepSize(this);
         }

         d->toggleViewAction->setChecked(false);
         emit visibilityChanged(false);
         break;

      case QEvent::Show: {
         d->toggleViewAction->setChecked(true);
         QPoint parentTopLeft(0, 0);
         if (isWindow()) {
            if (const QWindow *window = windowHandle()) {
               parentTopLeft = window->screen()->availableVirtualGeometry().topLeft();
            } else {
               parentTopLeft = QGuiApplication::primaryScreen()->availableVirtualGeometry().topLeft();
            }
         }
         emit visibilityChanged(geometry().right() >= parentTopLeft.x() && geometry().bottom() >= parentTopLeft.y());
      }
      break;
#endif

      case QEvent::ApplicationLayoutDirectionChange:
      case QEvent::LayoutDirectionChange:
      case QEvent::StyleChange:
      case QEvent::ParentChange:
         d->updateButtons();
         break;

      case QEvent::ZOrderChange: {
         bool onTop = false;
         if (win != nullptr) {
            const QObjectList &siblings = win->children();
            onTop = siblings.count() > 0 && siblings.last() == (QObject *)this;
         }

         if (!isFloating() && layout != nullptr && onTop) {
            layout->raise(this);
         }
         break;
      }
      case QEvent::WindowActivate:
      case QEvent::WindowDeactivate:
         update(qobject_cast<QDockWidgetLayout *>(this->layout())->titleArea());
         break;

      case QEvent::ContextMenu:
         if (d->state) {
            event->accept();
            return true;
         }
         break;

      // return true after calling the handler since we don't want
      // them to be passed onto the default handlers
      case QEvent::MouseButtonPress:
         if (d->mousePressEvent(static_cast<QMouseEvent *>(event))) {
            return true;
         }
         break;
      case QEvent::MouseButtonDblClick:
         if (d->mouseDoubleClickEvent(static_cast<QMouseEvent *>(event))) {
            return true;
         }
         break;
      case QEvent::MouseMove:
         if (d->mouseMoveEvent(static_cast<QMouseEvent *>(event))) {
            return true;
         }
         break;

      case QEvent::MouseButtonRelease:
         if (d->mouseReleaseEvent(static_cast<QMouseEvent *>(event))) {
            return true;
         }
         break;

      case QEvent::NonClientAreaMouseMove:
      case QEvent::NonClientAreaMouseButtonPress:
      case QEvent::NonClientAreaMouseButtonRelease:
      case QEvent::NonClientAreaMouseButtonDblClick:
         d->nonClientAreaMouseEvent(static_cast<QMouseEvent *>(event));
         return true;

      case QEvent::Move:
         d->moveEvent(static_cast<QMoveEvent *>(event));
         break;

      case QEvent::Resize:
         // if the mainwindow is plugging us, we don't want to update undocked geometry
         if (isFloating() && layout != nullptr && layout->pluggingWidget != this) {
            d->undockedGeometry = geometry();
         }
         break;

      default:
         break;
   }
   return QWidget::event(event);
}

#ifndef QT_NO_ACTION
QAction *QDockWidget::toggleViewAction() const
{
   Q_D(const QDockWidget);
   return d->toggleViewAction;
}
#endif

void QDockWidget::setTitleBarWidget(QWidget *widget)
{
   Q_D(QDockWidget);

   QDockWidgetLayout *layout = qobject_cast<QDockWidgetLayout *>(this->layout());
   layout->setWidgetForRole(QDockWidgetLayout::TitleBar, widget);
   d->updateButtons();

   if (isWindow()) {
      // this ensures the native decoration is drawn
      d->setWindowState(true, true);
   }
}

QWidget *QDockWidget::titleBarWidget() const
{
   QDockWidgetLayout *layout
      = qobject_cast<QDockWidgetLayout *>(this->layout());
   return layout->widgetForRole(QDockWidgetLayout::TitleBar);
}

void QDockWidget::_q_toggleView(bool isToggled)
{
   Q_D(QDockWidget);
   d->_q_toggleView(isToggled);
}

void QDockWidget::_q_toggleTopLevel()
{
   Q_D(QDockWidget);
   d->_q_toggleTopLevel();
}

#endif
