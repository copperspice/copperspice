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

#include <qcombobox.h>

#ifndef QT_NO_COMBOBOX

#include <qstylepainter.h>
#include <qplatform_theme.h>
#include <qplatform_menu.h>
#include <qlineedit.h>
#include <qapplication.h>
#include <qdesktopwidget.h>
#include <qlistview.h>
#include <qtableview.h>
#include <qitemdelegate.h>
#include <qmap.h>
#include <qmenu.h>
#include <qevent.h>
#include <qlayout.h>
#include <qscrollbar.h>
#include <qtreeview.h>
#include <qheaderview.h>
#include <qmath.h>
#include <qmetaobject.h>
#include <qabstractproxymodel.h>
#include <qstylehints.h>
#include <qdebug.h>

#include <qguiapplication_p.h>
#include <qapplication_p.h>
#include <qcombobox_p.h>
#include <qabstractitemmodel_p.h>
#include <qabstractscrollarea_p.h>

#ifndef QT_NO_EFFECTS
# include <qeffects_p.h>
#endif

#ifndef QT_NO_ACCESSIBILITY
#include <qaccessible.h>
#endif

QComboBoxPrivate::QComboBoxPrivate()
   : QWidgetPrivate(), model(nullptr), lineEdit(nullptr), container(nullptr),
     insertPolicy(QComboBox::InsertAtBottom),
     sizeAdjustPolicy(QComboBox::AdjustToContentsOnFirstShow), minimumContentsLength(0),
     shownOnce(false), autoCompletion(true), duplicatesEnabled(false), frame(true),
     maxVisibleItems(10), maxCount(INT_MAX), modelColumn(0), inserting(false),
     arrowState(QStyle::State_None), hoverControl(QStyle::SC_None),
     autoCompletionCaseSensitivity(Qt::CaseInsensitive), indexBeforeChange(-1)

#ifdef Q_OS_DARWIN
   , m_platformMenu(nullptr)
#endif

#ifndef QT_NO_COMPLETER
   , completer(nullptr)
#endif
{
}

QComboBoxPrivate::~QComboBoxPrivate()
{
#ifdef Q_OS_DARWIN
   cleanupNativePopup();
#endif
}

QStyleOptionMenuItem QComboMenuDelegate::getStyleOption(const QStyleOptionViewItem &option,
   const QModelIndex &index) const
{
   QStyleOptionMenuItem menuOption;

   QPalette resolvedpalette = option.palette.resolve(QApplication::palette("QMenu"));
   QVariant value = index.data(Qt::ForegroundRole);

   if (value.canConvert<QBrush>()) {
      resolvedpalette.setBrush(QPalette::WindowText, value.value<QBrush>());
      resolvedpalette.setBrush(QPalette::ButtonText, value.value<QBrush>());
      resolvedpalette.setBrush(QPalette::Text,       value.value<QBrush>());
   }

   menuOption.palette = resolvedpalette;
   menuOption.state = QStyle::State_None;

   if (mCombo->window()->isActiveWindow()) {
      menuOption.state = QStyle::State_Active;
   }

   if ((option.state & QStyle::State_Enabled) && (index.model()->flags(index) & Qt::ItemIsEnabled)) {
      menuOption.state |= QStyle::State_Enabled;
   } else {
      menuOption.palette.setCurrentColorGroup(QPalette::Disabled);
   }

   if (option.state & QStyle::State_Selected) {
      menuOption.state |= QStyle::State_Selected;
   }

   menuOption.checkType = QStyleOptionMenuItem::NonExclusive;
   menuOption.checked = mCombo->currentIndex() == index.row();

   if (QComboBoxDelegate::isSeparator(index)) {
      menuOption.menuItemType = QStyleOptionMenuItem::Separator;
   } else {
      menuOption.menuItemType = QStyleOptionMenuItem::Normal;
   }

   QVariant variant = index.model()->data(index, Qt::DecorationRole);
   switch (variant.type()) {
      case QVariant::Icon:
         menuOption.icon = variant.value<QIcon>();
         break;

      case QVariant::Color: {
         static QPixmap pixmap(option.decorationSize);
         pixmap.fill(variant.value<QColor>());
         menuOption.icon = pixmap;
         break;
      }

      default:
         menuOption.icon = variant.value<QPixmap>();
         break;
   }

   if (index.data(Qt::BackgroundRole).canConvert<QBrush>()) {
      menuOption.palette.setBrush(QPalette::All, QPalette::Background, index.data(Qt::BackgroundRole).value<QBrush>());
   }

   menuOption.text = index.model()->data(index, Qt::DisplayRole).toString().replace(QChar('&'), QString("&&"));

   menuOption.tabWidth = 0;
   menuOption.maxIconWidth =  option.decorationSize.width() + 4;
   menuOption.menuRect = option.rect;
   menuOption.rect = option.rect;

   // Make sure fonts set on the combo box also overrides the font for the popup menu.
   if (mCombo->testAttribute(Qt::WA_SetFont)
      || mCombo->testAttribute(Qt::WA_MacSmallSize)
      || mCombo->testAttribute(Qt::WA_MacMiniSize)
      || mCombo->font() != cs_app_fonts_hash()->value("QComboBox", QFont())) {
      menuOption.font = mCombo->font();

   } else {
      QVariant fontRoleData = index.data(Qt::FontRole);
      if (fontRoleData.isValid()) {
         menuOption.font = fontRoleData.value<QFont>();
      } else {
         menuOption.font = cs_app_fonts_hash()->value("QComboMenuItem", mCombo->font());
      }
   }

   menuOption.fontMetrics = QFontMetrics(menuOption.font);

   return menuOption;
}

#ifndef QT_NO_COMPLETER
void QComboBoxPrivate::_q_completerActivated(const QModelIndex &index)
{
   Q_Q(QComboBox);

   if (index.isValid() && q->completer()) {
      QAbstractProxyModel *proxy = qobject_cast<QAbstractProxyModel *>(q->completer()->completionModel());

      if (proxy) {
         q->setCurrentIndex(proxy->mapToSource(index).row());
         emitActivated(currentIndex);
      }
   }

#ifdef QT_KEYPAD_NAVIGATION
   if (QApplication::keypadNavigationEnabled() && q->isEditable() && q->completer()
         && q->completer()->completionMode() == QCompleter::UnfilteredPopupCompletion ) {
      q->setEditFocus(false);
   }
#endif

}
#endif

void QComboBoxPrivate::updateArrow(QStyle::StateFlag state)
{
   Q_Q(QComboBox);

   if (arrowState == state) {
      return;
   }

   arrowState = state;
   QStyleOptionComboBox opt;
   q->initStyleOption(&opt);
   q->update(q->rect());
}

void QComboBoxPrivate::_q_modelReset()
{
   Q_Q(QComboBox);
   if (lineEdit) {
      lineEdit->setText(QString());
      updateLineEditGeometry();
   }

   if (currentIndex.row() != indexBeforeChange) {
      _q_emitCurrentIndexChanged(currentIndex);
   }

   modelChanged();
   q->update();
}

void QComboBoxPrivate::_q_modelDestroyed()
{
   model = QAbstractItemModelPrivate::staticEmptyModel();
}

//Windows and KDE allows menus to cover the taskbar, while GNOME and Mac don't
QRect QComboBoxPrivate::popupGeometry(int screen) const
{
   bool useFullScreenForPopupMenu = false;

   if (const QPlatformTheme *theme = QGuiApplicationPrivate::platformTheme()) {
      useFullScreenForPopupMenu = theme->themeHint(QPlatformTheme::UseFullScreenForPopupMenu).toBool();
   }

   return useFullScreenForPopupMenu ?
      QApplication::desktop()->screenGeometry(screen) :
      QApplication::desktop()->availableGeometry(screen);
}

bool QComboBoxPrivate::updateHoverControl(const QPoint &pos)
{

   Q_Q(QComboBox);

   QRect lastHoverRect = hoverRect;
   QStyle::SubControl lastHoverControl = hoverControl;

   bool doesHover = q->testAttribute(Qt::WA_Hover);

   if (lastHoverControl != newHoverControl(pos) && doesHover) {
      q->update(lastHoverRect);
      q->update(hoverRect);
      return true;
   }

   return !doesHover;
}

QStyle::SubControl QComboBoxPrivate::newHoverControl(const QPoint &pos)
{
   Q_Q(QComboBox);

   QStyleOptionComboBox opt;
   q->initStyleOption(&opt);
   opt.subControls = QStyle::SC_All;
   hoverControl = q->style()->hitTestComplexControl(QStyle::CC_ComboBox, &opt, pos, q);

   hoverRect = (hoverControl != QStyle::SC_None)
      ? q->style()->subControlRect(QStyle::CC_ComboBox, &opt, hoverControl, q)
      : QRect();

   return hoverControl;
}

int QComboBoxPrivate::computeWidthHint() const
{
   Q_Q(const QComboBox);

   int width = 0;
   const int count = q->count();
   const int iconWidth = q->iconSize().width() + 4;
   const QFontMetrics &fontMetrics = q->fontMetrics();

   for (int i = 0; i < count; ++i) {
      const int textWidth = fontMetrics.width(q->itemText(i));
      if (q->itemIcon(i).isNull()) {
         width = (qMax(width, textWidth));
      } else {
         width = (qMax(width, textWidth + iconWidth));
      }
   }

   QStyleOptionComboBox opt;
   q->initStyleOption(&opt);
   QSize tmp(width, 0);
   tmp = q->style()->sizeFromContents(QStyle::CT_ComboBox, &opt, tmp, q);

   return tmp.width();
}

QSize QComboBoxPrivate::recomputeSizeHint(QSize &sh) const
{
   Q_Q(const QComboBox);

   if (!sh.isValid()) {
      bool hasIcon = sizeAdjustPolicy == QComboBox::AdjustToMinimumContentsLengthWithIcon ? true : false;
      int count = q->count();
      QSize iconSize = q->iconSize();
      const QFontMetrics &fm = q->fontMetrics();

      // text width
      if (&sh == &sizeHint || minimumContentsLength == 0) {

         switch (sizeAdjustPolicy) {
            case QComboBox::AdjustToContents:
            case QComboBox::AdjustToContentsOnFirstShow:
               if (count == 0) {
                  sh.rwidth() = 7 * fm.width('x');

               } else {
                  for (int i = 0; i < count; ++i) {
                     if (!q->itemIcon(i).isNull()) {
                        hasIcon = true;
                        sh.setWidth(qMax(sh.width(), fm.boundingRect(q->itemText(i)).width() + iconSize.width() + 4));
                     } else {
                        sh.setWidth(qMax(sh.width(), fm.boundingRect(q->itemText(i)).width()));
                     }
                  }
               }
               break;

            default:
               break;
         }

      } else {
         for (int i = 0; i < count && !hasIcon; ++i) {
            hasIcon = !q->itemIcon(i).isNull();
         }
      }

      if (minimumContentsLength > 0) {
         sh.setWidth(qMax(sh.width(), minimumContentsLength * fm.width('X') + (hasIcon ? iconSize.width() + 4 : 0)));
      }

      // height
      sh.setHeight(qMax(qCeil(QFontMetricsF(fm).height()), 14) + 2);
      if (hasIcon) {
         sh.setHeight(qMax(sh.height(), iconSize.height() + 2));
      }

      // add style and strut values
      QStyleOptionComboBox opt;
      q->initStyleOption(&opt);
      sh = q->style()->sizeFromContents(QStyle::CT_ComboBox, &opt, sh, q);
   }

   return sh.expandedTo(QApplication::globalStrut());
}

void QComboBoxPrivate::adjustComboBoxSize()
{
   viewContainer()->adjustSizeTimer.start(20, container);
}

