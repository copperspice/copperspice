/***********************************************************************
*
* Copyright (c) 2012-2020 Barbara Geller
* Copyright (c) 2012-2020 Ansel Sermersheim
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

#ifndef QSCRIPTENGINE_H
#define QSCRIPTENGINE_H

#include <qmetatype.h>
#include <qvariant.h>
#include <qsharedpointer.h>
#include <qobject.h>

#include <QtScript/qscriptvalue.h>
#include <QtScript/qscriptcontext.h>
#include <QtScript/qscriptstring.h>
#include <QtScript/qscriptprogram.h>
#include <qstringfwd.h>

class QDateTime;
class QScriptClass;
class QScriptEngineAgent;
class QScriptEnginePrivate;

template <class T>
inline QScriptValue qscriptQMetaObjectConstructor(QScriptContext *, QScriptEngine *, T *)
{
   return QScriptValue();
}

template <typename T>
inline QScriptValue qScriptValueFromValue(QScriptEngine *, const T &);

template <typename T>
inline T qscriptvalue_cast(const QScriptValue &);

class QScriptSyntaxCheckResultPrivate;

class Q_SCRIPT_EXPORT QScriptSyntaxCheckResult
{
 public:
   enum State {
      Error,
      Intermediate,
      Valid
   };

   QScriptSyntaxCheckResult(const QScriptSyntaxCheckResult &other);
   ~QScriptSyntaxCheckResult();

   State state() const;
   int errorLineNumber() const;
   int errorColumnNumber() const;
   QString errorMessage() const;

   QScriptSyntaxCheckResult &operator=(const QScriptSyntaxCheckResult &other);

 private:
   QScriptSyntaxCheckResult();
   QScriptSyntaxCheckResult(QScriptSyntaxCheckResultPrivate *d);
   QExplicitlySharedDataPointer<QScriptSyntaxCheckResultPrivate> d_ptr;

   Q_DECLARE_PRIVATE(QScriptSyntaxCheckResult)
   friend class QScriptEngine;
   friend class QScriptEnginePrivate;
};

class Q_SCRIPT_EXPORT QScriptEngine
   : public QObject

{
   SCRIPT_CS_OBJECT(QScriptEngine)

 public:
   enum ValueOwnership {
      QtOwnership,
      ScriptOwnership,
      AutoOwnership
   };

   enum QObjectWrapOption {
      ExcludeChildObjects = 0x0001,
      ExcludeSuperClassMethods = 0x0002,
      ExcludeSuperClassProperties = 0x0004,
      ExcludeSuperClassContents = 0x0006,
      SkipMethodsInEnumeration = 0x0008,
      ExcludeDeleteLater = 0x0010,
      ExcludeSlots = 0x0020,

      AutoCreateDynamicProperties = 0x0100,
      PreferExistingWrapperObject = 0x0200
   };
   using QObjectWrapOptions = QFlags<QObjectWrapOption>;

   QScriptEngine();

   explicit QScriptEngine(QObject *parent);
   virtual ~QScriptEngine();

   QScriptValue globalObject() const;
   void setGlobalObject(const QScriptValue &object);

   QScriptContext *currentContext() const;
   QScriptContext *pushContext();
   void popContext();

   bool canEvaluate(const QString &program) const;
   static QScriptSyntaxCheckResult checkSyntax(const QString &program);

   QScriptValue evaluate(const QString &program, const QString &fileName = QString(), int lineNumber = 1);

   QScriptValue evaluate(const QScriptProgram &program);

   bool isEvaluating() const;
   void abortEvaluation(const QScriptValue &result = QScriptValue());

   bool hasUncaughtException() const;
   QScriptValue uncaughtException() const;
   int uncaughtExceptionLineNumber() const;
   QStringList uncaughtExceptionBacktrace() const;
   void clearExceptions();

   QScriptValue nullValue();
   QScriptValue undefinedValue();

   typedef QScriptValue (*FunctionSignature)(QScriptContext *, QScriptEngine *);
   typedef QScriptValue (*FunctionWithArgSignature)(QScriptContext *, QScriptEngine *, void *);

   QScriptValue newFunction(FunctionSignature signature, int length = 0);
   QScriptValue newFunction(FunctionSignature signature, const QScriptValue &prototype, int length = 0);

   QScriptValue newFunction(FunctionWithArgSignature signature, void *arg);

   QScriptValue newVariant(const QVariant &value);
   QScriptValue newVariant(const QScriptValue &object, const QVariant &value);

   QScriptValue newRegExp(const QRegularExpression &regexp);

   QScriptValue newObject();
   QScriptValue newObject(QScriptClass *scriptClass, const QScriptValue &data = QScriptValue());
   QScriptValue newArray(uint length = 0);
   QScriptValue newRegExp(const QString &pattern, const QString &flags);
   QScriptValue newDate(qsreal value);
   QScriptValue newDate(const QDateTime &value);
   QScriptValue newActivationObject();


   QScriptValue newQObject(QObject *object, ValueOwnership ownership = QtOwnership,
      const QObjectWrapOptions &options = 0);
   QScriptValue newQObject(const QScriptValue &scriptObject, QObject *qtObject,
      ValueOwnership ownership = QtOwnership,
      const QObjectWrapOptions &options = 0);

   QScriptValue newQMetaObject(const QMetaObject *metaObject, const QScriptValue &ctor = QScriptValue());

   template <class T> QScriptValue scriptValueFromQMetaObject();

   QScriptValue defaultPrototype(int metaTypeId) const;
   void setDefaultPrototype(int metaTypeId, const QScriptValue &prototype);


   typedef QScriptValue (*MarshalFunction)(QScriptEngine *, const void *);
   typedef void (*DemarshalFunction)(const QScriptValue &, void *);

   template <typename T>
   inline QScriptValue toScriptValue(const T &value) {
      return qScriptValueFromValue(this, value);
   }

   template <typename T>
   inline T fromScriptValue(const QScriptValue &value) {
      return qscriptvalue_cast<T>(value);
   }

   void installTranslatorFunctions(const QScriptValue &object = QScriptValue());

   QScriptValue importExtension(const QString &extension);
   QStringList availableExtensions() const;
   QStringList importedExtensions() const;

   void collectGarbage();
   void reportAdditionalMemoryCost(int size);

   void setProcessEventsInterval(int interval);
   int processEventsInterval() const;

   void setAgent(QScriptEngineAgent *agent);
   QScriptEngineAgent *agent() const;

   QScriptString toStringHandle(const QString &str);
   QScriptValue toObject(const QScriptValue &value);

   QScriptValue objectById(qint64 id) const;


 public:
   SCRIPT_CS_SIGNAL_1(Public, void signalHandlerException(const QScriptValue &exception))
   SCRIPT_CS_SIGNAL_2(signalHandlerException, exception)

 private:
   QScriptValue create(int type, const void *ptr);

   bool convert(const QScriptValue &value, int type, void *ptr);
   static bool convertV2(const QScriptValue &value, int type, void *ptr);

   void registerCustomType(int type, MarshalFunction mf, DemarshalFunction df,
      const QScriptValue &prototype);

   friend inline void qScriptRegisterMetaType_helper(QScriptEngine *,
      int, MarshalFunction, DemarshalFunction, const QScriptValue &);

   friend inline QScriptValue qScriptValueFromValue_helper(QScriptEngine *, int, const void *);

   friend inline bool qscriptvalue_cast_helper(const QScriptValue &, int, void *);

 protected:
   QScriptEngine(QScriptEnginePrivate &dd, QObject *parent = nullptr);

 private:
   Q_DECLARE_PRIVATE(QScriptEngine)
   Q_DISABLE_COPY(QScriptEngine)

   SCRIPT_CS_SLOT_1(Private, void _q_objectDestroyed(QObject *un_named_arg1))
   SCRIPT_CS_SLOT_2(_q_objectDestroyed)

 protected:
   QScopedPointer<QScriptEnginePrivate> d_ptr;

};


#define Q_SCRIPT_DECLARE_QMETAOBJECT(T, _Arg1) \
template<> inline QScriptValue qscriptQMetaObjectConstructor<T>(QScriptContext *ctx, QScriptEngine *eng, T *) \
{ \
    _Arg1 arg1 = qscriptvalue_cast<_Arg1> (ctx->argument(0)); \
    T* t = new T(arg1); \
    if (ctx->isCalledAsConstructor()) \
        return eng->newQObject(ctx->thisObject(), t, QScriptEngine::AutoOwnership); \
    QScriptValue o = eng->newQObject(t, QScriptEngine::AutoOwnership); \
    o.setPrototype(ctx->callee().property(QString::fromLatin1("prototype"))); \
    return o; \
}

template <class T> QScriptValue QScriptEngine::scriptValueFromQMetaObject()
{
   typedef QScriptValue(*ConstructPtr)(QScriptContext *, QScriptEngine *, T *);
   ConstructPtr cptr = qscriptQMetaObjectConstructor<T>;
   return newQMetaObject(&T::staticMetaObject,
         newFunction(reinterpret_cast<FunctionWithArgSignature>(cptr), 0));
}

inline QScriptValue qScriptValueFromValue_helper(QScriptEngine *engine, int type, const void *ptr)
{
   if (!engine) {
      return QScriptValue();
   }

   return engine->create(type, ptr);
}

template <typename T>
inline QScriptValue qScriptValueFromValue(QScriptEngine *engine, const T &t)
{
   return qScriptValueFromValue_helper(engine, qMetaTypeId<T>(), &t);
}

template <>
inline QScriptValue qScriptValueFromValue<QVariant>(QScriptEngine *engine, const QVariant &v)
{
   return qScriptValueFromValue_helper(engine, v.userType(), v.data());
}

inline bool qscriptvalue_cast_helper(const QScriptValue &value, int type, void *ptr)
{
   return QScriptEngine::convertV2(value, type, ptr);
}

template<typename T>
T qscriptvalue_cast(const QScriptValue &value)
{
   T t;
   const int id = qMetaTypeId<T>();

   if (qscriptvalue_cast_helper(value, id, &t)) {
      return t;
   } else if (value.isVariant()) {
      return qvariant_cast<T>(value.toVariant());
   }

   return T();
}

template <>
inline QVariant qscriptvalue_cast<QVariant>(const QScriptValue &value)
{
   return value.toVariant();
}

inline void qScriptRegisterMetaType_helper(QScriptEngine *eng, int type, QScriptEngine::MarshalFunction mf,
      QScriptEngine::DemarshalFunction df, const QScriptValue &prototype)
{
   eng->registerCustomType(type, mf, df, prototype);
}

template<typename T>
int qScriptRegisterMetaType(QScriptEngine *eng, QScriptValue (*toScriptValue)(QScriptEngine *, const T &t),
   void (*fromScriptValue)(const QScriptValue &, T &t),
   const QScriptValue &prototype = QScriptValue(), T * /* dummy */ = 0  )
{
   const int id = qRegisterMetaType<T>(); // make sure it's registered

   qScriptRegisterMetaType_helper(
      eng, id, reinterpret_cast<QScriptEngine::MarshalFunction>(toScriptValue),
      reinterpret_cast<QScriptEngine::DemarshalFunction>(fromScriptValue),
      prototype);

   return id;
}

