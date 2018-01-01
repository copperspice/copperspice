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
#include "qscriptqobject_p.h"

#include <QtCore/qmetaobject.h>
#include <QtCore/qvarlengtharray.h>
#include <QtCore/qdebug.h>
#include <QtScript/qscriptable.h>
#include "qscriptengine_p.h"
#include "qscriptable_p.h"
#include "qscriptcontext_p.h"
#include "qscriptfunction_p.h"

#include "Error.h"
#include "PrototypeFunction.h"
#include "NativeFunctionWrapper.h"
#include "PropertyNameArray.h"
#include "JSFunction.h"
#include "JSString.h"
#include "JSValue.h"
#include "JSArray.h"
#include "RegExpObject.h"
#include "RegExpConstructor.h"

namespace JSC {
QT_USE_NAMESPACE
ASSERT_CLASS_FITS_IN_CELL(QScript::QObjectPrototype);
ASSERT_CLASS_FITS_IN_CELL(QScript::QMetaObjectWrapperObject);
ASSERT_CLASS_FITS_IN_CELL(QScript::QMetaObjectPrototype);
ASSERT_CLASS_FITS_IN_CELL(QScript::QtFunction);
ASSERT_CLASS_FITS_IN_CELL(QScript::QtPropertyFunction);
}

QT_BEGIN_NAMESPACE

namespace QScript {

struct QObjectConnection {
   int slotIndex;
   JSC::JSValue receiver;
   JSC::JSValue slot;
   JSC::JSValue senderWrapper;

   QObjectConnection(int i, JSC::JSValue r, JSC::JSValue s,
                     JSC::JSValue sw)
      : slotIndex(i), receiver(r), slot(s), senderWrapper(sw) {}
   QObjectConnection() : slotIndex(-1) {}

   bool hasTarget(JSC::JSValue r, JSC::JSValue s) const {
      if ((r && r.isObject()) != (receiver && receiver.isObject())) {
         return false;
      }

      if (((r && r.isObject()) && (receiver && receiver.isObject())) && (r != receiver)) {
         return false;
      }
      return (s == slot);
   }

   void mark(JSC::MarkStack &markStack) {
      if (senderWrapper) {
         // see if the sender should be marked or not;
         // if the C++ object is owned by script, we don't want
         // it to stay alive due to a script connection.
         Q_ASSERT(senderWrapper.inherits(&QScriptObject::info));

         QScriptObject *scriptObject = static_cast<QScriptObject *>(JSC::asObject(senderWrapper));

         if (!JSC::Heap::isCellMarked(scriptObject)) {
            QScriptObjectDelegate *delegate = scriptObject->delegate();
            Q_ASSERT(delegate && (delegate->type() == QScriptObjectDelegate::QtObject));
            QObjectDelegate *inst = static_cast<QObjectDelegate *>(delegate);

            if ((inst->ownership() == QScriptEngine::ScriptOwnership)
                  || ((inst->ownership() == QScriptEngine::AutoOwnership)
                      && inst->value() && !inst->value()->parent())) {
               senderWrapper = JSC::JSValue();
            } else {
               markStack.append(senderWrapper);
            }
         }
      }

      if (receiver) {
         markStack.append(receiver);
      }

      if (slot) {
         markStack.append(slot);
      }
   }
};

class QObjectNotifyCaller : public QObject
{
 public:
   void callConnectNotify(const char *signal) {
      connectNotify(signal);
   }

   void callDisconnectNotify(const char *signal) {
      disconnectNotify(signal);
   }
};

class QObjectConnectionManager: public QObject
{
 public:
   QObjectConnectionManager(QScriptEnginePrivate *engine);
   ~QObjectConnectionManager();

   bool addSignalHandler(QObject *sender, int signalIndex,
                         JSC::JSValue receiver,
                         JSC::JSValue slot,
                         JSC::JSValue senderWrapper,
                         Qt::ConnectionType type);

   bool removeSignalHandler(QObject *sender, int signalIndex,
                            JSC::JSValue receiver,
                            JSC::JSValue slot);

   static const QMetaObject &staticMetaObject();
   virtual const QMetaObject *metaObject() const;

   // qt_metacast()   removed 1/11/2014
   // qt_metacall()   removed 1/11/2014

   void execute(int slotIndex, void **argv);
   void mark(JSC::MarkStack &);

 private:
   QScriptEnginePrivate *engine;
   int slotCounter;
   QVector<QVector<QObjectConnection> > connections;
};

static bool hasMethodAccess(const QMetaMethod &method, int index, const QScriptEngine::QObjectWrapOptions &opt)
{
   return (method.access() != QMetaMethod::Private)
          && ((index != 2) || !(opt & QScriptEngine::ExcludeDeleteLater))
          && (!(opt & QScriptEngine::ExcludeSlots) || (method.methodType() != QMetaMethod::Slot));
}

static bool isEnumerableMetaProperty(const QMetaProperty &prop,
                                     const QMetaObject *mo, int index)
{
   return prop.isScriptable() && prop.isValid()
          // the following lookup is to ensure that we have the
          // "most derived" occurrence of the property with this name
          && (mo->indexOfProperty(prop.name()) == index);
}

/*! \internal
  Calculates the length of the name of the given \a method by looking
  for the first '(' character.
*/
static inline int methodNameLength(const QMetaMethod &method)
{

   QByteArray tempSignature = method.methodSignature();
   const char *signature    = tempSignature.constData();

   const char *s = signature;

   while (*s && (*s != '(')) {
      ++s;
   }
   return s - signature;
}

/*! \internal
  Makes a deep copy of the first \a nameLength characters of the given
  method \a signature and returns the copy.
*/
static inline QByteArray methodName(const char *signature, int nameLength)
{
   return QByteArray(signature, nameLength);
}

/*! \internal

  Returns true if the name of the given \a method is the same as that
  specified by the (signature, nameLength) pair, otherwise returns
  false.
*/
static inline bool methodNameEquals(const QMetaMethod &method, const char *signature, int nameLength)
{
   QByteArray tempSignature   = method.methodSignature();
   const char *otherSignature = tempSignature.constData();

   return ! qstrncmp(otherSignature, signature, nameLength) && (otherSignature[nameLength] == '(');
}

static QVariant variantFromValue(JSC::ExecState *exec, int targetType, JSC::JSValue value)
{
   QVariant v(targetType, (void *)0);

   if (QScriptEnginePrivate::convertValue(exec, value, targetType, v.data())) {
      return v;
   }

   if (uint(targetType) == QVariant::LastType) {
      return QScriptEnginePrivate::toVariant(exec, value);
   }

   if (QScriptEnginePrivate::isVariant(value)) {
      v = QScriptEnginePrivate::variantValue(value);
      if (v.canConvert(QVariant::Type(targetType))) {
         v.convert(QVariant::Type(targetType));
         return v;
      }

      QByteArray typeName = v.typeName();
      if (typeName.endsWith('*')
            && (QMetaType::type(typeName.left(typeName.size() - 1).constData()) == targetType)) {
         return QVariant(targetType, *reinterpret_cast<void **>(v.data()));
      }
   }

   return QVariant();
}

static const bool GeneratePropertyFunctions = true;

static unsigned flagsForMetaProperty(const QMetaProperty &prop)
{
   return (JSC::DontDelete
           | (!prop.isWritable() ? unsigned(JSC::ReadOnly) : unsigned(0))
           | (GeneratePropertyFunctions
              ? unsigned(JSC::Getter | JSC::Setter)
              : unsigned(0))
           | QObjectMemberAttribute);
}

static int indexOfMetaEnum(const QMetaObject *meta, const QByteArray &str)
{
   QByteArray scope;
   QByteArray name;
   int scopeIdx = str.lastIndexOf("::");
   if (scopeIdx != -1) {
      scope = str.left(scopeIdx);
      name = str.mid(scopeIdx + 2);
   } else {
      name = str;
   }
   for (int i = meta->enumeratorCount() - 1; i >= 0; --i) {
      QMetaEnum m = meta->enumerator(i);
      if ((m.name() == name) && (scope.isEmpty() || (m.scope() == scope))) {
         return i;
      }
   }
   return -1;
}

static inline QScriptable *scriptableFromQObject(QObject *qobj)
{

   /*  BROOM (script)
       void *ptr = qobj->qt_metacast("QScriptable");
   */
   void *ptr = nullptr;   // temp code

   return reinterpret_cast<QScriptable *>(ptr);
}

QtFunction::QtFunction(JSC::JSValue object, int initialIndex, bool maybeOverloaded,
                       JSC::JSGlobalData *data, WTF::PassRefPtr<JSC::Structure> sid, const JSC::Identifier &ident)
   : JSC::InternalFunction(data, sid, ident), data(new Data(object, initialIndex, maybeOverloaded))
{
}

QtFunction::~QtFunction()
{
   delete data;
}

JSC::CallType QtFunction::getCallData(JSC::CallData &callData)
{
   callData.native.function = call;
   return JSC::CallTypeHost;
}

void QtFunction::markChildren(JSC::MarkStack &markStack)
{
   if (data->object) {
      markStack.append(data->object);
   }
   JSC::InternalFunction::markChildren(markStack);
}

QScriptObject *QtFunction::wrapperObject() const
{
   Q_ASSERT(JSC::asObject(data->object)->inherits(&QScriptObject::info));
   return static_cast<QScriptObject *>(JSC::asObject(data->object));
}

QObject *QtFunction::qobject() const
{
   QScriptObject *scriptObject = wrapperObject();
   QScriptObjectDelegate *delegate = scriptObject->delegate();
   Q_ASSERT(delegate && (delegate->type() == QScriptObjectDelegate::QtObject));
   return static_cast<QScript::QObjectDelegate *>(delegate)->value();
}

const QMetaObject *QtFunction::metaObject() const
{
   QObject *qobj = qobject();
   if (!qobj) {
      return 0;
   }
   return qobj->metaObject();
}

int QtFunction::initialIndex() const
{
   return data->initialIndex;
}

bool QtFunction::maybeOverloaded() const
{
   return data->maybeOverloaded;
}

int QtFunction::mostGeneralMethod(QMetaMethod *out) const
{
   const QMetaObject *meta = metaObject();
   if (!meta) {
      return -1;
   }
   int index = initialIndex();
   QMetaMethod method = meta->method(index);
   if (maybeOverloaded() && (method.attributes() & QMetaMethod::Cloned)) {
      // find the most general method
      do {
         method = meta->method(--index);
      } while (method.attributes() & QMetaMethod::Cloned);
   }
   if (out) {
      *out = method;
   }
   return index;
}

QList<int> QScript::QtFunction::overloadedIndexes() const
{
   if (! maybeOverloaded()) {
      return QList<int>();
   }

   QList<int> result;
   const QMetaObject *meta = metaObject();
   QMetaMethod method = meta->method(initialIndex());

   int nameLength = methodNameLength(method);

   for (int index = mostGeneralMethod() - 1; index >= 0; --index) {
      if (methodNameEquals(meta->method(index), method.methodSignature().constData(), nameLength)) {
         result.append(index);
      }
   }
   return result;
}

class QScriptMetaType
{
 public:
   enum Kind {
      Invalid,
      Variant,
      MetaType,
      Unresolved,
      MetaEnum
   };

