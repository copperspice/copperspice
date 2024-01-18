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

// #define COMPILEDBINDINGS_DEBUG
// #define REGISTER_CLEANUP_DEBUG

#include <qdeclarativecompiledbindings_p.h>
#include <QtDeclarative/qdeclarativeinfo.h>
#include <qdeclarativecontext_p.h>
#include <qdeclarativejsast_p.h>
#include <qdeclarativejsengine_p.h>
#include <qdeclarativeexpression_p.h>
#include <QtCore/qcoreapplication.h>
#include <QtCore/qdebug.h>
#include <QtCore/qnumeric.h>
#include <qdeclarativeanchors_p_p.h>
#include <qdeclarativeglobal_p.h>
#include <qdeclarativefastproperties_p.h>
#include <qdeclarativedebugtrace_p.h>

QT_BEGIN_NAMESPACE

DEFINE_BOOL_CONFIG_OPTION(qmlExperimental, QML_EXPERIMENTAL);
DEFINE_BOOL_CONFIG_OPTION(qmlDisableOptimizer, QML_DISABLE_OPTIMIZER);
DEFINE_BOOL_CONFIG_OPTION(qmlDisableFastProperties, QML_DISABLE_FAST_PROPERTIES);
DEFINE_BOOL_CONFIG_OPTION(bindingsDump, QML_BINDINGS_DUMP);

Q_GLOBAL_STATIC(QDeclarativeFastProperties, fastProperties);

#if defined(Q_CC_GNU) && (! defined(Q_CC_INTEL) || __INTEL_COMPILER >= 1200)
#  define QML_THREADED_INTERPRETER
#endif

#define FOR_EACH_QML_INSTR(F) \
    F(Noop)                    /* Nop */ \
    F(BindingId)               /* id */ \
    F(Subscribe)               /* subscribe */ \
    F(SubscribeId)             /* subscribe */ \
    F(FetchAndSubscribe)       /* fetchAndSubscribe */ \
    F(LoadId)                  /* load */ \
    F(LoadScope)               /* load */ \
    F(LoadRoot)                /* load */ \
    F(LoadAttached)            /* attached */ \
    F(ConvertIntToReal)        /* unaryop */ \
    F(ConvertRealToInt)        /* unaryop */ \
    F(Real)                    /* real_value */ \
    F(Int)                     /* int_value */ \
    F(Bool)                    /* bool_value */ \
    F(String)                  /* string_value */ \
    F(AddReal)                 /* binaryop */ \
    F(AddInt)                  /* binaryop */ \
    F(AddString)               /* binaryop */ \
    F(MinusReal)               /* binaryop */ \
    F(MinusInt)                /* binaryop */ \
    F(CompareReal)             /* binaryop */ \
    F(CompareString)           /* binaryop */ \
    F(NotCompareReal)          /* binaryop */ \
    F(NotCompareString)        /* binaryop */ \
    F(GreaterThanReal)         /* binaryop */ \
    F(MaxReal)                 /* binaryop */ \
    F(MinReal)                 /* binaryop */ \
    F(NewString)               /* construct */ \
    F(NewUrl)                  /* construct */ \
    F(CleanupUrl)              /* cleanup */ \
    F(CleanupString)           /* cleanup */ \
    F(Copy)                    /* copy */ \
    F(Fetch)                   /* fetch */ \
    F(Store)                   /* store */ \
    F(Skip)                    /* skip */ \
    F(Done)                    /* done */ \
    /* Speculative property resolution */ \
    F(InitString)              /* initstring */ \
    F(FindGeneric)             /* find */ \
    F(FindGenericTerminal)     /* find */ \
    F(FindProperty)            /* find */ \
    F(FindPropertyTerminal)    /* find */ \
    F(CleanupGeneric)          /* cleanup */ \
    F(ConvertGenericToReal)    /* unaryop */ \
    F(ConvertGenericToBool)    /* unaryop */ \
    F(ConvertGenericToString)  /* unaryop */ \
    F(ConvertGenericToUrl)     /* unaryop */

#define QML_INSTR_ENUM(I) I,
#define QML_INSTR_ADDR(I) &&op_##I,

#ifdef QML_THREADED_INTERPRETER
#  define QML_BEGIN_INSTR(I) op_##I:
#  define QML_END_INSTR(I) ++instr; goto *instr->common.code;
#  define QML_INSTR_HEADER void *code;
#else
#  define QML_BEGIN_INSTR(I) case Instr::I:
#  define QML_END_INSTR(I) break;
#  define QML_INSTR_HEADER
#endif


using namespace QDeclarativeJS;

namespace {
// Supported types: int, qreal, QString (needs constr/destr), QObject*, bool
struct Register {
   void setUndefined() {
      type = 0;
   }
   void setUnknownButDefined() {
      type = -1;
   }
   void setNaN() {
      setqreal(qSNaN());
   }
   bool isUndefined() const {
      return type == 0;
   }

   void setQObject(QObject *o) {
      qobjectValue = o;
      type = QMetaType::QObjectStar;
   }
   QObject *getQObject() const {
      return qobjectValue;
   }

   void setqreal(qreal v) {
      qrealValue = v;
      type = QMetaType::QReal;
   }
   qreal getqreal() const {
      return qrealValue;
   }

   void setint(int v) {
      intValue = v;
      type = QMetaType::Int;
   }
   int getint() const {
      return intValue;
   }

   void setbool(bool v) {
      boolValue = v;
      type = QMetaType::Bool;
   }
   bool getbool() const {
      return boolValue;
   }

   QVariant *getvariantptr() {
      return (QVariant *)typeDataPtr();
   }
   QString *getstringptr() {
      return (QString *)typeDataPtr();
   }
   QUrl *geturlptr() {
      return (QUrl *)typeDataPtr();
   }
   const QVariant *getvariantptr() const {
      return (QVariant *)typeDataPtr();
   }
   const QString *getstringptr() const {
      return (QString *)typeDataPtr();
   }
   const QUrl *geturlptr() const {
      return (QUrl *)typeDataPtr();
   }

   void *typeDataPtr() {
      return (void *)&data;
   }
   void *typeMemory() {
      return (void *)data;
   }
   const void *typeDataPtr() const {
      return (void *)&data;
   }
   const void *typeMemory() const {
      return (void *)data;
   }

   int gettype() const {
      return type;
   }
   void settype(int t) {
      type = t;
   }

   int type;          // Optional type
   union {
      QObject *qobjectValue;
      qreal qrealValue;
      int intValue;
      bool boolValue;
      char data[sizeof(QVariant)];
      qint64 q_for_alignment_1;
      double q_for_alignment_2;
   };

#ifdef REGISTER_CLEANUP_DEBUG
   Register() {
      type = 0;
   }

   ~Register() {
      int allowedTypes[] = { QMetaType::QObjectStar, QMetaType::QReal, QMetaType::Int, QMetaType::Bool, 0 };
      bool found = (type == 0);
      int *ctype = allowedTypes;
      while (!found && *ctype) {
         found = (*ctype == type);
         ++ctype;
      }
      if (!found) {
         qWarning("Register leaked of type %d", type);
      }
   }
#endif
};
}

class QDeclarativeCompiledBindingsPrivate
{
   Q_DECLARE_PUBLIC(QDeclarativeCompiledBindings)

 public:
   QDeclarativeCompiledBindingsPrivate();
   virtual ~QDeclarativeCompiledBindingsPrivate();

   struct Binding : public QDeclarativeAbstractBinding, public QDeclarativeDelayedError {
      Binding() : enabled(false), updating(0), property(0),
         scope(0), target(0), parent(0) {}

      // Inherited from QDeclarativeAbstractBinding
      virtual void setEnabled(bool, QDeclarativePropertyPrivate::WriteFlags flags);
      virtual void update(QDeclarativePropertyPrivate::WriteFlags flags);
      virtual void destroy(DestroyMode mode);
      virtual void disconnect(DisconnectMode disconnectMode);

      int index: 30;
      bool enabled: 1;
      bool updating: 1;
      int property;
      QObject *scope;
      QObject *target;

      QDeclarativeCompiledBindingsPrivate *parent;
   };

   typedef QDeclarativeNotifierEndpoint Subscription;
   Subscription *subscriptions;
   QScriptDeclarativeClass::PersistentIdentifier *identifiers;

   void run(Binding *, QDeclarativePropertyPrivate::WriteFlags flags);

   const char *programData;
   QDeclarativeRefCount *dataRef;
   Binding *m_bindings;
   quint32 *m_signalTable;
   bool m_bindingsDisconnected;

   static int methodCount;

   void init();
   void run(int instr, QDeclarativeContextData *context,
            QDeclarativeDelayedError *error, QObject *scope, QObject *output, QDeclarativePropertyPrivate::WriteFlags storeFlags);


   inline void subscribeId(QDeclarativeContextData *p, int idIndex, int subIndex);
   inline void subscribe(QObject *o, int notifyIndex, int subIndex);
   inline void disconnectAll();
   inline void disconnectOne(Binding *bindingToDisconnect);

   QDeclarativePropertyCache::Data *findproperty(QObject *obj,
         const QScriptDeclarativeClass::Identifier &name,
         QDeclarativeEnginePrivate *enginePriv,
         QDeclarativePropertyCache::Data &local);
   bool findproperty(QObject *obj,
                     Register *output,
                     QDeclarativeEnginePrivate *enginePriv,
                     int subIdx,
                     const QScriptDeclarativeClass::Identifier &name,
                     bool isTerminal);
   void findgeneric(Register *output,                                 // value output
                    int subIdx,                                       // Subscription index in config
                    QDeclarativeContextData *context,                 // Context to search in
                    const QScriptDeclarativeClass::Identifier &name,
                    bool isTerminal);
};

QDeclarativeCompiledBindingsPrivate::QDeclarativeCompiledBindingsPrivate()
   : subscriptions(0), identifiers(0), programData(0), dataRef(0), m_bindings(0), m_signalTable(0),
     m_bindingsDisconnected(false)
{
}

QDeclarativeCompiledBindingsPrivate::~QDeclarativeCompiledBindingsPrivate()
{
   delete [] subscriptions;
   subscriptions = 0;
   delete [] identifiers;
   identifiers = 0;
   if (dataRef) {
      dataRef->release();
      dataRef = 0;
   }
}

int QDeclarativeCompiledBindingsPrivate::methodCount = -1;

QDeclarativeCompiledBindings::QDeclarativeCompiledBindings(const char *program, QDeclarativeContextData *context,
      QDeclarativeRefCount *dataRef)
   : QObject(*(new QDeclarativeCompiledBindingsPrivate))
{
   Q_D(QDeclarativeCompiledBindings);

   if (d->methodCount == -1) {
      d->methodCount = QDeclarativeCompiledBindings::staticMetaObject.methodCount();
   }

   d->programData = program;
   d->dataRef = dataRef;
   if (d->dataRef) {
      d->dataRef->addref();
   }

   d->init();

   QDeclarativeAbstractExpression::setContext(context);
}

QDeclarativeCompiledBindings::~QDeclarativeCompiledBindings()
{
   Q_D(QDeclarativeCompiledBindings);

   delete [] d->m_bindings;
}

QDeclarativeAbstractBinding *QDeclarativeCompiledBindings::configBinding(int index, QObject *target,
      QObject *scope, int property)
{
   Q_D(QDeclarativeCompiledBindings);

   QDeclarativeCompiledBindingsPrivate::Binding *rv = d->m_bindings + index;

   rv->index = index;
   rv->property = property;
   rv->target = target;
   rv->scope = scope;
   rv->parent = d;

   addref(); // This is decremented in Binding::destroy()

   return rv;
}

void QDeclarativeCompiledBindingsPrivate::Binding::setEnabled(bool e, QDeclarativePropertyPrivate::WriteFlags flags)
{
   if (enabled != e) {
      enabled = e;

      if (e) {
         update(flags);
      }
   }
}

void QDeclarativeCompiledBindingsPrivate::Binding::update(QDeclarativePropertyPrivate::WriteFlags flags)
{
   QDeclarativeDebugTrace::startRange(QDeclarativeDebugTrace::Binding);
   parent->run(this, flags);
   QDeclarativeDebugTrace::endRange(QDeclarativeDebugTrace::Binding);
}

void QDeclarativeCompiledBindingsPrivate::Binding::destroy(DestroyMode mode)
{
   if (mode == DisconnectBinding) {
      disconnect(QDeclarativeAbstractBinding::DisconnectOne);
   }

   enabled = false;
   removeFromObject();
   clear();
   parent->q_func()->release();
}

void QDeclarativeCompiledBindingsPrivate::Binding::disconnect(DisconnectMode disconnectMode)
{
   if (disconnectMode == QDeclarativeAbstractBinding::DisconnectAll) {
      parent->disconnectAll();
   } else {
      parent->disconnectOne(this);
   }
}

