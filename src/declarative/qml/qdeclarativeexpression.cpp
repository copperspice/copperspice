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

#include "qdeclarativeexpression.h"
#include "private/qdeclarativeexpression_p.h"

#include "private/qdeclarativeengine_p.h"
#include "private/qdeclarativecontext_p.h"
#include "private/qdeclarativerewrite_p.h"
#include "private/qdeclarativecompiler_p.h"
#include "private/qdeclarativeglobalscriptclass_p.h"

#include <QtCore/qdebug.h>
#include <QtScript/qscriptprogram.h>

#include <private/qscriptdeclarativeclass_p.h>

QT_BEGIN_NAMESPACE

bool QDeclarativeDelayedError::addError(QDeclarativeEnginePrivate *e)
{
   if (!e) {
      return false;
   }

   if (e->inProgressCreations == 0) {
      return false;   // Not in construction
   }

   if (prevError) {
      return true;   // Already in error chain
   }

   prevError = &e->erroredBindings;
   nextError = e->erroredBindings;
   e->erroredBindings = this;
   if (nextError) {
      nextError->prevError = &nextError;
   }

   return true;
}

QDeclarativeQtScriptExpression::QDeclarativeQtScriptExpression()
   : dataRef(0), expressionFunctionMode(ExplicitContext), scopeObject(0), trackChange(false),
     guardList(0), guardListLength(0), guardObject(0), guardObjectNotifyIndex(-1), deleted(0)
{
}

QDeclarativeQtScriptExpression::~QDeclarativeQtScriptExpression()
{
   if (guardList) {
      delete [] guardList;
      guardList = 0;
   }
   if (dataRef) {
      dataRef->release();
   }
   if (deleted) {
      *deleted = true;
   }
}

QDeclarativeExpressionPrivate::QDeclarativeExpressionPrivate()
   : expressionFunctionValid(true), line(-1)
{
}

QDeclarativeExpressionPrivate::~QDeclarativeExpressionPrivate()
{
}

void QDeclarativeExpressionPrivate::init(QDeclarativeContextData *ctxt, const QString &expr,
      QObject *me)
{
   expression = expr;

   QDeclarativeAbstractExpression::setContext(ctxt);
   scopeObject = me;
   expressionFunctionValid = false;
}

void QDeclarativeExpressionPrivate::init(QDeclarativeContextData *ctxt, const QScriptValue &func,
      QObject *me)
{
   expression = func.toString();

   QDeclarativeAbstractExpression::setContext(ctxt);
   scopeObject = me;

   expressionFunction = func;
   expressionFunctionMode = ExplicitContext;
   expressionFunctionValid = true;
}

void QDeclarativeExpressionPrivate::init(QDeclarativeContextData *ctxt, void *expr,
      QDeclarativeRefCount *rc,
      QObject *me, const QString &srcUrl, int lineNumber)
{
   url = srcUrl;
   line = lineNumber;

   if (dataRef) {
      dataRef->release();
   }
   dataRef = rc;
   if (dataRef) {
      dataRef->addref();
   }

   quint32 *exprData = (quint32 *)expr;
   QDeclarativeCompiledData *dd = (QDeclarativeCompiledData *)rc;

   expression = QString::fromRawData((QChar *)(exprData + 2), exprData[1]);

   int progIdx = *(exprData);
   bool isSharedProgram = progIdx & 0x80000000;
   progIdx &= 0x7FFFFFFF;

   QDeclarativeEngine *engine = ctxt->engine;
   QDeclarativeEnginePrivate *ep = QDeclarativeEnginePrivate::get(engine);
   QScriptEngine *scriptEngine = QDeclarativeEnginePrivate::getScriptEngine(engine);

   if (isSharedProgram) {

      if (!dd->cachedClosures.at(progIdx)) {
         QScriptContext *scriptContext = QScriptDeclarativeClass::pushCleanContext(scriptEngine);
         scriptContext->pushScope(ep->contextClass->newSharedContext());
         scriptContext->pushScope(ep->globalClass->staticGlobalObject());
         dd->cachedClosures[progIdx] = new QScriptValue(scriptEngine->evaluate(expression, url, line));
         scriptEngine->popContext();
      }

      expressionFunction = *dd->cachedClosures.at(progIdx);
      expressionFunctionMode = SharedContext;
      expressionFunctionValid = true;

   } else {

      if (!dd->cachedPrograms.at(progIdx)) {
         dd->cachedPrograms[progIdx] = new QScriptProgram(expression, url, line);
      }

      expressionFunction = evalInObjectScope(ctxt, me, *dd->cachedPrograms.at(progIdx),
                                             &expressionContext);

      expressionFunctionMode = ExplicitContext;
      expressionFunctionValid = true;
   }

   QDeclarativeAbstractExpression::setContext(ctxt);
   scopeObject = me;
}

