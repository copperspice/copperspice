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

#ifndef QABSTRACTBUTTON_H
#define QABSTRACTBUTTON_H

#include <QtGui/qicon.h>
#include <QtGui/qkeysequence.h>
#include <QtGui/qwidget.h>

QT_BEGIN_NAMESPACE

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

   void setCheckable(bool);
   bool isCheckable() const;

   bool isChecked() const;

   void setDown(bool);
   bool isDown() const;

   void setAutoRepeat(bool);
   bool autoRepeat() const;

   void setAutoRepeatDelay(int);
   int autoRepeatDelay() const;

   void setAutoRepeatInterval(int);
   int autoRepeatInterval() const;

   void setAutoExclusive(bool);
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
   GUI_CS_SLOT_1(Public, void setChecked(bool un_named_arg1))
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
   void paintEvent(QPaintEvent *e) override = 0;
   virtual bool hitButton(const QPoint &pos) const;
   virtual void checkStateSet();
   virtual void nextCheckState();

   bool event(QEvent *e) override;
   void keyPressEvent(QKeyEvent *e) override;
   void keyReleaseEvent(QKeyEvent *e) override;
   void mousePressEvent(QMouseEvent *e) override;
   void mouseReleaseEvent(QMouseEvent *e) override;
   void mouseMoveEvent(QMouseEvent *e) override;
   void focusInEvent(QFocusEvent *e) override;
   void focusOutEvent(QFocusEvent *e) override;
   void changeEvent(QEvent *e) override;
   void timerEvent(QTimerEvent *e) override;

   QAbstractButton(QAbstractButtonPrivate &dd, QWidget *parent = nullptr);

 private:
   Q_DECLARE_PRIVATE(QAbstractButton)
   Q_DISABLE_COPY(QAbstractButton)
   friend class QButtonGroup;
};

QT_END_NAMESPACE

#endif // QABSTRACTBUTTON_H
