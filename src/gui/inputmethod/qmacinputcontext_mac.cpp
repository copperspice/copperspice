/***********************************************************************
*
* Copyright (c) 2012-2014 Barbara Geller
* Copyright (c) 2012-2014 Ansel Sermersheim
* Copyright (c) 2012-2014 Digia Plc and/or its subsidiary(-ies).
* Copyright (c) 2008-2012 Nokia Corporation and/or its subsidiary(-ies).
* All rights reserved.
*
* This file is part of CopperSpice.
*
* CopperSpice is free software: you can redistribute it and/or 
* modify it under the terms of the GNU Lesser General Public License
* version 2.1 as published by the Free Software Foundation.
*
* CopperSpice is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
* Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public
* License along with CopperSpice.  If not, see 
* <http://www.gnu.org/licenses/>.
*
***********************************************************************/

#include <qconfig.h>
#include <qglobal.h>

#ifndef QT_NO_IM
#include <qvarlengtharray.h>
#include <qwidget.h>
#include <qmacinputcontext_p.h>
#include "qtextformat.h"
#include <qdebug.h>
#include <qapplication_p.h>
#include <qkeymapper_p.h>

QT_BEGIN_NAMESPACE

extern bool qt_sendSpontaneousEvent(QObject*, QEvent*);


QMacInputContext::QMacInputContext(QObject *parent)
    : QInputContext(parent), composing(false), recursionGuard(false), textDocument(0),
      keydownEvent(0), lastFocusWid(0)
{
   //  createTextDocument();
}

QMacInputContext::~QMacInputContext()
{
}

void
QMacInputContext::createTextDocument()
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
    if (!w)
        lastFocusWid = focusWidget();

    createTextDocument();
    QInputContext::setFocusWidget(w);
}

void QMacInputContext::initialize()
{
}

void MacInputContext::cleanup()
{
}

void QMacInputContext::setLastKeydownEvent(EventRef event)
{
    EventRef tmpEvent = keydownEvent;
    keydownEvent = event;

    if (keydownEvent)
        RetainEvent(keydownEvent);

    if (tmpEvent)
        ReleaseEvent(tmpEvent);
}

OSStatus QMacInputContext::globalEventProcessor(EventHandlerCallRef, EventRef event, void *)
{
    Q_UNUSED(event);
    return noErr;
}

QT_END_NAMESPACE

#endif // QT_NO_IM
