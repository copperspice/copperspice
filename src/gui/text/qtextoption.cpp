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

#include <qtextoption.h>
#include <qapplication.h>
#include <qlist.h>

QT_BEGIN_NAMESPACE

struct QTextOptionPrivate {
   QList<QTextOption::Tab> tabStops;
};

QTextOption::QTextOption()
   : align(Qt::AlignLeft),
     wordWrap(QTextOption::WordWrap),
     design(false),
     unused(0),
     f(0),
     tab(-1),
     d(0)
{
   direction = Qt::LayoutDirectionAuto;
}

QTextOption::QTextOption(Qt::Alignment alignment)
   : align(alignment),
     wordWrap(QTextOption::WordWrap),
     design(false),
     unused(0),
     f(0),
     tab(-1),
     d(0)
{
   direction = QApplication::layoutDirection();
}

QTextOption::~QTextOption()
{
   delete d;
}

QTextOption::QTextOption(const QTextOption &o)
   : align(o.align),
     wordWrap(o.wordWrap),
     design(o.design),
     direction(o.direction),
     unused(o.unused),
     f(o.f),
     tab(o.tab),
     d(0)
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

   QTextOptionPrivate *dNew = 0;
   if (o.d) {
      dNew = new QTextOptionPrivate(*o.d);
   }
   delete d;
   d = dNew;

   align = o.align;
   wordWrap = o.wordWrap;
   design = o.design;
   direction = o.direction;
   unused = o.unused;
   f = o.f;
   tab = o.tab;
   return *this;
}

void QTextOption::setTabArray(QList<qreal> tabStops) // Qt5/const ref
{
   if (!d) {
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

void QTextOption::setTabs(QList<QTextOption::Tab> tabStops) // Qt5/const ref
{
   if (!d) {
      d = new QTextOptionPrivate;
   }
   d->tabStops = tabStops;
}

QList<qreal> QTextOption::tabArray() const
{
   if (!d) {
      return QList<qreal>();
   }

   QList<qreal> answer;
   QList<QTextOption::Tab>::ConstIterator iter = d->tabStops.constBegin();
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

QT_END_NAMESPACE
