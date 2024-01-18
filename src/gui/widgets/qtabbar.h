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

#ifndef QTABBAR_H
#define QTABBAR_H

#include <qwidget.h>

#ifndef QT_NO_TABBAR

class QIcon;
class QTabBarPrivate;
class QStyleOptionTab;

class Q_GUI_EXPORT QTabBar: public QWidget
{
   GUI_CS_OBJECT(QTabBar)

   GUI_CS_ENUM(Shape)

   GUI_CS_PROPERTY_READ(shape, shape)
   GUI_CS_PROPERTY_WRITE(shape, setShape)
   GUI_CS_PROPERTY_READ(currentIndex, currentIndex)
   GUI_CS_PROPERTY_WRITE(currentIndex, setCurrentIndex)
   GUI_CS_PROPERTY_NOTIFY(currentIndex, currentChanged)
   GUI_CS_PROPERTY_READ(count, count)

   GUI_CS_PROPERTY_READ(drawBase, drawBase)
   GUI_CS_PROPERTY_WRITE(drawBase, setDrawBase)

   GUI_CS_PROPERTY_READ(iconSize, iconSize)
   GUI_CS_PROPERTY_WRITE(iconSize, setIconSize)

   GUI_CS_PROPERTY_READ(elideMode, elideMode)
   GUI_CS_PROPERTY_WRITE(elideMode, setElideMode)

   GUI_CS_PROPERTY_READ(usesScrollButtons, usesScrollButtons)
   GUI_CS_PROPERTY_WRITE(usesScrollButtons, setUsesScrollButtons)

   GUI_CS_PROPERTY_READ(tabsClosable, tabsClosable)
   GUI_CS_PROPERTY_WRITE(tabsClosable, setTabsClosable)

   GUI_CS_PROPERTY_READ(selectionBehaviorOnRemove, selectionBehaviorOnRemove)
   GUI_CS_PROPERTY_WRITE(selectionBehaviorOnRemove, setSelectionBehaviorOnRemove)

   GUI_CS_PROPERTY_READ(expanding, expanding)
   GUI_CS_PROPERTY_WRITE(expanding, setExpanding)

   GUI_CS_PROPERTY_READ(movable, isMovable)
   GUI_CS_PROPERTY_WRITE(movable, setMovable)

   GUI_CS_PROPERTY_READ(documentMode, documentMode)
   GUI_CS_PROPERTY_WRITE(documentMode, setDocumentMode)

   GUI_CS_PROPERTY_READ(autoHide, autoHide)
   GUI_CS_PROPERTY_WRITE(autoHide, setAutoHide)

   GUI_CS_PROPERTY_READ(changeCurrentOnDrag, changeCurrentOnDrag)
   GUI_CS_PROPERTY_WRITE(changeCurrentOnDrag, setChangeCurrentOnDrag)

 public:
   explicit QTabBar(QWidget *parent = nullptr);

   QTabBar(const QTabBar &) = delete;
   QTabBar &operator=(const QTabBar &) = delete;

   ~QTabBar();

   enum Shape { RoundedNorth, RoundedSouth, RoundedWest, RoundedEast,
      TriangularNorth, TriangularSouth, TriangularWest, TriangularEast
   };

   enum ButtonPosition {
      LeftSide,
      RightSide
   };

   enum SelectionBehavior {
      SelectLeftTab,
      SelectRightTab,
      SelectPreviousTab
   };

   Shape shape() const;
   void setShape(Shape shape);

   int addTab(const QString &text);
   int addTab(const QIcon &icon, const QString &text);

   int insertTab(int index, const QString &text);
   int insertTab(int index, const QIcon &icon, const QString &text);

   void removeTab(int index);
   void moveTab(int from, int to);

   bool isTabEnabled(int index) const;
   void setTabEnabled(int index, bool enable);

   QString tabText(int index) const;
   void setTabText(int index, const QString &text);

   QColor tabTextColor(int index) const;
   void setTabTextColor(int index, const QColor &color);

   QIcon tabIcon(int index) const;
   void setTabIcon(int index, const QIcon &icon);

