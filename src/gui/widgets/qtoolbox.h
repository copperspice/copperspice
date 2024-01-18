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

#ifndef QTOOLBOX_H
#define QTOOLBOX_H

#include <qframe.h>
#include <qicon.h>

#ifndef QT_NO_TOOLBOX

class QToolBoxPrivate;

class Q_GUI_EXPORT QToolBox : public QFrame
{
   GUI_CS_OBJECT(QToolBox)

   GUI_CS_PROPERTY_READ(currentIndex, currentIndex)
   GUI_CS_PROPERTY_WRITE(currentIndex, setCurrentIndex)
   GUI_CS_PROPERTY_NOTIFY(currentIndex, currentChanged)

   GUI_CS_PROPERTY_READ(count, count)

 public:
   explicit QToolBox(QWidget *parent = nullptr, Qt::WindowFlags flags = Qt::EmptyFlag);

   QToolBox(const QToolBox &) = delete;
   QToolBox &operator=(const QToolBox &) = delete;

   ~QToolBox();

   int addItem(QWidget *widget, const QString &text);
   int addItem(QWidget *widget, const QIcon &icon, const QString &text);
   int insertItem(int index, QWidget *widget, const QString &text);
   int insertItem(int index, QWidget *widget, const QIcon &icon, const QString &text);

   void removeItem(int index);

   void setItemEnabled(int index, bool enabled);
   bool isItemEnabled(int index) const;

   void setItemText(int index, const QString &text);
   QString itemText(int index) const;

   void setItemIcon(int index, const QIcon &icon);
   QIcon itemIcon(int index) const;

#ifndef QT_NO_TOOLTIP
   void setItemToolTip(int index, const QString &toolTip);
   QString itemToolTip(int index) const;
#endif

   int currentIndex() const;
   QWidget *currentWidget() const;
   QWidget *widget(int index) const;
   int indexOf(QWidget *widget) const;
   int count() const;

   GUI_CS_SLOT_1(Public, void setCurrentIndex(int index))
   GUI_CS_SLOT_2(setCurrentIndex)

   GUI_CS_SLOT_1(Public, void setCurrentWidget(QWidget *widget))
   GUI_CS_SLOT_2(setCurrentWidget)

   GUI_CS_SIGNAL_1(Public, void currentChanged(int index))
   GUI_CS_SIGNAL_2(currentChanged, index)

 protected:
   bool event(QEvent *event) override;

   virtual void itemInserted(int index);
   virtual void itemRemoved(int index);

   void showEvent(QShowEvent *event) override;
   void changeEvent(QEvent *event) override;

 private:
   Q_DECLARE_PRIVATE(QToolBox)

   GUI_CS_SLOT_1(Private, void _q_buttonClicked())
   GUI_CS_SLOT_2(_q_buttonClicked)

   GUI_CS_SLOT_1(Private, void _q_widgetDestroyed(QObject *object))
   GUI_CS_SLOT_2(_q_widgetDestroyed)
};

inline int QToolBox::addItem(QWidget *widget, const QString &text)
{
   return insertItem(-1, widget, QIcon(), text);
}

inline int QToolBox::addItem(QWidget *widget, const QIcon &icon, const QString &text)
{
   return insertItem(-1, widget, icon, text);
}

inline int QToolBox::insertItem(int index, QWidget *widget, const QString &text)
{
   return insertItem(index, widget, QIcon(), text);
}

#endif // QT_NO_TOOLBOX

#endif // QTOOLBOX_H