int QDeclarativeCompiledBindings::qt_metacall(QMetaObject::Call c, int id, void **)
{
   Q_D(QDeclarativeCompiledBindings);

   if (c == QMetaObject::InvokeMetaMethod && id >= d->methodCount) {
      id -= d->methodCount;

      quint32 *reeval = d->m_signalTable + d->m_signalTable[id];
      quint32 count = *reeval;
      ++reeval;
      for (quint32 ii = 0; ii < count; ++ii) {
         d->run(d->m_bindings + reeval[ii], QDeclarativePropertyPrivate::DontRemoveBinding);
      }
   }
   return -1;
}

void QDeclarativeCompiledBindingsPrivate::run(Binding *binding, QDeclarativePropertyPrivate::WriteFlags flags)
{
   Q_Q(QDeclarativeCompiledBindings);

   if (!binding->enabled) {
      return;
   }

   QDeclarativeContextData *context = q->QDeclarativeAbstractExpression::context();
   if (!context || !context->isValid()) {
      return;
   }

   if (binding->updating) {
      QString name;
      if (binding->property & 0xFFFF0000) {
         QDeclarativeEnginePrivate *ep = QDeclarativeEnginePrivate::get(context->engine);

         QDeclarativeValueType *vt = ep->valueTypes[(binding->property >> 16) & 0xFF];
         Q_ASSERT(vt);

         name = QLatin1String(binding->target->metaObject()->property(binding->property & 0xFFFF).name());
         name.append(QLatin1String("."));
         name.append(QLatin1String(vt->metaObject()->property(binding->property >> 24).name()));
      } else {
         name = QLatin1String(binding->target->metaObject()->property(binding->property).name());
      }
      qmlInfo(binding->target) << QCoreApplication::translate("QDeclarativeCompiledBindings",
                               "Binding loop detected for property \"%1\"").arg(name);
      return;
   }

   binding->updating = true;
   if (binding->property & 0xFFFF0000) {
      QDeclarativeEnginePrivate *ep = QDeclarativeEnginePrivate::get(context->engine);

      QDeclarativeValueType *vt = ep->valueTypes[(binding->property >> 16) & 0xFF];
      Q_ASSERT(vt);
      vt->read(binding->target, binding->property & 0xFFFF);

      QObject *target = vt;
      run(binding->index, context, binding, binding->scope, target, flags);

      vt->write(binding->target, binding->property & 0xFFFF, flags);
   } else {
      run(binding->index, context, binding, binding->scope, binding->target, flags);
   }
   binding->updating = false;
}

namespace {
// This structure is exactly 8-bytes in size
struct Instr {
   enum {
      FOR_EACH_QML_INSTR(QML_INSTR_ENUM)
   };

   union {
      struct {
         QML_INSTR_HEADER
         quint8 type;
         quint8 packing[7];
      } common;
      struct {
         QML_INSTR_HEADER
         quint8 type;
         quint8 packing;
         quint16 column;
         quint32 line;
      } id;
      struct {
         QML_INSTR_HEADER
         quint8 type;
         quint8 packing[3];
         quint16 subscriptions;
         quint16 identifiers;
      } init;
      struct {
         QML_INSTR_HEADER
         quint8 type;
         qint8 reg;
         quint16 offset;
         quint32 index;
      } subscribe;
      struct {
         QML_INSTR_HEADER
         quint8 type;
         qint8 reg;
         quint8 packing[2];
         quint32 index;
      } load;
      struct {
         QML_INSTR_HEADER
         quint8 type;
         qint8 output;
         qint8 reg;
         quint8 exceptionId;
         quint32 id;
      } attached;
      struct {
         QML_INSTR_HEADER
         quint8 type;
         qint8 output;
         qint8 reg;
         quint8 exceptionId;
         quint32 index;
      } store;
      struct {
         QML_INSTR_HEADER
         quint8 type;
         qint8 output;
         qint8 objectReg;
         quint8 exceptionId;
         quint16 subscription;
         quint16 function;
      } fetchAndSubscribe;
      struct {
         QML_INSTR_HEADER
         quint8 type;
         qint8 output;
         qint8 objectReg;
         quint8 exceptionId;
         quint32 index;
      } fetch;
      struct {
         QML_INSTR_HEADER
         quint8 type;
         qint8 reg;
         qint8 src;
         quint8 packing[5];
      } copy;
      struct {
         QML_INSTR_HEADER
         quint8 type;
         qint8 reg;
         quint8 packing[6];
      } construct;
      struct {
         QML_INSTR_HEADER
         quint8 type;
         qint8 reg;
         quint8 packing[2];
         float value;
      } real_value;
      struct {
         QML_INSTR_HEADER
         quint8 type;
         qint8 reg;
         quint8 packing[2];
         int value;
      } int_value;
      struct {
         QML_INSTR_HEADER
         quint8 type;
         qint8 reg;
         bool value;
         quint8 packing[5];
      } bool_value;
      struct {
         QML_INSTR_HEADER
         quint8 type;
         qint8 reg;
         quint16 length;
         quint32 offset;
      } string_value;
      struct {
         QML_INSTR_HEADER
         quint8 type;
         qint8 output;
         qint8 src1;
         qint8 src2;
         quint8 packing[4];
      } binaryop;
      struct {
         QML_INSTR_HEADER
         quint8 type;
         qint8 output;
         qint8 src;
         quint8 packing[5];
      } unaryop;
      struct {
         QML_INSTR_HEADER
         quint8 type;
         qint8 reg;
         quint8 packing[2];
         quint32 count;
      } skip;
      struct {
         QML_INSTR_HEADER
         quint8 type;
         qint8 reg;
         qint8 src;
         quint8 exceptionId;
         quint16 name;
         quint16 subscribeIndex;
      } find;
      struct {
         QML_INSTR_HEADER
         quint8 type;
         qint8 reg;
         quint8 packing[6];
      } cleanup;
      struct {
         QML_INSTR_HEADER
         quint8 type;
         quint8 packing[1];
         quint16 offset;
         quint32 dataIdx;
      } initstring;
   };
};

struct Program {
   quint32 bindings;
   quint32 dataLength;
   quint32 signalTableOffset;
   quint32 exceptionDataOffset;
   quint16 subscriptions;
   quint16 identifiers;
   quint16 instructionCount;
   quint16 compiled;

   const char *data() const {
      return ((const char *)this) + sizeof(Program);
   }
   const Instr *instructions() const {
      return (const Instr *)(data() + dataLength);
   }
};
}

struct QDeclarativeBindingCompilerPrivate {
   struct Result {
      Result() : unknownType(false), metaObject(0), type(-1), reg(-1) {}
      bool operator==(const Result &o) const {
         return unknownType == o.unknownType &&
                metaObject == o.metaObject &&
                type == o.type &&
                reg == o.reg;
      }
      bool operator!=(const Result &o) const {
         return !(*this == o);
      }
      bool unknownType;
      const QMetaObject *metaObject;
      int type;
      int reg;

      QSet<QString> subscriptionSet;
   };

   QDeclarativeBindingCompilerPrivate() : registers(0) {}

   void resetInstanceState();
   int commitCompile();

   QDeclarativeParser::Object *context;
   QDeclarativeParser::Object *component;
   QDeclarativeParser::Property *destination;
   QHash<QString, QDeclarativeParser::Object *> ids;
   QDeclarativeImports imports;
   QDeclarativeEnginePrivate *engine;

   QString contextName() const {
      return QLatin1String("$$$SCOPE_") + QString::number((quintptr)context, 16);
   }

   bool compile(QDeclarativeJS::AST::Node *);

   bool parseExpression(QDeclarativeJS::AST::Node *, Result &);

   bool tryName(QDeclarativeJS::AST::Node *);
   bool parseName(QDeclarativeJS::AST::Node *, Result &);

   bool tryArith(QDeclarativeJS::AST::Node *);
   bool parseArith(QDeclarativeJS::AST::Node *, Result &);
   bool numberArith(Result &, const Result &, const Result &, QSOperator::Op op);
   bool stringArith(Result &, const Result &, const Result &, QSOperator::Op op);

   bool tryLogic(QDeclarativeJS::AST::Node *);
   bool parseLogic(QDeclarativeJS::AST::Node *, Result &);

   bool tryConditional(QDeclarativeJS::AST::Node *);
   bool parseConditional(QDeclarativeJS::AST::Node *, Result &);

   bool tryConstant(QDeclarativeJS::AST::Node *);
   bool parseConstant(QDeclarativeJS::AST::Node *, Result &);

   bool tryMethod(QDeclarativeJS::AST::Node *);
   bool parseMethod(QDeclarativeJS::AST::Node *, Result &);

   bool buildName(QStringList &, QDeclarativeJS::AST::Node *, QList<QDeclarativeJS::AST::ExpressionNode *> *nodes = 0);
   bool fetch(Result &type, const QMetaObject *, int reg, int idx, const QStringList &,
              QDeclarativeJS::AST::ExpressionNode *);

   quint32 registers;
   QHash<int, QPair<int, int> > registerCleanups;
   int acquireReg(int cleanup = Instr::Noop, int cleanupType = 0);
   void registerCleanup(int reg, int cleanup, int cleanupType = 0);
   void releaseReg(int);

   int registerLiteralString(const QString &);
   int registerString(const QString &);
   QHash<QString, QPair<int, int> > registeredStrings;
   QByteArray data;

   bool subscription(const QStringList &, Result *);
   int subscriptionIndex(const QStringList &);
   bool subscriptionNeutral(const QSet<QString> &base, const QSet<QString> &lhs, const QSet<QString> &rhs);

   quint8 exceptionId(QDeclarativeJS::AST::ExpressionNode *);
   QVector<quint64> exceptions;

   QSet<int> usedSubscriptionIds;
   QSet<QString> subscriptionSet;
   QHash<QString, int> subscriptionIds;
   QVector<Instr> bytecode;

   // Committed binding data
   struct {
      QList<int> offsets;
      QList<QSet<int> > dependencies;

      QVector<Instr> bytecode;
      QByteArray data;
      QHash<QString, int> subscriptionIds;
      QVector<quint64> exceptions;

      QHash<QString, QPair<int, int> > registeredStrings;

      int count() const {
         return offsets.count();
      }
   } committed;

   QByteArray buildSignalTable() const;
   QByteArray buildExceptionData() const;
};

void QDeclarativeCompiledBindingsPrivate::subscribeId(QDeclarativeContextData *p, int idIndex, int subIndex)
{
   Q_Q(QDeclarativeCompiledBindings);

   QDeclarativeCompiledBindingsPrivate::Subscription *sub = (subscriptions + subIndex);
   sub->disconnect();

   if (p->idValues[idIndex]) {
      sub->target = q;
      sub->targetMethod = methodCount + subIndex;
      sub->connect(&p->idValues[idIndex].bindings);
   }
}

void QDeclarativeCompiledBindingsPrivate::subscribe(QObject *o, int notifyIndex, int subIndex)
{
   Q_Q(QDeclarativeCompiledBindings);

   QDeclarativeCompiledBindingsPrivate::Subscription *sub = (subscriptions + subIndex);
   sub->target = q;
   sub->targetMethod = methodCount + subIndex;
   if (o) {
      sub->connect(o, notifyIndex);
   } else {
      sub->disconnect();
   }
}

void QDeclarativeCompiledBindingsPrivate::disconnectAll()
{
   // This gets called multiple times in QDeclarativeData::disconnectNotifiers(), avoid unneeded
   // work for all but the first call.
   if (m_bindingsDisconnected) {
      return;
   }

   // We disconnect all subscriptions, so we can call disconnect() unconditionally if there is at
   // least one connection
   Program *program = (Program *)programData;
   for (int subIndex = 0; subIndex < program->subscriptions; ++subIndex) {
      Subscription *const sub = (subscriptions + subIndex);
      if (sub->isConnected()) {
         sub->disconnect();
      }
   }
   m_bindingsDisconnected = true;
}

void QDeclarativeCompiledBindingsPrivate::disconnectOne(
   QDeclarativeCompiledBindingsPrivate::Binding *bindingToDisconnect)
{
   // We iterate over the signal table to find all subscriptions for this binding. This is slowish,
   // but disconnectOne() is only called when overwriting a binding, which is quite rare.
   Program *program = (Program *)programData;
   for (int subIndex = 0; subIndex < program->subscriptions; ++subIndex) {
      Subscription *const sub = (subscriptions + subIndex);
      quint32 *reeval = m_signalTable + m_signalTable[subIndex];
      quint32 bindingCount = *reeval;
      ++reeval;
      for (quint32 bindingIndex = 0; bindingIndex < bindingCount; ++bindingIndex) {
         Binding *const binding = m_bindings + reeval[bindingIndex];
         if (binding == bindingToDisconnect) {
            sub->deref();
         }
      }
   }
}

// Conversion functions - these MUST match the QtScript expression path
inline static qreal toReal(Register *reg, int type, bool *ok = 0)
{
   if (ok) {
      *ok = true;
   }

   if (type == QMetaType::QReal) {
      return reg->getqreal();
   } else if (type == qMetaTypeId<QVariant>()) {
      return reg->getvariantptr()->toReal();
   } else {
      if (ok) {
         *ok = false;
      }
      return 0;
   }
}

