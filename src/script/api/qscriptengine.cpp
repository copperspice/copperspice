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

#include <config.h>
#include <qscriptengine.h>

#include <qalgorithms.h>
#include <qcoreapplication.h>
#include <qdir.h>
#include <qdebug.h>
#include <qfile.h>
#include <qfileinfo.h>
#include <qmetaobject.h>
#include <qscriptclass.h>
#include <qscriptcontextinfo.h>
#include <qscriptprogram.h>
#include <qstringlist.h>
#include <qshareddata.h>
#include <qpluginloader.h>
#include <qset.h>
#include <qtextstream.h>
#include <qtimezone.h>
#include <qscriptextensioninterface.h>

#include <qscriptsyntaxchecker_p.h>
#include <qscriptengine_p.h>
#include <qscriptengineagent_p.h>
#include <qscriptcontext_p.h>
#include <qscriptstring_p.h>
#include <qscriptvalue_p.h>
#include <qscriptvalueiterator.h>
#include <qscriptprogram_p.h>
#include <qscriptfunction_p.h>
#include <qscriptclassobject_p.h>
#include <qscriptvariant_p.h>
#include <qscriptqobject_p.h>
#include <qscriptglobalobject_p.h>
#include <qscriptactivationobject_p.h>
#include <qscriptstaticscopeobject_p.h>

#include <math.h>
#include <algorithm>

#include <CodeBlock.h>
#include <Error.h>
#include <Interpreter.h>

#include <ExceptionHelpers.h>
#include <PrototypeFunction.h>
#include <InitializeThreading.h>
#include <ObjectPrototype.h>
#include <SourceCode.h>
#include <FunctionPrototype.h>
#include <TimeoutChecker.h>
#include <JSFunction.h>
#include <Parser.h>
#include <PropertyNameArray.h>
#include <Operations.h>

#include <stdlib.h>

class QScriptSyntaxCheckResultPrivate : public QSharedData
{
 public:
   QScriptSyntaxCheckResultPrivate()
   { }

   ~QScriptSyntaxCheckResultPrivate() {}

   QScriptSyntaxCheckResult::State state;
   int errorColumnNumber;
   int errorLineNumber;
   QString errorMessage;
};

class QScriptTypeInfo
{
 public:
   QScriptTypeInfo()
      : signature(0, '\0'), marshal(nullptr), demarshal(nullptr)
   {
   }

   QByteArray signature;
   QScriptEngine::MarshalFunction marshal;
   QScriptEngine::DemarshalFunction demarshal;
   JSC::JSValue prototype;
};

namespace QScript {

static const qsreal D32 = 4294967296.0;

qint32 ToInt32(qsreal n)
{
   if (qIsNaN(n) || qIsInf(n) || (n == 0)) {
      return 0;
   }

   qsreal sign = (n < 0) ? -1.0 : 1.0;
   qsreal abs_n = fabs(n);

   n = ::fmod(sign * ::floor(abs_n), D32);
   const double D31 = D32 / 2.0;

   if (sign == -1 && n < -D31) {
      n += D32;
   }

   else if (sign != -1 && n >= D31) {
      n -= D32;
   }

   return qint32 (n);
}

quint32 ToUInt32(qsreal n)
{
   if (qIsNaN(n) || qIsInf(n) || (n == 0)) {
      return 0;
   }

   qsreal sign = (n < 0) ? -1.0 : 1.0;
   qsreal abs_n = fabs(n);

   n = ::fmod(sign * ::floor(abs_n), D32);

   if (n < 0) {
      n += D32;
   }

   return quint32 (n);
}

quint16 ToUInt16(qsreal n)
{
   static const qsreal D16 = 65536.0;

   if (qIsNaN(n) || qIsInf(n) || (n == 0)) {
      return 0;
   }

   qsreal sign = (n < 0) ? -1.0 : 1.0;
   qsreal abs_n = fabs(n);

   n = ::fmod(sign * ::floor(abs_n), D16);

   if (n < 0) {
      n += D16;
   }

   return quint16 (n);
}

qsreal ToInteger(qsreal n)
{
   if (qIsNaN(n)) {
      return 0;
   }

   if (n == 0 || qIsInf(n)) {
      return n;
   }

   int sign = n < 0 ? -1 : 1;
   return sign * ::floor(::fabs(n));
}

static const qsreal MsPerSecond = 1000.0;

static inline int MsFromTime(qsreal t)
{
   int r = int(::fmod(t, MsPerSecond));
   return (r >= 0) ? r : r + int(MsPerSecond);
}

/*!
  \internal
  Converts a JS date value (milliseconds) to a QDateTime (local time).
*/
QDateTime MsToDateTime(JSC::ExecState *exec, qsreal t)
{
   if (qIsNaN(t)) {
      return QDateTime();
   }
   JSC::GregorianDateTime tm;
   JSC::msToGregorianDateTime(exec, t, /*output UTC=*/true, tm);
   int ms = MsFromTime(t);

   QDateTime convertedUTC = QDateTime(QDate(tm.year + 1900, tm.month + 1, tm.monthDay),
         QTime(tm.hour, tm.minute, tm.second, ms), QTimeZone::utc());

   return convertedUTC.toLocalTime();
}

/*!
  \internal
  Converts a QDateTime to a JS date value (milliseconds).
*/
qsreal DateTimeToMs(JSC::ExecState *exec, const QDateTime &dt)
{
   if (!dt.isValid()) {
      return qSNaN();
   }
   QDateTime utc = dt.toUTC();
   QDate date = utc.date();
   QTime time = utc.time();
   JSC::GregorianDateTime tm;
   tm.year = date.year() - 1900;
   tm.month = date.month() - 1;
   tm.monthDay = date.day();
   tm.weekDay = date.dayOfWeek();
   tm.yearDay = date.dayOfYear();
   tm.hour = time.hour();
   tm.minute = time.minute();
   tm.second = time.second();
   return JSC::gregorianDateTimeToMS(exec, tm, time.msec(), /*inputIsUTC=*/true);
}

void GlobalClientData::mark(JSC::MarkStack &markStack)
{
   engine->mark(markStack);
}

void GlobalClientData::uncaughtException(JSC::ExecState *exec, unsigned bytecodeOffset,
   JSC::JSValue value)
{
   engine->uncaughtException(exec, bytecodeOffset, value);
}
class TimeoutCheckerProxy : public JSC::TimeoutChecker
{
 public:
   TimeoutCheckerProxy(const JSC::TimeoutChecker &originalChecker)
      : JSC::TimeoutChecker(originalChecker)
      , m_shouldProcessEvents(false)
      , m_shouldAbortEvaluation(false) {
   }

   void setShouldProcessEvents(bool shouldProcess) {
      m_shouldProcessEvents = shouldProcess;
   }

   void setShouldAbort(bool shouldAbort) {
      m_shouldAbortEvaluation = shouldAbort;
   }

   bool shouldAbort() {
      return m_shouldAbortEvaluation;
   }

   virtual bool didTimeOut(JSC::ExecState *exec) {
      if (JSC::TimeoutChecker::didTimeOut(exec)) {
         return true;
      }

      if (m_shouldProcessEvents) {
         QCoreApplication::processEvents();
      }

      return m_shouldAbortEvaluation;
   }

