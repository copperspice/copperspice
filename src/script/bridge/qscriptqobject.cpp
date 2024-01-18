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
#include <qscriptqobject_p.h>

#include <qmetaobject.h>
#include <qvarlengtharray.h>
#include <qdebug.h>
#include <qscriptable.h>

#include <qscriptengine_p.h>
#include <qscriptable_p.h>
#include <qscriptcontext_p.h>
#include <qscriptfunction_p.h>
#include <qscriptactivationobject_p.h>

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

ASSERT_CLASS_FITS_IN_CELL(QScript::QObjectPrototype);
ASSERT_CLASS_FITS_IN_CELL(QScript::QMetaObjectWrapperObject);
ASSERT_CLASS_FITS_IN_CELL(QScript::QMetaObjectPrototype);
ASSERT_CLASS_FITS_IN_CELL(QScript::QtFunction);
ASSERT_CLASS_FITS_IN_CELL(QScript::QtPropertyFunction);
}

namespace QScript {

struct QObjectConnection {
   uint marked: 1;
   uint slotIndex: 31;
   JSC::JSValue receiver;
   JSC::JSValue slot;
   JSC::JSValue senderWrapper;

   QObjectConnection(int i, JSC::JSValue r, JSC::JSValue s, JSC::JSValue sw)
      : marked(false), slotIndex(i), receiver(r), slot(s), senderWrapper(sw)
   {}

   QObjectConnection() : marked(false), slotIndex(0) {}

   bool hasTarget(JSC::JSValue r, JSC::JSValue s) const {
      if ((r && r.isObject()) != (receiver && receiver.isObject())) {
         return false;
      }

      if (((r && r.isObject()) && (receiver && receiver.isObject())) && (r != receiver)) {
         return false;
      }
      return (s == slot);
   }

   bool hasWeaklyReferencedSender() const {
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
                  && !inst->hasParent())) {
               return true;

            }
         }
      }

      return false;
   }

   void mark(JSC::MarkStack &markStack) {
      Q_ASSERT(!marked);
      if (senderWrapper) {
         markStack.append(senderWrapper);
      }

      if (receiver) {
         markStack.append(receiver);
      }

      if (slot) {
         markStack.append(slot);
      }

      marked = true;
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

   void execute(int slotIndex, void **argv);
   void clearMarkBits();
   int mark(JSC::MarkStack &);

 private:
   QScriptEnginePrivate *engine;
   int slotCounter;
   QVector<QVector<QObjectConnection>> connections;
};

static bool hasMethodAccess(const QMetaMethod &method, int index, const QScriptEngine::QObjectWrapOptions &opt)
{
   static const int deleteLaterIndex = QObject::staticMetaObject().indexOfMethod("deleteLater()");

   return (method.access() != QMetaMethod::Private)
      && ((index != deleteLaterIndex) || !(opt & QScriptEngine::ExcludeDeleteLater))
      && (!(opt & QScriptEngine::ExcludeSlots) || (method.methodType() != QMetaMethod::Slot));
}

static bool isEnumerableMetaProperty(const QMetaProperty &prop, const QMetaObject *mo, int index)
{
   return prop.isScriptable() && prop.isValid()
      // the following lookup is to ensure that we have the
      // "most derived" occurrence of the property with this name
      && (mo->indexOfProperty(prop.name()) == index);
}

// do not delete next two functions (copperspice)
static inline int methodNameLength(const QMetaMethod &method)
{
   const QString &tmpSignature = method.methodSignature();

   return tmpSignature.indexOf("(");
}

