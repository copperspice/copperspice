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

#ifndef QSCRIPTERRORLOGWIDGET_P_H
#define QSCRIPTERRORLOGWIDGET_P_H

#include <qscripterrorlogwidgetinterface_p.h>

QT_BEGIN_NAMESPACE

class QScriptErrorLogWidgetPrivate;
class QScriptErrorLogWidget:
   public QScriptErrorLogWidgetInterface
{
   SCRIPT_T_CS_OBJECT(QScriptErrorLogWidget)
 public:
   QScriptErrorLogWidget(QWidget *parent = nullptr);
   ~QScriptErrorLogWidget();

   void message(QtMsgType type, const QString &text,
                const QString &fileName = QString(),
                int lineNumber = -1, int columnNumber = -1,
                const QVariant &data = QVariant());

   void clear();

 private:
   Q_DECLARE_PRIVATE(QScriptErrorLogWidget)
   Q_DISABLE_COPY(QScriptErrorLogWidget)
};

QT_END_NAMESPACE

#endif
