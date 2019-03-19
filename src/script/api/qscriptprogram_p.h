/***********************************************************************
*
* Copyright (c) 2012-2019 Barbara Geller
* Copyright (c) 2012-2019 Ansel Sermersheim
*
* Copyright (C) 2015 The Qt Company Ltd.
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

#ifndef QSCRIPTPROGRAM_P_H
#define QSCRIPTPROGRAM_P_H

#include "RefPtr.h"

namespace JSC {
class EvalExecutable;
class ExecState;
}

QT_BEGIN_NAMESPACE

class QScriptEnginePrivate;

class QScriptProgramPrivate
{
 public:
   QScriptProgramPrivate(const QString &sourceCode,
                         const QString &fileName,
                         int firstLineNumber);
   ~QScriptProgramPrivate();

   static QScriptProgramPrivate *get(const QScriptProgram &q);

   JSC::EvalExecutable *executable(JSC::ExecState *exec,
                                   QScriptEnginePrivate *engine);
   void detachFromEngine();

   QAtomicInt ref;

   QString sourceCode;
   QString fileName;
   int firstLineNumber;

   QScriptEnginePrivate *engine;
   WTF::RefPtr<JSC::EvalExecutable> _executable;
   intptr_t sourceId;
   bool isCompiled;
};

QT_END_NAMESPACE

#endif
