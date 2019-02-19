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

#ifndef QXCBXSETTINGS_H
#define QXCBXSETTINGS_H

#include "qxcbscreen.h"

class QXcbXSettingsPrivate;

class QXcbXSettings : public QXcbWindowEventListener
{
   Q_DECLARE_PRIVATE(QXcbXSettings)

 public:
   QXcbXSettings(QXcbVirtualDesktop *screen);
   ~QXcbXSettings();
   bool initialized() const;

   QVariant setting(const QByteArray &property) const;

   typedef void (*PropertyChangeFunc)(QXcbVirtualDesktop *screen, const QByteArray &name, const QVariant &property, void *handle);
   void registerCallbackForProperty(const QByteArray &property, PropertyChangeFunc func, void *handle);
   void removeCallbackForHandle(const QByteArray &property, void *handle);
   void removeCallbackForHandle(void *handle);

   void handlePropertyNotifyEvent(const xcb_property_notify_event_t *event) override;
 private:
   QXcbXSettingsPrivate *d_ptr;
};

#endif // QXCBXSETTINGS_H
