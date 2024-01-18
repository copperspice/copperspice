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

#ifndef QTABBAR_P_H
#define QTABBAR_P_H

#include <qtabbar.h>
#include <qwidget_p.h>
#include <qicon.h>
#include <qtoolbutton.h>
#include <qdebug.h>
#include <qvariantanimation.h>

#ifndef QT_NO_TABBAR

#define ANIMATION_DURATION 250

#include <qstyleoption.h>

class QMovableTabWidget : public QWidget
{
 public:
   explicit QMovableTabWidget(QWidget *parent = nullptr);
   void setPixmap(const QPixmap &pixmap);

 protected:
   void paintEvent(QPaintEvent *e) override;

 private:
   QPixmap m_pixmap;
};

class QTabBarPrivate  : public QWidgetPrivate
{
   Q_DECLARE_PUBLIC(QTabBar)

 public:
   QTabBarPrivate()
      : currentIndex(-1), pressedIndex(-1), shape(QTabBar::RoundedNorth), layoutDirty(false),
        drawBase(true), scrollOffset(0), elideModeSetByUser(false), useScrollButtonsSetByUser(false),
        expanding(true), closeButtonOnTabs(false),
        selectionBehaviorOnRemove(QTabBar::SelectRightTab), paintWithOffsets(true), movable(false),
        dragInProgress(false), documentMode(false), autoHide(false), changeCurrentOnDrag(false),
        switchTabCurrentIndex(-1), switchTabTimerId(0), movingTab(nullptr)
   {}

   int currentIndex;
   int pressedIndex;
   QTabBar::Shape shape;
   bool layoutDirty;
   bool drawBase;
   int scrollOffset;

   struct Tab : public QEnableSharedFromThis<Tab> {
      Tab(const QIcon &ico, const QString &txt)
         : enabled(true), shortcutId(0), text(txt), icon(ico), leftWidget(nullptr), rightWidget(nullptr),
           lastTab(-1), dragOffset(0)

#ifndef QT_NO_ANIMATION
         , animation(nullptr)
#endif
      {}

      bool operator==(const Tab &other) const {
         return &other == this;
      }

      bool enabled;
      int shortcutId;
      QString text;

#ifndef QT_NO_TOOLTIP
      QString toolTip;
#endif

#ifndef QT_NO_WHATSTHIS
      QString whatsThis;
#endif

      QIcon icon;
      QRect rect;
      QRect minRect;
      QRect maxRect;

      QColor textColor;
      QVariant data;
      QWidget *leftWidget;
      QWidget *rightWidget;
      int lastTab;
      int dragOffset;

#ifndef QT_NO_ANIMATION

      ~Tab() {
         delete animation;
      }

      struct TabBarAnimation : public QVariantAnimation {
         TabBarAnimation(QSharedPointer<Tab> t, QTabBarPrivate *_priv) : tab(t), priv(_priv) {
            setEasingCurve(QEasingCurve::InOutQuad);
         }

         void updateCurrentValue(const QVariant &current) override;
         void updateState(State, State newState) override;

       private:
         // these are needed for the callbacks
         QSharedPointer<Tab> tab;
         QTabBarPrivate *priv;

      } *animation;

      void startAnimation(QTabBarPrivate *priv, int duration) {

         if (! priv->isAnimated()) {
            priv->moveTabFinished(priv->tabList.indexOf(sharedFromThis()));
            return;
         }

         if (! animation) {
            animation = new TabBarAnimation(sharedFromThis(), priv);
         }
         animation->setStartValue(dragOffset);
         animation->setEndValue(0);
         animation->setDuration(duration);
         animation->start();
      }
#else
      void startAnimation(QTabBarPrivate *priv, int duration) {
         priv->moveTabFinished(priv->tabList.indexOf(sharedFromThis()));
      }

#endif //QT_NO_ANIMATION

   }; //struct Tab

   QList<QSharedPointer<Tab>> tabList;
   mutable QHash<QString, QSize> textSizes;

   int calculateNewPosition(int from, int to, int index) const;
   void slide(int from, int to);
   void init();
   int extraWidth() const;

   QSharedPointer<Tab> at(int index);
   QSharedPointer<const Tab> at(int index) const;

   int indexAtPos(const QPoint &p) const;

   inline bool isAnimated() const {
      Q_Q(const QTabBar);
      return q->style()->styleHint(QStyle::SH_Widget_Animate, nullptr, q);
   }

   bool validIndex(int index) const {
      return index >= 0 && index < tabList.count();
   }

   void setCurrentNextEnabledIndex(int offset);

   QToolButton *rightB;    // right or bottom
   QToolButton *leftB;     // left or top

   void _q_scrollTabs();
   void _q_closeTab();
   void moveTab(int index, int offset);
   void moveTabFinished(int index);
   QRect hoverRect;

   void refresh();
   void layoutTabs();
   void layoutWidgets(int start = 0);
   void layoutTab(int index);
   void updateMacBorderMetrics();
   bool isTabInMacUnifiedToolbarArea() const;
   void setupMovableTab();
   void autoHideTabs();
   void initBasicStyleOption(QStyleOptionTab *option, int tabIndex) const;

   void makeVisible(int index);
   QSize iconSize;
   Qt::TextElideMode elideMode;
   bool elideModeSetByUser;
   bool useScrollButtons;
   bool useScrollButtonsSetByUser;

   bool expanding;
   bool closeButtonOnTabs;
   QTabBar::SelectionBehavior selectionBehaviorOnRemove;

   QPoint dragStartPosition;
   bool paintWithOffsets;
   bool movable;
   bool dragInProgress;
   bool documentMode;
   bool autoHide;
   bool changeCurrentOnDrag;

   int switchTabCurrentIndex;
   int switchTabTimerId;

   QMovableTabWidget *movingTab;

   // shared by tabwidget and qtabbar
   static void initStyleBaseOption(QStyleOptionTabBarBase *optTabBase, QTabBar *tabbar, QSize size) {
      QStyleOptionTab tabOverlap;
      tabOverlap.shape = tabbar->shape();

      int overlap = tabbar->style()->pixelMetric(QStyle::PM_TabBarBaseOverlap, &tabOverlap, tabbar);
      QWidget *theParent = tabbar->parentWidget();

      optTabBase->initFrom(tabbar);
      optTabBase->shape = tabbar->shape();
      optTabBase->documentMode = tabbar->documentMode();

      if (theParent && overlap > 0) {
         QRect rect;

         switch (tabOverlap.shape) {
            case QTabBar::RoundedNorth:
            case QTabBar::TriangularNorth:
               rect.setRect(0, size.height() - overlap, size.width(), overlap);
               break;

            case QTabBar::RoundedSouth:
            case QTabBar::TriangularSouth:
               rect.setRect(0, 0, size.width(), overlap);
               break;

            case QTabBar::RoundedEast:
            case QTabBar::TriangularEast:
               rect.setRect(0, 0, overlap, size.height());
               break;

            case QTabBar::RoundedWest:
            case QTabBar::TriangularWest:
               rect.setRect(size.width() - overlap, 0, overlap, size.height());
               break;
         }

         optTabBase->rect = rect;
      }
   }

   void killSwitchTabTimer();
};

class CloseButton : public QAbstractButton
{
   GUI_CS_OBJECT(CloseButton)

 public:
   CloseButton(QWidget *parent = nullptr);

   QSize sizeHint() const override;

   QSize minimumSizeHint() const override {
      return sizeHint();
   }

   void enterEvent(QEvent *event) override;
   void leaveEvent(QEvent *event) override;
   void paintEvent(QPaintEvent *event) override;
};



#endif

#endif
