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

#ifndef QSCRIPTENGINEAGENT_P_H
#define QSCRIPTENGINEAGENT_P_H

#include "Debugger.h"
#include "qscriptengineagent.h"
#include "CallFrame.h"
#include "SourceCode.h"
#include "UString.h"
#include "DebuggerCallFrame.h"

class QScriptEnginePrivate;
class QScriptEngineAgent;

class Q_SCRIPT_EXPORT QScriptEngineAgentPrivate : public JSC::Debugger
{
   Q_DECLARE_PUBLIC(QScriptEngineAgent)

 public:
   static QScriptEngineAgent *get(QScriptEngineAgentPrivate *p) {
      return p->q_func();
   }
   static QScriptEngineAgentPrivate *get(QScriptEngineAgent *p) {
      return p->d_func();
   }

   QScriptEngineAgentPrivate() {}
   virtual ~QScriptEngineAgentPrivate() {}

   void attach();
   void detach();

   //scripts
   virtual void sourceParsed(JSC::ExecState *, const JSC::SourceCode &, int /*errorLine*/,
      const JSC::UString & /*errorMsg*/) {}
   virtual void scriptUnload(qint64 id) {
      q_ptr->scriptUnload(id);
   }
   virtual void scriptLoad(qint64 id, const JSC::UString &program,
      const JSC::UString &fileName, int baseLineNumber) {
      q_ptr->scriptLoad(id, program, fileName, baseLineNumber);
   }

   //exceptions
   virtual void exception(const JSC::DebuggerCallFrame &frame, intptr_t sourceID, int lineno, bool hasHandler) {
      Q_UNUSED(frame);
      Q_UNUSED(sourceID);
      Q_UNUSED(lineno);
      Q_UNUSED(hasHandler);
   }
   virtual void exceptionThrow(const JSC::DebuggerCallFrame &frame, intptr_t sourceID, bool hasHandler);
   virtual void exceptionCatch(const JSC::DebuggerCallFrame &frame, intptr_t sourceID);

   //statements
   virtual void atStatement(const JSC::DebuggerCallFrame &, intptr_t sourceID, int lineno/*, int column*/);
   virtual void callEvent(const JSC::DebuggerCallFrame &, intptr_t sourceID, int lineno) {
      Q_UNUSED(lineno);
      q_ptr->contextPush();
      q_ptr->functionEntry(sourceID);
   }
   virtual void returnEvent(const JSC::DebuggerCallFrame &frame, intptr_t sourceID, int lineno);
   virtual void willExecuteProgram(const JSC::DebuggerCallFrame &frame, intptr_t sourceID, int lineno) {
      Q_UNUSED(frame);
      Q_UNUSED(sourceID);
      Q_UNUSED(lineno);
   }
   virtual void didExecuteProgram(const JSC::DebuggerCallFrame &frame, intptr_t sourceID, int lineno) {
      Q_UNUSED(frame);
      Q_UNUSED(sourceID);
      Q_UNUSED(lineno);
   }
   virtual void functionExit(const JSC::JSValue &returnValue, intptr_t sourceID);
   //others
   virtual void didReachBreakpoint(const JSC::DebuggerCallFrame &frame, intptr_t sourceID, int lineno/*, int column*/);

   virtual void evaluateStart(intptr_t sourceID) {
      q_ptr->functionEntry(sourceID);
   }
   virtual void evaluateStop(const JSC::JSValue &returnValue, intptr_t sourceID);

   QScriptEnginePrivate *engine;
   QScriptEngineAgent *q_ptr;
};

#endif