void QComboBoxPrivate::updateLayoutDirection()
{
   Q_Q(const QComboBox);

   QStyleOptionComboBox opt;
   q->initStyleOption(&opt);

   Qt::LayoutDirection dir = Qt::LayoutDirection(
         q->style()->styleHint(QStyle::SH_ComboBox_LayoutDirection, &opt, q));

   if (lineEdit) {
      lineEdit->setLayoutDirection(dir);
   }

   if (container) {
      container->setLayoutDirection(dir);
   }
}

void QComboBoxPrivateContainer::timerEvent(QTimerEvent *timerEvent)
{
   if (timerEvent->timerId() == adjustSizeTimer.timerId()) {
      adjustSizeTimer.stop();

      if (combo->sizeAdjustPolicy() == QComboBox::AdjustToContents) {
         combo->updateGeometry();
         combo->adjustSize();
         combo->update();
      }
   }
}

void QComboBoxPrivateContainer::resizeEvent(QResizeEvent *e)
{
   QStyleOptionComboBox opt = comboStyleOption();

   if (combo->style()->styleHint(QStyle::SH_ComboBox_Popup, &opt, combo)) {
      QStyleOption myOpt;
      myOpt.initFrom(this);

      QStyleHintReturnMask mask;
      if (combo->style()->styleHint(QStyle::SH_Menu_Mask, &myOpt, this, &mask)) {
         setMask(mask.region);
      }

   } else {
      clearMask();
   }

   QFrame::resizeEvent(e);
}

void QComboBoxPrivateContainer::leaveEvent(QEvent *)
{
   // On Mac using the Mac style we want to clear the selection
   // when the mouse moves outside the popup.
}

QComboBoxPrivateContainer::QComboBoxPrivateContainer(QAbstractItemView *itemView, QComboBox *parent)
   : QFrame(parent, Qt::Popup), combo(parent), view(nullptr), top(nullptr), bottom(nullptr),
     maybeIgnoreMouseButtonRelease(false)
{
   // we need the combobox and itemview
   Q_ASSERT(parent);
   Q_ASSERT(itemView);

   setAttribute(Qt::WA_WindowPropagation);
   setAttribute(Qt::WA_X11NetWmWindowTypeCombo);

   // setup container
   blockMouseReleaseTimer.setSingleShot(true);

   // we need a vertical layout
   QBoxLayout *layout =  new QBoxLayout(QBoxLayout::TopToBottom, this);
   layout->setSpacing(0);
   layout->setMargin(0);

   // set item view
   setItemView(itemView);

   // add scroller arrows if style needs them
   QStyleOptionComboBox opt = comboStyleOption();
   const bool usePopup = combo->style()->styleHint(QStyle::SH_ComboBox_Popup, &opt, combo);

   if (usePopup) {
      top    = new QComboBoxPrivateScroller(QAbstractSlider::SliderSingleStepSub, this);
      bottom = new QComboBoxPrivateScroller(QAbstractSlider::SliderSingleStepAdd, this);
      top->hide();
      bottom->hide();

   } else {
      setLineWidth(1);
   }

   setFrameStyle(combo->style()->styleHint(QStyle::SH_ComboBox_PopupFrameStyle, &opt, combo));

   if (top != nullptr) {
      layout->insertWidget(0, top);
      connect(top, &QComboBoxPrivateScroller::doScroll, this, &QComboBoxPrivateContainer::scrollItemView);
   }

   if (bottom != nullptr) {
      layout->addWidget(bottom);
      connect(bottom, &QComboBoxPrivateScroller::doScroll, this, &QComboBoxPrivateContainer::scrollItemView);
   }

   // Some styles (Mac) have a margin at the top and bottom of the popup.
   layout->insertSpacing(0, 0);
   layout->addSpacing(0);
   updateTopBottomMargin();
}

void QComboBoxPrivateContainer::scrollItemView(int action)
{
#ifndef QT_NO_SCROLLBAR
   if (view->verticalScrollBar()) {
      view->verticalScrollBar()->triggerAction(static_cast<QAbstractSlider::SliderAction>(action));
   }
#endif
}

void QComboBoxPrivateContainer::updateScrollers()
{
#ifndef QT_NO_SCROLLBAR
   if (! top || ! bottom) {
      return;
   }

   if (isVisible() == false) {
      return;
   }

   QStyleOptionComboBox opt = comboStyleOption();
   if (combo->style()->styleHint(QStyle::SH_ComboBox_Popup, &opt, combo) &&
      view->verticalScrollBar()->minimum() < view->verticalScrollBar()->maximum()) {

      bool needTop    = view->verticalScrollBar()->value() > (view->verticalScrollBar()->minimum() + topMargin());
      bool needBottom = view->verticalScrollBar()->value() < (view->verticalScrollBar()->maximum() - bottomMargin() - topMargin());

      if (needTop) {
         top->show();
      } else {
         top->hide();
      }

      if (needBottom) {
         bottom->show();
      } else {
         bottom->hide();
      }

   } else {
      top->hide();
      bottom->hide();
   }
#endif
}

void QComboBoxPrivateContainer::viewDestroyed()
{
   view = nullptr;
   setItemView(new QComboBoxListView());
}

QAbstractItemView *QComboBoxPrivateContainer::itemView() const
{
   return view;
}

void QComboBoxPrivateContainer::setItemView(QAbstractItemView *itemView)
{
   Q_ASSERT(itemView);

   // clean up old one
   if (view) {
      view->removeEventFilter(this);
      view->viewport()->removeEventFilter(this);

#ifndef QT_NO_SCROLLBAR
      disconnect(view->verticalScrollBar(), &QScrollBar::valueChanged, this, &QComboBoxPrivateContainer::updateScrollers);
      disconnect(view->verticalScrollBar(), &QScrollBar::rangeChanged, this, &QComboBoxPrivateContainer::updateScrollers);
#endif

      disconnect(view, &QObject::destroyed, this, &QComboBoxPrivateContainer::viewDestroyed);

      delete view;
      view = nullptr;
   }

   // setup the item view
   view = itemView;
   view->setParent(this);
   view->setAttribute(Qt::WA_MacShowFocusRect, false);
   qobject_cast<QBoxLayout *>(layout())->insertWidget(top ? 2 : 0, view);
   view->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
   view->installEventFilter(this);
   view->viewport()->installEventFilter(this);
   view->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

   QStyleOptionComboBox opt = comboStyleOption();
   const bool usePopup = combo->style()->styleHint(QStyle::SH_ComboBox_Popup, &opt, combo);

#ifndef QT_NO_SCROLLBAR
   if (usePopup) {
      view->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
   }
#endif

   if (combo->style()->styleHint(QStyle::SH_ComboBox_ListMouseTracking, &opt, combo) ||
      usePopup) {
      view->setMouseTracking(true);
   }
   view->setSelectionMode(QAbstractItemView::SingleSelection);
   view->setFrameStyle(QFrame::NoFrame);
   view->setLineWidth(0);
   view->setEditTriggers(QAbstractItemView::NoEditTriggers);

#ifndef QT_NO_SCROLLBAR
   connect(view->verticalScrollBar(), &QScrollBar::valueChanged, this, &QComboBoxPrivateContainer::updateScrollers);
   connect(view->verticalScrollBar(), &QScrollBar::rangeChanged, this, &QComboBoxPrivateContainer::updateScrollers);
#endif

   connect(view, &QObject::destroyed, this, &QComboBoxPrivateContainer::viewDestroyed);
}

int QComboBoxPrivateContainer::topMargin() const
{
   if (const QListView *lview = qobject_cast<const QListView *>(view)) {
      return lview->spacing();
   }

#ifndef QT_NO_TABLEVIEW
   if (const QTableView *tview = qobject_cast<const QTableView *>(view)) {
      return tview->showGrid() ? 1 : 0;
   }
#endif

   return 0;
}

int QComboBoxPrivateContainer::spacing() const
{
   QListView *lview = qobject_cast<QListView *>(view);

   if (lview) {
      return 2 * lview->spacing(); // QListView::spacing is the padding around the item.
   }

#ifndef QT_NO_TABLEVIEW
   QTableView *tview = qobject_cast<QTableView *>(view);

   if (tview) {
      return tview->showGrid() ? 1 : 0;
   }
#endif

   return 0;
}

void QComboBoxPrivateContainer::updateTopBottomMargin()
{
   if (! layout() || layout()->count() < 1) {
      return;
   }

   QBoxLayout *boxLayout = qobject_cast<QBoxLayout *>(layout());
   if (!boxLayout) {
      return;
   }

   const QStyleOptionComboBox opt = comboStyleOption();
   const bool usePopup = combo->style()->styleHint(QStyle::SH_ComboBox_Popup, &opt, combo);
   const int margin = usePopup ? combo->style()->pixelMetric(QStyle::PM_MenuVMargin, &opt, combo) : 0;

   QSpacerItem *topSpacer = boxLayout->itemAt(0)->spacerItem();
   if (topSpacer) {
      topSpacer->changeSize(0, margin, QSizePolicy::Minimum, QSizePolicy::Fixed);
   }

   QSpacerItem *bottomSpacer = boxLayout->itemAt(boxLayout->count() - 1)->spacerItem();
   if (bottomSpacer && bottomSpacer != topSpacer) {
      bottomSpacer->changeSize(0, margin, QSizePolicy::Minimum, QSizePolicy::Fixed);
   }

   boxLayout->invalidate();
}

void QComboBoxPrivateContainer::changeEvent(QEvent *e)
{
   if (e->type() == QEvent::StyleChange) {
      QStyleOptionComboBox opt = comboStyleOption();

      view->setMouseTracking(combo->style()->styleHint(QStyle::SH_ComboBox_ListMouseTracking, &opt, combo) ||
         combo->style()->styleHint(QStyle::SH_ComboBox_Popup, &opt, combo));
      setFrameStyle(combo->style()->styleHint(QStyle::SH_ComboBox_PopupFrameStyle, &opt, combo));
   }

   QWidget::changeEvent(e);
}

bool QComboBoxPrivateContainer::eventFilter(QObject *o, QEvent *e)
{
   switch (e->type()) {
      case QEvent::ShortcutOverride: {
         QKeyEvent *keyEvent = static_cast<QKeyEvent *>(e);

         switch (keyEvent->key()) {
            case Qt::Key_Enter:
            case Qt::Key_Return:

#ifdef QT_KEYPAD_NAVIGATION
            case Qt::Key_Select:
#endif
               if (view->currentIndex().isValid() && (view->currentIndex().flags() & Qt::ItemIsEnabled) ) {
                  combo->hidePopup();
                  emit itemSelected(view->currentIndex());
               }
               return true;

            case Qt::Key_Down:
               if (!(keyEvent->modifiers() & Qt::AltModifier)) {
                  break;
               }

               [[fallthrough]];

            case Qt::Key_F4:
               combo->hidePopup();
               return true;

            default:
               if (keyEvent->matches(QKeySequence::Cancel)) {
                  combo->hidePopup();
                  return true;
               }
               break;
         }
         break;
      }

      case QEvent::MouseMove:
         if (isVisible()) {
            QMouseEvent *m = static_cast<QMouseEvent *>(e);
            QWidget *widget = static_cast<QWidget *>(o);
            QPoint vector = widget->mapToGlobal(m->pos()) - initialClickPosition;

            if (vector.manhattanLength() > 9 && blockMouseReleaseTimer.isActive()) {
               blockMouseReleaseTimer.stop();
            }

            QModelIndex indexUnderMouse = view->indexAt(m->pos());
            if (indexUnderMouse.isValid() && ! QComboBoxDelegate::isSeparator(indexUnderMouse)) {
               view->setCurrentIndex(indexUnderMouse);
            }
         }
         break;

      case QEvent::MouseButtonPress:
         maybeIgnoreMouseButtonRelease = false;
         break;

      case QEvent::MouseButtonRelease: {
         bool ignoreEvent = maybeIgnoreMouseButtonRelease && popupTimer.elapsed() < QApplication::doubleClickInterval();
         QMouseEvent *m = static_cast<QMouseEvent *>(e);

         if (isVisible() && view->rect().contains(m->pos()) && view->currentIndex().isValid()
            && ! blockMouseReleaseTimer.isActive() && !ignoreEvent
            && (view->currentIndex().flags() & Qt::ItemIsEnabled)
            && (view->currentIndex().flags() & Qt::ItemIsSelectable)) {
            combo->hidePopup();
            emit itemSelected(view->currentIndex());
            return true;
         }
         break;
      }

      default:
         break;
   }

   return QFrame::eventFilter(o, e);
}