inline static QString toString(Register *reg, int type, bool *ok = 0)
{
   if (ok) {
      *ok = true;
   }

   if (type == QMetaType::QReal) {
      return QString::number(reg->getqreal());
   } else if (type == QMetaType::Int) {
      return QString::number(reg->getint());
   } else if (type == qMetaTypeId<QVariant>()) {
      return reg->getvariantptr()->toString();
   } else if (type == QMetaType::QString) {
      return *reg->getstringptr();
   } else {
      if (ok) {
         *ok = false;
      }
      return QString();
   }
}

inline static bool toBool(Register *reg, int type, bool *ok = 0)
{
   if (ok) {
      *ok = true;
   }

   if (type == QMetaType::Bool) {
      return reg->getbool();
   } else if (type == qMetaTypeId<QVariant>()) {
      return reg->getvariantptr()->toBool();
   } else {
      if (ok) {
         *ok = false;
      }
      return false;
   }
}

inline static QUrl toUrl(Register *reg, int type, QDeclarativeContextData *context, bool *ok = 0)
{
   if (ok) {
      *ok = true;
   }

   QUrl base;
   if (type == qMetaTypeId<QVariant>()) {
      QVariant *var = reg->getvariantptr();
      int vt = var->type();
      if (vt == QVariant::Url) {
         base = var->toUrl();
      } else if (vt == QVariant::ByteArray) {
         base = QUrl(QString::fromUtf8(var->toByteArray()));
      } else if (vt == QVariant::String) {
         base = QUrl(var->toString());
      } else {
         if (ok) {
            *ok = false;
         }
         return QUrl();
      }
   } else if (type == QMetaType::QString) {
      base = QUrl(*reg->getstringptr());
   } else {
      if (ok) {
         *ok = false;
      }
      return QUrl();
   }

   if (!base.isEmpty() && base.isRelative()) {
      return context->url.resolved(base);
   } else {
      return base;
   }
}

static QObject *variantToQObject(const QVariant &value, bool *ok)
{
   if (ok) {
      *ok = true;
   }

   if (value.userType() == QMetaType::QObjectStar) {
      return qvariant_cast<QObject *>(value);
   } else {
      if (ok) {
         *ok = false;
      }
      return 0;
   }
}

bool QDeclarativeCompiledBindingsPrivate::findproperty(QObject *obj, Register *output,
      QDeclarativeEnginePrivate *enginePriv,
      int subIdx, const QScriptDeclarativeClass::Identifier &name,
      bool isTerminal)
{
   if (!obj) {
      output->setUndefined();
      return false;
   }

   QDeclarativePropertyCache::Data local;
   QDeclarativePropertyCache::Data *property =
      QDeclarativePropertyCache::property(QDeclarativeEnginePrivate::get(enginePriv), obj, name, local);

   if (property) {
      if (subIdx != -1) {
         subscribe(obj, property->notifyIndex, subIdx);
      }

      if (property->flags & QDeclarativePropertyCache::Data::IsQObjectDerived) {
         void *args[] = { output->typeDataPtr(), 0 };
         QMetaObject::metacall(obj, QMetaObject::ReadProperty, property->coreIndex, args);
         output->settype(QMetaType::QObjectStar);
      } else if (property->propType == qMetaTypeId<QVariant>()) {
         QVariant v;
         void *args[] = { &v, 0 };
         QMetaObject::metacall(obj, QMetaObject::ReadProperty, property->coreIndex, args);

         if (isTerminal) {
            new (output->typeDataPtr()) QVariant(v);
            output->settype(qMetaTypeId<QVariant>());
         } else {
            bool ok;
            output->setQObject(variantToQObject(v, &ok));
            if (!ok) {
               output->setUndefined();
            } else {
               output->settype(QMetaType::QObjectStar);
            }
         }

      } else {
         if (!isTerminal) {
            output->setUndefined();
         } else if (property->propType == QMetaType::QReal) {
            void *args[] = { output->typeDataPtr(), 0 };
            QMetaObject::metacall(obj, QMetaObject::ReadProperty, property->coreIndex, args);
            output->settype(QMetaType::QReal);
         } else if (property->propType == QMetaType::Int) {
            void *args[] = { output->typeDataPtr(), 0 };
            QMetaObject::metacall(obj, QMetaObject::ReadProperty, property->coreIndex, args);
            output->settype(QMetaType::Int);
         } else if (property->propType == QMetaType::Bool) {
            void *args[] = { output->typeDataPtr(), 0 };
            QMetaObject::metacall(obj, QMetaObject::ReadProperty, property->coreIndex, args);
            output->settype(QMetaType::Bool);
         } else if (property->propType == QMetaType::QString) {
            new (output->typeDataPtr()) QString();
            void *args[] = { output->typeDataPtr(), 0 };
            QMetaObject::metacall(obj, QMetaObject::ReadProperty, property->coreIndex, args);
            output->settype(QMetaType::QString);
         } else {
            new (output->typeDataPtr())
            QVariant(obj->metaObject()->property(property->coreIndex).read(obj));
            output->settype(qMetaTypeId<QVariant>());
         }
      }

      return true;
   } else {
      output->setUndefined();
      return false;
   }
}

void QDeclarativeCompiledBindingsPrivate::findgeneric(Register *output,
      int subIdx,
      QDeclarativeContextData *context,
      const QScriptDeclarativeClass::Identifier &name,
      bool isTerminal)
{
   QDeclarativeEnginePrivate *enginePriv = QDeclarativeEnginePrivate::get(context->engine);

   while (context) {

      int contextPropertyIndex = context->propertyNames ? context->propertyNames->value(name) : -1;


      if (contextPropertyIndex != -1) {

         if (contextPropertyIndex < context->idValueCount) {
            output->setQObject(context->idValues[contextPropertyIndex]);
            output->settype(QMetaType::QObjectStar);

            if (subIdx != -1) {
               subscribeId(context, contextPropertyIndex, subIdx);
            }

         } else {
            QDeclarativeContextPrivate *cp = context->asQDeclarativeContextPrivate();
            const QVariant &value = cp->propertyValues.at(contextPropertyIndex);

            if (isTerminal) {
               new (output->typeDataPtr()) QVariant(value);
               output->settype(qMetaTypeId<QVariant>());
            } else {
               bool ok;
               output->setQObject(variantToQObject(value, &ok));
               if (!ok) {
                  output->setUndefined();
               } else {
                  output->settype(QMetaType::QObjectStar);
               }
               return;
            }

            if (subIdx != -1) {
               subscribe(context->asQDeclarativeContext(), contextPropertyIndex + cp->notifyIndex, subIdx);
            }


         }

         return;
      }

      if (QObject *root = context->contextObject) {

         if (findproperty(root, output, enginePriv, subIdx, name, isTerminal)) {
            return;
         }

      }

      context = context->parent;
   }

   output->setUndefined();
}

void QDeclarativeCompiledBindingsPrivate::init()
{
   Program *program = (Program *)programData;
   if (program->subscriptions) {
      subscriptions = new QDeclarativeCompiledBindingsPrivate::Subscription[program->subscriptions];
   }
   if (program->identifiers) {
      identifiers = new QScriptDeclarativeClass::PersistentIdentifier[program->identifiers];
   }

   m_signalTable = (quint32 *)(program->data() + program->signalTableOffset);
   m_bindings = new QDeclarativeCompiledBindingsPrivate::Binding[program->bindings];
}

static void throwException(int id, QDeclarativeDelayedError *error,
                           Program *program, QDeclarativeContextData *context,
                           const QString &description = QString())
{
   error->error.setUrl(context->url);
   if (description.isEmpty()) {
      error->error.setDescription(QLatin1String("TypeError: Result of expression is not an object"));
   } else {
      error->error.setDescription(description);
   }
   if (id != 0xFF) {
      quint64 e = *((quint64 *)(program->data() + program->exceptionDataOffset) + id);
      error->error.setLine((e >> 32) & 0xFFFFFFFF);
      error->error.setColumn(e & 0xFFFFFFFF);
   } else {
      error->error.setLine(-1);
      error->error.setColumn(-1);
   }
   if (!context->engine || !error->addError(QDeclarativeEnginePrivate::get(context->engine))) {
      QDeclarativeEnginePrivate::warning(context->engine, error->error);
   }
}