   inline QScriptMetaType()
      : m_kind(Invalid) { }

   inline Kind kind() const {
      return m_kind;
   }

   int typeId() const;

   inline bool isValid() const {
      return (m_kind != Invalid);
   }

   inline bool isVariant() const {
      return (m_kind == Variant);
   }

   inline bool isMetaType() const {
      return (m_kind == MetaType);
   }

   inline bool isUnresolved() const {
      return (m_kind == Unresolved);
   }

   inline bool isMetaEnum() const {
      return (m_kind == MetaEnum);
   }

   QByteArray name() const;

   inline int enumeratorIndex() const {
      Q_ASSERT(isMetaEnum());
      return m_typeId;
   }

   inline bool operator==(const QScriptMetaType &other) const {
      return (m_kind == other.m_kind) && (m_typeId == other.m_typeId);
   }

   static inline QScriptMetaType variant() {
      return QScriptMetaType(Variant);
   }

   static inline QScriptMetaType metaType(int typeId, const QByteArray &name) {
      return QScriptMetaType(MetaType, typeId, name);
   }

   static inline QScriptMetaType metaEnum(int enumIndex, const QByteArray &name) {
      return QScriptMetaType(MetaEnum, enumIndex, name);
   }

   static inline QScriptMetaType unresolved(const QByteArray &name) {
      return QScriptMetaType(Unresolved, /*typeId=*/0, name);
   }

 private:
   inline QScriptMetaType(Kind kind, int typeId = 0, const QByteArray &name = QByteArray())
      : m_kind(kind), m_typeId(typeId), m_name(name) { }

   Kind m_kind;
   int m_typeId;
   QByteArray m_name;
};

int QScriptMetaType::typeId() const
{
   if (isVariant()) {
      return QMetaType::type("QVariant");
   }
   return isMetaEnum() ? 2/*int*/ : m_typeId;
}

QByteArray QScriptMetaType::name() const
{
   if (!m_name.isEmpty()) {
      return m_name;
   } else if (m_kind == Variant) {
      return "QVariant";
   }
   return QMetaType::typeName(typeId());
}

class QScriptMetaMethod
{
 public:
   inline QScriptMetaMethod() {
   }
   inline QScriptMetaMethod(const QVector<QScriptMetaType> &types)
      : m_types(types), m_firstUnresolvedIndex(-1) {
      QVector<QScriptMetaType>::const_iterator it;
      for (it = m_types.constBegin(); it != m_types.constEnd(); ++it) {
         if ((*it).kind() == QScriptMetaType::Unresolved) {
            m_firstUnresolvedIndex = it - m_types.constBegin();
            break;
         }
      }
   }
   inline bool isValid() const {
      return !m_types.isEmpty();
   }

   inline QScriptMetaType returnType() const {
      return m_types.at(0);
   }

   inline int argumentCount() const {
      return m_types.count() - 1;
   }

   inline QScriptMetaType argumentType(int arg) const {
      return m_types.at(arg + 1);
   }

   inline bool fullyResolved() const {
      return m_firstUnresolvedIndex == -1;
   }

   inline bool hasUnresolvedReturnType() const {
      return (m_firstUnresolvedIndex == 0);
   }

   inline int firstUnresolvedIndex() const {
      return m_firstUnresolvedIndex;
   }

   inline int count() const {
      return m_types.count();
   }

   inline QScriptMetaType type(int index) const {
      return m_types.at(index);
   }

   inline QVector<QScriptMetaType> types() const {
      return m_types;
   }

 private:
   QVector<QScriptMetaType> m_types;
   int m_firstUnresolvedIndex;
};

struct QScriptMetaArguments {
   int matchDistance;
   int index;
   QScriptMetaMethod method;
   QVarLengthArray<QVariant, 9> args;

   inline QScriptMetaArguments(int dist, int idx, const QScriptMetaMethod &mtd,
                               const QVarLengthArray<QVariant, 9> &as)
      : matchDistance(dist), index(idx), method(mtd), args(as) { }
   inline QScriptMetaArguments()
      : index(-1) { }