void QComboBoxPrivateContainer::showEvent(QShowEvent *)
{
   combo->update();
}

void QComboBoxPrivateContainer::hideEvent(QHideEvent *)
{
   emit resetButton();
   combo->update();

#ifndef QT_NO_GRAPHICSVIEW
   // QGraphicsScenePrivate::removePopup closes the combo box popup, it hides it non-explicitly.
   // Hiding/showing the QComboBox after this will unexpectedly show the popup as well.
   // Re-hiding the popup container makes sure it is explicitly hidden.
   if (QGraphicsProxyWidget *proxy = graphicsProxyWidget()) {
      proxy->hide();
   }
#endif
}

void QComboBoxPrivateContainer::mousePressEvent(QMouseEvent *e)
{

   QStyleOptionComboBox opt = comboStyleOption();
   opt.subControls = QStyle::SC_All;
   opt.activeSubControls = QStyle::SC_ComboBoxArrow;

   QStyle::SubControl sc = combo->style()->hitTestComplexControl(QStyle::CC_ComboBox, &opt,
         combo->mapFromGlobal(e->globalPos()), combo);

   if ((combo->isEditable() && sc == QStyle::SC_ComboBoxArrow)
      || (!combo->isEditable() && sc != QStyle::SC_None)) {
      setAttribute(Qt::WA_NoMouseReplay);
   }

   combo->hidePopup();
}

void QComboBoxPrivateContainer::mouseReleaseEvent(QMouseEvent *e)
{
   (void) e;

   if (! blockMouseReleaseTimer.isActive()) {
      combo->hidePopup();
      emit resetButton();
   }
}

QStyleOptionComboBox QComboBoxPrivateContainer::comboStyleOption() const
{
   // ### This should use QComboBox's initStyleOption(), but it's protected
   // perhaps, we could cheat by having the QCombo private instead?
   QStyleOptionComboBox opt;
   opt.initFrom(combo);
   opt.subControls = QStyle::SC_All;
   opt.activeSubControls = QStyle::SC_None;
   opt.editable = combo->isEditable();

   return opt;
}

QComboBox::QComboBox(QWidget *parent)
   : QWidget(*new QComboBoxPrivate(), parent, Qt::EmptyFlag)
{
   Q_D(QComboBox);
   d->init();
}

// internal
QComboBox::QComboBox(QComboBoxPrivate &dd, QWidget *parent)
   : QWidget(dd, parent, Qt::EmptyFlag)
{
   Q_D(QComboBox);
   d->init();
}

void QComboBoxPrivate::init()
{
   Q_Q(QComboBox);

#ifdef Q_OS_DARWIN
   // On OS X, only line edits and list views always get tab focus. It's only
   // when we enable full keyboard access that other controls can get tab focus.
   // When it's not editable, a combobox looks like a button, and it behaves as
   // such in this respect.

   if (! q->isEditable()) {
      q->setFocusPolicy(Qt::TabFocus);
   } else

#endif
   {
      q->setFocusPolicy(Qt::WheelFocus);
   }

   q->setSizePolicy(QSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed, QSizePolicy::ComboBox));

   setLayoutItemMargins(QStyle::SE_ComboBoxLayoutItem);
   q->setModel(new QStandardItemModel(0, 1, q));

   if (! q->isEditable()) {
      q->setAttribute(Qt::WA_InputMethodEnabled, false);
   } else {
      q->setAttribute(Qt::WA_InputMethodEnabled);
   }
}

QComboBoxPrivateContainer *QComboBoxPrivate::viewContainer()
{
   if (container) {
      return container;
   }

   Q_Q(QComboBox);

   container = new QComboBoxPrivateContainer(new QComboBoxListView(q), q);
   container->itemView()->setModel(model);
   container->itemView()->setTextElideMode(Qt::ElideMiddle);
   updateDelegate(true);
   updateLayoutDirection();
   updateViewContainerPaletteAndOpacity();

   QObject::connect(container, &QComboBoxPrivateContainer::itemSelected,   q, &QComboBox::_q_itemSelected);
   QObject::connect(container, &QComboBoxPrivateContainer::resetButton,    q, &QComboBox::_q_resetButton);

   QObject::connect(container->itemView()->selectionModel(), &QItemSelectionModel::currentChanged,
         q, &QComboBox::_q_emitHighlighted);

   return container;
}

void QComboBoxPrivate::_q_resetButton()
{
   updateArrow(QStyle::State_None);
}

void QComboBoxPrivate::_q_dataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight)
{
   Q_Q(QComboBox);

   if (inserting || topLeft.parent() != root) {
      return;
   }

   if (sizeAdjustPolicy == QComboBox::AdjustToContents) {
      sizeHint = QSize();
      adjustComboBoxSize();
      q->updateGeometry();
   }

   if (currentIndex.row() >= topLeft.row() && currentIndex.row() <= bottomRight.row()) {
      const QString text = q->itemText(currentIndex.row());

      if (lineEdit) {
         lineEdit->setText(text);
         updateLineEditGeometry();
      } else {
         emit q->currentTextChanged(text);
      }

      q->update();

#ifndef QT_NO_ACCESSIBILITY
      QAccessibleValueChangeEvent event(q, text);
      QAccessible::updateAccessibility(&event);
#endif
   }
}

void QComboBoxPrivate::_q_rowsInserted(const QModelIndex &parent, int start, int end)
{
   Q_Q(QComboBox);

   if (inserting || parent != root) {
      return;
   }

   if (sizeAdjustPolicy == QComboBox::AdjustToContents) {
      sizeHint = QSize();
      adjustComboBoxSize();
      q->updateGeometry();
   }

   if (start == 0 && (end - start + 1) == q->count() && !currentIndex.isValid()) {
      // set current index if combo was previously empty
      q->setCurrentIndex(0);

   } else if (currentIndex.row() != indexBeforeChange) {
      // need to emit changed if model updated index "silently"

      q->update();
      _q_emitCurrentIndexChanged(currentIndex);
   }
}

void QComboBoxPrivate::_q_updateIndexBeforeChange()
{
   indexBeforeChange = currentIndex.row();
}

void QComboBoxPrivate::_q_rowsRemoved(const QModelIndex &parent, int, int)
{
   Q_Q(QComboBox);

   if (parent != root) {
      return;
   }

   if (sizeAdjustPolicy == QComboBox::AdjustToContents) {
      sizeHint = QSize();
      adjustComboBoxSize();
      q->updateGeometry();
   }

   // model has changed the currentIndex
   if (currentIndex.row() != indexBeforeChange) {
      if (!currentIndex.isValid() && q->count()) {
         q->setCurrentIndex(qMin(q->count() - 1, qMax(indexBeforeChange, 0)));
         return;
      }

      if (lineEdit) {
         lineEdit->setText(q->itemText(currentIndex.row()));
         updateLineEditGeometry();
      }

      q->update();
      _q_emitCurrentIndexChanged(currentIndex);
   }
}

void QComboBoxPrivate::updateViewContainerPaletteAndOpacity()
{
   if (! container) {
      return;
   }

   Q_Q(QComboBox);
   QStyleOptionComboBox opt;
   q->initStyleOption(&opt);

#ifndef QT_NO_MENU
   if (q->style()->styleHint(QStyle::SH_ComboBox_Popup, &opt, q)) {
      QMenu menu;
      menu.ensurePolished();
      container->setPalette(menu.palette());
      container->setWindowOpacity(menu.windowOpacity());
   } else
#endif

   {
      container->setPalette(q->palette());
      container->setWindowOpacity(1.0);
   }

   if (lineEdit) {
      lineEdit->setPalette(q->palette());
   }
}

void QComboBoxPrivate::updateFocusPolicy()
{
#ifdef Q_OS_DARWIN
   Q_Q(QComboBox);

   // refer to comment in QComboBoxPrivate::init()
   if (q->isEditable()) {
      q->setFocusPolicy(Qt::WheelFocus);
   } else {
      q->setFocusPolicy(Qt::TabFocus);
   }
#endif
}

void QComboBox::initStyleOption(QStyleOptionComboBox *option) const
{
   if (! option) {
      return;
   }

   Q_D(const QComboBox);

   option->initFrom(this);
   option->editable = isEditable();
   option->frame = d->frame;

   if (hasFocus() && !option->editable) {
      option->state |= QStyle::State_Selected;
   }

   option->subControls = QStyle::SC_All;

   if (d->arrowState == QStyle::State_Sunken) {
      option->activeSubControls = QStyle::SC_ComboBoxArrow;
      option->state |= d->arrowState;
   } else {
      option->activeSubControls = d->hoverControl;
   }

   if (d->currentIndex.isValid()) {
      option->currentText = currentText();
      option->currentIcon = d->itemIcon(d->currentIndex);
   }

   option->iconSize = iconSize();

   if (d->container && d->container->isVisible()) {
      option->state |= QStyle::State_On;
   }
}

void QComboBoxPrivate::updateLineEditGeometry()
{
   if (! lineEdit) {
      return;
   }

   Q_Q(QComboBox);

   QStyleOptionComboBox opt;
   q->initStyleOption(&opt);

   QRect editRect = q->style()->subControlRect(QStyle::CC_ComboBox, &opt,
         QStyle::SC_ComboBoxEditField, q);

   if (! q->itemIcon(q->currentIndex()).isNull()) {
      QRect comboRect(editRect);
      editRect.setWidth(editRect.width() - q->iconSize().width() - 4);
      editRect = QStyle::alignedRect(q->layoutDirection(), Qt::AlignRight, editRect.size(), comboRect);
   }

   lineEdit->setGeometry(editRect);
}

Qt::MatchFlags QComboBoxPrivate::matchFlags() const
{
   // Base how duplicates are determined on the autocompletion case sensitivity
   Qt::MatchFlags flags = Qt::MatchFixedString;

#ifndef QT_NO_COMPLETER
   if (! lineEdit->completer() || lineEdit->completer()->caseSensitivity() == Qt::CaseSensitive)
#endif
   {
      flags |= Qt::MatchCaseSensitive;
   }

   return flags;
}

void QComboBoxPrivate::_q_editingFinished()
{
   Q_Q(QComboBox);

   if (lineEdit && !lineEdit->text().isEmpty() && itemText(currentIndex) != lineEdit->text()) {
      const int index = q_func()->findText(lineEdit->text(), matchFlags());

      if (index != -1) {
         q->setCurrentIndex(index);
         emitActivated(currentIndex);
      }
   }
}