static void dumpInstruction(const Instr *instr)
{
   switch (instr->common.type) {
      case Instr::Noop:
         qWarning().nospace() << "\t" << "Noop";
         break;
      case Instr::BindingId:
         qWarning().nospace() << instr->id.line << ":" << instr->id.column << ":";
         break;
      case Instr::Subscribe:
         qWarning().nospace() << "\t" << "Subscribe" << "\t\t" << instr->subscribe.offset << "\t" << instr->subscribe.reg << "\t"
                              << instr->subscribe.index;
         break;
      case Instr::SubscribeId:
         qWarning().nospace() << "\t" << "SubscribeId" << "\t\t" << instr->subscribe.offset << "\t" << instr->subscribe.reg <<
                              "\t" << instr->subscribe.index;
         break;
      case Instr::FetchAndSubscribe:
         qWarning().nospace() << "\t" << "FetchAndSubscribe" << "\t" << instr->fetchAndSubscribe.output << "\t" <<
                              instr->fetchAndSubscribe.objectReg << "\t" << instr->fetchAndSubscribe.subscription;
         break;
      case Instr::LoadId:
         qWarning().nospace() << "\t" << "LoadId" << "\t\t\t" << instr->load.index << "\t" << instr->load.reg;
         break;
      case Instr::LoadScope:
         qWarning().nospace() << "\t" << "LoadScope" << "\t\t" << instr->load.index << "\t" << instr->load.reg;
         break;
      case Instr::LoadRoot:
         qWarning().nospace() << "\t" << "LoadRoot" << "\t\t" << instr->load.index << "\t" << instr->load.reg;
         break;
      case Instr::LoadAttached:
         qWarning().nospace() << "\t" << "LoadAttached" << "\t\t" << instr->attached.output << "\t" << instr->attached.reg <<
                              "\t" << instr->attached.id;
         break;
      case Instr::ConvertIntToReal:
         qWarning().nospace() << "\t" << "ConvertIntToReal" << "\t" << instr->unaryop.output << "\t" << instr->unaryop.src;
         break;
      case Instr::ConvertRealToInt:
         qWarning().nospace() << "\t" << "ConvertRealToInt" << "\t" << instr->unaryop.output << "\t" << instr->unaryop.src;
         break;
      case Instr::Real:
         qWarning().nospace() << "\t" << "Real" << "\t\t\t" << instr->real_value.reg << "\t" << instr->real_value.value;
         break;
      case Instr::Int:
         qWarning().nospace() << "\t" << "Int" << "\t\t\t" << instr->int_value.reg << "\t" << instr->int_value.value;
         break;
      case Instr::Bool:
         qWarning().nospace() << "\t" << "Bool" << "\t\t\t" << instr->bool_value.reg << "\t" << instr->bool_value.value;
         break;
      case Instr::String:
         qWarning().nospace() << "\t" << "String" << "\t\t\t" << instr->string_value.reg << "\t" << instr->string_value.offset <<
                              "\t" << instr->string_value.length;
         break;
      case Instr::AddReal:
         qWarning().nospace() << "\t" << "AddReal" << "\t\t\t" << instr->binaryop.output << "\t" << instr->binaryop.src1 << "\t"
                              << instr->binaryop.src2;
         break;
      case Instr::AddInt:
         qWarning().nospace() << "\t" << "AddInt" << "\t\t\t" << instr->binaryop.output << "\t" << instr->binaryop.src1 << "\t"
                              << instr->binaryop.src2;
         break;
      case Instr::AddString:
         qWarning().nospace() << "\t" << "AddString" << "\t\t" << instr->binaryop.output << "\t" << instr->binaryop.src1 << "\t"
                              << instr->binaryop.src2;
         break;
      case Instr::MinusReal:
         qWarning().nospace() << "\t" << "MinusReal" << "\t\t" << instr->binaryop.output << "\t" << instr->binaryop.src1 << "\t"
                              << instr->binaryop.src2;
         break;
      case Instr::MinusInt:
         qWarning().nospace() << "\t" << "MinusInt" << "\t\t" << instr->binaryop.output << "\t" << instr->binaryop.src1 << "\t"
                              << instr->binaryop.src2;
         break;
      case Instr::CompareReal:
         qWarning().nospace() << "\t" << "CompareReal" << "\t\t" << instr->binaryop.output << "\t" << instr->binaryop.src1 <<
                              "\t" << instr->binaryop.src2;
         break;
      case Instr::CompareString:
         qWarning().nospace() << "\t" << "CompareString" << "\t\t" << instr->binaryop.output << "\t" << instr->binaryop.src1 <<
                              "\t" << instr->binaryop.src2;
         break;
      case Instr::NotCompareReal:
         qWarning().nospace() << "\t" << "NotCompareReal" << "\t\t" << instr->binaryop.output << "\t" << instr->binaryop.src1 <<
                              "\t" << instr->binaryop.src2;
         break;
      case Instr::NotCompareString:
         qWarning().nospace() << "\t" << "NotCompareString" << "\t\t" << instr->binaryop.output << "\t" << instr->binaryop.src1
                              << "\t" << instr->binaryop.src2;
         break;
      case Instr::GreaterThanReal:
         qWarning().nospace() << "\t" << "GreaterThanReal" << "\t\t" << instr->binaryop.output << "\t" << instr->binaryop.src1 <<
                              "\t" << instr->binaryop.src2;
         break;
      case Instr::MaxReal:
         qWarning().nospace() << "\t" << "MaxReal" << "\t\t\t" << instr->binaryop.output << "\t" << instr->binaryop.src1 << "\t"
                              << instr->binaryop.src2;
         break;
      case Instr::MinReal:
         qWarning().nospace() << "\t" << "MinReal" << "\t\t\t" << instr->binaryop.output << "\t" << instr->binaryop.src1 << "\t"
                              << instr->binaryop.src2;
         break;
      case Instr::NewString:
         qWarning().nospace() << "\t" << "NewString" << "\t\t" << instr->construct.reg;
         break;
      case Instr::NewUrl:
         qWarning().nospace() << "\t" << "NewUrl" << "\t\t\t" << instr->construct.reg;
         break;
      case Instr::CleanupString:
         qWarning().nospace() << "\t" << "CleanupString" << "\t\t" << instr->cleanup.reg;
         break;
      case Instr::CleanupUrl:
         qWarning().nospace() << "\t" << "CleanupUrl" << "\t\t" << instr->cleanup.reg;
         break;
      case Instr::Fetch:
         qWarning().nospace() << "\t" << "Fetch" << "\t\t\t" << instr->fetch.output << "\t" << instr->fetch.index << "\t" <<
                              instr->fetch.objectReg;
         break;
      case Instr::Store:
         qWarning().nospace() << "\t" << "Store" << "\t\t\t" << instr->store.output << "\t" << instr->store.index << "\t" <<
                              instr->store.reg;
         break;
      case Instr::Copy:
         qWarning().nospace() << "\t" << "Copy" << "\t\t\t" << instr->copy.reg << "\t" << instr->copy.src;
         break;
      case Instr::Skip:
         qWarning().nospace() << "\t" << "Skip" << "\t\t\t" << instr->skip.reg << "\t" << instr->skip.count;
         break;
      case Instr::Done:
         qWarning().nospace() << "\t" << "Done";
         break;
      case Instr::InitString:
         qWarning().nospace() << "\t" << "InitString" << "\t\t" << instr->initstring.offset << "\t" << instr->initstring.dataIdx;
         break;
      case Instr::FindGeneric:
         qWarning().nospace() << "\t" << "FindGeneric" << "\t\t" << instr->find.reg << "\t" << instr->find.name;
         break;
      case Instr::FindGenericTerminal:
         qWarning().nospace() << "\t" << "FindGenericTerminal" << "\t" << instr->find.reg << "\t" <<  instr->find.name;
         break;
      case Instr::FindProperty:
         qWarning().nospace() << "\t" << "FindProperty" << "\t\t" << instr->find.reg << "\t" << instr->find.src << "\t" <<
                              instr->find.name;
         break;
      case Instr::FindPropertyTerminal:
         qWarning().nospace() << "\t" << "FindPropertyTerminal" << "\t" << instr->find.reg << "\t" << instr->find.src << "\t" <<
                              instr->find.name;
         break;
      case Instr::CleanupGeneric:
         qWarning().nospace() << "\t" << "CleanupGeneric" << "\t\t" << instr->cleanup.reg;
         break;
      case Instr::ConvertGenericToReal:
         qWarning().nospace() << "\t" << "ConvertGenericToReal" << "\t" << instr->unaryop.output << "\t" << instr->unaryop.src;
         break;
      case Instr::ConvertGenericToBool:
         qWarning().nospace() << "\t" << "ConvertGenericToBool" << "\t" << instr->unaryop.output << "\t" << instr->unaryop.src;
         break;
      case Instr::ConvertGenericToString:
         qWarning().nospace() << "\t" << "ConvertGenericToString" << "\t" << instr->unaryop.output << "\t" << instr->unaryop.src;
         break;
      case Instr::ConvertGenericToUrl:
         qWarning().nospace() << "\t" << "ConvertGenericToUrl" << "\t" << instr->unaryop.output << "\t" << instr->unaryop.src;
         break;
      default:
         qWarning().nospace() << "\t" << "Unknown";
         break;
   }
}