   Qt::TextElideMode elideMode() const;
   void setElideMode(Qt::TextElideMode value);

#ifndef QT_NO_TOOLTIP
   void setTabToolTip(int index, const QString &tip);
   QString tabToolTip(int index) const;
#endif

#ifndef QT_NO_WHATSTHIS
   void setTabWhatsThis(int index, const QString &text);
   QString tabWhatsThis(int index) const;
#endif

   void setTabData(int index, const QVariant &data);
   QVariant tabData(int index) const;

   QRect tabRect(int index) const;
   int tabAt(const QPoint &pos) const;

   int currentIndex() const;
   int count() const;

   QSize sizeHint() const override;
   QSize minimumSizeHint() const override;

   void setDrawBase(bool drawTheBase);
   bool drawBase() const;

   QSize iconSize() const;
   void setIconSize(const QSize &size);

   bool usesScrollButtons() const;
   void setUsesScrollButtons(bool useButtons);

   bool tabsClosable() const;
   void setTabsClosable(bool closeable);

   void setTabButton(int index, ButtonPosition position, QWidget *widget);
   QWidget *tabButton(int index, ButtonPosition position) const;

   SelectionBehavior selectionBehaviorOnRemove() const;
   void setSelectionBehaviorOnRemove(SelectionBehavior behavior);

   bool expanding() const;
   void setExpanding(bool enable);

   bool isMovable() const;
   void setMovable(bool movable);

   bool documentMode() const;
   void setDocumentMode(bool set);

   bool autoHide() const;
   void setAutoHide(bool hide);
   bool changeCurrentOnDrag() const;
   void setChangeCurrentOnDrag(bool change);

   GUI_CS_SLOT_1(Public, void setCurrentIndex(int index))
   GUI_CS_SLOT_2(setCurrentIndex)

   GUI_CS_SIGNAL_1(Public, void currentChanged(int index))
   GUI_CS_SIGNAL_2(currentChanged, index)

   GUI_CS_SIGNAL_1(Public, void tabCloseRequested(int index))
   GUI_CS_SIGNAL_2(tabCloseRequested, index)

   GUI_CS_SIGNAL_1(Public, void tabMoved(int from, int to))
   GUI_CS_SIGNAL_2(tabMoved, from, to)

   GUI_CS_SIGNAL_1(Public, void tabBarClicked(int index))
   GUI_CS_SIGNAL_2(tabBarClicked, index)

   GUI_CS_SIGNAL_1(Public, void tabBarDoubleClicked(int index))
   GUI_CS_SIGNAL_2(tabBarDoubleClicked, index)

 protected:
   virtual QSize tabSizeHint(int index) const;
   virtual QSize minimumTabSizeHint(int index) const;
   virtual void tabInserted(int index);
   virtual void tabRemoved(int index);
   virtual void tabLayoutChange();

   bool event(QEvent *event) override;
   void resizeEvent(QResizeEvent *event) override;
   void showEvent(QShowEvent *event) override;
   void hideEvent(QHideEvent *event) override;
   void paintEvent(QPaintEvent *event) override;
   void mousePressEvent (QMouseEvent *event) override;
   void mouseMoveEvent (QMouseEvent *event) override;
   void mouseReleaseEvent (QMouseEvent *event) override;

#ifndef QT_NO_WHEELEVENT
   void wheelEvent(QWheelEvent *event) override;
#endif

   void keyPressEvent(QKeyEvent *event) override;
   void changeEvent(QEvent *event) override;
   void timerEvent(QTimerEvent *event) override;
   void initStyleOption(QStyleOptionTab *option, int tabIndex) const;

#ifndef QT_NO_ACCESSIBILITY
   friend class QAccessibleTabBar;
#endif

 private:
   Q_DECLARE_PRIVATE(QTabBar)

   GUI_CS_SLOT_1(Private, void _q_scrollTabs())
   GUI_CS_SLOT_2(_q_scrollTabs)

   GUI_CS_SLOT_1(Private, void _q_closeTab())
   GUI_CS_SLOT_2(_q_closeTab)
};

#endif // QT_NO_TABBAR

#endif