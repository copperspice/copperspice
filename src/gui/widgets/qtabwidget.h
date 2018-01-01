/***********************************************************************
*
* Copyright (c) 2012-2018 Barbara Geller
* Copyright (c) 2012-2018 Ansel Sermersheim
* Copyright (c) 2012-2016 Digia Plc and/or its subsidiary(-ies).
* Copyright (c) 2008-2012 Nokia Corporation and/or its subsidiary(-ies).
* All rights reserved.
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
* <http://www.gnu.org/licenses/>.
*
***********************************************************************/

#ifndef QTABWIDGET_H
#define QTABWIDGET_H

#include <QtGui/qwidget.h>
#include <QtGui/qicon.h>

QT_BEGIN_NAMESPACE

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

 public:
   explicit QTabWidget(QWidget *parent = nullptr);
   ~QTabWidget();

   int addTab(QWidget *widget, const QString &);
   int addTab(QWidget *widget, const QIcon &icon, const QString &label);

   int insertTab(int index, QWidget *widget, const QString &);
   int insertTab(int index, QWidget *widget, const QIcon &icon, const QString &label);

   void removeTab(int index);

   bool isTabEnabled(int index) const;
   void setTabEnabled(int index, bool);

   QString tabText(int index) const;
   void setTabText(int index, const QString &);

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

   enum TabPosition { North, South, West, East };

   TabPosition tabPosition() const;
   void setTabPosition(TabPosition);

   bool tabsClosable() const;
   void setTabsClosable(bool closeable);

   bool isMovable() const;
   void setMovable(bool movable);

   enum TabShape { Rounded, Triangular };
   TabShape tabShape() const;
   void setTabShape(TabShape s);

   QSize sizeHint() const override;
   QSize minimumSizeHint() const override;
   int heightForWidth(int width) const override;

   void setCornerWidget(QWidget *w, Qt::Corner corner = Qt::TopRightCorner);
   QWidget *cornerWidget(Qt::Corner corner = Qt::TopRightCorner) const;

   Qt::TextElideMode elideMode() const;
   void setElideMode(Qt::TextElideMode);

   QSize iconSize() const;
   void setIconSize(const QSize &size);

   bool usesScrollButtons() const;
   void setUsesScrollButtons(bool useButtons);

   bool documentMode() const;
   void setDocumentMode(bool set);

   void clear();

   GUI_CS_SLOT_1(Public, void setCurrentIndex(int index))
   GUI_CS_SLOT_2(setCurrentIndex)
   GUI_CS_SLOT_1(Public, void setCurrentWidget(QWidget *widget))
   GUI_CS_SLOT_2(setCurrentWidget)

   GUI_CS_SIGNAL_1(Public, void currentChanged(int index))
   GUI_CS_SIGNAL_2(currentChanged, index)
   GUI_CS_SIGNAL_1(Public, void tabCloseRequested(int index))
   GUI_CS_SIGNAL_2(tabCloseRequested, index)

 protected:
   virtual void tabInserted(int index);
   virtual void tabRemoved(int index);

   void showEvent(QShowEvent *) override;
   void resizeEvent(QResizeEvent *) override;
   void keyPressEvent(QKeyEvent *) override;
   void paintEvent(QPaintEvent *) override;
   void setTabBar(QTabBar *);
   QTabBar *tabBar() const;
   void changeEvent(QEvent *) override;
   bool event(QEvent *) override;
   void initStyleOption(QStyleOptionTabWidgetFrame *option) const;

 private:
   Q_DECLARE_PRIVATE(QTabWidget)
   Q_DISABLE_COPY(QTabWidget)

   GUI_CS_SLOT_1(Private, void _q_showTab(int un_named_arg1))
   GUI_CS_SLOT_2(_q_showTab)

   GUI_CS_SLOT_1(Private, void _q_removeTab(int un_named_arg1))
   GUI_CS_SLOT_2(_q_removeTab)

   GUI_CS_SLOT_1(Private, void _q_tabMoved(int un_named_arg1, int un_named_arg2))
   GUI_CS_SLOT_2(_q_tabMoved)

   void setUpLayout(bool = false);
};

#endif // QT_NO_TABWIDGET

QT_END_NAMESPACE

#endif // QTABWIDGET_H