QScriptValue QDeclarativeExpressionPrivate::evalInObjectScope(QDeclarativeContextData *context, QObject *object,
      const QString &program, const QString &fileName,
      int lineNumber, QScriptValue *contextObject)
{
   QDeclarativeEnginePrivate *ep = QDeclarativeEnginePrivate::get(context->engine);
   QScriptContext *scriptContext = QScriptDeclarativeClass::pushCleanContext(&ep->scriptEngine);
   if (contextObject) {
      *contextObject = ep->contextClass->newContext(context, object);
      scriptContext->pushScope(*contextObject);
   } else {
      scriptContext->pushScope(ep->contextClass->newContext(context, object));
   }
   scriptContext->pushScope(ep->globalClass->staticGlobalObject());
   QScriptValue rv = ep->scriptEngine.evaluate(program, fileName, lineNumber);
   ep->scriptEngine.popContext();
   return rv;
}

QScriptValue QDeclarativeExpressionPrivate::evalInObjectScope(QDeclarativeContextData *context, QObject *object,
      const QScriptProgram &program,
      QScriptValue *contextObject)
{
   QDeclarativeEnginePrivate *ep = QDeclarativeEnginePrivate::get(context->engine);
   QScriptContext *scriptContext = QScriptDeclarativeClass::pushCleanContext(&ep->scriptEngine);
   if (contextObject) {
      *contextObject = ep->contextClass->newContext(context, object);
      scriptContext->pushScope(*contextObject);
   } else {
      scriptContext->pushScope(ep->contextClass->newContext(context, object));
   }
   scriptContext->pushScope(ep->globalClass->staticGlobalObject());
   QScriptValue rv = ep->scriptEngine.evaluate(program);
   ep->scriptEngine.popContext();
   return rv;
}

/*!
    \class QDeclarativeExpression
    \since 4.7
    \brief The QDeclarativeExpression class evaluates JavaScript in a QML context.

    For example, given a file \c main.qml like this:

    \qml
    import QtQuick 1.0

    Item {
        width: 200; height: 200
    }
    \endqml

    The following code evaluates a JavaScript expression in the context of the
    above QML:

    \code
    QDeclarativeEngine *engine = new QDeclarativeEngine;
    QDeclarativeComponent component(engine, QUrl::fromLocalFile("main.qml"));

    QObject *myObject = component.create();
    QDeclarativeExpression *expr = new QDeclarativeExpression(engine->rootContext(), myObject, "width * 2");
    int result = expr->evaluate().toInt();  // result = 400
    \endcode
*/

static int QDeclarativeExpression_notifyIdx = -1;

/*!
    Create an invalid QDeclarativeExpression.

    As the expression will not have an associated QDeclarativeContext, this will be a
    null expression object and its value will always be an invalid QVariant.
 */
QDeclarativeExpression::QDeclarativeExpression()
   : QObject(*new QDeclarativeExpressionPrivate, 0)
{
   Q_D(QDeclarativeExpression);

   if (QDeclarativeExpression_notifyIdx == -1) {
      QDeclarativeExpression_notifyIdx = QDeclarativeExpression::staticMetaObject.indexOfMethod("_q_notify()");
   }
   d->setNotifyObject(this, QDeclarativeExpression_notifyIdx);
}