 private:
   bool m_shouldProcessEvents;
   bool m_shouldAbortEvaluation;
};

static int toDigit(char c)
{
   if ((c >= '0') && (c <= '9')) {
      return c - '0';
   } else if ((c >= 'a') && (c <= 'z')) {
      return 10 + c - 'a';
   } else if ((c >= 'A') && (c <= 'Z')) {
      return 10 + c - 'A';
   }
   return -1;
}

qsreal integerFromString(const char *buf, int size, int radix)
{
   if (size == 0) {
      return qSNaN();
   }

   qsreal sign = 1.0;
   int i = 0;
   if (buf[0] == '+') {
      ++i;
   } else if (buf[0] == '-') {
      sign = -1.0;
      ++i;
   }

   if (((size - i) >= 2) && (buf[i] == '0')) {
      if (((buf[i + 1] == 'x') || (buf[i + 1] == 'X'))
         && (radix < 34)) {
         if ((radix != 0) && (radix != 16)) {
            return 0;
         }
         radix = 16;
         i += 2;
      } else {
         if (radix == 0) {
            radix = 8;
            ++i;
         }
      }
   } else if (radix == 0) {
      radix = 10;
   }

   int j = i;
   for ( ; i < size; ++i) {
      int d = toDigit(buf[i]);
      if ((d == -1) || (d >= radix)) {
         break;
      }
   }
   qsreal result;
   if (j == i) {
      if (!qstrcmp(buf, "Infinity")) {
         result = qInf();
      } else {
         result = qSNaN();
      }
   } else {
      result = 0;
      qsreal multiplier = 1;
      for (--i ; i >= j; --i, multiplier *= radix) {
         result += toDigit(buf[i]) * multiplier;
      }
   }
   result *= sign;
   return result;
}

qsreal integerFromString(const QString &str, int radix)
{
   QByteArray ba = str.trimmed().toUtf8();
   return integerFromString(ba.constData(), ba.size(), radix);
}

bool isFunction(JSC::JSValue value)
{
   if (!value || !value.isObject()) {
      return false;
   }
   JSC::CallData callData;
   return (JSC::asObject(value)->getCallData(callData) != JSC::CallTypeNone);
}

static JSC::JSValue JSC_HOST_CALL functionConnect(JSC::ExecState *, JSC::JSObject *, JSC::JSValue,
   const JSC::ArgList &);
static JSC::JSValue JSC_HOST_CALL functionDisconnect(JSC::ExecState *, JSC::JSObject *, JSC::JSValue,
   const JSC::ArgList &);

JSC::JSValue JSC_HOST_CALL functionDisconnect(JSC::ExecState *exec, JSC::JSObject * /*callee*/, JSC::JSValue thisObject,
   const JSC::ArgList &args)
{

   if (args.size() == 0) {
      return JSC::throwError(exec, JSC::GeneralError, "Function.prototype.disconnect() No arguments passed");
   }

   if (! JSC::asObject(thisObject)->inherits(&QScript::QtFunction::info)) {
      return JSC::throwError(exec, JSC::TypeError, "Function.prototype.disconnect() Object is not a signal");
   }

   QScript::QtFunction *qtSignal = static_cast<QScript::QtFunction *>(JSC::asObject(thisObject));

   const QMetaObject *meta = qtSignal->metaObject();

   if (! meta) {
      return JSC::throwError(exec, JSC::TypeError, "Function.prototype.discconnect() Can not disconnect from deleted QObject");
   }

   QMetaMethod sig = meta->method(qtSignal->initialIndex());

   if (sig.methodType() != QMetaMethod::Signal) {
      QString message = QString::fromLatin1("Function.prototype.disconnect() %0::%1 is not a signal")
         .formatArg(qtSignal->metaObject()->className())
         .formatArg(sig.methodSignature());

      return JSC::throwError(exec, JSC::TypeError, message);
   }

   QScriptEnginePrivate *engine = scriptEngineFromExec(exec);

   JSC::JSValue receiver;
   JSC::JSValue slot;
   JSC::JSValue arg0 = args.at(0);

   if (args.size() < 2) {
      slot = arg0;
   } else {
      receiver = arg0;
      JSC::JSValue arg1 = args.at(1);

      if (isFunction(arg1)) {
         slot = arg1;
      } else {
         QScript::SaveFrameHelper saveFrame(engine, exec);
         JSC::UString propertyName = QScriptEnginePrivate::toString(exec, arg1);
         slot = QScriptEnginePrivate::property(exec, arg0, propertyName, QScriptValue::ResolvePrototype);
      }
   }

   if (! isFunction(slot)) {
      return JSC::throwError(exec, JSC::TypeError, "Function.prototype.disconnect() Target is not a function");
   }

   bool ok = engine->scriptDisconnect(thisObject, receiver, slot);
   if (!ok) {
      QString message = QString::fromLatin1("Function.prototype.disconnect() Failed to disconnect from %0::%1")
         .formatArg(qtSignal->metaObject()->className())
         .formatArg(sig.methodSignature());

      return JSC::throwError(exec, JSC::GeneralError, message);
   }

   return JSC::jsUndefined();
}

JSC::JSValue JSC_HOST_CALL functionConnect(JSC::ExecState *exec, JSC::JSObject * /*callee*/, JSC::JSValue thisObject,
   const JSC::ArgList &args)
{
   if (args.size() == 0) {
      return JSC::throwError(exec, JSC::GeneralError, "Function.prototype.connect() No arguments passed");
   }

   if (!JSC::asObject(thisObject)->inherits(&QScript::QtFunction::info)) {
      return JSC::throwError(exec, JSC::TypeError, "Function.prototype.connect() Object is not a signal");
   }

   QScript::QtFunction *qtSignal = static_cast<QScript::QtFunction *>(JSC::asObject(thisObject));

   const QMetaObject *meta = qtSignal->metaObject();
   if (!meta) {
      return JSC::throwError(exec, JSC::TypeError, "Function.prototype.connect() Can not connect to deleted QObject");
   }

   QMetaMethod sig = meta->method(qtSignal->initialIndex());
   if (sig.methodType() != QMetaMethod::Signal) {
      QString message = QString("Function.prototype.connect() %0::%1 is not a signal")
         .formatArg(qtSignal->metaObject()->className())
         .formatArg(sig.methodSignature());

      return JSC::throwError(exec, JSC::TypeError, message);
   }

   {
      QList<int> overloads = qtSignal->overloadedIndexes();

      if (!overloads.isEmpty()) {
         overloads.append(qtSignal->initialIndex());

         QString signature = sig.methodSignature();

         QString message = QString("Function.prototype.connect() Ambiguous connect to %0::%1(); candidates are\n")
            .formatArg(qtSignal->metaObject()->className())
            .formatArg(signature.left(signature.indexOf('(')));

         for (int i = 0; i < overloads.size(); ++i) {
            QMetaMethod mtd = meta->method(overloads.at(i));
            message.append(QString("    %0\n").formatArg(mtd.methodSignature()));
         }

         message.append(QString("Use e.g. object['%0'].connect() to connect to a particular overload")
            .formatArg(signature));

         return JSC::throwError(exec, JSC::GeneralError, message);
      }
   }

   QScriptEnginePrivate *engine = scriptEngineFromExec(exec);

   JSC::JSValue receiver;
   JSC::JSValue slot;
   JSC::JSValue arg0 = args.at(0);

   if (args.size() < 2) {
      slot = arg0;
   } else {
      receiver = arg0;
      JSC::JSValue arg1 = args.at(1);
      if (isFunction(arg1)) {
         slot = arg1;
      } else {
         QScript::SaveFrameHelper saveFrame(engine, exec);
         JSC::UString propertyName = QScriptEnginePrivate::toString(exec, arg1);
         slot = QScriptEnginePrivate::property(exec, arg0, propertyName, QScriptValue::ResolvePrototype);
      }
   }

   if (!isFunction(slot)) {
      return JSC::throwError(exec, JSC::TypeError, "Function.prototype.connect() Target is not a function");
   }

   bool ok = engine->scriptConnect(thisObject, receiver, slot, Qt::AutoConnection);
   if (!ok) {
      QString message = QString("Function.prototype.connect() Failed to connect to %0::%1")
         .formatArg(qtSignal->metaObject()->className())
         .formatArg(sig.methodSignature());

      return JSC::throwError(exec, JSC::GeneralError, message);
   }

   return JSC::jsUndefined();
}

static JSC::JSValue JSC_HOST_CALL functionPrint(JSC::ExecState *, JSC::JSObject *, JSC::JSValue, const JSC::ArgList &);
static JSC::JSValue JSC_HOST_CALL functionGC(JSC::ExecState *, JSC::JSObject *, JSC::JSValue, const JSC::ArgList &);
static JSC::JSValue JSC_HOST_CALL functionVersion(JSC::ExecState *, JSC::JSObject *, JSC::JSValue,
   const JSC::ArgList &);

JSC::JSValue JSC_HOST_CALL functionPrint(JSC::ExecState *exec, JSC::JSObject *, JSC::JSValue, const JSC::ArgList &args)
{
   QString result;
   for (unsigned i = 0; i < args.size(); ++i) {
      if (i != 0) {
         result.append(QLatin1Char(' '));
      }
      QString s(args.at(i).toString(exec));
      if (exec->hadException()) {
         break;
      }
      result.append(s);
   }
   if (exec->hadException()) {
      return exec->exception();
   }
   qDebug("%s", csPrintable(result));
   return JSC::jsUndefined();
}

JSC::JSValue JSC_HOST_CALL functionGC(JSC::ExecState *exec, JSC::JSObject *, JSC::JSValue, const JSC::ArgList &)
{
   QScriptEnginePrivate *engine = scriptEngineFromExec(exec);
   engine->collectGarbage();
   return JSC::jsUndefined();
}

JSC::JSValue JSC_HOST_CALL functionVersion(JSC::ExecState *exec, JSC::JSObject *, JSC::JSValue, const JSC::ArgList &)
{
   return JSC::JSValue(exec, 1);
}

static JSC::JSValue JSC_HOST_CALL functionQsTranslate(JSC::ExecState *, JSC::JSObject *, JSC::JSValue,
   const JSC::ArgList &);
static JSC::JSValue JSC_HOST_CALL functionQsTranslateNoOp(JSC::ExecState *, JSC::JSObject *, JSC::JSValue,
   const JSC::ArgList &);
static JSC::JSValue JSC_HOST_CALL functionQsTr(JSC::ExecState *, JSC::JSObject *, JSC::JSValue, const JSC::ArgList &);
static JSC::JSValue JSC_HOST_CALL functionQsTrNoOp(JSC::ExecState *, JSC::JSObject *, JSC::JSValue,
   const JSC::ArgList &);
static JSC::JSValue JSC_HOST_CALL functionQsTrId(JSC::ExecState *, JSC::JSObject *, JSC::JSValue, const JSC::ArgList &);
static JSC::JSValue JSC_HOST_CALL functionQsTrIdNoOp(JSC::ExecState *, JSC::JSObject *, JSC::JSValue,
   const JSC::ArgList &);

JSC::JSValue JSC_HOST_CALL functionQsTranslate(JSC::ExecState *exec, JSC::JSObject *, JSC::JSValue,
   const JSC::ArgList &args)
{
   if (args.size() < 2) {
      return JSC::throwError(exec, JSC::GeneralError, "qsTranslate() requires at least two arguments");
   }

   if (!args.at(0).isString()) {
      return JSC::throwError(exec, JSC::GeneralError, "qsTranslate(): first argument (context) must be a string");
   }

   if (!args.at(1).isString()) {
      return JSC::throwError(exec, JSC::GeneralError, "qsTranslate(): second argument (text) must be a string");
   }

   if ((args.size() > 2) && !args.at(2).isString()) {
      return JSC::throwError(exec, JSC::GeneralError, "qsTranslate(): third argument (comment) must be a string");
   }

   int n = -1;

   if ((args.size() > 3)) {
      if (args.at(3).isString()) {
         qWarning("qsTranslate(): Specifying the encoding as fourth argument is deprecated");

         if (args.size() > 4) {
            if (args.at(4).isNumber()) {
               n = args.at(4).toInt32(exec);
            } else {
               return JSC::throwError(exec, JSC::GeneralError, "qsTranslate(): fifth argument (n) must be a number");
            }
         }

      } else if (args.at(3).isNumber()) {
         n = args.at(3).toInt32(exec);

      } else {
         return JSC::throwError(exec, JSC::GeneralError, "qsTranslate(): fourth argument (n) must be a number");
      }
   }

   JSC::UString context = args.at(0).toString(exec);
   JSC::UString text = args.at(1).toString(exec);
   JSC::UString comment;

   if (args.size() > 2) {
      comment = args.at(2).toString(exec);
   }

   JSC::UString result;

   // pass the encoding
   result = QCoreApplication::translate(context.UTF8String().c_str(), text.UTF8String().c_str(),
            comment.UTF8String().c_str(), n);

   return JSC::jsString(exec, result);
}

JSC::JSValue JSC_HOST_CALL functionQsTranslateNoOp(JSC::ExecState *, JSC::JSObject *, JSC::JSValue,
   const JSC::ArgList &args)
{
   if (args.size() < 2) {
      return JSC::jsUndefined();
   }
   return args.at(1);
}

JSC::JSValue JSC_HOST_CALL functionQsTr(JSC::ExecState *exec, JSC::JSObject *, JSC::JSValue, const JSC::ArgList &args)
{
   if (args.size() < 1) {
      return JSC::throwError(exec, JSC::GeneralError, "qsTr() requires at least one argument");
   }
   if (!args.at(0).isString()) {
      return JSC::throwError(exec, JSC::GeneralError, "qsTr(): first argument (text) must be a string");
   }
   if ((args.size() > 1) && !args.at(1).isString()) {
      return JSC::throwError(exec, JSC::GeneralError, "qsTr(): second argument (comment) must be a string");
   }
   if ((args.size() > 2) && !args.at(2).isNumber()) {
      return JSC::throwError(exec, JSC::GeneralError, "qsTr(): third argument (n) must be a number");
   }

   QScriptEnginePrivate *engine = scriptEngineFromExec(exec);
   JSC::UString context;

   // The first non-empty source URL in the call stack determines the translation context.
   {
      JSC::ExecState *frame = exec->callerFrame()->removeHostCallFrameFlag();
      while (frame) {
         if (frame->codeBlock() && QScriptEnginePrivate::hasValidCodeBlockRegister(frame)
            && frame->codeBlock()->source()
            && !frame->codeBlock()->source()->url().isEmpty()) {
            context = engine->translationContextFromUrl(frame->codeBlock()->source()->url());
            break;
         }
         frame = frame->callerFrame()->removeHostCallFrameFlag();
      }
   }

   JSC::UString text = args.at(0).toString(exec);

   JSC::UString comment;
   if (args.size() > 1) {
      comment = args.at(1).toString(exec);
   }

   int n = -1;
   if (args.size() > 2) {
      n = args.at(2).toInt32(exec);
   }

   JSC::UString result;

   result = QCoreApplication::translate(context.UTF8String().c_str(), text.UTF8String().c_str(),
            comment.UTF8String().c_str(), n);

   return JSC::jsString(exec, result);
}

JSC::JSValue JSC_HOST_CALL functionQsTrNoOp(JSC::ExecState *, JSC::JSObject *, JSC::JSValue, const JSC::ArgList &args)
{
   if (args.size() < 1) {
      return JSC::jsUndefined();
   }
   return args.at(0);
}

JSC::JSValue JSC_HOST_CALL functionQsTrId(JSC::ExecState *exec, JSC::JSObject *, JSC::JSValue, const JSC::ArgList &args)
{
   if (args.size() < 1) {
      return JSC::throwError(exec, JSC::GeneralError, "qsTrId() requires at least one argument");
   }
   if (!args.at(0).isString()) {
      return JSC::throwError(exec, JSC::TypeError, "qsTrId(): first argument (id) must be a string");
   }
   if ((args.size() > 1) && !args.at(1).isNumber()) {
      return JSC::throwError(exec, JSC::TypeError, "qsTrId(): second argument (n) must be a number");
   }
   JSC::UString id = args.at(0).toString(exec);
   int n = -1;
   if (args.size() > 1) {
      n = args.at(1).toInt32(exec);
   }
   return JSC::jsString(exec, qtTrId(id.UTF8String().c_str(), n));
}

JSC::JSValue JSC_HOST_CALL functionQsTrIdNoOp(JSC::ExecState *, JSC::JSObject *, JSC::JSValue, const JSC::ArgList &args)
{
   if (args.size() < 1) {
      return JSC::jsUndefined();
   }
   return args.at(0);
}

static JSC::JSValue JSC_HOST_CALL stringProtoFuncArg(JSC::ExecState *, JSC::JSObject *, JSC::JSValue,
   const JSC::ArgList &);

JSC::JSValue JSC_HOST_CALL stringProtoFuncArg(JSC::ExecState *exec, JSC::JSObject *, JSC::JSValue thisObject,
   const JSC::ArgList &args)
{
   QString value(thisObject.toString(exec));
   JSC::JSValue arg = (args.size() != 0) ? args.at(0) : JSC::jsUndefined();
   QString result;

   if (arg.isString()) {
      result = value.formatArg(arg.toString(exec));

   } else if (arg.isNumber()) {
      result = value.formatArg(arg.toNumber(exec));
   }

   return JSC::jsString(exec, result);
}

static QScriptValue __setupPackage__(QScriptContext *ctx, QScriptEngine *eng)
{
   QString path = ctx->argument(0).toString();
   QStringList components = path.split('.');
   QScriptValue o = eng->globalObject();

   for (int i = 0; i < components.count(); ++i) {
      QString name = components.at(i);
      QScriptValue oo = o.property(name);

      if (!oo.isValid()) {
         oo = eng->newObject();
         o.setProperty(name, oo);
      }
      o = oo;
   }
   return o;
}

} // namespace QScript

QScriptEnginePrivate::QScriptEnginePrivate()
   : originalGlobalObjectProxy(nullptr), currentFrame(nullptr), qobjectPrototype(nullptr),
     qmetaobjectPrototype(nullptr), variantPrototype(nullptr), activeAgent(nullptr), agentLineNumber(-1),
     registeredScriptValues(nullptr), freeScriptValues(nullptr), freeScriptValuesCount(0),
     registeredScriptStrings(nullptr), processEventsInterval(-1), inEval(false), uncaughtExceptionLineNumber(-1)
{
   if (! QCoreApplication::instance()) {
      qFatal("QScriptEngine: Must construct a Q(Core)Application before a QScriptEngine");
      return;
   }

   JSC::initializeThreading();
   JSC::IdentifierTable *oldTable = JSC::currentIdentifierTable();
   globalData = JSC::JSGlobalData::create().releaseRef();
   globalData->clientData = new QScript::GlobalClientData(this);
   JSC::JSGlobalObject *globalObject = new (globalData)QScript::GlobalObject();

   JSC::ExecState *exec = globalObject->globalExec();

   scriptObjectStructure = QScriptObject::createStructure(globalObject->objectPrototype());
   staticScopeObjectStructure = QScriptStaticScopeObject::createStructure(JSC::jsNull());

   qobjectPrototype = new (exec) QScript::QObjectPrototype(exec,
      QScript::QObjectPrototype::createStructure(globalObject->objectPrototype()),
      globalObject->prototypeFunctionStructure());

   qobjectWrapperObjectStructure = QScriptObject::createStructure(qobjectPrototype);

   qmetaobjectPrototype = new (exec) QScript::QMetaObjectPrototype(exec,
      QScript::QMetaObjectPrototype::createStructure(globalObject->objectPrototype()),
      globalObject->prototypeFunctionStructure());

   qmetaobjectWrapperObjectStructure = QScript::QMetaObjectWrapperObject::createStructure(qmetaobjectPrototype);

   variantPrototype = new (exec) QScript::QVariantPrototype(exec,
      QScript::QVariantPrototype::createStructure(globalObject->objectPrototype()),
      globalObject->prototypeFunctionStructure());
   variantWrapperObjectStructure = QScriptObject::createStructure(variantPrototype);

   globalObject->putDirectFunction(exec, new (exec)JSC::NativeFunctionWrapper(exec,
         globalObject->prototypeFunctionStructure(), 1, JSC::Identifier(exec, "print"), QScript::functionPrint));

   globalObject->putDirectFunction(exec, new (exec)JSC::NativeFunctionWrapper(exec,
         globalObject->prototypeFunctionStructure(), 0, JSC::Identifier(exec, "gc"), QScript::functionGC));

   globalObject->putDirectFunction(exec, new (exec)JSC::NativeFunctionWrapper(exec,
         globalObject->prototypeFunctionStructure(), 0, JSC::Identifier(exec, "version"), QScript::functionVersion));

   // ### rather than extending Function.prototype, consider creating a QtSignal.prototype
   globalObject->functionPrototype()->putDirectFunction(exec, new (exec)JSC::NativeFunctionWrapper(exec,
         globalObject->prototypeFunctionStructure(), 1, JSC::Identifier(exec, "disconnect"), QScript::functionDisconnect));

   globalObject->functionPrototype()->putDirectFunction(exec, new (exec)JSC::NativeFunctionWrapper(exec,
         globalObject->prototypeFunctionStructure(), 1, JSC::Identifier(exec, "connect"), QScript::functionConnect));

   JSC::TimeoutChecker *originalChecker = globalData->timeoutChecker;
   globalData->timeoutChecker = new QScript::TimeoutCheckerProxy(*originalChecker);
   delete originalChecker;

   currentFrame = exec;

   cachedTranslationUrl = JSC::UString();
   cachedTranslationContext = JSC::UString();
   JSC::setCurrentIdentifierTable(oldTable);
}

