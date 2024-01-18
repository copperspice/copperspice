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

#include "qscriptcompletiontaskinterface_p.h"
#include "qscriptcompletiontaskinterface_p_p.h"

QT_BEGIN_NAMESPACE

QScriptCompletionTaskInterfacePrivate::QScriptCompletionTaskInterfacePrivate()
{
   type = QScriptCompletionTaskInterface::NoCompletion;
}

QScriptCompletionTaskInterfacePrivate::~QScriptCompletionTaskInterfacePrivate()
{
}

QScriptCompletionTaskInterface::~QScriptCompletionTaskInterface()
{
}

QScriptCompletionTaskInterface::QScriptCompletionTaskInterface(
   QScriptCompletionTaskInterfacePrivate &dd, QObject *parent)
   : QObject(dd, parent)
{
}

QScriptCompletionTaskInterface::CompletionType QScriptCompletionTaskInterface::completionType() const
{
   Q_D(const QScriptCompletionTaskInterface);
   return static_cast<QScriptCompletionTaskInterface::CompletionType>(d->type);
}

int QScriptCompletionTaskInterface::resultCount() const
{
   Q_D(const QScriptCompletionTaskInterface);
   return d->results.size();
}

QString QScriptCompletionTaskInterface::resultAt(int index) const
{
   Q_D(const QScriptCompletionTaskInterface);
   return d->results.value(index);
}

void QScriptCompletionTaskInterface::addResult(const QString &result)
{
   Q_D(QScriptCompletionTaskInterface);
   d->results.append(result);
}

int QScriptCompletionTaskInterface::position() const
{
   Q_D(const QScriptCompletionTaskInterface);
   return d->position;
}

int QScriptCompletionTaskInterface::length() const
{
   Q_D(const QScriptCompletionTaskInterface);
   return d->length;
}

QString QScriptCompletionTaskInterface::appendix() const
{
   Q_D(const QScriptCompletionTaskInterface);
   return d->appendix;
}

QT_END_NAMESPACE