static inline bool methodNameEquals(const QMetaMethod &method, const QString &signature, int nameLength)
{
   const QString &tmpSignature = method.methodSignature();

   return (tmpSignature.leftView(nameLength) == signature.leftView(nameLength) && tmpSignature[nameLength] == '(');
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

static int indexOfMetaEnum(const QMetaObject *meta, const QString &str)
{
   QString scope;
   QString name;
   int scopeIdx = str.lastIndexOf("::");

   if (scopeIdx != -1) {
      scope = str.left(scopeIdx);
      name  = str.mid(scopeIdx + 2);

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

static inline QScriptable *scriptableFromQObject(QObject *qobject)
{
   return dynamic_cast<QScriptable *>(qobject);
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
   if (! qobj) {
      return nullptr;
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
   if (! meta) {
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
      if (methodNameEquals(meta->method(index), method.methodSignature(), nameLength)) {
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

   uint typeId() const;

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

   QString name() const;

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

   static inline QScriptMetaType metaType(uint typeId, const QString &name) {
      return QScriptMetaType(MetaType, typeId, name);
   }

   static inline QScriptMetaType metaEnum(int enumIndex, const QString &name) {
      return QScriptMetaType(MetaEnum, enumIndex, name);
   }

   static inline QScriptMetaType unresolved(const QString &name) {
      return QScriptMetaType(Unresolved, /*typeId=*/0, name);
   }

 private:
   inline QScriptMetaType(Kind kind, uint typeId = QVariant::Invalid, const QString &name = QString())
      : m_kind(kind), m_typeId(typeId), m_name(name) { }

   Kind m_kind;
   uint m_typeId;
   QString m_name;
};

uint QScriptMetaType::typeId() const
{
   uint retval;

   if (isVariant()) {
      retval = QVariant::Variant;

   } else if (isMetaEnum()) {
      retval = QVariant::Int;

   } else {
      retval = m_typeId;
   }

   return retval;
}

QString QScriptMetaType::name() const
{
   if (! m_name.isEmpty()) {
      return m_name;

   } else if (m_kind == QScriptMetaType::Kind::Variant) {
      return QString("QVariant");

   }

   return QVariant::typeToName(typeId());
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
      return ! m_types.isEmpty();
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
      : matchDistance(dist), index(idx), method(mtd), args(as)
   { }

   inline QScriptMetaArguments()
      : index(-1)
   { }

   inline bool isValid() const {
      return (index != -1);
   }
};

static QMetaMethod metaMethod(const QMetaObject *meta, QMetaMethod::MethodType type, int index)
{
   if (type != QMetaMethod::Constructor) {
      return meta->method(index);

   } else {
      return meta->constructor(index);
   }
}

template <class Delegate>
static JSC::JSValue delegateQtMethod(JSC::ExecState *exec, QMetaMethod::MethodType callType,
                  const JSC::ArgList &scriptArgs, const QMetaObject *meta,
                  int initialIndex, bool maybeOverloaded, Delegate &delegate)
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

   QString tempSignature;
   QString initialMethodSignature;

   for (index = initialIndex; index >= 0; --index) {
      QMetaMethod method = metaMethod(meta, callType, index);

      if (index == initialIndex) {
         initialMethodSignature = method.methodSignature();
         nameLength = methodNameLength(method);

      } else if (! methodNameEquals(method, initialMethodSignature, nameLength)) {
         continue;
      }

      QList<QString> parameterTypeNames = method.parameterTypes();

      QVector<QScriptMetaType> types;
      types.resize(1 + parameterTypeNames.size());

      QScriptMetaType *typesData = types.data();

      // resolve return type
      QString returnTypeName = method.typeName();
      uint rtype = QVariant::nameToType(returnTypeName);

      if ((rtype == QVariant::Invalid) && ! returnTypeName.isEmpty()) {
         int enumIndex = indexOfMetaEnum(meta, returnTypeName);

         if (enumIndex != -1) {
            typesData[0] = QScriptMetaType::metaEnum(enumIndex, returnTypeName);
         } else {
            typesData[0] = QScriptMetaType::unresolved(returnTypeName);
         }

      } else {
         if (callType == QMetaMethod::Constructor) {
            typesData[0] = QScriptMetaType::metaType(QVariant::ObjectStar, "QObject*");

         } else if (rtype == QVariant::Variant) {
            typesData[0] = QScriptMetaType::variant();

         } else {
            typesData[0] = QScriptMetaType::metaType(rtype, returnTypeName);
         }
      }

      // resolve argument types
      for (int i = 0; i < parameterTypeNames.count(); ++i) {
         QString argTypeName = parameterTypeNames.at(i);
         uint atype = QVariant::nameToType(argTypeName);

         if (atype == QVariant::Invalid) {
            int enumIndex = indexOfMetaEnum(meta, argTypeName);

            if (enumIndex != -1) {
               typesData[1 + i] = QScriptMetaType::metaEnum(enumIndex, argTypeName);
            } else {
               typesData[1 + i] = QScriptMetaType::unresolved(argTypeName);
            }

         } else if (atype == QVariant::Variant) {
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

      if (! mtd.fullyResolved()) {
         // remember it so we can give an error message later if necessary
         unresolved.append(QScriptMetaArguments(INT_MAX, index, mtd, QVarLengthArray<QVariant, 9>()));

         if (mtd.hasUnresolvedReturnType()) {
            continue;
         }
      }

      if (args.count() != mtd.count()) {
         args.resize(mtd.count());
      }

      QScriptMetaType retType = mtd.returnType();
      args[0] = QVariant(retType.typeId(), (void *)nullptr);    // the result

      // try to convert arguments
      bool converted    = true;
      int matchDistance = 0;

      for (int i = 0; converted && i < mtd.argumentCount(); ++i) {
         JSC::JSValue actual;

         if (i < (int)scriptArgs.size()) {
            actual = scriptArgs.at(i);
         } else {
            actual = JSC::jsUndefined();
         }

         QScriptMetaType argType = mtd.argumentType(i);

         uint tid = QVariant::Invalid;
         QVariant v;

         if (argType.isUnresolved()) {
            v = QVariant(QVariant::ObjectStar, (void *)nullptr);

            /* emerald (script, hold)
               converted = QScriptEnginePrivate::convertToNativeQObject(
                     exec, actual, argType.name(), reinterpret_cast<void **>(v.data()));
            */
            converted = false;

         } else if (argType.isVariant()) {
            if (QScriptEnginePrivate::isVariant(actual)) {
               v = QScriptEnginePrivate::variantValue(actual);

            } else {
               v = QScriptEnginePrivate::toVariant(exec, actual);
               converted = v.isValid() || actual.isUndefined() || actual.isNull();
            }

         } else {
            tid = argType.typeId();

            v   = QScriptEnginePrivate::convertValue(exec, actual, tid);
            converted = v.isValid();

            if (exec->hadException()) {
               return exec->exception();
            }
         }

         if (! converted) {
            if (QScriptEnginePrivate::isVariant(actual)) {
               if (tid == QVariant::Invalid) {
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
                  QString newTypeName = vv.typeName();

                  if (newTypeName.endsWith('*')) {
                     newTypeName.chop(1);
                     uint newId = QVariant::nameToType(newTypeName);

                     if (newId == tid) {
                        // variant was Widget *, user wanted Widget

                        converted = false;
                        matchDistance += 10;

                        v = QVariant();
                     }
                  }
               }

            } else if (actual.isNumber() || actual.isString()) {
               // is it an enum value
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

                     if (m.valueToKey(ival) != QString()) {
                        v.setValue(ival);
                        converted = true;
                        matchDistance += 10;
                     }

                  } else {
                     JSC::UString sval = QScriptEnginePrivate::toString(exec, actual);
                     int ival = m.keyToValue(convertToString(sval));

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
                  case QVariant::Double:
                     // perfect
                     break;

                  case QVariant::Float:
                     matchDistance += 1;
                     break;

                  case QVariant::LongLong:
                  case QVariant::ULongLong:
                     matchDistance += 2;
                     break;

                  case QVariant::Long:
                  case QVariant::ULong:
                     matchDistance += 3;
                     break;

                  case QVariant::Int:
                  case QVariant::UInt:
                     matchDistance += 4;
                     break;

                  case QVariant::Short:
                  case QVariant::UShort:
                     matchDistance += 5;
                     break;

                  case QVariant::Char:
                  case QVariant::UChar:
                     matchDistance += 6;
                     break;

                  default:
                     matchDistance += 10;
                     break;
               }

            } else if (actual.isString()) {
               switch (tid) {
                  case QVariant::String:
                     // perfect
                     break;

                  default:
                     matchDistance += 10;
                     break;
               }

            } else if (actual.isBoolean()) {
               switch (tid) {
                  case QVariant::Bool:
                     // perfect
                     break;

                  default:
                     matchDistance += 10;
                     break;
               }

            } else if (QScriptEnginePrivate::isDate(actual)) {
               switch (tid) {
                  case QVariant::DateTime:
                     // perfect
                     break;

                  case QVariant::Date:
                     matchDistance += 1;
                     break;

                  case QVariant::Time:
                     matchDistance += 2;
                     break;

                  default:
                     matchDistance += 10;
                     break;
               }

            } else if (QScriptEnginePrivate::isRegExp(actual)) {
               switch (tid) {
                  case QVariant::RegularExpression:
                     // perfect
                     break;

                  default:
                     matchDistance += 10;
                     break;
               }

            } else if (QScriptEnginePrivate::isVariant(actual)) {
               if (argType.isVariant() || (QScriptEnginePrivate::toVariant(exec, actual).userType() == tid)) {
                  // perfect
               } else {
                  matchDistance += 10;
               }

            } else if (QScriptEnginePrivate::isArray(actual)) {
               switch (tid) {
                  case QVariant::StringList:
                  case QVariant::List:
                     matchDistance += 5;
                     break;

                  default:
                     matchDistance += 10;
                     break;
               }

            } else if (QScriptEnginePrivate::isQObject(actual)) {
               switch (tid) {
                  case QVariant::ObjectStar:
                  case QVariant::WidgetStar:
                     // perfect
                     break;

                  default:
                     if (isPtr2QObject(tid)) {
                        matchDistance += 10;
                     }

                     break;
               }

            } else if (actual.isNull()) {
               switch (tid) {
                  case QVariant::VoidStar:
                  case QVariant::ObjectStar:
                  case QVariant::WidgetStar:
                     // perfect
                     break;

                  default:
                     if (! argType.name().endsWith('*')) {
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
         if ((scriptArgs.size() == (size_t)mtd.argumentCount()) && (matchDistance == 0)) {
            // perfect match, use this one
            chosenMethod = mtd;
            chosenIndex  = index;
            break;

         } else {
            bool redundant = false;
            if ((callType != QMetaMethod::Constructor) && (index < meta->methodOffset())) {
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
                  if ((args.count() > otherArgs.args.count()) || ((args.count() == otherArgs.args.count())
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

      if (! maybeOverloaded) {
         break;
      }
   }

   JSC::JSValue result;

   if ((chosenIndex == -1) && candidates.isEmpty()) {

      QString funName = initialMethodSignature.left(nameLength);

      if (! conversionFailed.isEmpty()) {
         QString message = QString("Incompatible type of argument(s) in call to %0(), candidates were\n").formatArg(funName);

         for (int i = 0; i < conversionFailed.size(); ++i) {
            if (i > 0) {
               message += "\n";
            }

            QMetaMethod mtd = metaMethod(meta, callType, conversionFailed.at(i));
            message += QString("    %0").formatArg(mtd.methodSignature());
         }

         result = JSC::throwError(exec, JSC::TypeError, message);

      } else if (!unresolved.isEmpty()) {
         QScriptMetaArguments argsInstance = unresolved.first();
         int unresolvedIndex = argsInstance.method.firstUnresolvedIndex();
         Q_ASSERT(unresolvedIndex != -1);

         QScriptMetaType unresolvedType = argsInstance.method.type(unresolvedIndex);
         QString unresolvedTypeName = unresolvedType.name();

         QString message = QString("can not call %0(): ").formatArg(funName);

         if (unresolvedIndex > 0) {
            message.append(QString("argument %0 has unknown type `%1'").formatArg(unresolvedIndex).formatArg(unresolvedTypeName));

         } else {
            message.append(QString("unknown return type `%0'").formatArg(unresolvedTypeName));

         }
         message.append(QString(" (register the type with qScriptRegisterMetaType())"));
         result = JSC::throwError(exec, JSC::TypeError, message);

      } else {
         QString message = QString("too few arguments in call to %0(); candidates are\n").formatArg(funName);

         for (int i = 0; i < tooFewArgs.size(); ++i) {
            if (i > 0) {
               message += "\n";
            }

            QMetaMethod mtd = metaMethod(meta, callType, tooFewArgs.at(i));
            message += QString("    %0").formatArg(mtd.methodSignature());
         }

         result = JSC::throwError(exec, JSC::SyntaxError, message);
      }

   } else {
      if (chosenIndex == -1) {
         QScriptMetaArguments metaArgs = candidates.at(0);

         if ((candidates.size() > 1) && (metaArgs.args.count() == candidates.at(1).args.count())
            && (metaArgs.matchDistance == candidates.at(1).matchDistance)) {

            // found ambiguous call
            QString funName = initialMethodSignature.left(nameLength);
            QString message = QString("Ambiguous call of overloaded function %0(); candidates were\n").formatArg(funName);

            for (int i = 0; i < candidates.size(); ++i) {
               if (i > 0) {
                  message += QLatin1String("\n");
               }

               QMetaMethod mtd = metaMethod(meta, callType, candidates.at(i).index);
               message += QString("    %0").formatArg(mtd.methodSignature());
            }

            result = JSC::throwError(exec, JSC::TypeError, message);

         } else {
            chosenMethod = metaArgs.method;
            chosenIndex = metaArgs.index;
            args = metaArgs.args;
         }
      }

      if (chosenIndex != -1) {
         result = delegate(exec, callType, meta, chosenMethod, chosenIndex, args);
      }
   }

   return result;
}

struct QtMethodCaller {
   QtMethodCaller(QObject *o)
      : thisQObject(o)
   {}

   JSC::JSValue operator()(JSC::ExecState *exec, QMetaMethod::MethodType callType,
      const QMetaObject *meta, const QScriptMetaMethod &chosenMethod,
      int chosenIndex, const QVarLengthArray<QVariant, 9> &args) {
      JSC::JSValue result;

      (void) chosenIndex;

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

               /* emerald (script, hold)
                  params[i] = const_cast<void *>(v.constData());
               */
               params[i] = nullptr;

               break;

            default:
               Q_ASSERT(0);
         }
      }

      QScriptable *scriptable = nullptr;
      if (thisQObject) {
         scriptable = scriptableFromQObject(thisQObject);
      }

      QScriptEngine *oldEngine = nullptr;
      QScriptEnginePrivate *engine = QScript::scriptEngineFromExec(exec);

      if (scriptable) {
         oldEngine = QScriptablePrivate::get(scriptable)->engine;
         QScriptablePrivate::get(scriptable)->engine = QScriptEnginePrivate::get(engine);
      }

      if (callType == QMetaMethod::Constructor) {
         Q_ASSERT(meta != nullptr);

         /* emerald (script, hold)
              meta->static_metacall(QMetaObject::CreateInstance, chosenIndex, params);
         */

      } else {

         /* emerald (script, hold)
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

         } else if (retType.typeId() != QVariant::Void) {

            QVariant tmp = QVariant(retType.typeId(), params[0]);
            result = QScriptEnginePrivate::create(exec, tmp);

            if (! result) {
               result = engine->newVariant(tmp);
            }

         } else {
            result = JSC::jsUndefined();
         }
      }

      return result;
   }

 private:
   QObject *thisQObject;
};

static JSC::JSValue callQtMethod(JSC::ExecState *exec, QMetaMethod::MethodType callType,
   QObject *thisQObject, const JSC::ArgList &scriptArgs,
   const QMetaObject *meta, int initialIndex, bool maybeOverloaded)
{
   QtMethodCaller caller(thisQObject);

   return delegateQtMethod<QtMethodCaller>(exec, callType, scriptArgs, meta,
         initialIndex, maybeOverloaded, caller);
}

JSC::JSValue QtFunction::execute(JSC::ExecState *exec, JSC::JSValue thisValue,
   const JSC::ArgList &scriptArgs)
{
   Q_ASSERT(data->object.inherits(&QScriptObject::info));

   QScriptObject *scriptObject = static_cast<QScriptObject *>(JSC::asObject(data->object));
   QScriptObjectDelegate *delegate = scriptObject->delegate();

   Q_ASSERT(delegate && (delegate->type() == QScriptObjectDelegate::QtObject));

   QObject *qobj = static_cast<QScript::QObjectDelegate *>(delegate)->value();

   if (! qobj) {
      return JSC::throwError(exec, JSC::GeneralError, QString("Can not call function of deleted QObject"));
   }

   QScriptEnginePrivate *engine = scriptEngineFromExec(exec);

   const QMetaObject *meta = qobj->metaObject();
   QObject *thisQObject = nullptr;

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

const JSC::ClassInfo QtFunction::info = { "QtFunction", &InternalFunction::info, nullptr, nullptr };

JSC::JSValue JSC_HOST_CALL QtFunction::call(JSC::ExecState *exec, JSC::JSObject *callee,
   JSC::JSValue thisValue, const JSC::ArgList &args)
{
   if (! callee->inherits(&QtFunction::info)) {
      return throwError(exec, JSC::TypeError, "Invoked object must inherit from QtFunction");
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

struct QtMethodIndexReturner {
   JSC::JSValue operator()(JSC::ExecState *exec, QMetaMethod::MethodType,
      const QMetaObject *, const QScriptMetaMethod &,
      int chosenIndex, const QVarLengthArray<QVariant, 9> &) {
      return JSC::jsNumber(exec, chosenIndex);
   }
};
int QtFunction::specificIndex(const QScriptContext *context) const
{
   if (!maybeOverloaded()) {
      return initialIndex();
   }
   JSC::ExecState *exec = const_cast<JSC::ExecState *>(QScriptEnginePrivate::frameForContext(context));
   int argCount = exec->argumentCount();

   // Create arguments list wrapper; this logic must match
   // JITStubs.cpp op_call_NotJSFunction, and Interpreter.cpp op_call
   JSC::Register *argv = exec->registers() - JSC::RegisterFile::CallFrameHeaderSize - argCount;
   JSC::ArgList args(argv + 1, argCount - 1);

   QtMethodIndexReturner returner;
   JSC::JSValue result =  delegateQtMethod<QtMethodIndexReturner>(
         exec, QMetaMethod::Method, args, metaObject(),
         initialIndex(), maybeOverloaded(), returner);
   if (exec->hadException() || !result || !result.isInt32()) {
      return initialIndex();
   }
   return result.asInt32();
}

const JSC::ClassInfo QtPropertyFunction::info = { "QtPropertyFunction", &InternalFunction::info, nullptr, nullptr };

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
      return throwError(exec, JSC::TypeError, "Invoked object must inherit from QtPropertyFunction");
   }

   QtPropertyFunction *qfun =  static_cast<QtPropertyFunction *>(callee);
   return qfun->execute(exec, thisValue, args);
}

JSC::JSValue QtPropertyFunction::execute(JSC::ExecState *exec, JSC::JSValue thisValue, const JSC::ArgList &args)
{
   JSC::JSValue result = JSC::jsUndefined();

   QScriptEnginePrivate *engine  = scriptEngineFromExec(exec);
   JSC::ExecState *previousFrame = engine->currentFrame;
   engine->currentFrame = exec;

   JSC::JSValue qobjectValue = engine->toUsableValue(thisValue);
   QObject *qobject = QScriptEnginePrivate::toQObject(exec, qobjectValue);

   while ((! qobject || (qobject->metaObject() != data->meta))
      && JSC::asObject(qobjectValue)->prototype().isObject()) {
      qobjectValue = JSC::asObject(qobjectValue)->prototype();
      qobject = QScriptEnginePrivate::toQObject(exec, qobjectValue);
   }
   Q_ASSERT_X(qobject, Q_FUNC_INFO, "This object must be a QObject");

   QMetaProperty prop = data->meta->property(data->index);
   Q_ASSERT(prop.isScriptable());

   if (args.size() == 0) {
      // get
      if (prop.isValid()) {
         QScriptable *scriptable  = scriptableFromQObject(qobject);
         QScriptEngine *oldEngine = nullptr;

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

      if (prop.isEnumType() && arg.isString() && ! engine->hasDemarshalFunction(prop.userType())) {
         // give QMetaProperty::write() a chance to convert from string to enum value
         v = (QString)arg.toString(exec);

      } else {
         v = QScriptEnginePrivate::jscValueToVariant(exec, arg, prop.userType());
      }

      QScriptable *scriptable  = scriptableFromQObject(qobject);
      QScriptEngine *oldEngine = nullptr;

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
   // Note: this has to be kept in sync with getOwnPropertyDescriptor

#ifndef QT_NO_PROPERTIES
   QString name = convertToString(propertyName.ustring());
   QObject *qobject = data->value;

   if (! qobject) {
      QString message = QString("Can not access member `%0' of deleted QObject").formatArg((name));
      slot.setValue(JSC::throwError(exec, JSC::GeneralError, message));
      return true;
   }

   const QMetaObject *meta = qobject->metaObject();
   {
      QHash<QString, JSC::JSValue>::const_iterator it = data->cachedMembers.constFind(name);

      if (it != data->cachedMembers.constEnd()) {
         if (GeneratePropertyFunctions && (meta->indexOfProperty(name) != -1)) {
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
      QString normalized = QMetaObject::normalizedSignature(name);

      if (-1 != (index = meta->indexOfMethod(normalized))) {
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

   index = meta->indexOfProperty(name);

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

   const int offset = (opt & QScriptEngine::ExcludeSuperClassMethods) ? meta->methodOffset() : 0;

   for (index = meta->methodCount() - 1; index >= offset; --index) {
      QMetaMethod method = meta->method(index);

      if (hasMethodAccess(method, index, opt) && methodNameEquals(method, name, name.length())) {
         QtFunction *fun = new (exec)QtFunction(object, index, true,
            &exec->globalData(), eng->originalGlobalObject()->functionStructure(), propertyName);

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
   // Note: this has to be kept in sync with getOwnPropertySlot

#ifndef QT_NO_PROPERTIES
   QString name = convertToString(propertyName.ustring());
   QObject *qobject = data->value;

   if (!qobject) {
      QString message = QString("cannot access member `%0' of deleted QObject").formatArg(name);
      descriptor.setValue(JSC::throwError(exec, JSC::GeneralError, message));
      return true;
   }

   const QScriptEngine::QObjectWrapOptions &opt = data->options;

   const QMetaObject *meta = qobject->metaObject();
   {
      QHash<QString, JSC::JSValue>::const_iterator it = data->cachedMembers.constFind(name);

      if (it != data->cachedMembers.constEnd()) {
         int index;

         if (GeneratePropertyFunctions && ((index = meta->indexOfProperty(name)) != -1)) {
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
      QString normalized = QMetaObject::normalizedSignature(name);

      if (-1 != (index = meta->indexOfMethod(normalized))) {
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

   index = meta->indexOfProperty(name);

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

   const int offset = (opt & QScriptEngine::ExcludeSuperClassMethods) ? meta->methodOffset() : 0;

   for (index = meta->methodCount() - 1; index >= offset; --index) {
      QMetaMethod method = meta->method(index);

      if (hasMethodAccess(method, index, opt) && methodNameEquals(method, name, name.length())) {
         QtFunction *fun = new (exec)QtFunction(object, index, true,
            &exec->globalData(), eng->originalGlobalObject()->functionStructure(), propertyName);

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
   QString name = convertToString(propertyName.ustring());
   QObject *qobject = data->value;

   if (! qobject) {
      QString message = QString("Can not access member `%0' of deleted QObject").formatArg(name);
      JSC::throwError(exec, JSC::GeneralError, message);
      return;
   }

   const QScriptEngine::QObjectWrapOptions &opt = data->options;
   const QMetaObject *meta = qobject->metaObject();

   QScriptEnginePrivate *eng = scriptEngineFromExec(exec);
   int index = -1;

   if (name.contains('(')) {
      QString normalized = QMetaObject::normalizedSignature(name);

      if (-1 != (index = meta->indexOfMethod(normalized))) {
         QMetaMethod method = meta->method(index);

         if (hasMethodAccess(method, index, opt)) {
            if (!(opt & QScriptEngine::ExcludeSuperClassMethods) || (index >= meta->methodOffset())) {
               data->cachedMembers.insert(name, value);
               return;
            }
         }
      }
   }

   index = meta->indexOfProperty(name);

   if (index != -1) {
      QMetaProperty prop = meta->property(index);

      if (prop.isScriptable()) {

         if (!(opt & QScriptEngine::ExcludeSuperClassProperties) || (index >= meta->propertyOffset())) {
            if (GeneratePropertyFunctions) {
               // ### ideally JSC would do this for us already, i.e. find out
               // that the property is a setter and call the setter.
               // Maybe QtPropertyFunction needs to inherit JSC::GetterSetter.
               JSC::JSValue fun;

               QHash<QString, JSC::JSValue>::const_iterator it;
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

      if (hasMethodAccess(method, index, opt) && methodNameEquals(method, name, name.length())) {
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
   QString name = convertToString(propertyName.ustring());
   QObject *qobject = data->value;

   if (!qobject) {
      QString message = QString("cannot access member `%0' of deleted QObject").formatArg(name);
      JSC::throwError(exec, JSC::GeneralError, message);
      return false;
   }

   const QMetaObject *meta = qobject->metaObject();
   {
      QHash<QString, JSC::JSValue>::iterator it = data->cachedMembers.find(name);

      if (it != data->cachedMembers.end()) {
         if (GeneratePropertyFunctions && (meta->indexOfProperty(name) != -1)) {
            return false;
         }

         data->cachedMembers.erase(it);
         return true;
      }
   }

   const QScriptEngine::QObjectWrapOptions &opt = data->options;
   int index = meta->indexOfProperty(name);

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
   JSC::PropertyNameArray &propertyNames, JSC::EnumerationMode mode)
{
#ifndef QT_NO_PROPERTIES
   QObject *qobject = data->value;

   if (!qobject) {
      QString message = QString("Can not access property names of deleted QObject");
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
            QString name = QString(prop.name());
            propertyNames.add(JSC::Identifier(exec, name));
         }
      }
   }

   {
      QList<QString> dpNames = qobject->dynamicPropertyNames();

      for (int i = 0; i < dpNames.size(); ++i) {
         QString name = QString(dpNames.at(i));
         propertyNames.add(JSC::Identifier(exec, name));
      }
   }

   if (!(opt & QScriptEngine::SkipMethodsInEnumeration)) {
      int i = (opt & QScriptEngine::ExcludeSuperClassMethods) ? meta->methodOffset() : 0;

      for ( ; i < meta->methodCount(); ++i) {
         QMetaMethod method = meta->method(i);

         if (hasMethodAccess(method, i, opt)) {
            QMetaMethod method = meta->method(i);

            QString sig = QString(method.methodSignature());
            propertyNames.add(JSC::Identifier(exec, sig));
         }
      }
   }

   QScriptObjectDelegate::getOwnPropertyNames(object, exec, propertyNames, mode);
#endif //QT_NO_PROPERTIES
}

void QObjectDelegate::markChildren(QScriptObject *object, JSC::MarkStack &markStack)
{
   QHash<QString, JSC::JSValue>::const_iterator it;

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
   (void) exec;

   if (! o2->inherits(&QScriptObject::info)) {
      return false;
   }

   QScriptObject *object = static_cast<QScriptObject *>(o2);
   QScriptObjectDelegate *delegate = object->delegate();

   if (! delegate || (delegate->type() != QScriptObjectDelegate::QtObject)) {
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
   QString str  = QString::fromUtf8("%0(name = \"%1\")").formatArg(meta->className()).formatArg(name);

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

const JSC::ClassInfo QMetaObjectWrapperObject::info = { "QMetaObject", nullptr, nullptr, nullptr };

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

bool QMetaObjectWrapperObject::getOwnPropertySlot(JSC::ExecState *exec, const JSC::Identifier &propertyName,
   JSC::PropertySlot &slot)
{
   const QMetaObject *meta = data->value;
   if (! meta) {
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

   QString name = convertToString(propertyName.ustring());

   for (int i = 0; i < meta->enumeratorCount(); ++i) {
      QMetaEnum e = meta->enumerator(i);

      for (int j = 0; j < e.keyCount(); ++j) {
         const QString &key = e.key(j);

         if (key == name) {
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

   QString name = propertyName.ustring();

   for (int i = 0; i < meta->enumeratorCount(); ++i) {
      QMetaEnum e = meta->enumerator(i);

      for (int j = 0; j < e.keyCount(); ++j) {
         const QString &key = e.key(j);

         if (key == name) {
            descriptor.setDescriptor(JSC::JSValue(exec, e.value(j)), JSC::ReadOnly | JSC::DontDelete);
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
      QString name = convertToString(propertyName.ustring());

      for (int i = 0; i < meta->enumeratorCount(); ++i) {
         QMetaEnum e = meta->enumerator(i);

         for (int j = 0; j < e.keyCount(); ++j) {
            if (e.key(j) == name) {
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
      QString name = convertToString(propertyName.ustring());

      for (int i = 0; i < meta->enumeratorCount(); ++i) {
         QMetaEnum e = meta->enumerator(i);

         for (int j = 0; j < e.keyCount(); ++j) {
            if (e.key(j) == name) {
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

   if (! result || !result.isObject()) {
      return nullptr;
   }

   return JSC::asObject(result);
}

JSC::JSValue QMetaObjectWrapperObject::execute(JSC::ExecState *exec, const JSC::ArgList &args)
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
         JSC::JSValue result = callQtMethod(exec, QMetaMethod::Constructor, nullptr,
               args, meta, meta->constructorCount() - 1, true);

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
         QString message = QString("no constructor for %0").formatArg(meta->className());
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

const QMetaObject &QObjectConnectionManager::staticMetaObject()
{
   return QObject::staticMetaObject();
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
      qWarning("CsScript: Unable to execute signal handler");

      // we can't do a script function call during GC, so we're forced to ignore this signal
      return;
   }

   const QMetaObject *meta  = sender()->metaObject();
   const QMetaMethod method = meta->method(signalIndex);

   QList<QString> parameterTypes = method.parameterTypes();
   int argc = parameterTypes.count();

   JSC::ExecState *exec = engine->currentFrame;
   QVarLengthArray<JSC::JSValue, 8> argsVector(argc);

   for (int i = 0; i < argc; ++i) {
      JSC::JSValue actual;
      void *arg = argv[i + 1];

      QString typeName = parameterTypes.at(i);
      uint argType = QVariant::nameToType(parameterTypes.at(i));

      if (argType == QVariant::Invalid) {
         qWarning("QScriptEngine: Unable to handle unregistered datatype '%s' "
            "when invoking handler of signal %s::%s",
            csPrintable(typeName), csPrintable(meta->className()), csPrintable(method.methodSignature()));

         actual = JSC::jsUndefined();

      } else if (argType == QVariant::Variant) {
         actual = QScriptEnginePrivate::jscValueFromVariant(exec, *reinterpret_cast<QVariant *>(arg));

      } else {
         actual = QScriptEnginePrivate::create(exec, QVariant(argType, arg));
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

void QObjectConnectionManager::clearMarkBits()
{
   for (int i = 0; i < connections.size(); ++i) {
      QVector<QObjectConnection> &cs = connections[i];
      for (int j = 0; j < cs.size(); ++j) {
         cs[j].marked = false;
      }
   }
}
int QObjectConnectionManager::mark(JSC::MarkStack &markStack)
{
   int markedCount = 0;

   for (int i = 0; i < connections.size(); ++i) {
      QVector<QObjectConnection> &cs = connections[i];

      for (int j = 0; j < cs.size(); ++j) {
         QObjectConnection &c = cs[j];

         if (!c.marked) {
            if (c.hasWeaklyReferencedSender()) {
               // do not mark the connection, we do not want the script-owned
               // sender object to stay alive merely due to a connection
            } else {
               c.mark(markStack);
               ++markedCount;
            }
         }
      }
   }

   return markedCount;
}

bool QObjectConnectionManager::addSignalHandler(QObject *sender, int signalIndex, JSC::JSValue receiver,
   JSC::JSValue function, JSC::JSValue senderWrapper, Qt::ConnectionType type)
{
   if (connections.size() <= signalIndex) {
      connections.resize(signalIndex + 1);
   }

   QVector<QObjectConnection> &cs = connections[signalIndex];

   (void) sender;
   (void) type;

   /* emerald (script, hold)
      int absSlotIndex = slotCounter + metaObject()->methodOffset();
      bool ok = QMetaObject::connect(sender, signalIndex, this, absSlotIndex, type);
   */

   // temp value, remove when above code comes back in
   bool ok = false;

   if (ok) {
      cs.append(QObjectConnection(slotCounter++, receiver, function, senderWrapper));
   }

   return ok;
}

bool QObjectConnectionManager::removeSignalHandler(QObject *sender, int signalIndex,
                  JSC::JSValue receiver, JSC::JSValue slot)
{
   (void) sender;

   if (connections.size() <= signalIndex) {
      return false;
   }

   QVector<QObjectConnection> &cs = connections[signalIndex];

   for (int i = 0; i < cs.size(); ++i) {
      const QObjectConnection &c = cs.at(i);

      if (c.hasTarget(receiver, slot)) {

         /* emerald (script, hold)
                     int absSlotIndex = c.slotIndex + metaObject()->methodOffset();
                     bool ok = QMetaObject::disconnect(sender, signalIndex, this, absSlotIndex);
         */

         // temp value, remove when abovt code comes back in
         bool ok = false;

         if (ok) {
            cs.remove(i);
         }

         return ok;
      }
   }
   return false;
}

QObjectData::QObjectData(QScriptEnginePrivate *eng)
   : engine(eng), connectionManager(nullptr)
{
}

QObjectData::~QObjectData()
{
   if (connectionManager) {
      delete connectionManager;
      connectionManager = nullptr;
   }
}

void QObjectData::clearConnectionMarkBits()
{
   if (connectionManager) {
      connectionManager->clearMarkBits();
   }
}
int QObjectData::markConnections(JSC::MarkStack &markStack)
{
   if (connectionManager) {
      return connectionManager->mark(markStack);
   }

   return 0;
}

// This function assumes all objects reachable elsewhere in the JS environment
// (stack, heap) have been marked already (see QScriptEnginePrivate::mark()).
// This determines whether any of Qt Script's internal QObject wrappers are only
// weakly referenced and can be discarded.
void QObjectData::markWrappers(JSC::MarkStack &markStack)
{
   QList<QScript::QObjectWrapperInfo>::iterator it;

   for (it = wrappers.begin(); it != wrappers.end(); ) {
      const QScript::QObjectWrapperInfo &info = *it;
      if (JSC::Heap::isCellMarked(info.object)) {
         ++it;
      } else if (info.isCollectableWhenWeaklyReferenced()) {
         it = wrappers.erase(it);
      } else {
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
   if (! connectionManager) {
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

   return nullptr;
}

void QObjectData::registerWrapper(QScriptObject *wrapper, QScriptEngine::ValueOwnership ownership,
   const QScriptEngine::QObjectWrapOptions &options)
{
   wrappers.append(QObjectWrapperInfo(wrapper, ownership, options));
}

} // namespace QScript

namespace JSC {
   ASSERT_CLASS_FITS_IN_CELL(QScript::QtFunction);
}
