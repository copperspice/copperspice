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

#ifndef QDECLARATIVECOMPILER_P_H
#define QDECLARATIVECOMPILER_P_H

#include "qdeclarative.h"
#include "qdeclarativeerror.h"
#include "qdeclarativeinstruction_p.h"
#include "qdeclarativeparser_p.h"
#include "qdeclarativeengine_p.h"
#include "qbitfield_p.h"
#include "qdeclarativepropertycache_p.h"
#include "qdeclarativeintegercache_p.h"
#include "qdeclarativetypenamecache_p.h"
#include "qdeclarativetypeloader_p.h"

#include <QtCore/qbytearray.h>
#include <QtCore/qset.h>
#include <QtCore/QCoreApplication>

QT_BEGIN_NAMESPACE

class QDeclarativeEngine;
class QDeclarativeComponent;
class QDeclarativeContext;
class QDeclarativeContextData;

class QScriptProgram;
class QDeclarativeCompiledData : public QDeclarativeRefCount, public QDeclarativeCleanup
{
 public:
   QDeclarativeCompiledData(QDeclarativeEngine *engine);
   virtual ~QDeclarativeCompiledData();

   QString name;
   QUrl url;
   QDeclarativeTypeNameCache *importCache;

   struct TypeReference {
      TypeReference()
         : type(0), typePropertyCache(0), component(0) {}

      QByteArray className;
      QDeclarativeType *type;
      QDeclarativePropertyCache *typePropertyCache;
      QDeclarativeCompiledData *component;

      QObject *createInstance(QDeclarativeContextData *, const QBitField &, QList<QDeclarativeError> *) const;
      const QMetaObject *metaObject() const;
      QDeclarativePropertyCache *propertyCache() const;
      QDeclarativePropertyCache *createPropertyCache(QDeclarativeEngine *);
   };
   QList<TypeReference> types;
   struct CustomTypeData {
      int index;
      int type;
   };

   const QMetaObject *root;
   QAbstractDynamicMetaObject rootData;
   QDeclarativePropertyCache *rootPropertyCache;
   QList<QString> primitives;
   QList<float> floatData;
   QList<int> intData;
   QList<CustomTypeData> customTypeData;
   QList<QByteArray> datas;
   QList<QDeclarativeParser::Location> locations;
   QList<QDeclarativeInstruction> bytecode;
   QList<QScriptProgram *> cachedPrograms;
   QList<QScriptValue *> cachedClosures;
   QList<QDeclarativePropertyCache *> propertyCaches;
   QList<QDeclarativeIntegerCache *> contextCaches;
   QList<QDeclarativeParser::Object::ScriptBlock> scripts;
   QList<QUrl> urls;

   void dumpInstructions();

 protected:
   virtual void clear(); // From QDeclarativeCleanup

 private:
   void dump(QDeclarativeInstruction *, int idx = -1);
   QDeclarativeCompiledData(const QDeclarativeCompiledData &other);
   QDeclarativeCompiledData &operator=(const QDeclarativeCompiledData &other);
   QByteArray packData;
   friend class QDeclarativeCompiler;
   int pack(const char *, size_t);

   int indexForString(const QString &);
   int indexForByteArray(const QByteArray &);
   int indexForFloat(float *, int);
   int indexForInt(int *, int);
   int indexForLocation(const QDeclarativeParser::Location &);
   int indexForLocation(const QDeclarativeParser::LocationSpan &);
   int indexForUrl(const QUrl &);
};

class QMetaObjectBuilder;
class QDeclarativeCompiler
{
   Q_DECLARE_TR_FUNCTIONS(QDeclarativeCompiler)
 public:
   QDeclarativeCompiler();

   bool compile(QDeclarativeEngine *, QDeclarativeTypeData *, QDeclarativeCompiledData *);

   bool isError() const;
   QList<QDeclarativeError> errors() const;

   static bool isAttachedPropertyName(const QByteArray &);
   static bool isSignalPropertyName(const QByteArray &);

   int evaluateEnum(const QByteArray &script) const; // for QDeclarativeCustomParser::evaluateEnum
   const QMetaObject *resolveType(const QByteArray &name) const; // for QDeclarativeCustomParser::resolveType
   int rewriteBinding(const QString &expression, const QByteArray &name); // for QDeclarativeCustomParser::rewriteBinding

