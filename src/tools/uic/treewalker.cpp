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

#include <treewalker.h>

#include <ui4.h>

void TreeWalker::acceptUI(DomUI *ui)
{
   acceptWidget(ui->elementWidget());

   const DomButtonGroups *domButtonGroups = ui->elementButtonGroups();

   if (domButtonGroups != nullptr) {
      acceptButtonGroups(domButtonGroups);
   }

   acceptTabStops(ui->elementTabStops());

   if (ui->elementImages()) {
      acceptImages(ui->elementImages());
   }
}

void TreeWalker::acceptLayoutDefault(DomLayoutDefault *layoutDefault)
{
   (void) layoutDefault;
}

void TreeWalker::acceptLayoutFunction(DomLayoutFunction *layoutFunction)
{
   (void) layoutFunction;
}

void TreeWalker::acceptTabStops(DomTabStops *tabStops)
{
   (void) tabStops;
}

void TreeWalker::acceptLayout(DomLayout *layout)
{
   for (auto item : layout->elementProperty()) {
      acceptProperty(item);
   }

   for (auto item : layout->elementItem()) {
      acceptLayoutItem(item);
   }
}

void TreeWalker::acceptLayoutItem(DomLayoutItem *layoutItem)
{
   switch (layoutItem->kind()) {
      case DomLayoutItem::Widget:
         acceptWidget(layoutItem->elementWidget());
         return;

      case DomLayoutItem::Layout:
         acceptLayout(layoutItem->elementLayout());
         return;

      case DomLayoutItem::Spacer:
         acceptSpacer(layoutItem->elementSpacer());
         return;

      case DomLayoutItem::Unknown:
         break;
   }

   Q_ASSERT(0);
}

void TreeWalker::acceptWidget(DomWidget *widget)
{
   for (auto item : widget->elementAction()) {
      acceptAction(item);
   }

   for (auto item : widget->elementActionGroup()) {
      acceptActionGroup(item);
   }

   for (auto item : widget->elementAddAction()) {
      acceptActionRef(item);
   }

   for (auto item : widget->elementProperty()) {
      acceptProperty(item);
   }

   // recurse down
   DomWidgets childWidgets;

   for (auto child : widget->elementWidget()) {
      childWidgets += child;
      acceptWidget(child);
   }

   if (! widget->elementLayout().isEmpty()) {
      acceptLayout(widget->elementLayout().at(0));
   }

   const DomScripts scripts(widget->elementScript());
   acceptWidgetScripts(scripts, widget, childWidgets);
}

void TreeWalker::acceptSpacer(DomSpacer *spacer)
{
   for (auto item :  spacer->elementProperty()) {
      acceptProperty(item);
   }
}

void TreeWalker::acceptColor(DomColor *color)
{
   (void) color;
}

void TreeWalker::acceptColorGroup(DomColorGroup *colorGroup)
{
   (void) colorGroup;
}

void TreeWalker::acceptPalette(DomPalette *palette)
{
   acceptColorGroup(palette->elementActive());
   acceptColorGroup(palette->elementInactive());
   acceptColorGroup(palette->elementDisabled());
}

void TreeWalker::acceptFont(DomFont *font)
{
   (void) font;
}

void TreeWalker::acceptPoint(DomPoint *point)
{
   (void) point;
}

void TreeWalker::acceptRect(DomRect *rect)
{
   (void) rect;
}

void TreeWalker::acceptSizePolicy(DomSizePolicy *sizePolicy)
{
   (void) sizePolicy;
}

void TreeWalker::acceptSize(DomSize *size)
{
   (void) size;
}

void TreeWalker::acceptDate(DomDate *date)
{
   (void) date;
}

void TreeWalker::acceptTime(DomTime *time)
{
   (void) time;
}

void TreeWalker::acceptDateTime(DomDateTime *dateTime)
{
   (void) dateTime;
}

void TreeWalker::acceptProperty(DomProperty *property)
{
   switch (property->kind()) {
      case DomProperty::Bool:
      case DomProperty::Color:
      case DomProperty::Cstring:
      case DomProperty::Cursor:
      case DomProperty::CursorShape:
      case DomProperty::Enum:
      case DomProperty::Font:
      case DomProperty::Pixmap:
      case DomProperty::IconSet:
      case DomProperty::Palette:
      case DomProperty::Point:
      case DomProperty::PointF:
      case DomProperty::Rect:
      case DomProperty::RectF:
      case DomProperty::Set:
      case DomProperty::Locale:
      case DomProperty::SizePolicy:
      case DomProperty::Size:
      case DomProperty::SizeF:
      case DomProperty::String:
      case DomProperty::Number:
      case DomProperty::LongLong:
      case DomProperty::Char:
      case DomProperty::Date:
      case DomProperty::Time:
      case DomProperty::DateTime:
      case DomProperty::Url:
      case DomProperty::Unknown:
      case DomProperty::StringList:
      case DomProperty::Float:
      case DomProperty::Double:
      case DomProperty::UInt:
      case DomProperty::ULongLong:
      case DomProperty::Brush:
         break;
   }
}

void TreeWalker::acceptCustomWidgets(DomCustomWidgets *customWidgets)
{
   for (auto item : customWidgets->elementCustomWidget()) {
      acceptCustomWidget(item);
   }
}

void TreeWalker::acceptCustomWidget(DomCustomWidget *customWidget)
{
   (void) customWidget;
}

void TreeWalker::acceptAction(DomAction *action)
{
   (void) action;
}

void TreeWalker::acceptActionGroup(DomActionGroup *actionGroup)
{
   for (auto item : actionGroup->elementAction()) {
      acceptAction(item);
   }

   for (auto item : actionGroup->elementActionGroup()) {
      acceptActionGroup(item);
   }
}

void TreeWalker::acceptActionRef(DomActionRef *actionRef)
{
   (void) actionRef;
}

void TreeWalker::acceptImages(DomImages *images)
{
   for (auto item : images->elementImage()) {
      acceptImage(item);
   }
}

void TreeWalker::acceptImage(DomImage *image)
{
   (void) image;
}

void TreeWalker::acceptIncludes(DomIncludes *includes)
{
   for (auto item : includes->elementInclude()) {
      acceptInclude(item);
   }
}

void TreeWalker::acceptInclude(DomInclude *incl)
{
   (void) incl;
}

void TreeWalker::acceptConnections(DomConnections *connections)
{
   for (auto item : connections->elementConnection()) {
      acceptConnection(item);
   }
}

void TreeWalker::acceptConnection(DomConnection *connection)
{
   acceptConnectionHints(connection->elementHints());
}

void TreeWalker::acceptConnectionHints(DomConnectionHints *connectionHints)
{
   for (auto item : connectionHints->elementHint()) {
      acceptConnectionHint(item);
   }
}

void TreeWalker::acceptConnectionHint(DomConnectionHint *connectionHint)
{
   (void) connectionHint;
}

void TreeWalker::acceptWidgetScripts(const DomScripts &, DomWidget *, const  DomWidgets &)
{
}

void TreeWalker::acceptButtonGroups(const DomButtonGroups *domButtonGroups)
{
   for (auto item : domButtonGroups->elementButtonGroup()) {
      acceptButtonGroup(item);
   }
}

void TreeWalker::acceptButtonGroup(const DomButtonGroup *)
{
}