QScriptEnginePrivate::~QScriptEnginePrivate()
{
   QScript::APIShim shim(this);

   //disconnect all loadedScripts and generate all jsc::debugger::scriptUnload events
   QHash<intptr_t, QScript::UStringSourceProviderWithFeedback *>::const_iterator it;

   for (it = loadedScripts.constBegin(); it != loadedScripts.constEnd(); ++it) {
      it.value()->disconnectFromEngine();
   }

   while (!ownedAgents.isEmpty()) {
      delete ownedAgents.takeFirst();
   }

   detachAllRegisteredScriptPrograms();
   detachAllRegisteredScriptValues();
   detachAllRegisteredScriptStrings();
   qDeleteAll(m_qobjectData);
   qDeleteAll(m_typeInfos);
   globalData->heap.destroy();
   globalData->deref();

   while (freeScriptValues) {
      QScriptValuePrivate *p = freeScriptValues;
      freeScriptValues = p->next;
      free(p);
   }
}

QVariant QScriptEnginePrivate::jscValueToVariant(JSC::ExecState *exec, JSC::JSValue value, uint targetType)
{
   if (targetType == QVariant::Variant) {
      return toVariant(exec, value);
   }

   QVariant newValue = convertValue(exec, value, targetType);

   if (newValue.isValid()) {
      return newValue;
   }

   if (isVariant(value)) {
      newValue = variantValue(value);

      if (newValue.canConvert(QVariant::Type(targetType))) {
         newValue.convert(QVariant::Type(targetType));
         return newValue;
      }
   }

   return QVariant();
}

JSC::JSValue QScriptEnginePrivate::arrayFromStringList(JSC::ExecState *exec, const QStringList &lst)
{
   JSC::JSValue arr =  newArray(exec, lst.size());
   for (int i = 0; i < lst.size(); ++i) {
      setProperty(exec, arr, i, JSC::jsString(exec, lst.at(i)));
   }

   return arr;
}

QStringList QScriptEnginePrivate::stringListFromArray(JSC::ExecState *exec, JSC::JSValue arr)
{
   QStringList lst;
   uint len = toUInt32(exec, property(exec, arr, exec->propertyNames().length));
   for (uint i = 0; i < len; ++i) {
      lst.append(toString(exec, property(exec, arr, i)));
   }
   return lst;
}

JSC::JSValue QScriptEnginePrivate::arrayFromVariantList(JSC::ExecState *exec, const QVariantList &lst)
{
   JSC::JSValue arr = newArray(exec, lst.size());
   for (int i = 0; i < lst.size(); ++i) {
      setProperty(exec, arr, i, jscValueFromVariant(exec, lst.at(i)));
   }
   return arr;
}

QVariantList QScriptEnginePrivate::variantListFromArray(JSC::ExecState *exec, JSC::JSArray *arr)
{
   QScriptEnginePrivate *eng = QScript::scriptEngineFromExec(exec);
   if (eng->visitedConversionObjects.contains(arr)) {
      return QVariantList();   // Avoid recursion.
   }

   eng->visitedConversionObjects.insert(arr);
   QVariantList lst;
   uint len = toUInt32(exec, property(exec, arr, exec->propertyNames().length));

   for (uint i = 0; i < len; ++i) {
      lst.append(toVariant(exec, property(exec, arr, i)));
   }
   eng->visitedConversionObjects.remove(arr);
   return lst;
}

JSC::JSValue QScriptEnginePrivate::objectFromVariantMap(JSC::ExecState *exec, const QVariantMap &vmap)
{
   JSC::JSValue obj = JSC::constructEmptyObject(exec);
   QVariantMap::const_iterator it;
   for (it = vmap.constBegin(); it != vmap.constEnd(); ++it) {
      setProperty(exec, obj, it.key(), jscValueFromVariant(exec, it.value()));
   }
   return obj;
}

QVariantMap QScriptEnginePrivate::variantMapFromObject(JSC::ExecState *exec, JSC::JSObject *obj)
{
   QScriptEnginePrivate *eng = QScript::scriptEngineFromExec(exec);
   if (eng->visitedConversionObjects.contains(obj)) {
      return QVariantMap();   // Avoid recursion.
   }
   eng->visitedConversionObjects.insert(obj);
   JSC::PropertyNameArray propertyNames(exec);
   obj->getOwnPropertyNames(exec, propertyNames, JSC::IncludeDontEnumProperties);
   QVariantMap vmap;
   JSC::PropertyNameArray::const_iterator it = propertyNames.begin();
   for ( ; it != propertyNames.end(); ++it) {
      vmap.insert(it->ustring(), toVariant(exec, property(exec, obj, *it)));
   }
   eng->visitedConversionObjects.remove(obj);
   return vmap;
}

JSC::JSValue QScriptEnginePrivate::defaultPrototype(int metaTypeId) const
{
   QScriptTypeInfo *info = m_typeInfos.value(metaTypeId);
   if (!info) {
      return JSC::JSValue();
   }
   return info->prototype;
}

void QScriptEnginePrivate::setDefaultPrototype(int metaTypeId, JSC::JSValue prototype)
{
   QScriptTypeInfo *info = m_typeInfos.value(metaTypeId);
   if (!info) {
      info = new QScriptTypeInfo();
      m_typeInfos.insert(metaTypeId, info);
   }
   info->prototype = prototype;
}

JSC::JSGlobalObject *QScriptEnginePrivate::originalGlobalObject() const
{
   return globalData->head;
}

JSC::JSObject *QScriptEnginePrivate::customGlobalObject() const
{
   QScript::GlobalObject *glob = static_cast<QScript::GlobalObject *>(originalGlobalObject());
   return glob->customGlobalObject;
}

JSC::JSObject *QScriptEnginePrivate::getOriginalGlobalObjectProxy()
{
   if (!originalGlobalObjectProxy) {
      JSC::ExecState *exec = currentFrame;
      originalGlobalObjectProxy = new (exec)QScript::OriginalGlobalObjectProxy(scriptObjectStructure, originalGlobalObject());
   }

   return originalGlobalObjectProxy;
}

JSC::JSObject *QScriptEnginePrivate::globalObject() const
{
   QScript::GlobalObject *glob = static_cast<QScript::GlobalObject *>(originalGlobalObject());
   if (glob->customGlobalObject) {
      return glob->customGlobalObject;
   }
   return glob;
}

void QScriptEnginePrivate::setGlobalObject(JSC::JSObject *object)
{
   if (object == globalObject()) {
      return;
   }

   QScript::GlobalObject *glob = static_cast<QScript::GlobalObject *>(originalGlobalObject());

   if (object == originalGlobalObjectProxy) {
      glob->customGlobalObject = nullptr;
      // Sync the internal prototype, since JSObject::prototype() is not virtual.
      glob->setPrototype(originalGlobalObjectProxy->prototype());

   } else {
      Q_ASSERT(object != originalGlobalObject());
      glob->customGlobalObject = object;
      // Sync the internal prototype, since JSObject::prototype() is not virtual.
      glob->setPrototype(object->prototype());
   }
}

// internal (cs)
JSC::JSValue QScriptEnginePrivate::toUsableValue(JSC::JSValue value)
{
   if (! value || ! value.isObject() || ! JSC::asObject(value)->isGlobalObject()) {
      return value;
   }

   Q_ASSERT(JSC::asObject(value) == originalGlobalObject());
   if (customGlobalObject()) {
      return customGlobalObject();
   }

   if (!originalGlobalObjectProxy) {
      originalGlobalObjectProxy = new (currentFrame)QScript::OriginalGlobalObjectProxy(scriptObjectStructure,
         originalGlobalObject());
   }

   return originalGlobalObjectProxy;
}

// internal (cs)
JSC::JSValue QScriptEnginePrivate::thisForContext(JSC::ExecState *frame)
{
   if (frame->codeBlock() != nullptr) {
      return frame->thisValue();

   } else if (frame == frame->lexicalGlobalObject()->globalExec()) {
      return frame->globalThisValue();

   } else {
      JSC::Register *thisRegister = thisRegisterForFrame(frame);
      return thisRegister->jsValue();
   }
}

JSC::Register *QScriptEnginePrivate::thisRegisterForFrame(JSC::ExecState *frame)
{
   Q_ASSERT(frame->codeBlock() == nullptr); // only for native calls
   return frame->registers() - JSC::RegisterFile::CallFrameHeaderSize - frame->argumentCount();
}

// internal (cs)
uint QScriptEnginePrivate::contextFlags(JSC::ExecState *exec)
{
   if (exec->codeBlock()) {
      return 0;   //js function doesn't have flags
   }

   return exec->returnValueRegister();
}

void QScriptEnginePrivate::setContextFlags(JSC::ExecState *exec, uint flags)
{
   Q_ASSERT(!exec->codeBlock());
   exec->registers()[JSC::RegisterFile::ReturnValueRegister] = JSC::Register::withInt(flags);
}

void QScriptEnginePrivate::mark(JSC::MarkStack &markStack)
{
   Q_Q(QScriptEngine);

   if (originalGlobalObject()) {
      markStack.append(originalGlobalObject());
      markStack.append(globalObject());
      if (originalGlobalObjectProxy) {
         markStack.append(originalGlobalObjectProxy);
      }
   }

   if (qobjectPrototype) {
      markStack.append(qobjectPrototype);
   }
   if (qmetaobjectPrototype) {
      markStack.append(qmetaobjectPrototype);
   }
   if (variantPrototype) {
      markStack.append(variantPrototype);
   }

   {
      QScriptValuePrivate *it;
      for (it = registeredScriptValues; it != nullptr; it = it->next) {
         if (it->isJSC()) {
            markStack.append(it->jscValue);
         }
      }
   }

   {
      QHash<int, QScriptTypeInfo *>::const_iterator it;
      for (it = m_typeInfos.constBegin(); it != m_typeInfos.constEnd(); ++it) {
         if ((*it)->prototype) {
            markStack.append((*it)->prototype);
         }
      }
   }

   if (q) {
      QScriptContext *context = q->currentContext();

      while (context) {
         JSC::ScopeChainNode *node = frameForContext(context)->scopeChain();
         JSC::ScopeChainIterator it(node);

         for (it = node->begin(); it != node->end(); ++it) {
            JSC::JSObject *object = *it;
            if (object) {
               markStack.append(object);
            }
         }

         context = context->parentContext();
      }
   }
   markQObjectData(markStack);
}

bool QScriptEnginePrivate::isCollecting() const
{
   return globalData->heap.isBusy();
}

void QScriptEnginePrivate::collectGarbage()
{
   QScript::APIShim shim(this);
   globalData->heap.collectAllGarbage();
}

void QScriptEnginePrivate::reportAdditionalMemoryCost(int size)
{
   if (size > 0) {
      globalData->heap.reportExtraMemoryCost(size);
   }
}

QScript::TimeoutCheckerProxy *QScriptEnginePrivate::timeoutChecker() const
{
   return static_cast<QScript::TimeoutCheckerProxy *>(globalData->timeoutChecker);
}

void QScriptEnginePrivate::agentDeleted(QScriptEngineAgent *agent)
{
   ownedAgents.removeOne(agent);
   if (activeAgent == agent) {
      QScriptEngineAgentPrivate::get(agent)->detach();
      activeAgent = nullptr;
   }
}

JSC::JSValue QScriptEnginePrivate::evaluateHelper(JSC::ExecState *exec, intptr_t sourceId,
   JSC::EvalExecutable *executable, bool &compile)
{
   Q_Q(QScriptEngine);

   bool temp = inEval;
   inEval = true;

   q->currentContext()->activationObject(); //force the creation of a context for native function;

   JSC::Debugger *debugger = originalGlobalObject()->debugger();
   if (debugger) {
      debugger->evaluateStart(sourceId);
   }

   q->clearExceptions();
   JSC::DynamicGlobalObjectScope dynamicGlobalObjectScope(exec, exec->scopeChain()->globalObject);

   if (compile && !executable->isCompiled()) {
      JSC::JSObject *error = executable->compile(exec, exec->scopeChain());
      if (error) {
         compile = false;
         exec->setException(error);

         if (debugger) {
            debugger->exceptionThrow(JSC::DebuggerCallFrame(exec, error), sourceId, false);
            debugger->evaluateStop(error, sourceId);
         }

         inEval = temp;
         return error;
      }
   }

   JSC::JSValue thisValue = thisForContext(exec);
   JSC::JSObject *thisObject = (!thisValue || thisValue.isUndefinedOrNull())
      ? exec->dynamicGlobalObject() : thisValue.toObject(exec);
   JSC::JSValue exceptionValue;
   timeoutChecker()->setShouldAbort(false);

   if (processEventsInterval > 0) {
      timeoutChecker()->reset();
   }

   JSC::JSValue result = exec->interpreter()->execute(executable, exec, thisObject, exec->scopeChain(), &exceptionValue);

   if (timeoutChecker()->shouldAbort()) {
      if (abortResult.isError()) {
         exec->setException(scriptValueToJSCValue(abortResult));
      }

      if (debugger) {
         debugger->evaluateStop(scriptValueToJSCValue(abortResult), sourceId);
      }

      inEval = temp;
      return scriptValueToJSCValue(abortResult);
   }

   if (exceptionValue) {
      exec->setException(exceptionValue);

      if (debugger) {
         debugger->evaluateStop(exceptionValue, sourceId);
      }

      inEval = temp;
      return exceptionValue;
   }

   if (debugger) {
      debugger->evaluateStop(result, sourceId);
   }

   Q_ASSERT(!exec->hadException());

   inEval = temp;

   return result;
}

bool QScriptEnginePrivate::isLikelyStackOverflowError(JSC::ExecState *exec, JSC::JSValue value)
{
   if (!isError(value)) {
      return false;
   }

   JSC::JSValue name = property(exec, value, exec->propertyNames().name);
   if (!name || !name.isString() || name.toString(exec) != "RangeError") {
      return false;
   }

   JSC::JSValue message = property(exec, value, exec->propertyNames().message);
   if (!message || !message.isString() || message.toString(exec) != "Maximum call stack size exceeded.") {
      return false;
   }

   return true;
}