/*!  \internal */
QDeclarativeExpression::QDeclarativeExpression(QDeclarativeContextData *ctxt, void *expr,
      QDeclarativeRefCount *rc, QObject *me,
      const QString &url, int lineNumber,
      QDeclarativeExpressionPrivate &dd)
   : QObject(dd, 0)
{
   Q_D(QDeclarativeExpression);
   d->init(ctxt, expr, rc, me, url, lineNumber);

   if (QDeclarativeExpression_notifyIdx == -1) {
      QDeclarativeExpression_notifyIdx = QDeclarativeExpression::staticMetaObject.indexOfMethod("_q_notify()");
   }
   d->setNotifyObject(this, QDeclarativeExpression_notifyIdx);
}

/*!
    Create a QDeclarativeExpression object that is a child of \a parent.

    The \a expression JavaScript will be executed in the \a ctxt QDeclarativeContext.
    If specified, the \a scope object's properties will also be in scope during
    the expression's execution.
*/
QDeclarativeExpression::QDeclarativeExpression(QDeclarativeContext *ctxt,
      QObject *scope,
      const QString &expression,
      QObject *parent)
   : QObject(*new QDeclarativeExpressionPrivate, parent)
{
   Q_D(QDeclarativeExpression);
   d->init(QDeclarativeContextData::get(ctxt), expression, scope);

   if (QDeclarativeExpression_notifyIdx == -1) {
      QDeclarativeExpression_notifyIdx = QDeclarativeExpression::staticMetaObject.indexOfMethod("_q_notify()");
   }
   d->setNotifyObject(this, QDeclarativeExpression_notifyIdx);
}

/*!
    \internal
*/
QDeclarativeExpression::QDeclarativeExpression(QDeclarativeContextData *ctxt, QObject *scope,
      const QString &expression)
   : QObject(*new QDeclarativeExpressionPrivate, 0)
{
   Q_D(QDeclarativeExpression);
   d->init(ctxt, expression, scope);

   if (QDeclarativeExpression_notifyIdx == -1) {
      QDeclarativeExpression_notifyIdx = QDeclarativeExpression::staticMetaObject.indexOfMethod("_q_notify()");
   }
   d->setNotifyObject(this, QDeclarativeExpression_notifyIdx);
}

/*!  \internal */
QDeclarativeExpression::QDeclarativeExpression(QDeclarativeContextData *ctxt, QObject *scope,
      const QString &expression, QDeclarativeExpressionPrivate &dd)
   : QObject(dd, 0)
{
   Q_D(QDeclarativeExpression);
   d->init(ctxt, expression, scope);

   if (QDeclarativeExpression_notifyIdx == -1) {
      QDeclarativeExpression_notifyIdx = QDeclarativeExpression::staticMetaObject.indexOfMethod("_q_notify()");
   }
   d->setNotifyObject(this, QDeclarativeExpression_notifyIdx);
}

/*!  \internal */
QDeclarativeExpression::QDeclarativeExpression(QDeclarativeContextData *ctxt, QObject *scope, const QScriptValue &func,
      QDeclarativeExpressionPrivate &dd)
   : QObject(dd, 0)
{
   Q_D(QDeclarativeExpression);
   d->init(ctxt, func, scope);

   if (QDeclarativeExpression_notifyIdx == -1) {
      QDeclarativeExpression_notifyIdx = QDeclarativeExpression::staticMetaObject.indexOfMethod("_q_notify()");
   }
   d->setNotifyObject(this, QDeclarativeExpression_notifyIdx);
}

/*!
    Destroy the QDeclarativeExpression instance.
*/
QDeclarativeExpression::~QDeclarativeExpression()
{
}

/*!
    Returns the QDeclarativeEngine this expression is associated with, or 0 if there
    is no association or the QDeclarativeEngine has been destroyed.
*/
QDeclarativeEngine *QDeclarativeExpression::engine() const
{
   Q_D(const QDeclarativeExpression);
   return d->context() ? d->context()->engine : 0;
}