void QDeclarativeCompiledBindingsPrivate::run(int instrIndex,
      QDeclarativeContextData *context, QDeclarativeDelayedError *error,
      QObject *scope, QObject *output, QDeclarativePropertyPrivate::WriteFlags storeFlags)
{
   Q_Q(QDeclarativeCompiledBindings);

   error->removeError();

   Register registers[32];

   QDeclarativeEnginePrivate *engine = QDeclarativeEnginePrivate::get(context->engine);
   Program *program = (Program *)programData;
   const Instr *instr = program->instructions();
   instr += instrIndex;
   const char *data = program->data();

#ifdef QML_THREADED_INTERPRETER
   static void *decode_instr[] = {
      FOR_EACH_QML_INSTR(QML_INSTR_ADDR)
   };

   if (!program->compiled) {
      program->compiled = true;
      const Instr *inop = program->instructions();
      for (int i = 0; i < program->instructionCount; ++i) {
         Instr *op = (Instr *) inop++;
         op->common.code = decode_instr[op->common.type];
      }
   }

   goto *instr->common.code;
#else
   // return;

#ifdef COMPILEDBINDINGS_DEBUG
   qWarning().nospace() << "Begin binding run";
#endif

   while (instr) {
      switch (instr->common.type) {

#ifdef COMPILEDBINDINGS_DEBUG
            dumpInstruction(instr);
#endif

#endif

   QML_BEGIN_INSTR(Noop)
   QML_END_INSTR(Noop)

   QML_BEGIN_INSTR(BindingId)
   QML_END_INSTR(BindingId)

   QML_BEGIN_INSTR(SubscribeId)
   subscribeId(context, instr->subscribe.index, instr->subscribe.offset);
   QML_END_INSTR(SubscribeId)

   QML_BEGIN_INSTR(Subscribe) {
      QObject *o = 0;
      const Register &object = registers[instr->subscribe.reg];
      if (!object.isUndefined()) {
         o = object.getQObject();
      }
      subscribe(o, instr->subscribe.index, instr->subscribe.offset);
   }
   QML_END_INSTR(Subscribe)

   QML_BEGIN_INSTR(FetchAndSubscribe) {
      const Register &input = registers[instr->fetchAndSubscribe.objectReg];
      Register &output = registers[instr->fetchAndSubscribe.output];

      if (input.isUndefined()) {
         throwException(instr->fetchAndSubscribe.exceptionId, error, program, context);
         return;
      }

      QObject *object = input.getQObject();
      if (!object) {
         output.setUndefined();
      } else {
         int subIdx = instr->fetchAndSubscribe.subscription;
         QDeclarativeCompiledBindingsPrivate::Subscription *sub = 0;
         if (subIdx != -1) {
            sub = (subscriptions + subIdx);
            sub->target = q;
            sub->targetMethod = methodCount + subIdx;
         }
         fastProperties()->accessor(instr->fetchAndSubscribe.function)(object, output.typeDataPtr(), sub);
      }
   }
   QML_END_INSTR(FetchAndSubscribe)

   QML_BEGIN_INSTR(LoadId)
   registers[instr->load.reg].setQObject(context->idValues[instr->load.index].data());
   QML_END_INSTR(LoadId)

   QML_BEGIN_INSTR(LoadScope)
   registers[instr->load.reg].setQObject(scope);
   QML_END_INSTR(LoadScope)

   QML_BEGIN_INSTR(LoadRoot)
   registers[instr->load.reg].setQObject(context->contextObject);
   QML_END_INSTR(LoadRoot)

   QML_BEGIN_INSTR(LoadAttached) {
      const Register &input = registers[instr->attached.reg];
      Register &output = registers[instr->attached.output];
      if (input.isUndefined()) {
         throwException(instr->attached.exceptionId, error, program, context);
         return;
      }

      QObject *object = registers[instr->attached.reg].getQObject();
      if (!object) {
         output.setUndefined();
      } else {
         QObject *attached =
            qmlAttachedPropertiesObjectById(instr->attached.id,
                                            registers[instr->attached.reg].getQObject(),
                                            true);
         Q_ASSERT(attached);
         output.setQObject(attached);
      }
   }
   QML_END_INSTR(LoadAttached)

   QML_BEGIN_INSTR(ConvertIntToReal) {
      const Register &input = registers[instr->unaryop.src];
      Register &output = registers[instr->unaryop.output];
      if (input.isUndefined()) {
         output.setUndefined();
      } else {
         output.setqreal(qreal(input.getint()));
      }
   }
   QML_END_INSTR(ConvertIntToReal)

   QML_BEGIN_INSTR(ConvertRealToInt) {
      const Register &input = registers[instr->unaryop.src];
      Register &output = registers[instr->unaryop.output];
      if (input.isUndefined()) {
         output.setUndefined();
      } else {
         output.setint(qRound(input.getqreal()));
      }
   }
   QML_END_INSTR(ConvertRealToInt)

   QML_BEGIN_INSTR(Real)
   registers[instr->real_value.reg].setqreal(instr->real_value.value);
   QML_END_INSTR(Real)

   QML_BEGIN_INSTR(Int)
   registers[instr->int_value.reg].setint(instr->int_value.value);
   QML_END_INSTR(Int)

   QML_BEGIN_INSTR(Bool)
   registers[instr->bool_value.reg].setbool(instr->bool_value.value);
   QML_END_INSTR(Bool)

   QML_BEGIN_INSTR(String) {
      Register &output = registers[instr->string_value.reg];
      new (output.getstringptr())
      QString((QChar *)(data + instr->string_value.offset), instr->string_value.length);
      output.settype(QMetaType::QString);
   }
   QML_END_INSTR(String)

   QML_BEGIN_INSTR(AddReal) {
      const Register &lhs = registers[instr->binaryop.src1];
      const Register &rhs = registers[instr->binaryop.src2];
      Register &output = registers[instr->binaryop.output];
      if (lhs.isUndefined() || rhs.isUndefined()) {
         output.setNaN();
      } else {
         output.setqreal(lhs.getqreal() + rhs.getqreal());
      }
   }
   QML_END_INSTR(AddReal)

   QML_BEGIN_INSTR(AddInt) {
      const Register &lhs = registers[instr->binaryop.src1];
      const Register &rhs = registers[instr->binaryop.src2];
      Register &output = registers[instr->binaryop.output];
      if (lhs.isUndefined() || rhs.isUndefined()) {
         output.setNaN();
      } else {
         output.setint(lhs.getint() + rhs.getint());
      }
   }
   QML_END_INSTR(AddInt)

   QML_BEGIN_INSTR(AddString) {
      const Register &lhs = registers[instr->binaryop.src1];
      const Register &rhs = registers[instr->binaryop.src2];
      Register &output = registers[instr->binaryop.output];
      if (lhs.isUndefined() && rhs.isUndefined()) {
         output.setNaN();
      } else {
         if (lhs.isUndefined())
            new (output.getstringptr())
            QString(QLatin1String("undefined") + *registers[instr->binaryop.src2].getstringptr());
         else if (rhs.isUndefined())
            new (output.getstringptr())
            QString(*registers[instr->binaryop.src1].getstringptr() + QLatin1String("undefined"));
         else
            new (output.getstringptr())
            QString(*registers[instr->binaryop.src1].getstringptr() +
                    *registers[instr->binaryop.src2].getstringptr());
         output.settype(QMetaType::QString);
      }
   }
   QML_END_INSTR(AddString)

   QML_BEGIN_INSTR(MinusReal) {
      const Register &lhs = registers[instr->binaryop.src1];
      const Register &rhs = registers[instr->binaryop.src2];
      Register &output = registers[instr->binaryop.output];
      if (lhs.isUndefined() || rhs.isUndefined()) {
         output.setNaN();
      } else {
         output.setqreal(lhs.getqreal() - rhs.getqreal());
      }
   }
   QML_END_INSTR(MinusReal)

   QML_BEGIN_INSTR(MinusInt) {
      const Register &lhs = registers[instr->binaryop.src1];
      const Register &rhs = registers[instr->binaryop.src2];
      Register &output = registers[instr->binaryop.output];
      if (lhs.isUndefined() || rhs.isUndefined()) {
         output.setNaN();
      } else {
         output.setint(lhs.getint() - rhs.getint());
      }
   }
   QML_END_INSTR(MinusInt)

   QML_BEGIN_INSTR(CompareReal) {
      const Register &lhs = registers[instr->binaryop.src1];
      const Register &rhs = registers[instr->binaryop.src2];
      Register &output = registers[instr->binaryop.output];
      if (lhs.isUndefined() || rhs.isUndefined()) {
         output.setbool(lhs.isUndefined() == rhs.isUndefined());
      } else {
         output.setbool(lhs.getqreal() == rhs.getqreal());
      }
   }
   QML_END_INSTR(CompareReal)

   QML_BEGIN_INSTR(CompareString) {
      const Register &lhs = registers[instr->binaryop.src1];
      const Register &rhs = registers[instr->binaryop.src2];
      Register &output = registers[instr->binaryop.output];
      if (lhs.isUndefined() || rhs.isUndefined()) {
         output.setbool(lhs.isUndefined() == rhs.isUndefined());
      } else {
         output.setbool(*lhs.getstringptr() == *rhs.getstringptr());
      }
   }
   QML_END_INSTR(CompareString)

   QML_BEGIN_INSTR(NotCompareReal) {
      const Register &lhs = registers[instr->binaryop.src1];
      const Register &rhs = registers[instr->binaryop.src2];
      Register &output = registers[instr->binaryop.output];
      if (lhs.isUndefined() || rhs.isUndefined()) {
         output.setbool(lhs.isUndefined() != rhs.isUndefined());
      } else {
         output.setbool(lhs.getqreal() != rhs.getqreal());
      }
   }
   QML_END_INSTR(NotCompareReal)

   QML_BEGIN_INSTR(NotCompareString) {
      const Register &lhs = registers[instr->binaryop.src1];
      const Register &rhs = registers[instr->binaryop.src2];
      Register &output = registers[instr->binaryop.output];
      if (lhs.isUndefined() || rhs.isUndefined()) {
         output.setbool(lhs.isUndefined() != rhs.isUndefined());
      } else {
         output.setbool(*lhs.getstringptr() != *rhs.getstringptr());
      }
   }
   QML_END_INSTR(NotCompareString)

   QML_BEGIN_INSTR(GreaterThanReal) {
      const Register &lhs = registers[instr->binaryop.src1];
      const Register &rhs = registers[instr->binaryop.src2];
      Register &output = registers[instr->binaryop.output];
      if (lhs.isUndefined() || rhs.isUndefined()) {
         output.setbool(false);
      } else {
         output.setbool(lhs.getqreal() > rhs.getqreal());
      }
   }
   QML_END_INSTR(GreaterThanReal)

   QML_BEGIN_INSTR(MaxReal) {
      const Register &lhs = registers[instr->binaryop.src1];
      const Register &rhs = registers[instr->binaryop.src2];
      Register &output = registers[instr->binaryop.output];
      if (lhs.isUndefined() || rhs.isUndefined()) {
         output.setNaN();
      } else {
         output.setqreal(qMax(lhs.getqreal(), rhs.getqreal()));
      }
   }
   QML_END_INSTR(MaxReal)

   QML_BEGIN_INSTR(MinReal) {
      const Register &lhs = registers[instr->binaryop.src1];
      const Register &rhs = registers[instr->binaryop.src2];
      Register &output = registers[instr->binaryop.output];
      if (lhs.isUndefined() || rhs.isUndefined()) {
         output.setNaN();
      } else {
         output.setqreal(qMin(lhs.getqreal(), rhs.getqreal()));
      }
   }
   QML_END_INSTR(MinReal)

   QML_BEGIN_INSTR(NewString) {
      Register &output = registers[instr->construct.reg];
      new (output.getstringptr()) QString;
      output.settype(QMetaType::QString);
   }
   QML_END_INSTR(NewString)

   QML_BEGIN_INSTR(NewUrl) {
      Register &output = registers[instr->construct.reg];
      new (output.geturlptr()) QUrl;
      output.settype(QMetaType::QUrl);
   }
   QML_END_INSTR(NewUrl)

   QML_BEGIN_INSTR(CleanupString)
   registers[instr->cleanup.reg].getstringptr()->~QString();
#ifdef REGISTER_CLEANUP_DEBUG
   registers[instr->cleanup.reg].setUndefined();
#endif
   QML_END_INSTR(CleanupString)

   QML_BEGIN_INSTR(CleanupUrl)
   registers[instr->cleanup.reg].geturlptr()->~QUrl();
#ifdef REGISTER_CLEANUP_DEBUG
   registers[instr->cleanup.reg].setUndefined();
#endif
   QML_END_INSTR(CleanupUrl)

   QML_BEGIN_INSTR(Fetch) {
      const Register &input = registers[instr->fetch.objectReg];
      Register &output = registers[instr->fetch.output];

      if (input.isUndefined()) {
         throwException(instr->fetch.exceptionId, error, program, context);
         return;
      }

      QObject *object = input.getQObject();
      if (!object) {
         output.setUndefined();
      } else {
         void *argv[] = { output.typeDataPtr(), 0 };
         QMetaObject::metacall(object, QMetaObject::ReadProperty, instr->fetch.index, argv);
      }
   }
   QML_END_INSTR(Fetch)

   QML_BEGIN_INSTR(Store) {
      Register &data = registers[instr->store.reg];
      if (data.isUndefined()) {
         throwException(instr->store.exceptionId, error, program, context,
                        QLatin1String("Unable to assign undefined value"));
         return;
      }

      int status = -1;
      void *argv[] = { data.typeDataPtr(), 0, &status, &storeFlags };
      QMetaObject::metacall(output, QMetaObject::WriteProperty,
                            instr->store.index, argv);
   }
   QML_END_INSTR(Store)

   QML_BEGIN_INSTR(Copy)
   registers[instr->copy.reg] = registers[instr->copy.src];
   QML_END_INSTR(Copy)

   QML_BEGIN_INSTR(Skip)
   if (instr->skip.reg == -1 || !registers[instr->skip.reg].getbool()) {
      instr += instr->skip.count;
   }
   QML_END_INSTR(Skip)

   QML_BEGIN_INSTR(Done)
   return;
   QML_END_INSTR(Done)

   QML_BEGIN_INSTR(InitString)
   if (!identifiers[instr->initstring.offset].identifier) {
      quint32 len = *(quint32 *)(data + instr->initstring.dataIdx);
      QChar *strdata = (QChar *)(data + instr->initstring.dataIdx + sizeof(quint32));

      QString str = QString::fromRawData(strdata, len);

      identifiers[instr->initstring.offset] = engine->objectClass->createPersistentIdentifier(str);
   }
   QML_END_INSTR(InitString)

   QML_BEGIN_INSTR(FindGenericTerminal)
   // We start the search in the parent context, as we know that the
   // name is not present in the current context or it would have been
   // found during the static compile
   findgeneric(registers + instr->find.reg, instr->find.subscribeIndex,
               context->parent,
               identifiers[instr->find.name].identifier,
               instr->common.type == Instr::FindGenericTerminal);
   QML_END_INSTR(FindGenericTerminal)

   QML_BEGIN_INSTR(FindGeneric)
   // We start the search in the parent context, as we know that the
   // name is not present in the current context or it would have been
   // found during the static compile
   findgeneric(registers + instr->find.reg, instr->find.subscribeIndex,
               context->parent,
               identifiers[instr->find.name].identifier,
               instr->common.type == Instr::FindGenericTerminal);
   QML_END_INSTR(FindGeneric)

   QML_BEGIN_INSTR(FindPropertyTerminal) {
      const Register &object = registers[instr->find.src];
      if (object.isUndefined()) {
         throwException(instr->find.exceptionId, error, program, context);
         return;
      }

      findproperty(object.getQObject(), registers + instr->find.reg,
                   QDeclarativeEnginePrivate::get(context->engine),
                   instr->find.subscribeIndex, identifiers[instr->find.name].identifier,
                   instr->common.type == Instr::FindPropertyTerminal);
   }
   QML_END_INSTR(FindPropertyTerminal)

   QML_BEGIN_INSTR(FindProperty) {
      const Register &object = registers[instr->find.src];
      if (object.isUndefined()) {
         throwException(instr->find.exceptionId, error, program, context);
         return;
      }

      findproperty(object.getQObject(), registers + instr->find.reg,
                   QDeclarativeEnginePrivate::get(context->engine),
                   instr->find.subscribeIndex, identifiers[instr->find.name].identifier,
                   instr->common.type == Instr::FindPropertyTerminal);
   }
   QML_END_INSTR(FindProperty)

   QML_BEGIN_INSTR(CleanupGeneric) {
      int type = registers[instr->cleanup.reg].gettype();
      if (type == qMetaTypeId<QVariant>()) {
         registers[instr->cleanup.reg].getvariantptr()->~QVariant();
#ifdef REGISTER_CLEANUP_DEBUG
         registers[instr->cleanup.reg].setUndefined();
#endif
      } else if (type == QMetaType::QString) {
         registers[instr->cleanup.reg].getstringptr()->~QString();
#ifdef REGISTER_CLEANUP_DEBUG
         registers[instr->cleanup.reg].setUndefined();
#endif
      } else if (type == QMetaType::QUrl) {
         registers[instr->cleanup.reg].geturlptr()->~QUrl();
#ifdef REGISTER_CLEANUP_DEBUG
         registers[instr->cleanup.reg].setUndefined();
#endif
      }
   }
   QML_END_INSTR(CleanupGeneric)

   QML_BEGIN_INSTR(ConvertGenericToReal) {
      Register &output = registers[instr->unaryop.output];
      Register &input = registers[instr->unaryop.src];
      bool ok = true;
      output.setqreal(toReal(&input, input.gettype(), &ok));
      if (!ok) {
         output.setUndefined();
      }
   }
   QML_END_INSTR(ConvertGenericToReal)

   QML_BEGIN_INSTR(ConvertGenericToBool) {
      Register &output = registers[instr->unaryop.output];
      Register &input = registers[instr->unaryop.src];
      bool ok = true;
      output.setbool(toBool(&input, input.gettype(), &ok));
      if (!ok) {
         output.setUndefined();
      }
   }
   QML_END_INSTR(ConvertGenericToBool)

   QML_BEGIN_INSTR(ConvertGenericToString) {
      Register &output = registers[instr->unaryop.output];
      Register &input = registers[instr->unaryop.src];
      bool ok = true;
      QString str = toString(&input, input.gettype(), &ok);
      if (ok) {
         new (output.getstringptr()) QString(str);
         output.settype(QMetaType::QString);
      } else {
         output.setUndefined();
      }
   }
   QML_END_INSTR(ConvertGenericToString)

   QML_BEGIN_INSTR(ConvertGenericToUrl) {
      Register &output = registers[instr->unaryop.output];
      Register &input = registers[instr->unaryop.src];
      bool ok = true;
      QUrl url = toUrl(&input, input.gettype(), context, &ok);
      if (ok) {
         new (output.geturlptr()) QUrl(url);
         output.settype(QMetaType::QUrl);
      } else {
         output.setUndefined();
      }
   }
   QML_END_INSTR(ConvertGenericToUrl)

#ifdef QML_THREADED_INTERPRETER
   // nothing to do
#else
default:
   qFatal("EEK");
   break;
} // switch

++instr;
} // while
#endif
}

void QDeclarativeBindingCompiler::dump(const QByteArray &programData)
{
   const Program *program = (const Program *)programData.constData();

   qWarning() << "Program.bindings:" << program->bindings;
   qWarning() << "Program.dataLength:" << program->dataLength;
   qWarning() << "Program.subscriptions:" << program->subscriptions;
   qWarning() << "Program.indentifiers:" << program->identifiers;

   int count = program->instructionCount;
   const Instr *instr = program->instructions();

   while (count--) {

      dumpInstruction(instr);
      ++instr;
   }
}

/*!
Clear the state associated with attempting to compile a specific binding.
This does not clear the global "committed binding" states.
*/
void QDeclarativeBindingCompilerPrivate::resetInstanceState()
{
   registers = 0;
   registerCleanups.clear();
   data = committed.data;
   exceptions = committed.exceptions;
   usedSubscriptionIds.clear();
   subscriptionSet.clear();
   subscriptionIds = committed.subscriptionIds;
   registeredStrings = committed.registeredStrings;
   bytecode.clear();
}

/*!
Mark the last compile as successful, and add it to the "committed data"
section.

Returns the index for the committed binding.
*/
int QDeclarativeBindingCompilerPrivate::commitCompile()
{
   int rv = committed.count();
   committed.offsets << committed.bytecode.count();
   committed.dependencies << usedSubscriptionIds;
   committed.bytecode << bytecode;
   committed.data = data;
   committed.exceptions = exceptions;
   committed.subscriptionIds = subscriptionIds;
   committed.registeredStrings = registeredStrings;
   return rv;
}