void QScriptEnginePrivate::uncaughtException(JSC::ExecState *exec, unsigned bytecodeOffset,
   JSC::JSValue value)
{
   // do not capture exception information if we already have.
   if (uncaughtExceptionLineNumber != -1) {
      return;
   }

   QScript::SaveFrameHelper saveFrame(this, exec);

   uncaughtExceptionLineNumber = exec->codeBlock()->lineNumberForBytecodeOffset(exec, bytecodeOffset);

   if (isLikelyStackOverflowError(exec, value)) {
      // Don't save the backtrace, it's likely to take forever to create.
      uncaughtExceptionBacktrace.clear();
   } else {
      uncaughtExceptionBacktrace = contextForFrame(exec)->backtrace();
   }
}

void QScriptEnginePrivate::markQObjectData(JSC::MarkStack &markStack)
{
   QHash<QObject *, QScript::QObjectData *>::const_iterator it;
   // 1. Clear connection mark bits for all objects
   for (it = m_qobjectData.constBegin(); it != m_qobjectData.constEnd(); ++it) {
      QScript::QObjectData *qdata = it.value();
      qdata->clearConnectionMarkBits();
   }

   // 2. Iterate until no more connections are marked
   int markedCount;
   do {
      // Drain the stack to ensure mark bits are set; this is used to determine
      // whether a connection's sender object is weakly referenced
      markStack.drain();

      markedCount = 0;

      for (it = m_qobjectData.constBegin(); it != m_qobjectData.constEnd(); ++it) {
         QScript::QObjectData *qdata = it.value();
         markedCount += qdata->markConnections(markStack);
      }
   } while (markedCount > 0);

   markStack.drain(); // One last time before marking wrappers

   // 3. Mark all wrappers
   for (it = m_qobjectData.constBegin(); it != m_qobjectData.constEnd(); ++it) {
      QScript::QObjectData *qdata = it.value();
      qdata->markWrappers(markStack);
   }
}

JSC::JSValue QScriptEnginePrivate::newQObject(QObject *object, QScriptEngine::ValueOwnership ownership,
   const QScriptEngine::QObjectWrapOptions &options)
{
   if (!object) {
      return JSC::jsNull();
   }

   JSC::ExecState *exec = currentFrame;
   QScript::QObjectData *data = qobjectData(object);
   bool preferExisting = (options & QScriptEngine::PreferExistingWrapperObject) != 0;

   QScriptEngine::QObjectWrapOptions opt = options & ~QScriptEngine::PreferExistingWrapperObject;
   QScriptObject *result = nullptr;

   if (preferExisting) {
      result = data->findWrapper(ownership, opt);

      if (result) {
         return result;
      }
   }
   result = new (exec) QScriptObject(qobjectWrapperObjectStructure);
   if (preferExisting) {
      data->registerWrapper(result, ownership, opt);
   }

   result->setDelegate(new QScript::QObjectDelegate(object, ownership, options));

   // if (setDefaultPrototype)
   {
      const QMetaObject *meta = object->metaObject();

      while (meta) {
         QString typeString = meta->className();
         typeString.append('*');

         uint typeId = QVariant::nameToType(typeString);

         if (typeId != QVariant::Invalid) {
            JSC::JSValue proto = defaultPrototype(typeId);

            if (proto) {
               result->setPrototype(proto);
               break;
            }
         }
         meta = meta->superClass();
      }
   }

   return result;
}

JSC::JSValue QScriptEnginePrivate::newQMetaObject(
   const QMetaObject *metaObject, JSC::JSValue ctor)
{
   if (! metaObject) {
      return JSC::jsNull();
   }

   JSC::ExecState *exec = currentFrame;
   QScript::QMetaObjectWrapperObject *result = new (exec) QScript::QMetaObjectWrapperObject(exec, metaObject, ctor,
      qmetaobjectWrapperObjectStructure);

   return result;
}

bool QScriptEnginePrivate::convertToNativeQObject(JSC::ExecState *exec, JSC::JSValue value,
   const QString &targetType, void **result)
{
   if (! targetType.endsWith('*')) {
      return false;
   }

   if (QObject *qobject = toQObject(exec, value)) {

      int start = targetType.startsWith("const ") ? 6 : 0;
      QString className = targetType.mid(start, targetType.size() - start - 1);

      (void) qobject;
      (void) result;

      /*  emerald (script, hold)
         if (void *instance = qobject->qt_metacast(className)) {
            *result = instance;
            return true;
          }
      */
   }

   return false;
}

QScript::QObjectData *QScriptEnginePrivate::qobjectData(QObject *object)
{
   QHash<QObject *, QScript::QObjectData *>::const_iterator it;
   it = m_qobjectData.constFind(object);

   if (it != m_qobjectData.constEnd()) {
      return it.value();
   }

   QScript::QObjectData *data = new QScript::QObjectData(this);
   m_qobjectData.insert(object, data);
   QObject::connect(object, SIGNAL(destroyed(QObject *)), q_func(), SLOT(_q_objectDestroyed(QObject *)));

   return data;
}

void QScriptEnginePrivate::_q_objectDestroyed(QObject *object)
{
   QHash<QObject *, QScript::QObjectData *>::iterator it;
   it = m_qobjectData.find(object);

   Q_ASSERT(it != m_qobjectData.end());

   QScript::QObjectData *data = it.value();
   m_qobjectData.erase(it);

   delete data;
}

void QScriptEnginePrivate::disposeQObject(QObject *object)
{
   // TODO
   /*       if (isCollecting()) {
              // wait until we're done with GC before deleting it
              int index = m_qobjectsToBeDeleted.indexOf(object);
              if (index == -1)
                  m_qobjectsToBeDeleted.append(object);
            } else
   */

   {
      delete object;
   }
}

void QScriptEnginePrivate::emitSignalHandlerException()
{
   Q_Q(QScriptEngine);
   emit q->signalHandlerException(q->uncaughtException());
}

bool QScriptEnginePrivate::scriptConnect(QObject *sender, const QString &signal, JSC::JSValue receiver,
   JSC::JSValue function, Qt::ConnectionType type)
{
   Q_ASSERT(sender);
   Q_ASSERT(! signal.isEmpty());

   const QMetaObject *meta = sender->metaObject();
   int index = meta->indexOfSignal(QMetaObject::normalizedSignature(signal + 1));

   if (index == -1) {
      return false;
   }

   return scriptConnect(sender, index, receiver, function, /*wrapper=*/JSC::JSValue(), type);
}

bool QScriptEnginePrivate::scriptDisconnect(QObject *sender, const QString &signal, JSC::JSValue receiver,
   JSC::JSValue function)
{
   Q_ASSERT(sender);
   Q_ASSERT(! signal.isEmpty());

   const QMetaObject *meta = sender->metaObject();
   int index = meta->indexOfSignal(QMetaObject::normalizedSignature(signal + 1));

   if (index == -1) {
      return false;
   }

   return scriptDisconnect(sender, index, receiver, function);
}

bool QScriptEnginePrivate::scriptConnect(QObject *sender, int signalIndex, JSC::JSValue receiver,
   JSC::JSValue function, JSC::JSValue senderWrapper, Qt::ConnectionType type)
{
   QScript::QObjectData *data = qobjectData(sender);
   return data->addSignalHandler(sender, signalIndex, receiver, function, senderWrapper, type);
}

bool QScriptEnginePrivate::scriptDisconnect(QObject *sender, int signalIndex,
   JSC::JSValue receiver, JSC::JSValue function)
{
   QScript::QObjectData *data = qobjectData(sender);

   if (!data) {
      return false;
   }
   return data->removeSignalHandler(sender, signalIndex, receiver, function);
}

bool QScriptEnginePrivate::scriptConnect(JSC::JSValue signal, JSC::JSValue receiver,
   JSC::JSValue function, Qt::ConnectionType type)
{
   QScript::QtFunction *fun = static_cast<QScript::QtFunction *>(JSC::asObject(signal));
   int index = fun->mostGeneralMethod();
   return scriptConnect(fun->qobject(), index, receiver, function, fun->wrapperObject(), type);
}

bool QScriptEnginePrivate::scriptDisconnect(JSC::JSValue signal, JSC::JSValue receiver,
   JSC::JSValue function)
{
   QScript::QtFunction *fun = static_cast<QScript::QtFunction *>(JSC::asObject(signal));
   int index = fun->mostGeneralMethod();
   return scriptDisconnect(fun->qobject(), index, receiver, function);
}

void QScriptEnginePrivate::detachAllRegisteredScriptPrograms()
{
   QSet<QScriptProgramPrivate *>::const_iterator it;

   for (it = registeredScriptPrograms.constBegin(); it != registeredScriptPrograms.constEnd(); ++it) {
      (*it)->detachFromEngine();
   }

   registeredScriptPrograms.clear();
}

void QScriptEnginePrivate::detachAllRegisteredScriptValues()
{
   QScriptValuePrivate *it;
   QScriptValuePrivate *next;

   for (it = registeredScriptValues; it != nullptr; it = next) {
      it->detachFromEngine();
      next = it->next;
      it->prev = nullptr;
      it->next = nullptr;
   }

   registeredScriptValues = nullptr;
}

void QScriptEnginePrivate::detachAllRegisteredScriptStrings()
{
   QScriptStringPrivate *it;
   QScriptStringPrivate *next;

   for (it = registeredScriptStrings; it != nullptr; it = next) {
      it->detachFromEngine();
      next = it->next;
      it->prev = nullptr;
      it->next = nullptr;
   }

   registeredScriptStrings = nullptr;
}

JSC::JSValue QScriptEnginePrivate::newRegExp(JSC::ExecState *exec, const QRegularExpression &regexp)
{
   JSC::JSValue buf[2];
   JSC::ArgList args(buf, sizeof(buf));

   QString pattern = cs_internal_regexp_toCanonical(regexp.pattern(), regexp.patternOptions());

   JSC::UString jscPattern = pattern;
   QString flags;

   if (regexp.patternOptions() & QPatternOption::CaseInsensitiveOption) {
      flags.append('i');
   }

   JSC::UString jscFlags = flags;
   buf[0] = JSC::jsString(exec, jscPattern);
   buf[1] = JSC::jsString(exec, jscFlags);

   return JSC::constructRegExp(exec, args);
}

JSC::JSValue QScriptEnginePrivate::newRegExp(JSC::ExecState *exec, const QString &pattern, const QString &flags)
{
   JSC::JSValue buf[2];
   JSC::ArgList args(buf, sizeof(buf));
   JSC::UString jscPattern = pattern;
   QString strippedFlags;

   if (flags.contains(QLatin1Char('i'))) {
      strippedFlags += QLatin1Char('i');
   }

   if (flags.contains(QLatin1Char('m'))) {
      strippedFlags += QLatin1Char('m');
   }

   if (flags.contains(QLatin1Char('g'))) {
      strippedFlags += QLatin1Char('g');
   }

   JSC::UString jscFlags = strippedFlags;
   buf[0] = JSC::jsString(exec, jscPattern);
   buf[1] = JSC::jsString(exec, jscFlags);

   return JSC::constructRegExp(exec, args);
}

JSC::JSValue QScriptEnginePrivate::newVariant(const QVariant &value)
{
   QScriptObject *obj = new (currentFrame) QScriptObject(variantWrapperObjectStructure);
   obj->setDelegate(new QScript::QVariantDelegate(value));
   JSC::JSValue proto = defaultPrototype(value.userType());

   if (proto) {
      obj->setPrototype(proto);
   }
   return obj;
}

JSC::JSValue QScriptEnginePrivate::newVariant(JSC::JSValue objectValue,
   const QVariant &value)
{
   if (!isObject(objectValue)) {
      return newVariant(value);
   }
   JSC::JSObject *jscObject = JSC::asObject(objectValue);
   if (!jscObject->inherits(&QScriptObject::info)) {
      qWarning("QScriptEngine::newVariant(): changing class of non-QScriptObject not supported");
      return JSC::JSValue();
   }
   QScriptObject *jscScriptObject = static_cast<QScriptObject *>(jscObject);
   if (!isVariant(objectValue)) {
      jscScriptObject->setDelegate(new QScript::QVariantDelegate(value));
   } else {
      setVariantValue(objectValue, value);
   }
   return objectValue;
}

QRegularExpression QScriptEnginePrivate::toRegExp(JSC::ExecState *exec, JSC::JSValue value)
{
   if (! isRegExp(value)) {
      return QRegularExpression();
   }

   QString pattern = toString(exec, property(exec, value, "source", QScriptValue::ResolvePrototype));
   QPatternOptionFlags flags = QPatternOption::NoPatternOption;

   if (toBool(exec, property(exec, value, "ignoreCase", QScriptValue::ResolvePrototype))) {
      flags = QPatternOption::CaseInsensitiveOption;
   }

   return QRegularExpression(pattern, flags);
}

QVariant QScriptEnginePrivate::toVariant(JSC::ExecState *exec, JSC::JSValue value)
{
   if (!value) {
      return QVariant();

   } else if (isObject(value)) {
      if (isVariant(value)) {
         return variantValue(value);
      }

      else if (isQObject(value)) {
         return QVariant::fromValue(toQObject(exec, value));
      }

      else if (isDate(value)) {
         return QVariant(toDateTime(exec, value));
      }

      else if (isRegExp(value)) {
         return QVariant(toRegExp(exec, value));
      }

      else if (isArray(value)) {
         return variantListFromArray(exec, JSC::asArray(value));

      } else if (QScriptDeclarativeClass *dc = declarativeClass(value)) {
         return dc->toVariant(declarativeObject(value));

      }

      return variantMapFromObject(exec, JSC::asObject(value));

   } else if (value.isInt32()) {
      return QVariant(toInt32(exec, value));
   } else if (value.isDouble()) {
      return QVariant(toNumber(exec, value));
   } else if (value.isString()) {
      return QVariant(toString(exec, value));
   } else if (value.isBoolean()) {
      return QVariant(toBool(exec, value));
   }
   return QVariant();
}

JSC::JSValue QScriptEnginePrivate::propertyHelper(JSC::ExecState *exec, JSC::JSValue value, const JSC::Identifier &id,
   int resolveMode)
{
   JSC::JSValue result;
   if (!(resolveMode & QScriptValue::ResolvePrototype)) {
      // Look in the object's own properties
      JSC::JSObject *object = JSC::asObject(value);
      JSC::PropertySlot slot(object);
      if (object->getOwnPropertySlot(exec, id, slot)) {
         result = slot.getValue(exec, id);
      }
   }
   if (!result && (resolveMode & QScriptValue::ResolveScope)) {
      // ### check if it's a function object and look in the scope chain
      JSC::JSValue scope = property(exec, value, "__qt_scope__", QScriptValue::ResolveLocal);
      if (isObject(scope)) {
         result = property(exec, scope, id, resolveMode);
      }
   }
   return result;
}

