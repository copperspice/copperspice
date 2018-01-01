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

#ifndef QWSMANAGER_P_H
#define QWSMANAGER_P_H

#include <QtGui/qregion.h>
#include <QtGui/qdecoration_qws.h>

#ifndef QT_NO_QWS_MANAGER

#include <QtCore/qhash.h>

QT_BEGIN_NAMESPACE

class QWidget;
class QMenu;

class QWSManagerPrivate
{
   Q_DECLARE_PUBLIC(QWSManager)

 public:
   QWSManagerPrivate();
   virtual ~QWSManagerPrivate() {}

   int activeRegion;
   QWidget *managed;
   QMenu *popup;

   enum MenuAction {
      NormalizeAction,
      TitleAction,
      BottomRightAction,
      MinimizeAction,
      MaximizeAction,
      CloseAction,
      LastMenuAction
   };
   QAction *menuActions[LastMenuAction];

   static QWidget *active;
   static QPoint mousePos;

   // Region caching to avoid getting a regiontype's
   // QRegion for each mouse move event
   int previousRegionType;
   bool previousRegionRepainted; // Hover/Press handled
   bool entireDecorationNeedsRepaint;

   struct RegionCaching {
      int regionType;
      QRegion region;
      Qt::WindowFlags windowFlags;
      QRect windowGeometry;
   } cached_region;

   bool newCachedRegion(const QPoint &pos);
   int cachedRegionAt() {
      return cached_region.regionType;
   }

   void dirtyRegion(int decorationRegion, QDecoration::DecorationState state, const QRegion &clip = QRegion());
   void clearDirtyRegions();

   QList<int> dirtyRegions;
   QList<QDecoration::DecorationState> dirtyStates;
   QRegion dirtyClip;
};

#endif // QT_NO_QWS_MANAGER

QT_END_NAMESPACE

#endif // QWSMANAGER_P_H