/*!
    Returns the QDeclarativeContext this expression is associated with, or 0 if there
    is no association or the QDeclarativeContext has been destroyed.
*/
QDeclarativeContext *QDeclarativeExpression::context() const
{
   Q_D(const QDeclarativeExpression);
   QDeclarativeContextData *data = d->context();
   return data ? data->asQDeclarativeContext() : 0;
}

/*!
    Returns the expression string.
*/
QString QDeclarativeExpression::expression() const
{
   Q_D(const QDeclarativeExpression);
   return d->expression;
}

/*!
    Set the expression to \a expression.
*/
void QDeclarativeExpression::setExpression(const QString &expression)
{
   Q_D(QDeclarativeExpression);

   d->resetNotifyOnChange();
   d->expression = expression;
   d->expressionFunctionValid = false;
   d->expressionFunction = QScriptValue();
}

void QDeclarativeExpressionPrivate::exceptionToError(QScriptEngine *scriptEngine,
      QDeclarativeError &error)
{
   if (scriptEngine->hasUncaughtException() &&
         scriptEngine->uncaughtException().isError()) {

      QString fileName;
      int lineNumber = scriptEngine->uncaughtExceptionLineNumber();

      QScriptValue exception = scriptEngine->uncaughtException();
      QLatin1String fileNameProp("fileName");

      if (!exception.property(fileNameProp).toString().isEmpty()) {
         fileName = exception.property(fileNameProp).toString();
      } else {
         fileName = QLatin1String("<Unknown File>");
      }

      error.setUrl(QUrl(fileName));
      error.setLine(lineNumber);
      error.setColumn(-1);
      error.setDescription(exception.toString());
   } else {
      error = QDeclarativeError();
   }
}

bool QDeclarativeQtScriptExpression::notifyOnValueChange() const
{
   return trackChange;
}

void QDeclarativeQtScriptExpression::setNotifyOnValueChange(bool notify)
{
   trackChange = notify;
   if (!notify && guardList) {
      clearGuards();
   }
}

void QDeclarativeQtScriptExpression::resetNotifyOnChange()
{
   clearGuards();
}

void QDeclarativeQtScriptExpression::setNotifyObject(QObject *object, int notifyIndex)
{
   if (guardList) {
      clearGuards();
   }

   if (!object || notifyIndex == -1) {
      guardObject = 0;
      notifyIndex = -1;
   } else {
      guardObject = object;
      guardObjectNotifyIndex = notifyIndex;

   }
}

void QDeclarativeQtScriptExpression::setEvaluateFlags(EvaluateFlags flags)
{
   evalFlags = flags;
}

QDeclarativeQtScriptExpression::EvaluateFlags QDeclarativeQtScriptExpression::evaluateFlags() const
{
   return evalFlags;
}

QScriptValue QDeclarativeQtScriptExpression::scriptValue(QObject *secondaryScope, bool *isUndefined)
{
   Q_ASSERT(context() && context()->engine);
   Q_ASSERT(!trackChange || (guardObject && guardObjectNotifyIndex != -1));

   if (!expressionFunction.isValid()) {
      if (isUndefined) {
         *isUndefined = true;
      }
      return QScriptValue();
   }

   DeleteWatcher watcher(this);

   QDeclarativeEnginePrivate *ep = QDeclarativeEnginePrivate::get(context()->engine);

   bool lastCaptureProperties = ep->captureProperties;
   QPODVector<QDeclarativeEnginePrivate::CapturedProperty, 16> lastCapturedProperties;
   ep->captureProperties = trackChange;
   ep->capturedProperties.copyAndClear(lastCapturedProperties);

   QScriptValue value = eval(secondaryScope, isUndefined);

   if (!watcher.wasDeleted() && trackChange) {
      if (ep->capturedProperties.count() == 0) {

         if (guardList) {
            clearGuards();
         }

      } else {

         updateGuards(ep->capturedProperties);

      }
   }

   lastCapturedProperties.copyAndClear(ep->capturedProperties);
   ep->captureProperties = lastCaptureProperties;

   return value;
}

