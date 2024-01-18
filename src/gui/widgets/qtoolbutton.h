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

#ifndef QTOOLBUTTON_H
#define QTOOLBUTTON_H

#include <qabstractbutton.h>

#ifndef QT_NO_TOOLBUTTON

class QToolButtonPrivate;
class QMenu;
class QStyleOptionToolButton;

class Q_GUI_EXPORT QToolButton : public QAbstractButton
{
   GUI_CS_OBJECT(QToolButton)

   GUI_CS_ENUM(ToolButtonPopupMode)

#ifndef QT_NO_MENU
   GUI_CS_PROPERTY_READ(popupMode, popupMode)
   GUI_CS_PROPERTY_WRITE(popupMode, setPopupMode)
#endif

   GUI_CS_PROPERTY_READ(toolButtonStyle, toolButtonStyle)
   GUI_CS_PROPERTY_WRITE(toolButtonStyle, setToolButtonStyle)

   GUI_CS_PROPERTY_READ(autoRaise, autoRaise)
   GUI_CS_PROPERTY_WRITE(autoRaise, setAutoRaise)

   GUI_CS_PROPERTY_READ(arrowType, arrowType)
   GUI_CS_PROPERTY_WRITE(arrowType, setArrowType)

 public:
   GUI_CS_REGISTER_ENUM(
      enum ToolButtonPopupMode {
         DelayedPopup,
         MenuButtonPopup,
         InstantPopup
      };
   )

   explicit QToolButton(QWidget *parent = nullptr);

   QToolButton(const QToolButton &) = delete;
   QToolButton &operator=(const QToolButton &) = delete;

   ~QToolButton();

   QSize sizeHint() const override;
   QSize minimumSizeHint() const override;

   Qt::ToolButtonStyle toolButtonStyle() const;

   Qt::ArrowType arrowType() const;
   void setArrowType(Qt::ArrowType type);

#ifndef QT_NO_MENU
   void setMenu(QMenu *menu);
   QMenu *menu() const;

   void setPopupMode(ToolButtonPopupMode mode);
   ToolButtonPopupMode popupMode() const;
#endif

   QAction *defaultAction() const;

   void setAutoRaise(bool enable);
   bool autoRaise() const;

#ifndef QT_NO_MENU
   GUI_CS_SLOT_1(Public, void showMenu())
   GUI_CS_SLOT_2(showMenu)
#endif

   GUI_CS_SLOT_1(Public, void setToolButtonStyle(Qt::ToolButtonStyle style))
   GUI_CS_SLOT_2(setToolButtonStyle)

   GUI_CS_SLOT_1(Public, void setDefaultAction(QAction *action))
   GUI_CS_SLOT_2(setDefaultAction)

   GUI_CS_SIGNAL_1(Public, void triggered(QAction *action))
   GUI_CS_SIGNAL_2(triggered, action)

 protected:
   QToolButton(QToolButtonPrivate &, QWidget *parent);

   bool event(QEvent *event) override;
   void mousePressEvent(QMouseEvent *event) override;
   void mouseReleaseEvent(QMouseEvent *event) override;
   void paintEvent(QPaintEvent *event) override;
   void actionEvent(QActionEvent *event) override;

   void enterEvent(QEvent *event) override;
   void leaveEvent(QEvent *event) override;
   void timerEvent(QTimerEvent *event) override;
   void changeEvent(QEvent *event) override;

   bool hitButton(const QPoint &pos) const override;
   void nextCheckState() override;
   void initStyleOption(QStyleOptionToolButton *option) const;

 private:
   Q_DECLARE_PRIVATE(QToolButton)

#ifndef QT_NO_MENU
   GUI_CS_SLOT_1(Private, void _q_buttonPressed())
   GUI_CS_SLOT_2(_q_buttonPressed)

   GUI_CS_SLOT_1(Private, void _q_buttonReleased())
   GUI_CS_SLOT_2(_q_buttonReleased)

   GUI_CS_SLOT_1(Private, void _q_updateButtonDown())
   GUI_CS_SLOT_2(_q_updateButtonDown)

   GUI_CS_SLOT_1(Private, void _q_menuTriggered(QAction *action))
   GUI_CS_SLOT_2(_q_menuTriggered)
#endif

   GUI_CS_SLOT_1(Private, void _q_actionTriggered())
   GUI_CS_SLOT_2(_q_actionTriggered)
};

#endif // QT_NO_TOOLBUTTON

#endif // QTOOLBUTTON_H
