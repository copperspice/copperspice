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

#ifndef QABSTRACTLAYOUTSTYLEINFO_P_H
#define QABSTRACTLAYOUTSTYLEINFO_P_H

#include <qnamespace.h>

#include <qlayoutpolicy_p.h>

class Q_GUI_EXPORT QAbstractLayoutStyleInfo
{
 public:
   QAbstractLayoutStyleInfo()
      : m_isWindow(false)
   { }

   virtual ~QAbstractLayoutStyleInfo()
   { }

   virtual qreal combinedLayoutSpacing(QLayoutPolicy::ControlTypes, QLayoutPolicy::ControlTypes,
         Qt::Orientation) const {
      return -1;
   }

   virtual qreal perItemSpacing(QLayoutPolicy::ControlType, QLayoutPolicy::ControlType,
         Qt::Orientation) const {
      return -1;
   }

   virtual qreal spacing(Qt::Orientation orientation) const = 0;

   virtual bool hasChangedCore() const {
      return false;   // ### Remove when usage is gone from subclasses
   }

   virtual void invalidate() { }

   virtual qreal windowMargin(Qt::Orientation orientation) const = 0;

   bool isWindow() const {
      return m_isWindow;
   }

 protected:
   unsigned m_isWindow : 1;
   mutable unsigned m_hSpacingState: 2;
   mutable unsigned m_vSpacingState: 2;
   mutable qreal m_spacing[2];
};

#endif