void QComboBoxPrivate::_q_returnPressed()
{
   Q_Q(QComboBox);

   // The insertion code below does not apply when the policy is QComboBox::NoInsert.
   // In case a completer is installed, item activation via the completer is handled
   // in _q_completerActivated(). Otherwise _q_editingFinished() updates the current
   // index as appropriate.

   if (insertPolicy == QComboBox::NoInsert) {
      return;
   }

   if (lineEdit && !lineEdit->text().isEmpty()) {
      if (q->count() >= maxCount && !(this->insertPolicy == QComboBox::InsertAtCurrent)) {
         return;
      }

      lineEdit->deselect();
      lineEdit->end(false);

      QString text = lineEdit->text();

      // check for duplicates (if not enabled) and quit
      int index = -1;

      if (! duplicatesEnabled) {
         index = q->findText(text, matchFlags());
         if (index != -1) {
            q->setCurrentIndex(index);
            emitActivated(currentIndex);
            return;
         }
      }

      switch (insertPolicy) {
         case QComboBox::InsertAtTop:
            index = 0;
            break;

         case QComboBox::InsertAtBottom:
            index = q->count();
            break;

         case QComboBox::InsertAtCurrent:
         case QComboBox::InsertAfterCurrent:
         case QComboBox::InsertBeforeCurrent:
            if (! q->count() || !currentIndex.isValid()) {
               index = 0;
            } else if (insertPolicy == QComboBox::InsertAtCurrent) {
               q->setItemText(q->currentIndex(), text);
            } else if (insertPolicy == QComboBox::InsertAfterCurrent) {
               index = q->currentIndex() + 1;
            } else if (insertPolicy == QComboBox::InsertBeforeCurrent) {
               index = q->currentIndex();
            }
            break;

         case QComboBox::InsertAlphabetically:
            index = 0;

            for (int i = 0; i < q->count(); i++, index++ ) {
               if (text.toLower() < q->itemText(i).toLower()) {
                  break;
               }
            }
            break;

         default:
            break;
      }

      if (index >= 0) {
         q->insertItem(index, text);
         q->setCurrentIndex(index);
         emitActivated(currentIndex);
      }
   }
}

void QComboBoxPrivate::_q_itemSelected(const QModelIndex &item)
{
   Q_Q(QComboBox);

   if (item != currentIndex) {
      setCurrentIndex(item);
   } else if (lineEdit) {
      lineEdit->selectAll();
      lineEdit->setText(q->itemText(currentIndex.row()));
   }

   emitActivated(currentIndex);
}

void QComboBoxPrivate::emitActivated(const QModelIndex &index)
{
   Q_Q(QComboBox);

   if (! index.isValid()) {
      return;
   }

   QString text(itemText(index));
   emit q->activated(index.row());
   emit q->activated(text);
}

void QComboBoxPrivate::_q_emitHighlighted(const QModelIndex &index)
{
   Q_Q(QComboBox);

   if (! index.isValid()) {
      return;
   }

   QString text(itemText(index));
   emit q->highlighted(index.row());
   emit q->highlighted(text);
}

void QComboBoxPrivate::_q_emitCurrentIndexChanged(const QModelIndex &index)
{
   Q_Q(QComboBox);
   const QString text = itemText(index);

   emit q->cs_currentIndexChanged(index.row());
   emit q->currentIndexChanged(index.row());

   emit q->currentIndexChanged(text);

   // signal lineEdit.textChanged maybe connected to signal currentTextChanged, do not emit twice
   if (! lineEdit) {
      emit q->currentTextChanged(text);
   }

#ifndef QT_NO_ACCESSIBILITY
   QAccessibleValueChangeEvent event(q, text);
   QAccessible::updateAccessibility(&event);
#endif
}

QString QComboBoxPrivate::itemText(const QModelIndex &index) const
{
   return index.isValid() ? model->data(index, itemRole()).toString() : QString();
}

int QComboBoxPrivate::itemRole() const
{
   return q_func()->isEditable() ? Qt::EditRole : Qt::DisplayRole;
}

QComboBox::~QComboBox()
{
   // ### check delegateparent and delete delegate if it is a child of this comboBox
   Q_D(QComboBox);

   try {
      disconnect(d->model, &QAbstractItemModel::destroyed, this, &QComboBox::_q_modelDestroyed);

   } catch(...) {
      ;
   }
}

int QComboBox::maxVisibleItems() const
{
   Q_D(const QComboBox);
   return d->maxVisibleItems;
}

void QComboBox::setMaxVisibleItems(int maxItems)
{
   Q_D(QComboBox);

   if (maxItems < 0) {
      qWarning("QComboBox::setMaxVisibleItems() Invalid number of items (%d) must be >= 0", maxItems);
      return;
   }

   d->maxVisibleItems = maxItems;
}

int QComboBox::count() const
{
   Q_D(const QComboBox);
   return d->model->rowCount(d->root);
}


void QComboBox::setMaxCount(int max)
{
   Q_D(QComboBox);

   if (max < 0) {
      qWarning("QComboBox::setMaxCount() Invalid count (%d) must be >= 0", max);
      return;
   }

   if (max < count()) {
      d->model->removeRows(max, count() - max, d->root);
   }

   d->maxCount = max;
}

int QComboBox::maxCount() const
{
   Q_D(const QComboBox);
   return d->maxCount;
}

#ifndef QT_NO_COMPLETER

bool QComboBox::autoCompletion() const
{
   Q_D(const QComboBox);
   return d->autoCompletion;
}

void QComboBox::setAutoCompletion(bool enable)
{
   Q_D(QComboBox);

#ifdef QT_KEYPAD_NAVIGATION
   if (QApplication::keypadNavigationEnabled() && ! enable && isEditable()) {
      qWarning("QComboBox::setAutoCompletion() Auto completion must be enabled when "
            "the values in a combo box can be modified");
   }
#endif

   d->autoCompletion = enable;

   if (d->lineEdit == nullptr) {
      return;
   }

   if (enable) {
      if (d->lineEdit->completer()) {
         return;
      }

      d->completer = new QCompleter(d->model, d->lineEdit);

      connect(d->completer.data(), cs_mp_cast<const QModelIndex &>(&QCompleter::activated),
            this, &QComboBox::_q_completerActivated);

      d->completer->setCaseSensitivity(d->autoCompletionCaseSensitivity);
      d->completer->setCompletionMode(QCompleter::InlineCompletion);
      d->completer->setCompletionColumn(d->modelColumn);
      d->lineEdit->setCompleter(d->completer);
      d->completer->setWidget(this);

   } else {
      d->lineEdit->setCompleter(nullptr);
   }
}

Qt::CaseSensitivity QComboBox::autoCompletionCaseSensitivity() const
{
   Q_D(const QComboBox);
   return d->autoCompletionCaseSensitivity;
}

void QComboBox::setAutoCompletionCaseSensitivity(Qt::CaseSensitivity sensitivity)
{
   Q_D(QComboBox);

   d->autoCompletionCaseSensitivity = sensitivity;
   if (d->lineEdit && d->lineEdit->completer()) {
      d->lineEdit->completer()->setCaseSensitivity(sensitivity);
   }
}

#endif // QT_NO_COMPLETER

bool QComboBox::duplicatesEnabled() const
{
   Q_D(const QComboBox);
   return d->duplicatesEnabled;
}

void QComboBox::setDuplicatesEnabled(bool enable)
{
   Q_D(QComboBox);
   d->duplicatesEnabled = enable;
}

int QComboBox::findData(const QVariant &data, int role, Qt::MatchFlags flags) const
{
   Q_D(const QComboBox);

   QModelIndexList result;
   QModelIndex start = d->model->index(0, d->modelColumn, d->root);
   result = d->model->match(start, role, data, 1, flags);

   if (result.isEmpty()) {
      return -1;
   }

   return result.first().row();
}

QComboBox::InsertPolicy QComboBox::insertPolicy() const
{
   Q_D(const QComboBox);
   return d->insertPolicy;
}

void QComboBox::setInsertPolicy(InsertPolicy policy)
{
   Q_D(QComboBox);
   d->insertPolicy = policy;
}

QComboBox::SizeAdjustPolicy QComboBox::sizeAdjustPolicy() const
{
   Q_D(const QComboBox);
   return d->sizeAdjustPolicy;
}

void QComboBox::setSizeAdjustPolicy(QComboBox::SizeAdjustPolicy policy)
{
   Q_D(QComboBox);

   if (policy == d->sizeAdjustPolicy) {
      return;
   }

   d->sizeAdjustPolicy = policy;
   d->sizeHint = QSize();
   d->adjustComboBoxSize();
   updateGeometry();
}

int QComboBox::minimumContentsLength() const
{
   Q_D(const QComboBox);
   return d->minimumContentsLength;
}

void QComboBox::setMinimumContentsLength(int characters)
{
   Q_D(QComboBox);

   if (characters == d->minimumContentsLength || characters < 0) {
      return;
   }

   d->minimumContentsLength = characters;

   if (d->sizeAdjustPolicy == AdjustToContents || d->sizeAdjustPolicy == AdjustToMinimumContentsLengthWithIcon) {
      d->sizeHint = QSize();
      d->adjustComboBoxSize();
      updateGeometry();
   }
}

QSize QComboBox::iconSize() const
{
   Q_D(const QComboBox);
   if (d->iconSize.isValid()) {
      return d->iconSize;
   }

   int iconWidth = style()->pixelMetric(QStyle::PM_SmallIconSize, nullptr, this);
   return QSize(iconWidth, iconWidth);
}

void QComboBox::setIconSize(const QSize &size)
{
   Q_D(QComboBox);

   if (size == d->iconSize) {
      return;
   }

   view()->setIconSize(size);
   d->iconSize = size;
   d->sizeHint = QSize();
   updateGeometry();
}

bool QComboBox::isEditable() const
{
   Q_D(const QComboBox);
   return d->lineEdit != nullptr;
}

void QComboBoxPrivate::updateDelegate(bool force)
{
   Q_Q(QComboBox);

   QStyleOptionComboBox opt;
   q->initStyleOption(&opt);

   if (q->style()->styleHint(QStyle::SH_ComboBox_Popup, &opt, q)) {
      if (force || qobject_cast<QComboBoxDelegate *>(q->itemDelegate())) {
         q->setItemDelegate(new QComboMenuDelegate(q->view(), q));
      }

   } else {
      if (force || qobject_cast<QComboMenuDelegate *>(q->itemDelegate())) {
         q->setItemDelegate(new QComboBoxDelegate(q->view(), q));
      }
   }
}

QIcon QComboBoxPrivate::itemIcon(const QModelIndex &index) const
{
   QVariant decoration = model->data(index, Qt::DecorationRole);

   if (decoration.type() == QVariant::Pixmap) {
      return QIcon(decoration.value<QPixmap>());
   } else {
      return decoration.value<QIcon>();
   }
}

void QComboBox::setEditable(bool editable)
{
   Q_D(QComboBox);

   if (isEditable() == editable) {
      return;
   }

   QStyleOptionComboBox opt;
   initStyleOption(&opt);

   if (editable) {
      if (style()->styleHint(QStyle::SH_ComboBox_Popup, &opt, this)) {
         d->viewContainer()->updateScrollers();
         view()->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
      }

      QLineEdit *le = new QLineEdit(this);
      setLineEdit(le);

   } else {
      if (style()->styleHint(QStyle::SH_ComboBox_Popup, &opt, this)) {
         d->viewContainer()->updateScrollers();
         view()->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
      }

      setAttribute(Qt::WA_InputMethodEnabled, false);
      d->lineEdit->hide();
      d->lineEdit->deleteLater();
      d->lineEdit = nullptr;
   }

   d->updateDelegate();
   d->updateFocusPolicy();
   d->viewContainer()->updateTopBottomMargin();

   if (! testAttribute(Qt::WA_Resized)) {
      adjustSize();
   }
}

