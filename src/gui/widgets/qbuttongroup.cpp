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

#include "qbuttongroup_p.h"

#ifndef QT_NO_BUTTONGROUP

#include "qabstractbutton_p.h"

// detect a checked button other than the current one
void QButtonGroupPrivate::detectCheckedButton()
{
   QAbstractButton *previous = checkedButton;
   checkedButton = nullptr;
   if (exclusive) {
      return;
   }
   for (int i = 0; i < buttonList.count(); i++) {
      if (buttonList.at(i) != previous && buttonList.at(i)->isChecked()) {
         checkedButton = buttonList.at(i);
         return;
      }
   }
}

QButtonGroup::QButtonGroup(QObject *parent)
   : QObject(parent), d_ptr(new QButtonGroupPrivate)
{
   d_ptr->q_ptr = this;
}

QButtonGroup::~QButtonGroup()
{
   Q_D(QButtonGroup);

   for (int i = 0; i < d->buttonList.count(); ++i) {
      d->buttonList.at(i)->d_func()->group = nullptr;
   }
}

bool QButtonGroup::exclusive() const
{
   Q_D(const QButtonGroup);
   return d->exclusive;
}

void QButtonGroup::setExclusive(bool exclusive)
{
   Q_D(QButtonGroup);
   d->exclusive = exclusive;
}

void QButtonGroup::addButton(QAbstractButton *button, int id)
{
   Q_D(QButtonGroup);
   if (QButtonGroup *previous = button->d_func()->group) {
      previous->removeButton(button);
   }
   button->d_func()->group = this;
   d->buttonList.append(button);
   if (id == -1) {
      const QHash<QAbstractButton *, int>::const_iterator it
         = std::min_element(d->mapping.cbegin(), d->mapping.cend());
      if (it == d->mapping.cend()) {
         d->mapping[button] = -2;
      } else {
         d->mapping[button] = *it - 1;
      }
   } else {
      d->mapping[button] = id;
   }
   if (d->exclusive && button->isChecked()) {
      button->d_func()->notifyChecked();
   }
}

void QButtonGroup::removeButton(QAbstractButton *button)
{
   Q_D(QButtonGroup);
   if (d->checkedButton == button) {
      d->detectCheckedButton();
   }
   if (button->d_func()->group == this) {
      button->d_func()->group = nullptr;
      d->buttonList.removeAll(button);
      d->mapping.remove(button);
   }
}

QList<QAbstractButton *> QButtonGroup::buttons() const
{
   Q_D(const QButtonGroup);
   return d->buttonList;
}

QAbstractButton *QButtonGroup::checkedButton() const
{
   Q_D(const QButtonGroup);
   return d->checkedButton;
}

QAbstractButton *QButtonGroup::button(int id) const
{
   Q_D(const QButtonGroup);
   return d->mapping.key(id);
}

void QButtonGroup::setId(QAbstractButton *button, int id)
{
   Q_D(QButtonGroup);

   if (button && id != -1) {
      d->mapping[button] = id;
   }
}

int QButtonGroup::id(QAbstractButton *button) const
{
   Q_D(const QButtonGroup);
   return d->mapping.value(button, -1);
}

int QButtonGroup::checkedId() const
{
   Q_D(const QButtonGroup);
   return d->mapping.value(d->checkedButton, -1);
}

#endif // QT_NO_BUTTONGROUP
