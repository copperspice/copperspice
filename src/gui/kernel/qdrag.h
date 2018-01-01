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

#ifndef QDRAG_H
#define QDRAG_H

#include <QtCore/qobject.h>
#include <QScopedPointer>

QT_BEGIN_NAMESPACE

#ifndef QT_NO_DRAGANDDROP
class QMimeData;
class QDragPrivate;
class QWidget;
class QPixmap;
class QPoint;
class QDragManager;

class Q_GUI_EXPORT QDrag : public QObject
{
   GUI_CS_OBJECT(QDrag)
   Q_DECLARE_PRIVATE(QDrag)

 public:
   explicit QDrag(QWidget *dragSource);
   ~QDrag();

   void setMimeData(QMimeData *data);
   QMimeData *mimeData() const;

   void setPixmap(const QPixmap &);
   QPixmap pixmap() const;

   void setHotSpot(const QPoint &hotspot);
   QPoint hotSpot() const;

   QWidget *source() const;
   QWidget *target() const;

   Qt::DropAction start(Qt::DropActions supportedActions = Qt::CopyAction);
   Qt::DropAction exec(Qt::DropActions supportedActions = Qt::MoveAction);
   Qt::DropAction exec(Qt::DropActions supportedActions, Qt::DropAction defaultAction);

   void setDragCursor(const QPixmap &cursor, Qt::DropAction action);

   GUI_CS_SIGNAL_1(Public, void actionChanged(Qt::DropAction action))
   GUI_CS_SIGNAL_2(actionChanged, action)
   GUI_CS_SIGNAL_1(Public, void targetChanged(QWidget *newTarget))
   GUI_CS_SIGNAL_2(targetChanged, newTarget)

 protected:
   QScopedPointer<QDragPrivate> d_ptr;

 private:

#ifdef Q_OS_MAC
   friend class QWidgetPrivate;
#endif

   friend class QDragManager;
   Q_DISABLE_COPY(QDrag)

};

#endif // QT_NO_DRAGANDDROP

QT_END_NAMESPACE

#endif // QDRAG_H