void QComboBox::setLineEdit(QLineEdit *edit)
{
   Q_D(QComboBox);

   if (edit == nullptr) {
      qWarning("QComboBox::setLineEdit() Unable to set a line edit to an invalid value (nullptr)");
      return;
   }

   if (edit == d->lineEdit) {
      return;
   }

   edit->setText(currentText());
   delete d->lineEdit;

   d->lineEdit = edit;
   qt_widget_private(d->lineEdit)->inheritsInputMethodHints = 1;

   if (d->lineEdit->parent() != this) {
      d->lineEdit->setParent(this);
   }

   connect(d->lineEdit, &QLineEdit::returnPressed,         this, &QComboBox::_q_returnPressed);
   connect(d->lineEdit, &QLineEdit::editingFinished,       this, &QComboBox::_q_editingFinished);
   connect(d->lineEdit, &QLineEdit::textChanged,           this, &QComboBox::editTextChanged);
   connect(d->lineEdit, &QLineEdit::textChanged,           this, &QComboBox::currentTextChanged);
   connect(d->lineEdit, &QLineEdit::cursorPositionChanged, this, &QComboBox::updateMicroFocus);
   connect(d->lineEdit, &QLineEdit::selectionChanged,      this, &QComboBox::updateMicroFocus);

   d->lineEdit->setFrame(false);
   d->lineEdit->setContextMenuPolicy(Qt::NoContextMenu);
   d->updateFocusPolicy();
   d->lineEdit->setFocusProxy(this);
   d->lineEdit->setAttribute(Qt::WA_MacShowFocusRect, false);

#ifndef QT_NO_COMPLETER
   setAutoCompletion(d->autoCompletion);
#endif

#if defined(QT_KEYPAD_NAVIGATION) && ! defined(QT_NO_COMPLETER)
   if (QApplication::keypadNavigationEnabled()) {
      // Editable combo boxes will have a completer that is set to UnfilteredPopupCompletion.
      // This means that when the user enters edit mode they are immediately presented with a
      // list of possible completions.
      setAutoCompletion(true);

      if (d->completer) {
         d->completer->setCompletionMode(QCompleter::UnfilteredPopupCompletion);
         connect(d->completer, &QCompletere::activated, this, &QComboBox::_q_completerActivated);
      }
   }
#endif

   setAttribute(Qt::WA_InputMethodEnabled);
   d->updateLayoutDirection();
   d->updateLineEditGeometry();

   if (isVisible()) {
      d->lineEdit->show();
   }

   update();
}

QLineEdit *QComboBox::lineEdit() const
{
   Q_D(const QComboBox);
   return d->lineEdit;
}

#ifndef QT_NO_VALIDATOR
void QComboBox::setValidator(const QValidator *v)
{
   Q_D(QComboBox);

   if (d->lineEdit) {
      d->lineEdit->setValidator(v);
   }
}

const QValidator *QComboBox::validator() const
{
   Q_D(const QComboBox);
   return d->lineEdit ? d->lineEdit->validator() : nullptr;
}
#endif

#ifndef QT_NO_COMPLETER
void QComboBox::setCompleter(QCompleter *c)
{
   Q_D(QComboBox);

   if (d->lineEdit == nullptr) {
      return;
   }

   d->lineEdit->setCompleter(c);

   if (c != nullptr) {
      connect(c, cs_mp_cast<const QModelIndex &>(&QCompleter::activated), this, &QComboBox::_q_completerActivated);
      c->setWidget(this);
   }
}

QCompleter *QComboBox::completer() const
{
   Q_D(const QComboBox);
   return d->lineEdit ? d->lineEdit->completer() : nullptr;
}
#endif

QAbstractItemDelegate *QComboBox::itemDelegate() const
{
   return view()->itemDelegate();
}

void QComboBox::setItemDelegate(QAbstractItemDelegate *delegate)
{
   if (! delegate) {
      qWarning("QComboBox::setItemDelegate() Unable to set a delegate to an invalid value (nullptr)");
      return;
   }

   delete view()->itemDelegate();
   view()->setItemDelegate(delegate);
}

QAbstractItemModel *QComboBox::model() const
{
   Q_D(const QComboBox);

   if (d->model == QAbstractItemModelPrivate::staticEmptyModel()) {
      QComboBox *that = const_cast<QComboBox *>(this);
      that->setModel(new QStandardItemModel(0, 1, that));
   }

   return d->model;
}

void QComboBox::setModel(QAbstractItemModel *model)
{
   Q_D(QComboBox);

   if (! model) {
      qWarning("QComboBox::setModel() Unable to set a model to an invalid value (nullptr)");
      return;
   }

   if (model == d->model) {
      return;
   }

#ifndef QT_NO_COMPLETER
   if (d->lineEdit && d->lineEdit->completer() && d->lineEdit->completer() == d->completer) {
      d->lineEdit->completer()->setModel(model);
   }
#endif

   if (d->model) {
      disconnect(d->model, &QAbstractItemModel::dataChanged,           this, &QComboBox::_q_dataChanged);
      disconnect(d->model, &QAbstractItemModel::rowsAboutToBeInserted, this, &QComboBox::_q_updateIndexBeforeChange);
      disconnect(d->model, &QAbstractItemModel::rowsInserted,          this, &QComboBox::_q_rowsInserted);
      disconnect(d->model, &QAbstractItemModel::rowsAboutToBeRemoved,  this, &QComboBox::_q_updateIndexBeforeChange);
      disconnect(d->model, &QAbstractItemModel::rowsRemoved,           this, &QComboBox::_q_rowsRemoved);
      disconnect(d->model, &QAbstractItemModel::destroyed,             this, &QComboBox::_q_modelDestroyed);
      disconnect(d->model, &QAbstractItemModel::modelAboutToBeReset,   this, &QComboBox::_q_updateIndexBeforeChange);
      disconnect(d->model, &QAbstractItemModel::modelReset,            this, &QComboBox::_q_modelReset);

      if (d->model->QObject::parent() == this) {
         delete d->model;
      }
   }

   d->model = model;

   connect(model, &QAbstractItemModel::dataChanged,           this, &QComboBox::_q_dataChanged);
   connect(model, &QAbstractItemModel::rowsAboutToBeInserted, this, &QComboBox::_q_updateIndexBeforeChange);
   connect(model, &QAbstractItemModel::rowsInserted,          this, &QComboBox::_q_rowsInserted);
   connect(model, &QAbstractItemModel::rowsAboutToBeRemoved,  this, &QComboBox::_q_updateIndexBeforeChange);
   connect(model, &QAbstractItemModel::rowsRemoved,           this, &QComboBox::_q_rowsRemoved);
   connect(model, &QAbstractItemModel::destroyed,             this, &QComboBox::_q_modelDestroyed);
   connect(model, &QAbstractItemModel::modelAboutToBeReset,   this, &QComboBox::_q_updateIndexBeforeChange);
   connect(model, &QAbstractItemModel::modelReset,            this, &QComboBox::_q_modelReset);

   if (d->container) {
      d->container->itemView()->setModel(model);
      connect(d->container->itemView()->selectionModel(), &QItemSelectionModel::currentChanged, this,
                  &QComboBox::_q_emitHighlighted, Qt::UniqueConnection);
   }

   setRootModelIndex(QModelIndex());
   bool currentReset  = false;
   const int rowCount = count();

   for (int pos = 0; pos < rowCount; pos++) {
      if (d->model->index(pos, d->modelColumn, d->root).flags() & Qt::ItemIsEnabled) {
         setCurrentIndex(pos);
         currentReset = true;
         break;
      }
   }

   if (! currentReset) {
      setCurrentIndex(-1);
   }

   d->modelChanged();
}

QModelIndex QComboBox::rootModelIndex() const
{
   Q_D(const QComboBox);
   return QModelIndex(d->root);
}

void QComboBox::setRootModelIndex(const QModelIndex &index)
{
   Q_D(QComboBox);

   if (d->root == index) {
      return;
   }

   d->root = QPersistentModelIndex(index);
   view()->setRootIndex(index);
   update();
}

int QComboBox::currentIndex() const
{
   Q_D(const QComboBox);
   return d->currentIndex.row();
}

void QComboBox::setCurrentIndex(int index)
{
   Q_D(QComboBox);
   QModelIndex mi = d->model->index(index, d->modelColumn, d->root);
   d->setCurrentIndex(mi);
}

void QComboBox::setCurrentText(const QString &text)
{
   if (isEditable()) {
      setEditText(text);
   } else {
      const int i = findText(text);
      if (i > -1) {
         setCurrentIndex(i);
      }
   }
}
void QComboBoxPrivate::setCurrentIndex(const QModelIndex &mi)
{
   Q_Q(QComboBox);

   QModelIndex normalized = mi.sibling(mi.row(), modelColumn); // no-op if mi.column() == modelColumn

   if (!normalized.isValid()) {
      normalized = mi;   // Fallback to passed index
   }

   bool indexChanged = (normalized != currentIndex);
   if (indexChanged) {
      currentIndex = QPersistentModelIndex(normalized);
   }

   if (lineEdit) {
      const QString newText = itemText(normalized);
      if (lineEdit->text() != newText) {
         lineEdit->setText(newText);

#ifndef QT_NO_COMPLETER
         if (lineEdit && lineEdit->completer()) {
            lineEdit->completer()->setCompletionPrefix(newText);
         }
#endif

      }
      updateLineEditGeometry();
   }

   if (indexChanged) {
      q->update();
      _q_emitCurrentIndexChanged(currentIndex);
   }
}

QString QComboBox::currentText() const
{
   Q_D(const QComboBox);

   if (d->lineEdit) {
      return d->lineEdit->text();
   } else if (d->currentIndex.isValid()) {
      return d->itemText(d->currentIndex);
   } else {
      return QString();
   }
}

QVariant QComboBox::currentData(int role) const
{
   Q_D(const QComboBox);
   return d->currentIndex.data(role);
}

QString QComboBox::itemText(int index) const
{
   Q_D(const QComboBox);
   QModelIndex mi = d->model->index(index, d->modelColumn, d->root);
   return d->itemText(mi);
}

QIcon QComboBox::itemIcon(int index) const
{
   Q_D(const QComboBox);
   QModelIndex mi = d->model->index(index, d->modelColumn, d->root);
   return d->itemIcon(mi);
}

QVariant QComboBox::itemData(int index, int role) const
{
   Q_D(const QComboBox);
   QModelIndex mi = d->model->index(index, d->modelColumn, d->root);
   return d->model->data(mi, role);
}

void QComboBox::insertItem(int index, const QIcon &icon, const QString &text, const QVariant &userData)
{
   Q_D(QComboBox);

   int itemCount = count();
   index = qBound(0, index, itemCount);

   if (index >= d->maxCount) {
      return;
   }

   // For the common case where we are using the built in QStandardItemModel
   // construct a QStandardItem, reducing the number of expensive signals from the model
   if (QStandardItemModel *m = qobject_cast<QStandardItemModel *>(d->model)) {
      QStandardItem *item = new QStandardItem(text);

      if (!icon.isNull()) {
         item->setData(icon, Qt::DecorationRole);
      }

      if (userData.isValid()) {
         item->setData(userData, Qt::UserRole);
      }

      m->insertRow(index, item);
      ++itemCount;

   } else {
      d->inserting = true;
      if (d->model->insertRows(index, 1, d->root)) {
         QModelIndex item = d->model->index(index, d->modelColumn, d->root);

         if (icon.isNull() && !userData.isValid()) {
            d->model->setData(item, text, Qt::EditRole);

         } else {
            QMap<int, QVariant> values;

            if (!text.isEmpty()) {
               values.insert(Qt::EditRole, text);
            }
            if (!icon.isNull()) {
               values.insert(Qt::DecorationRole, icon);
            }
            if (userData.isValid()) {
               values.insert(Qt::UserRole, userData);
            }
            if (!values.isEmpty()) {
               d->model->setItemData(item, values);
            }
         }

         d->inserting = false;
         d->_q_rowsInserted(d->root, index, index);
         ++itemCount;

      } else {
         d->inserting = false;
      }
   }

   if (itemCount > d->maxCount) {
      d->model->removeRows(itemCount - 1, itemCount - d->maxCount, d->root);
   }
}

