/***********************************************************************
*
* Copyright (c) 2012-2025 Barbara Geller
* Copyright (c) 2012-2025 Ansel Sermersheim
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

#ifndef TREEWALKER_H
#define TREEWALKER_H

#include <qlist.h>

class DomAction;
class DomActionGroup;
class DomActionRef;
class DomButtonGroup;
class DomButtonGroups;
class DomColor;
class DomColorGroup;
class DomConnection;
class DomConnectionHint;
class DomConnectionHints;
class DomConnections;
class DomCustomWidget;
class DomCustomWidgets;
class DomDate;
class DomDateTime;
class DomFont;
class DomImage;
class DomImages;
class DomInclude;
class DomIncludes;
class DomItem;
class DomLayout;
class DomLayoutDefault;
class DomLayoutFunction;
class DomLayoutItem;
class DomPalette;
class DomPoint;
class DomProperty;
class DomRect;
class DomResource;
class DomResourcePixmap;
class DomResources;
class DomSize;
class DomSizePolicy;
class DomSpacer;
class DomString;
class DomTabStops;
class DomTime;
class DomUI;
class DomWidget;

struct TreeWalker {
   virtual ~TreeWalker()
   {
   }

   virtual void acceptUI(DomUI *ui);
   virtual void acceptLayoutDefault(DomLayoutDefault *layoutDefault);
   virtual void acceptLayoutFunction(DomLayoutFunction *layoutFunction);
   virtual void acceptTabStops(DomTabStops *tabStops);
   virtual void acceptCustomWidgets(DomCustomWidgets *customWidgets);
   virtual void acceptCustomWidget(DomCustomWidget *customWidget);
   virtual void acceptLayout(DomLayout *layout);
   virtual void acceptLayoutItem(DomLayoutItem *layoutItem);
   virtual void acceptWidget(DomWidget *widget);
   virtual void acceptSpacer(DomSpacer *spacer);
   virtual void acceptColor(DomColor *color);
   virtual void acceptColorGroup(DomColorGroup *colorGroup);
   virtual void acceptPalette(DomPalette *palette);
   virtual void acceptFont(DomFont *font);
   virtual void acceptPoint(DomPoint *point);
   virtual void acceptRect(DomRect *rect);
   virtual void acceptSizePolicy(DomSizePolicy *sizePolicy);
   virtual void acceptSize(DomSize *size);
   virtual void acceptDate(DomDate *date);
   virtual void acceptTime(DomTime *time);
   virtual void acceptDateTime(DomDateTime *dateTime);
   virtual void acceptProperty(DomProperty *property);

   using DomWidgets = QList<DomWidget *>;

   virtual void acceptImages(DomImages *images);
   virtual void acceptImage(DomImage *image);
   virtual void acceptIncludes(DomIncludes *includes);
   virtual void acceptInclude(DomInclude *incl);
   virtual void acceptAction(DomAction *action);
   virtual void acceptActionGroup(DomActionGroup *actionGroup);
   virtual void acceptActionRef(DomActionRef *actionRef);
   virtual void acceptConnections(DomConnections *connections);
   virtual void acceptConnection(DomConnection *connection);
   virtual void acceptConnectionHints(DomConnectionHints *connectionHints);
   virtual void acceptConnectionHint(DomConnectionHint *connectionHint);
   virtual void acceptButtonGroups(const DomButtonGroups *buttonGroups);
   virtual void acceptButtonGroup(const DomButtonGroup *buttonGroup);
};

#endif