JSC::JSValue QScriptEnginePrivate::propertyHelper(JSC::ExecState *exec, JSC::JSValue value, quint32 index,
   int resolveMode)
{
   JSC::JSValue result;

   if (!(resolveMode & QScriptValue::ResolvePrototype)) {
      // Look in the object's own properties
      JSC::JSObject *object = JSC::asObject(value);
      JSC::PropertySlot slot(object);
      if (object->getOwnPropertySlot(exec, index, slot)) {
         result = slot.getValue(exec, index);
      }
   }

   return result;
}

void QScriptEnginePrivate::setProperty(JSC::ExecState *exec, JSC::JSValue objectValue, const JSC::Identifier &id,
   JSC::JSValue value, const QScriptValue::PropertyFlags &flags)
{
   JSC::JSObject *thisObject = JSC::asObject(objectValue);
   JSC::JSValue setter = thisObject->lookupSetter(exec, id);
   JSC::JSValue getter = thisObject->lookupGetter(exec, id);

   if ((flags & QScriptValue::PropertyGetter) || (flags & QScriptValue::PropertySetter)) {
      if (!value) {
         // deleting getter/setter
         if ((flags & QScriptValue::PropertyGetter) && (flags & QScriptValue::PropertySetter)) {
            // deleting both: just delete the property
            thisObject->deleteProperty(exec, id);
         } else if (flags & QScriptValue::PropertyGetter) {
            // preserve setter, if there is one
            thisObject->deleteProperty(exec, id);
            if (setter && setter.isObject()) {
               thisObject->defineSetter(exec, id, JSC::asObject(setter));
            }
         } else { // flags & QScriptValue::PropertySetter
            // preserve getter, if there is one
            thisObject->deleteProperty(exec, id);
            if (getter && getter.isObject()) {
               thisObject->defineGetter(exec, id, JSC::asObject(getter));
            }
         }
      } else {
         if (value.isObject()) { // ### should check if it has callData()
            // defining getter/setter
            if (id == exec->propertyNames().underscoreProto) {
               qWarning("QScriptValue::setProperty() failed: "
                  "cannot set getter or setter of native property `__proto__'");
            } else {
               if (flags & QScriptValue::PropertyGetter) {
                  thisObject->defineGetter(exec, id, JSC::asObject(value));
               }
               if (flags & QScriptValue::PropertySetter) {
                  thisObject->defineSetter(exec, id, JSC::asObject(value));
               }
            }
         } else {
            qWarning("QScriptValue::setProperty(): getter/setter must be a function");
         }
      }

   } else {
      // setting the value
      if (getter && getter.isObject() && !(setter && setter.isObject())) {
         qWarning("QScriptValue::setProperty() failed: "
            "property '%s' has a getter but no setter", csPrintable(QString(id.ustring())));
         return;
      }

      if (!value) {
         // ### check if it's a getter/setter property
         thisObject->deleteProperty(exec, id);

      } else if (flags != QScriptValue::KeepExistingFlags) {
         if (thisObject->hasOwnProperty(exec, id)) {
            thisObject->deleteProperty(exec, id);   // ### can't we just update the attributes?
         }

         thisObject->putWithAttributes(exec, id, value, propertyFlagsToJSCAttributes(flags));

      } else {
         JSC::PutPropertySlot slot;
         thisObject->put(exec, id, value, slot);
      }
   }
}

void QScriptEnginePrivate::setProperty(JSC::ExecState *exec, JSC::JSValue objectValue, quint32 index,
   JSC::JSValue value, const QScriptValue::PropertyFlags &flags)
{
   if (! value) {
      JSC::asObject(objectValue)->deleteProperty(exec, index);
   } else {
      if ((flags & QScriptValue::PropertyGetter) || (flags & QScriptValue::PropertySetter)) {
         // fall back to string-based setProperty(), since there is no
         // JSC::JSObject::defineGetter(unsigned)
         setProperty(exec, objectValue, JSC::Identifier::from(exec, index), value, flags);
      } else {
         if (flags != QScriptValue::KeepExistingFlags) {
            //                if (JSC::asObject(d->jscValue)->hasOwnProperty(exec, arrayIndex))
            //                    JSC::asObject(d->jscValue)->deleteProperty(exec, arrayIndex);
            unsigned attribs = 0;
            if (flags & QScriptValue::ReadOnly) {
               attribs |= JSC::ReadOnly;
            }
            if (flags & QScriptValue::SkipInEnumeration) {
               attribs |= JSC::DontEnum;
            }
            if (flags & QScriptValue::Undeletable) {
               attribs |= JSC::DontDelete;
            }
            attribs |= flags & QScriptValue::UserRange;
            JSC::asObject(objectValue)->putWithAttributes(exec, index, value, attribs);
         } else {
            JSC::asObject(objectValue)->put(exec, index, value);
         }
      }
   }
}

QScriptValue::PropertyFlags QScriptEnginePrivate::propertyFlags(JSC::ExecState *exec, JSC::JSValue value,
   const JSC::Identifier &id, const QScriptValue::ResolveFlags &mode)
{
   JSC::JSObject *object = JSC::asObject(value);
   unsigned attribs = 0;
   JSC::PropertyDescriptor descriptor;

   if (object->getOwnPropertyDescriptor(exec, id, descriptor)) {
      attribs = descriptor.attributes();
   } else {
      if ((mode & QScriptValue::ResolvePrototype) && object->prototype() && object->prototype().isObject()) {
         JSC::JSValue proto = object->prototype();
         return propertyFlags(exec, proto, id, mode);
      }
      return Qt::EmptyFlag;
   }

   QScriptValue::PropertyFlags result = Qt::EmptyFlag;
   if (attribs & JSC::ReadOnly) {
      result |= QScriptValue::ReadOnly;
   }
   if (attribs & JSC::DontEnum) {
      result |= QScriptValue::SkipInEnumeration;
   }
   if (attribs & JSC::DontDelete) {
      result |= QScriptValue::Undeletable;
   }
   //We cannot rely on attribs JSC::Setter/Getter because they are not necesserly set by JSC (bug?)
   if (attribs & JSC::Getter || !object->lookupGetter(exec, id).isUndefinedOrNull()) {
      result |= QScriptValue::PropertyGetter;
   }

   if (attribs & JSC::Setter || !object->lookupSetter(exec, id).isUndefinedOrNull()) {
      result |= QScriptValue::PropertySetter;
   }

   if (attribs & QScript::QObjectMemberAttribute) {
      result |= QScriptValue::QObjectMember;
   }

   result |= QScriptValue::PropertyFlag(attribs & QScriptValue::UserRange);
   return result;
}

QScriptString QScriptEnginePrivate::toStringHandle(const JSC::Identifier &name)
{
   QScriptString result;
   QScriptStringPrivate *p = new QScriptStringPrivate(this, name, QScriptStringPrivate::HeapAllocated);
   QScriptStringPrivate::init(result, p);
   registerScriptString(p);
   return result;
}

QScriptEngine::QScriptEngine()
   : QObject(nullptr), d_ptr(new QScriptEnginePrivate)
{
   d_ptr->q_ptr = this;
}

QScriptEngine::QScriptEngine(QObject *parent)
   : QObject(parent), d_ptr(new QScriptEnginePrivate)
{
   d_ptr->q_ptr = this;
}

// internal (cs)
QScriptEngine::QScriptEngine(QScriptEnginePrivate &dd, QObject *parent)
   : QObject(parent), d_ptr(&dd)
{
   d_ptr->q_ptr = this;
}

QScriptEngine::~QScriptEngine()
{
}

QScriptValue QScriptEngine::globalObject() const
{
   Q_D(const QScriptEngine);
   QScript::APIShim shim(const_cast<QScriptEnginePrivate *>(d));
   JSC::JSObject *result = d->globalObject();
   return const_cast<QScriptEnginePrivate *>(d)->scriptValueFromJSCValue(result);
}

void QScriptEngine::setGlobalObject(const QScriptValue &object)
{
   Q_D(QScriptEngine);

   if (!object.isObject()) {
      return;
   }

   QScript::APIShim shim(d);
   JSC::JSObject *jscObject = JSC::asObject(d->scriptValueToJSCValue(object));
   d->setGlobalObject(jscObject);
}

QScriptValue QScriptEngine::nullValue()
{
   Q_D(QScriptEngine);
   return d->scriptValueFromJSCValue(JSC::jsNull());
}

QScriptValue QScriptEngine::undefinedValue()
{
   Q_D(QScriptEngine);
   return d->scriptValueFromJSCValue(JSC::jsUndefined());
}

QScriptValue QScriptEngine::newFunction(QScriptEngine::FunctionSignature fun, const QScriptValue &prototype,
   int length)
{
   Q_D(QScriptEngine);
   QScript::APIShim shim(d);
   JSC::ExecState *exec = d->currentFrame;
   JSC::JSValue function = new (exec)QScript::FunctionWrapper(exec, length, JSC::Identifier(exec, ""), fun);
   QScriptValue result = d->scriptValueFromJSCValue(function);

   result.setProperty(QLatin1String("prototype"), prototype,
      QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);

   const_cast<QScriptValue &>(prototype).setProperty(QLatin1String("constructor"), result, QScriptValue::SkipInEnumeration);

   return result;
}

QScriptValue QScriptEngine::newRegExp(const QRegularExpression &regexp)
{
   Q_D(QScriptEngine);
   QScript::APIShim shim(d);

   return d->scriptValueFromJSCValue(d->newRegExp(d->currentFrame, regexp));
}

QScriptValue QScriptEngine::newVariant(const QVariant &value)
{
   Q_D(QScriptEngine);
   QScript::APIShim shim(d);

   return d->scriptValueFromJSCValue(d->newVariant(value));
}

QScriptValue QScriptEngine::newVariant(const QScriptValue &object, const QVariant &value)
{
   Q_D(QScriptEngine);
   QScript::APIShim shim(d);
   JSC::JSValue jsObject = d->scriptValueToJSCValue(object);
   return d->scriptValueFromJSCValue(d->newVariant(jsObject, value));
}

QScriptValue QScriptEngine::newQObject(QObject *object, ValueOwnership ownership, const QObjectWrapOptions &options)
{
   Q_D(QScriptEngine);
   QScript::APIShim shim(d);
   JSC::JSValue jscQObject = d->newQObject(object, ownership, options);
   return d->scriptValueFromJSCValue(jscQObject);
}

QScriptValue QScriptEngine::newQObject(const QScriptValue &scriptObject, QObject *qtObject,
   ValueOwnership ownership, const QObjectWrapOptions &options)
{
   Q_D(QScriptEngine);

   if (!scriptObject.isObject()) {
      return newQObject(qtObject, ownership, options);
   }

   QScript::APIShim shim(d);
   JSC::JSObject *jscObject = JSC::asObject(QScriptValuePrivate::get(scriptObject)->jscValue);

   if (!jscObject->inherits(&QScriptObject::info)) {
      qWarning("QScriptEngine::newQObject(): changing class of non-QScriptObject not supported");
      return QScriptValue();
   }

   QScriptObject *jscScriptObject = static_cast<QScriptObject *>(jscObject);
   if (!scriptObject.isQObject()) {
      jscScriptObject->setDelegate(new QScript::QObjectDelegate(qtObject, ownership, options));
   } else {
      QScript::QObjectDelegate *delegate = static_cast<QScript::QObjectDelegate *>(jscScriptObject->delegate());
      delegate->setValue(qtObject);
      delegate->setOwnership(ownership);
      delegate->setOptions(options);
   }
   return scriptObject;
}

QScriptValue QScriptEngine::newObject()
{
   Q_D(QScriptEngine);
   QScript::APIShim shim(d);
   return d->scriptValueFromJSCValue(d->newObject());
}

QScriptValue QScriptEngine::newObject(QScriptClass *scriptClass, const QScriptValue &data)
{
   Q_D(QScriptEngine);
   QScript::APIShim shim(d);
   JSC::ExecState *exec = d->currentFrame;
   QScriptObject *result = new (exec) QScriptObject(d->scriptObjectStructure);
   result->setDelegate(new QScript::ClassObjectDelegate(scriptClass));
   QScriptValue scriptObject = d->scriptValueFromJSCValue(result);
   scriptObject.setData(data);
   QScriptValue proto = scriptClass->prototype();

   if (proto.isValid()) {
      scriptObject.setPrototype(proto);
   }

   return scriptObject;
}

// internal (cs)
QScriptValue QScriptEngine::newActivationObject()
{
   qWarning("QScriptEngine::newActivationObject() not implemented");
   // ### JSActivation or JSVariableObject?
   return QScriptValue();
}