void QComboBox::insertItems(int index, const QStringList &list)
{
   Q_D(QComboBox);

   if (list.isEmpty()) {
      return;
   }

   index = qBound(0, index, count());
   int insertCount = qMin(d->maxCount - index, list.count());

   if (insertCount <= 0) {
      return;
   }

   // For the common case where we are using the built in QStandardItemModel
   // construct a QStandardItem, reducing the number of expensive signals from the model

   if (QStandardItemModel *m = qobject_cast<QStandardItemModel *>(d->model)) {
      QList<QStandardItem *> items;
      QStandardItem *hiddenRoot = m->invisibleRootItem();
      for (int i = 0; i < insertCount; ++i) {
         items.append(new QStandardItem(list.at(i)));
      }

      hiddenRoot->insertRows(index, items);

   } else {
      d->inserting = true;

      if (d->model->insertRows(index, insertCount, d->root)) {
         QModelIndex item;

         for (int i = 0; i < insertCount; ++i) {
            item = d->model->index(i + index, d->modelColumn, d->root);
            d->model->setData(item, list.at(i), Qt::EditRole);
         }

         d->inserting = false;
         d->_q_rowsInserted(d->root, index, index + insertCount - 1);
      } else {
         d->inserting = false;
      }
   }

   int mc = count();
   if (mc > d->maxCount) {
      d->model->removeRows(d->maxCount, mc - d->maxCount, d->root);
   }
}

void QComboBox::insertSeparator(int index)
{
   Q_D(QComboBox);

   int itemCount = count();
   index = qBound(0, index, itemCount);

   if (index >= d->maxCount) {
      return;
   }

   insertItem(index, QIcon(), QString());
   QComboBoxDelegate::setSeparator(d->model, d->model->index(index, 0, d->root));
}

void QComboBox::removeItem(int index)
{
   Q_D(QComboBox);

   if (index < 0 || index >= count()) {
      return;
   }

   d->model->removeRows(index, 1, d->root);
}

void QComboBox::setItemText(int index, const QString &text)
{
   Q_D(const QComboBox);

   QModelIndex item = d->model->index(index, d->modelColumn, d->root);

   if (item.isValid()) {
      d->model->setData(item, text, Qt::EditRole);
   }
}

void QComboBox::setItemIcon(int index, const QIcon &icon)
{
   Q_D(const QComboBox);

   QModelIndex item = d->model->index(index, d->modelColumn, d->root);

   if (item.isValid()) {
      d->model->setData(item, icon, Qt::DecorationRole);
   }
}

void QComboBox::setItemData(int index, const QVariant &value, int role)
{
   Q_D(const QComboBox);

   QModelIndex item = d->model->index(index, d->modelColumn, d->root);

   if (item.isValid()) {
      d->model->setData(item, value, role);
   }
}

QAbstractItemView *QComboBox::view() const
{
   Q_D(const QComboBox);
   return const_cast<QComboBoxPrivate *>(d)->viewContainer()->itemView();
}

void QComboBox::setView(QAbstractItemView *itemView)
{
   Q_D(QComboBox);

   if (! itemView) {
      qWarning("QComboBox::setView() Unable to set a view to an invalid value (nullptr)");
      return;
   }

   if (itemView->model() != d->model) {
      itemView->setModel(d->model);
   }

   d->viewContainer()->setItemView(itemView);
}

QSize QComboBox::minimumSizeHint() const
{
   Q_D(const QComboBox);
   return d->recomputeSizeHint(d->minimumSizeHint);
}

QSize QComboBox::sizeHint() const
{
   Q_D(const QComboBox);
   return d->recomputeSizeHint(d->sizeHint);
}

#ifdef Q_OS_DARWIN

namespace {
struct IndexSetter {
   int index;
   QComboBox *cb;

   void operator()(void) {
      cb->setCurrentIndex(index);
      emit cb->activated(index);
      emit cb->activated(cb->itemText(index));
   }
};
}

void QComboBoxPrivate::cleanupNativePopup()
{
   if (!m_platformMenu) {
      return;
   }

   int count = int(m_platformMenu->tag());
   for (int i = 0; i < count; ++i) {
      m_platformMenu->menuItemAt(i)->deleteLater();
   }

   delete m_platformMenu;
   m_platformMenu = nullptr;
}

bool QComboBoxPrivate::showNativePopup()
{
   Q_Q(QComboBox);

   cleanupNativePopup();

   QPlatformTheme *theme = QGuiApplicationPrivate::instance()->platformTheme();
   m_platformMenu = theme->createPlatformMenu();
   if (!m_platformMenu) {
      return false;
   }

   int itemsCount = q->count();
   m_platformMenu->setTag(quintptr(itemsCount));

   QPlatformMenuItem *currentItem = nullptr;
   int currentIndex = q->currentIndex();

   for (int i = 0; i < itemsCount; ++i) {
      QPlatformMenuItem *item = theme->createPlatformMenuItem();
      QModelIndex rowIndex = model->index(i, modelColumn, root);

      QVariant textVariant = model->data(rowIndex, Qt::EditRole);
      item->setText(textVariant.toString());

      QVariant iconVariant = model->data(rowIndex, Qt::DecorationRole);

      if (iconVariant.canConvert<QIcon>()) {
         item->setIcon(iconVariant.value<QIcon>());
      }
      item->setCheckable(true);
      item->setChecked(i == currentIndex);
      if (!currentItem || i == currentIndex) {
         currentItem = item;
      }

      IndexSetter setter = { i, q };
      QObject::connect(item, &QPlatformMenuItem::activated, q, setter);

      m_platformMenu->insertMenuItem(item, nullptr);
      m_platformMenu->syncMenuItem(item);
   }

   QWindow *tlw = q->window()->windowHandle();
   m_platformMenu->setFont(q->font());
   m_platformMenu->setMinimumWidth(q->rect().width());

   QPoint offset = QPoint(0, 7);

   if (q->testAttribute(Qt::WA_MacSmallSize)) {
      offset = QPoint(-1, 7);
   } else if (q->testAttribute(Qt::WA_MacMiniSize)) {
      offset = QPoint(-2, 6);
   }

   m_platformMenu->showPopup(tlw, QRect(tlw->mapFromGlobal(q->mapToGlobal(offset)), QSize()), currentItem);

#ifdef Q_OS_DARWIN
   // The Cocoa popup will swallow any mouse release event.
   // We need to fake one here to un-press the button.
   QMouseEvent mouseReleased(QEvent::MouseButtonRelease, q->pos(), Qt::LeftButton,
      Qt::MouseButtons(Qt::LeftButton), Qt::KeyboardModifiers());

   qApp->sendEvent(q, &mouseReleased);
#endif

   return true;
}

#endif // Q_OS_DARWIN

void QComboBox::showPopup()
{
   Q_D(QComboBox);

   if (count() <= 0) {
      return;
   }

   QStyle *const style = this->style();
   QStyleOptionComboBox opt;
   initStyleOption(&opt);

   const bool usePopup = style->styleHint(QStyle::SH_ComboBox_Popup, &opt, this);

#ifdef Q_OS_DARWIN
   if (usePopup && (! d->container ||
         (view()->metaObject()->className() == "QComboBoxListView" &&
          view()->itemDelegate()->metaObject()->className() == "QComboMenuDelegate"))
         && style->styleHint(QStyle::SH_ComboBox_UseNativePopup, &opt, this) && d->showNativePopup()) {
      return;
   }
#endif

#if defined(QT_KEYPAD_NAVIGATION) && ! defined(QT_NO_COMPLETER)
   if (QApplication::keypadNavigationEnabled() && d->completer) {
      // editable combo box is line edit plus completer
      setEditFocus(true);
      d->completer->complete(); // show popup

      return;
   }
#endif

   // set current item and select it
   view()->selectionModel()->setCurrentIndex(d->currentIndex, QItemSelectionModel::ClearAndSelect);
   QComboBoxPrivateContainer *container = d->viewContainer();

   QRect listRect(style->subControlRect(QStyle::CC_ComboBox, &opt,
         QStyle::SC_ComboBoxListBoxPopup, this));

   QRect screen = d->popupGeometry(QApplication::desktop()->screenNumber(this));

   QPoint below = mapToGlobal(listRect.bottomLeft());
   int belowHeight = screen.bottom() - below.y();

   QPoint above = mapToGlobal(listRect.topLeft());
   int aboveHeight = above.y() - screen.y();
   bool boundToScreen = !window()->testAttribute(Qt::WA_DontShowOnScreen);

   {
      int listHeight = 0;
      int count = 0;
      QStack<QModelIndex> toCheck;
      toCheck.push(view()->rootIndex());

#ifndef QT_NO_TREEVIEW
      QTreeView *treeView = qobject_cast<QTreeView *>(view());
      if (treeView && treeView->header() && !treeView->header()->isHidden()) {
         listHeight += treeView->header()->height();
      }
#endif

      while (! toCheck.isEmpty()) {
         QModelIndex parent = toCheck.pop();
         for (int i = 0; i < d->model->rowCount(parent); ++i) {
            QModelIndex idx = d->model->index(i, d->modelColumn, parent);
            if (!idx.isValid()) {
               continue;
            }

            listHeight += view()->visualRect(idx).height();

#ifndef QT_NO_TREEVIEW
            if (d->model->hasChildren(idx) && treeView && treeView->isExpanded(idx)) {
               toCheck.push(idx);
            }
#endif
            ++count;
            if (!usePopup && count >= d->maxVisibleItems) {
               toCheck.clear();
               break;
            }
         }
      }

      if (count > 1) {
         listHeight += (count - 1) * container->spacing();
      }

      listRect.setHeight(listHeight);
   }

   {
      // add the spacing for the grid on the top and the bottom;
      int heightMargin = container->topMargin()  + container->bottomMargin();

      // add the frame of the container
      int marginTop, marginBottom;
      container->getContentsMargins(nullptr, &marginTop, nullptr, &marginBottom);
      heightMargin += marginTop + marginBottom;

      //add the frame of the view
      view()->getContentsMargins(nullptr, &marginTop, nullptr, &marginBottom);

      marginTop += static_cast<QAbstractScrollAreaPrivate *>(QAbstractScrollAreaPrivate::get(view()))->top;
      marginBottom += static_cast<QAbstractScrollAreaPrivate *>(QAbstractScrollAreaPrivate::get(view()))->bottom;

      heightMargin += marginTop + marginBottom;
      listRect.setHeight(listRect.height() + heightMargin);
   }

   // Add space for margin at top and bottom if the style wants it.
   if (usePopup) {
      listRect.setHeight(listRect.height() + style->pixelMetric(QStyle::PM_MenuVMargin, &opt, this) * 2);
   }

   // Make sure the popup is wide enough to display its contents.
   if (usePopup) {
      const int diff = d->computeWidthHint() - width();
      if (diff > 0) {
         listRect.setWidth(listRect.width() + diff);
      }
   }

   //we need to activate the layout to make sure the min/maximum size are set when the widget was not yet show
   container->layout()->activate();

   //takes account of the minimum/maximum size of the container
   listRect.setSize( listRect.size().expandedTo(container->minimumSize())
      .boundedTo(container->maximumSize()));

   // make sure the widget fits on screen
   if (boundToScreen) {
      if (listRect.width() > screen.width() ) {
         listRect.setWidth(screen.width());
      }
      if (mapToGlobal(listRect.bottomRight()).x() > screen.right()) {
         below.setX(screen.x() + screen.width() - listRect.width());
         above.setX(screen.x() + screen.width() - listRect.width());
      }
      if (mapToGlobal(listRect.topLeft()).x() < screen.x() ) {
         below.setX(screen.x());
         above.setX(screen.x());
      }
   }

   if (usePopup) {
      // Position horizontally.
      listRect.moveLeft(above.x());

      // Position vertically so the curently selected item lines up
      // with the combo box.
      const QRect currentItemRect = view()->visualRect(view()->currentIndex());
      const int offset = listRect.top() - currentItemRect.top();
      listRect.moveTop(above.y() + offset - listRect.top());

      // Clamp the listRect height and vertical position so we don't expand outside the
      // available screen geometry.This may override the vertical position, but it is more
      // important to show as much as possible of the popup.
      const int height = !boundToScreen ? listRect.height() : qMin(listRect.height(), screen.height());
      listRect.setHeight(height);

      if (boundToScreen) {
         if (listRect.top() < screen.top()) {
            listRect.moveTop(screen.top());
         }
         if (listRect.bottom() > screen.bottom()) {
            listRect.moveBottom(screen.bottom());
         }
      }

   } else if (!boundToScreen || listRect.height() <= belowHeight) {
      listRect.moveTopLeft(below);

   } else if (listRect.height() <= aboveHeight) {
      listRect.moveBottomLeft(above);

   } else if (belowHeight >= aboveHeight) {
      listRect.setHeight(belowHeight);
      listRect.moveTopLeft(below);

   } else {
      listRect.setHeight(aboveHeight);
      listRect.moveBottomLeft(above);
   }

   if (qApp) {
      QGuiApplication::inputMethod()->reset();
   }

   QScrollBar *sb = view()->horizontalScrollBar();
   Qt::ScrollBarPolicy policy = view()->horizontalScrollBarPolicy();

   bool needHorizontalScrollBar = (policy == Qt::ScrollBarAsNeeded || policy == Qt::ScrollBarAlwaysOn)
      && sb->minimum() < sb->maximum();

   if (needHorizontalScrollBar) {
      listRect.adjust(0, 0, 0, sb->height());
   }

   container->setGeometry(listRect);

#ifndef Q_OS_DARWIN
   const bool updatesEnabled = container->updatesEnabled();
#endif

#if ! defined(QT_NO_EFFECTS)
   bool scrollDown = (listRect.topLeft() == below);

   if (QApplication::isEffectEnabled(Qt::UI_AnimateCombo)
      && !style->styleHint(QStyle::SH_ComboBox_Popup, &opt, this) && !window()->testAttribute(Qt::WA_DontShowOnScreen)) {
      qScrollEffect(container, scrollDown ? QEffects::DownScroll : QEffects::UpScroll, 150);
   }
#endif

   // Don't disable updates on Mac OS X. Windows are displayed immediately on this platform,
   // which means that the window will be visible before the call to container->show() returns.
   // If updates are disabled at this point we'll miss our chance at painting the popup
   // menu before it's shown, causing flicker since the window then displays the standard gray background.
#ifndef Q_OS_DARWIN
   container->setUpdatesEnabled(false);
#endif

   bool startTimer = ! container->isVisible();
   container->raise();
   container->show();
   container->updateScrollers();
   view()->setFocus();

   view()->scrollTo(view()->currentIndex(),
      style->styleHint(QStyle::SH_ComboBox_Popup, &opt, this)
      ? QAbstractItemView::PositionAtCenter
      : QAbstractItemView::EnsureVisible);

#ifndef Q_OS_DARWIN
   container->setUpdatesEnabled(updatesEnabled);
#endif

   container->update();

#ifdef QT_KEYPAD_NAVIGATION
   if (QApplication::keypadNavigationEnabled()) {
      view()->setEditFocus(true);
   }
#endif

   if (startTimer) {
      container->popupTimer.start();
      container->maybeIgnoreMouseButtonRelease = true;
   }
}

