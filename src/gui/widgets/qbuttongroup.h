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

#ifndef QBUTTONGROUP_H
#define QBUTTONGROUP_H

#include <qobject.h>
#include <qscopedpointer.h>

#ifndef QT_NO_BUTTONGROUP

class QAbstractButton;
class QAbstractButtonPrivate;
class QButtonGroupPrivate;

class Q_GUI_EXPORT QButtonGroup : public QObject
{
   GUI_CS_OBJECT(QButtonGroup)

   GUI_CS_PROPERTY_READ(exclusive, exclusive)
   GUI_CS_PROPERTY_WRITE(exclusive, setExclusive)

 public:
   explicit QButtonGroup(QObject *parent = nullptr);

   QButtonGroup(const QButtonGroup &) = delete;
   QButtonGroup &operator=(const QButtonGroup &) = delete;

   ~QButtonGroup();

   void setExclusive(bool enable);
   bool exclusive() const;

   void addButton(QAbstractButton *button, int id = -1);
   void removeButton(QAbstractButton *button);

   QList<QAbstractButton *> buttons() const;

   QAbstractButton *checkedButton() const;
   // no setter on purpose

   QAbstractButton *button(int id) const;
   void setId(QAbstractButton *button, int id);
   int id(QAbstractButton *button) const;
   int checkedId() const;

   GUI_CS_SIGNAL_1(Public, void buttonClicked(QAbstractButton *button))
   GUI_CS_SIGNAL_OVERLOAD(buttonClicked, (QAbstractButton *), button)

   GUI_CS_SIGNAL_1(Public, void buttonClicked(int id))
   GUI_CS_SIGNAL_OVERLOAD(buttonClicked, (int), id)

   GUI_CS_SIGNAL_1(Public, void buttonPressed(QAbstractButton *button))
   GUI_CS_SIGNAL_OVERLOAD(buttonPressed, (QAbstractButton *), button)

   GUI_CS_SIGNAL_1(Public, void buttonPressed(int id))
   GUI_CS_SIGNAL_OVERLOAD(buttonPressed, (int), id)

   GUI_CS_SIGNAL_1(Public, void buttonReleased(QAbstractButton *button))
   GUI_CS_SIGNAL_OVERLOAD(buttonReleased, (QAbstractButton *), button)

   GUI_CS_SIGNAL_1(Public, void buttonReleased(int id))
   GUI_CS_SIGNAL_OVERLOAD(buttonReleased, (int), id)

   GUI_CS_SIGNAL_1(Public, void buttonToggled(QAbstractButton *button, bool enable))
   GUI_CS_SIGNAL_OVERLOAD(buttonToggled, (QAbstractButton *, bool), button, enable)

   GUI_CS_SIGNAL_1(Public, void buttonToggled(int id, bool enable))
   GUI_CS_SIGNAL_OVERLOAD(buttonToggled, (int, bool), id, enable)

 protected:
   QScopedPointer<QButtonGroupPrivate> d_ptr;

 private:
   Q_DECLARE_PRIVATE(QButtonGroup)

   friend class QAbstractButton;
   friend class QAbstractButtonPrivate;
};

#endif // QT_NO_BUTTONGROUP

#endif
