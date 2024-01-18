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

#ifndef QDND_P_H
#define QDND_P_H

#include <qobject.h>
#include <qmap.h>
#include <qmimedata.h>
#include <qdrag.h>
#include <qpixmap.h>
#include <qcursor.h>
#include <qpoint.h>
#include <qwindow.h>
#include <qbackingstore.h>

class QEventLoop;
class QMouseEvent;
class QPlatformDrag;

#if ! (defined(QT_NO_DRAGANDDROP) && defined(QT_NO_CLIPBOARD))

class Q_GUI_EXPORT QInternalMimeData : public QMimeData
{
   GUI_CS_OBJECT(QInternalMimeData)

 public:
   QInternalMimeData();
   ~QInternalMimeData();

   bool hasFormat(const QString &mimeType) const override;
   QStringList formats() const override;
   static bool canReadData(const QString &mimeType);

   static QStringList formatsHelper(const QMimeData *data);
   static bool hasFormatHelper(const QString &mimeType, const QMimeData *data);
   static QByteArray renderDataHelper(const QString &mimeType, const QMimeData *data);

 protected:
   QVariant retrieveData(const QString &mimeType, QVariant::Type type) const override;

   virtual bool hasFormat_sys(const QString &mimeType) const = 0;
   virtual QStringList formats_sys() const = 0;
   virtual QVariant retrieveData_sys(const QString &mimeType, QVariant::Type type) const = 0;
};

#endif

#ifndef QT_NO_DRAGANDDROP

class QDragPrivate
{
 public:

   QDragPrivate()
      : source(nullptr), target(nullptr), data(nullptr)
   { }

   virtual ~QDragPrivate() {};

   QObject *source;
   QObject *target;
   QMimeData *data;
   QPixmap pixmap;
   QPoint hotspot;

   Qt::DropAction executed_action;
   Qt::DropActions supported_actions;
   Qt::DropAction default_action;

   QMap<Qt::DropAction, QPixmap> customCursors;
};

class QDragManager : public QObject
{
   GUI_CS_OBJECT(QDragManager)

 public:
   QDragManager();

   QDragManager(const QDragManager &) = delete;
   QDragManager &operator=(const QDragManager &) = delete;

   ~QDragManager();

   static QDragManager *self();

   Qt::DropAction drag(QDrag *);

   void setCurrentTarget(QObject *target, bool dropped = false);
   QObject *currentTarget() const;

   QDrag *object() const {
      return m_object;
   }
   QObject *source() const;

 private:
   QMimeData *m_platformDropData;

   QObject *m_currentDropTarget;
   QPlatformDrag *m_platformDrag;
   QDrag *m_object;

   static QDragManager *m_instance;
};

#endif // ! QT_NO_DRAGANDDROP

#endif