QScriptValue QDeclarativeQtScriptExpression::eval(QObject *secondaryScope, bool *isUndefined)
{
   Q_ASSERT(context() && context()->engine);

   DeleteWatcher watcher(this);

   QDeclarativeEngine *engine = context()->engine;
   QDeclarativeEnginePrivate *ep = QDeclarativeEnginePrivate::get(engine);

   QScriptEngine *scriptEngine = QDeclarativeEnginePrivate::getScriptEngine(engine);

   QDeclarativeContextData *oldSharedContext = 0;
   QObject *oldSharedScope = 0;
   QObject *oldOverride = 0;
   bool isShared = (expressionFunctionMode == SharedContext);

   if (isShared) {
      oldSharedContext = ep->sharedContext;
      oldSharedScope = ep->sharedScope;
      ep->sharedContext = context();
      ep->sharedScope = scopeObject;
   } else {
      oldOverride = ep->contextClass->setOverrideObject(expressionContext, secondaryScope);
   }

   QScriptValue thisObject;
   if (evalFlags & RequiresThisObject) {
      thisObject = ep->objectClass->newQObject(scopeObject);
   }
   QScriptValue svalue = expressionFunction.call(thisObject); // This could cause this c++ object to be deleted

   if (isShared) {
      ep->sharedContext = oldSharedContext;
      ep->sharedScope = oldSharedScope;
   } else if (!watcher.wasDeleted()) {
      ep->contextClass->setOverrideObject(expressionContext, oldOverride);
   }

   if (isUndefined) {
      *isUndefined = svalue.isUndefined() || scriptEngine->hasUncaughtException();
   }

   // Handle exception
   if (scriptEngine->hasUncaughtException()) {
      if (!watcher.wasDeleted()) {
         QDeclarativeExpressionPrivate::exceptionToError(scriptEngine, error);
      }

      scriptEngine->clearExceptions();
      return QScriptValue();
   } else {
      if (!watcher.wasDeleted()) {
         error = QDeclarativeError();
      }

      return svalue;
   }
}

void QDeclarativeQtScriptExpression::updateGuards(const QPODVector<QDeclarativeEnginePrivate::CapturedProperty, 16>
      &properties)
{
   Q_ASSERT(guardObject);
   Q_ASSERT(guardObjectNotifyIndex != -1);

   if (properties.count() != guardListLength) {
      QDeclarativeNotifierEndpoint *newGuardList = new QDeclarativeNotifierEndpoint[properties.count()];

      for (int ii = 0; ii < qMin(guardListLength, properties.count()); ++ii) {
         guardList[ii].copyAndClear(newGuardList[ii]);
      }

      delete [] guardList;
      guardList = newGuardList;
      guardListLength = properties.count();
   }

   bool outputWarningHeader = false;
   bool noChanges = true;
   for (int ii = 0; ii < properties.count(); ++ii) {
      QDeclarativeNotifierEndpoint &guard = guardList[ii];
      const QDeclarativeEnginePrivate::CapturedProperty &property = properties.at(ii);

      guard.target = guardObject;
      guard.targetMethod = guardObjectNotifyIndex;

      if (property.notifier != 0) {

         if (!noChanges && guard.isConnected(property.notifier)) {
            // Nothing to do

         } else {
            noChanges = false;

            bool existing = false;
            for (int jj = 0; !existing && jj < ii; ++jj)
               if (guardList[jj].isConnected(property.notifier)) {
                  existing = true;
               }

            if (existing) {
               // duplicate
               guard.disconnect();
            } else {
               guard.connect(property.notifier);
            }
         }


      } else if (property.notifyIndex != -1) {

         if (!noChanges && guard.isConnected(property.object, property.notifyIndex)) {
            // Nothing to do

         } else {
            noChanges = false;

            bool existing = false;
            for (int jj = 0; !existing && jj < ii; ++jj)
               if (guardList[jj].isConnected(property.object, property.notifyIndex)) {
                  existing = true;
               }

            if (existing) {
               // duplicate
               guard.disconnect();
            } else {
               guard.connect(property.object, property.notifyIndex);
            }
         }

      } else {
         if (!outputWarningHeader) {
            outputWarningHeader = true;
            qWarning() << "QDeclarativeExpression: Expression" << expression
                       << "depends on non-NOTIFYable properties:";
         }

         const QMetaObject *metaObj = property.object->metaObject();
         QMetaProperty metaProp = metaObj->property(property.coreIndex);

         qWarning().nospace() << "    " << metaObj->className() << "::" << metaProp.name();
      }
   }
}

