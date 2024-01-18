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

#ifndef QTABWIDGET_H
#define QTABWIDGET_H

#include <qwidget.h>
#include <qicon.h>

#ifndef QT_NO_TABWIDGET

class QTabBar;
class QTabWidgetPrivate;
class QStyleOptionTabWidgetFrame;

class Q_GUI_EXPORT QTabWidget : public QWidget
{
   GUI_CS_OBJECT(QTabWidget)

   GUI_CS_ENUM(TabPosition)
   GUI_CS_ENUM(TabShape)

   GUI_CS_PROPERTY_READ(tabPosition, tabPosition)
   GUI_CS_PROPERTY_WRITE(tabPosition, setTabPosition)
   GUI_CS_PROPERTY_READ(tabShape, tabShape)
   GUI_CS_PROPERTY_WRITE(tabShape, setTabShape)
   GUI_CS_PROPERTY_READ(currentIndex, currentIndex)
   GUI_CS_PROPERTY_WRITE(currentIndex, setCurrentIndex)
   GUI_CS_PROPERTY_NOTIFY(currentIndex, currentChanged)
   GUI_CS_PROPERTY_READ(count, count)
   GUI_CS_PROPERTY_READ(iconSize, iconSize)
   GUI_CS_PROPERTY_WRITE(iconSize, setIconSize)
   GUI_CS_PROPERTY_READ(elideMode, elideMode)
   GUI_CS_PROPERTY_WRITE(elideMode, setElideMode)

   GUI_CS_PROPERTY_READ(usesScrollButtons, usesScrollButtons)
   GUI_CS_PROPERTY_WRITE(usesScrollButtons, setUsesScrollButtons)

   GUI_CS_PROPERTY_READ(documentMode, documentMode)
   GUI_CS_PROPERTY_WRITE(documentMode, setDocumentMode)

   GUI_CS_PROPERTY_READ(tabsClosable, tabsClosable)
   GUI_CS_PROPERTY_WRITE(tabsClosable, setTabsClosable)

   GUI_CS_PROPERTY_READ(movable, isMovable)
   GUI_CS_PROPERTY_WRITE(movable, setMovable)

   GUI_CS_PROPERTY_READ(tabBarAutoHide, tabBarAutoHide)
   GUI_CS_PROPERTY_WRITE(tabBarAutoHide, setTabBarAutoHide)

 public:
   GUI_CS_REGISTER_ENUM(
      enum TabPosition {
         North,
         South,
         West,
         East
      };
   )

   GUI_CS_REGISTER_ENUM(
      enum TabShape {
         Rounded,
         Triangular
      };
   )

   explicit QTabWidget(QWidget *parent = nullptr);

   QTabWidget(const QTabWidget &) = delete;
   QTabWidget &operator=(const QTabWidget &) = delete;

   ~QTabWidget();

   int addTab(QWidget *widget, const QString &label);
   int addTab(QWidget *widget, const QIcon &icon, const QString &label);

   int insertTab(int index, QWidget *widget, const QString &label);
   int insertTab(int index, QWidget *widget, const QIcon &icon, const QString &label);

   void removeTab(int index);

   bool isTabEnabled(int index) const;
   void setTabEnabled(int index, bool enable);

   QString tabText(int index) const;
   void setTabText(int index, const QString &label);

   QIcon tabIcon(int index) const;
   void setTabIcon(int index, const QIcon &icon);

#ifndef QT_NO_TOOLTIP
   void setTabToolTip(int index, const QString &tip);
   QString tabToolTip(int index) const;
#endif

#ifndef QT_NO_WHATSTHIS
   void setTabWhatsThis(int index, const QString &text);
   QString tabWhatsThis(int index) const;
#endif

   int currentIndex() const;
   QWidget *currentWidget() const;
   QWidget *widget(int index) const;
   int indexOf(QWidget *widget) const;
   int count() const;

   TabPosition tabPosition() const;
   void setTabPosition(TabPosition value);

   bool tabsClosable() const;
   void setTabsClosable(bool closeable);

   bool isMovable() const;
   void setMovable(bool movable);

   TabShape tabShape() const;
   void setTabShape(TabShape s);

   QSize sizeHint() const override;
   QSize minimumSizeHint() const override;
   int heightForWidth(int width) const override;
   bool hasHeightForWidth() const override;

   void setCornerWidget(QWidget *widget, Qt::Corner corner = Qt::TopRightCorner);
   QWidget *cornerWidget(Qt::Corner corner = Qt::TopRightCorner) const;

   Qt::TextElideMode elideMode() const;
   void setElideMode(Qt::TextElideMode value);

   QSize iconSize() const;
   void setIconSize(const QSize &size);

   bool usesScrollButtons() const;
   void setUsesScrollButtons(bool useButtons);

   bool documentMode() const;
   void setDocumentMode(bool set);

   bool tabBarAutoHide() const;
   void setTabBarAutoHide(bool enable);
   void clear();

   QTabBar *tabBar() const;
   GUI_CS_SLOT_1(Public, void setCurrentIndex(int index))
   GUI_CS_SLOT_2(setCurrentIndex)

   GUI_CS_SLOT_1(Public, void setCurrentWidget(QWidget *widget))
   GUI_CS_SLOT_2(setCurrentWidget)

   GUI_CS_SIGNAL_1(Public, void currentChanged(int index))
   GUI_CS_SIGNAL_2(currentChanged, index)

   GUI_CS_SIGNAL_1(Public, void tabCloseRequested(int index))
   GUI_CS_SIGNAL_2(tabCloseRequested, index)

   GUI_CS_SIGNAL_1(Public, void tabBarClicked(int index))
   GUI_CS_SIGNAL_2(tabBarClicked, index)

   GUI_CS_SIGNAL_1(Public, void tabBarDoubleClicked(int index))
   GUI_CS_SIGNAL_2(tabBarDoubleClicked, index)

 protected:
   virtual void tabInserted(int index);
   virtual void tabRemoved(int index);

   void showEvent(QShowEvent *event) override;
   void resizeEvent(QResizeEvent *event) override;
   void keyPressEvent(QKeyEvent *event) override;
   void paintEvent(QPaintEvent *event) override;

   void setTabBar(QTabBar *tabBar);

   void changeEvent(QEvent *event) override;
   bool event(QEvent *event) override;
   void initStyleOption(QStyleOptionTabWidgetFrame *option) const;

 private:
   Q_DECLARE_PRIVATE(QTabWidget)

   GUI_CS_SLOT_1(Private, void _q_showTab(int index))
   GUI_CS_SLOT_2(_q_showTab)

   GUI_CS_SLOT_1(Private, void _q_removeTab(int index))
   GUI_CS_SLOT_2(_q_removeTab)

   GUI_CS_SLOT_1(Private, void _q_tabMoved(int from, int to))
   GUI_CS_SLOT_2(_q_tabMoved)

   void setUpLayout(bool = false);
};

#endif // QT_NO_TABWIDGET

#endif // QTABWIDGET_H
