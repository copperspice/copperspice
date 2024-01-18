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

#ifndef QWINDOWSCURSOR_H
#define QWINDOWSCURSOR_H

#include <Cocoa/Cocoa.h>

#include <qhash.h>
#include <qplatform_cursor.h>

class QCocoaCursor : public QPlatformCursor
{
 public:
   QCocoaCursor();
   ~QCocoaCursor();

   void changeCursor(QCursor *cursor, QWindow *window) override;
   QPoint pos() const override;
   void setPos(const QPoint &position) override;

 private:
   QHash<Qt::CursorShape, NSCursor *> m_cursors;
   NSCursor *convertCursor(QCursor *cursor);
   NSCursor *createCursorData(QCursor *cursor);
   NSCursor *createCursorFromBitmap(const QBitmap *bitmap, const QBitmap *mask, const QPoint hotspot = QPoint());
   NSCursor *createCursorFromPixmap(const QPixmap pixmap, const QPoint hotspot = QPoint());
};


#endif
