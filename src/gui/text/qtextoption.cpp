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

#include <qtextoption.h>

#include <qapplication.h>
#include <qlist.h>

struct QTextOptionPrivate {
   QList<QTextOption::Tab> tabStops;
};

QTextOption::QTextOption()
   : align(Qt::AlignLeft), wordWrap(QTextOption::WordWrap),
     design(false), f(0), tab(-1), d(nullptr)
{
   direction = Qt::LayoutDirectionAuto;
}

QTextOption::QTextOption(Qt::Alignment alignment)
   : align(alignment), wordWrap(QTextOption::WordWrap),
     design(false), f(0), tab(-1), d(nullptr)
{
   direction = QGuiApplication::layoutDirection();
}

QTextOption::~QTextOption()
{
   delete d;
}

QTextOption::QTextOption(const QTextOption &o)
   : align(o.align), wordWrap(o.wordWrap), design(o.design),
     direction(o.direction), f(o.f), tab(o.tab), d(nullptr)
{
   if (o.d) {
      d = new QTextOptionPrivate(*o.d);
   }
}

QTextOption &QTextOption::operator=(const QTextOption &o)
{
   if (this == &o) {
      return *this;
   }

   QTextOptionPrivate *dNew = nullptr;
   if (o.d) {
      dNew = new QTextOptionPrivate(*o.d);
   }

   delete d;
   d = dNew;

   align     = o.align;
   wordWrap  = o.wordWrap;
   design    = o.design;
   direction = o.direction;
   f         = o.f;
   tab       = o.tab;

   return *this;
}

void QTextOption::setTabArray(const QList<qreal> &tabStops)
{
   if (! d) {
      d = new QTextOptionPrivate;
   }

   QList<QTextOption::Tab> tabs;
   QTextOption::Tab tab;

   for (qreal pos : tabStops) {
      tab.position = pos;
      tabs.append(tab);
   }
   d->tabStops = tabs;
}

void QTextOption::setTabs(const QList<QTextOption::Tab> &tabStops)
{
   if (! d) {
      d = new QTextOptionPrivate;
   }
   d->tabStops = tabStops;
}

QList<qreal> QTextOption::tabArray() const
{
   QList<qreal> answer;

   if (! d) {
      return answer;
   }
   QList<QTextOption::Tab>::const_iterator iter = d->tabStops.constBegin();

   while (iter != d->tabStops.constEnd()) {
      answer.append( (*iter).position);
      ++iter;
   }

   return answer;
}

QList<QTextOption::Tab> QTextOption::tabs() const
{
   if (! d) {
      return QList<QTextOption::Tab>();
   }

   return d->tabStops;
}