bool QDeclarativeBindingCompilerPrivate::compile(QDeclarativeJS::AST::Node *node)
{
   resetInstanceState();

   if (destination->type == -1) {
      return false;
   }

   if (bindingsDump()) {
      QDeclarativeJS::AST::ExpressionNode *n = node->expressionCast();
      if (n) {
         Instr id;
         id.common.type = Instr::BindingId;
         id.id.column = n->firstSourceLocation().startColumn;
         id.id.line = n->firstSourceLocation().startLine;
         bytecode << id;
      }
   }

   Result type;

   if (!parseExpression(node, type)) {
      return false;
   }

   if (subscriptionSet.count() > 0xFFFF ||
         registeredStrings.count() > 0xFFFF) {
      return false;
   }

   if (type.unknownType) {
      if (!qmlExperimental()) {
         return false;
      }

      if (destination->type != QMetaType::QReal &&
            destination->type != QVariant::String &&
            destination->type != QMetaType::Bool &&
            destination->type != QVariant::Url) {
         return false;
      }

      int convertReg = acquireReg();
      if (convertReg == -1) {
         return false;
      }

      if (destination->type == QMetaType::QReal) {
         Instr convert;
         convert.common.type = Instr::ConvertGenericToReal;
         convert.unaryop.output = convertReg;
         convert.unaryop.src = type.reg;
         bytecode << convert;
      } else if (destination->type == QVariant::String) {
         Instr convert;
         convert.common.type = Instr::ConvertGenericToString;
         convert.unaryop.output = convertReg;
         convert.unaryop.src = type.reg;
         bytecode << convert;
      } else if (destination->type == QMetaType::Bool) {
         Instr convert;
         convert.common.type = Instr::ConvertGenericToBool;
         convert.unaryop.output = convertReg;
         convert.unaryop.src = type.reg;
         bytecode << convert;
      } else if (destination->type == QVariant::Url) {
         Instr convert;
         convert.common.type = Instr::ConvertGenericToUrl;
         convert.unaryop.output = convertReg;
         convert.unaryop.src = type.reg;
         bytecode << convert;
      }

      Instr cleanup;
      cleanup.common.type = Instr::CleanupGeneric;
      cleanup.cleanup.reg = type.reg;
      bytecode << cleanup;

      Instr instr;
      instr.common.type = Instr::Store;
      instr.store.output = 0;
      instr.store.index = destination->index;
      instr.store.reg = convertReg;
      instr.store.exceptionId = exceptionId(node->expressionCast());
      bytecode << instr;

      if (destination->type == QVariant::String) {
         Instr cleanup;
         cleanup.common.type = Instr::CleanupString;
         cleanup.cleanup.reg = convertReg;
         bytecode << cleanup;
      } else if (destination->type == QVariant::Url) {
         Instr cleanup;
         cleanup.common.type = Instr::CleanupUrl;
         cleanup.cleanup.reg = convertReg;
         bytecode << cleanup;
      }

      releaseReg(convertReg);

      Instr done;
      done.common.type = Instr::Done;
      bytecode << done;

   } else {
      // Can we store the final value?
      if (type.type == QVariant::Int &&
            destination->type == QMetaType::QReal) {
         Instr instr;
         instr.common.type = Instr::ConvertIntToReal;
         instr.unaryop.output = type.reg;
         instr.unaryop.src = type.reg;
         bytecode << instr;
         type.type = QMetaType::QReal;
      } else if (type.type == QMetaType::QReal &&
                 destination->type == QVariant::Int) {
         Instr instr;
         instr.common.type = Instr::ConvertRealToInt;
         instr.unaryop.output = type.reg;
         instr.unaryop.src = type.reg;
         bytecode << instr;
         type.type = QVariant::Int;
      } else if (type.type == destination->type) {
      } else {
         const QMetaObject *from = type.metaObject;
         const QMetaObject *to = engine->rawMetaObjectForType(destination->type);

         if (QDeclarativePropertyPrivate::canConvert(from, to)) {
            type.type = destination->type;
         }
      }

      if (type.type == destination->type) {
         Instr instr;
         instr.common.type = Instr::Store;
         instr.store.output = 0;
         instr.store.index = destination->index;
         instr.store.reg = type.reg;
         instr.store.exceptionId = exceptionId(node->expressionCast());
         bytecode << instr;

         releaseReg(type.reg);

         Instr done;
         done.common.type = Instr::Done;
         bytecode << done;
      } else {
         return false;
      }
   }

   return true;
}

bool QDeclarativeBindingCompilerPrivate::parseExpression(QDeclarativeJS::AST::Node *node, Result &type)
{
   while (node->kind == AST::Node::Kind_NestedExpression) {
      node = static_cast<AST::NestedExpression *>(node)->expression;
   }

   if (tryArith(node)) {
      if (!parseArith(node, type)) {
         return false;
      }
   } else if (tryLogic(node)) {
      if (!parseLogic(node, type)) {
         return false;
      }
   } else if (tryConditional(node)) {
      if (!parseConditional(node, type)) {
         return false;
      }
   } else if (tryName(node)) {
      if (!parseName(node, type)) {
         return false;
      }
   } else if (tryConstant(node)) {
      if (!parseConstant(node, type)) {
         return false;
      }
   } else if (tryMethod(node)) {
      if (!parseMethod(node, type)) {
         return false;
      }
   } else {
      return false;
   }
   return true;
}

bool QDeclarativeBindingCompilerPrivate::tryName(QDeclarativeJS::AST::Node *node)
{
   return node->kind == AST::Node::Kind_IdentifierExpression ||
          node->kind == AST::Node::Kind_FieldMemberExpression;
}

bool QDeclarativeBindingCompilerPrivate::parseName(AST::Node *node, Result &type)
{
   QStringList nameParts;
   QList<AST::ExpressionNode *> nameNodes;
   if (!buildName(nameParts, node, &nameNodes)) {
      return false;
   }

   int reg = acquireReg();
   if (reg == -1) {
      return false;
   }
   type.reg = reg;

   QDeclarativeParser::Object *absType = 0;

   QStringList subscribeName;

   bool wasAttachedObject = false;

   for (int ii = 0; ii < nameParts.count(); ++ii) {
      const QString &name = nameParts.at(ii);

      // We don't handle signal properties or attached properties
      if (name.length() > 2 && name.startsWith(QLatin1String("on")) &&
            name.at(2).isUpper()) {
         return false;
      }

      QDeclarativeType *attachType = 0;
      if (name.at(0).isUpper()) {
         // Could be an attached property
         if (ii == nameParts.count() - 1) {
            return false;
         }
         if (nameParts.at(ii + 1).at(0).isUpper()) {
            return false;
         }

         QDeclarativeImportedNamespace *ns = 0;
         if (!imports.resolveType(name.toUtf8(), &attachType, 0, 0, 0, &ns)) {
            return false;
         }
         if (ns || !attachType || !attachType->attachedPropertiesType()) {
            return false;
         }

         wasAttachedObject = true;
      }

      if (ii == 0) {

         if (attachType) {
            Instr instr;
            instr.common.type = Instr::LoadScope;
            instr.load.index = 0;
            instr.load.reg = reg;
            bytecode << instr;

            Instr attach;
            attach.common.type = Instr::LoadAttached;
            attach.attached.output = reg;
            attach.attached.reg = reg;
            attach.attached.id = attachType->attachedPropertiesId();
            attach.attached.exceptionId = exceptionId(nameNodes.at(ii));
            bytecode << attach;

            subscribeName << contextName();
            subscribeName << QLatin1String("$$$ATTACH_") + name;

            absType = 0;
            type.metaObject = attachType->attachedPropertiesType();

            continue;
         } else if (ids.contains(name)) {
            QDeclarativeParser::Object *idObject = ids.value(name);
            absType = idObject;
            type.metaObject = absType->metaObject();

            // We check if the id object is the root or
            // scope object to avoid a subscription
            if (idObject == component) {
               Instr instr;
               instr.common.type = Instr::LoadRoot;
               instr.load.index = 0;
               instr.load.reg = reg;
               bytecode << instr;
            } else if (idObject == context) {
               Instr instr;
               instr.common.type = Instr::LoadScope;
               instr.load.index = 0;
               instr.load.reg = reg;
               bytecode << instr;
            } else {
               Instr instr;
               instr.common.type = Instr::LoadId;
               instr.load.index = idObject->idIndex;
               instr.load.reg = reg;
               bytecode << instr;

               subscribeName << QLatin1String("$$$ID_") + name;

               if (subscription(subscribeName, &type)) {
                  Instr sub;
                  sub.common.type = Instr::SubscribeId;
                  sub.subscribe.offset = subscriptionIndex(subscribeName);
                  sub.subscribe.reg = reg;
                  sub.subscribe.index = instr.load.index;
                  bytecode << sub;
               }
            }

         } else {

            QByteArray utf8Name = name.toUtf8();
            const char *cname = utf8Name.constData();

            int d0Idx = (context == component) ? -1 : context->metaObject()->indexOfProperty(cname);
            int d1Idx = -1;
            if (d0Idx == -1) {
               d1Idx = component->metaObject()->indexOfProperty(cname);
            }

            if (d0Idx != -1) {
               Instr instr;
               instr.common.type = Instr::LoadScope;
               instr.load.index = 0;
               instr.load.reg = reg;
               bytecode << instr;

               subscribeName << contextName();
               subscribeName << name;

               if (!fetch(type, context->metaObject(), reg, d0Idx, subscribeName, nameNodes.at(ii))) {
                  return false;
               }
            } else if (d1Idx != -1) {
               Instr instr;
               instr.common.type = Instr::LoadRoot;
               instr.load.index = 0;
               instr.load.reg = reg;
               bytecode << instr;

               subscribeName << QLatin1String("$$$ROOT");
               subscribeName << name;

               if (!fetch(type, component->metaObject(), reg, d1Idx, subscribeName, nameNodes.at(ii))) {
                  return false;
               }
            } else if (qmlExperimental()) {
               Instr find;
               if (nameParts.count() == 1) {
                  find.common.type = Instr::FindGenericTerminal;
               } else {
                  find.common.type = Instr::FindGeneric;
               }

               find.find.reg = reg;
               find.find.src = -1;
               find.find.name = registerString(name);
               find.find.exceptionId = exceptionId(nameNodes.at(ii));

               subscribeName << QString(QLatin1String("$$$Generic_") + name);
               if (subscription(subscribeName, &type)) {
                  find.find.subscribeIndex = subscriptionIndex(subscribeName);
               } else {
                  find.find.subscribeIndex = -1;
               }

               bytecode << find;
               type.unknownType = true;
            }

            if (!type.unknownType && type.type == -1) {
               return false;   // Couldn't fetch that type
            }
         }

      } else {

         if (attachType) {
            Instr attach;
            attach.common.type = Instr::LoadAttached;
            attach.attached.output = reg;
            attach.attached.reg = reg;
            attach.attached.id = attachType->attachedPropertiesId();
            bytecode << attach;

            absType = 0;
            type.metaObject = attachType->attachedPropertiesType();

            subscribeName << QLatin1String("$$$ATTACH_") + name;
            continue;
         }

         const QMetaObject *mo = 0;
         if (absType) {
            mo = absType->metaObject();
         } else if (type.metaObject) {
            mo = type.metaObject;
         }

         QByteArray utf8Name = name.toUtf8();
         const char *cname = utf8Name.constData();
         int idx = mo ? mo->indexOfProperty(cname) : -1;
         if (absType && idx == -1) {
            return false;
         }

         subscribeName << name;

         if (absType || (wasAttachedObject && idx != -1) || (mo && mo->property(idx).isFinal())) {
            absType = 0;
            if (!fetch(type, mo, reg, idx, subscribeName, nameNodes.at(ii))) {
               return false;
            }
         } else {

            Instr prop;
            if (ii == nameParts.count() - 1 ) {
               prop.common.type = Instr::FindPropertyTerminal;
            } else {
               prop.common.type = Instr::FindProperty;
            }

            prop.find.reg = reg;
            prop.find.src = reg;
            prop.find.name = registerString(name);
            prop.find.exceptionId = exceptionId(nameNodes.at(ii));

            if (subscription(subscribeName, &type)) {
               prop.find.subscribeIndex = subscriptionIndex(subscribeName);
            } else {
               prop.find.subscribeIndex = -1;
            }

            type.unknownType = true;
            type.metaObject = 0;
            type.type = -1;
            type.reg = reg;
            bytecode << prop;
         }
      }

      wasAttachedObject = false;
   }

   return true;
}

bool QDeclarativeBindingCompilerPrivate::tryArith(QDeclarativeJS::AST::Node *node)
{
   if (node->kind != AST::Node::Kind_BinaryExpression) {
      return false;
   }

   AST::BinaryExpression *expression = static_cast<AST::BinaryExpression *>(node);
   if (expression->op == QSOperator::Add ||
         expression->op == QSOperator::Sub) {
      return true;
   } else {
      return false;
   }
}

