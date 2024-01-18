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

#ifndef QSCRIPTERRORLOGWIDGETINTERFACE_P_H
#define QSCRIPTERRORLOGWIDGETINTERFACE_P_H

#include <QtGui/qwidget.h>
#include "qscriptmessagehandlerinterface_p.h"

QT_BEGIN_NAMESPACE

class QScriptErrorLogWidgetInterfacePrivate;

class QScriptErrorLogWidgetInterface: public QWidget, public QScriptMessageHandlerInterface
{
   SCRIPT_T_CS_OBJECT(QScriptErrorLogWidgetInterface)

 public:
   ~QScriptErrorLogWidgetInterface();

   virtual void clear() = 0;

 protected:
   QScriptErrorLogWidgetInterface(QScriptErrorLogWidgetInterfacePrivate &dd,
                                  QWidget *parent, Qt::WindowFlags flags);

 private:
   Q_DECLARE_PRIVATE(QScriptErrorLogWidgetInterface)
   Q_DISABLE_COPY(QScriptErrorLogWidgetInterface)
};

QT_END_NAMESPACE

#endif