QScriptValue QDeclarativeExpressionPrivate::scriptValue(QObject *secondaryScope, bool *isUndefined)
{
   if (!expressionFunctionValid) {
      QDeclarativeEngine *engine = context()->engine;
      QDeclarativeEnginePrivate *ep = QDeclarativeEnginePrivate::get(engine);

      QScriptEngine *scriptEngine = QDeclarativeEnginePrivate::getScriptEngine(engine);

      QScriptContext *scriptContext = QScriptDeclarativeClass::pushCleanContext(scriptEngine);
      expressionContext = ep->contextClass->newContext(context(), scopeObject);
      scriptContext->pushScope(expressionContext);
      scriptContext->pushScope(ep->globalClass->staticGlobalObject());

      QDeclarativeRewrite::RewriteBinding rewriteBinding;
      rewriteBinding.setName(name);
      bool ok = true;
      const QString code = rewriteBinding(expression, &ok);
      if (ok) {
         expressionFunction = scriptEngine->evaluate(code, url, line);
      }

      scriptEngine->popContext();
      expressionFunctionMode = ExplicitContext;
      expressionFunctionValid = true;
   }

   return QDeclarativeQtScriptExpression::scriptValue(secondaryScope, isUndefined);
}

QVariant QDeclarativeExpressionPrivate::value(QObject *secondaryScope, bool *isUndefined)
{
   Q_Q(QDeclarativeExpression);

   if (!context() || !context()->isValid()) {
      qWarning("QDeclarativeExpression: Attempted to evaluate an expression in an invalid context");
      return QVariant();
   }

   QDeclarativeEnginePrivate *ep = QDeclarativeEnginePrivate::get(q->engine());

   return ep->scriptValueToVariant(scriptValue(secondaryScope, isUndefined), qMetaTypeId<QList<QObject *> >());
}

/*!
    Evaulates the expression, returning the result of the evaluation,
    or an invalid QVariant if the expression is invalid or has an error.

    \a valueIsUndefined is set to true if the expression resulted in an
    undefined value.

    \sa hasError(), error()
*/
QVariant QDeclarativeExpression::evaluate(bool *valueIsUndefined)
{
   Q_D(QDeclarativeExpression);
   return d->value(0, valueIsUndefined);
}

/*!
Returns true if the valueChanged() signal is emitted when the expression's evaluated
value changes.
*/
bool QDeclarativeExpression::notifyOnValueChanged() const
{
   Q_D(const QDeclarativeExpression);
   return d->notifyOnValueChange();
}

/*!
  Sets whether the valueChanged() signal is emitted when the
  expression's evaluated value changes.

  If \a notifyOnChange is true, the QDeclarativeExpression will
  monitor properties involved in the expression's evaluation, and emit
  QDeclarativeExpression::valueChanged() if they have changed.  This
  allows an application to ensure that any value associated with the
  result of the expression remains up to date.

  If \a notifyOnChange is false (default), the QDeclarativeExpression
  will not montitor properties involved in the expression's
  evaluation, and QDeclarativeExpression::valueChanged() will never be
  emitted.  This is more efficient if an application wants a "one off"
  evaluation of the expression.
*/
void QDeclarativeExpression::setNotifyOnValueChanged(bool notifyOnChange)
{
   Q_D(QDeclarativeExpression);
   d->setNotifyOnValueChange(notifyOnChange);
}