bool QDeclarativeBindingCompilerPrivate::parseArith(QDeclarativeJS::AST::Node *node, Result &type)
{
   AST::BinaryExpression *expression = static_cast<AST::BinaryExpression *>(node);

   type.reg = acquireReg();
   if (type.reg == -1) {
      return false;
   }

   Result lhs;
   Result rhs;

   if (!parseExpression(expression->left, lhs)) {
      return false;
   }
   if (!parseExpression(expression->right, rhs)) {
      return false;
   }

   if ((lhs.type == QVariant::Int || lhs.type == QMetaType::QReal) &&
         (rhs.type == QVariant::Int || rhs.type == QMetaType::QReal)) {
      return numberArith(type, lhs, rhs, (QSOperator::Op)expression->op);
   } else if (expression->op == QSOperator::Sub) {
      return numberArith(type, lhs, rhs, (QSOperator::Op)expression->op);
   } else if ((lhs.type == QMetaType::QString || lhs.unknownType) &&
              (rhs.type == QMetaType::QString || rhs.unknownType) &&
              (lhs.type == QMetaType::QString || rhs.type == QMetaType::QString)) {
      return stringArith(type, lhs, rhs, (QSOperator::Op)expression->op);
   } else {
      return false;
   }
}

bool QDeclarativeBindingCompilerPrivate::numberArith(Result &type, const Result &lhs, const Result &rhs,
      QSOperator::Op op)
{
   bool nativeReal = rhs.type == QMetaType::QReal ||
                     lhs.type == QMetaType::QReal ||
                     lhs.unknownType ||
                     rhs.unknownType;

   if (nativeReal && lhs.type == QMetaType::Int) {
      Instr convert;
      convert.common.type = Instr::ConvertIntToReal;
      convert.unaryop.output = lhs.reg;
      convert.unaryop.src = lhs.reg;
      bytecode << convert;
   }

   if (nativeReal && rhs.type == QMetaType::Int) {
      Instr convert;
      convert.common.type = Instr::ConvertIntToReal;
      convert.unaryop.output = rhs.reg;
      convert.unaryop.src = rhs.reg;
      bytecode << convert;
   }

   int lhsTmp = -1;
   int rhsTmp = -1;

   if (lhs.unknownType) {
      if (!qmlExperimental()) {
         return false;
      }

      lhsTmp = acquireReg();
      if (lhsTmp == -1) {
         return false;
      }

      Instr conv;
      conv.common.type = Instr::ConvertGenericToReal;
      conv.unaryop.output = lhsTmp;
      conv.unaryop.src = lhs.reg;
      bytecode << conv;
   }

   if (rhs.unknownType) {
      if (!qmlExperimental()) {
         return false;
      }

      rhsTmp = acquireReg();
      if (rhsTmp == -1) {
         return false;
      }

      Instr conv;
      conv.common.type = Instr::ConvertGenericToReal;
      conv.unaryop.output = rhsTmp;
      conv.unaryop.src = rhs.reg;
      bytecode << conv;
   }

   Instr arith;
   if (op == QSOperator::Add) {
      arith.common.type = nativeReal ? Instr::AddReal : Instr::AddInt;
   } else if (op == QSOperator::Sub) {
      arith.common.type = nativeReal ? Instr::MinusReal : Instr::MinusInt;
   } else {
      qFatal("Unsupported arithmetic operator");
   }

   arith.binaryop.output = type.reg;
   arith.binaryop.src1 = (lhsTmp == -1) ? lhs.reg : lhsTmp;
   arith.binaryop.src2 = (rhsTmp == -1) ? rhs.reg : rhsTmp;
   bytecode << arith;

   type.metaObject = 0;
   type.type = nativeReal ? QMetaType::QReal : QMetaType::Int;
   type.subscriptionSet.unite(lhs.subscriptionSet);
   type.subscriptionSet.unite(rhs.subscriptionSet);

   if (lhsTmp != -1) {
      releaseReg(lhsTmp);
   }
   if (rhsTmp != -1) {
      releaseReg(rhsTmp);
   }
   releaseReg(lhs.reg);
   releaseReg(rhs.reg);

   return true;
}

bool QDeclarativeBindingCompilerPrivate::stringArith(Result &type, const Result &lhs, const Result &rhs,
      QSOperator::Op op)
{
   if (op != QSOperator::Add) {
      return false;
   }

   int lhsTmp = -1;
   int rhsTmp = -1;

   if (lhs.unknownType) {
      if (!qmlExperimental()) {
         return false;
      }

      lhsTmp = acquireReg(Instr::CleanupString);
      if (lhsTmp == -1) {
         return false;
      }

      Instr convert;
      convert.common.type = Instr::ConvertGenericToString;
      convert.unaryop.output = lhsTmp;
      convert.unaryop.src = lhs.reg;
      bytecode << convert;
   }

   if (rhs.unknownType) {
      if (!qmlExperimental()) {
         return false;
      }

      rhsTmp = acquireReg(Instr::CleanupString);
      if (rhsTmp == -1) {
         return false;
      }

      Instr convert;
      convert.common.type = Instr::ConvertGenericToString;
      convert.unaryop.output = rhsTmp;
      convert.unaryop.src = rhs.reg;
      bytecode << convert;
   }

   type.reg = acquireReg(Instr::CleanupString);
   if (type.reg == -1) {
      return false;
   }

   type.type = QMetaType::QString;

   Instr add;
   add.common.type = Instr::AddString;
   add.binaryop.output = type.reg;
   add.binaryop.src1 = (lhsTmp == -1) ? lhs.reg : lhsTmp;
   add.binaryop.src2 = (rhsTmp == -1) ? rhs.reg : rhsTmp;
   bytecode << add;

   if (lhsTmp != -1) {
      releaseReg(lhsTmp);
   }
   if (rhsTmp != -1) {
      releaseReg(rhsTmp);
   }
   releaseReg(lhs.reg);
   releaseReg(rhs.reg);

   return true;
}

bool QDeclarativeBindingCompilerPrivate::tryLogic(QDeclarativeJS::AST::Node *node)
{
   if (node->kind != AST::Node::Kind_BinaryExpression) {
      return false;
   }

   AST::BinaryExpression *expression = static_cast<AST::BinaryExpression *>(node);
   if (expression->op == QSOperator::Gt ||
         expression->op == QSOperator::Equal ||
         expression->op == QSOperator::NotEqual) {
      return true;
   } else {
      return false;
   }
}

bool QDeclarativeBindingCompilerPrivate::parseLogic(QDeclarativeJS::AST::Node *node, Result &type)
{
   AST::BinaryExpression *expression = static_cast<AST::BinaryExpression *>(node);

   Result lhs;
   Result rhs;

   if (!parseExpression(expression->left, lhs)) {
      return false;
   }
   if (!parseExpression(expression->right, rhs)) {
      return false;
   }

   type.reg = acquireReg();
   if (type.reg == -1) {
      return false;
   }

   type.metaObject = 0;
   type.type = QVariant::Bool;

   if (lhs.type == QMetaType::QReal && rhs.type == QMetaType::QReal) {

      Instr op;
      if (expression->op == QSOperator::Gt) {
         op.common.type = Instr::GreaterThanReal;
      } else if (expression->op == QSOperator::Equal) {
         op.common.type = Instr::CompareReal;
      } else if (expression->op == QSOperator::NotEqual) {
         op.common.type = Instr::NotCompareReal;
      } else {
         return false;
      }
      op.binaryop.output = type.reg;
      op.binaryop.src1 = lhs.reg;
      op.binaryop.src2 = rhs.reg;
      bytecode << op;


   } else if (lhs.type == QMetaType::QString && rhs.type == QMetaType::QString) {

      Instr op;
      if (expression->op == QSOperator::Equal) {
         op.common.type = Instr::CompareString;
      } else if (expression->op == QSOperator::NotEqual) {
         op.common.type = Instr::NotCompareString;
      } else {
         return false;
      }
      op.binaryop.output = type.reg;
      op.binaryop.src1 = lhs.reg;
      op.binaryop.src2 = rhs.reg;
      bytecode << op;

   } else {
      return false;
   }

   releaseReg(lhs.reg);
   releaseReg(rhs.reg);

   return true;
}

bool QDeclarativeBindingCompilerPrivate::tryConditional(QDeclarativeJS::AST::Node *node)
{
   return (node->kind == AST::Node::Kind_ConditionalExpression);
}

bool QDeclarativeBindingCompilerPrivate::parseConditional(QDeclarativeJS::AST::Node *node, Result &type)
{
   AST::ConditionalExpression *expression = static_cast<AST::ConditionalExpression *>(node);

   AST::Node *test = expression->expression;
   if (test->kind == AST::Node::Kind_NestedExpression) {
      test = static_cast<AST::NestedExpression *>(test)->expression;
   }

   Result etype;
   if (!parseExpression(test, etype)) {
      return false;
   }

   if (etype.type != QVariant::Bool) {
      return false;
   }

   Instr skip;
   skip.common.type = Instr::Skip;
   skip.skip.reg = etype.reg;
   skip.skip.count = 0;
   int skipIdx = bytecode.count();
   bytecode << skip;

   // Release to allow reuse of reg
   releaseReg(etype.reg);

   QSet<QString> preSubSet = subscriptionSet;

   // int preConditionalSubscriptions = subscriptionSet.count();

   Result ok;
   if (!parseExpression(expression->ok, ok)) {
      return false;
   }
   if (ok.unknownType) {
      return false;
   }

   int skipIdx2 = bytecode.count();
   skip.skip.reg = -1;
   bytecode << skip;

   // Release to allow reuse of reg in else path
   releaseReg(ok.reg);
   bytecode[skipIdx].skip.count = bytecode.count() - skipIdx - 1;

   subscriptionSet = preSubSet;

   Result ko;
   if (!parseExpression(expression->ko, ko)) {
      return false;
   }
   if (ko.unknownType) {
      return false;
   }

   // Do not release reg here, so that its ownership passes to the caller
   bytecode[skipIdx2].skip.count = bytecode.count() - skipIdx2 - 1;

   if (ok != ko) {
      return false;   // Must be same type and in same register
   }

   subscriptionSet = preSubSet;

   if (!subscriptionNeutral(subscriptionSet, ok.subscriptionSet, ko.subscriptionSet)) {
      return false;   // Conditionals cannot introduce new subscriptions
   }

   type = ok;

   return true;
}

bool QDeclarativeBindingCompilerPrivate::tryConstant(QDeclarativeJS::AST::Node *node)
{
   return node->kind == AST::Node::Kind_TrueLiteral ||
          node->kind == AST::Node::Kind_FalseLiteral ||
          node->kind == AST::Node::Kind_NumericLiteral ||
          node->kind == AST::Node::Kind_StringLiteral;
}

bool QDeclarativeBindingCompilerPrivate::parseConstant(QDeclarativeJS::AST::Node *node, Result &type)
{
   type.metaObject = 0;
   type.type = -1;
   type.reg = acquireReg();
   if (type.reg == -1) {
      return false;
   }

   if (node->kind == AST::Node::Kind_TrueLiteral) {
      type.type = QVariant::Bool;
      Instr instr;
      instr.common.type = Instr::Bool;
      instr.bool_value.reg = type.reg;
      instr.bool_value.value = true;
      bytecode << instr;
      return true;
   } else if (node->kind == AST::Node::Kind_FalseLiteral) {
      type.type = QVariant::Bool;
      Instr instr;
      instr.common.type = Instr::Bool;
      instr.bool_value.reg = type.reg;
      instr.bool_value.value = false;
      bytecode << instr;
      return true;
   } else if (node->kind == AST::Node::Kind_NumericLiteral) {
      qreal value = qreal(static_cast<AST::NumericLiteral *>(node)->value);

      if (qreal(float(value)) != value) {
         return false;
      }

      type.type = QMetaType::QReal;
      Instr instr;
      instr.common.type = Instr::Real;
      instr.real_value.reg = type.reg;
      instr.real_value.value = float(value);
      bytecode << instr;
      return true;
   } else if (node->kind == AST::Node::Kind_StringLiteral) {
      QString str = static_cast<AST::StringLiteral *>(node)->value->asString();
      type.type = QMetaType::QString;
      type.reg = registerLiteralString(str);
      return true;
   } else {
      return false;
   }
}

bool QDeclarativeBindingCompilerPrivate::tryMethod(QDeclarativeJS::AST::Node *node)
{
   return node->kind == AST::Node::Kind_CallExpression;
}

