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

#include <qglobal.h>

#ifndef QT_NO_IM
#include <qvarlengtharray.h>
#include <qwidget.h>
#include <qmacinputcontext_p.h>
#include <qtextformat.h>
#include <qdebug.h>
#include <qapplication_p.h>
#include <qkeymapper_p.h>

QT_BEGIN_NAMESPACE

extern bool qt_sendSpontaneousEvent(QObject *, QEvent *);

QMacInputContext::QMacInputContext(QObject *parent)
   : QInputContext(parent), composing(false), recursionGuard(false), keydownEvent(0), lastFocusWid(0)
{
}

QMacInputContext::~QMacInputContext()
{
}

void QMacInputContext::createTextDocument()
{
}

QString QMacInputContext::language()
{
   return QString();
}

void QMacInputContext::mouseHandler(int pos, QMouseEvent *e)
{
   Q_UNUSED(pos);
   Q_UNUSED(e);
}

void QMacInputContext::setFocusWidget(QWidget *w)
{
   if (! w) {
      lastFocusWid = focusWidget();
   }

   QInputContext::setFocusWidget(w);
}

void QMacInputContext::initialize()
{
}

void QMacInputContext::cleanup()
{
}

void QMacInputContext::setLastKeydownEvent(EventRef event)
{
   EventRef tmpEvent = keydownEvent;
   keydownEvent = event;

   if (keydownEvent) {
      CS_RetainEvent(keydownEvent);
   }

   if (tmpEvent) {
      CS_ReleaseEvent(tmpEvent);
   }
}

QT_END_NAMESPACE

#endif // QT_NO_IM