template <class Container>
QScriptValue qScriptValueFromSequence(QScriptEngine *eng, const Container &cont)
{
   QScriptValue a = eng->newArray();
   typename Container::const_iterator begin = cont.begin();
   typename Container::const_iterator end = cont.end();
   typename Container::const_iterator it;
   quint32 i;
   for (it = begin, i = 0; it != end; ++it, ++i) {
      a.setProperty(i, eng->toScriptValue(*it));
   }
   return a;
}

template <class Container>
void qScriptValueToSequence(const QScriptValue &value, Container &cont)
{
   quint32 len = value.property(QLatin1String("length")).toUInt32();
   for (quint32 i = 0; i < len; ++i) {
      QScriptValue item = value.property(i);
      cont.push_back(qscriptvalue_cast<typename Container::value_type>(item));
   }
}

template<typename T>
int qScriptRegisterSequenceMetaType(QScriptEngine *engine, const QScriptValue &prototype = QScriptValue(), T * = nullptr)
{
   return qScriptRegisterMetaType<T>(engine, qScriptValueFromSequence, qScriptValueToSequence, prototype);
}


Q_SCRIPT_EXPORT bool qScriptConnect(QObject *sender, const QString &signal,
   const QScriptValue &receiver, const QScriptValue &function);

Q_SCRIPT_EXPORT bool qScriptDisconnect(QObject *sender, const QString &signal,
   const QScriptValue &receiver, const QScriptValue &function);


Q_DECLARE_OPERATORS_FOR_FLAGS(QScriptEngine::QObjectWrapOptions)


#endif