QScriptValue QScriptEngine::newFunction(QScriptEngine::FunctionSignature fun, int length)
{
   Q_D(QScriptEngine);
   QScript::APIShim shim(d);
   JSC::ExecState *exec = d->currentFrame;
   JSC::JSValue function = new (exec)QScript::FunctionWrapper(exec, length, JSC::Identifier(exec, ""), fun);
   QScriptValue result = d->scriptValueFromJSCValue(function);
   QScriptValue proto = newObject();
   result.setProperty(QLatin1String("prototype"), proto,
      QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
   proto.setProperty(QLatin1String("constructor"), result, QScriptValue::SkipInEnumeration);
   return result;
}

// internal (cs)
QScriptValue QScriptEngine::newFunction(QScriptEngine::FunctionWithArgSignature fun, void *arg)
{
   Q_D(QScriptEngine);
   QScript::APIShim shim(d);
   JSC::ExecState *exec = d->currentFrame;
   JSC::JSValue function = new (exec)QScript::FunctionWithArgWrapper(exec, /*length=*/0, JSC::Identifier(exec, ""), fun,
      arg);
   QScriptValue result = d->scriptValueFromJSCValue(function);
   QScriptValue proto = newObject();
   result.setProperty(QLatin1String("prototype"), proto,
      QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
   proto.setProperty(QLatin1String("constructor"), result, QScriptValue::SkipInEnumeration);
   return result;
}

QScriptValue QScriptEngine::newArray(uint length)
{
   Q_D(QScriptEngine);
   QScript::APIShim shim(d);
   return d->scriptValueFromJSCValue(d->newArray(d->currentFrame, length));
}

QScriptValue QScriptEngine::newRegExp(const QString &pattern, const QString &flags)
{
   Q_D(QScriptEngine);
   QScript::APIShim shim(d);
   return d->scriptValueFromJSCValue(d->newRegExp(d->currentFrame, pattern, flags));
}

QScriptValue QScriptEngine::newDate(qsreal value)
{
   Q_D(QScriptEngine);
   QScript::APIShim shim(d);
   return d->scriptValueFromJSCValue(d->newDate(d->currentFrame, value));
}

QScriptValue QScriptEngine::newDate(const QDateTime &value)
{
   Q_D(QScriptEngine);
   QScript::APIShim shim(d);
   return d->scriptValueFromJSCValue(d->newDate(d->currentFrame, value));
}

QScriptValue QScriptEngine::newQMetaObject(
   const QMetaObject *metaObject, const QScriptValue &ctor)
{
   Q_D(QScriptEngine);
   QScript::APIShim shim(d);
   JSC::JSValue jscCtor = d->scriptValueToJSCValue(ctor);
   JSC::JSValue jscQMetaObject = d->newQMetaObject(metaObject, jscCtor);
   return d->scriptValueFromJSCValue(jscQMetaObject);
}

bool QScriptEngine::canEvaluate(const QString &program) const
{
   return QScriptEnginePrivate::canEvaluate(program);
}

bool QScriptEnginePrivate::canEvaluate(const QString &program)
{
   QScript::SyntaxChecker checker;
   QScript::SyntaxChecker::Result result = checker.checkSyntax(program);
   return (result.state != QScript::SyntaxChecker::Intermediate);
}

QScriptSyntaxCheckResult QScriptEngine::checkSyntax(const QString &program)
{
   return QScriptEnginePrivate::checkSyntax(program);
}

QScriptSyntaxCheckResult QScriptEnginePrivate::checkSyntax(const QString &program)
{
   QScript::SyntaxChecker checker;
   QScript::SyntaxChecker::Result result = checker.checkSyntax(program);
   QScriptSyntaxCheckResultPrivate *p = new QScriptSyntaxCheckResultPrivate();

   switch (result.state) {
      case QScript::SyntaxChecker::Error:
         p->state = QScriptSyntaxCheckResult::Error;
         break;

      case QScript::SyntaxChecker::Intermediate:
         p->state = QScriptSyntaxCheckResult::Intermediate;
         break;

      case QScript::SyntaxChecker::Valid:
         p->state = QScriptSyntaxCheckResult::Valid;
         break;
   }

   p->errorLineNumber = result.errorLineNumber;
   p->errorColumnNumber = result.errorColumnNumber;
   p->errorMessage = result.errorMessage;

   return QScriptSyntaxCheckResult(p);
}

QScriptValue QScriptEngine::evaluate(const QString &program, const QString &fileName, int lineNumber)
{
   Q_D(QScriptEngine);
   QScript::APIShim shim(d);
   WTF::PassRefPtr<QScript::UStringSourceProviderWithFeedback> provider
      = QScript::UStringSourceProviderWithFeedback::create(program, fileName, lineNumber, d);
   intptr_t sourceId = provider->asID();
   JSC::SourceCode source(provider, lineNumber); //after construction of SourceCode provider variable will be null.

   JSC::ExecState *exec = d->currentFrame;
   WTF::RefPtr<JSC::EvalExecutable> executable = JSC::EvalExecutable::create(exec, source);
   bool compile = true;

   return d->scriptValueFromJSCValue(d->evaluateHelper(exec, sourceId, executable.get(), compile));
}

QScriptValue QScriptEngine::evaluate(const QScriptProgram &program)
{
   Q_D(QScriptEngine);
   QScriptProgramPrivate *program_d = QScriptProgramPrivate::get(program);
   if (!program_d) {
      return QScriptValue();
   }

   QScript::APIShim shim(d);
   JSC::ExecState *exec = d->currentFrame;
   JSC::EvalExecutable *executable = program_d->executable(exec, d);
   bool compile = !program_d->isCompiled;
   JSC::JSValue result = d->evaluateHelper(exec, program_d->sourceId,
         executable, compile);
   if (compile) {
      program_d->isCompiled = true;
   }
   return d->scriptValueFromJSCValue(result);
}

QScriptContext *QScriptEngine::currentContext() const
{
   Q_D(const QScriptEngine);
   return const_cast<QScriptEnginePrivate *>(d)->contextForFrame(d->currentFrame);
}

QScriptContext *QScriptEngine::pushContext()
{
   Q_D(QScriptEngine);
   QScript::APIShim shim(d);

   JSC::CallFrame *newFrame = d->pushContext(d->currentFrame, d->currentFrame->globalData().dynamicGlobalObject,
         JSC::ArgList(), nullptr);

   if (agent()) {
      agent()->contextPush();
   }

   return d->contextForFrame(newFrame);
}

JSC::CallFrame *QScriptEnginePrivate::pushContext(JSC::CallFrame *exec, JSC::JSValue _thisObject,
   const JSC::ArgList &args, JSC::JSObject *callee, bool calledAsConstructor)
{
   JSC::JSValue thisObject = _thisObject;
   if (!callee) {
      // callee can't be zero, as this can cause JSC to crash during GC
      // marking phase if the context's Arguments object has been created.
      // Fake it by using the global object. Note that this is also handled
      // in QScriptContext::callee(), as that function should still return
      // an invalid value.
      callee = originalGlobalObject();
   }

   if (calledAsConstructor) {
      //JSC doesn't create default created object for native functions. so we do it
      JSC::JSValue prototype = callee->get(exec, exec->propertyNames().prototype);
      JSC::Structure *structure = prototype.isObject() ? JSC::asObject(prototype)->inheritorID()
         : originalGlobalObject()->emptyObjectStructure();
      thisObject = new (exec) QScriptObject(structure);
   }

   int flags = NativeContext;
   if (calledAsConstructor) {
      flags |= CalledAsConstructorContext;
   }

   //build a frame
   JSC::CallFrame *newCallFrame = exec;

   if (callee == nullptr || exec->returnPC() == nullptr || (contextFlags(exec) & NativeContext)
            || (exec->codeBlock() && exec->callee() != callee)) {

      // interpreter did not build a frame for us,  need to check if the Interpreter might have already created
      // a frame for function called from JS

      JSC::Interpreter *interp = exec->interpreter();
      JSC::Register *oldEnd = interp->registerFile().end();
      int argc = args.size() + 1; //add "this"
      JSC::Register *newEnd = oldEnd + argc + JSC::RegisterFile::CallFrameHeaderSize;

      if (! interp->registerFile().grow(newEnd)) {
         return nullptr;   //### Stack overflow
      }

      newCallFrame = JSC::CallFrame::create(oldEnd);
      newCallFrame[0] = thisObject;

      int dst = 0;
      JSC::ArgList::const_iterator it;
      for (it = args.begin(); it != args.end(); ++it) {
         newCallFrame[++dst] = *it;
      }

      newCallFrame += argc + JSC::RegisterFile::CallFrameHeaderSize;
      newCallFrame->init(nullptr, nullptr, globalExec()->scopeChain(), exec, flags | ShouldRestoreCallFrame, argc, callee);

   } else {
      setContextFlags(newCallFrame, flags);

#if ENABLE(JIT)
      exec->registers()[JSC::RegisterFile::Callee] = JSC::JSValue(callee); //JIT let the callee set the 'callee'
#endif

      if (calledAsConstructor) {
         //update the new created this
         JSC::Register *thisRegister = thisRegisterForFrame(newCallFrame);
         *thisRegister = thisObject;
      }
   }
   currentFrame = newCallFrame;
   return newCallFrame;
}

void QScriptEngine::popContext()
{
   if (agent()) {
      agent()->contextPop();
   }

   Q_D(QScriptEngine);
   QScript::APIShim shim(d);

   if (d->currentFrame->returnPC() != nullptr || d->currentFrame->codeBlock() != nullptr
            || ! currentContext()->parentContext()) {
      qWarning("QScriptEngine::popContext() does not match with pushContext()");
      return;
   }

   d->popContext();
}

void QScriptEnginePrivate::popContext()
{
   uint flags = contextFlags(currentFrame);
   bool hasScope = flags & HasScopeContext;
   if (flags & ShouldRestoreCallFrame) { //normal case
      JSC::RegisterFile &registerFile = currentFrame->interpreter()->registerFile();
      JSC::Register *const newEnd = currentFrame->registers() - JSC::RegisterFile::CallFrameHeaderSize -
         currentFrame->argumentCount();
      if (hasScope) {
         currentFrame->scopeChain()->pop()->deref();
      }
      registerFile.shrink(newEnd);
   } else if (hasScope) { //the stack frame was created by the Interpreter, we don't need to rewind it.
      currentFrame->setScopeChain(currentFrame->scopeChain()->pop());
      currentFrame->scopeChain()->deref();
   }

   currentFrame = currentFrame->callerFrame();
}

bool QScriptEngine::hasUncaughtException() const
{
   Q_D(const QScriptEngine);
   JSC::ExecState *exec = d->globalExec();
   return exec->hadException() || d->currentException().isValid();
}

QScriptValue QScriptEngine::uncaughtException() const
{
   Q_D(const QScriptEngine);
   QScriptValue result;
   JSC::ExecState *exec = d->globalExec();
   if (exec->hadException()) {
      result = const_cast<QScriptEnginePrivate *>(d)->scriptValueFromJSCValue(exec->exception());
   } else {
      result = d->currentException();
   }
   return result;
}

int QScriptEngine::uncaughtExceptionLineNumber() const
{
   Q_D(const QScriptEngine);
   if (!hasUncaughtException()) {
      return -1;
   }
   if (d->uncaughtExceptionLineNumber != -1) {
      return d->uncaughtExceptionLineNumber;
   }

   return uncaughtException().property(QLatin1String("lineNumber")).toInt32();
}

QStringList QScriptEngine::uncaughtExceptionBacktrace() const
{
   Q_D(const QScriptEngine);
   return d->uncaughtExceptionBacktrace;
}

void QScriptEngine::clearExceptions()
{
   Q_D(QScriptEngine);
   JSC::ExecState *exec = d->currentFrame;
   exec->clearException();
   d->clearCurrentException();
}

QScriptValue QScriptEngine::defaultPrototype(int metaTypeId) const
{
   Q_D(const QScriptEngine);
   return const_cast<QScriptEnginePrivate *>(d)->scriptValueFromJSCValue(d->defaultPrototype(metaTypeId));
}

void QScriptEngine::setDefaultPrototype(int metaTypeId, const QScriptValue &prototype)
{
   Q_D(QScriptEngine);
   d->setDefaultPrototype(metaTypeId, d->scriptValueToJSCValue(prototype));
}

QScriptValue QScriptEngine::create(const QVariant &data)
{
   Q_D(QScriptEngine);
   QScript::APIShim shim(d);

   return d->scriptValueFromJSCValue(d->create(d->currentFrame, data));
}

JSC::JSValue QScriptEnginePrivate::create(JSC::ExecState *exec, const QVariant &data)
{
   uint type = data.userType();

   JSC::JSValue result;
   QScriptEnginePrivate *eng = exec ? QScript::scriptEngineFromExec(exec) : nullptr;
   QScriptTypeInfo *info     = eng ? eng->m_typeInfos.value(type) : nullptr;

   if (info && info->marshal) {
      result = eng->scriptValueToJSCValue(info->marshal(eng->q_func(), data));

   } else {
      switch (type) {

         case QVariant::Invalid:
         case QVariant::Void:
            return JSC::jsUndefined();

         case QVariant::Bool:
            return JSC::jsBoolean(data.getData<bool>());

         case QVariant::Int:
            return JSC::jsNumber(exec, data.getData<int>());

         case QVariant::UInt:
            return JSC::jsNumber(exec, data.getData<uint>());

         case QVariant::Short:
            return JSC::jsNumber(exec, data.getData<short>());

         case QVariant::UShort:
            return JSC::jsNumber(exec, data.getData<ushort>());

         case QVariant::Long:
            return JSC::jsNumber(exec, data.getData<long>());

         case QVariant::ULong:
            return JSC::jsNumber(exec, data.getData<ulong>());

         case QVariant::LongLong:
            return JSC::jsNumber(exec, qsreal(data.getData<qint64>()));

         case QVariant::ULongLong:
            return JSC::jsNumber(exec, qsreal(data.getData<quint64>()));

         case QVariant::Double:
            return JSC::jsNumber(exec, qsreal(data.getData<double>()));

         case QVariant::Float:
            return JSC::jsNumber(exec, data.getData<float>());

         case QVariant::Char:
            return JSC::jsNumber(exec, data.getData<char>());

         case QVariant::UChar:
            return JSC::jsNumber(exec, data.getData<uchar>());

         case QVariant::QChar:
            return JSC::jsNumber(exec, data.getData<QChar32>().unicode());

         case QVariant::String:
            return JSC::jsString(exec, data.getData<QString>());

         //
         case QVariant::StringList:
            result = arrayFromStringList(exec, data.getData<QStringList>());
            break;

         case QVariant::List:
            result = arrayFromVariantList(exec, data.getData<QVariantList>());
            break;

         case QVariant::Map:
            result = objectFromVariantMap(exec, data.getData<QVariantMap>());
            break;

         case QVariant::DateTime:
            result = newDate(exec, data.getData<QDateTime>());
            break;

         case QVariant::Date:
            result = newDate(exec, QDateTime(data.getData<QDate>()));
            break;

         case QVariant::RegularExpression:
            result = newRegExp(exec, data.getData<QRegularExpression>());
            break;

         case QVariant::ObjectStar:
         case QVariant::WidgetStar:
            result = eng->newQObject(data.getData<QObject *>());
            break;

         default:
            if (isPtr2QObject(type)) {
               result = eng->newQObject(data.getData<QObject *>());
               break;
            }

            if (type == QVariant::typeToTypeId<QScriptValue>()) {
               result = eng->scriptValueToJSCValue(data.getData<QScriptValue>());

               if (! result) {
                  return JSC::jsUndefined();
               }

            } else if (type == QVariant::typeToTypeId<QObjectList>()) {
               qScriptRegisterSequenceMetaType<QObjectList>(eng->q_func());
               return create(exec, data);

            } else if (type == QVariant::typeToTypeId<QList<int>>()) {
               qScriptRegisterSequenceMetaType<QList<int>>(eng->q_func());
               return create(exec, data);

            } else {
               QString typeName = QVariant::typeToName(type);

               if (typeName.endsWith('*')) {
                  return JSC::jsNull();

               } else {
                  result = eng->newVariant(data);
               }
            }
      }
   }

   if (result && result.isObject() && info && info->prototype
               && JSC::JSValue::strictEqual(exec, JSC::asObject(result)->prototype(),
               eng->originalGlobalObject()->objectPrototype())) {
      JSC::asObject(result)->setPrototype(info->prototype);
   }

   return result;
}

QVariant QScriptEnginePrivate::convertValue(JSC::ExecState *exec, JSC::JSValue value, uint type)
{
   QVariant retval;

   QScriptEnginePrivate *eng = exec ? QScript::scriptEngineFromExec(exec) : nullptr;

   if (eng) {
      QScriptTypeInfo *info = eng->m_typeInfos.value(type);

      if (info && info->demarshal) {
         retval = info->demarshal(eng->scriptValueFromJSCValue(value));
         return retval;
      }
   }

   switch (type) {
      case QVariant::Bool:
         retval.setValue<bool>(toBool(exec, value));
         return retval;

      case QVariant::Int:
         retval.setValue<int>(toInt32(exec, value));
         return retval;

      case QVariant::UInt:
         retval.setValue<uint>(toUInt32(exec, value));
         return retval;

      case QVariant::Long:
         retval.setValue<long>(long(toInteger(exec, value)));
         return retval;

      case QVariant::ULong:
         retval.setValue<ulong>(ulong(toInteger(exec, value)));
         return retval;

      case QVariant::LongLong:
         retval.setValue<qint64>(qint64(toInteger(exec, value)));
         return retval;

      case QVariant::ULongLong:
         retval.setValue<quint64>(quint64(toInteger(exec, value)));
         return retval;

      case QVariant::Double:
         retval.setValue<double>(toNumber(exec, value));
         return retval;

      case QVariant::Float:
         retval.setValue<float>(toNumber(exec, value));
         return retval;

      case QVariant::Short:
         retval.setValue<short>(short(toInt32(exec, value)));
         return retval;

      case QVariant::UShort:
         retval.setValue<unsigned short>(QScript::ToUInt16(toNumber(exec, value)));
         return retval;

      case QVariant::Char:
         retval.setValue<char>(char(toInt32(exec, value)));
         return retval;

      case QVariant::UChar:
         retval.setValue<unsigned char>((unsigned char)(toInt32(exec, value)));
         return retval;

      case QVariant::QChar:
         if (value.isString()) {
            QString str = toString(exec, value);
            retval.setValue<QChar>(str.isEmpty() ? QChar() : str.at(0));
         } else {
            retval.setValue<QChar>(char32_t(QScript::ToUInt32(toNumber(exec, value))));
         }
         return retval;

      case QVariant::String:
         if (value.isUndefined() || value.isNull()) {
            retval.setValue<QString>(QString());
         } else {
            retval.setValue<QString>(toString(exec, value));
         }
         return retval;

      case QVariant::RegularExpression:
         if (isRegExp(value)) {
            retval.setValue<QRegularExpression>(toRegExp(exec, value));
            return retval;
         }
         break;

      case QVariant::StringList:
         if (isArray(value)) {
            retval.setValue<QStringList>(stringListFromArray(exec, value));
            return retval;
         }
         break;

      case QVariant::DateTime:
         if (isDate(value)) {
            retval.setValue<QDateTime>(toDateTime(exec, value));
            return retval;
         }
         break;

      case QVariant::Date:
         if (isDate(value)) {
            retval.setValue<QDate>(toDateTime(exec, value).date());
            return retval;
         }
         break;

      case QVariant::ObjectStar:
         if (isQObject(value)) {
            retval.setValue<QObject *>(toQObject(exec, value));
            return retval;
         }
         break;

      case QVariant::WidgetStar:
         if (isQObject(value)) {

            /* emerald (script, hold)
               QObject *tmp = toQObject(exec, value);

               if (! tmp || tmp->isWidgetType()) {
                  retval.setValue<QWidget *>(reinterpret_cast<QWidget *>(tmp));
                  return retval;
               }
            */

            return retval;

         }
         break;

      case QVariant::List:
         if (isArray(value)) {
            retval.setValue<QVariantList>(variantListFromArray(exec, JSC::asArray(value)));
            return retval;
         }
         break;

      case QVariant::Map:
         if (isObject(value)) {
            retval.setValue<QVariantMap>(variantMapFromObject(exec, JSC::asObject(value)));
            return retval;
         }
         break;

      case QVariant::Variant:
         retval.setValue<QVariant>(toVariant(exec, value));
         return retval;

      default:
         break;
   }

   QString name = QVariant::typeToName(type);

   if (isVariant(value) && name.endsWith('*')) {

      name.chop(1);
      uint valueType = QVariant::nameToType(name);

      QVariant &var  = variantValue(value);

      if (valueType == var.userType()) {
         // variant was Widget *, user wanted Widget
         return QVariant();

      } else {
         // look in the prototype chain
         JSC::JSValue proto = JSC::asObject(value)->prototype();

         while (proto.isObject()) {
            bool canCast = false;

            if (isVariant(proto)) {
               canCast = (type == variantValue(proto).userType())
                  || (valueType && (valueType == variantValue(proto).userType()));

            } else if (isQObject(proto)) {

               /*  emerald (script, hold)
                      if (QObject *qobject = toQObject(exec, proto)) {
                        canCast = qobject->qt_metacast(name) != 0;
                      }
               */
            }

            if (canCast) {
               QString varTypeName = QVariant::typeToName(var.userType());

               if (varTypeName.endsWith('*')) {
                  // variant was Widget *, user wanted Widget
                  return QVariant();

               } else {
                  return var;

               }
            }

            proto = JSC::asObject(proto)->prototype();
         }
      }

   } else if (type == QVariant::typeToTypeId<QScriptValue>()) {
      if (eng) {
         retval.setValue<QScriptValue>(eng->scriptValueFromJSCValue(value));
      }

   } else if (type == QVariant::typeToTypeId<QList<QObject *>>()) {
      if (eng) {
         qScriptRegisterSequenceMetaType<QList<QObject *>>(eng->q_func());
         retval = convertValue(exec, value, type);
      }

   } else if (type == QVariant::typeToTypeId<QList<int>>()) {
      if (eng) {
         qScriptRegisterSequenceMetaType<QList<int>>(eng->q_func());
         retval = convertValue(exec, value, type);
      }
   }

   return retval;
}

QVariant QScriptEnginePrivate::convertNumber(qsreal value, uint type)
{
   QVariant retval;

   switch (type) {

      case QVariant::Bool:
         retval.setValue<bool>(QScript::ToBool(value));
         break;

      case QVariant::Int:
         retval.setValue<int>(QScript::ToInt32(value));
         break;

      case QVariant::UInt:
         retval.setValue<uint>(QScript::ToUInt32(value));
         break;

      case QVariant::Long:
         retval.setValue<long>(long(QScript::ToInteger(value)));
         break;

      case QVariant::ULong:
         retval.setValue<ulong>(ulong(QScript::ToInteger(value)));
         break;

      case QVariant::LongLong:
         retval.setValue<qint64>(qint64(QScript::ToInteger(value)));
         break;

      case QVariant::ULongLong:
         retval.setValue<quint64>(quint64(QScript::ToInteger(value)));
         break;

      case QVariant::Double:
         retval.setValue<double>(value);
         break;

      case QVariant::String:
         retval.setValue<QString>(QScript::ToString(value));
         break;

      case QVariant::Float:
         retval.setValue<float>(value);
         break;

      case QVariant::Short:
         retval.setValue<short>(short(QScript::ToInt32(value)));
         break;

      case QVariant::UShort:
         retval.setValue<unsigned short>(QScript::ToUInt16(value));
         break;

      case QVariant::Char:
         retval.setValue<char>(char(QScript::ToInt32(value)));
         break;

      case QVariant::UChar:
         retval.setValue<unsigned char>((unsigned char)(QScript::ToInt32(value)));
         break;

      case QVariant::QChar:
         retval.setValue<QChar>(QChar(QScript::ToUInt16(value)));
         break;

      default:
         break;
   }

   return retval;
}

QVariant QScriptEnginePrivate::convertString(const QString &value, uint type)
{
   QVariant retval;

   switch (type) {

      case QVariant::Bool:
         retval.setValue<bool>(QScript::ToBool(value));
         break;

      case QVariant::Int:
         retval.setValue<int>(QScript::ToInt32(value));
         break;

      case QVariant::UInt:
         retval.setValue<uint>(QScript::ToUInt32(value));
         break;

      case QVariant::Long:
         retval.setValue<long>(long(QScript::ToInteger(value)));
         break;

      case QVariant::ULong:
         retval.setValue<ulong>(ulong(QScript::ToInteger(value)));
         break;

      case QVariant::LongLong:
         retval.setValue<qint64>(qint64(QScript::ToInteger(value)));
         break;

      case QVariant::ULongLong:
         retval.setValue<quint64>(quint64(QScript::ToInteger(value)));
         break;

      case QVariant::Double:
         retval.setValue<double>(QScript::ToNumber(value));
         break;

      case QVariant::String:
         retval.setValue<QString>(value);
         break;

      case QVariant::Float:
         retval.setValue<float>(QScript::ToNumber(value));
         break;

      case QVariant::Short:
         retval.setValue<short>(short(QScript::ToInt32(value)));
         break;

      case QVariant::UShort:
         retval.setValue<unsigned short>(QScript::ToUInt16(value));
         break;

      case QVariant::Char:
         retval.setValue<char>(char(QScript::ToInt32(value)));
         break;

      case QVariant::UChar:
         retval.setValue<unsigned char>((unsigned char)(QScript::ToInt32(value)));
         break;

      case QVariant::QChar:
         retval.setValue<QChar>(QChar(QScript::ToUInt16(value)));
         break;

      default:
         break;
   }

   return retval;
}

bool QScriptEnginePrivate::hasDemarshalFunction(uint type) const
{
   QScriptTypeInfo *info = m_typeInfos.value(type);
   return info && (info->demarshal != nullptr);
}

JSC::UString QScriptEnginePrivate::translationContextFromUrl(const JSC::UString &url)
{
   if (url != cachedTranslationUrl) {

      const QString &baseName = QFileInfo(url).baseName();

      if (baseName.startsWith("qrc:", Qt::CaseInsensitive)) {
         cachedTranslationContext = baseName.mid(4);
      } else {
         cachedTranslationContext = baseName;
      }

      cachedTranslationUrl = url;
   }

   return cachedTranslationContext;
}

// internal (cs)
QVariant QScriptEngine::convert(const QScriptValue &value, uint type)
{
   Q_D(QScriptEngine);
   QScript::APIShim shim(d);

   return QScriptEnginePrivate::convertValue(d->currentFrame, d->scriptValueToJSCValue(value), type);
}

// internal (cs)
QVariant QScriptEngine::convertV2(const QScriptValue &value, uint type)
{
   QScriptValuePrivate *vp = QScriptValuePrivate::get(value);

   if (vp) {
      switch (vp->type) {
         case QScriptValuePrivate::JavaScriptCore: {
            if (vp->engine) {
               QScript::APIShim shim(vp->engine);
               return QScriptEnginePrivate::convertValue(vp->engine->currentFrame, vp->jscValue, type);

            } else {
               return QScriptEnginePrivate::convertValue(nullptr, vp->jscValue, type);
            }
         }

         case QScriptValuePrivate::Number:
            return QScriptEnginePrivate::convertNumber(vp->numberValue, type);

         case QScriptValuePrivate::String:
            return QScriptEnginePrivate::convertString(vp->stringValue, type);
      }
   }

   return QVariant();
}

// internal (cs)
void QScriptEngine::registerCustomType(uint type, MarshalFunction mf,
   DemarshalFunction df, const QScriptValue &prototype)
{
   Q_D(QScriptEngine);

   QScript::APIShim shim(d);
   QScriptTypeInfo *info = d->m_typeInfos.value(type);

   if (!info) {
      info = new QScriptTypeInfo();
      d->m_typeInfos.insert(type, info);
   }

   info->marshal = mf;
   info->demarshal = df;
   info->prototype = d->scriptValueToJSCValue(prototype);
}

void QScriptEngine::installTranslatorFunctions(const QScriptValue &object)
{
   Q_D(QScriptEngine);
   QScript::APIShim shim(d);
   JSC::ExecState *exec = d->currentFrame;
   JSC::JSValue jscObject = d->scriptValueToJSCValue(object);
   JSC::JSGlobalObject *glob = d->originalGlobalObject();
   if (!jscObject || !jscObject.isObject()) {
      jscObject = d->globalObject();
   }
   //    unsigned attribs = JSC::DontEnum;

   JSC::asObject(jscObject)->putDirectFunction(exec, new (exec)JSC::NativeFunctionWrapper(exec,
         glob->prototypeFunctionStructure(), 5, JSC::Identifier(exec, "qsTranslate"), QScript::functionQsTranslate));
   JSC::asObject(jscObject)->putDirectFunction(exec, new (exec)JSC::NativeFunctionWrapper(exec,
         glob->prototypeFunctionStructure(), 2, JSC::Identifier(exec, "QT_TRANSLATE_NOOP"), QScript::functionQsTranslateNoOp));
   JSC::asObject(jscObject)->putDirectFunction(exec, new (exec)JSC::NativeFunctionWrapper(exec,
         glob->prototypeFunctionStructure(), 3, JSC::Identifier(exec, "qsTr"), QScript::functionQsTr));
   JSC::asObject(jscObject)->putDirectFunction(exec, new (exec)JSC::NativeFunctionWrapper(exec,
         glob->prototypeFunctionStructure(), 1, JSC::Identifier(exec, "QT_TR_NOOP"), QScript::functionQsTrNoOp));
   JSC::asObject(jscObject)->putDirectFunction(exec, new (exec)JSC::NativeFunctionWrapper(exec,
         glob->prototypeFunctionStructure(), 1, JSC::Identifier(exec, "qsTrId"), QScript::functionQsTrId));
   JSC::asObject(jscObject)->putDirectFunction(exec, new (exec)JSC::NativeFunctionWrapper(exec,
         glob->prototypeFunctionStructure(), 1, JSC::Identifier(exec, "QT_TRID_NOOP"), QScript::functionQsTrIdNoOp));

   glob->stringPrototype()->putDirectFunction(exec, new (exec)JSC::NativeFunctionWrapper(exec,
         glob->prototypeFunctionStructure(), 1, JSC::Identifier(exec, "arg"), QScript::stringProtoFuncArg));
}

QScriptValue QScriptEngine::importExtension(const QString &extension)
{

#if defined(QT_NO_SETTINGS)
   Q_UNUSED(extension);
#else
   Q_D(QScriptEngine);
   QScript::APIShim shim(d);
   if (d->importedExtensions.contains(extension)) {
      return undefinedValue();   // already imported
   }

   QScriptContext *context = currentContext();
   QCoreApplication *app = QCoreApplication::instance();
   if (!app) {
      return context->throwError(QLatin1String("No application object"));
   }

   QObjectList staticPlugins = QPluginLoader::staticInstances();
   QStringList libraryPaths = app->libraryPaths();
   QString dot = QLatin1String(".");
   QStringList pathComponents = extension.split(dot);
   QString initDotJs = QLatin1String("__init__.js");

   QString ext;
   for (int i = 0; i < pathComponents.count(); ++i) {
      if (!ext.isEmpty()) {
         ext.append(dot);
      }
      ext.append(pathComponents.at(i));
      if (d->importedExtensions.contains(ext)) {
         continue;   // already imported
      }

      if (d->extensionsBeingImported.contains(ext)) {
         return context->throwError(QString("recursive import of %0").formatArg(extension));
      }
      d->extensionsBeingImported.insert(ext);

      QScriptExtensionInterface *iface = nullptr;
      QString initjsContents;
      QString initjsFileName;

      // look for the extension in static plugins
      for (int j = 0; j < staticPlugins.size(); ++j) {
         iface = qobject_cast<QScriptExtensionInterface *>(staticPlugins.at(j));
         if (!iface) {
            continue;
         }
         if (iface->keys().contains(ext)) {
            break;              // use this one
         } else {
            iface = nullptr;   // keep looking
         }
      }

      {
         // look for __init__.js resource
         QString path = QString::fromLatin1(":/qtscriptextension");
         for (int j = 0; j <= i; ++j) {
            path.append(QLatin1Char('/'));
            path.append(pathComponents.at(j));
         }
         path.append(QLatin1Char('/'));
         path.append(initDotJs);
         QFile file(path);
         if (file.open(QIODevice::ReadOnly)) {
            QTextStream ts(&file);
            initjsContents = ts.readAll();
            initjsFileName = path;
            file.close();
         }
      }

      if (!iface && initjsContents.isEmpty()) {
         // look for the extension in library paths
         for (int j = 0; j < libraryPaths.count(); ++j) {
            QString libPath = libraryPaths.at(j) + QDir::separator() + QLatin1String("script");
            QDir dir(libPath);
            if (!dir.exists(dot)) {
               continue;
            }

            // look for C++ plugin
            QFileInfoList files = dir.entryInfoList(QDir::Files);
            for (int k = 0; k < files.count(); ++k) {
               QFileInfo entry = files.at(k);
               QString filePath = entry.canonicalFilePath();
               QPluginLoader loader(filePath);
               iface = qobject_cast<QScriptExtensionInterface *>(loader.instance());

               if (iface) {
                  if (iface->keys().contains(ext)) {
                     break;              // use this one
                  } else {
                     iface = nullptr;   // keep looking
                  }
               }
            }

            // look for __init__.js in the corresponding dir
            QDir dirdir(libPath);
            bool dirExists = dirdir.exists();

            for (int k = 0; dirExists && (k <= i); ++k) {
               dirExists = dirdir.cd(pathComponents.at(k));
            }

            if (dirExists && dirdir.exists(initDotJs)) {
               QFile file(dirdir.canonicalPath() + QDir::separator() + initDotJs);

               if (file.open(QIODevice::ReadOnly)) {
                  QTextStream ts(&file);
                  initjsContents = ts.readAll();
                  initjsFileName = file.fileName();
                  file.close();
               }
            }

            if (iface || !initjsContents.isEmpty()) {
               break;
            }
         }
      }

      if (!iface && initjsContents.isEmpty()) {
         d->extensionsBeingImported.remove(ext);
         return context->throwError(QString("Unable to import %0: no such extension").formatArg(extension));
      }

      // initialize the extension in a new context
      QScriptContext *ctx = pushContext();
      ctx->setThisObject(globalObject());
      ctx->activationObject().setProperty(QLatin1String("__extension__"), ext,
         QScriptValue::ReadOnly | QScriptValue::Undeletable);

      ctx->activationObject().setProperty("__setupPackage__", newFunction(QScript::__setupPackage__));
      ctx->activationObject().setProperty("__postInit__", QScriptValue(QScriptValue::UndefinedValue));

      // the script is evaluated first
      if (!initjsContents.isEmpty()) {
         QScriptValue ret = evaluate(initjsContents, initjsFileName);
         if (hasUncaughtException()) {
            popContext();
            d->extensionsBeingImported.remove(ext);
            return ret;
         }
      }

      // next, the C++ plugin is called
      if (iface) {
         iface->initialize(ext, this);
         if (hasUncaughtException()) {
            QScriptValue ret = uncaughtException(); // ctx_p->returnValue();
            popContext();
            d->extensionsBeingImported.remove(ext);
            return ret;
         }
      }

      // if the __postInit__ function has been set, we call it
      QScriptValue postInit = ctx->activationObject().property(QLatin1String("__postInit__"));
      if (postInit.isFunction()) {
         postInit.call(globalObject());
         if (hasUncaughtException()) {
            QScriptValue ret = uncaughtException(); // ctx_p->returnValue();
            popContext();
            d->extensionsBeingImported.remove(ext);
            return ret;
         }
      }

      popContext();

      d->importedExtensions.insert(ext);
      d->extensionsBeingImported.remove(ext);
   } // for (i)
#endif
   return undefinedValue();
}

QStringList QScriptEngine::availableExtensions() const
{
#if defined(QT_NO_SETTINGS)
   return QStringList();
#else
   QCoreApplication *app = QCoreApplication::instance();
   if (!app) {
      return QStringList();
   }

   QSet<QString> result;

   QObjectList staticPlugins = QPluginLoader::staticInstances();
   for (int i = 0; i < staticPlugins.size(); ++i) {
      QScriptExtensionInterface *iface;
      iface = qobject_cast<QScriptExtensionInterface *>(staticPlugins.at(i));

      if (iface) {
         QStringList keys = iface->keys();
         for (int j = 0; j < keys.count(); ++j) {
            result << keys.at(j);
         }
      }
   }

   QStringList libraryPaths = app->libraryPaths();
   for (int i = 0; i < libraryPaths.count(); ++i) {
      QString libPath = libraryPaths.at(i) + QDir::separator() + QLatin1String("script");
      QDir dir(libPath);
      if (!dir.exists()) {
         continue;
      }

      // look for C++ plugins
      QFileInfoList files = dir.entryInfoList(QDir::Files);
      for (int j = 0; j < files.count(); ++j) {
         QFileInfo entry = files.at(j);
         QString filePath = entry.canonicalFilePath();
         QPluginLoader loader(filePath);
         QScriptExtensionInterface *iface;

         iface = qobject_cast<QScriptExtensionInterface *>(loader.instance());
         if (iface) {
            QStringList keys = iface->keys();
            for (int k = 0; k < keys.count(); ++k) {
               result << keys.at(k);
            }
         }
      }

      // look for scripts
      QString initDotJs = QLatin1String("__init__.js");
      QList<QFileInfo> stack;
      stack << dir.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot);
      while (!stack.isEmpty()) {
         QFileInfo entry = stack.takeLast();
         QDir dd(entry.canonicalFilePath());
         if (dd.exists(initDotJs)) {
            QString rpath = dir.relativeFilePath(dd.canonicalPath());
            QStringList components = rpath.split(QLatin1Char('/'));
            result << components.join(QLatin1String("."));
            stack << dd.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot);
         }
      }
   }

   QStringList lst = result.toList();
   std::sort(lst.begin(), lst.end());

   return lst;
#endif
}

