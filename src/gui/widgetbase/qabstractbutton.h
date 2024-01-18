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

#ifndef QABSTRACTBUTTON_H
#define QABSTRACTBUTTON_H

#include <qicon.h>
#include <qkeysequence.h>
#include <qwidget.h>

class QButtonGroup;

class QAbstractButtonPrivate;

class Q_GUI_EXPORT QAbstractButton : public QWidget
{
   GUI_CS_OBJECT(QAbstractButton)

   GUI_CS_PROPERTY_READ(text, text)
   GUI_CS_PROPERTY_WRITE(text, setText)
   GUI_CS_PROPERTY_READ(icon, icon)
   GUI_CS_PROPERTY_WRITE(icon, setIcon)
   GUI_CS_PROPERTY_READ(iconSize, iconSize)
   GUI_CS_PROPERTY_WRITE(iconSize, setIconSize)

#ifndef QT_NO_SHORTCUT
   GUI_CS_PROPERTY_READ(shortcut, shortcut)
   GUI_CS_PROPERTY_WRITE(shortcut, setShortcut)
#endif

   GUI_CS_PROPERTY_READ(checkable, isCheckable)
   GUI_CS_PROPERTY_WRITE(checkable, setCheckable)

   GUI_CS_PROPERTY_READ(checked, isChecked)
   GUI_CS_PROPERTY_WRITE(checked, setChecked)
   GUI_CS_PROPERTY_DESIGNABLE_NONSTATIC(checked, isCheckable())
   GUI_CS_PROPERTY_NOTIFY(checked, toggled)
   GUI_CS_PROPERTY_USER(checked, true)

   GUI_CS_PROPERTY_READ(autoRepeat, autoRepeat)
   GUI_CS_PROPERTY_WRITE(autoRepeat, setAutoRepeat)

   GUI_CS_PROPERTY_READ(autoExclusive, autoExclusive)
   GUI_CS_PROPERTY_WRITE(autoExclusive, setAutoExclusive)

   GUI_CS_PROPERTY_READ(autoRepeatDelay, autoRepeatDelay)
   GUI_CS_PROPERTY_WRITE(autoRepeatDelay, setAutoRepeatDelay)

   GUI_CS_PROPERTY_READ(autoRepeatInterval, autoRepeatInterval)
   GUI_CS_PROPERTY_WRITE(autoRepeatInterval, setAutoRepeatInterval)

   GUI_CS_PROPERTY_READ(down, isDown)
   GUI_CS_PROPERTY_WRITE(down, setDown)
   GUI_CS_PROPERTY_DESIGNABLE(down, false)

 public:
   explicit QAbstractButton(QWidget *parent = nullptr);

   QAbstractButton(const QAbstractButton &) = delete;
   QAbstractButton &operator=(const QAbstractButton &) = delete;

   ~QAbstractButton();

   void setText(const QString &text);
   QString text() const;

   void setIcon(const QIcon &icon);
   QIcon icon() const;

   QSize iconSize() const;

#ifndef QT_NO_SHORTCUT
   void setShortcut(const QKeySequence &key);
   QKeySequence shortcut() const;
#endif

   void setCheckable(bool enable);
   bool isCheckable() const;

   bool isChecked() const;

   void setDown(bool enable);
   bool isDown() const;

   void setAutoRepeat(bool enable);
   bool autoRepeat() const;

   void setAutoRepeatDelay(int delay);
   int autoRepeatDelay() const;

   void setAutoRepeatInterval(int interval);
   int autoRepeatInterval() const;

   void setAutoExclusive(bool enable);
   bool autoExclusive() const;

#ifndef QT_NO_BUTTONGROUP
   QButtonGroup *group() const;
#endif

   GUI_CS_SLOT_1(Public, void setIconSize(const QSize &size))
   GUI_CS_SLOT_2(setIconSize)

   GUI_CS_SLOT_1(Public, void animateClick(int msec = 100))
   GUI_CS_SLOT_2(animateClick)

   GUI_CS_SLOT_1(Public, void click())
   GUI_CS_SLOT_2(click)

   GUI_CS_SLOT_1(Public, void toggle())
   GUI_CS_SLOT_2(toggle)

   GUI_CS_SLOT_1(Public, void setChecked(bool checked))
   GUI_CS_SLOT_2(setChecked)

   GUI_CS_SIGNAL_1(Public, void pressed())
   GUI_CS_SIGNAL_2(pressed)

   GUI_CS_SIGNAL_1(Public, void released())
   GUI_CS_SIGNAL_2(released)

   GUI_CS_SIGNAL_1(Public, void clicked(bool checked = false))
   GUI_CS_SIGNAL_2(clicked, checked)

   GUI_CS_SIGNAL_1(Public, void toggled(bool checked))
   GUI_CS_SIGNAL_2(toggled, checked)

 protected:
   void paintEvent(QPaintEvent *event) override = 0;
   virtual bool hitButton(const QPoint &pos) const;
   virtual void checkStateSet();
   virtual void nextCheckState();

   bool event(QEvent *event) override;
   void keyPressEvent(QKeyEvent *event) override;
   void keyReleaseEvent(QKeyEvent *event) override;
   void mousePressEvent(QMouseEvent *event) override;
   void mouseReleaseEvent(QMouseEvent *event) override;
   void mouseMoveEvent(QMouseEvent *event) override;
   void focusInEvent(QFocusEvent *event) override;
   void focusOutEvent(QFocusEvent *event) override;
   void changeEvent(QEvent *event) override;
   void timerEvent(QTimerEvent *event) override;

   QAbstractButton(QAbstractButtonPrivate &dd, QWidget *parent = nullptr);

 private:
   Q_DECLARE_PRIVATE(QAbstractButton)
   friend class QButtonGroup;
};

#endif
