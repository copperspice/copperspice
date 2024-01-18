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

#ifndef QCOCOADRAG_H
#define QCOCOADRAG_H

#include <Cocoa/Cocoa.h>

#include <qplatform_drag.h>

#include <qsimpledrag_p.h>
#include <qdnd_p.h>

class QCocoaDrag : public QPlatformDrag
{
 public:
   QCocoaDrag();
   ~QCocoaDrag();

   QMimeData *platformDropData() override;
   Qt::DropAction drag(QDrag *m_drag) override;

   Qt::DropAction defaultAction(Qt::DropActions possibleActions,
      Qt::KeyboardModifiers modifiers) const override;

   /**
   * to meet NSView dragImage:at guarantees, we need to record the original
   * event and view when handling an event in QNSView
   */
   void setLastMouseEvent(NSEvent *event, NSView *view);

   void setAcceptedAction(Qt::DropAction act);

 private:
   QDrag *m_drag;
   NSEvent *m_lastEvent;
   NSView *m_lastView;
   Qt::DropAction m_executed_drop_action;

   QPixmap dragPixmap(QDrag *drag, QPoint &hotSpot) const;
};

class QCocoaDropData : public QInternalMimeData
{
 public:
   QCocoaDropData(NSPasteboard *pasteboard);
   ~QCocoaDropData();

   CFStringRef dropPasteboard;

 protected:
   bool hasFormat_sys(const QString &mimeType) const;
   QStringList formats_sys() const;
   QVariant retrieveData_sys(const QString &mimeType, QVariant::Type type) const;

};

#endif