QStringList QScriptEngine::importedExtensions() const
{
   Q_D(const QScriptEngine);
   QStringList lst = d->importedExtensions.toList();
   std::sort(lst.begin(), lst.end());
   return lst;
}

void QScriptEngine::collectGarbage()
{
   Q_D(QScriptEngine);
   d->collectGarbage();
}

void QScriptEngine::reportAdditionalMemoryCost(int size)
{
   Q_D(QScriptEngine);
   d->reportAdditionalMemoryCost(size);
}

void QScriptEngine::setProcessEventsInterval(int interval)
{
   Q_D(QScriptEngine);
   d->processEventsInterval = interval;

   if (interval > 0) {
      d->globalData->timeoutChecker->setCheckInterval(interval);
   }

   d->timeoutChecker()->setShouldProcessEvents(interval > 0);
}

int QScriptEngine::processEventsInterval() const
{
   Q_D(const QScriptEngine);
   return d->processEventsInterval;
}

bool QScriptEngine::isEvaluating() const
{
   Q_D(const QScriptEngine);
   return (d->currentFrame != d->globalExec()) || d->inEval;
}

void QScriptEngine::abortEvaluation(const QScriptValue &result)
{
   Q_D(QScriptEngine);
   if (!isEvaluating()) {
      return;
   }
   d->abortResult = result;
   d->timeoutChecker()->setShouldAbort(true);
   JSC::throwError(d->currentFrame, JSC::createInterruptedExecutionException(&d->currentFrame->globalData()).toObject(
         d->currentFrame));
}

