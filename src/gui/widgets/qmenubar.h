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

#ifndef QMENUBAR_H
#define QMENUBAR_H

#include <qmenu.h>

#ifndef QT_NO_MENUBAR

class QMenuBarPrivate;
class QStyleOptionMenuItem;
class QWindowsStyle;
class QPlatformMenuBar;

class Q_GUI_EXPORT QMenuBar : public QWidget
{
   GUI_CS_OBJECT(QMenuBar)

   GUI_CS_PROPERTY_READ(defaultUp, isDefaultUp)
   GUI_CS_PROPERTY_WRITE(defaultUp, setDefaultUp)

   GUI_CS_PROPERTY_READ(nativeMenuBar, isNativeMenuBar)
   GUI_CS_PROPERTY_WRITE(nativeMenuBar, setNativeMenuBar)

 public:
   explicit QMenuBar(QWidget *parent = nullptr);

   QMenuBar(const QMenuBar &) = delete;
   QMenuBar &operator=(const QMenuBar &) = delete;

   ~QMenuBar();

   using QWidget::addAction;

   QAction *addAction(const QString &text);
   QAction *addAction(const QString &text, const QObject *receiver, const QString &member);

   QAction *addMenu(QMenu *menu);
   QMenu *addMenu(const QString &title);
   QMenu *addMenu(const QIcon &icon, const QString &title);

   QAction *addSeparator();
   QAction *insertSeparator(QAction *before);

   QAction *insertMenu(QAction *before, QMenu *menu);

   void clear();

   QAction *activeAction() const;
   void setActiveAction(QAction *action);

   void setDefaultUp(bool enabled);
   bool isDefaultUp() const;

   QSize sizeHint() const override;
   QSize minimumSizeHint() const override;
   int heightForWidth(int width) const override;

   QRect actionGeometry(QAction *action) const;
   QAction *actionAt(const QPoint &point) const;

   void setCornerWidget(QWidget *widget, Qt::Corner corner = Qt::TopRightCorner);
   QWidget *cornerWidget(Qt::Corner corner = Qt::TopRightCorner) const;

#ifdef Q_OS_DARWIN
   NSMenu *toNSMenu();
#endif

   bool isNativeMenuBar() const;
   void setNativeMenuBar(bool nativeMenuBar);
   QPlatformMenuBar *platformMenuBar();
   GUI_CS_SLOT_1(Public, void setVisible(bool visible) override)
   GUI_CS_SLOT_2(setVisible)

   GUI_CS_SIGNAL_1(Public, void triggered(QAction *action))
   GUI_CS_SIGNAL_2(triggered, action)

   GUI_CS_SIGNAL_1(Public, void hovered(QAction *action))
   GUI_CS_SIGNAL_2(hovered, action)

 protected:
   void changeEvent(QEvent *event) override;
   void keyPressEvent(QKeyEvent *event) override;
   void mouseReleaseEvent(QMouseEvent *event) override;
   void mousePressEvent(QMouseEvent *event) override;
   void mouseMoveEvent(QMouseEvent *event) override;
   void leaveEvent(QEvent *event) override;
   void paintEvent(QPaintEvent *event) override;
   void resizeEvent(QResizeEvent *event) override;
   void actionEvent(QActionEvent *event) override;
   void focusOutEvent(QFocusEvent *event) override;
   void focusInEvent(QFocusEvent *event) override;
   void timerEvent(QTimerEvent *event) override;
   bool eventFilter(QObject *object, QEvent *event) override;
   bool event(QEvent *event) override;
   void initStyleOption(QStyleOptionMenuItem *option, const QAction *action) const;

 private:
   Q_DECLARE_PRIVATE(QMenuBar)

   GUI_CS_SLOT_1(Private, void _q_actionTriggered())
   GUI_CS_SLOT_2(_q_actionTriggered)

   GUI_CS_SLOT_1(Private, void _q_actionHovered())
   GUI_CS_SLOT_2(_q_actionHovered)

   GUI_CS_SLOT_1(Private, void _q_internalShortcutActivated(int id))
   GUI_CS_SLOT_2(_q_internalShortcutActivated)

   GUI_CS_SLOT_1(Private, void _q_updateLayout())
   GUI_CS_SLOT_2(_q_updateLayout)

   friend class QMenu;
   friend class QMenuPrivate;
   friend class QWindowsStyle;
};

#endif // QT_NO_MENUBAR

#endif