   inline bool isValid() const {
      return (index != -1);
   }
};

static QMetaMethod metaMethod(const QMetaObject *meta,
                              QMetaMethod::MethodType type,
                              int index)
{
   if (type != QMetaMethod::Constructor) {
      return meta->method(index);
   } else {
      return meta->constructor(index);
   }
}

static JSC::JSValue callQtMethod(JSC::ExecState *exec, QMetaMethod::MethodType callType,
                                 QObject *thisQObject, const JSC::ArgList &scriptArgs,
                                 const QMetaObject *meta, int initialIndex, bool maybeOverloaded)
{
   QScriptMetaMethod chosenMethod;
   int chosenIndex = -1;

   QVarLengthArray<QVariant, 9> args;
   QVector<QScriptMetaArguments> candidates;
   QVector<QScriptMetaArguments> unresolved;
   QVector<int> tooFewArgs;
   QVector<int> conversionFailed;
   int index;
   int nameLength = 0;

   exec->clearException();
   QScriptEnginePrivate *engine = QScript::scriptEngineFromExec(exec);

   QByteArray tempSignature;
   const char *initialMethodSignature = 0;

   for (index = initialIndex; index >= 0; --index) {
      QMetaMethod method = metaMethod(meta, callType, index);

      if (index == initialIndex) {

         tempSignature          = method.methodSignature();
         initialMethodSignature = tempSignature.constData();

         nameLength = methodNameLength(method);

      } else {
         if (! methodNameEquals(method, initialMethodSignature, nameLength)) {
            continue;
         }
      }

      QList<QByteArray> parameterTypeNames = method.parameterTypes();

      QVector<QScriptMetaType> types;
      types.resize(1 + parameterTypeNames.size());

      QScriptMetaType *typesData = types.data();

      // resolve return type
      QByteArray returnTypeName = method.typeName();
      int rtype = QMetaType::type(returnTypeName.constData());

      if ((rtype == 0) && !returnTypeName.isEmpty()) {
         int enumIndex = indexOfMetaEnum(meta, returnTypeName);

         if (enumIndex != -1) {
            typesData[0] = QScriptMetaType::metaEnum(enumIndex, returnTypeName);
         } else {
            typesData[0] = QScriptMetaType::unresolved(returnTypeName);
         }

      } else {
         if (callType == QMetaMethod::Constructor) {
            typesData[0] = QScriptMetaType::metaType(QMetaType::QObjectStar, "QObject*");
         } else if (rtype == QMetaType::QVariant) {
            typesData[0] = QScriptMetaType::variant();
         } else {
            typesData[0] = QScriptMetaType::metaType(rtype, returnTypeName);
         }
      }

      // resolve argument types
      for (int i = 0; i < parameterTypeNames.count(); ++i) {
         QByteArray argTypeName = parameterTypeNames.at(i);
         int atype = QMetaType::type(argTypeName.constData());

         if (atype == 0) {
            int enumIndex = indexOfMetaEnum(meta, argTypeName);

            if (enumIndex != -1) {
               typesData[1 + i] = QScriptMetaType::metaEnum(enumIndex, argTypeName);
            } else {
               typesData[1 + i] = QScriptMetaType::unresolved(argTypeName);
            }

         } else if (atype == QMetaType::QVariant) {
            typesData[1 + i] = QScriptMetaType::variant();
         } else {
            typesData[1 + i] = QScriptMetaType::metaType(atype, argTypeName);
         }
      }

      QScriptMetaMethod mtd = QScriptMetaMethod(types);

      if (int(scriptArgs.size()) < mtd.argumentCount()) {
         tooFewArgs.append(index);
         continue;
      }

      if (!mtd.fullyResolved()) {
         // remember it so we can give an error message later, if necessary
         unresolved.append(QScriptMetaArguments(INT_MAX, index, mtd, QVarLengthArray<QVariant, 9>()));

         if (mtd.hasUnresolvedReturnType()) {
            continue;
         }
      }

      if (args.count() != mtd.count()) {
         args.resize(mtd.count());
      }

      QScriptMetaType retType = mtd.returnType();
      args[0] = QVariant(retType.typeId(), (void *)0); // the result

      // try to convert arguments
      bool converted = true;
      int matchDistance = 0;
      for (int i = 0; converted && i < mtd.argumentCount(); ++i) {
         JSC::JSValue actual;
         if (i < (int)scriptArgs.size()) {
            actual = scriptArgs.at(i);
         } else {
            actual = JSC::jsUndefined();
         }
         QScriptMetaType argType = mtd.argumentType(i);
         int tid = -1;
         QVariant v;
         if (argType.isUnresolved()) {
            v = QVariant(QMetaType::QObjectStar, (void *)0);
            converted = QScriptEnginePrivate::convertToNativeQObject(
                           exec, actual, argType.name(), reinterpret_cast<void **>(v.data()));
         } else if (argType.isVariant()) {
            if (QScriptEnginePrivate::isVariant(actual)) {
               v = QScriptEnginePrivate::variantValue(actual);
            } else {
               v = QScriptEnginePrivate::toVariant(exec, actual);
               converted = v.isValid() || actual.isUndefined() || actual.isNull();
            }
         } else {
            tid = argType.typeId();
            v = QVariant(tid, (void *)0);
            converted = QScriptEnginePrivate::convertValue(exec, actual, tid, v.data());
            if (exec->hadException()) {
               return exec->exception();
            }
         }

         if (!converted) {
            if (QScriptEnginePrivate::isVariant(actual)) {
               if (tid == -1) {
                  tid = argType.typeId();
               }
               QVariant vv = QScriptEnginePrivate::variantValue(actual);
               if (vv.canConvert(QVariant::Type(tid))) {
                  v = vv;
                  converted = v.convert(QVariant::Type(tid));
                  if (converted && (vv.userType() != tid)) {
                     matchDistance += 10;
                  }
               } else {
                  QByteArray vvTypeName = vv.typeName();
                  if (vvTypeName.endsWith('*')
                        && (vvTypeName.left(vvTypeName.size() - 1) == argType.name())) {
                     v = QVariant(tid, *reinterpret_cast<void **>(vv.data()));
                     converted = true;
                     matchDistance += 10;
                  }
               }
            } else if (actual.isNumber() || actual.isString()) {
               // see if it's an enum value
               QMetaEnum m;
               if (argType.isMetaEnum()) {
                  m = meta->enumerator(argType.enumeratorIndex());
               } else {
                  int mi = indexOfMetaEnum(meta, argType.name());
                  if (mi != -1) {
                     m = meta->enumerator(mi);
                  }
               }
               if (m.isValid()) {
                  if (actual.isNumber()) {
                     int ival = QScriptEnginePrivate::toInt32(exec, actual);

                     if (m.valueToKey(ival) != 0) {
                        v.setValue(ival);
                        converted = true;
                        matchDistance += 10;
                     }

                  } else {
                     JSC::UString sval = QScriptEnginePrivate::toString(exec, actual);
                     int ival = m.keyToValue(convertToLatin1(sval).constData());

                     if (ival != -1) {
                        v.setValue(ival);
                        converted = true;
                        matchDistance += 10;
                     }
                  }
               }
            }

         } else {
            // determine how well the conversion matched
            if (actual.isNumber()) {
               switch (tid) {
                  case QMetaType::Double:
                     // perfect
                     break;
                  case QMetaType::Float:
                     matchDistance += 1;
                     break;
                  case QMetaType::LongLong:
                  case QMetaType::ULongLong:
                     matchDistance += 2;
                     break;
                  case QMetaType::Long:
                  case QMetaType::ULong:
                     matchDistance += 3;
                     break;
                  case QMetaType::Int:
                  case QMetaType::UInt:
                     matchDistance += 4;
                     break;
                  case QMetaType::Short:
                  case QMetaType::UShort:
                     matchDistance += 5;
                     break;
                  case QMetaType::Char:
                  case QMetaType::UChar:
                     matchDistance += 6;
                     break;
                  default:
                     matchDistance += 10;
                     break;
               }
            } else if (actual.isString()) {
               switch (tid) {
                  case QMetaType::QString:
                     // perfect
                     break;
                  default:
                     matchDistance += 10;
                     break;
               }
            } else if (actual.isBoolean()) {
               switch (tid) {
                  case QMetaType::Bool:
                     // perfect
                     break;
                  default:
                     matchDistance += 10;
                     break;
               }
            } else if (QScriptEnginePrivate::isDate(actual)) {
               switch (tid) {
                  case QMetaType::QDateTime:
                     // perfect
                     break;
                  case QMetaType::QDate:
                     matchDistance += 1;
                     break;
                  case QMetaType::QTime:
                     matchDistance += 2;
                     break;
                  default:
                     matchDistance += 10;
                     break;
               }
            } else if (QScriptEnginePrivate::isRegExp(actual)) {
               switch (tid) {
                  case QMetaType::QRegExp:
                     // perfect
                     break;
                  default:
                     matchDistance += 10;
                     break;
               }
            } else if (QScriptEnginePrivate::isVariant(actual)) {
               if (argType.isVariant()
                     || (QScriptEnginePrivate::toVariant(exec, actual).userType() == tid)) {
                  // perfect
               } else {
                  matchDistance += 10;
               }
            } else if (QScriptEnginePrivate::isArray(actual)) {
               switch (tid) {
                  case QMetaType::QStringList:
                  case QMetaType::QVariantList:
                     matchDistance += 5;
                     break;
                  default:
                     matchDistance += 10;
                     break;
               }
            } else if (QScriptEnginePrivate::isQObject(actual)) {
               switch (tid) {
                  case QMetaType::QObjectStar:
                  case QMetaType::QWidgetStar:
                     // perfect
                     break;
                  default:
                     matchDistance += 10;
                     break;
               }
            } else if (actual.isNull()) {
               switch (tid) {
                  case QMetaType::VoidStar:
                  case QMetaType::QObjectStar:
                  case QMetaType::QWidgetStar:
                     // perfect
                     break;
                  default:
                     if (!argType.name().endsWith('*')) {
                        matchDistance += 10;
                     }
                     break;
               }
            } else {
               matchDistance += 10;
            }
         }

         if (converted) {
            args[i + 1] = v;
         }
      }

      if (converted) {
         if ((scriptArgs.size() == (size_t)mtd.argumentCount())
               && (matchDistance == 0)) {
            // perfect match, use this one
            chosenMethod = mtd;
            chosenIndex = index;
            break;
         } else {
            bool redundant = false;
            if ((callType != QMetaMethod::Constructor)
                  && (index < meta->methodOffset())) {
               // it is possible that a virtual method is redeclared in a subclass,
               // in which case we want to ignore the superclass declaration
               for (int i = 0; i < candidates.size(); ++i) {
                  const QScriptMetaArguments &other = candidates.at(i);
                  if (mtd.types() == other.method.types()) {
                     redundant = true;
                     break;
                  }
               }
            }
            if (!redundant) {
               QScriptMetaArguments metaArgs(matchDistance, index, mtd, args);
               if (candidates.isEmpty()) {
                  candidates.append(metaArgs);
               } else {
                  const QScriptMetaArguments &otherArgs = candidates.at(0);
                  if ((args.count() > otherArgs.args.count())
                        || ((args.count() == otherArgs.args.count())
                            && (matchDistance <= otherArgs.matchDistance))) {
                     candidates.prepend(metaArgs);
                  } else {
                     candidates.append(metaArgs);
                  }
               }
            }
         }
      } else if (mtd.fullyResolved()) {
         conversionFailed.append(index);
      }

      if (!maybeOverloaded) {
         break;
      }
   }

   JSC::JSValue result;
   if ((chosenIndex == -1) && candidates.isEmpty()) {
      //        context->calleeMetaIndex = initialIndex;
      //#ifndef Q_SCRIPT_NO_EVENT_NOTIFY
      //        engine->notifyFunctionEntry(context);
      //#endif

      QString funName = QString::fromLatin1(methodName(initialMethodSignature, nameLength));
      if (!conversionFailed.isEmpty()) {
         QString message = QString::fromLatin1("incompatible type of argument(s) in call to %0(); candidates were\n")
                           .arg(funName);

         for (int i = 0; i < conversionFailed.size(); ++i) {
            if (i > 0) {
               message += QLatin1String("\n");
            }
            QMetaMethod mtd = metaMethod(meta, callType, conversionFailed.at(i));
            message += QString::fromLatin1("    %0").arg(QString::fromLatin1(mtd.methodSignature().constData()));
         }
         result = JSC::throwError(exec, JSC::TypeError, message);

      } else if (!unresolved.isEmpty()) {
         QScriptMetaArguments argsInstance = unresolved.first();
         int unresolvedIndex = argsInstance.method.firstUnresolvedIndex();
         Q_ASSERT(unresolvedIndex != -1);

         QScriptMetaType unresolvedType = argsInstance.method.type(unresolvedIndex);
         QString unresolvedTypeName = QString::fromLatin1(unresolvedType.name());
         QString message = QString::fromLatin1("cannot call %0(): ").arg(funName);

         if (unresolvedIndex > 0) {
            message.append(QString::fromLatin1("argument %0 has unknown type `%1'").
                           arg(unresolvedIndex).arg(unresolvedTypeName));

         } else {
            message.append(QString::fromLatin1("unknown return type `%0'")
                           .arg(unresolvedTypeName));

         }
         message.append(QString::fromLatin1(" (register the type with qScriptRegisterMetaType())"));
         result = JSC::throwError(exec, JSC::TypeError, message);

      } else {
         QString message = QString::fromLatin1("too few arguments in call to %0(); candidates are\n").arg(funName);

         for (int i = 0; i < tooFewArgs.size(); ++i) {
            if (i > 0) {
               message += QLatin1String("\n");
            }
            QMetaMethod mtd = metaMethod(meta, callType, tooFewArgs.at(i));
            message += QString::fromLatin1("    %0").arg(mtd.methodSignature().constData() );
         }
         result = JSC::throwError(exec, JSC::SyntaxError, message);
      }

   } else {
      if (chosenIndex == -1) {
         QScriptMetaArguments metaArgs = candidates.at(0);
         if ((candidates.size() > 1) && (metaArgs.args.count() == candidates.at(1).args.count())
               && (metaArgs.matchDistance == candidates.at(1).matchDistance)) {

            // ambiguous call
            QByteArray funName = methodName(initialMethodSignature, nameLength);
            QString message = QString::fromLatin1("Ambiguous call of overloaded function %0(); candidates were\n")
                              .arg(QString::fromLatin1(funName));

            for (int i = 0; i < candidates.size(); ++i) {
               if (i > 0) {
                  message += QLatin1String("\n");
               }

               QMetaMethod mtd = metaMethod(meta, callType, candidates.at(i).index);
               message += QString::fromLatin1("    %0").arg( mtd.methodSignature().constData() );
            }

            result = JSC::throwError(exec, JSC::TypeError, message);

         } else {
            chosenMethod = metaArgs.method;
            chosenIndex = metaArgs.index;
            args = metaArgs.args;
         }
      }

      if (chosenIndex != -1) {
         // call it
         // context->calleeMetaIndex = chosenIndex;

         QVarLengthArray<void *, 9> array(args.count());
         void **params = array.data();
         for (int i = 0; i < args.count(); ++i) {
            const QVariant &v = args[i];
            switch (chosenMethod.type(i).kind()) {
               case QScriptMetaType::Variant:
                  params[i] = const_cast<QVariant *>(&v);
                  break;
               case QScriptMetaType::MetaType:
               case QScriptMetaType::MetaEnum:
               case QScriptMetaType::Unresolved:
                  params[i] = const_cast<void *>(v.constData());
                  break;
               default:
                  Q_ASSERT(0);
            }
         }

         QScriptable *scriptable = 0;
         if (thisQObject) {
            scriptable = scriptableFromQObject(thisQObject);
         }
         QScriptEngine *oldEngine = 0;
         if (scriptable) {
            oldEngine = QScriptablePrivate::get(scriptable)->engine;
            QScriptablePrivate::get(scriptable)->engine = QScriptEnginePrivate::get(engine);
         }

         // ### fixme
         //#ifndef Q_SCRIPT_NO_EVENT_NOTIFY
         //            engine->notifyFunctionEntry(context);
         //#endif

         if (callType == QMetaMethod::Constructor) {
            Q_ASSERT(meta != 0);

            /*  BROOM (script)
                            meta->static_metacall(QMetaObject::CreateInstance, chosenIndex, params);
            */


         } else {

            /* BROOM (script)
                            QMetaObject::metacall(thisQObject, QMetaObject::InvokeMetaMethod, chosenIndex, params);
            */
         }

         if (scriptable) {
            QScriptablePrivate::get(scriptable)->engine = oldEngine;
         }

         if (exec->hadException()) {
            result = exec->exception() ; // propagate
         } else {
            QScriptMetaType retType = chosenMethod.returnType();
            if (retType.isVariant()) {
               result = QScriptEnginePrivate::jscValueFromVariant(exec, *(QVariant *)params[0]);
            } else if (retType.typeId() != 0) {
               result = QScriptEnginePrivate::create(exec, retType.typeId(), params[0]);
               if (!result) {
                  result = engine->newVariant(QVariant(retType.typeId(), params[0]));
               }
            } else {
               result = JSC::jsUndefined();
            }
         }
      }
   }

   return result;
}

JSC::JSValue QtFunction::execute(JSC::ExecState *exec, JSC::JSValue thisValue,
                                 const JSC::ArgList &scriptArgs)
{
   Q_ASSERT(data->object.inherits(&QScriptObject::info));

   QScriptObject *scriptObject = static_cast<QScriptObject *>(JSC::asObject(data->object));
   QScriptObjectDelegate *delegate = scriptObject->delegate();

   Q_ASSERT(delegate && (delegate->type() == QScriptObjectDelegate::QtObject));

   QObject *qobj = static_cast<QScript::QObjectDelegate *>(delegate)->value();

   if (!qobj) {
      return JSC::throwError(exec, JSC::GeneralError, QString::fromLatin1("cannot call function of deleted QObject"));
   }

   QScriptEnginePrivate *engine = scriptEngineFromExec(exec);

   const QMetaObject *meta = qobj->metaObject();
   QObject *thisQObject    = 0;

   thisValue = engine->toUsableValue(thisValue);

   if (thisValue.inherits(&QScriptObject::info)) {
      delegate = static_cast<QScriptObject *>(JSC::asObject(thisValue))->delegate();

      if (delegate && (delegate->type() == QScriptObjectDelegate::QtObject)) {
         thisQObject = static_cast<QScript::QObjectDelegate *>  (delegate)->value();
      }
   }

   if (! thisQObject)   {
      thisQObject = qobj; // ### TypeError
   }

   //
   const QMetaObject *temp = thisQObject->metaObject();

   while (temp != nullptr) {

      if (temp == meta) {
         // invoking a function in the prototype
         thisQObject = qobj;
         break;
      }

      temp = temp->superClass();
   }

   return callQtMethod(exec, QMetaMethod::Method, thisQObject, scriptArgs, meta, data->initialIndex,
                       data->maybeOverloaded);
}

const JSC::ClassInfo QtFunction::info = { "QtFunction", &InternalFunction::info, 0, 0 };

JSC::JSValue JSC_HOST_CALL QtFunction::call(JSC::ExecState *exec, JSC::JSObject *callee,
      JSC::JSValue thisValue, const JSC::ArgList &args)
{
   if (!callee->inherits(&QtFunction::info)) {
      return throwError(exec, JSC::TypeError, "callee is not a QtFunction object");
   }
   QtFunction *qfun =  static_cast<QtFunction *>(callee);
   QScriptEnginePrivate *eng_p = scriptEngineFromExec(exec);
   JSC::ExecState *previousFrame = eng_p->currentFrame;
   eng_p->currentFrame = exec;
   eng_p->pushContext(exec, thisValue, args, callee);
   JSC::JSValue result = qfun->execute(eng_p->currentFrame, thisValue, args);
   eng_p->popContext();
   eng_p->currentFrame = previousFrame;
   return result;
}

const JSC::ClassInfo QtPropertyFunction::info = { "QtPropertyFunction", &InternalFunction::info, 0, 0 };

QtPropertyFunction::QtPropertyFunction(const QMetaObject *meta, int index,
                                       JSC::JSGlobalData *data,
                                       WTF::PassRefPtr<JSC::Structure> sid,
                                       const JSC::Identifier &ident)
   : JSC::InternalFunction(data, sid, ident),
     data(new Data(meta, index))
{
}

QtPropertyFunction::~QtPropertyFunction()
{
   delete data;
}

JSC::CallType QtPropertyFunction::getCallData(JSC::CallData &callData)
{
   callData.native.function = call;
   return JSC::CallTypeHost;
}

JSC::JSValue JSC_HOST_CALL QtPropertyFunction::call(
   JSC::ExecState *exec, JSC::JSObject *callee,
   JSC::JSValue thisValue, const JSC::ArgList &args)
{
   if (!callee->inherits(&QtPropertyFunction::info)) {
      return throwError(exec, JSC::TypeError, "callee is not a QtPropertyFunction object");
   }
   QtPropertyFunction *qfun =  static_cast<QtPropertyFunction *>(callee);
   return qfun->execute(exec, thisValue, args);
}

JSC::JSValue QtPropertyFunction::execute(JSC::ExecState *exec,
      JSC::JSValue thisValue,
      const JSC::ArgList &args)
{
   JSC::JSValue result = JSC::jsUndefined();

   QScriptEnginePrivate *engine = scriptEngineFromExec(exec);
   JSC::ExecState *previousFrame = engine->currentFrame;
   engine->currentFrame = exec;

   JSC::JSValue qobjectValue = engine->toUsableValue(thisValue);
   QObject *qobject = QScriptEnginePrivate::toQObject(exec, qobjectValue);
   while ((!qobject || (qobject->metaObject() != data->meta))
          && JSC::asObject(qobjectValue)->prototype().isObject()) {
      qobjectValue = JSC::asObject(qobjectValue)->prototype();
      qobject = QScriptEnginePrivate::toQObject(exec, qobjectValue);
   }
   Q_ASSERT_X(qobject, Q_FUNC_INFO, "this-object must be a QObject");

   QMetaProperty prop = data->meta->property(data->index);
   Q_ASSERT(prop.isScriptable());
   if (args.size() == 0) {
      // get
      if (prop.isValid()) {
         QScriptable *scriptable = scriptableFromQObject(qobject);
         QScriptEngine *oldEngine = 0;
         if (scriptable) {
            engine->pushContext(exec, thisValue, args, this);
            oldEngine = QScriptablePrivate::get(scriptable)->engine;
            QScriptablePrivate::get(scriptable)->engine = QScriptEnginePrivate::get(engine);
         }

         QVariant v = prop.read(qobject);

         if (scriptable) {
            QScriptablePrivate::get(scriptable)->engine = oldEngine;
            engine->popContext();
         }

         result = QScriptEnginePrivate::jscValueFromVariant(exec, v);
      }
   } else {
      // set
      JSC::JSValue arg = args.at(0);
      QVariant v;
      if (prop.isEnumType() && arg.isString()
            && !engine->hasDemarshalFunction(prop.userType())) {
         // give QMetaProperty::write() a chance to convert from
         // string to enum value
         v = (QString)arg.toString(exec);
      } else {
         v = variantFromValue(exec, prop.userType(), arg);
      }

      QScriptable *scriptable = scriptableFromQObject(qobject);
      QScriptEngine *oldEngine = 0;
      if (scriptable) {
         engine->pushContext(exec, thisValue, args, this);
         oldEngine = QScriptablePrivate::get(scriptable)->engine;
         QScriptablePrivate::get(scriptable)->engine = QScriptEnginePrivate::get(engine);
      }

      prop.write(qobject, v);

      if (scriptable) {
         QScriptablePrivate::get(scriptable)->engine = oldEngine;
         engine->popContext();
      }

      result = arg;
   }
   engine->currentFrame = previousFrame;
   return result;
}

const QMetaObject *QtPropertyFunction::metaObject() const
{
   return data->meta;
}

int QtPropertyFunction::propertyIndex() const
{
   return data->index;
}


QObjectDelegate::QObjectDelegate(
   QObject *object, QScriptEngine::ValueOwnership ownership,
   const QScriptEngine::QObjectWrapOptions &options)
   : data(new Data(object, ownership, options))
{
}

QObjectDelegate::~QObjectDelegate()
{
   switch (data->ownership) {
      case QScriptEngine::QtOwnership:
         break;
      case QScriptEngine::ScriptOwnership:
         if (data->value) {
            delete data->value;   // ### fixme
         }
         //            eng->disposeQObject(value);
         break;
      case QScriptEngine::AutoOwnership:
         if (data->value && !data->value->parent()) {
            delete data->value;   // ### fixme
         }
         //            eng->disposeQObject(value);
         break;
   }
   delete data;
}

QScriptObjectDelegate::Type QObjectDelegate::type() const
{
   return QtObject;
}

bool QObjectDelegate::getOwnPropertySlot(QScriptObject *object, JSC::ExecState *exec,
      const JSC::Identifier &propertyName,
      JSC::PropertySlot &slot)
{
   //Note: this has to be kept in sync with getOwnPropertyDescriptor
#ifndef QT_NO_PROPERTIES
   QByteArray name = convertToLatin1(propertyName.ustring());
   QObject *qobject = data->value;

   if (!qobject) {
      QString message = QString::fromLatin1("cannot access member `%0' of deleted QObject")
                        .arg(QString::fromLatin1(name));
      slot.setValue(JSC::throwError(exec, JSC::GeneralError, message));
      return true;
   }

   const QMetaObject *meta = qobject->metaObject();
   {
      QHash<QByteArray, JSC::JSValue>::const_iterator it = data->cachedMembers.constFind(name);

      if (it != data->cachedMembers.constEnd()) {
         if (GeneratePropertyFunctions && (meta->indexOfProperty(name.constData()) != -1)) {
            slot.setGetterSlot(JSC::asObject(it.value()));
         } else {
            slot.setValue(it.value());
         }

         return true;
      }
   }

   const QScriptEngine::QObjectWrapOptions &opt = data->options;
   QScriptEnginePrivate *eng = scriptEngineFromExec(exec);
   int index = -1;

   if (name.contains('(')) {
      QByteArray normalized = QMetaObject::normalizedSignature(name.constData());

      if (-1 != (index = meta->indexOfMethod(normalized.constData()))) {
         QMetaMethod method = meta->method(index);

         if (hasMethodAccess(method, index, opt)) {
            if (!(opt & QScriptEngine::ExcludeSuperClassMethods) || (index >= meta->methodOffset())) {

               QtFunction *fun = new (exec)QtFunction(object, index, false,
                  &exec->globalData(), eng->originalGlobalObject()->functionStructure(), propertyName);

               slot.setValue(fun);
               data->cachedMembers.insert(name, fun);

               return true;
            }
         }
      }
   }

   index = meta->indexOfProperty(name.constData());

   if (index != -1) {
      QMetaProperty prop = meta->property(index);

      if (prop.isScriptable()) {
         if (! (opt & QScriptEngine::ExcludeSuperClassProperties) || (index >= meta->propertyOffset())) {

            if (GeneratePropertyFunctions) {
               QtPropertyFunction *fun = new (exec)QtPropertyFunction(meta, index, &exec->globalData(),
                  eng->originalGlobalObject()->functionStructure(), propertyName);

               data->cachedMembers.insert(name, fun);
               slot.setGetterSlot(fun);

            } else {
               JSC::JSValue val;

               if (! prop.isValid()) {
                  val = JSC::jsUndefined();
               } else {
                  val = QScriptEnginePrivate::jscValueFromVariant(exec, prop.read(qobject));
               }

               slot.setValue(val);
            }
            return true;
         }
      }
   }

   index = qobject->dynamicPropertyNames().indexOf(name);
   if (index != -1) {
      JSC::JSValue val = QScriptEnginePrivate::jscValueFromVariant(exec, qobject->property(name));
      slot.setValue(val);
      return true;
   }

   const int offset = (opt & QScriptEngine::ExcludeSuperClassMethods)
                      ? meta->methodOffset() : 0;
   for (index = meta->methodCount() - 1; index >= offset; --index) {
      QMetaMethod method = meta->method(index);
      if (hasMethodAccess(method, index, opt)
            && methodNameEquals(method, name.constData(), name.length())) {
         QtFunction *fun = new (exec)QtFunction(
            object, index, /*maybeOverloaded=*/true,
            &exec->globalData(), eng->originalGlobalObject()->functionStructure(),
            propertyName);
         slot.setValue(fun);
         data->cachedMembers.insert(name, fun);
         return true;
      }
   }

   if (!(opt & QScriptEngine::ExcludeChildObjects)) {
      QList<QObject *> children = qobject->children();
      for (index = 0; index < children.count(); ++index) {
         QObject *child = children.at(index);
         if (child->objectName() == QString(propertyName.ustring())) {
            QScriptEngine::QObjectWrapOptions opt = QScriptEngine::PreferExistingWrapperObject;
            slot.setValue(eng->newQObject(child, QScriptEngine::QtOwnership, opt));
            return true;
         }
      }
   }

   return QScriptObjectDelegate::getOwnPropertySlot(object, exec, propertyName, slot);
#else //QT_NO_PROPERTIES
   return false;
#endif //QT_NO_PROPERTIES
}


bool QObjectDelegate::getOwnPropertyDescriptor(QScriptObject *object, JSC::ExecState *exec,
      const JSC::Identifier &propertyName,
      JSC::PropertyDescriptor &descriptor)
{
   //Note: this has to be kept in sync with getOwnPropertySlot
#ifndef QT_NO_PROPERTIES
   QByteArray name = convertToLatin1(propertyName.ustring());
   QObject *qobject = data->value;
   if (!qobject) {
      QString message = QString::fromLatin1("cannot access member `%0' of deleted QObject")
                        .arg(QString::fromLatin1(name));
      descriptor.setValue(JSC::throwError(exec, JSC::GeneralError, message));
      return true;
   }

   const QScriptEngine::QObjectWrapOptions &opt = data->options;

   const QMetaObject *meta = qobject->metaObject();
   {
      QHash<QByteArray, JSC::JSValue>::const_iterator it = data->cachedMembers.constFind(name);
      if (it != data->cachedMembers.constEnd()) {
         int index;

         if (GeneratePropertyFunctions && ((index = meta->indexOfProperty(name.constData())) != -1)) {
            QMetaProperty prop = meta->property(index);
            descriptor.setAccessorDescriptor(it.value(), it.value(), flagsForMetaProperty(prop));

            if (! prop.isWritable()) {
               descriptor.setWritable(false);
            }

         } else {
            unsigned attributes = QObjectMemberAttribute;
            if (opt & QScriptEngine::SkipMethodsInEnumeration) {
               attributes |= JSC::DontEnum;
            }
            descriptor.setDescriptor(it.value(), attributes);
         }
         return true;
      }
   }

   QScriptEnginePrivate *eng = scriptEngineFromExec(exec);
   int index = -1;

   if (name.contains('(')) {
      QByteArray normalized = QMetaObject::normalizedSignature(name.constData());

      if (-1 != (index = meta->indexOfMethod(normalized.constData()))) {
         QMetaMethod method = meta->method(index);

         if (hasMethodAccess(method, index, opt)) {
            if (!(opt & QScriptEngine::ExcludeSuperClassMethods) || (index >= meta->methodOffset())) {
               QtFunction *fun = new (exec)QtFunction(object, index, false,
                  &exec->globalData(), eng->originalGlobalObject()->functionStructure(), propertyName);

               data->cachedMembers.insert(name, fun);
               unsigned attributes = QObjectMemberAttribute;

               if (opt & QScriptEngine::SkipMethodsInEnumeration) {
                  attributes |= JSC::DontEnum;
               }
               descriptor.setDescriptor(fun, attributes);
               return true;
            }
         }
      }
   }

   index = meta->indexOfProperty(name.constData());

   if (index != -1) {
      QMetaProperty prop = meta->property(index);

      if (prop.isScriptable()) {
         if (! (opt & QScriptEngine::ExcludeSuperClassProperties) || (index >= meta->propertyOffset())) {
            unsigned attributes = flagsForMetaProperty(prop);

            if (GeneratePropertyFunctions) {
               QtPropertyFunction *fun = new (exec)QtPropertyFunction(meta, index, &exec->globalData(),
                  eng->originalGlobalObject()->functionStructure(), propertyName);

               data->cachedMembers.insert(name, fun);
               descriptor.setAccessorDescriptor(fun, fun, attributes);

               if (attributes & JSC::ReadOnly) {
                  descriptor.setWritable(false);
               }

            } else {
               JSC::JSValue val;
               if (!prop.isValid()) {
                  val = JSC::jsUndefined();
               } else {
                  val = QScriptEnginePrivate::jscValueFromVariant(exec, prop.read(qobject));
               }
               descriptor.setDescriptor(val, attributes);
            }
            return true;
         }
      }
   }

   index = qobject->dynamicPropertyNames().indexOf(name);
   if (index != -1) {
      JSC::JSValue val = QScriptEnginePrivate::jscValueFromVariant(exec, qobject->property(name));
      descriptor.setDescriptor(val, QObjectMemberAttribute);
      return true;
   }

   const int offset = (opt & QScriptEngine::ExcludeSuperClassMethods)
                      ? meta->methodOffset() : 0;
   for (index = meta->methodCount() - 1; index >= offset; --index) {
      QMetaMethod method = meta->method(index);
      if (hasMethodAccess(method, index, opt)
            && methodNameEquals(method, name.constData(), name.length())) {
         QtFunction *fun = new (exec)QtFunction(
            object, index, /*maybeOverloaded=*/true,
            &exec->globalData(), eng->originalGlobalObject()->functionStructure(),
            propertyName);
         unsigned attributes = QObjectMemberAttribute;
         if (opt & QScriptEngine::SkipMethodsInEnumeration) {
            attributes |= JSC::DontEnum;
         }
         descriptor.setDescriptor(fun, attributes);
         data->cachedMembers.insert(name, fun);
         return true;
      }
   }

   if (!(opt & QScriptEngine::ExcludeChildObjects)) {
      QList<QObject *> children = qobject->children();
      for (index = 0; index < children.count(); ++index) {
         QObject *child = children.at(index);
         if (child->objectName() == QString(propertyName.ustring())) {
            QScriptEngine::QObjectWrapOptions opt = QScriptEngine::PreferExistingWrapperObject;
            descriptor.setDescriptor(eng->newQObject(child, QScriptEngine::QtOwnership, opt),
                                     JSC::ReadOnly | JSC::DontDelete | JSC::DontEnum);
            return true;
         }
      }
   }

   return QScriptObjectDelegate::getOwnPropertyDescriptor(object, exec, propertyName, descriptor);
#else //QT_NO_PROPERTIES
   return false;

#endif //QT_NO_PROPERTIES
}

void QObjectDelegate::put(QScriptObject *object, JSC::ExecState *exec,
                  const JSC::Identifier &propertyName, JSC::JSValue value, JSC::PutPropertySlot &slot)
{
#ifndef QT_NO_PROPERTIES
   QByteArray name = convertToLatin1(propertyName.ustring());
   QObject *qobject = data->value;

   if (! qobject) {
      QString message = QString::fromLatin1("cannot access member `%0' of deleted QObject")
                        .arg(QString::fromLatin1(name));
      JSC::throwError(exec, JSC::GeneralError, message);
      return;
   }

   const QScriptEngine::QObjectWrapOptions &opt = data->options;
   const QMetaObject *meta = qobject->metaObject();

   QScriptEnginePrivate *eng = scriptEngineFromExec(exec);
   int index = -1;

   if (name.contains('(')) {
      QByteArray normalized = QMetaObject::normalizedSignature(name.constData());
      if (-1 != (index = meta->indexOfMethod(normalized.constData()))) {
         QMetaMethod method = meta->method(index);

         if (hasMethodAccess(method, index, opt)) {
            if (!(opt & QScriptEngine::ExcludeSuperClassMethods) || (index >= meta->methodOffset())) {
               data->cachedMembers.insert(name, value);
               return;
            }
         }
      }
   }

   index = meta->indexOfProperty(name.constData());

   if (index != -1) {
      QMetaProperty prop = meta->property(index);

      if (prop.isScriptable()) {

         if (!(opt & QScriptEngine::ExcludeSuperClassProperties) || (index >= meta->propertyOffset())) {
            if (GeneratePropertyFunctions) {
               // ### ideally JSC would do this for us already, i.e. find out
               // that the property is a setter and call the setter.
               // Maybe QtPropertyFunction needs to inherit JSC::GetterSetter.
               JSC::JSValue fun;

               QHash<QByteArray, JSC::JSValue>::const_iterator it;
               it = data->cachedMembers.constFind(name);

               if (it != data->cachedMembers.constEnd()) {
                  fun = it.value();

               } else {
                  fun = new (exec)QtPropertyFunction(meta, index, &exec->globalData(),
                     eng->originalGlobalObject()->functionStructure(), propertyName);

                  data->cachedMembers.insert(name, fun);
               }
               JSC::CallData callData;
               JSC::CallType callType = fun.getCallData(callData);
               JSC::JSValue argv[1] = { value };
               JSC::ArgList args(argv, 1);
               (void)JSC::call(exec, fun, callType, callData, object, args);

            } else {
               QVariant v;

               if (prop.isEnumType() && value.isString() && !eng->hasDemarshalFunction(prop.userType())) {
                  // give QMetaProperty::write() a chance to convert from
                  // string to enum value
                  v = (QString)value.toString(exec);

               } else {
                  v = QScriptEnginePrivate::jscValueToVariant(exec, value, prop.userType());
               }

               (void)prop.write(qobject, v);
            }
            return;
         }
      }
   }

   const int offset = (opt & QScriptEngine::ExcludeSuperClassMethods) ? meta->methodOffset() : 0;

   for (index = meta->methodCount() - 1; index >= offset; --index) {
      QMetaMethod method = meta->method(index);

      if (hasMethodAccess(method, index, opt) && methodNameEquals(method, name.constData(), name.length())) {
         data->cachedMembers.insert(name, value);
         return;
      }
   }

   index = qobject->dynamicPropertyNames().indexOf(name);
   if ((index != -1) || (opt & QScriptEngine::AutoCreateDynamicProperties)) {
      QVariant v = QScriptEnginePrivate::toVariant(exec, value);
      (void)qobject->setProperty(name, v);
      return;
   }

   QScriptObjectDelegate::put(object, exec, propertyName, value, slot);
#endif //QT_NO_PROPERTIES
}

bool QObjectDelegate::deleteProperty(QScriptObject *object, JSC::ExecState *exec,
                  const JSC::Identifier &propertyName)
{
#ifndef QT_NO_PROPERTIES
   QByteArray name = convertToLatin1(propertyName.ustring());
   QObject *qobject = data->value;

   if (!qobject) {
      QString message = QString::fromLatin1("cannot access member `%0' of deleted QObject")
                        .arg(QString::fromLatin1(name));
      JSC::throwError(exec, JSC::GeneralError, message);
      return false;
   }

   const QMetaObject *meta = qobject->metaObject();
   {
      QHash<QByteArray, JSC::JSValue>::iterator it = data->cachedMembers.find(name);
      if (it != data->cachedMembers.end()) {
         if (GeneratePropertyFunctions && (meta->indexOfProperty(name.constData()) != -1)) {
            return false;
         }

         data->cachedMembers.erase(it);
         return true;
      }
   }

   const QScriptEngine::QObjectWrapOptions &opt = data->options;
   int index = meta->indexOfProperty(name.constData());

   if (index != -1) {
      QMetaProperty prop = meta->property(index);

      if (prop.isScriptable() &&
            (! (opt & QScriptEngine::ExcludeSuperClassProperties) || (index >= meta->propertyOffset()))) {
         return false;
      }
   }

   index = qobject->dynamicPropertyNames().indexOf(name);

   if (index != -1) {
      (void)qobject->setProperty(name, QVariant());
      return true;
   }

   return QScriptObjectDelegate::deleteProperty(object, exec, propertyName);

#else //QT_NO_PROPERTIES
   return false;
#endif //QT_NO_PROPERTIES
}

void QObjectDelegate::getOwnPropertyNames(QScriptObject *object, JSC::ExecState *exec,
      JSC::PropertyNameArray &propertyNames,
      JSC::EnumerationMode mode)
{
#ifndef QT_NO_PROPERTIES
   QObject *qobject = data->value;

   if (!qobject) {
      QString message = QString::fromLatin1("Can not access property names of deleted QObject");
      JSC::throwError(exec, JSC::GeneralError, message);
      return;
   }

   const QScriptEngine::QObjectWrapOptions &opt = data->options;
   const QMetaObject *meta = qobject->metaObject();

   {
      int i = (opt & QScriptEngine::ExcludeSuperClassProperties) ? meta->propertyOffset() : 0;

      for ( ; i < meta->propertyCount(); ++i) {
         QMetaProperty prop = meta->property(i);

         if (isEnumerableMetaProperty(prop, meta, i)) {
            QString name = QString::fromLatin1(prop.name());
            propertyNames.add(JSC::Identifier(exec, name));
         }
      }
   }

   {
      QList<QByteArray> dpNames = qobject->dynamicPropertyNames();
      for (int i = 0; i < dpNames.size(); ++i) {
         QString name = QString::fromLatin1(dpNames.at(i));
         propertyNames.add(JSC::Identifier(exec, name));
      }
   }

   if (!(opt & QScriptEngine::SkipMethodsInEnumeration)) {
      int i = (opt & QScriptEngine::ExcludeSuperClassMethods) ? meta->methodOffset() : 0;

      for ( ; i < meta->methodCount(); ++i) {
         QMetaMethod method = meta->method(i);

         if (hasMethodAccess(method, i, opt)) {
            QMetaMethod method = meta->method(i);

            QString sig = QString::fromLatin1(method.methodSignature().constData());
            propertyNames.add(JSC::Identifier(exec, sig));
         }
      }
   }

   QScriptObjectDelegate::getOwnPropertyNames(object, exec, propertyNames, mode);
#endif //QT_NO_PROPERTIES
}

void QObjectDelegate::markChildren(QScriptObject *object, JSC::MarkStack &markStack)
{
   QHash<QByteArray, JSC::JSValue>::const_iterator it;

   for (it = data->cachedMembers.constBegin(); it != data->cachedMembers.constEnd(); ++it) {
      JSC::JSValue val = it.value();

      if (val) {
         markStack.append(val);
      }
   }

   QScriptObjectDelegate::markChildren(object, markStack);
}

bool QObjectDelegate::compareToObject(QScriptObject *, JSC::ExecState *exec, JSC::JSObject *o2)
{
   if (!o2->inherits(&QScriptObject::info)) {
      return false;
   }

   QScriptObject *object = static_cast<QScriptObject *>(o2);
   QScriptObjectDelegate *delegate = object->delegate();

   if (!delegate || (delegate->type() != QScriptObjectDelegate::QtObject)) {
      return false;
   }

   return value() == static_cast<QObjectDelegate *>(delegate)->value();
}

static JSC::JSValue JSC_HOST_CALL qobjectProtoFuncFindChild(JSC::ExecState *exec, JSC::JSObject *,
      JSC::JSValue thisValue, const JSC::ArgList &args)
{
   QScriptEnginePrivate *engine = scriptEngineFromExec(exec);
   thisValue = engine->toUsableValue(thisValue);
   if (!thisValue.inherits(&QScriptObject::info)) {
      return throwError(exec, JSC::TypeError, "this object is not a QObject");
   }

   QScriptObject *scriptObject = static_cast<QScriptObject *>(JSC::asObject(thisValue));
   QScriptObjectDelegate *delegate = scriptObject->delegate();
   if (!delegate || (delegate->type() != QScriptObjectDelegate::QtObject)) {
      return throwError(exec, JSC::TypeError, "this object is not a QObject");
   }

   QObject *obj = static_cast<QObjectDelegate *>(delegate)->value();
   QString name;
   if (args.size() != 0) {
      name = args.at(0).toString(exec);
   }

   QObject *child = obj->findChild<QObject *>(name);
   QScriptEngine::QObjectWrapOptions opt = QScriptEngine::PreferExistingWrapperObject;
   return engine->newQObject(child, QScriptEngine::QtOwnership, opt);
}

static JSC::JSValue JSC_HOST_CALL qobjectProtoFuncFindChildren(JSC::ExecState *exec, JSC::JSObject *,
      JSC::JSValue thisValue, const JSC::ArgList &args)
{
   QScriptEnginePrivate *engine = scriptEngineFromExec(exec);
   thisValue = engine->toUsableValue(thisValue);

   // extract the QObject
   if (!thisValue.inherits(&QScriptObject::info)) {
      return throwError(exec, JSC::TypeError, "this object is not a QObject");
   }

   QScriptObject *scriptObject = static_cast<QScriptObject *>(JSC::asObject(thisValue));
   QScriptObjectDelegate *delegate = scriptObject->delegate();
   if (!delegate || (delegate->type() != QScriptObjectDelegate::QtObject)) {
      return throwError(exec, JSC::TypeError, "this object is not a QObject");
   }
   const QObject *const obj = static_cast<QObjectDelegate *>(delegate)->value();

   // find the children
   QList<QObject *> children;
   if (args.size() != 0) {
      const JSC::JSValue arg = args.at(0);

      if (arg.inherits(&JSC::RegExpObject::info)) {
         const QObjectList allChildren = obj->children();

         JSC::RegExpObject *const regexp = JSC::asRegExpObject(arg);

         const int allChildrenCount = allChildren.size();
         for (int i = 0; i < allChildrenCount; ++i) {
            QObject *const child = allChildren.at(i);
            const JSC::UString childName = child->objectName();
            JSC::RegExpConstructor *regExpConstructor = engine->originalGlobalObject()->regExpConstructor();

            int position;
            int length;

            regExpConstructor->performMatch(regexp->regExp(), childName, 0, position, length);
            if (position >= 0) {
               children.append(child);
            }
         }
      } else {
         const QString name(args.at(0).toString(exec));
         children = obj->findChildren<QObject *>(name);
      }

   } else {
      children = obj->findChildren<QObject *>(QString());
   }

   // create the result array with the children
   const int length = children.size();
   JSC::JSArray *const result = JSC::constructEmptyArray(exec, length);

   QScriptEngine::QObjectWrapOptions opt = QScriptEngine::PreferExistingWrapperObject;
   for (int i = 0; i < length; ++i) {
      QObject *const child = children.at(i);
      result->put(exec, i, engine->newQObject(child, QScriptEngine::QtOwnership, opt));
   }

   return JSC::JSValue(result);
}

static JSC::JSValue JSC_HOST_CALL qobjectProtoFuncToString(JSC::ExecState *exec, JSC::JSObject *,
      JSC::JSValue thisValue, const JSC::ArgList &)
{
   QScriptEnginePrivate *engine = scriptEngineFromExec(exec);
   thisValue = engine->toUsableValue(thisValue);

   if (!thisValue.inherits(&QScriptObject::info)) {
      return JSC::jsUndefined();
   }

   QScriptObject *scriptObject     = static_cast<QScriptObject *>(JSC::asObject(thisValue));
   QScriptObjectDelegate *delegate = scriptObject->delegate();

   if (!delegate || (delegate->type() != QScriptObjectDelegate::QtObject)) {
      return JSC::jsUndefined();
   }

   QObject *obj = static_cast<QObjectDelegate *>(delegate)->value();

   const QMetaObject *meta = obj ? obj->metaObject() : & QObject::staticMetaObject();

   QString name = obj ? obj->objectName() : QString::fromUtf8("unnamed");

   QString str = QString::fromUtf8("%0(name = \"%1\")").arg(QLatin1String(meta->className())).arg(name);

   return JSC::jsString(exec, str);
}

QObjectPrototype::QObjectPrototype(JSC::ExecState *exec, WTF::PassRefPtr<JSC::Structure> structure,
                                   JSC::Structure *prototypeFunctionStructure)
   : QScriptObject(structure)
{
   setDelegate(new QObjectDelegate(new QObjectPrototypeObject(), QScriptEngine::AutoOwnership,
                                   QScriptEngine::ExcludeSuperClassMethods
                                   | QScriptEngine::ExcludeSuperClassProperties
                                   | QScriptEngine::ExcludeChildObjects));

   putDirectFunction(exec, new (exec) JSC::NativeFunctionWrapper(exec, prototypeFunctionStructure, /*length=*/0,
                     exec->propertyNames().toString, qobjectProtoFuncToString), JSC::DontEnum);
   putDirectFunction(exec, new (exec) JSC::NativeFunctionWrapper(exec, prototypeFunctionStructure, /*length=*/1,
                     JSC::Identifier(exec, "findChild"), qobjectProtoFuncFindChild), JSC::DontEnum);
   putDirectFunction(exec, new (exec) JSC::NativeFunctionWrapper(exec, prototypeFunctionStructure, /*length=*/1,
                     JSC::Identifier(exec, "findChildren"), qobjectProtoFuncFindChildren), JSC::DontEnum);
   this->structure()->setHasGetterSetterProperties(true);
}

const JSC::ClassInfo QMetaObjectWrapperObject::info = { "QMetaObject", 0, 0, 0 };

QMetaObjectWrapperObject::QMetaObjectWrapperObject(
   JSC::ExecState *exec, const QMetaObject *metaObject, JSC::JSValue ctor,
   WTF::PassRefPtr<JSC::Structure> sid)
   : JSC::JSObject(sid),
     data(new Data(metaObject, ctor))
{
   if (!ctor) {
      data->prototype = new (exec)JSC::JSObject(exec->lexicalGlobalObject()->emptyObjectStructure());
   }
}

QMetaObjectWrapperObject::~QMetaObjectWrapperObject()
{
   delete data;
}

bool QMetaObjectWrapperObject::getOwnPropertySlot(
   JSC::ExecState *exec, const JSC::Identifier &propertyName,
   JSC::PropertySlot &slot)
{
   const QMetaObject *meta = data->value;
   if (!meta) {
      return false;
   }

   if (propertyName == exec->propertyNames().prototype) {
      if (data->ctor) {
         slot.setValue(data->ctor.get(exec, propertyName));
      } else {
         slot.setValue(data->prototype);
      }
      return true;
   }

   QByteArray name = convertToLatin1(propertyName.ustring());

   for (int i = 0; i < meta->enumeratorCount(); ++i) {
      QMetaEnum e = meta->enumerator(i);
      for (int j = 0; j < e.keyCount(); ++j) {
         const char *key = e.key(j);
         if (!qstrcmp(key, name.constData())) {
            slot.setValue(JSC::JSValue(exec, e.value(j)));
            return true;
         }
      }
   }

   return JSC::JSObject::getOwnPropertySlot(exec, propertyName, slot);
}

bool QMetaObjectWrapperObject::getOwnPropertyDescriptor(
   JSC::ExecState *exec, const JSC::Identifier &propertyName,
   JSC::PropertyDescriptor &descriptor)
{
   const QMetaObject *meta = data->value;
   if (!meta) {
      return false;
   }

   if (propertyName == exec->propertyNames().prototype) {
      descriptor.setDescriptor(data->ctor
                               ? data->ctor.get(exec, propertyName)
                               : data->prototype,
                               JSC::DontDelete | JSC::DontEnum);
      return true;
   }

   QByteArray name = QString(propertyName.ustring()).toLatin1();

   for (int i = 0; i < meta->enumeratorCount(); ++i) {
      QMetaEnum e = meta->enumerator(i);
      for (int j = 0; j < e.keyCount(); ++j) {
         const char *key = e.key(j);
         if (!qstrcmp(key, name.constData())) {
            descriptor.setDescriptor(JSC::JSValue(exec, e.value(j)),
                                     JSC::ReadOnly | JSC::DontDelete);
            return true;
         }
      }
   }

   return JSC::JSObject::getOwnPropertyDescriptor(exec, propertyName, descriptor);
}

void QMetaObjectWrapperObject::put(JSC::ExecState *exec, const JSC::Identifier &propertyName,
                                   JSC::JSValue value, JSC::PutPropertySlot &slot)
{
   if (propertyName == exec->propertyNames().prototype) {
      if (data->ctor) {
         data->ctor.put(exec, propertyName, value, slot);
      } else {
         data->prototype = value;
      }
      return;
   }
   const QMetaObject *meta = data->value;
   if (meta) {
      QByteArray name = convertToLatin1(propertyName.ustring());
      for (int i = 0; i < meta->enumeratorCount(); ++i) {
         QMetaEnum e = meta->enumerator(i);
         for (int j = 0; j < e.keyCount(); ++j) {
            if (!qstrcmp(e.key(j), name.constData())) {
               return;
            }
         }
      }
   }
   JSC::JSObject::put(exec, propertyName, value, slot);
}

bool QMetaObjectWrapperObject::deleteProperty(
   JSC::ExecState *exec, const JSC::Identifier &propertyName)
{
   if (propertyName == exec->propertyNames().prototype) {
      return false;
   }
   const QMetaObject *meta = data->value;
   if (meta) {
      QByteArray name = convertToLatin1(propertyName.ustring());
      for (int i = 0; i < meta->enumeratorCount(); ++i) {
         QMetaEnum e = meta->enumerator(i);
         for (int j = 0; j < e.keyCount(); ++j) {
            if (!qstrcmp(e.key(j), name.constData())) {
               return false;
            }
         }
      }
   }
   return JSC::JSObject::deleteProperty(exec, propertyName);
}

void QMetaObjectWrapperObject::getOwnPropertyNames(JSC::ExecState *exec,
      JSC::PropertyNameArray &propertyNames,
      JSC::EnumerationMode mode)
{
   const QMetaObject *meta = data->value;
   if (!meta) {
      return;
   }
   for (int i = 0; i < meta->enumeratorCount(); ++i) {
      QMetaEnum e = meta->enumerator(i);
      for (int j = 0; j < e.keyCount(); ++j) {
         propertyNames.add(JSC::Identifier(exec, e.key(j)));
      }
   }
   JSC::JSObject::getOwnPropertyNames(exec, propertyNames, mode);
}

void QMetaObjectWrapperObject::markChildren(JSC::MarkStack &markStack)
{
   if (data->ctor) {
      markStack.append(data->ctor);
   }
   if (data->prototype) {
      markStack.append(data->prototype);
   }
   JSC::JSObject::markChildren(markStack);
}

JSC::CallType QMetaObjectWrapperObject::getCallData(JSC::CallData &callData)
{
   callData.native.function = call;
   return JSC::CallTypeHost;
}

JSC::ConstructType QMetaObjectWrapperObject::getConstructData(JSC::ConstructData &constructData)
{
   constructData.native.function = construct;
   return JSC::ConstructTypeHost;
}

JSC::JSValue JSC_HOST_CALL QMetaObjectWrapperObject::call(
   JSC::ExecState *exec, JSC::JSObject *callee,
   JSC::JSValue thisValue, const JSC::ArgList &args)
{
   QScriptEnginePrivate *eng_p = scriptEngineFromExec(exec);
   thisValue = eng_p->toUsableValue(thisValue);
   if (!callee->inherits(&QMetaObjectWrapperObject::info)) {
      return throwError(exec, JSC::TypeError, "callee is not a QMetaObject");
   }
   QMetaObjectWrapperObject *self =  static_cast<QMetaObjectWrapperObject *>(callee);
   JSC::ExecState *previousFrame = eng_p->currentFrame;
   eng_p->pushContext(exec, thisValue, args, callee);
   JSC::JSValue result = self->execute(eng_p->currentFrame, args);
   eng_p->popContext();
   eng_p->currentFrame = previousFrame;
   return result;
}

JSC::JSObject *QMetaObjectWrapperObject::construct(JSC::ExecState *exec, JSC::JSObject *callee,
      const JSC::ArgList &args)
{
   QMetaObjectWrapperObject *self = static_cast<QMetaObjectWrapperObject *>(callee);
   QScriptEnginePrivate *eng_p = scriptEngineFromExec(exec);
   JSC::ExecState *previousFrame = eng_p->currentFrame;
   eng_p->pushContext(exec, JSC::JSValue(), args, callee, true);
   JSC::JSValue result = self->execute(eng_p->currentFrame, args);
   eng_p->popContext();
   eng_p->currentFrame = previousFrame;
   if (!result || !result.isObject()) {
      return 0;
   }
   return JSC::asObject(result);
}

JSC::JSValue QMetaObjectWrapperObject::execute(JSC::ExecState *exec,
      const JSC::ArgList &args)
{
   if (data->ctor) {
      QScriptEnginePrivate *eng_p = QScript::scriptEngineFromExec(exec);
      QScriptContext *ctx = eng_p->contextForFrame(exec);
      JSC::CallData callData;
      JSC::CallType callType = data->ctor.getCallData(callData);
      Q_UNUSED(callType);
      Q_ASSERT_X(callType == JSC::CallTypeHost, Q_FUNC_INFO, "script constructors not supported");
      if (data->ctor.inherits(&FunctionWithArgWrapper::info)) {
         FunctionWithArgWrapper *wrapper = static_cast<FunctionWithArgWrapper *>(JSC::asObject(data->ctor));
         QScriptValue result = wrapper->function()(ctx, QScriptEnginePrivate::get(eng_p), wrapper->arg());
         return eng_p->scriptValueToJSCValue(result);
      } else {
         Q_ASSERT(data->ctor.inherits(&FunctionWrapper::info));
         FunctionWrapper *wrapper = static_cast<FunctionWrapper *>(JSC::asObject(data->ctor));
         QScriptValue result = wrapper->function()(ctx, QScriptEnginePrivate::get(eng_p));
         return eng_p->scriptValueToJSCValue(result);
      }

   } else {
      const QMetaObject *meta = data->value;
      if (meta->constructorCount() > 0) {
         JSC::JSValue result = callQtMethod(exec, QMetaMethod::Constructor, /*thisQObject=*/0,
                                            args, meta, meta->constructorCount() - 1, /*maybeOverloaded=*/true);

         if (!exec->hadException()) {
            Q_ASSERT(result && result.inherits(&QScriptObject::info));
            QScriptObject *object = static_cast<QScriptObject *>(JSC::asObject(result));
            QScript::QObjectDelegate *delegate = static_cast<QScript::QObjectDelegate *>(object->delegate());
            delegate->setOwnership(QScriptEngine::AutoOwnership);

            if (data->prototype) {
               object->setPrototype(data->prototype);
            }
         }
         return result;

      } else {
         QString message = QString::fromLatin1("no constructor for %0").arg(QLatin1String(meta->className()));
         return JSC::throwError(exec, JSC::TypeError, message);
      }
   }
}

static JSC::JSValue JSC_HOST_CALL qmetaobjectProtoFuncClassName(
   JSC::ExecState *exec, JSC::JSObject *, JSC::JSValue thisValue, const JSC::ArgList &)
{
   QScriptEnginePrivate *engine = scriptEngineFromExec(exec);
   thisValue = engine->toUsableValue(thisValue);

   if (!thisValue.inherits(&QMetaObjectWrapperObject::info)) {
      return throwError(exec, JSC::TypeError, "this object is not a QMetaObject");
   }

   const QMetaObject *meta = static_cast<QMetaObjectWrapperObject *>(JSC::asObject(thisValue))->value();
   return JSC::jsString(exec, meta->className());
}

QMetaObjectPrototype::QMetaObjectPrototype(JSC::ExecState *exec, WTF::PassRefPtr<JSC::Structure> structure,
      JSC::Structure *prototypeFunctionStructure)
   : QMetaObjectWrapperObject(exec, &Qt::staticMetaObject(), /*ctor=*/JSC::JSValue(), structure)
{
   putDirectFunction(exec, new (exec) JSC::NativeFunctionWrapper(exec,
                     prototypeFunctionStructure, /*length=*/0, JSC::Identifier(exec, "className"), qmetaobjectProtoFuncClassName),
                     JSC::DontEnum);
}

static const uint qt_meta_data_QObjectConnectionManager[] = {

   // content:
   1,       // revision
   0,       // classname
   0,    0, // classinfo
   1,   10, // methods
   0,    0, // properties
   0,    0, // enums/sets

   // slots: signature, parameters, type, tag, flags
   35,   34,   34,   34, 0x0a,

   0        // eod
};

static const char qt_meta_stringdata_QObjectConnectionManager[] = {
   "QScript::QObjectConnectionManager\0\0execute()\0"
};

const QMetaObject &QObjectConnectionManager::staticMetaObject()
{
   // BROOM (script)
   return QObject::staticMetaObject();

   //  { &QObject::staticMetaObject, qt_meta_stringdata_QObjectConnectionManager, qt_meta_data_QObjectConnectionManager, 0 }
}

const QMetaObject *QObjectConnectionManager::metaObject() const
{
   return &staticMetaObject();
}

void QObjectConnectionManager::execute(int slotIndex, void **argv)
{
   JSC::JSValue receiver;
   JSC::JSValue slot;
   JSC::JSValue senderWrapper;
   int signalIndex = -1;
   QScript::APIShim shim(engine);

   for (int i = 0; i < connections.size(); ++i) {
      const QVector<QObjectConnection> &cs = connections.at(i);
      for (int j = 0; j < cs.size(); ++j) {
         const QObjectConnection &c = cs.at(j);
         if (c.slotIndex == slotIndex) {
            receiver = c.receiver;
            slot = c.slot;
            senderWrapper = c.senderWrapper;
            signalIndex = i;
            break;
         }
      }
   }

   if (! slot) {
      // This connection no longer exists (can happen if the signal is
      // emitted from another thread and the call gets queued, but the
      // connection is removed before the QMetaCallEvent gets processed).
      return;
   }
   Q_ASSERT(slot.isObject());

   if (engine->isCollecting()) {
      qWarning("QtScript: can't execute signal handler during GC");
      // we can't do a script function call during GC,
      // so we're forced to ignore this signal
      return;
   }

#if 0
   QScriptFunction *fun = engine->convertToNativeFunction(slot);
   if (fun == 0) {
      // the signal handler has been GC'ed. This can only happen when
      // a QObject is owned by the engine, the engine is destroyed, and
      // there is a script function connected to the destroyed() signal
      Q_ASSERT(signalIndex <= 1); // destroyed(QObject*)
      return;
   }
#endif

   const QMetaObject *meta  = sender()->metaObject();
   const QMetaMethod method = meta->method(signalIndex);

   QList<QByteArray> parameterTypes = method.parameterTypes();
   int argc = parameterTypes.count();

   JSC::ExecState *exec = engine->currentFrame;
   QVarLengthArray<JSC::JSValue, 8> argsVector(argc);

   for (int i = 0; i < argc; ++i) {
      JSC::JSValue actual;
      void *arg = argv[i + 1];

      QByteArray typeName = parameterTypes.at(i);
      int argType = QMetaType::type(parameterTypes.at(i).constData());

      if (! argType) {
         qWarning("QScriptEngine: Unable to handle unregistered datatype '%s' "
                  "when invoking handler of signal %s::%s",
                  typeName.constData(), meta->className(), method.methodSignature().constData());

         actual = JSC::jsUndefined();

      } else if (argType == QMetaType::QVariant) {
         actual = QScriptEnginePrivate::jscValueFromVariant(exec, *reinterpret_cast<QVariant *>(arg));

      } else {
         actual = QScriptEnginePrivate::create(exec, argType, arg);
      }

      argsVector[i] = actual;
   }
   JSC::ArgList jscArgs(argsVector.data(), argsVector.size());

   JSC::JSValue senderObject;
   if (senderWrapper && senderWrapper.inherits(&QScriptObject::info)) { // ### check if it's actually a QObject wrapper
      senderObject = senderWrapper;
   } else {
      QScriptEngine::QObjectWrapOptions opt = QScriptEngine::PreferExistingWrapperObject;
      senderObject = engine->newQObject(sender(), QScriptEngine::QtOwnership, opt);
   }

   JSC::JSValue thisObject;
   if (receiver && receiver.isObject()) {
      thisObject = receiver;
   } else {
      thisObject = engine->globalObject();
   }

   JSC::CallData callData;
   JSC::CallType callType = slot.getCallData(callData);
   if (exec->hadException()) {
      exec->clearException();   // ### otherwise JSC asserts
   }
   JSC::call(exec, slot, callType, callData, thisObject, jscArgs);

   if (exec->hadException()) {
      if (slot.inherits(&QtFunction::info) && !static_cast<QtFunction *>(JSC::asObject(slot))->qobject()) {
         // The function threw an error because the target QObject has been deleted.
         // The connections list is stale; remove the signal handler and ignore the exception.
         removeSignalHandler(sender(), signalIndex, receiver, slot);
         exec->clearException();
      } else {
         engine->emitSignalHandlerException();
      }
   }
}

QObjectConnectionManager::QObjectConnectionManager(QScriptEnginePrivate *eng)
   : engine(eng), slotCounter(0)
{
}

QObjectConnectionManager::~QObjectConnectionManager()
{
}

void QObjectConnectionManager::mark(JSC::MarkStack &markStack)
{
   for (int i = 0; i < connections.size(); ++i) {
      QVector<QObjectConnection> &cs = connections[i];
      for (int j = 0; j < cs.size(); ++j) {
         cs[j].mark(markStack);
      }
   }
}

bool QObjectConnectionManager::addSignalHandler(QObject *sender, int signalIndex, JSC::JSValue receiver,
      JSC::JSValue function, JSC::JSValue senderWrapper, Qt::ConnectionType type)
{
   if (connections.size() <= signalIndex) {
      connections.resize(signalIndex + 1);
   }

   QVector<QObjectConnection> &cs = connections[signalIndex];
   int absSlotIndex = slotCounter + metaObject()->methodOffset();


   /*  BROOM (script)
       bool ok = QMetaObject::connect(sender, signalIndex, this, absSlotIndex, type);
   */

   // temp value, remove when about code comes back in
   bool ok = false;

   if (ok) {
      cs.append(QObjectConnection(slotCounter++, receiver, function, senderWrapper));
      QMetaMethod signal = sender->metaObject()->method(signalIndex);

      // BROOM (script)

      //    QByteArray signalString;
      //    signalString.append('2'); // signal code
      //    signalString.append(signal.signature());

      //    static_cast<QObjectNotifyCaller *>(sender)->callConnectNotify(signalString);
   }

   return ok;
}

bool QObjectConnectionManager::removeSignalHandler(QObject *sender, int signalIndex, JSC::JSValue receiver,
      JSC::JSValue slot)
{
   if (connections.size() <= signalIndex) {
      return false;
   }

   QVector<QObjectConnection> &cs = connections[signalIndex];

   for (int i = 0; i < cs.size(); ++i) {
      const QObjectConnection &c = cs.at(i);

      if (c.hasTarget(receiver, slot)) {

         /* BROOM (script)
                     int absSlotIndex = c.slotIndex + metaObject()->methodOffset();
                     bool ok = QMetaObject::disconnect(sender, signalIndex, this, absSlotIndex);
         */
         // temp value, remove when about code comes back in
         bool ok = false;


         if (ok) {
            cs.remove(i);
            QMetaMethod signal = sender->metaObject()->method(signalIndex);

            // BROOM (script)

            //          QByteArray signalString;
            //          signalString.append('2'); // signal code
            //          signalString.append(signal.signature());
            //          static_cast<QScript::QObjectNotifyCaller *>(sender)->callDisconnectNotify(signalString);
         }

         return ok;
      }
   }
   return false;
}

QObjectData::QObjectData(QScriptEnginePrivate *eng)
   : engine(eng), connectionManager(0)
{
}

QObjectData::~QObjectData()
{
   if (connectionManager) {
      delete connectionManager;
      connectionManager = 0;
   }
}

void QObjectData::mark(JSC::MarkStack &markStack)
{
   if (connectionManager) {
      connectionManager->mark(markStack);
   }

   {
      QList<QScript::QObjectWrapperInfo>::iterator it;

      for (it = wrappers.begin(); it != wrappers.end(); ) {
         const QScript::QObjectWrapperInfo &info = *it;
         // ### don't mark if there are no other references.
         // we need something like isMarked()
         markStack.append(info.object);
         ++it;
      }
   }
}

bool QObjectData::addSignalHandler(QObject *sender, int signalIndex, JSC::JSValue receiver,
                                   JSC::JSValue slot, JSC::JSValue senderWrapper, Qt::ConnectionType type)
{
   if (!connectionManager) {
      connectionManager = new QObjectConnectionManager(engine);
   }

   return connectionManager->addSignalHandler(sender, signalIndex, receiver, slot, senderWrapper, type);
}

bool QObjectData::removeSignalHandler(QObject *sender, int signalIndex,
                                      JSC::JSValue receiver, JSC::JSValue slot)
{
   if (!connectionManager) {
      return false;
   }
   return connectionManager->removeSignalHandler(sender, signalIndex, receiver, slot);
}

QScriptObject *QObjectData::findWrapper(QScriptEngine::ValueOwnership ownership,
                                        const QScriptEngine::QObjectWrapOptions &options) const
{
   for (int i = 0; i < wrappers.size(); ++i) {
      const QObjectWrapperInfo &info = wrappers.at(i);

      if ((info.ownership == ownership) && (info.options == options)) {
         return info.object;
      }
   }
   return 0;
}

void QObjectData::registerWrapper(QScriptObject *wrapper, QScriptEngine::ValueOwnership ownership,
                                  const QScriptEngine::QObjectWrapOptions &options)
{
   wrappers.append(QObjectWrapperInfo(wrapper, ownership, options));
}

} // namespace QScript

QT_END_NAMESPACE

namespace JSC {
ASSERT_CLASS_FITS_IN_CELL(QScript::QtFunction);
}