bool qScriptConnect(QObject *sender, const QString &signal, const QScriptValue &receiver, const QScriptValue &function)
{
   if (! sender || signal.isEmpty()) {
      return false;
   }

   if (! function.isFunction()) {
      return false;
   }

   if (receiver.isObject() && (receiver.engine() != function.engine())) {
      return false;
   }

   QScriptEnginePrivate *engine = QScriptEnginePrivate::get(function.engine());
   QScript::APIShim shim(engine);
   JSC::JSValue jscReceiver = engine->scriptValueToJSCValue(receiver);
   JSC::JSValue jscFunction = engine->scriptValueToJSCValue(function);

   return engine->scriptConnect(sender, signal, jscReceiver, jscFunction, Qt::AutoConnection);
}

bool qScriptDisconnect(QObject *sender, const QString &signal, const QScriptValue &receiver, const QScriptValue &function)
{
   if (! sender || signal.isEmpty()) {
      return false;
   }

   if (! function.isFunction()) {
      return false;
   }

   if (receiver.isObject() && (receiver.engine() != function.engine())) {
      return false;
   }

   QScriptEnginePrivate *engine = QScriptEnginePrivate::get(function.engine());
   QScript::APIShim shim(engine);
   JSC::JSValue jscReceiver = engine->scriptValueToJSCValue(receiver);
   JSC::JSValue jscFunction = engine->scriptValueToJSCValue(function);

   return engine->scriptDisconnect(sender, signal, jscReceiver, jscFunction);
}

void QScriptEngine::setAgent(QScriptEngineAgent *agent)
{
   Q_D(QScriptEngine);

   if (agent && (agent->engine() != this)) {
      qWarning("QScriptEngine::setAgent(): "
         "cannot set agent belonging to different engine");
      return;
   }

   QScript::APIShim shim(d);
   if (d->activeAgent) {
      QScriptEngineAgentPrivate::get(d->activeAgent)->detach();
   }

   d->activeAgent = agent;
   if (agent) {
      QScriptEngineAgentPrivate::get(agent)->attach();
   }
}

QScriptEngineAgent *QScriptEngine::agent() const
{
   Q_D(const QScriptEngine);
   return d->activeAgent;
}

QScriptString QScriptEngine::toStringHandle(const QString &str)
{
   Q_D(QScriptEngine);
   QScript::APIShim shim(d);
   return d->toStringHandle(JSC::Identifier(d->currentFrame, str));
}

QScriptValue QScriptEngine::toObject(const QScriptValue &value)
{
   Q_D(QScriptEngine);
   QScript::APIShim shim(d);
   JSC::JSValue jscValue = d->scriptValueToJSCValue(value);
   if (!jscValue || jscValue.isUndefined() || jscValue.isNull()) {
      return QScriptValue();
   }
   JSC::ExecState *exec = d->currentFrame;
   JSC::JSValue result = jscValue.toObject(exec);
   return d->scriptValueFromJSCValue(result);
}

// internal
QScriptValue QScriptEngine::objectById(qint64 id) const
{
   Q_D(const QScriptEngine);
   // Assumes that the cell was not been garbage collected
   return const_cast<QScriptEnginePrivate *>(d)->scriptValueFromJSCValue((JSC::JSCell *)id);
}

QScriptSyntaxCheckResult::QScriptSyntaxCheckResult(const QScriptSyntaxCheckResult &other)
   : d_ptr(other.d_ptr)
{
}

// internal
QScriptSyntaxCheckResult::QScriptSyntaxCheckResult(QScriptSyntaxCheckResultPrivate *d)
   : d_ptr(d)
{
}

// internal
QScriptSyntaxCheckResult::QScriptSyntaxCheckResult()
   : d_ptr(nullptr)
{
}

QScriptSyntaxCheckResult::~QScriptSyntaxCheckResult()
{
}

QScriptSyntaxCheckResult::State QScriptSyntaxCheckResult::state() const
{
   Q_D(const QScriptSyntaxCheckResult);
   if (!d) {
      return Valid;
   }
   return d->state;
}

int QScriptSyntaxCheckResult::errorLineNumber() const
{
   Q_D(const QScriptSyntaxCheckResult);
   if (!d) {
      return -1;
   }
   return d->errorLineNumber;
}

int QScriptSyntaxCheckResult::errorColumnNumber() const
{
   Q_D(const QScriptSyntaxCheckResult);
   if (!d) {
      return -1;
   }
   return d->errorColumnNumber;
}

QString QScriptSyntaxCheckResult::errorMessage() const
{
   Q_D(const QScriptSyntaxCheckResult);
   if (!d) {
      return QString();
   }
   return d->errorMessage;
}

QScriptSyntaxCheckResult &QScriptSyntaxCheckResult::operator=(const QScriptSyntaxCheckResult &other)
{
   d_ptr = other.d_ptr;
   return *this;
}

void QScriptEngine::_q_objectDestroyed(QObject *object)
{
   Q_D(QScriptEngine);
   d->_q_objectDestroyed(object);
}

QScriptEnginePrivate *QScriptEnginePrivate::cs_getPrivate(QScriptEngine *object)
{
   return object->d_ptr.data();
}
