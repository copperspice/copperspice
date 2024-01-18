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

#ifndef QXCB_WM_SUPPORT_H
#define QXCB_WM_SUPPORT_H

#include <qxcb_object.h>
#include <qxcb_connection.h>
#include <qvector.h>

class QXcbWMSupport : public QXcbObject
{
 public:
   QXcbWMSupport(QXcbConnection *c);

   bool isSupportedByWM(xcb_atom_t atom) const;
   const QVector<xcb_window_t> &virtualRoots() const {
      return net_virtual_roots;
   }

 private:
   friend class QXcbConnection;
   void updateNetWMAtoms();
   void updateVirtualRoots();

   QVector<xcb_atom_t> net_wm_atoms;
   QVector<xcb_window_t> net_virtual_roots;
};

#endif