/*!
    Returns the source file URL for this expression.  The source location must
    have been previously set by calling setSourceLocation().
*/
QString QDeclarativeExpression::sourceFile() const
{
   Q_D(const QDeclarativeExpression);
   return d->url;
}

/*!
    Returns the source file line number for this expression.  The source location
    must have been previously set by calling setSourceLocation().
*/
int QDeclarativeExpression::lineNumber() const
{
   Q_D(const QDeclarativeExpression);
   return d->line;
}

/*!
    Set the location of this expression to \a line of \a url. This information
    is used by the script engine.
*/
void QDeclarativeExpression::setSourceLocation(const QString &url, int line)
{
   Q_D(QDeclarativeExpression);
   d->url = url;
   d->line = line;
}

/*!
    Returns the expression's scope object, if provided, otherwise 0.

    In addition to data provided by the expression's QDeclarativeContext, the scope
    object's properties are also in scope during the expression's evaluation.
*/
QObject *QDeclarativeExpression::scopeObject() const
{
   Q_D(const QDeclarativeExpression);
   return d->scopeObject;
}

/*!
    Returns true if the last call to evaluate() resulted in an error,
    otherwise false.

    \sa error(), clearError()
*/
bool QDeclarativeExpression::hasError() const
{
   Q_D(const QDeclarativeExpression);
   return d->error.isValid();
}

/*!
    Clear any expression errors.  Calls to hasError() following this will
    return false.

    \sa hasError(), error()
*/
void QDeclarativeExpression::clearError()
{
   Q_D(QDeclarativeExpression);
   d->error = QDeclarativeError();
}

/*!
    Return any error from the last call to evaluate().  If there was no error,
    this returns an invalid QDeclarativeError instance.

    \sa hasError(), clearError()
*/

QDeclarativeError QDeclarativeExpression::error() const
{
   Q_D(const QDeclarativeExpression);
   return d->error;
}

/*! \internal */
void QDeclarativeExpressionPrivate::_q_notify()
{
   emitValueChanged();
}

void QDeclarativeQtScriptExpression::clearGuards()
{
   delete [] guardList;
   guardList = 0;
   guardListLength = 0;
}

/*!
    \fn void QDeclarativeExpression::valueChanged()

    Emitted each time the expression value changes from the last time it was
    evaluated.  The expression must have been evaluated at least once (by
    calling QDeclarativeExpression::evaluate()) before this signal will be emitted.
*/

void QDeclarativeExpressionPrivate::emitValueChanged()
{
   Q_Q(QDeclarativeExpression);
   emit q->valueChanged();
}

QDeclarativeAbstractExpression::QDeclarativeAbstractExpression()
   : m_context(0), m_prevExpression(0), m_nextExpression(0)
{
}

QDeclarativeAbstractExpression::~QDeclarativeAbstractExpression()
{
   if (m_prevExpression) {
      *m_prevExpression = m_nextExpression;
      if (m_nextExpression) {
         m_nextExpression->m_prevExpression = m_prevExpression;
      }
   }
}

QDeclarativeContextData *QDeclarativeAbstractExpression::context() const
{
   return m_context;
}

void QDeclarativeAbstractExpression::setContext(QDeclarativeContextData *context)
{
   if (m_prevExpression) {
      *m_prevExpression = m_nextExpression;
      if (m_nextExpression) {
         m_nextExpression->m_prevExpression = m_prevExpression;
      }
      m_prevExpression = 0;
      m_nextExpression = 0;
   }

   m_context = context;

   if (m_context) {
      m_nextExpression = m_context->expressions;
      if (m_nextExpression) {
         m_nextExpression->m_prevExpression = &m_nextExpression;
      }
      m_prevExpression = &context->expressions;
      m_context->expressions = this;
   }
}

void QDeclarativeAbstractExpression::refresh()
{
}

bool QDeclarativeAbstractExpression::isValid() const
{
   return m_context != 0;
}

void QDeclarativeExpression_q_notify()
{
   Q_D(QDeclarativeExpression);
   d->_q_notify();
}


QT_END_NAMESPACE
