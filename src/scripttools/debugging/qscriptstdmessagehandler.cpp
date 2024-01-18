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

#include "qscriptstdmessagehandler_p.h"
#include <stdio.h>

QT_BEGIN_NAMESPACE

/*!
  \since 4.5
  \class QScriptStdMessageHandler
  \internal

  \brief The QScriptStdMessageHandler class implements a message handler that writes to standard output.
*/

class QScriptStdMessageHandlerPrivate
{
 public:
   QScriptStdMessageHandlerPrivate() {}
   ~QScriptStdMessageHandlerPrivate() {}
};

QScriptStdMessageHandler::QScriptStdMessageHandler()
   : d_ptr(new QScriptStdMessageHandlerPrivate)
{
}

QScriptStdMessageHandler::~QScriptStdMessageHandler()
{
}

void QScriptStdMessageHandler::message(QtMsgType type, const QString &text,
                                       const QString &fileName,
                                       int lineNumber, int columnNumber,
                                       const QVariant &/*data*/)
{
   QString msg;
   if (!fileName.isEmpty() || (lineNumber != -1)) {
      if (!fileName.isEmpty()) {
         msg.append(fileName);
      } else {
         msg.append(QLatin1String("<noname>"));
      }
      if (lineNumber != -1) {
         msg.append(QLatin1Char(':'));
         msg.append(QString::number(lineNumber));
         if (columnNumber != -1) {
            msg.append(QLatin1Char(':'));
            msg.append(QString::number(columnNumber));
         }
      }
      msg.append(QLatin1String(": "));
   }
   msg.append(text);

   FILE *fp = (type == QtDebugMsg) ? stdout : stderr;
   fprintf(fp, "%s\n", msg.toLatin1().constData());
   fflush(fp);
}

QT_END_NAMESPACE