 private:
   static void reset(QDeclarativeCompiledData *);

   struct BindingContext {
      BindingContext()
         : stack(0), owner(0), object(0) {}
      BindingContext(QDeclarativeParser::Object *o)
         : stack(0), owner(0), object(o) {}
      BindingContext incr() const {
         BindingContext rv(object);
         rv.stack = stack + 1;
         return rv;
      }
      bool isSubContext() const {
         return stack != 0;
      }
      int stack;
      int owner;
      QDeclarativeParser::Object *object;
   };

   void compileTree(QDeclarativeParser::Object *tree);


   bool buildObject(QDeclarativeParser::Object *obj, const BindingContext &);
   bool buildComponent(QDeclarativeParser::Object *obj, const BindingContext &);
   bool buildSubObject(QDeclarativeParser::Object *obj, const BindingContext &);
   bool buildSignal(QDeclarativeParser::Property *prop, QDeclarativeParser::Object *obj,
                    const BindingContext &);
   bool buildProperty(QDeclarativeParser::Property *prop, QDeclarativeParser::Object *obj,
                      const BindingContext &);
   bool buildPropertyInNamespace(QDeclarativeImportedNamespace *ns,
                                 QDeclarativeParser::Property *prop,
                                 QDeclarativeParser::Object *obj,
                                 const BindingContext &);
   bool buildIdProperty(QDeclarativeParser::Property *prop, QDeclarativeParser::Object *obj);
   bool buildAttachedProperty(QDeclarativeParser::Property *prop,
                              QDeclarativeParser::Object *obj,
                              const BindingContext &ctxt);
   bool buildGroupedProperty(QDeclarativeParser::Property *prop,
                             QDeclarativeParser::Object *obj,
                             const BindingContext &ctxt);
   bool buildValueTypeProperty(QObject *type,
                               QDeclarativeParser::Object *obj,
                               QDeclarativeParser::Object *baseObj,
                               const BindingContext &ctxt);
   bool buildListProperty(QDeclarativeParser::Property *prop,
                          QDeclarativeParser::Object *obj,
                          const BindingContext &ctxt);
   bool buildScriptStringProperty(QDeclarativeParser::Property *prop,
                                  QDeclarativeParser::Object *obj,
                                  const BindingContext &ctxt);
   bool buildPropertyAssignment(QDeclarativeParser::Property *prop,
                                QDeclarativeParser::Object *obj,
                                const BindingContext &ctxt);
   bool buildPropertyObjectAssignment(QDeclarativeParser::Property *prop,
                                      QDeclarativeParser::Object *obj,
                                      QDeclarativeParser::Value *value,
                                      const BindingContext &ctxt);
   bool buildPropertyOnAssignment(QDeclarativeParser::Property *prop,
                                  QDeclarativeParser::Object *obj,
                                  QDeclarativeParser::Object *baseObj,
                                  QDeclarativeParser::Value *value,
                                  const BindingContext &ctxt);
   bool buildPropertyLiteralAssignment(QDeclarativeParser::Property *prop,
                                       QDeclarativeParser::Object *obj,
                                       QDeclarativeParser::Value *value,
                                       const BindingContext &ctxt);
   bool doesPropertyExist(QDeclarativeParser::Property *prop, QDeclarativeParser::Object *obj);
   bool testLiteralAssignment(const QMetaProperty &prop,
                              QDeclarativeParser::Value *value);
   bool testQualifiedEnumAssignment(const QMetaProperty &prop,
                                    QDeclarativeParser::Object *obj,
                                    QDeclarativeParser::Value *value,
                                    bool *isAssignment);
   enum DynamicMetaMode { IgnoreAliases, ResolveAliases, ForceCreation };
   bool mergeDynamicMetaProperties(QDeclarativeParser::Object *obj);
   bool buildDynamicMeta(QDeclarativeParser::Object *obj, DynamicMetaMode mode);
   bool checkDynamicMeta(QDeclarativeParser::Object *obj);
   bool buildBinding(QDeclarativeParser::Value *, QDeclarativeParser::Property *prop,
                     const BindingContext &ctxt);
   bool buildComponentFromRoot(QDeclarativeParser::Object *obj, const BindingContext &);
   bool compileAlias(QMetaObjectBuilder &,
                     QByteArray &data,
                     QDeclarativeParser::Object *obj,
                     const QDeclarativeParser::Object::DynamicProperty &);
   bool completeComponentBuild();
   bool checkValidId(QDeclarativeParser::Value *, const QString &);


