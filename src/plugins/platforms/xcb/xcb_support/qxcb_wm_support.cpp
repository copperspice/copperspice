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

#include <qxcb_wm_support.h>
#include <qxcb_screen.h>
#include <qdebug.h>

QXcbWMSupport::QXcbWMSupport(QXcbConnection *c)
   : QXcbObject(c)
{
   updateNetWMAtoms();
   updateVirtualRoots();
}

bool QXcbWMSupport::isSupportedByWM(xcb_atom_t atom) const
{
   return net_wm_atoms.contains(atom);
}

void QXcbWMSupport::updateNetWMAtoms()
{
   net_wm_atoms.clear();

   xcb_window_t root = connection()->primaryScreen()->root();
   int offset = 0;
   int remaining = 0;
   do {
      xcb_get_property_cookie_t cookie = xcb_get_property(xcb_connection(), false, root, atom(QXcbAtom::_NET_SUPPORTED), XCB_ATOM_ATOM,
            offset, 1024);
      xcb_get_property_reply_t *reply = xcb_get_property_reply(xcb_connection(), cookie, nullptr);
      if (!reply) {
         break;
      }

      remaining = 0;

      if (reply->type == XCB_ATOM_ATOM && reply->format == 32) {
         int len = xcb_get_property_value_length(reply) / sizeof(xcb_atom_t);
         xcb_atom_t *atoms = (xcb_atom_t *)xcb_get_property_value(reply);
         int s = net_wm_atoms.size();
         net_wm_atoms.resize(s + len);
         memcpy(net_wm_atoms.data() + s, atoms, len * sizeof(xcb_atom_t));

         remaining = reply->bytes_after;
         offset += len;
      }

      free(reply);
   } while (remaining > 0);
}

// update the virtual roots array
void QXcbWMSupport::updateVirtualRoots()
{
   net_virtual_roots.clear();

   if (!isSupportedByWM(atom(QXcbAtom::_NET_VIRTUAL_ROOTS))) {
      return;
   }

   xcb_window_t root = connection()->primaryScreen()->root();
   int offset = 0;
   int remaining = 0;
   do {
      xcb_get_property_cookie_t cookie = xcb_get_property(xcb_connection(), false, root, atom(QXcbAtom::_NET_VIRTUAL_ROOTS),
            XCB_ATOM_WINDOW, offset, 1024);
      xcb_get_property_reply_t *reply = xcb_get_property_reply(xcb_connection(), cookie, nullptr);

      if (! reply) {
         break;
      }

      remaining = 0;

      if (reply->type == XCB_ATOM_WINDOW && reply->format == 32) {
         int len = xcb_get_property_value_length(reply) / sizeof(xcb_window_t);
         xcb_window_t *roots = (xcb_window_t *)xcb_get_property_value(reply);
         int s = net_virtual_roots.size();
         net_virtual_roots.resize(s + len);
         memcpy(net_virtual_roots.data() + s, roots, len * sizeof(xcb_window_t));

         remaining = reply->bytes_after;
         offset += len;
      }

      free(reply);

   } while (remaining > 0);

#if defined(CS_SHOW_DEBUG_PLATFORM)
   qDebug() << "QXcbWMSupport::updateVirtualRoots()";

   for (int i = 0; i < net_virtual_roots.size(); ++i) {
      qDebug() << "\n   " << connection()->atomName(net_virtual_roots.at(i));
   }
#endif
}