void QComboBox::hidePopup()
{
   Q_D(QComboBox);

   if (d->container && d->container->isVisible()) {

#if ! defined(QT_NO_EFFECTS)
      d->model->blockSignals(true);
      d->container->itemView()->blockSignals(true);
      d->container->blockSignals(true);

      // Flash selected/triggered item (if any)
      if (style()->styleHint(QStyle::SH_Menu_FlashTriggeredItem)) {
         QItemSelectionModel *selectionModel = view() ? view()->selectionModel() : nullptr;

         if (selectionModel && selectionModel->hasSelection()) {
            QEventLoop eventLoop;
            const QItemSelection selection = selectionModel->selection();

            // Deselect item and wait 60 ms.
            selectionModel->select(selection, QItemSelectionModel::Toggle);
            QTimer::singleShot(60, &eventLoop, SLOT(quit()));
            eventLoop.exec();

            // Select item and wait 20 ms.
            selectionModel->select(selection, QItemSelectionModel::Toggle);
            QTimer::singleShot(20, &eventLoop, SLOT(quit()));
            eventLoop.exec();
         }
      }

      // Fade out
      bool needFade = style()->styleHint(QStyle::SH_Menu_FadeOutOnHide);
      bool didFade = false;

      if (needFade) {

#if defined(Q_OS_DARWIN)
         QPlatformNativeInterface *platformNativeInterface = qApp->platformNativeInterface();
         int at = platformNativeInterface->metaObject()->indexOfMethod("fadeWindow()");

         if (at != -1) {
            QMetaMethod windowFade = platformNativeInterface->metaObject()->method(at);
            windowFade.invoke(platformNativeInterface, Q_ARG(QWindow *, d->container->windowHandle()));
            didFade = true;
         }

#endif
         // Other platform implementations welcome
      }

      d->model->blockSignals(false);
      d->container->itemView()->blockSignals(false);
      d->container->blockSignals(false);

      if (! didFade)
#endif // QT_NO_EFFECTS

         // Fade should implicitly hide as well ;-)
         d->container->hide();
   }

#ifdef QT_KEYPAD_NAVIGATION
   if (QApplication::keypadNavigationEnabled() && isEditable() && hasFocus()) {
      setEditFocus(true);
   }
#endif

   d->_q_resetButton();
}

void QComboBox::clear()
{
   Q_D(QComboBox);
   d->model->removeRows(0, d->model->rowCount(d->root), d->root);

#ifndef QT_NO_ACCESSIBILITY
   QAccessibleValueChangeEvent event(this, QString());
   QAccessible::updateAccessibility(&event);
#endif
}

void QComboBox::clearEditText()
{
   Q_D(QComboBox);

   if (d->lineEdit) {
      d->lineEdit->clear();
   }

#ifndef QT_NO_ACCESSIBILITY
   QAccessibleValueChangeEvent event(this, QString());
   QAccessible::updateAccessibility(&event);
#endif
}

void QComboBox::setEditText(const QString &text)
{
   Q_D(QComboBox);

   if (d->lineEdit) {
      d->lineEdit->setText(text);
   }

#ifndef QT_NO_ACCESSIBILITY
   QAccessibleValueChangeEvent event(this, text);
   QAccessible::updateAccessibility(&event);
#endif
}

void QComboBox::focusInEvent(QFocusEvent *e)
{
   Q_D(QComboBox);
   update();

   if (d->lineEdit) {
      d->lineEdit->event(e);

#ifndef QT_NO_COMPLETER
      if (d->lineEdit->completer()) {
         d->lineEdit->completer()->setWidget(this);
      }
#endif

   }
}

void QComboBox::focusOutEvent(QFocusEvent *e)
{
   Q_D(QComboBox);

   update();
   if (d->lineEdit) {
      d->lineEdit->event(e);
   }
}

void QComboBox::changeEvent(QEvent *e)
{
   Q_D(QComboBox);

   switch (e->type()) {
      case QEvent::StyleChange:
         d->updateDelegate();

#ifdef Q_OS_DARWIN
      case QEvent::MacSizeChange:
#endif
         d->sizeHint = QSize(); // invalidate size hint
         d->minimumSizeHint = QSize();
         d->updateLayoutDirection();
         if (d->lineEdit) {
            d->updateLineEditGeometry();
         }
         d->setLayoutItemMargins(QStyle::SE_ComboBoxLayoutItem);

         if (e->type() == QEvent::MacSizeChange) {
            QPlatformTheme::Font f = QPlatformTheme::SystemFont;

            if (testAttribute(Qt::WA_MacSmallSize)) {
               f = QPlatformTheme::SmallFont;
            } else if (testAttribute(Qt::WA_MacMiniSize)) {
               f = QPlatformTheme::MiniFont;
            }

            if (const QFont *platformFont = QApplicationPrivate::platformTheme()->font(f)) {
               QFont f = font();
               f.setPointSizeF(platformFont->pointSizeF());
               setFont(f);
            }
         }

         // ### need to update scrollers as well
         break;

      case QEvent::EnabledChange:
         if (! isEnabled()) {
            hidePopup();
         }
         break;

      case QEvent::PaletteChange: {
         d->updateViewContainerPaletteAndOpacity();
         break;
      }

      case QEvent::FontChange:
         d->sizeHint = QSize(); // invalidate size hint
         d->viewContainer()->setFont(font());

         if (d->lineEdit) {
            d->updateLineEditGeometry();
         }
         break;

      default:
         break;
   }

   QWidget::changeEvent(e);
}

void QComboBox::resizeEvent(QResizeEvent *)
{
   Q_D(QComboBox);
   d->updateLineEditGeometry();
}

void QComboBox::paintEvent(QPaintEvent *)
{
   QStylePainter painter(this);
   painter.setPen(palette().color(QPalette::Text));

   // draw the combobox frame, focusrect and selected etc.
   QStyleOptionComboBox opt;
   initStyleOption(&opt);
   painter.drawComplexControl(QStyle::CC_ComboBox, opt);

   // draw the icon and text
   painter.drawControl(QStyle::CE_ComboBoxLabel, opt);
}

void QComboBox::showEvent(QShowEvent *e)
{
   Q_D(QComboBox);
   if (!d->shownOnce && d->sizeAdjustPolicy == QComboBox::AdjustToContentsOnFirstShow) {
      d->sizeHint = QSize();
      updateGeometry();
   }
   d->shownOnce = true;
   QWidget::showEvent(e);
}

void QComboBox::hideEvent(QHideEvent *)
{
   hidePopup();
}

bool QComboBox::event(QEvent *event)
{
   Q_D(QComboBox);

   switch (event->type()) {
      case QEvent::LayoutDirectionChange:
      case QEvent::ApplicationLayoutDirectionChange:
         d->updateLayoutDirection();
         d->updateLineEditGeometry();
         break;

      case QEvent::HoverEnter:
      case QEvent::HoverLeave:
      case QEvent::HoverMove:
         if (const QHoverEvent *he = static_cast<const QHoverEvent *>(event)) {
            d->updateHoverControl(he->pos());
         }
         break;

      case QEvent::ShortcutOverride:
         if (d->lineEdit) {
            return d->lineEdit->event(event);
         }
         break;

#ifdef QT_KEYPAD_NAVIGATION
      case QEvent::EnterEditFocus:
         if (!d->lineEdit) {
            setEditFocus(false);         // We never want edit focus if we are not editable
         } else {
            d->lineEdit->event(event);   // so cursor starts
         }
         break;

      case QEvent::LeaveEditFocus:
         if (d->lineEdit) {
            d->lineEdit->event(event);   // so cursor stops
         }
         break;
#endif

      default:
         break;
   }

   return QWidget::event(event);
}

void QComboBox::mousePressEvent(QMouseEvent *e)
{
   Q_D(QComboBox);

   if (! QGuiApplication::styleHints()->setFocusOnTouchRelease()) {
      d->showPopupFromMouseEvent(e);
   }
}