   void genObject(QDeclarativeParser::Object *obj);
   void genObjectBody(QDeclarativeParser::Object *obj);
   void genValueTypeProperty(QDeclarativeParser::Object *obj, QDeclarativeParser::Property *);
   void genComponent(QDeclarativeParser::Object *obj);
   void genValueProperty(QDeclarativeParser::Property *prop, QDeclarativeParser::Object *obj);
   void genListProperty(QDeclarativeParser::Property *prop, QDeclarativeParser::Object *obj);
   void genPropertyAssignment(QDeclarativeParser::Property *prop,
                              QDeclarativeParser::Object *obj,
                              QDeclarativeParser::Property *valueTypeProperty = 0);
   void genLiteralAssignment(const QMetaProperty &prop,
                             QDeclarativeParser::Value *value);
   void genBindingAssignment(QDeclarativeParser::Value *binding,
                             QDeclarativeParser::Property *prop,
                             QDeclarativeParser::Object *obj,
                             QDeclarativeParser::Property *valueTypeProperty = 0);
   int genContextCache();

   int genValueTypeData(QDeclarativeParser::Property *prop, QDeclarativeParser::Property *valueTypeProp);
   int genPropertyData(QDeclarativeParser::Property *prop);

   int componentTypeRef();

   static QDeclarativeType *toQmlType(QDeclarativeParser::Object *from);
   bool canCoerce(int to, QDeclarativeParser::Object *from);

   QStringList deferredProperties(QDeclarativeParser::Object *);
   int indexOfProperty(QDeclarativeParser::Object *, const QByteArray &, bool *notInRevision = 0);
   int indexOfSignal(QDeclarativeParser::Object *, const QByteArray &, bool *notInRevision = 0);

   void addId(const QString &, QDeclarativeParser::Object *);

   void dumpStats();

   struct BindingReference {
      QDeclarativeParser::Variant expression;
      QDeclarativeParser::Property *property;
      QDeclarativeParser::Value *value;

      enum DataType { QtScript, Experimental };
      DataType dataType;

      int compiledIndex;

      QByteArray compiledData;
      BindingContext bindingContext;
   };
   void addBindingReference(const BindingReference &);

   struct ComponentCompileState {
      ComponentCompileState()
         : parserStatusCount(0), pushedProperties(0), root(0) {}
      QHash<QString, QDeclarativeParser::Object *> ids;
      QHash<int, QDeclarativeParser::Object *> idIndexes;
      int parserStatusCount;
      int pushedProperties;

      QByteArray compiledBindingData;

      QHash<QDeclarativeParser::Value *, BindingReference> bindings;
      QHash<QDeclarativeParser::Value *, BindingContext> signalExpressions;
      QList<QDeclarativeParser::Object *> aliasingObjects;
      QDeclarativeParser::Object *root;
   };
   ComponentCompileState compileState;

   struct ComponentStat {
      ComponentStat() : ids(0), objects(0) {}

      int lineNumber;

      int ids;
      QList<QDeclarativeParser::LocationSpan> scriptBindings;
      QList<QDeclarativeParser::LocationSpan> optimizedBindings;
      int objects;
   };
   ComponentStat componentStat;

   void saveComponentState();

   ComponentCompileState componentState(QDeclarativeParser::Object *);
   QHash<QDeclarativeParser::Object *, ComponentCompileState> savedCompileStates;
   QList<ComponentStat> savedComponentStats;

   QList<QDeclarativeError> exceptions;
   QDeclarativeCompiledData *output;
   QDeclarativeEngine *engine;
   QDeclarativeEnginePrivate *enginePrivate;
   QDeclarativeParser::Object *unitRoot;
   QDeclarativeTypeData *unit;
};
QT_END_NAMESPACE

#endif // QDECLARATIVECOMPILER_P_H
