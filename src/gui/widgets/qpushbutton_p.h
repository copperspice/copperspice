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

#ifndef QPushButton_P_H
#define QPushButton_P_H

#include <qabstractbutton_p.h>



class QDialog;
class QPushButton;

class QPushButtonPrivate : public QAbstractButtonPrivate
{
   Q_DECLARE_PUBLIC(QPushButton)

 public:
   enum AutoDefaultValue { Off = 0, On = 1, Auto = 2 };

   QPushButtonPrivate()
      : QAbstractButtonPrivate(QSizePolicy::PushButton), autoDefault(Auto),
        defaultButton(false), flat(false), menuOpen(false), lastAutoDefault(false)
   { }

   void init() {
      resetLayoutItemMargins();
   }

   static QPushButtonPrivate *get(QPushButton *b) {
      return b->d_func();
   }

#ifndef QT_NO_MENU
   QPoint adjustedMenuPosition();
#endif

   void resetLayoutItemMargins();
   void _q_popupPressed();
   QDialog *dialogParent() const;

   QPointer<QMenu> menu;
   uint autoDefault : 2;
   uint defaultButton : 1;
   uint flat : 1;
   uint menuOpen : 1;
   mutable uint lastAutoDefault : 1;
};


#endif