void QComboBoxPrivate::showPopupFromMouseEvent(QMouseEvent *e)
{
   Q_Q(QComboBox);

   QStyleOptionComboBox opt;
   q->initStyleOption(&opt);

   QStyle::SubControl sc = q->style()->hitTestComplexControl(QStyle::CC_ComboBox, &opt, e->pos(), q);

   if (e->button() == Qt::LeftButton
      && ! (sc == QStyle::SC_None && e->type() == QEvent::MouseButtonRelease)
      && (sc == QStyle::SC_ComboBoxArrow || !q->isEditable())
      && ! viewContainer()->isVisible()) {

      if (sc == QStyle::SC_ComboBoxArrow) {
         updateArrow(QStyle::State_Sunken);
      }

#ifdef QT_KEYPAD_NAVIGATION
      // if the container already exists then d->viewContainer() is safe to call

      if (container)
#endif

      {
         // We have restricted the next couple of lines, because by not calling
         // viewContainer(), we avoid creating the QComboBoxPrivateContainer.
         viewContainer()->blockMouseReleaseTimer.start(QApplication::doubleClickInterval());
         viewContainer()->initialClickPosition = q->mapToGlobal(e->pos());
      }

      q->showPopup();

      // The code below ensures that regular mousepress and pick item still works
      // If it was not called the viewContainer would ignore event since it didn't have
      // a mousePressEvent first.

      if (viewContainer()) {
         viewContainer()->maybeIgnoreMouseButtonRelease = false;
      }

   } else {

#ifdef QT_KEYPAD_NAVIGATION
      if (QApplication::keypadNavigationEnabled() && sc == QStyle::SC_ComboBoxEditField && lineEdit) {
         lineEdit->event(e);  //so lineedit can move cursor, etc
         return;
      }
#endif

      e->ignore();
   }
}

void QComboBox::mouseReleaseEvent(QMouseEvent *e)
{
   Q_D(QComboBox);

   d->updateArrow(QStyle::State_None);
   if (QGuiApplication::styleHints()->setFocusOnTouchRelease() && hasFocus()) {
      d->showPopupFromMouseEvent(e);
   }
}

void QComboBox::keyPressEvent(QKeyEvent *e)
{
   Q_D(QComboBox);

#ifndef QT_NO_COMPLETER
   if (d->lineEdit && d->lineEdit->completer() && d->lineEdit->completer()->popup()
         && d->lineEdit->completer()->popup()->isVisible()) {
      // provide same autocompletion support as line edit
      d->lineEdit->event(e);
      return;
   }
#endif

   enum Move { NoMove = 0, MoveUp, MoveDown, MoveFirst, MoveLast};

   Move move = NoMove;
   int newIndex = currentIndex();

   switch (e->key()) {
      case Qt::Key_Up:
         if (e->modifiers() & Qt::ControlModifier) {
            break;   // pass to line edit for auto completion
         }
         [[fallthrough]];

      case Qt::Key_PageUp:
#ifdef QT_KEYPAD_NAVIGATION
         if (QApplication::keypadNavigationEnabled()) {
            e->ignore();
         } else
#endif
            move = MoveUp;
         break;

      case Qt::Key_Down:
         if (e->modifiers() & Qt::AltModifier) {
            showPopup();
            return;
         } else if (e->modifiers() & Qt::ControlModifier) {
            break;   // pass to line edit for auto completion
         }
         [[fallthrough]];

      case Qt::Key_PageDown:
#ifdef QT_KEYPAD_NAVIGATION
         if (QApplication::keypadNavigationEnabled()) {
            e->ignore();
         } else
#endif
            move = MoveDown;
         break;

      case Qt::Key_Home:
         if (!d->lineEdit) {
            move = MoveFirst;
         }
         break;

      case Qt::Key_End:
         if (!d->lineEdit) {
            move = MoveLast;
         }
         break;

      case Qt::Key_F4:
         if (!e->modifiers()) {
            showPopup();
            return;
         }
         break;

      case Qt::Key_Space:
         if (!d->lineEdit) {
            showPopup();
            return;
         }
         break;

      case Qt::Key_Enter:
      case Qt::Key_Return:
      case Qt::Key_Escape:
         if (!d->lineEdit) {
            e->ignore();
         }
         break;

#ifdef QT_KEYPAD_NAVIGATION
      case Qt::Key_Select:
         if (QApplication::keypadNavigationEnabled()
            && (!hasEditFocus() || !d->lineEdit)) {
            showPopup();
            return;
         }
         break;

      case Qt::Key_Left:
      case Qt::Key_Right:
         if (QApplication::keypadNavigationEnabled() && !hasEditFocus()) {
            e->ignore();
         }
         break;

      case Qt::Key_Back:
         if (QApplication::keypadNavigationEnabled()) {
            if (!hasEditFocus() || !d->lineEdit) {
               e->ignore();
            }
         } else {
            e->ignore(); // let the surounding dialog have it
         }
         break;
#endif

      default:
         if (!d->lineEdit) {
            if (!e->text().isEmpty()) {
               d->keyboardSearchString(e->text());
            } else {
               e->ignore();
            }
         }
   }

   if (move != NoMove) {
      e->accept();

      switch (move) {
         case MoveFirst:
            newIndex = -1;
            [[fallthrough]];

         case MoveDown:
            ++newIndex;

            while ((newIndex < count()) && !(d->model->flags(d->model->index(newIndex, d->modelColumn, d->root))
                     & Qt::ItemIsEnabled)) {
               ++newIndex;
            }
            break;

         case MoveLast:
            newIndex = count();
            [[fallthrough]];

         case MoveUp:
            --newIndex;

            while ((newIndex >= 0) && !(d->model->flags(d->model->index(newIndex, d->modelColumn, d->root)) & Qt::ItemIsEnabled)) {
               --newIndex;
            }
            break;

         default:
            e->ignore();
            break;
      }

      if (newIndex >= 0 && newIndex < count() && newIndex != currentIndex()) {
         setCurrentIndex(newIndex);
         d->emitActivated(d->currentIndex);
      }

   } else if (d->lineEdit) {
      d->lineEdit->event(e);
   }
}

void QComboBox::keyReleaseEvent(QKeyEvent *e)
{
   Q_D(QComboBox);

   if (d->lineEdit) {
      d->lineEdit->event(e);
   } else {
      QWidget::keyReleaseEvent(e);
   }
}

#ifndef QT_NO_WHEELEVENT
void QComboBox::wheelEvent(QWheelEvent *e)
{
#ifdef Q_OS_DARWIN
   // no code here
   (void) e;

#else
   Q_D(QComboBox);
   if (!d->viewContainer()->isVisible()) {
      int newIndex = currentIndex();

      if (e->delta() > 0) {
         --newIndex;

         while ((newIndex >= 0) &&
               ! (d->model->flags(d->model->index(newIndex, d->modelColumn, d->root)) & Qt::ItemIsEnabled)) {
            --newIndex;
         }

      } else if (e->delta() < 0) {
         ++newIndex;

         while ((newIndex < count()) &&
               ! (d->model->flags(d->model->index(newIndex, d->modelColumn, d->root)) & Qt::ItemIsEnabled)) {
            ++newIndex;
         }
      }

      if (newIndex >= 0 && newIndex < count() && newIndex != currentIndex()) {
         setCurrentIndex(newIndex);
         d->emitActivated(d->currentIndex);
      }

      e->accept();
   }
#endif
}

#endif      // QT_NO_WHEELEVENT

#ifndef QT_NO_CONTEXTMENU
void QComboBox::contextMenuEvent(QContextMenuEvent *e)
{
   Q_D(QComboBox);

   if (d->lineEdit) {
      Qt::ContextMenuPolicy p = d->lineEdit->contextMenuPolicy();
      d->lineEdit->setContextMenuPolicy(Qt::DefaultContextMenu);
      d->lineEdit->event(e);
      d->lineEdit->setContextMenuPolicy(p);
   }
}
#endif

void QComboBoxPrivate::keyboardSearchString(const QString &text)
{
   // use keyboardSearch from the listView so we do not duplicate code
   QAbstractItemView *view = viewContainer()->itemView();
   view->setCurrentIndex(currentIndex);
   int currentRow = view->currentIndex().row();
   view->keyboardSearch(text);
   if (currentRow != view->currentIndex().row()) {
      setCurrentIndex(view->currentIndex());
      emitActivated(currentIndex);
   }
}

void QComboBoxPrivate::modelChanged()
{
   Q_Q(QComboBox);

   if (sizeAdjustPolicy == QComboBox::AdjustToContents) {
      sizeHint = QSize();
      adjustComboBoxSize();
      q->updateGeometry();
   }
}

void QComboBox::inputMethodEvent(QInputMethodEvent *e)
{
   Q_D(QComboBox);

   if (d->lineEdit) {
      d->lineEdit->event(e);
   } else {
      if (!e->commitString().isEmpty()) {
         d->keyboardSearchString(e->commitString());
      } else {
         e->ignore();
      }
   }
}

QVariant QComboBox::inputMethodQuery(Qt::InputMethodQuery query) const
{
   Q_D(const QComboBox);

   if (d->lineEdit) {
      return d->lineEdit->inputMethodQuery(query);
   }

   return QWidget::inputMethodQuery(query);
}

bool QComboBox::hasFrame() const
{
   Q_D(const QComboBox);
   return d->frame;
}

void QComboBox::setFrame(bool enable)
{
   Q_D(QComboBox);

   d->frame = enable;
   update();
   updateGeometry();
}

int QComboBox::modelColumn() const
{
   Q_D(const QComboBox);
   return d->modelColumn;
}

void QComboBox::setModelColumn(int visibleColumn)
{
   Q_D(QComboBox);

   d->modelColumn = visibleColumn;
   QListView *lv = qobject_cast<QListView *>(d->viewContainer()->itemView());
   if (lv) {
      lv->setModelColumn(visibleColumn);
   }

#ifndef QT_NO_COMPLETER
   if (d->lineEdit && d->lineEdit->completer()
      && d->lineEdit->completer() == d->completer) {
      d->lineEdit->completer()->setCompletionColumn(visibleColumn);
   }
#endif

   setCurrentIndex(currentIndex()); //update the text to the text of the new column;
}

void QComboBox::_q_itemSelected(const QModelIndex &index)
{
   Q_D(QComboBox);
   d->_q_itemSelected(index);
}

void QComboBox::_q_emitHighlighted(const QModelIndex &index)
{
   Q_D(QComboBox);
   d->_q_emitHighlighted(index);
}

void QComboBox::_q_emitCurrentIndexChanged(const QModelIndex &index)
{
   Q_D(QComboBox);
   d->_q_emitCurrentIndexChanged(index);
}

void QComboBox::_q_editingFinished()
{
   Q_D(QComboBox);
   d->_q_editingFinished();
}

void QComboBox::_q_returnPressed()
{
   Q_D(QComboBox);
   d->_q_returnPressed();
}

void QComboBox::_q_resetButton()
{
   Q_D(QComboBox);
   d->_q_resetButton();
}

void QComboBox::_q_dataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight)
{
   Q_D(QComboBox);
   d->_q_dataChanged(topLeft, bottomRight);
}

void QComboBox::_q_updateIndexBeforeChange()
{
   Q_D(QComboBox);
   d->_q_updateIndexBeforeChange();
}

void QComboBox::_q_rowsInserted(const QModelIndex &parent, int start, int end)
{
   Q_D(QComboBox);
   d->_q_rowsInserted(parent, start, end);
}

void QComboBox::_q_rowsRemoved(const QModelIndex &parent, int start, int end)
{
   Q_D(QComboBox);
   d->_q_rowsRemoved(parent, start, end);
}

void QComboBox::_q_modelDestroyed()
{
   Q_D(QComboBox);
   d->_q_modelDestroyed();
}

void QComboBox::_q_modelReset()
{
   Q_D(QComboBox);
   d->_q_modelReset();
}

#ifndef QT_NO_COMPLETER
void QComboBox::_q_completerActivated(const QModelIndex &index)
{
   Q_D(QComboBox);
   d->_q_completerActivated(index);
}
#endif

#endif // QT_NO_COMBOBOX
