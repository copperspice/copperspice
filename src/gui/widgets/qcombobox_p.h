/***********************************************************************
*
* Copyright (c) 2012-2014 Barbara Geller
* Copyright (c) 2012-2014 Ansel Sermersheim
* Copyright (c) 2012-2014 Digia Plc and/or its subsidiary(-ies).
* Copyright (c) 2008-2012 Nokia Corporation and/or its subsidiary(-ies).
* All rights reserved.
*
* This file is part of CopperSpice.
*
* CopperSpice is free software: you can redistribute it and/or
* modify it under the terms of the GNU Lesser General Public License
* version 2.1 as published by the Free Software Foundation.
*
* CopperSpice is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
* Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public
* License along with CopperSpice.  If not, see
* <http://www.gnu.org/licenses/>.
*
***********************************************************************/

#ifndef QCOMBOBOX_P_H
#define QCOMBOBOX_P_H

#include <QtGui/qcombobox.h>

#ifndef QT_NO_COMBOBOX
#include <QtGui/qabstractslider.h>
#include <QtGui/qapplication.h>
#include <QtGui/qitemdelegate.h>
#include <QtGui/qstandarditemmodel.h>
#include <QtGui/qlineedit.h>
#include <QtGui/qlistview.h>
#include <QtGui/qpainter.h>
#include <QtGui/qstyle.h>
#include <QtGui/qstyleoption.h>
#include <QtCore/qhash.h>
#include <QtCore/qpair.h>
#include <QtCore/qtimer.h>
#include <qwidget_p.h>
#include <QtCore/qpointer.h>
#include <QtGui/qcompleter.h>
#include <QtGui/qevent.h>
#include <QtCore/qdebug.h>

#include <limits.h>

QT_BEGIN_NAMESPACE

class QAction;
class QStandardItemModel;

class QComboBoxListView : public QListView
{
   CS_OBJECT(QComboBoxListView)

 public:
   QComboBoxListView(QComboBox *cmb = 0) : combo(cmb) {}

 protected:
   void resizeEvent(QResizeEvent *event) {
      resizeContents(viewport()->width(), contentsSize().height());
      QListView::resizeEvent(event);
   }

   QStyleOptionViewItem viewOptions() const {
      QStyleOptionViewItem option = QListView::viewOptions();
      option.showDecorationSelected = true;
      if (combo) {
         option.font = combo->font();
      }
      return option;
   }

   void paintEvent(QPaintEvent *e) {
      if (combo) {
         QStyleOptionComboBox opt;
         opt.initFrom(combo);
         opt.editable = combo->isEditable();
         if (combo->style()->styleHint(QStyle::SH_ComboBox_Popup, &opt, combo)) {
            //we paint the empty menu area to avoid having blank space that can happen when scrolling
            QStyleOptionMenuItem menuOpt;
            menuOpt.initFrom(this);
            menuOpt.palette = palette();
            menuOpt.state = QStyle::State_None;
            menuOpt.checkType = QStyleOptionMenuItem::NotCheckable;
            menuOpt.menuRect = e->rect();
            menuOpt.maxIconWidth = 0;
            menuOpt.tabWidth = 0;
            QPainter p(viewport());
            combo->style()->drawControl(QStyle::CE_MenuEmptyArea, &menuOpt, &p, this);
         }
      }
      QListView::paintEvent(e);
   }

 private:
   QComboBox *combo;
};

class QComboBoxPrivateScroller : public QWidget
{
   CS_OBJECT(QComboBoxPrivateScroller)

 public:
   QComboBoxPrivateScroller(QAbstractSlider::SliderAction action, QWidget *parent)
      : QWidget(parent), sliderAction(action) {
      setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
      setAttribute(Qt::WA_NoMousePropagation);
   }
   QSize sizeHint() const {
      return QSize(20, style()->pixelMetric(QStyle::PM_MenuScrollerHeight));
   }

   GUI_CS_SIGNAL_1(Public, void doScroll(int action))
   GUI_CS_SIGNAL_2(doScroll, action)

 protected:
   inline void stopTimer() {
      timer.stop();
   }

   inline void startTimer() {
      timer.start(100, this);
      fast = false;
   }

   void enterEvent(QEvent *) {
      startTimer();
   }

   void leaveEvent(QEvent *) {
      stopTimer();
   }
   void timerEvent(QTimerEvent *e) {
      if (e->timerId() == timer.timerId()) {
         emit doScroll(sliderAction);
         if (fast) {
            emit doScroll(sliderAction);
            emit doScroll(sliderAction);
         }
      }
   }
   void hideEvent(QHideEvent *) {
      stopTimer();
   }

