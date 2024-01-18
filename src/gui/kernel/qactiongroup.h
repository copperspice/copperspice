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

#ifndef QACTIONGROUP_H
#define QACTIONGROUP_H

#include <qaction.h>
#include <qscopedpointer.h>

#ifndef QT_NO_ACTION

class QActionGroupPrivate;

class Q_GUI_EXPORT QActionGroup : public QObject
{
   GUI_CS_OBJECT(QActionGroup)

   GUI_CS_PROPERTY_READ(exclusive, isExclusive)
   GUI_CS_PROPERTY_WRITE(exclusive, setExclusive)

   GUI_CS_PROPERTY_READ(enabled, isEnabled)
   GUI_CS_PROPERTY_WRITE(enabled, setEnabled)

   GUI_CS_PROPERTY_READ(visible, isVisible)
   GUI_CS_PROPERTY_WRITE(visible, setVisible)

 public:
   explicit QActionGroup(QObject *parent);

   QActionGroup(const QActionGroup &) = delete;
   QActionGroup &operator=(const QActionGroup &) = delete;

   ~QActionGroup();

   QAction *addAction(QAction *action);
   QAction *addAction(const QString &text);
   QAction *addAction(const QIcon &icon, const QString &text);
   void removeAction(QAction *action);
   QList<QAction *> actions() const;

   QAction *checkedAction() const;
   bool isExclusive() const;
   bool isEnabled() const;
   bool isVisible() const;

   GUI_CS_SLOT_1(Public, void setEnabled(bool b))
   GUI_CS_SLOT_2(setEnabled)

   GUI_CS_SLOT_1(Public, void setDisabled(bool b) { setEnabled(! b); })
   GUI_CS_SLOT_2(setDisabled)

   GUI_CS_SLOT_1(Public, void setVisible(bool b))
   GUI_CS_SLOT_2(setVisible)

   GUI_CS_SLOT_1(Public, void setExclusive(bool b))
   GUI_CS_SLOT_2(setExclusive)

   GUI_CS_SIGNAL_1(Public, void triggered(QAction *action))
   GUI_CS_SIGNAL_2(triggered, action)

   GUI_CS_SIGNAL_1(Public, void hovered(QAction *action))
   GUI_CS_SIGNAL_2(hovered, action)

 protected:
   QScopedPointer<QActionGroupPrivate> d_ptr;

 private:
   Q_DECLARE_PRIVATE(QActionGroup)

   GUI_CS_SLOT_1(Private, void _q_actionTriggered())
   GUI_CS_SLOT_2(_q_actionTriggered)

   GUI_CS_SLOT_1(Private, void _q_actionChanged())
   GUI_CS_SLOT_2(_q_actionChanged)

   GUI_CS_SLOT_1(Private, void _q_actionHovered())
   GUI_CS_SLOT_2(_q_actionHovered)
};

#endif // QT_NO_ACTION

#endif