bool QDeclarativeBindingCompilerPrivate::parseMethod(QDeclarativeJS::AST::Node *node, Result &result)
{
   AST::CallExpression *expr = static_cast<AST::CallExpression *>(node);

   QStringList name;
   if (!buildName(name, expr->base)) {
      return false;
   }

   if (name.count() != 2 || name.at(0) != QLatin1String("Math")) {
      return false;
   }

   QString method = name.at(1);

   AST::ArgumentList *args = expr->arguments;
   if (!args) {
      return false;
   }
   AST::ExpressionNode *arg0 = args->expression;
   args = args->next;
   if (!args) {
      return false;
   }
   AST::ExpressionNode *arg1 = args->expression;
   if (args->next != 0) {
      return false;
   }
   if (!arg0 || !arg1) {
      return false;
   }

   Result r0;
   if (!parseExpression(arg0, r0)) {
      return false;
   }
   Result r1;
   if (!parseExpression(arg1, r1)) {
      return false;
   }

   if (r0.type != QMetaType::QReal || r1.type != QMetaType::QReal) {
      return false;
   }

   Instr op;
   if (method == QLatin1String("max")) {
      op.common.type = Instr::MaxReal;
   } else if (method == QLatin1String("min")) {
      op.common.type = Instr::MinReal;
   } else {
      return false;
   }
   // We release early to reuse registers
   releaseReg(r0.reg);
   releaseReg(r1.reg);

   op.binaryop.output = acquireReg();
   if (op.binaryop.output == -1) {
      return false;
   }

   op.binaryop.src1 = r0.reg;
   op.binaryop.src2 = r1.reg;
   bytecode << op;

   result.type = QMetaType::QReal;
   result.reg = op.binaryop.output;

   return true;
}

bool QDeclarativeBindingCompilerPrivate::buildName(QStringList &name,
      QDeclarativeJS::AST::Node *node,
      QList<QDeclarativeJS::AST::ExpressionNode *> *nodes)
{
   if (node->kind == AST::Node::Kind_IdentifierExpression) {
      name << static_cast<AST::IdentifierExpression *>(node)->name->asString();
      if (nodes) {
         *nodes << static_cast<AST::IdentifierExpression *>(node);
      }
   } else if (node->kind == AST::Node::Kind_FieldMemberExpression) {
      AST::FieldMemberExpression *expr =
         static_cast<AST::FieldMemberExpression *>(node);

      if (!buildName(name, expr->base, nodes)) {
         return false;
      }

      name << expr->name->asString();
      if (nodes) {
         *nodes << expr;
      }
   } else {
      return false;
   }

   return true;
}

bool QDeclarativeBindingCompilerPrivate::fetch(Result &rv, const QMetaObject *mo, int reg,
      int idx, const QStringList &subName,
      QDeclarativeJS::AST::ExpressionNode *node)
{
   QMetaProperty prop = mo->property(idx);
   rv.metaObject = 0;
   rv.type = 0;

   //XXX binding optimizer doesn't handle properties with a revision
   if (prop.revision() > 0) {
      return false;
   }

   int fastFetchIndex = fastProperties()->accessorIndexForProperty(mo, idx);

   Instr fetch;

   if (!qmlDisableFastProperties() && fastFetchIndex != -1) {
      fetch.common.type = Instr::FetchAndSubscribe;
      fetch.fetchAndSubscribe.objectReg = reg;
      fetch.fetchAndSubscribe.output = reg;
      fetch.fetchAndSubscribe.function = fastFetchIndex;
      fetch.fetchAndSubscribe.subscription = subscriptionIndex(subName);
      fetch.fetchAndSubscribe.exceptionId = exceptionId(node);
   } else {
      if (subscription(subName, &rv) && prop.hasNotifySignal() && prop.notifySignalIndex() != -1) {
         Instr sub;
         sub.common.type = Instr::Subscribe;
         sub.subscribe.offset = subscriptionIndex(subName);
         sub.subscribe.reg = reg;
         sub.subscribe.index = prop.notifySignalIndex();
         bytecode << sub;
      }

      fetch.common.type = Instr::Fetch;
      fetch.fetch.objectReg = reg;
      fetch.fetch.index = idx;
      fetch.fetch.output = reg;
      fetch.fetch.exceptionId = exceptionId(node);
   }

   rv.type = prop.userType();
   rv.metaObject = engine->metaObjectForType(rv.type);
   rv.reg = reg;

   if (rv.type == QMetaType::QString) {
      int tmp = acquireReg();
      if (tmp == -1) {
         return false;
      }
      Instr copy;
      copy.common.type = Instr::Copy;
      copy.copy.reg = tmp;
      copy.copy.src = reg;
      bytecode << copy;
      releaseReg(tmp);
      fetch.fetch.objectReg = tmp;

      Instr setup;
      setup.common.type = Instr::NewString;
      setup.construct.reg = reg;
      bytecode << setup;
      registerCleanup(reg, Instr::CleanupString);
   }

   bytecode << fetch;

   if (!rv.metaObject &&
         rv.type != QMetaType::QReal &&
         rv.type != QMetaType::Int &&
         rv.type != QMetaType::Bool &&
         rv.type != qMetaTypeId<QDeclarativeAnchorLine>() &&
         rv.type != QMetaType::QString) {
      rv.metaObject = 0;
      rv.type = 0;
      return false; // Unsupported type (string not supported yet);
   }

   return true;
}

void QDeclarativeBindingCompilerPrivate::registerCleanup(int reg, int cleanup, int cleanupType)
{
   registerCleanups.insert(reg, qMakePair(cleanup, cleanupType));
}

int QDeclarativeBindingCompilerPrivate::acquireReg(int cleanup, int cleanupType)
{
   for (int ii = 0; ii < 32; ++ii) {
      if (!(registers & (1 << ii))) {
         registers |= (1 << ii);

         if (cleanup != Instr::Noop) {
            registerCleanup(ii, cleanup, cleanupType);
         }

         return ii;
      }
   }
   return -1;
}

void QDeclarativeBindingCompilerPrivate::releaseReg(int reg)
{
   Q_ASSERT(reg >= 0 && reg <= 31);

   if (registerCleanups.contains(reg)) {
      QPair<int, int> c = registerCleanups[reg];
      registerCleanups.remove(reg);
      Instr cleanup;
      cleanup.common.type = (quint8)c.first;
      cleanup.cleanup.reg = reg;
      bytecode << cleanup;
   }

   quint32 mask = 1 << reg;
   registers &= ~mask;
}

// Returns a reg
int QDeclarativeBindingCompilerPrivate::registerLiteralString(const QString &str)
{
   QByteArray strdata((const char *)str.constData(), str.length() * sizeof(QChar));
   int offset = data.count();
   data += strdata;

   int reg = acquireReg(Instr::CleanupString);
   if (reg == -1) {
      return false;
   }

   Instr string;
   string.common.type = Instr::String;
   string.string_value.reg = reg;
   string.string_value.offset = offset;
   string.string_value.length = str.length();
   bytecode << string;

   return reg;
}

// Returns an identifier offset
int QDeclarativeBindingCompilerPrivate::registerString(const QString &string)
{
   Q_ASSERT(!string.isEmpty());

   QHash<QString, QPair<int, int> >::ConstIterator iter = registeredStrings.find(string);

   if (iter == registeredStrings.end()) {
      quint32 len = string.length();
      QByteArray lendata((const char *)&len, sizeof(quint32));
      QByteArray strdata((const char *)string.constData(), string.length() * sizeof(QChar));
      strdata.prepend(lendata);
      int rv = data.count();
      data += strdata;

      iter = registeredStrings.insert(string, qMakePair(registeredStrings.count(), rv));
   }

   Instr reg;
   reg.common.type = Instr::InitString;
   reg.initstring.offset = iter->first;
   reg.initstring.dataIdx = iter->second;
   bytecode << reg;
   return reg.initstring.offset;
}

bool QDeclarativeBindingCompilerPrivate::subscription(const QStringList &sub, Result *result)
{
   QString str = sub.join(QLatin1String("."));
   result->subscriptionSet.insert(str);

   if (subscriptionSet.contains(str)) {
      return false;
   } else {
      subscriptionSet.insert(str);
      return true;
   }
}

int QDeclarativeBindingCompilerPrivate::subscriptionIndex(const QStringList &sub)
{
   QString str = sub.join(QLatin1String("."));
   QHash<QString, int>::ConstIterator iter = subscriptionIds.find(str);
   if (iter == subscriptionIds.end()) {
      iter = subscriptionIds.insert(str, subscriptionIds.count());
   }
   usedSubscriptionIds.insert(*iter);
   return *iter;
}

/*
    Returns true if lhs contains no subscriptions that aren't also in base or rhs AND
    rhs contains no subscriptions that aren't also in base or lhs.
*/
bool QDeclarativeBindingCompilerPrivate::subscriptionNeutral(const QSet<QString> &base,
      const QSet<QString> &lhs,
      const QSet<QString> &rhs)
{
   QSet<QString> difflhs = lhs;
   difflhs.subtract(rhs);
   QSet<QString> diffrhs = rhs;
   diffrhs.subtract(lhs);

   difflhs.unite(diffrhs);
   difflhs.subtract(base);

   return difflhs.isEmpty();
}

quint8 QDeclarativeBindingCompilerPrivate::exceptionId(QDeclarativeJS::AST::ExpressionNode *n)
{
   quint8 rv = 0xFF;
   if (n && exceptions.count() < 0xFF) {
      rv = (quint8)exceptions.count();
      QDeclarativeJS::AST::SourceLocation l = n->firstSourceLocation();
      quint64 e = l.startLine;
      e <<= 32;
      e |= l.startColumn;
      exceptions.append(e);
   }
   return rv;
}

QDeclarativeBindingCompiler::QDeclarativeBindingCompiler()
   : d(new QDeclarativeBindingCompilerPrivate)
{
}

QDeclarativeBindingCompiler::~QDeclarativeBindingCompiler()
{
   delete d;
   d = 0;
}

/*
Returns true if any bindings were compiled.
*/
bool QDeclarativeBindingCompiler::isValid() const
{
   return !d->committed.bytecode.isEmpty();
}

/*
-1 on failure, otherwise the binding index to use.
*/
int QDeclarativeBindingCompiler::compile(const Expression &expression, QDeclarativeEnginePrivate *engine)
{
   if (!expression.expression.asAST()) {
      return false;
   }

   if (!qmlExperimental() && expression.property->isValueTypeSubProperty) {
      return -1;
   }

   if (qmlDisableOptimizer()) {
      return -1;
   }

   d->context = expression.context;
   d->component = expression.component;
   d->destination = expression.property;
   d->ids = expression.ids;
   d->imports = expression.imports;
   d->engine = engine;

   if (d->compile(expression.expression.asAST())) {
      return d->commitCompile();
   } else {
      return -1;
   }
}


QByteArray QDeclarativeBindingCompilerPrivate::buildSignalTable() const
{
   QHash<int, QList<int> > table;

   for (int ii = 0; ii < committed.count(); ++ii) {
      const QSet<int> &deps = committed.dependencies.at(ii);
      for (QSet<int>::ConstIterator iter = deps.begin(); iter != deps.end(); ++iter) {
         table[*iter].append(ii);
      }
   }

   QVector<quint32> header;
   QVector<quint32> data;
   for (int ii = 0; ii < committed.subscriptionIds.count(); ++ii) {
      header.append(committed.subscriptionIds.count() + data.count());
      const QList<int> &bindings = table[ii];
      data.append(bindings.count());
      for (int jj = 0; jj < bindings.count(); ++jj) {
         data.append(bindings.at(jj));
      }
   }
   header << data;

   return QByteArray((const char *)header.constData(), header.count() * sizeof(quint32));
}

QByteArray QDeclarativeBindingCompilerPrivate::buildExceptionData() const
{
   QByteArray rv;
   rv.resize(committed.exceptions.count() * sizeof(quint64));
   ::memcpy(rv.data(), committed.exceptions.constData(), rv.size());
   return rv;
}

/*
Returns the compiled program.
*/
QByteArray QDeclarativeBindingCompiler::program() const
{
   QByteArray programData;

   if (isValid()) {
      Program prog;
      prog.bindings = d->committed.count();

      QVector<Instr> bytecode;
      Instr skip;
      skip.common.type = Instr::Skip;
      skip.skip.reg = -1;
      for (int ii = 0; ii < d->committed.count(); ++ii) {
         skip.skip.count = d->committed.count() - ii - 1;
         skip.skip.count += d->committed.offsets.at(ii);
         bytecode << skip;
      }
      bytecode << d->committed.bytecode;

      QByteArray data = d->committed.data;
      while (data.count() % 4) {
         data.append('\0');
      }
      prog.signalTableOffset = data.count();
      data += d->buildSignalTable();
      while (data.count() % 4) {
         data.append('\0');
      }
      prog.exceptionDataOffset = data.count();
      data += d->buildExceptionData();

      prog.dataLength = 4 * ((data.size() + 3) / 4);
      prog.subscriptions = d->committed.subscriptionIds.count();
      prog.identifiers = d->committed.registeredStrings.count();
      prog.instructionCount = bytecode.count();
      prog.compiled = false;
      int size = sizeof(Program) + bytecode.count() * sizeof(Instr);
      size += prog.dataLength;

      programData.resize(size);
      memcpy(programData.data(), &prog, sizeof(Program));
      if (prog.dataLength)
         memcpy((char *)((Program *)programData.data())->data(), data.constData(),
                data.size());
      memcpy((char *)((Program *)programData.data())->instructions(), bytecode.constData(),
             bytecode.count() * sizeof(Instr));
   }

   return programData;
}



QT_END_NAMESPACE