   void mouseMoveEvent(QMouseEvent *e) {
      // Enable fast scrolling if the cursor is directly above or below the popup.
      const int mouseX = e->pos().x();
      const int mouseY = e->pos().y();
      const bool horizontallyInside = pos().x() < mouseX && mouseX < rect().right() + 1;
      const bool verticallyOutside = (sliderAction == QAbstractSlider::SliderSingleStepAdd) ?
                                     rect().bottom() + 1 < mouseY : mouseY < pos().y();

      fast = horizontallyInside && verticallyOutside;
   }

   void paintEvent(QPaintEvent *) {
      QPainter p(this);
      QStyleOptionMenuItem menuOpt;
      menuOpt.init(this);
      menuOpt.checkType = QStyleOptionMenuItem::NotCheckable;
      menuOpt.menuRect = rect();
      menuOpt.maxIconWidth = 0;
      menuOpt.tabWidth = 0;
      menuOpt.menuItemType = QStyleOptionMenuItem::Scroller;

      if (sliderAction == QAbstractSlider::SliderSingleStepAdd) {
         menuOpt.state |= QStyle::State_DownArrow;
      }

      p.eraseRect(rect());
      style()->drawControl(QStyle::CE_MenuScroller, &menuOpt, &p);
   }
 
 private:
   QAbstractSlider::SliderAction sliderAction;
   QBasicTimer timer;
   bool fast;
};

class QComboBoxPrivateContainer : public QFrame
{
   CS_OBJECT(QComboBoxPrivateContainer)

 public:
   QComboBoxPrivateContainer(QAbstractItemView *itemView, QComboBox *parent);
   QAbstractItemView *itemView() const;
   void setItemView(QAbstractItemView *itemView);
   int spacing() const;
   void updateTopBottomMargin();

   QTimer blockMouseReleaseTimer;
   QBasicTimer adjustSizeTimer;
   QPoint initialClickPosition;
 
   GUI_CS_SLOT_1(Public, void scrollItemView(int action))
   GUI_CS_SLOT_2(scrollItemView)
   GUI_CS_SLOT_1(Public, void updateScrollers())
   GUI_CS_SLOT_2(updateScrollers)
   GUI_CS_SLOT_1(Public, void viewDestroyed())
   GUI_CS_SLOT_2(viewDestroyed)

   GUI_CS_SIGNAL_1(Public, void itemSelected(const QModelIndex &un_named_arg1))
   GUI_CS_SIGNAL_2(itemSelected, un_named_arg1)
   GUI_CS_SIGNAL_1(Public, void resetButton())
   GUI_CS_SIGNAL_2(resetButton)

 protected:
   void changeEvent(QEvent *e);
   bool eventFilter(QObject *o, QEvent *e);
   void mousePressEvent(QMouseEvent *e);
   void mouseReleaseEvent(QMouseEvent *e);
   void showEvent(QShowEvent *e);
   void hideEvent(QHideEvent *e);
   void timerEvent(QTimerEvent *timerEvent);
   void leaveEvent(QEvent *e);
   void resizeEvent(QResizeEvent *e);
   QStyleOptionComboBox comboStyleOption() const;
 
 private:
   QComboBox *combo;
   QAbstractItemView *view;
   QComboBoxPrivateScroller *top;
   QComboBoxPrivateScroller *bottom;

#ifdef QT_SOFTKEYS_ENABLED
   QAction *selectAction;
   QAction *cancelAction;
#endif
};

class QComboMenuDelegate : public QAbstractItemDelegate
{
   CS_OBJECT(QComboMenuDelegate)

 public:
   QComboMenuDelegate(QObject *parent, QComboBox *cmb) : QAbstractItemDelegate(parent), mCombo(cmb) {}

 protected:
   void paint(QPainter *painter,
              const QStyleOptionViewItem &option,
              const QModelIndex &index) const {

      QStyleOptionMenuItem opt = getStyleOption(option, index);
      painter->fillRect(option.rect, opt.palette.background());

      mCombo->style()->drawControl(QStyle::CE_MenuItem, &opt, painter, mCombo);
   }
   QSize sizeHint(const QStyleOptionViewItem &option,
                  const QModelIndex &index) const {
      QStyleOptionMenuItem opt = getStyleOption(option, index);
      return mCombo->style()->sizeFromContents(QStyle::CT_MenuItem, &opt, option.rect.size(), mCombo);
   }

 private:
   QStyleOptionMenuItem getStyleOption(const QStyleOptionViewItem &option, const QModelIndex &index) const;
   QComboBox *mCombo;
};

// Note that this class is intentionally not using QStyledItemDelegate
// Vista does not use the new theme for combo boxes and there might
// be other side effects from using the new class
class QComboBoxDelegate : public QItemDelegate
{
   CS_OBJECT(QComboBoxDelegate)

 public:
   QComboBoxDelegate(QObject *parent, QComboBox *cmb) : QItemDelegate(parent), mCombo(cmb) {}

