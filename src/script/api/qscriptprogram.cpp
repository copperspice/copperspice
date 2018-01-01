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

#include "config.h"
#include "qscriptprogram.h"
#include "qscriptprogram_p.h"
#include "qscriptengine.h"
#include "qscriptengine_p.h"

#include "SamplingTool.h"
#include "Executable.h"

QT_BEGIN_NAMESPACE

/*!
  \since 4.7
  \class QScriptProgram

  \brief The QScriptProgram class encapsulates a Qt Script program.

  \ingroup script

  QScriptProgram retains the compiled representation of the script if
  possible. Thus, QScriptProgram can be used to evaluate the same
  script multiple times more efficiently.

  \code
  QScriptEngine engine;
  QScriptProgram program("1 + 2");
  QScriptValue result = engine.evaluate(program);
  \endcode
*/

QScriptProgramPrivate::QScriptProgramPrivate(const QString &src,
      const QString &fn,
      int ln)
   : sourceCode(src), fileName(fn), firstLineNumber(ln),
     engine(0), _executable(0), sourceId(-1), isCompiled(false)
{
   ref.store(0);
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
   engine = 0;
}

/*!
  Constructs a null QScriptProgram.
*/
QScriptProgram::QScriptProgram()
   : d_ptr(0)
{
}

/*!
  Constructs a new QScriptProgram with the given \a sourceCode, \a
  fileName and \a firstLineNumber.
*/
QScriptProgram::QScriptProgram(const QString &sourceCode,
                               const QString fileName,
                               int firstLineNumber)
   : d_ptr(new QScriptProgramPrivate(sourceCode, fileName, firstLineNumber))
{
}

/*!
  Constructs a new QScriptProgram that is a copy of \a other.
*/
QScriptProgram::QScriptProgram(const QScriptProgram &other)
   : d_ptr(other.d_ptr)
{
}

/*!
  Destroys this QScriptProgram.
*/
QScriptProgram::~QScriptProgram()
{
}

/*!
  Assigns the \a other value to this QScriptProgram.
*/
QScriptProgram &QScriptProgram::operator=(const QScriptProgram &other)
{
   d_ptr = other.d_ptr;
   return *this;
}

/*!
  Returns true if this QScriptProgram is null; otherwise
  returns false.
*/
bool QScriptProgram::isNull() const
{
   Q_D(const QScriptProgram);
   return (d == 0);
}

/*!
  Returns the source code of this program.
*/
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

QT_END_NAMESPACE
