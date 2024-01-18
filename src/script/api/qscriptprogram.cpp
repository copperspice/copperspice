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

#include "config.h"

#include <qscriptprogram.h>
#include <qscriptengine.h>

#include <qscriptprogram_p.h>
#include <qscriptengine_p.h>

#include "SamplingTool.h"
#include "Executable.h"

QScriptProgramPrivate::QScriptProgramPrivate(const QString &src, const QString &fn, int ln)
   : sourceCode(src), fileName(fn), firstLineNumber(ln),
     engine(nullptr), _executable(nullptr), sourceId(-1), isCompiled(false)
{
}

QScriptProgramPrivate::~QScriptProgramPrivate()
{
   if (engine) {
      QScript::APIShim shim(engine);
      _executable.clear();
      engine->unregisterScriptProgram(this);
   }
}

QScriptProgramPrivate *QScriptProgramPrivate::get(const QScriptProgram &q)
{
   return const_cast<QScriptProgramPrivate *>(q.d_func());
}

JSC::EvalExecutable *QScriptProgramPrivate::executable(JSC::ExecState *exec,
   QScriptEnginePrivate *eng)
{
   if (_executable) {
      if (eng == engine) {
         return _executable.get();
      }

      // "Migrating" to another engine; clean up old state
      QScript::APIShim shim(engine);
      _executable.clear();
      engine->unregisterScriptProgram(this);
   }

   WTF::PassRefPtr<QScript::UStringSourceProviderWithFeedback> provider
      = QScript::UStringSourceProviderWithFeedback::create(sourceCode, fileName, firstLineNumber, eng);
   sourceId = provider->asID();
   JSC::SourceCode source(provider, firstLineNumber); //after construction of SourceCode provider variable will be null.
   _executable = JSC::EvalExecutable::create(exec, source);
   engine = eng;
   engine->registerScriptProgram(this);
   isCompiled = false;
   return _executable.get();
}

void QScriptProgramPrivate::detachFromEngine()
{
   _executable.clear();
   sourceId = -1;
   isCompiled = false;
   engine = nullptr;
}

QScriptProgram::QScriptProgram()
   : d_ptr(nullptr)
{
}

QScriptProgram::QScriptProgram(const QString &sourceCode,
   const QString fileName,
   int firstLineNumber)
   : d_ptr(new QScriptProgramPrivate(sourceCode, fileName, firstLineNumber))
{
}

QScriptProgram::QScriptProgram(const QScriptProgram &other)
   : d_ptr(other.d_ptr)
{
}

QScriptProgram::~QScriptProgram()
{
}

QScriptProgram &QScriptProgram::operator=(const QScriptProgram &other)
{
   d_ptr = other.d_ptr;
   return *this;
}

bool QScriptProgram::isNull() const
{
   Q_D(const QScriptProgram);
   return (d == nullptr);
}

QString QScriptProgram::sourceCode() const
{
   Q_D(const QScriptProgram);
   if (!d) {
      return QString();
   }
   return d->sourceCode;
}

/*!
  Returns the filename associated with this program.
*/
QString QScriptProgram::fileName() const
{
   Q_D(const QScriptProgram);
   if (!d) {
      return QString();
   }
   return d->fileName;
}

/*!
  Returns the line number associated with this program.
*/
int QScriptProgram::firstLineNumber() const
{
   Q_D(const QScriptProgram);
   if (!d) {
      return -1;
   }
   return d->firstLineNumber;
}

/*!
  Returns true if this QScriptProgram is equal to \a other;
  otherwise returns false.
*/
bool QScriptProgram::operator==(const QScriptProgram &other) const
{
   Q_D(const QScriptProgram);
   if (d == other.d_func()) {
      return true;
   }
   return (sourceCode() == other.sourceCode())
      && (fileName() == other.fileName())
      && (firstLineNumber() == other.firstLineNumber());
}

/*!
  Returns true if this QScriptProgram is not equal to \a other;
  otherwise returns false.
*/
bool QScriptProgram::operator!=(const QScriptProgram &other) const
{
   return !operator==(other);
}

