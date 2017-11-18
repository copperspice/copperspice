/***********************************************************************
*
* Copyright (c) 2012-2017 Barbara Geller
* Copyright (c) 2012-2017 Ansel Sermersheim
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

#include <algorithm>

#include <config.h>
#include <qscriptengine.h>
#include <qscriptsyntaxchecker_p.h>

#include <qscriptengine_p.h>
#include <qscriptengineagent_p.h>
#include <qscriptcontext_p.h>
#include <qscriptstring_p.h>
#include <qscriptvalue_p.h>
#include <qscriptvalueiterator.h>
#include <qscriptclass.h>
#include <qscriptcontextinfo.h>
#include <qscriptprogram.h>
#include <qscriptprogram_p.h>
#include <qdebug.h>

#include <QtCore/qstringlist.h>
#include <QtCore/qmetaobject.h>

#include <math.h>

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

#include <qscriptfunction_p.h>
#include <qscriptclassobject_p.h>
#include <qscriptvariant_p.h>
#include <qscriptqobject_p.h>
#include <qscriptglobalobject_p.h>
#include <qscriptactivationobject_p.h>
#include <qscriptstaticscopeobject_p.h>

#include <QtCore/qcoreapplication.h>
#include <QtCore/qdir.h>
#include <QtCore/qfile.h>
#include <QtCore/qfileinfo.h>
#include <QtCore/qpluginloader.h>
#include <QtCore/qset.h>
#include <QtCore/qtextstream.h>
#include <qscriptextensioninterface.h>

Q_DECLARE_METATYPE(QScriptValue)
Q_DECLARE_METATYPE(QObjectList)
Q_DECLARE_METATYPE(QList<int>)

QT_BEGIN_NAMESPACE

class QScriptSyntaxCheckResultPrivate
{
 public:
   QScriptSyntaxCheckResultPrivate() {
      ref.store(0);
   }
   ~QScriptSyntaxCheckResultPrivate() {}

   QScriptSyntaxCheckResult::State state;
   int errorColumnNumber;
   int errorLineNumber;
   QString errorMessage;
   QAtomicInt ref;
};

class QScriptTypeInfo
{
 public:
   QScriptTypeInfo() : signature(0, '\0'), marshal(0), demarshal(0) {
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
                                      QTime(tm.hour, tm.minute, tm.second, ms), Qt::UTC);
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
      return JSC::throwError(exec, JSC::TypeError,
                             "Function.prototype.discconnect() Can not disconnect from deleted QObject");
   }

   QMetaMethod sig = meta->method(qtSignal->initialIndex());

   if (sig.methodType() != QMetaMethod::Signal) {
      QString message = QString::fromLatin1("Function.prototype.disconnect() %0::%1 is not a signal")
                        .arg(QLatin1String(qtSignal->metaObject()->className()))
                        .arg(QLatin1String(sig.methodSignature().constData()));

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
                        .arg(QLatin1String(qtSignal->metaObject()->className()))
                        .arg(QLatin1String(sig.methodSignature().constData()));

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
      QString message = QString::fromLatin1("Function.prototype.connect() %0::%1 is not a signal")
                        .arg(QLatin1String(qtSignal->metaObject()->className()))
                        .arg(QLatin1String(sig.methodSignature().constData()));

      return JSC::throwError(exec, JSC::TypeError, message);
   }

   {
      QList<int> overloads = qtSignal->overloadedIndexes();

      if (!overloads.isEmpty()) {
         overloads.append(qtSignal->initialIndex());

         QByteArray signature = sig.methodSignature();

         QString message = QString::fromLatin1("Function.prototype.connect() Ambiguous connect to %0::%1(); candidates are\n")
                           .arg(QLatin1String(qtSignal->metaObject()->className()))
                           .arg(QLatin1String(signature.left(signature.indexOf('('))));

         for (int i = 0; i < overloads.size(); ++i) {
            QMetaMethod mtd = meta->method(overloads.at(i));
            message.append(QString::fromLatin1("    %0\n").arg(QString::fromLatin1(mtd.methodSignature().constData())));
         }

         message.append(QString::fromLatin1("Use e.g. object['%0'].connect() to connect to a particular overload")
                        .arg(QLatin1String(signature)));

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
      QString message = QString::fromLatin1("Function.prototype.connect() Failed to connect to %0::%1")
                        .arg(QLatin1String(qtSignal->metaObject()->className()))
                        .arg(QLatin1String(sig.methodSignature().constData()));

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
   qDebug("%s", qPrintable(result));
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

#ifndef QT_NO_TRANSLATION

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
   if ((args.size() > 3) && !args.at(3).isString()) {
      return JSC::throwError(exec, JSC::GeneralError, "qsTranslate(): fourth argument (encoding) must be a string");
   }
   if ((args.size() > 4) && !args.at(4).isNumber()) {
      return JSC::throwError(exec, JSC::GeneralError, "qsTranslate(): fifth argument (n) must be a number");
   }

   JSC::UString context = args.at(0).toString(exec);
   JSC::UString text = args.at(1).toString(exec);
   JSC::UString comment;

   if (args.size() > 2) {
      comment = args.at(2).toString(exec);
   }

   QCoreApplication::Encoding encoding = QCoreApplication::UnicodeUTF8;
   if (args.size() > 3) {
      JSC::UString encStr = args.at(3).toString(exec);
      if (encStr == "CodecForTr") {
         encoding = QCoreApplication::CodecForTr;
      } else if (encStr == "UnicodeUTF8") {
         encoding = QCoreApplication::UnicodeUTF8;
      } else {
         return JSC::throwError(exec, JSC::GeneralError,
                                QString::fromLatin1("qsTranslate(): invalid encoding '%0'").arg(encStr));
      }
   }

   int n = -1;
   if (args.size() > 4) {
      n = args.at(4).toInt32(exec);
   }

   JSC::UString result;

   result = QCoreApplication::translate(context.UTF8String().c_str(),
                                        text.UTF8String().c_str(),
                                        comment.UTF8String().c_str(),
                                        encoding, n);

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

   result = QCoreApplication::translate(context.UTF8String().c_str(),
                                        text.UTF8String().c_str(),
                                        comment.UTF8String().c_str(),
                                        QCoreApplication::UnicodeUTF8, n);

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
#endif // QT_NO_TRANSLATION

static JSC::JSValue JSC_HOST_CALL stringProtoFuncArg(JSC::ExecState *, JSC::JSObject *, JSC::JSValue,
      const JSC::ArgList &);

JSC::JSValue JSC_HOST_CALL stringProtoFuncArg(JSC::ExecState *exec, JSC::JSObject *, JSC::JSValue thisObject,
      const JSC::ArgList &args)
{
   QString value(thisObject.toString(exec));
   JSC::JSValue arg = (args.size() != 0) ? args.at(0) : JSC::jsUndefined();
   QString result;
   if (arg.isString()) {
      result = value.arg(arg.toString(exec));
   } else if (arg.isNumber()) {
      result = value.arg(arg.toNumber(exec));
   }
   return JSC::jsString(exec, result);
}

static QScriptValue __setupPackage__(QScriptContext *ctx, QScriptEngine *eng)
{
   QString path = ctx->argument(0).toString();
   QStringList components = path.split(QLatin1Char('.'));
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
   : originalGlobalObjectProxy(0), currentFrame(0),
     qobjectPrototype(0), qmetaobjectPrototype(0), variantPrototype(0),
     activeAgent(0), agentLineNumber(-1),
     registeredScriptValues(0), freeScriptValues(0), freeScriptValuesCount(0),
     registeredScriptStrings(0), processEventsInterval(-1), inEval(false)
{
   qMetaTypeId<QScriptValue>();
   qMetaTypeId<QList<int> >();
   qMetaTypeId<QObjectList>();


   if (!QCoreApplication::instance()) {
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
      qFree(p);
   }
}

QVariant QScriptEnginePrivate::jscValueToVariant(JSC::ExecState *exec, JSC::JSValue value, int targetType)
{
   QVariant v(targetType, (void *)0);

   if (convertValue(exec, value, targetType, v.data())) {
      return v;
   }

   if (uint(targetType) == QVariant::LastType) {
      return toVariant(exec, value);
   }

   if (isVariant(value)) {
      v = variantValue(value);

      if (v.canConvert(QVariant::Type(targetType))) {
         v.convert(QVariant::Type(targetType));
         return v;
      }

      QByteArray typeName = v.typeName();
      if (typeName.endsWith('*') && (QMetaType::type(typeName.left(typeName.size() - 1).constData()) == targetType)) {
         return QVariant(targetType, *reinterpret_cast<void **>(v.data()));
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
      glob->customGlobalObject = 0;
      // Sync the internal prototype, since JSObject::prototype() is not virtual.
      glob->setPrototype(originalGlobalObjectProxy->prototype());
   } else {
      Q_ASSERT(object != originalGlobalObject());
      glob->customGlobalObject = object;
      // Sync the internal prototype, since JSObject::prototype() is not virtual.
      glob->setPrototype(object->prototype());
   }
}

/*!
  \internal

  If the given \a value is the original global object, returns the custom
  global object or a proxy to the original global object; otherwise returns \a
  value.
*/
JSC::JSValue QScriptEnginePrivate::toUsableValue(JSC::JSValue value)
{
   if (!value || !value.isObject() || !JSC::asObject(value)->isGlobalObject()) {
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
/*!
    \internal
    Return the 'this' value for a given context
*/
JSC::JSValue QScriptEnginePrivate::thisForContext(JSC::ExecState *frame)
{
   if (frame->codeBlock() != 0) {
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
   Q_ASSERT(frame->codeBlock() == 0); // only for native calls
   return frame->registers() - JSC::RegisterFile::CallFrameHeaderSize - frame->argumentCount();
}

/*! \internal
     For native context, we use the ReturnValueRegister entry in the stackframe header to store flags.
     We can do that because this header is not used as the native function return their value thought C++

     when setting flags, NativeContext should always be set

     contextFlags returns 0 for non native context
 */
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
      for (it = registeredScriptValues; it != 0; it = it->next) {
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


   markStack.drain(); // make sure everything is marked before marking qobject data

   {
      QHash<QObject *, QScript::QObjectData *>::const_iterator it;
      for (it = m_qobjectData.constBegin(); it != m_qobjectData.constEnd(); ++it) {
         QScript::QObjectData *qdata = it.value();
         qdata->mark(markStack);
      }
   }
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
      activeAgent = 0;
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
   QScriptObject *result = 0;

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

   /*if (setDefaultPrototype)*/
   {
      const QMetaObject *meta = object->metaObject();

      while (meta) {
         QByteArray typeString = meta->className();
         typeString.append('*');

         int typeId = QMetaType::type(typeString.constData());

         if (typeId != 0) {
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
   if (!metaObject) {
      return JSC::jsNull();
   }
   JSC::ExecState *exec = currentFrame;
   QScript::QMetaObjectWrapperObject *result = new (exec) QScript::QMetaObjectWrapperObject(exec, metaObject, ctor,
         qmetaobjectWrapperObjectStructure);
   return result;
}

bool QScriptEnginePrivate::convertToNativeQObject(JSC::ExecState *exec, JSC::JSValue value,
      const QByteArray &targetType, void **result)
{
   if (!targetType.endsWith('*')) {
      return false;
   }
   if (QObject *qobject = toQObject(exec, value)) {
      int start = targetType.startsWith("const ") ? 6 : 0;
      QByteArray className = targetType.mid(start, targetType.size() - start - 1);

      /*  BROOM (script)
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
   /*    if (isCollecting()) {
           // wait until we're done with GC before deleting it
           int index = m_qobjectsToBeDeleted.indexOf(object);
           if (index == -1)
               m_qobjectsToBeDeleted.append(object);
               } else*/ {
      delete object;
   }
}

void QScriptEnginePrivate::emitSignalHandlerException()
{
   Q_Q(QScriptEngine);
   emit q->signalHandlerException(q->uncaughtException());
}

bool QScriptEnginePrivate::scriptConnect(QObject *sender, const char *signal,
      JSC::JSValue receiver, JSC::JSValue function,
      Qt::ConnectionType type)
{
   Q_ASSERT(sender);
   Q_ASSERT(signal);

   const QMetaObject *meta = sender->metaObject();
   int index = meta->indexOfSignal(QMetaObject::normalizedSignature(signal + 1).constData());

   if (index == -1) {
      return false;
   }

   return scriptConnect(sender, index, receiver, function, /*wrapper=*/JSC::JSValue(), type);
}

bool QScriptEnginePrivate::scriptDisconnect(QObject *sender, const char *signal,
      JSC::JSValue receiver, JSC::JSValue function)
{
   Q_ASSERT(sender);
   Q_ASSERT(signal);

   const QMetaObject *meta = sender->metaObject();
   int index = meta->indexOfSignal(QMetaObject::normalizedSignature(signal + 1).constData());

   if (index == -1) {
      return false;
   }

   return scriptDisconnect(sender, index, receiver, function);
}

bool QScriptEnginePrivate::scriptConnect(QObject *sender, int signalIndex,
      JSC::JSValue receiver, JSC::JSValue function,
      JSC::JSValue senderWrapper,
      Qt::ConnectionType type)
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
   for (it = registeredScriptValues; it != 0; it = next) {
      it->detachFromEngine();
      next = it->next;
      it->prev = 0;
      it->next = 0;
   }
   registeredScriptValues = 0;
}

void QScriptEnginePrivate::detachAllRegisteredScriptStrings()
{
   QScriptStringPrivate *it;
   QScriptStringPrivate *next;
   for (it = registeredScriptStrings; it != 0; it = next) {
      it->detachFromEngine();
      next = it->next;
      it->prev = 0;
      it->next = 0;
   }
   registeredScriptStrings = 0;
}

#ifndef QT_NO_REGEXP

Q_CORE_EXPORT QString qt_regexp_toCanonical(const QString &, QRegExp::PatternSyntax);

JSC::JSValue QScriptEnginePrivate::newRegExp(JSC::ExecState *exec, const QRegExp &regexp)
{
   JSC::JSValue buf[2];
   JSC::ArgList args(buf, sizeof(buf));

   //convert the pattern to a ECMAScript pattern
   QString pattern = qt_regexp_toCanonical(regexp.pattern(), regexp.patternSyntax());
   if (regexp.isMinimal()) {
      QString ecmaPattern;
      int len = pattern.length();
      ecmaPattern.reserve(len);
      int i = 0;
      const QChar *wc = pattern.unicode();
      bool inBracket = false;
      while (i < len) {
         QChar c = wc[i++];
         ecmaPattern += c;
         switch (c.unicode()) {
            case '?':
            case '+':
            case '*':
            case '}':
               if (!inBracket) {
                  ecmaPattern += QLatin1Char('?');
               }
               break;
            case '\\':
               if (i < len) {
                  ecmaPattern += wc[i++];
               }
               break;
            case '[':
               inBracket = true;
               break;
            case ']':
               inBracket = false;
               break;
            default:
               break;
         }
      }
      pattern = ecmaPattern;
   }

   JSC::UString jscPattern = pattern;
   QString flags;
   if (regexp.caseSensitivity() == Qt::CaseInsensitive) {
      flags.append(QLatin1Char('i'));
   }
   JSC::UString jscFlags = flags;
   buf[0] = JSC::jsString(exec, jscPattern);
   buf[1] = JSC::jsString(exec, jscFlags);
   return JSC::constructRegExp(exec, args);
}

#endif

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

#ifndef QT_NO_REGEXP

QRegExp QScriptEnginePrivate::toRegExp(JSC::ExecState *exec, JSC::JSValue value)
{
   if (!isRegExp(value)) {
      return QRegExp();
   }
   QString pattern = toString(exec, property(exec, value, "source", QScriptValue::ResolvePrototype));
   Qt::CaseSensitivity kase = Qt::CaseSensitive;
   if (toBool(exec, property(exec, value, "ignoreCase", QScriptValue::ResolvePrototype))) {
      kase = Qt::CaseInsensitive;
   }
   return QRegExp(pattern, kase, QRegExp::RegExp2);
}

#endif

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

#ifndef QT_NO_REGEXP
      else if (isRegExp(value)) {
         return QVariant(toRegExp(exec, value));
      }
#endif
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
                  "property '%s' has a getter but no setter",
                  qPrintable(QString(id.ustring())));
         return;
      }
      if (!value) {
         // ### check if it's a getter/setter property
         thisObject->deleteProperty(exec, id);
      } else if (flags != QScriptValue::KeepExistingFlags) {
         if (thisObject->hasOwnProperty(exec, id)) {
            thisObject->deleteProperty(exec, id);   // ### hmmm - can't we just update the attributes?
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
   if (!value) {
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
      const JSC::Identifier &id,
      const QScriptValue::ResolveFlags &mode)
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
      return 0;
   }
   QScriptValue::PropertyFlags result = 0;
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


/*!
    Constructs a QScriptEngine object.

    The globalObject() is initialized to have properties as described in
    \l{ECMA-262}, Section 15.1.
*/
QScriptEngine::QScriptEngine()
   : QObject(0), d_ptr(new QScriptEnginePrivate)
{
   d_ptr->q_ptr = this;
}

/*!
    Constructs a QScriptEngine object with the given \a parent.

    The globalObject() is initialized to have properties as described in
    \l{ECMA-262}, Section 15.1.
*/

QScriptEngine::QScriptEngine(QObject *parent)
   : QObject(parent), d_ptr(new QScriptEnginePrivate)
{
   d_ptr->q_ptr = this;
}

/*! \internal
*/
QScriptEngine::QScriptEngine(QScriptEnginePrivate &dd, QObject *parent)
   : QObject(parent), d_ptr(&dd)
{
}

/*!
  Destroys this QScriptEngine.
*/
QScriptEngine::~QScriptEngine()
{
}

/*!
  Returns this engine's Global Object.

  By default, the Global Object contains the built-in objects that are
  part of \l{ECMA-262}, such as Math, Date and String. Additionally,
  you can set properties of the Global Object to make your own
  extensions available to all script code. Non-local variables in
  script code will be created as properties of the Global Object, as
  well as local variables in global code.
*/
QScriptValue QScriptEngine::globalObject() const
{
   Q_D(const QScriptEngine);
   QScript::APIShim shim(const_cast<QScriptEnginePrivate *>(d));
   JSC::JSObject *result = d->globalObject();
   return const_cast<QScriptEnginePrivate *>(d)->scriptValueFromJSCValue(result);
}

/*!
  \since 4.5

  Sets this engine's Global Object to be the given \a object.
  If \a object is not a valid script object, this function does
  nothing.

  When setting a custom global object, you may want to use
  QScriptValueIterator to copy the properties of the standard Global
  Object; alternatively, you can set the internal prototype of your
  custom object to be the original Global Object.
*/
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

/*!
  Returns a QScriptValue of the primitive type Null.

  \sa undefinedValue()
*/
QScriptValue QScriptEngine::nullValue()
{
   Q_D(QScriptEngine);
   return d->scriptValueFromJSCValue(JSC::jsNull());
}

/*!
  Returns a QScriptValue of the primitive type Undefined.

  \sa nullValue()
*/
QScriptValue QScriptEngine::undefinedValue()
{
   Q_D(QScriptEngine);
   return d->scriptValueFromJSCValue(JSC::jsUndefined());
}

/*!
  Creates a constructor function from \a fun, with the given \a length.
  The \c{prototype} property of the resulting function is set to be the
  given \a prototype. The \c{constructor} property of \a prototype is
  set to be the resulting function.

  When a function is called as a constructor (e.g. \c{new Foo()}), the
  `this' object associated with the function call is the new object
  that the function is expected to initialize; the prototype of this
  default constructed object will be the function's public
  \c{prototype} property. If you always want the function to behave as
  a constructor (e.g. \c{Foo()} should also create a new object), or
  if you need to create your own object rather than using the default
  `this' object, you should make sure that the prototype of your
  object is set correctly; either by setting it manually, or, when
  wrapping a custom type, by having registered the defaultPrototype()
  of that type. Example:

  \snippet doc/src/snippets/code/src_script_qscriptengine.cpp 9

  To wrap a custom type and provide a constructor for it, you'd typically
  do something like this:

  \snippet doc/src/snippets/code/src_script_qscriptengine.cpp 10
*/
QScriptValue QScriptEngine::newFunction(QScriptEngine::FunctionSignature fun,
                                        const QScriptValue &prototype,
                                        int length)
{
   Q_D(QScriptEngine);
   QScript::APIShim shim(d);
   JSC::ExecState *exec = d->currentFrame;
   JSC::JSValue function = new (exec)QScript::FunctionWrapper(exec, length, JSC::Identifier(exec, ""), fun);
   QScriptValue result = d->scriptValueFromJSCValue(function);
   result.setProperty(QLatin1String("prototype"), prototype,
                      QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
   const_cast<QScriptValue &>(prototype)
   .setProperty(QLatin1String("constructor"), result, QScriptValue::SkipInEnumeration);
   return result;
}

#ifndef QT_NO_REGEXP

/*!
  Creates a QtScript object of class RegExp with the given
  \a regexp.

  \sa QScriptValue::toRegExp()
*/
QScriptValue QScriptEngine::newRegExp(const QRegExp &regexp)
{
   Q_D(QScriptEngine);
   QScript::APIShim shim(d);
   return d->scriptValueFromJSCValue(d->newRegExp(d->currentFrame, regexp));
}

#endif // QT_NO_REGEXP

/*!
  Creates a QtScript object holding the given variant \a value.

  If a default prototype has been registered with the meta type id of
  \a value, then the prototype of the created object will be that
  prototype; otherwise, the prototype will be the Object prototype
  object.

  \sa setDefaultPrototype(), QScriptValue::toVariant(), reportAdditionalMemoryCost()
*/
QScriptValue QScriptEngine::newVariant(const QVariant &value)
{
   Q_D(QScriptEngine);
   QScript::APIShim shim(d);
   return d->scriptValueFromJSCValue(d->newVariant(value));
}

/*!
  \since 4.4
  \overload

  Initializes the given Qt Script \a object to hold the given variant
  \a value, and returns the \a object.

  This function enables you to "promote" a plain Qt Script object
  (created by the newObject() function) to a variant, or to replace
  the variant contained inside an object previously created by the
  newVariant() function.

  The prototype() of the \a object will remain unchanged.

  If \a object is not an object, this function behaves like the normal
  newVariant(), i.e. it creates a new script object and returns it.

  This function is useful when you want to provide a script
  constructor for a C++ type. If your constructor is invoked in a
  \c{new} expression (QScriptContext::isCalledAsConstructor() returns
  true), you can pass QScriptContext::thisObject() (the default
  constructed script object) to this function to initialize the new
  object.

  \sa reportAdditionalMemoryCost()
*/
QScriptValue QScriptEngine::newVariant(const QScriptValue &object,
                                       const QVariant &value)
{
   Q_D(QScriptEngine);
   QScript::APIShim shim(d);
   JSC::JSValue jsObject = d->scriptValueToJSCValue(object);
   return d->scriptValueFromJSCValue(d->newVariant(jsObject, value));
}

/*!
  Creates a QtScript object that wraps the given QObject \a
  object, using the given \a ownership. The given \a options control
  various aspects of the interaction with the resulting script object.

  Signals and slots, properties and children of \a object are
  available as properties of the created QScriptValue. For more
  information, see the \l{QtScript} documentation.

  If \a object is a null pointer, this function returns nullValue().

  If a default prototype has been registered for the \a object's class
  (or its superclass, recursively), the prototype of the new script
  object will be set to be that default prototype.

  If the given \a object is deleted outside of QtScript's control, any
  attempt to access the deleted QObject's members through the QtScript
  wrapper object (either by script code or C++) will result in a
  script exception.

  \sa QScriptValue::toQObject(), reportAdditionalMemoryCost()
*/
QScriptValue QScriptEngine::newQObject(QObject *object, ValueOwnership ownership,
                                       const QObjectWrapOptions &options)
{
   Q_D(QScriptEngine);
   QScript::APIShim shim(d);
   JSC::JSValue jscQObject = d->newQObject(object, ownership, options);
   return d->scriptValueFromJSCValue(jscQObject);
}

/*!
  \since 4.4
  \overload

  Initializes the given \a scriptObject to hold the given \a qtObject,
  and returns the \a scriptObject.

  This function enables you to "promote" a plain Qt Script object
  (created by the newObject() function) to a QObject proxy, or to
  replace the QObject contained inside an object previously created by
  the newQObject() function.

  The prototype() of the \a scriptObject will remain unchanged.

  If \a scriptObject is not an object, this function behaves like the
  normal newQObject(), i.e. it creates a new script object and returns
  it.

  This function is useful when you want to provide a script
  constructor for a QObject-based class. If your constructor is
  invoked in a \c{new} expression
  (QScriptContext::isCalledAsConstructor() returns true), you can pass
  QScriptContext::thisObject() (the default constructed script object)
  to this function to initialize the new object.

  \sa reportAdditionalMemoryCost()
*/
QScriptValue QScriptEngine::newQObject(const QScriptValue &scriptObject,
                                       QObject *qtObject,
                                       ValueOwnership ownership,
                                       const QObjectWrapOptions &options)
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

/*!
  Creates a QtScript object of class Object.

  The prototype of the created object will be the Object
  prototype object.

  \sa newArray(), QScriptValue::setProperty()
*/
QScriptValue QScriptEngine::newObject()
{
   Q_D(QScriptEngine);
   QScript::APIShim shim(d);
   return d->scriptValueFromJSCValue(d->newObject());
}

/*!
  \since 4.4
  \overload

  Creates a QtScript Object of the given class, \a scriptClass.

  The prototype of the created object will be the Object
  prototype object.

  \a data, if specified, is set as the internal data of the
  new object (using QScriptValue::setData()).

  \sa QScriptValue::scriptClass(), reportAdditionalMemoryCost()
*/
QScriptValue QScriptEngine::newObject(QScriptClass *scriptClass,
                                      const QScriptValue &data)
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

/*!
  \internal
*/
QScriptValue QScriptEngine::newActivationObject()
{
   qWarning("QScriptEngine::newActivationObject() not implemented");
   // ### JSActivation or JSVariableObject?
   return QScriptValue();
}

/*!
  Creates a QScriptValue that wraps a native (C++) function. \a fun
  must be a C++ function with signature QScriptEngine::FunctionSignature.  \a
  length is the number of arguments that \a fun expects; this becomes
  the \c{length} property of the created QScriptValue.

  Note that \a length only gives an indication of the number of
  arguments that the function expects; an actual invocation of a
  function can include any number of arguments. You can check the
  \l{QScriptContext::argumentCount()}{argumentCount()} of the
  QScriptContext associated with the invocation to determine the
  actual number of arguments passed.

  A \c{prototype} property is automatically created for the resulting
  function object, to provide for the possibility that the function
  will be used as a constructor.

  By combining newFunction() and the property flags
  QScriptValue::PropertyGetter and QScriptValue::PropertySetter, you
  can create script object properties that behave like normal
  properties in script code, but are in fact accessed through
  functions (analogous to how properties work in \l{Qt's Property
  System}). Example:

  \snippet doc/src/snippets/code/src_script_qscriptengine.cpp 11

  When the property \c{foo} of the script object is subsequently
  accessed in script code, \c{getSetFoo()} will be invoked to handle
  the access.  In this particular case, we chose to store the "real"
  value of \c{foo} as a property of the accessor function itself; you
  are of course free to do whatever you like in this function.

  In the above example, a single native function was used to handle
  both reads and writes to the property; the argument count is used to
  determine if we are handling a read or write. You can also use two
  separate functions; just specify the relevant flag
  (QScriptValue::PropertyGetter or QScriptValue::PropertySetter) when
  setting the property, e.g.:

  \snippet doc/src/snippets/code/src_script_qscriptengine.cpp 12

  \sa QScriptValue::call()
*/
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

/*!
  \internal
  \since 4.4
*/
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

/*!
  Creates a QtScript object of class Array with the given \a length.

  \sa newObject()
*/
QScriptValue QScriptEngine::newArray(uint length)
{
   Q_D(QScriptEngine);
   QScript::APIShim shim(d);
   return d->scriptValueFromJSCValue(d->newArray(d->currentFrame, length));
}

/*!
  Creates a QtScript object of class RegExp with the given
  \a pattern and \a flags.

  The legal flags are 'g' (global), 'i' (ignore case), and 'm'
  (multiline).
*/
QScriptValue QScriptEngine::newRegExp(const QString &pattern, const QString &flags)
{
   Q_D(QScriptEngine);
   QScript::APIShim shim(d);
   return d->scriptValueFromJSCValue(d->newRegExp(d->currentFrame, pattern, flags));
}

/*!
  Creates a QtScript object of class Date with the given
  \a value (the number of milliseconds since 01 January 1970,
  UTC).
*/
QScriptValue QScriptEngine::newDate(qsreal value)
{
   Q_D(QScriptEngine);
   QScript::APIShim shim(d);
   return d->scriptValueFromJSCValue(d->newDate(d->currentFrame, value));
}

/*!
  Creates a QtScript object of class Date from the given \a value.

  \sa QScriptValue::toDateTime()
*/
QScriptValue QScriptEngine::newDate(const QDateTime &value)
{
   Q_D(QScriptEngine);
   QScript::APIShim shim(d);
   return d->scriptValueFromJSCValue(d->newDate(d->currentFrame, value));
}


/*!
  Creates a QtScript object that represents a QObject class, using the
  the given \a metaObject and constructor \a ctor.

  Enums of \a metaObject (declared with Q_ENUMS) are available as
  properties of the created QScriptValue. When the class is called as
  a function, \a ctor will be called to create a new instance of the
  class.

  Example:

  \snippet doc/src/snippets/code/src_script_qscriptengine.cpp 27

  \sa newQObject(), scriptValueFromQMetaObject()
*/
QScriptValue QScriptEngine::newQMetaObject(
   const QMetaObject *metaObject, const QScriptValue &ctor)
{
   Q_D(QScriptEngine);
   QScript::APIShim shim(d);
   JSC::JSValue jscCtor = d->scriptValueToJSCValue(ctor);
   JSC::JSValue jscQMetaObject = d->newQMetaObject(metaObject, jscCtor);
   return d->scriptValueFromJSCValue(jscQMetaObject);
}

/*!
  \fn QScriptValue QScriptEngine::scriptValueFromQMetaObject()

  Creates a QScriptValue that represents the Qt class \c{T}.

  This function is used in combination with one of the
  Q_SCRIPT_DECLARE_QMETAOBJECT() macro. Example:

  \snippet doc/src/snippets/code/src_script_qscriptengine.cpp 13

  \sa QScriptEngine::newQMetaObject()
*/

/*!
  \fn QScriptValue qScriptValueFromQMetaObject(QScriptEngine *engine)
  \since 4.3
  \relates QScriptEngine
  \obsolete

  Uses \a engine to create a QScriptValue that represents the Qt class
  \c{T}.

  This function is equivalent to
  QScriptEngine::scriptValueFromQMetaObject().

  \note This function was provided as a workaround for MSVC 6
  which did not support member template functions. It is advised
  to use the other form in new code.

  \sa QScriptEngine::newQMetaObject()
*/

/*!
  \obsolete

  Returns true if \a program can be evaluated; i.e. the code is
  sufficient to determine whether it appears to be a syntactically
  correct program, or contains a syntax error.

  This function returns false if \a program is incomplete; i.e. the
  input is syntactically correct up to the point where the input is
  terminated.

  Note that this function only does a static check of \a program;
  e.g. it does not check whether references to variables are
  valid, and so on.

  A typical usage of canEvaluate() is to implement an interactive
  interpreter for QtScript. The user is repeatedly queried for
  individual lines of code; the lines are concatened internally, and
  only when canEvaluate() returns true for the resulting program is it
  passed to evaluate().

  The following are some examples to illustrate the behavior of
  canEvaluate(). (Note that all example inputs are assumed to have an
  explicit newline as their last character, since otherwise the
  QtScript parser would automatically insert a semi-colon character at
  the end of the input, and this could cause canEvaluate() to produce
  different results.)

  Given the input
  \snippet doc/src/snippets/code/src_script_qscriptengine.cpp 14
  canEvaluate() will return true, since the program appears to be complete.

  Given the input
  \snippet doc/src/snippets/code/src_script_qscriptengine.cpp 15
  canEvaluate() will return false, since the if-statement is not complete,
  but is syntactically correct so far.

  Given the input
  \snippet doc/src/snippets/code/src_script_qscriptengine.cpp 16
  canEvaluate() will return true, but evaluate() will throw a
  SyntaxError given the same input.

  Given the input
  \snippet doc/src/snippets/code/src_script_qscriptengine.cpp 17
  canEvaluate() will return true, even though the code is clearly not
  syntactically valid QtScript code. evaluate() will throw a
  SyntaxError when this code is evaluated.

  Given the input
  \snippet doc/src/snippets/code/src_script_qscriptengine.cpp 18
  canEvaluate() will return true, but evaluate() will throw a
  ReferenceError if \c{foo} is not defined in the script
  environment.

  \sa evaluate(), checkSyntax()
*/
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

/*!
  \since 4.5

  Checks the syntax of the given \a program. Returns a
  QScriptSyntaxCheckResult object that contains the result of the check.
*/
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



/*!
  Evaluates \a program, using \a lineNumber as the base line number,
  and returns the result of the evaluation.

  The script code will be evaluated in the current context.

  The evaluation of \a program can cause an exception in the
  engine; in this case the return value will be the exception
  that was thrown (typically an \c{Error} object). You can call
  hasUncaughtException() to determine if an exception occurred in
  the last call to evaluate().

  \a lineNumber is used to specify a starting line number for \a
  program; line number information reported by the engine that pertain
  to this evaluation (e.g. uncaughtExceptionLineNumber()) will be
  based on this argument. For example, if \a program consists of two
  lines of code, and the statement on the second line causes a script
  exception, uncaughtExceptionLineNumber() would return the given \a
  lineNumber plus one. When no starting line number is specified, line
  numbers will be 1-based.

  \a fileName is used for error reporting. For example in error objects
  the file name is accessible through the "fileName" property if it's
  provided with this function.

  \sa canEvaluate(), hasUncaughtException(), isEvaluating(), abortEvaluation()
*/

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

/*!
  \since 4.7

  Evaluates the given \a program and returns the result of the
  evaluation.
*/
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

/*!
  Returns the current context.

  The current context is typically accessed to retrieve the arguments
  and `this' object in native functions; for convenience, it is
  available as the first argument in QScriptEngine::FunctionSignature.
*/
QScriptContext *QScriptEngine::currentContext() const
{
   Q_D(const QScriptEngine);
   return const_cast<QScriptEnginePrivate *>(d)->contextForFrame(d->currentFrame);
}

/*!
  Enters a new execution context and returns the associated
  QScriptContext object.

  Once you are done with the context, you should call popContext() to
  restore the old context.

  By default, the `this' object of the new context is the Global Object.
  The context's \l{QScriptContext::callee()}{callee}() will be invalid.

  This function is useful when you want to evaluate script code
  as if it were the body of a function. You can use the context's
  \l{QScriptContext::activationObject()}{activationObject}() to initialize
  local variables that will be available to scripts. Example:

  \snippet doc/src/snippets/code/src_script_qscriptengine.cpp 19

  In the above example, the new variable "tmp" defined in the script
  will be local to the context; in other words, the script doesn't
  have any effect on the global environment.

  Returns 0 in case of stack overflow

  \sa popContext()
*/
QScriptContext *QScriptEngine::pushContext()
{
   Q_D(QScriptEngine);
   QScript::APIShim shim(d);

   JSC::CallFrame *newFrame = d->pushContext(d->currentFrame, d->currentFrame->globalData().dynamicGlobalObject,
                              JSC::ArgList(), /*callee = */0);

   if (agent()) {
      agent()->contextPush();
   }

   return d->contextForFrame(newFrame);
}

/*! \internal
   push a context for a native function.
   JSC native function doesn't have different stackframe or context. so we need to create one.

   use popContext right after to go back to the previous context the context if no stack overflow has hapenned

   exec is the current top frame.

   return the new top frame. (might be the same as exec if a new stackframe was not needed) or 0 if stack overflow
*/
JSC::CallFrame *QScriptEnginePrivate::pushContext(JSC::CallFrame *exec, JSC::JSValue _thisObject,
      const JSC::ArgList &args, JSC::JSObject *callee, bool calledAsConstructor,
      bool clearScopeChain)
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
   if (callee == 0 //called from  public QScriptEngine::pushContext
         || exec->returnPC() == 0 || (contextFlags(exec) & NativeContext) //called from native-native call
         || (exec->codeBlock() && exec->callee() != callee)) { //the interpreter did not build a frame for us.
      //We need to check if the Interpreter might have already created a frame for function called from JS.
      JSC::Interpreter *interp = exec->interpreter();
      JSC::Register *oldEnd = interp->registerFile().end();
      int argc = args.size() + 1; //add "this"
      JSC::Register *newEnd = oldEnd + argc + JSC::RegisterFile::CallFrameHeaderSize;
      if (!interp->registerFile().grow(newEnd)) {
         return 0;   //### Stack overflow
      }
      newCallFrame = JSC::CallFrame::create(oldEnd);
      newCallFrame[0] = thisObject;
      int dst = 0;
      JSC::ArgList::const_iterator it;
      for (it = args.begin(); it != args.end(); ++it) {
         newCallFrame[++dst] = *it;
      }
      newCallFrame += argc + JSC::RegisterFile::CallFrameHeaderSize;

      if (!clearScopeChain) {
         newCallFrame->init(0, /*vPC=*/0, exec->scopeChain(), exec, flags | ShouldRestoreCallFrame, argc, callee);
      } else {
         newCallFrame->init(0, /*vPC=*/0, globalExec()->scopeChain(), exec, flags | ShouldRestoreCallFrame, argc, callee);
      }
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


/*!
  Pops the current execution context and restores the previous one.
  This function must be used in conjunction with pushContext().

  \sa pushContext()
*/
void QScriptEngine::popContext()
{
   if (agent()) {
      agent()->contextPop();
   }
   Q_D(QScriptEngine);
   QScript::APIShim shim(d);
   if (d->currentFrame->returnPC() != 0 || d->currentFrame->codeBlock() != 0
         || !currentContext()->parentContext()) {
      qWarning("QScriptEngine::popContext() doesn't match with pushContext()");
      return;
   }

   d->popContext();
}

/*! \internal
    counter part of QScriptEnginePrivate::pushContext
 */
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

/*!
  Returns true if the last script evaluation resulted in an uncaught
  exception; otherwise returns false.

  The exception state is cleared when evaluate() is called.

  \sa uncaughtException(), uncaughtExceptionLineNumber()
*/
bool QScriptEngine::hasUncaughtException() const
{
   Q_D(const QScriptEngine);
   JSC::ExecState *exec = d->globalExec();
   return exec->hadException() || d->currentException().isValid();
}

/*!
  Returns the current uncaught exception, or an invalid QScriptValue
  if there is no uncaught exception.

  The exception value is typically an \c{Error} object; in that case,
  you can call toString() on the return value to obtain an error
  message.

  \sa hasUncaughtException(), uncaughtExceptionLineNumber(),
*/
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

/*!
  Returns the line number where the last uncaught exception occurred.

  Line numbers are 1-based, unless a different base was specified as
  the second argument to evaluate().

  \sa hasUncaughtException()
*/
int QScriptEngine::uncaughtExceptionLineNumber() const
{
   if (!hasUncaughtException()) {
      return -1;
   }
   return uncaughtException().property(QLatin1String("lineNumber")).toInt32();
}

/*!
  Returns a human-readable backtrace of the last uncaught exception.

  It is in the form \c{<function-name>()@<file-name>:<line-number>}.

  \sa uncaughtException()
*/
QStringList QScriptEngine::uncaughtExceptionBacktrace() const
{
   if (!hasUncaughtException()) {
      return QStringList();
   }
   // ### currently no way to get a full backtrace from JSC without installing a
   // debugger that reimplements exception() and store the backtrace there.
   QScriptValue value = uncaughtException();
   if (!value.isError()) {
      return QStringList();
   }
   QStringList result;
   result.append(QString::fromLatin1("<anonymous>()@%0:%1")
                 .arg(value.property(QLatin1String("fileName")).toString())
                 .arg(value.property(QLatin1String("lineNumber")).toInt32()));
   return result;
}

/*!
  \since 4.4

  Clears any uncaught exceptions in this engine.

  \sa hasUncaughtException()
*/
void QScriptEngine::clearExceptions()
{
   Q_D(QScriptEngine);
   JSC::ExecState *exec = d->currentFrame;
   exec->clearException();
   d->clearCurrentException();
}

/*!
  Returns the default prototype associated with the given \a metaTypeId,
  or an invalid QScriptValue if no default prototype has been set.

  \sa setDefaultPrototype()
*/
QScriptValue QScriptEngine::defaultPrototype(int metaTypeId) const
{
   Q_D(const QScriptEngine);
   return const_cast<QScriptEnginePrivate *>(d)->scriptValueFromJSCValue(d->defaultPrototype(metaTypeId));
}

/*!
  Sets the default prototype of the C++ type identified by the given
  \a metaTypeId to \a prototype.

  The default prototype provides a script interface for values of
  type \a metaTypeId when a value of that type is accessed from script
  code.  Whenever the script engine (implicitly or explicitly) creates
  a QScriptValue from a value of type \a metaTypeId, the default
  prototype will be set as the QScriptValue's prototype.

  The \a prototype object itself may be constructed using one of two
  principal techniques; the simplest is to subclass QScriptable, which
  enables you to define the scripting API of the type through QObject
  properties and slots.  Another possibility is to create a script
  object by calling newObject(), and populate the object with the
  desired properties (e.g. native functions wrapped with
  newFunction()).

  \sa defaultPrototype(), qScriptRegisterMetaType(), QScriptable, {Default Prototypes Example}
*/
void QScriptEngine::setDefaultPrototype(int metaTypeId, const QScriptValue &prototype)
{
   Q_D(QScriptEngine);
   d->setDefaultPrototype(metaTypeId, d->scriptValueToJSCValue(prototype));
}

/*!
  \typedef QScriptEngine::FunctionSignature
  \relates QScriptEngine

  The function signature \c{QScriptValue f(QScriptContext *, QScriptEngine *)}.

  A function with such a signature can be passed to
  QScriptEngine::newFunction() to wrap the function.
*/

/*!
  \typedef QScriptEngine::FunctionWithArgSignature
  \relates QScriptEngine

  The function signature \c{QScriptValue f(QScriptContext *, QScriptEngine *, void *)}.

  A function with such a signature can be passed to
  QScriptEngine::newFunction() to wrap the function.
*/

/*!
    \typedef QScriptEngine::MarshalFunction
    \internal
*/

/*!
    \typedef QScriptEngine::DemarshalFunction
    \internal
*/

/*!
    \internal
*/
QScriptValue QScriptEngine::create(int type, const void *ptr)
{
   Q_D(QScriptEngine);
   QScript::APIShim shim(d);
   return d->scriptValueFromJSCValue(d->create(d->currentFrame, type, ptr));
}

JSC::JSValue QScriptEnginePrivate::create(JSC::ExecState *exec, int type, const void *ptr)
{
   Q_ASSERT(ptr != 0);
   JSC::JSValue result;
   QScriptEnginePrivate *eng = exec ? QScript::scriptEngineFromExec(exec) : 0;
   QScriptTypeInfo *info = eng ? eng->m_typeInfos.value(type) : 0;
   if (info && info->marshal) {
      result = eng->scriptValueToJSCValue(info->marshal(eng->q_func(), ptr));
   } else {
      // check if it's one of the types we know
      switch (QMetaType::Type(type)) {
         case QMetaType::Void:
            return JSC::jsUndefined();
         case QMetaType::Bool:
            return JSC::jsBoolean(*reinterpret_cast<const bool *>(ptr));
         case QMetaType::Int:
            return JSC::jsNumber(exec, *reinterpret_cast<const int *>(ptr));
         case QMetaType::UInt:
            return JSC::jsNumber(exec, *reinterpret_cast<const uint *>(ptr));
         case QMetaType::LongLong:
            return JSC::jsNumber(exec, qsreal(*reinterpret_cast<const qint64 *>(ptr)));
         case QMetaType::ULongLong:
            return JSC::jsNumber(exec, qsreal(*reinterpret_cast<const quint64 *>(ptr)));
         case QMetaType::Double:
            return JSC::jsNumber(exec, qsreal(*reinterpret_cast<const double *>(ptr)));
         case QMetaType::QString:
            return JSC::jsString(exec, *reinterpret_cast<const QString *>(ptr));
         case QMetaType::Float:
            return JSC::jsNumber(exec, *reinterpret_cast<const float *>(ptr));
         case QMetaType::Short:
            return JSC::jsNumber(exec, *reinterpret_cast<const short *>(ptr));
         case QMetaType::UShort:
            return JSC::jsNumber(exec, *reinterpret_cast<const unsigned short *>(ptr));
         case QMetaType::Char:
            return JSC::jsNumber(exec, *reinterpret_cast<const char *>(ptr));
         case QMetaType::UChar:
            return JSC::jsNumber(exec, *reinterpret_cast<const unsigned char *>(ptr));
         case QMetaType::QChar:
            return JSC::jsNumber(exec, (*reinterpret_cast<const QChar *>(ptr)).unicode());
         case QMetaType::QStringList:
            result = arrayFromStringList(exec, *reinterpret_cast<const QStringList *>(ptr));
            break;
         case QMetaType::QVariantList:
            result = arrayFromVariantList(exec, *reinterpret_cast<const QVariantList *>(ptr));
            break;
         case QMetaType::QVariantMap:
            result = objectFromVariantMap(exec, *reinterpret_cast<const QVariantMap *>(ptr));
            break;
         case QMetaType::QDateTime:
            result = newDate(exec, *reinterpret_cast<const QDateTime *>(ptr));
            break;
         case QMetaType::QDate:
            result = newDate(exec, QDateTime(*reinterpret_cast<const QDate *>(ptr)));
            break;
#ifndef QT_NO_REGEXP
         case QMetaType::QRegExp:
            result = newRegExp(exec, *reinterpret_cast<const QRegExp *>(ptr));
            break;
#endif

         case QMetaType::QObjectStar:
         case QMetaType::QWidgetStar:
            result = eng->newQObject(*reinterpret_cast<QObject *const *>(ptr));
            break;

         case QMetaType::QVariant:
            result = eng->newVariant(*reinterpret_cast<const QVariant *>(ptr));
            break;

         default:
            if (type == qMetaTypeId<QScriptValue>()) {
               result = eng->scriptValueToJSCValue(*reinterpret_cast<const QScriptValue *>(ptr));
               if (!result) {
                  return JSC::jsUndefined();
               }
            }


            // lazy registration of some common list types
            else if (type == qMetaTypeId<QObjectList>()) {
               qScriptRegisterSequenceMetaType<QObjectList>(eng->q_func());
               return create(exec, type, ptr);
            }

            else if (type == qMetaTypeId<QList<int> >()) {
               qScriptRegisterSequenceMetaType<QList<int> >(eng->q_func());
               return create(exec, type, ptr);
            }

            else {
               QByteArray typeName = QMetaType::typeName(type);
               if (typeName.endsWith('*') && !*reinterpret_cast<void *const *>(ptr)) {
                  return JSC::jsNull();
               } else {
                  result = eng->newVariant(QVariant(type, ptr));
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

bool QScriptEnginePrivate::convertValue(JSC::ExecState *exec, JSC::JSValue value,
                                        int type, void *ptr)
{
   QScriptEnginePrivate *eng = exec ? QScript::scriptEngineFromExec(exec) : 0;
   if (eng) {
      QScriptTypeInfo *info = eng->m_typeInfos.value(type);
      if (info && info->demarshal) {
         info->demarshal(eng->scriptValueFromJSCValue(value), ptr);
         return true;
      }
   }

   // check if it's one of the types we know
   switch (QMetaType::Type(type)) {
      case QMetaType::Bool:
         *reinterpret_cast<bool *>(ptr) = toBool(exec, value);
         return true;
      case QMetaType::Int:
         *reinterpret_cast<int *>(ptr) = toInt32(exec, value);
         return true;
      case QMetaType::UInt:
         *reinterpret_cast<uint *>(ptr) = toUInt32(exec, value);
         return true;
      case QMetaType::LongLong:
         *reinterpret_cast<qint64 *>(ptr) = qint64(toInteger(exec, value));
         return true;
      case QMetaType::ULongLong:
         *reinterpret_cast<quint64 *>(ptr) = quint64(toInteger(exec, value));
         return true;
      case QMetaType::Double:
         *reinterpret_cast<double *>(ptr) = toNumber(exec, value);
         return true;
      case QMetaType::QString:
         if (value.isUndefined() || value.isNull()) {
            *reinterpret_cast<QString *>(ptr) = QString();
         } else {
            *reinterpret_cast<QString *>(ptr) = toString(exec, value);
         }
         return true;
      case QMetaType::Float:
         *reinterpret_cast<float *>(ptr) = toNumber(exec, value);
         return true;
      case QMetaType::Short:
         *reinterpret_cast<short *>(ptr) = short(toInt32(exec, value));
         return true;
      case QMetaType::UShort:
         *reinterpret_cast<unsigned short *>(ptr) = QScript::ToUInt16(toNumber(exec, value));
         return true;
      case QMetaType::Char:
         *reinterpret_cast<char *>(ptr) = char(toInt32(exec, value));
         return true;
      case QMetaType::UChar:
         *reinterpret_cast<unsigned char *>(ptr) = (unsigned char)(toInt32(exec, value));
         return true;
      case QMetaType::QChar:
         if (value.isString()) {
            QString str = toString(exec, value);
            *reinterpret_cast<QChar *>(ptr) = str.isEmpty() ? QChar() : str.at(0);
         } else {
            *reinterpret_cast<QChar *>(ptr) = QChar(QScript::ToUInt16(toNumber(exec, value)));
         }
         return true;
      case QMetaType::QDateTime:
         if (isDate(value)) {
            *reinterpret_cast<QDateTime *>(ptr) = toDateTime(exec, value);
            return true;
         }
         break;
      case QMetaType::QDate:
         if (isDate(value)) {
            *reinterpret_cast<QDate *>(ptr) = toDateTime(exec, value).date();
            return true;
         }
         break;

#ifndef QT_NO_REGEXP
      case QMetaType::QRegExp:
         if (isRegExp(value)) {
            *reinterpret_cast<QRegExp *>(ptr) = toRegExp(exec, value);
            return true;
         }
         break;
#endif

      case QMetaType::QObjectStar:
         if (isQObject(value) || value.isNull()) {
            *reinterpret_cast<QObject **>(ptr) = toQObject(exec, value);
            return true;
         }
         break;

      case QMetaType::QWidgetStar:
         if (isQObject(value) || value.isNull()) {
            QObject *qo = toQObject(exec, value);
            if (!qo || qo->isWidgetType()) {
               *reinterpret_cast<QWidget **>(ptr) = reinterpret_cast<QWidget *>(qo);
               return true;
            }
         }
         break;

      case QMetaType::QStringList:
         if (isArray(value)) {
            *reinterpret_cast<QStringList *>(ptr) = stringListFromArray(exec, value);
            return true;
         }
         break;
      case QMetaType::QVariantList:
         if (isArray(value)) {
            *reinterpret_cast<QVariantList *>(ptr) = variantListFromArray(exec, JSC::asArray(value));
            return true;
         }
         break;
      case QMetaType::QVariantMap:
         if (isObject(value)) {
            *reinterpret_cast<QVariantMap *>(ptr) = variantMapFromObject(exec, JSC::asObject(value));
            return true;
         }
         break;
      case QMetaType::QVariant:
         *reinterpret_cast<QVariant *>(ptr) = toVariant(exec, value);
         return true;
      default:
         ;
   }

   QByteArray name = QMetaType::typeName(type);

   if (convertToNativeQObject(exec, value, name, reinterpret_cast<void **>(ptr))) {
      return true;
   }

   if (isVariant(value) && name.endsWith('*')) {
      int valueType = QMetaType::type(name.left(name.size() - 1).constData());
      QVariant &var = variantValue(value);

      if (valueType == var.userType()) {
         *reinterpret_cast<void **>(ptr) = var.data();
         return true;

      } else {
         // look in the prototype chain
         JSC::JSValue proto = JSC::asObject(value)->prototype();
         while (proto.isObject()) {
            bool canCast = false;
            if (isVariant(proto)) {
               canCast = (type == variantValue(proto).userType())
                         || (valueType && (valueType == variantValue(proto).userType()));
            }

            else if (isQObject(proto)) {
               QByteArray className = name.left(name.size() - 1);

               /*  BROOM (script)
                                   if (QObject *qobject = toQObject(exec, proto))
                                       canCast = qobject->qt_metacast(className) != 0;
               */

            }

            if (canCast) {
               QByteArray varTypeName = QMetaType::typeName(var.userType());
               if (varTypeName.endsWith('*')) {
                  *reinterpret_cast<void **>(ptr) = *reinterpret_cast<void **>(var.data());
               } else {
                  *reinterpret_cast<void **>(ptr) = var.data();
               }
               return true;
            }
            proto = JSC::asObject(proto)->prototype();
         }
      }
   } else if (value.isNull() && name.endsWith('*')) {
      *reinterpret_cast<void **>(ptr) = 0;
      return true;
   } else if (type == qMetaTypeId<QScriptValue>()) {
      if (!eng) {
         return false;
      }
      *reinterpret_cast<QScriptValue *>(ptr) = eng->scriptValueFromJSCValue(value);
      return true;
   }

   // lazy registration of some common list types

   else if (type == qMetaTypeId<QObjectList>()) {
      if (!eng) {
         return false;
      }
      qScriptRegisterSequenceMetaType<QObjectList>(eng->q_func());
      return convertValue(exec, value, type, ptr);
   }

   else if (type == qMetaTypeId<QList<int> >()) {
      if (!eng) {
         return false;
      }
      qScriptRegisterSequenceMetaType<QList<int> >(eng->q_func());
      return convertValue(exec, value, type, ptr);
   }

   return false;
}

bool QScriptEnginePrivate::convertNumber(qsreal value, int type, void *ptr)
{
   switch (QMetaType::Type(type)) {
      case QMetaType::Bool:
         *reinterpret_cast<bool *>(ptr) = QScript::ToBool(value);
         return true;
      case QMetaType::Int:
         *reinterpret_cast<int *>(ptr) = QScript::ToInt32(value);
         return true;
      case QMetaType::UInt:
         *reinterpret_cast<uint *>(ptr) = QScript::ToUInt32(value);
         return true;
      case QMetaType::LongLong:
         *reinterpret_cast<qint64 *>(ptr) = qint64(QScript::ToInteger(value));
         return true;
      case QMetaType::ULongLong:
         *reinterpret_cast<quint64 *>(ptr) = quint64(QScript::ToInteger(value));
         return true;
      case QMetaType::Double:
         *reinterpret_cast<double *>(ptr) = value;
         return true;
      case QMetaType::QString:
         *reinterpret_cast<QString *>(ptr) = QScript::ToString(value);
         return true;
      case QMetaType::Float:
         *reinterpret_cast<float *>(ptr) = value;
         return true;
      case QMetaType::Short:
         *reinterpret_cast<short *>(ptr) = short(QScript::ToInt32(value));
         return true;
      case QMetaType::UShort:
         *reinterpret_cast<unsigned short *>(ptr) = QScript::ToUInt16(value);
         return true;
      case QMetaType::Char:
         *reinterpret_cast<char *>(ptr) = char(QScript::ToInt32(value));
         return true;
      case QMetaType::UChar:
         *reinterpret_cast<unsigned char *>(ptr) = (unsigned char)(QScript::ToInt32(value));
         return true;
      case QMetaType::QChar:
         *reinterpret_cast<QChar *>(ptr) = QChar(QScript::ToUInt16(value));
         return true;
      default:
         break;
   }
   return false;
}

bool QScriptEnginePrivate::convertString(const QString &value, int type, void *ptr)
{
   switch (QMetaType::Type(type)) {
      case QMetaType::Bool:
         *reinterpret_cast<bool *>(ptr) = QScript::ToBool(value);
         return true;
      case QMetaType::Int:
         *reinterpret_cast<int *>(ptr) = QScript::ToInt32(value);
         return true;
      case QMetaType::UInt:
         *reinterpret_cast<uint *>(ptr) = QScript::ToUInt32(value);
         return true;
      case QMetaType::LongLong:
         *reinterpret_cast<qint64 *>(ptr) = qint64(QScript::ToInteger(value));
         return true;
      case QMetaType::ULongLong:
         *reinterpret_cast<quint64 *>(ptr) = quint64(QScript::ToInteger(value));
         return true;
      case QMetaType::Double:
         *reinterpret_cast<double *>(ptr) = QScript::ToNumber(value);
         return true;
      case QMetaType::QString:
         *reinterpret_cast<QString *>(ptr) = value;
         return true;
      case QMetaType::Float:
         *reinterpret_cast<float *>(ptr) = QScript::ToNumber(value);
         return true;
      case QMetaType::Short:
         *reinterpret_cast<short *>(ptr) = short(QScript::ToInt32(value));
         return true;
      case QMetaType::UShort:
         *reinterpret_cast<unsigned short *>(ptr) = QScript::ToUInt16(value);
         return true;
      case QMetaType::Char:
         *reinterpret_cast<char *>(ptr) = char(QScript::ToInt32(value));
         return true;
      case QMetaType::UChar:
         *reinterpret_cast<unsigned char *>(ptr) = (unsigned char)(QScript::ToInt32(value));
         return true;
      case QMetaType::QChar:
         *reinterpret_cast<QChar *>(ptr) = QChar(QScript::ToUInt16(value));
         return true;
      default:
         break;
   }
   return false;
}

bool QScriptEnginePrivate::hasDemarshalFunction(int type) const
{
   QScriptTypeInfo *info = m_typeInfos.value(type);
   return info && (info->demarshal != 0);
}

JSC::UString QScriptEnginePrivate::translationContextFromUrl(const JSC::UString &url)
{
   if (url != cachedTranslationUrl) {
      cachedTranslationContext = QFileInfo(url).baseName();
      cachedTranslationUrl = url;
   }
   return cachedTranslationContext;
}

/*!
    \internal
*/
bool QScriptEngine::convert(const QScriptValue &value, int type, void *ptr)
{
   Q_D(QScriptEngine);
   QScript::APIShim shim(d);
   return QScriptEnginePrivate::convertValue(d->currentFrame, d->scriptValueToJSCValue(value), type, ptr);
}

/*!
    \internal
*/
bool QScriptEngine::convertV2(const QScriptValue &value, int type, void *ptr)
{
   QScriptValuePrivate *vp = QScriptValuePrivate::get(value);
   if (vp) {
      switch (vp->type) {
         case QScriptValuePrivate::JavaScriptCore: {
            if (vp->engine) {
               QScript::APIShim shim(vp->engine);
               return QScriptEnginePrivate::convertValue(vp->engine->currentFrame, vp->jscValue, type, ptr);
            } else {
               return QScriptEnginePrivate::convertValue(0, vp->jscValue, type, ptr);
            }
         }
         case QScriptValuePrivate::Number:
            return QScriptEnginePrivate::convertNumber(vp->numberValue, type, ptr);
         case QScriptValuePrivate::String:
            return QScriptEnginePrivate::convertString(vp->stringValue, type, ptr);
      }
   }
   return false;
}

/*!
    \internal
*/
void QScriptEngine::registerCustomType(int type, MarshalFunction mf,
                                       DemarshalFunction df,
                                       const QScriptValue &prototype)
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

/*!
  \since 4.5

  Installs translator functions on the given \a object, or on the Global
  Object if no object is specified.

  The relation between Qt Script translator functions and C++ translator
  functions is described in the following table:

    \table
    \header \o Script Function \o Corresponding C++ Function
    \row    \o qsTr()       \o QObject::tr()
    \row    \o QT_TR_NOOP() \o QT_TR_NOOP()
    \row    \o qsTranslate() \o QCoreApplication::translate()
    \row    \o QT_TRANSLATE_NOOP() \o QT_TRANSLATE_NOOP()
    \row    \o qsTrId() (since 4.7) \o qtTrId()
    \row    \o QT_TRID_NOOP() (since 4.7) \o QT_TRID_NOOP()
    \endtable

  \sa {Internationalization with Qt}
*/
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

#ifndef QT_NO_TRANSLATION
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
#endif

   glob->stringPrototype()->putDirectFunction(exec, new (exec)JSC::NativeFunctionWrapper(exec,
         glob->prototypeFunctionStructure(), 1, JSC::Identifier(exec, "arg"), QScript::stringProtoFuncArg));
}

/*!
    Imports the given \a extension into this QScriptEngine.  Returns
    undefinedValue() if the extension was successfully imported. You
    can call hasUncaughtException() to check if an error occurred; in
    that case, the return value is the value that was thrown by the
    exception (usually an \c{Error} object).

    QScriptEngine ensures that a particular extension is only imported
    once; subsequent calls to importExtension() with the same extension
    name will do nothing and return undefinedValue().

    \sa availableExtensions(), QScriptExtensionPlugin, {Creating QtScript Extensions}
*/
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
         return context->throwError(QString::fromLatin1("recursive import of %0")
                                    .arg(extension));
      }
      d->extensionsBeingImported.insert(ext);

      QScriptExtensionInterface *iface = 0;
      QString initjsContents;
      QString initjsFileName;

      // look for the extension in static plugins
      for (int j = 0; j < staticPlugins.size(); ++j) {
         iface = qobject_cast<QScriptExtensionInterface *>(staticPlugins.at(j));
         if (!iface) {
            continue;
         }
         if (iface->keys().contains(ext)) {
            break;   // use this one
         } else {
            iface = 0;   // keep looking
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
                     break;   // use this one
                  } else {
                     iface = 0;   // keep looking
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
               QFile file(dirdir.canonicalPath()
                          + QDir::separator() + initDotJs);
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
         return context->throwError(
                   QString::fromLatin1("Unable to import %0: no such extension")
                   .arg(extension));
      }

      // initialize the extension in a new context
      QScriptContext *ctx = pushContext();
      ctx->setThisObject(globalObject());
      ctx->activationObject().setProperty(QLatin1String("__extension__"), ext,
                                          QScriptValue::ReadOnly | QScriptValue::Undeletable);
      ctx->activationObject().setProperty(QLatin1String("__setupPackage__"),
                                          newFunction(QScript::__setupPackage__));
      ctx->activationObject().setProperty(QLatin1String("__postInit__"), QScriptValue(QScriptValue::UndefinedValue));

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

/*!
    \since 4.4

    Returns a list naming the available extensions that can be
    imported using the importExtension() function. This list includes
    extensions that have been imported.

    \sa importExtension(), importedExtensions()
*/
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

/*!
    \since 4.4

    Returns a list naming the extensions that have been imported
    using the importExtension() function.

    \sa availableExtensions()
*/
QStringList QScriptEngine::importedExtensions() const
{
   Q_D(const QScriptEngine);
   QStringList lst = d->importedExtensions.toList();
   std::sort(lst.begin(), lst.end());
   return lst;
}

/*! \fn QScriptValue QScriptEngine::toScriptValue(const T &value)

    Creates a QScriptValue with the given \a value.

    Note that the template type \c{T} must be known to QMetaType.

    See \l{Conversion Between QtScript and C++ Types} for a
    description of the built-in type conversion provided by
    QtScript. By default, the types that are not specially handled by
    QtScript are represented as QVariants (e.g. the \a value is passed
    to newVariant()); you can change this behavior by installing your
    own type conversion functions with qScriptRegisterMetaType().

    \sa fromScriptValue(), qScriptRegisterMetaType()
*/

/*! \fn T QScriptEngine::fromScriptValue(const QScriptValue &value)

    Returns the given \a value converted to the template type \c{T}.

    Note that \c{T} must be known to QMetaType.

    See \l{Conversion Between QtScript and C++ Types} for a
    description of the built-in type conversion provided by
    QtScript.

    \sa toScriptValue(), qScriptRegisterMetaType()
*/

/*!
    \fn QScriptValue qScriptValueFromValue(QScriptEngine *engine, const T &value)
    \since 4.3
    \relates QScriptEngine
    \obsolete

    Creates a QScriptValue using the given \a engine with the given \a
    value of template type \c{T}.

    This function is equivalent to QScriptEngine::toScriptValue().

    \note This function was provided as a workaround for MSVC 6
    which did not support member template functions. It is advised
    to use the other form in new code.

    \sa QScriptEngine::toScriptValue(), qscriptvalue_cast()
*/

/*!
    \fn T qScriptValueToValue(const QScriptValue &value)
    \since 4.3
    \relates QScriptEngine
    \obsolete

    Returns the given \a value converted to the template type \c{T}.

    This function is equivalent to QScriptEngine::fromScriptValue().

    \note This function was provided as a workaround for MSVC 6
    which did not support member template functions. It is advised
    to use the other form in new code.

    \sa QScriptEngine::fromScriptValue()
*/

/*!
    \fn QScriptValue qScriptValueFromSequence(QScriptEngine *engine, const Container &container)
    \since 4.3
    \relates QScriptEngine

    Creates an array in the form of a QScriptValue using the given \a engine
    with the given \a container of template type \c{Container}.

    The \c Container type must provide a \c const_iterator class to enable the
    contents of the container to be copied into the array.

    Additionally, the type of each element in the sequence should be
    suitable for conversion to a QScriptValue.  See
    \l{Conversion Between QtScript and C++ Types} for more information
    about the restrictions on types that can be used with QScriptValue.

    \sa QScriptEngine::fromScriptValue()
*/

/*!
    \fn void qScriptValueToSequence(const QScriptValue &value, Container &container)
    \since 4.3
    \relates QScriptEngine

    Copies the elements in the sequence specified by \a value to the given
    \a container of template type \c{Container}.

    The \a value used is typically an array, but any container can be copied
    as long as it provides a \c length property describing how many elements
    it contains.

    Additionally, the type of each element in the sequence must be
    suitable for conversion to a C++ type from a QScriptValue.  See
    \l{Conversion Between QtScript and C++ Types} for more information
    about the restrictions on types that can be used with
    QScriptValue.

    \sa qscriptvalue_cast()
*/

/*!
    \fn T qscriptvalue_cast(const QScriptValue &value)
    \since 4.3
    \relates QScriptValue

    Returns the given \a value converted to the template type \c{T}.

    \sa qScriptRegisterMetaType(), QScriptEngine::toScriptValue()
*/

/*! \fn int qScriptRegisterMetaType(
            QScriptEngine *engine,
            QScriptValue (*toScriptValue)(QScriptEngine *, const T &t),
            void (*fromScriptValue)(const QScriptValue &, T &t),
            const QScriptValue &prototype = QScriptValue())
    \relates QScriptEngine

    Registers the type \c{T} in the given \a engine. \a toScriptValue must
    be a function that will convert from a value of type \c{T} to a
    QScriptValue, and \a fromScriptValue a function that does the
    opposite. \a prototype, if valid, is the prototype that's set on
    QScriptValues returned by \a toScriptValue.

    Returns the internal ID used by QMetaType.

    You only need to call this function if you want to provide custom
    conversion of values of type \c{T}, i.e. if the default
    QVariant-based representation and conversion is not
    appropriate. (Note that custom QObject-derived types also fall in
    this category; e.g. for a QObject-derived class called MyObject,
    you probably want to define conversion functions for MyObject*
    that utilize QScriptEngine::newQObject() and
    QScriptValue::toQObject().)

    If you only want to define a common script interface for values of
    type \c{T}, and don't care how those values are represented
    (i.e. storing them in QVariants is fine), use
    \l{QScriptEngine::setDefaultPrototype()}{setDefaultPrototype}()
    instead; this will minimize conversion costs.

    You need to declare the custom type first with
    Q_DECLARE_METATYPE().

    After a type has been registered, you can convert from a
    QScriptValue to that type using
    \l{QScriptEngine::fromScriptValue()}{fromScriptValue}(), and
    create a QScriptValue from a value of that type using
    \l{QScriptEngine::toScriptValue()}{toScriptValue}(). The engine
    will take care of calling the proper conversion function when
    calling C++ slots, and when getting or setting a C++ property;
    i.e. the custom type may be used seamlessly on both the C++ side
    and the script side.

    The following is an example of how to use this function. We will
    specify custom conversion of our type \c{MyStruct}. Here's the C++
    type:

    \snippet doc/src/snippets/code/src_script_qscriptengine.cpp 20

    We must declare it so that the type will be known to QMetaType:

    \snippet doc/src/snippets/code/src_script_qscriptengine.cpp 21

    Next, the \c{MyStruct} conversion functions. We represent the
    \c{MyStruct} value as a script object and just copy the properties:

    \snippet doc/src/snippets/code/src_script_qscriptengine.cpp 22

    Now we can register \c{MyStruct} with the engine:
    \snippet doc/src/snippets/code/src_script_qscriptengine.cpp 23

    Working with \c{MyStruct} values is now easy:
    \snippet doc/src/snippets/code/src_script_qscriptengine.cpp 24

    If you want to be able to construct values of your custom type
    from script code, you have to register a constructor function for
    the type. For example:

    \snippet doc/src/snippets/code/src_script_qscriptengine.cpp 25

    \sa qScriptRegisterSequenceMetaType(), qRegisterMetaType()
*/

/*!
    \macro Q_SCRIPT_DECLARE_QMETAOBJECT(QMetaObject, ArgType)
    \since 4.3
    \relates QScriptEngine

    Declares the given \a QMetaObject. Used in combination with
    QScriptEngine::scriptValueFromQMetaObject() to make enums and
    instantiation of \a QMetaObject available to script code. The
    constructor generated by this macro takes a single argument of
    type \a ArgType; typically the argument is the parent type of the
    new instance, in which case \a ArgType is \c{QWidget*} or
    \c{QObject*}. Objects created by the constructor will have
    QScriptEngine::AutoOwnership ownership.
*/

/*! \fn int qScriptRegisterSequenceMetaType(
            QScriptEngine *engine,
            const QScriptValue &prototype = QScriptValue())
    \relates QScriptEngine

    Registers the sequence type \c{T} in the given \a engine. This
    function provides conversion functions that convert between \c{T}
    and Qt Script \c{Array} objects. \c{T} must provide a
    const_iterator class and begin(), end() and push_back()
    functions. If \a prototype is valid, it will be set as the
    prototype of \c{Array} objects due to conversion from \c{T};
    otherwise, the standard \c{Array} prototype will be used.

    Returns the internal ID used by QMetaType.

    You need to declare the container type first with
    Q_DECLARE_METATYPE(). If the element type isn't a standard Qt/C++
    type, it must be declared using Q_DECLARE_METATYPE() as well.
    Example:

    \snippet doc/src/snippets/code/src_script_qscriptengine.cpp 26

    \sa qScriptRegisterMetaType()
*/

/*!
  Runs the garbage collector.

  The garbage collector will attempt to reclaim memory by locating and
  disposing of objects that are no longer reachable in the script
  environment.

  Normally you don't need to call this function; the garbage collector
  will automatically be invoked when the QScriptEngine decides that
  it's wise to do so (i.e. when a certain number of new objects have
  been created). However, you can call this function to explicitly
  request that garbage collection should be performed as soon as
  possible.

  \sa reportAdditionalMemoryCost()
*/
void QScriptEngine::collectGarbage()
{
   Q_D(QScriptEngine);
   d->collectGarbage();
}

/*!
  \since 4.7

  Reports an additional memory cost of the given \a size, measured in
  bytes, to the garbage collector.

  This function can be called to indicate that a Qt Script object has
  memory associated with it that isn't managed by Qt Script itself.
  Reporting the additional cost makes it more likely that the garbage
  collector will be triggered.

  Note that if the additional memory is shared with objects outside
  the scripting environment, the cost should not be reported, since
  collecting the Qt Script object would not cause the memory to be
  freed anyway.

  Negative \a size values are ignored, i.e. this function can't be
  used to report that the additional memory has been deallocated.

  \sa collectGarbage()
*/
void QScriptEngine::reportAdditionalMemoryCost(int size)
{
   Q_D(QScriptEngine);
   d->reportAdditionalMemoryCost(size);
}

/*!

  Sets the interval between calls to QCoreApplication::processEvents
  to \a interval milliseconds.

  While the interpreter is running, all event processing is by default
  blocked. This means for instance that the gui will not be updated
  and timers will not be fired. To allow event processing during
  interpreter execution one can specify the processing interval to be
  a positive value, indicating the number of milliseconds between each
  time QCoreApplication::processEvents() is called.

  The default value is -1, which disables event processing during
  interpreter execution.

  You can use QCoreApplication::postEvent() to post an event that
  performs custom processing at the next interval. For example, you
  could keep track of the total running time of the script and call
  abortEvaluation() when you detect that the script has been running
  for a long time without completing.

  \sa processEventsInterval()
*/
void QScriptEngine::setProcessEventsInterval(int interval)
{
   Q_D(QScriptEngine);
   d->processEventsInterval = interval;

   if (interval > 0) {
      d->globalData->timeoutChecker->setCheckInterval(interval);
   }

   d->timeoutChecker()->setShouldProcessEvents(interval > 0);
}

/*!

  Returns the interval in milliseconds between calls to
  QCoreApplication::processEvents() while the interpreter is running.

  \sa setProcessEventsInterval()
*/
int QScriptEngine::processEventsInterval() const
{
   Q_D(const QScriptEngine);
   return d->processEventsInterval;
}

/*!
  \since 4.4

  Returns true if this engine is currently evaluating a script,
  otherwise returns false.

  \sa evaluate(), abortEvaluation()
*/
bool QScriptEngine::isEvaluating() const
{
   Q_D(const QScriptEngine);
   return (d->currentFrame != d->globalExec()) || d->inEval;
}

/*!
  \since 4.4

  Aborts any script evaluation currently taking place in this engine.
  The given \a result is passed back as the result of the evaluation
  (i.e. it is returned from the call to evaluate() being aborted).

  If the engine isn't evaluating a script (i.e. isEvaluating() returns
  false), this function does nothing.

  Call this function if you need to abort a running script for some
  reason, e.g.  when you have detected that the script has been
  running for several seconds without completing.

  \sa evaluate(), isEvaluating(), setProcessEventsInterval()
*/
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

/*!
  \since 4.4
  \relates QScriptEngine

  Creates a connection from the \a signal in the \a sender to the
  given \a function. If \a receiver is an object, it will act as the
  `this' object when the signal handler function is invoked. Returns
  true if the connection succeeds; otherwise returns false.

  \sa qScriptDisconnect(), QScriptEngine::signalHandlerException()
*/
bool qScriptConnect(QObject *sender, const char *signal,
                    const QScriptValue &receiver, const QScriptValue &function)
{
   if (!sender || !signal) {
      return false;
   }
   if (!function.isFunction()) {
      return false;
   }
   if (receiver.isObject() && (receiver.engine() != function.engine())) {
      return false;
   }
   QScriptEnginePrivate *engine = QScriptEnginePrivate::get(function.engine());
   QScript::APIShim shim(engine);
   JSC::JSValue jscReceiver = engine->scriptValueToJSCValue(receiver);
   JSC::JSValue jscFunction = engine->scriptValueToJSCValue(function);
   return engine->scriptConnect(sender, signal, jscReceiver, jscFunction,
                                Qt::AutoConnection);
}

/*!
  \since 4.4
  \relates QScriptEngine

  Disconnects the \a signal in the \a sender from the given (\a
  receiver, \a function) pair. Returns true if the connection is
  successfully broken; otherwise returns false.

  \sa qScriptConnect()
*/
bool qScriptDisconnect(QObject *sender, const char *signal,
                       const QScriptValue &receiver, const QScriptValue &function)
{
   if (!sender || !signal) {
      return false;
   }
   if (!function.isFunction()) {
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

/*!
    \since 4.4
    \fn void QScriptEngine::signalHandlerException(const QScriptValue &exception)

    This signal is emitted when a script function connected to a signal causes
    an \a exception.

    \sa qScriptConnect()
*/


/*!
  \since 4.4

  Installs the given \a agent on this engine. The agent will be
  notified of various events pertaining to script execution. This is
  useful when you want to find out exactly what the engine is doing,
  e.g. when evaluate() is called. The agent interface is the basis of
  tools like debuggers and profilers.

  The engine maintains ownership of the \a agent.

  Calling this function will replace the existing agent, if any.

  \sa agent()
*/
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

/*!
  \since 4.4

  Returns the agent currently installed on this engine, or 0 if no
  agent is installed.

  \sa setAgent()
*/
QScriptEngineAgent *QScriptEngine::agent() const
{
   Q_D(const QScriptEngine);
   return d->activeAgent;
}

/*!
  \since 4.4

  Returns a handle that represents the given string, \a str.

  QScriptString can be used to quickly look up properties, and
  compare property names, of script objects.

  \sa QScriptValue::property()
*/
QScriptString QScriptEngine::toStringHandle(const QString &str)
{
   Q_D(QScriptEngine);
   QScript::APIShim shim(d);
   return d->toStringHandle(JSC::Identifier(d->currentFrame, str));
}

/*!
  \since 4.5

  Converts the given \a value to an object, if such a conversion is
  possible; otherwise returns an invalid QScriptValue. The conversion
  is performed according to the following table:

    \table
    \header \o Input Type \o Result
    \row    \o Undefined  \o An invalid QScriptValue.
    \row    \o Null       \o An invalid QScriptValue.
    \row    \o Boolean    \o A new Boolean object whose internal value is set to the value of the boolean.
    \row    \o Number     \o A new Number object whose internal value is set to the value of the number.
    \row    \o String     \o A new String object whose internal value is set to the value of the string.
    \row    \o Object     \o The result is the object itself (no conversion).
    \endtable

    \sa newObject()
*/
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

/*!
  \internal

  Returns the object with the given \a id, or an invalid
  QScriptValue if there is no object with that id.

  \sa QScriptValue::objectId()
*/
QScriptValue QScriptEngine::objectById(qint64 id) const
{
   Q_D(const QScriptEngine);
   // Assumes that the cell was not been garbage collected
   return const_cast<QScriptEnginePrivate *>(d)->scriptValueFromJSCValue((JSC::JSCell *)id);
}

/*!
  \since 4.5
  \class QScriptSyntaxCheckResult

  \brief The QScriptSyntaxCheckResult class provides the result of a script syntax check.

  \ingroup script
  \mainclass

  QScriptSyntaxCheckResult is returned by QScriptEngine::checkSyntax() to
  provide information about the syntactical (in)correctness of a script.
*/

/*!
    \enum QScriptSyntaxCheckResult::State

    This enum specifies the state of a syntax check.

    \value Error The program contains a syntax error.
    \value Intermediate The program is incomplete.
    \value Valid The program is a syntactically correct Qt Script program.
*/

/*!
  Constructs a new QScriptSyntaxCheckResult from the \a other result.
*/
QScriptSyntaxCheckResult::QScriptSyntaxCheckResult(const QScriptSyntaxCheckResult &other)
   : d_ptr(other.d_ptr)
{
}

/*!
  \internal
*/
QScriptSyntaxCheckResult::QScriptSyntaxCheckResult(QScriptSyntaxCheckResultPrivate *d)
   : d_ptr(d)
{
}

/*!
  \internal
*/
QScriptSyntaxCheckResult::QScriptSyntaxCheckResult()
   : d_ptr(0)
{
}

/*!
  Destroys this QScriptSyntaxCheckResult.
*/
QScriptSyntaxCheckResult::~QScriptSyntaxCheckResult()
{
}

/*!
  Returns the state of this QScriptSyntaxCheckResult.
*/
QScriptSyntaxCheckResult::State QScriptSyntaxCheckResult::state() const
{
   Q_D(const QScriptSyntaxCheckResult);
   if (!d) {
      return Valid;
   }
   return d->state;
}

/*!
  Returns the error line number of this QScriptSyntaxCheckResult, or -1 if
  there is no error.

  \sa state(), errorMessage()
*/
int QScriptSyntaxCheckResult::errorLineNumber() const
{
   Q_D(const QScriptSyntaxCheckResult);
   if (!d) {
      return -1;
   }
   return d->errorLineNumber;
}

/*!
  Returns the error column number of this QScriptSyntaxCheckResult, or -1 if
  there is no error.

  \sa state(), errorLineNumber()
*/
int QScriptSyntaxCheckResult::errorColumnNumber() const
{
   Q_D(const QScriptSyntaxCheckResult);
   if (!d) {
      return -1;
   }
   return d->errorColumnNumber;
}

/*!
  Returns the error message of this QScriptSyntaxCheckResult, or an empty
  string if there is no error.

  \sa state(), errorLineNumber()
*/
QString QScriptSyntaxCheckResult::errorMessage() const
{
   Q_D(const QScriptSyntaxCheckResult);
   if (!d) {
      return QString();
   }
   return d->errorMessage;
}

/*!
  Assigns the \a other result to this QScriptSyntaxCheckResult, and returns a
  reference to this QScriptSyntaxCheckResult.
*/
QScriptSyntaxCheckResult &QScriptSyntaxCheckResult::operator=(const QScriptSyntaxCheckResult &other)
{
   d_ptr = other.d_ptr;
   return *this;
}

void QScriptEngine::_q_objectDestroyed(QObject *un_named_arg1)
{
   Q_D(QScriptEngine);
   d->_q_objectDestroyed(un_named_arg1);
}

QScriptEnginePrivate *QScriptEnginePrivate::cs_getPrivate(QScriptEngine *object)
{
   return object->d_ptr.data();
}

QT_END_NAMESPACE