   static bool isSeparator(const QModelIndex &index) {
      return index.data(Qt::AccessibleDescriptionRole).toString() == QLatin1String("separator");
   }
   static void setSeparator(QAbstractItemModel *model, const QModelIndex &index) {
      model->setData(index, QString::fromLatin1("separator"), Qt::AccessibleDescriptionRole);
      if (QStandardItemModel *m = qobject_cast<QStandardItemModel *>(model))
         if (QStandardItem *item = m->itemFromIndex(index)) {
            item->setFlags(item->flags() & ~(Qt::ItemIsSelectable | Qt::ItemIsEnabled));
         }
   }

 protected:
   void paint(QPainter *painter,
              const QStyleOptionViewItem &option,
              const QModelIndex &index) const {
      if (isSeparator(index)) {
         QRect rect = option.rect;
         if (const QStyleOptionViewItemV3 *v3 = qstyleoption_cast<const QStyleOptionViewItemV3 *>(&option))
            if (const QAbstractItemView *view = qobject_cast<const QAbstractItemView *>(v3->widget)) {
               rect.setWidth(view->viewport()->width());
            }
         QStyleOption opt;
         opt.rect = rect;
         mCombo->style()->drawPrimitive(QStyle::PE_IndicatorToolBarSeparator, &opt, painter, mCombo);
      } else {
         QItemDelegate::paint(painter, option, index);
      }
   }

   QSize sizeHint(const QStyleOptionViewItem &option,
                  const QModelIndex &index) const {
      if (isSeparator(index)) {
         int pm = mCombo->style()->pixelMetric(QStyle::PM_DefaultFrameWidth, 0, mCombo);
         return QSize(pm, pm);
      }
      return QItemDelegate::sizeHint(option, index);
   }
 private:
   QComboBox *mCombo;
};

class QComboBoxPrivate : public QWidgetPrivate
{
   Q_DECLARE_PUBLIC(QComboBox)

 public:
   QComboBoxPrivate();
   ~QComboBoxPrivate() {}
   void init();
   QComboBoxPrivateContainer *viewContainer();
   void updateLineEditGeometry();
   Qt::MatchFlags matchFlags() const;
   void _q_editingFinished();
   void _q_returnPressed();
   void _q_complete();
   void _q_itemSelected(const QModelIndex &item);
   bool contains(const QString &text, int role);
   void emitActivated(const QModelIndex &);
   void _q_emitHighlighted(const QModelIndex &);
   void _q_emitCurrentIndexChanged(const QModelIndex &index);
   void _q_modelDestroyed();
   void _q_modelReset();

#ifdef QT_KEYPAD_NAVIGATION
   void _q_completerActivated();
#endif

   void _q_resetButton();
   void _q_dataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight);
   void _q_updateIndexBeforeChange();
   void _q_rowsInserted(const QModelIndex &parent, int start, int end);
   void _q_rowsRemoved(const QModelIndex &parent, int start, int end);
   void updateArrow(QStyle::StateFlag state);
   bool updateHoverControl(const QPoint &pos);
   QRect popupGeometry(int screen = -1) const;
   QStyle::SubControl newHoverControl(const QPoint &pos);
   int computeWidthHint() const;
   QSize recomputeSizeHint(QSize &sh) const;
   void adjustComboBoxSize();
   QString itemText(const QModelIndex &index) const;
   QIcon itemIcon(const QModelIndex &index) const;
   int itemRole() const;
   void updateLayoutDirection();
   void setCurrentIndex(const QModelIndex &index);
   void updateDelegate(bool force = false);
   void keyboardSearchString(const QString &text);
   void modelChanged();
   void updateViewContainerPaletteAndOpacity();

   QAbstractItemModel *model;
   QLineEdit *lineEdit;
   QComboBoxPrivateContainer *container;
   QComboBox::InsertPolicy insertPolicy;
   QComboBox::SizeAdjustPolicy sizeAdjustPolicy;
   int minimumContentsLength;
   QSize iconSize;
   uint shownOnce : 1;
   uint autoCompletion : 1;
   uint duplicatesEnabled : 1;
   uint frame : 1;
   uint padding : 26;
   int maxVisibleItems;
   int maxCount;
   int modelColumn;
   bool inserting;
   mutable QSize minimumSizeHint;
   mutable QSize sizeHint;
   QStyle::StateFlag arrowState;
   QStyle::SubControl hoverControl;
   QRect hoverRect;
   QPersistentModelIndex currentIndex;
   QPersistentModelIndex root;
   Qt::CaseSensitivity autoCompletionCaseSensitivity;
   int indexBeforeChange;

#ifndef QT_NO_COMPLETER
   QPointer<QCompleter> completer;
#endif

   static QPalette viewContainerPalette(QComboBox *cmb) {
      return cmb->d_func()->viewContainer()->palette();
   }
};

QT_END_NAMESPACE

#endif // QT_NO_COMBOBOX

#endif // QCOMBOBOX_P_H
