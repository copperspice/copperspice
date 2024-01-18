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

#ifndef QSCRIPTCOMPLETIONTASKINTERFACE_P_H
#define QSCRIPTCOMPLETIONTASKINTERFACE_P_H

#include <qobject.h>

QT_BEGIN_NAMESPACE

class QString;
class QScriptCompletionTaskInterfacePrivate;

class QScriptCompletionTaskInterface : public QObject
{
   SCRIPT_T_CS_OBJECT(QScriptCompletionTaskInterface)

 public:
   enum CompletionType {
      NoCompletion,
      CommandNameCompletion,
      CommandArgumentCompletion,
      ScriptIdentifierCompletion
   };

   ~QScriptCompletionTaskInterface();

   virtual void start() = 0;

   CompletionType completionType() const;

   int resultCount() const;
   QString resultAt(int index) const;

   int position() const;
   int length() const;

   QString appendix() const;

 protected:
   void addResult(const QString &result);

 public:
   CS_SIGNAL_1(Public, void finished())
   CS_SIGNAL_2(finished)

 protected:
   QScriptCompletionTaskInterface(
      QScriptCompletionTaskInterfacePrivate &dd, QObject *parent);

 private:
   Q_DECLARE_PRIVATE(QScriptCompletionTaskInterface)
   Q_DISABLE_COPY(QScriptCompletionTaskInterface)
};

QT_END_NAMESPACE

#endif
