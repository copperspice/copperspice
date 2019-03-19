/***********************************************************************
*
* Copyright (c) 2012-2019 Barbara Geller
* Copyright (c) 2012-2019 Ansel Sermersheim
*
* Copyright (C) 2015 The Qt Company Ltd.
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

#include "treewalker.h"
#include "ui4.h"

QT_BEGIN_NAMESPACE

void TreeWalker::acceptUI(DomUI *ui)
{
   acceptWidget(ui->elementWidget());
   if (const DomButtonGroups *domButtonGroups = ui->elementButtonGroups()) {
      acceptButtonGroups(domButtonGroups);
   }

   acceptTabStops(ui->elementTabStops());

   if (ui->elementImages()) {
      acceptImages(ui->elementImages());
   }
}

void TreeWalker::acceptLayoutDefault(DomLayoutDefault *layoutDefault)
{
   Q_UNUSED(layoutDefault);
}

void TreeWalker::acceptLayoutFunction(DomLayoutFunction *layoutFunction)
{
   Q_UNUSED(layoutFunction);
}

void TreeWalker::acceptTabStops(DomTabStops *tabStops)
{
   Q_UNUSED(tabStops);
}

void TreeWalker::acceptLayout(DomLayout *layout)
{
   for (int i = 0; i < layout->elementProperty().size(); ++i) {
      acceptProperty(layout->elementProperty().at(i));
   }

   for (int i = 0; i < layout->elementItem().size(); ++i) {
      acceptLayoutItem(layout->elementItem().at(i));
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

   Q_ASSERT( 0 );
}

void TreeWalker::acceptWidget(DomWidget *widget)
{
   for (int i = 0; i < widget->elementAction().size(); ++i) {
      acceptAction(widget->elementAction().at(i));
   }

   for (int i = 0; i < widget->elementActionGroup().size(); ++i) {
      acceptActionGroup(widget->elementActionGroup().at(i));
   }

   for (int i = 0; i < widget->elementAddAction().size(); ++i) {
      acceptActionRef(widget->elementAddAction().at(i));
   }

   for (int i = 0; i < widget->elementProperty().size(); ++i) {
      acceptProperty(widget->elementProperty().at(i));
   }



   // recurse down
   DomWidgets childWidgets;
   for (int i = 0; i < widget->elementWidget().size(); ++i) {
      DomWidget *child = widget->elementWidget().at(i);
      childWidgets += child;
      acceptWidget(child);
   }

   if (!widget->elementLayout().isEmpty()) {
      acceptLayout(widget->elementLayout().at(0));
   }

   const DomScripts scripts(widget->elementScript());
   acceptWidgetScripts(scripts, widget, childWidgets);
}

void TreeWalker::acceptSpacer(DomSpacer *spacer)
{
   for (int i = 0; i < spacer->elementProperty().size(); ++i) {
      acceptProperty(spacer->elementProperty().at(i));
   }
}

void TreeWalker::acceptColor(DomColor *color)
{
   Q_UNUSED(color);
}

void TreeWalker::acceptColorGroup(DomColorGroup *colorGroup)
{
   Q_UNUSED(colorGroup);
}

void TreeWalker::acceptPalette(DomPalette *palette)
{
   acceptColorGroup(palette->elementActive());
   acceptColorGroup(palette->elementInactive());
   acceptColorGroup(palette->elementDisabled());
}

void TreeWalker::acceptFont(DomFont *font)
{
   Q_UNUSED(font);
}

void TreeWalker::acceptPoint(DomPoint *point)
{
   Q_UNUSED(point);
}

void TreeWalker::acceptRect(DomRect *rect)
{
   Q_UNUSED(rect);
}

void TreeWalker::acceptSizePolicy(DomSizePolicy *sizePolicy)
{
   Q_UNUSED(sizePolicy);
}

void TreeWalker::acceptSize(DomSize *size)
{
   Q_UNUSED(size);
}

void TreeWalker::acceptDate(DomDate *date)
{
   Q_UNUSED(date);
}

void TreeWalker::acceptTime(DomTime *time)
{
   Q_UNUSED(time);
}

void TreeWalker::acceptDateTime(DomDateTime *dateTime)
{
   Q_UNUSED(dateTime);
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
   for (int i = 0; i < customWidgets->elementCustomWidget().size(); ++i) {
      acceptCustomWidget(customWidgets->elementCustomWidget().at(i));
   }
}

void TreeWalker::acceptCustomWidget(DomCustomWidget *customWidget)
{
   Q_UNUSED(customWidget);
}

void TreeWalker::acceptAction(DomAction *action)
{
   Q_UNUSED(action);
}

void TreeWalker::acceptActionGroup(DomActionGroup *actionGroup)
{
   for (int i = 0; i < actionGroup->elementAction().size(); ++i) {
      acceptAction(actionGroup->elementAction().at(i));
   }

   for (int i = 0; i < actionGroup->elementActionGroup().size(); ++i) {
      acceptActionGroup(actionGroup->elementActionGroup().at(i));
   }
}

void TreeWalker::acceptActionRef(DomActionRef *actionRef)
{
   Q_UNUSED(actionRef);
}

void TreeWalker::acceptImages(DomImages *images)
{
   for (int i = 0; i < images->elementImage().size(); ++i) {
      acceptImage(images->elementImage().at(i));
   }
}

void TreeWalker::acceptImage(DomImage *image)
{
   Q_UNUSED(image);
}

void TreeWalker::acceptIncludes(DomIncludes *includes)
{
   for (int i = 0; i < includes->elementInclude().size(); ++i) {
      acceptInclude(includes->elementInclude().at(i));
   }
}

void TreeWalker::acceptInclude(DomInclude *incl)
{
   Q_UNUSED(incl);
}

void TreeWalker::acceptConnections(DomConnections *connections)
{
   for (int i = 0; i < connections->elementConnection().size(); ++i) {
      acceptConnection(connections->elementConnection().at(i));
   }
}

void TreeWalker::acceptConnection(DomConnection *connection)
{
   acceptConnectionHints(connection->elementHints());
}

void TreeWalker::acceptConnectionHints(DomConnectionHints *connectionHints)
{
   for (int i = 0; i < connectionHints->elementHint().size(); ++i) {
      acceptConnectionHint(connectionHints->elementHint().at(i));
   }
}

void TreeWalker::acceptConnectionHint(DomConnectionHint *connectionHint)
{
   Q_UNUSED(connectionHint);
}

void TreeWalker::acceptWidgetScripts(const DomScripts &, DomWidget *, const  DomWidgets &)
{
}

void TreeWalker::acceptButtonGroups(const DomButtonGroups *domButtonGroups)
{
   typedef QList<DomButtonGroup *> DomButtonGroupList;
   const DomButtonGroupList domGroups = domButtonGroups->elementButtonGroup();
   const DomButtonGroupList::const_iterator cend = domGroups.constEnd();
   for (DomButtonGroupList::const_iterator it = domGroups.constBegin(); it != cend; ++it) {
      acceptButtonGroup(*it);
   }
}

void TreeWalker::acceptButtonGroup(const DomButtonGroup *)
{
}

QT_END_NAMESPACE
