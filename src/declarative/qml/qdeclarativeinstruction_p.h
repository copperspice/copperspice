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

#ifndef QDECLARATIVEINSTRUCTION_P_H
#define QDECLARATIVEINSTRUCTION_P_H

#include <QtCore/qglobal.h>

QT_BEGIN_NAMESPACE

class QDeclarativeCompiledData;
class QDeclarativeInstruction
{
 public:
   enum Type {
      //
      // Object Creation
      //
      //    CreateObject - Create a new object instance and push it on the
      //                   object stack
      //    SetId - Set the id of the object on the top of the object stack
      //    SetDefault - Sets the instance on the top of the object stack to
      //                 be the context's default object.
      //    StoreMetaObject - Assign the dynamic metaobject to object on the
      //                      top of the stack.
      Init,                     /* init */
      CreateObject,             /* create */
      CreateSimpleObject,       /* createSimple */
      SetId,                    /* setId */
      SetDefault,
      CreateComponent,          /* createComponent */
      StoreMetaObject,          /* storeMeta */

      //
      // Precomputed single assignment
      //
      //    StoreFloat - Store a float in a core property
      //    StoreDouble - Store a double in a core property
      //    StoreInteger - Store a int or uint in a core property
      //    StoreBool - Store a bool in a core property
      //    StoreString - Store a QString in a core property
      //    StoreUrl - Store a QUrl in a core property
      //    StoreColor - Store a QColor in a core property
      //    StoreDate - Store a QDate in a core property
      //    StoreTime - Store a QTime in a core property
      //    StoreDateTime - Store a QDateTime in a core property
      //    StoreVariant - Store a QVariant in a core property
      //    StoreObject - Pop the object on the top of the object stack and
      //                  store it in a core property
      StoreFloat,               /* storeFloat */
      StoreDouble,              /* storeDouble */
      StoreInteger,             /* storeInteger */
      StoreBool,                /* storeBool */
      StoreString,              /* storeString */
      StoreUrl,                 /* storeUrl */
      StoreColor,               /* storeColor */
      StoreDate,                /* storeDate */
      StoreTime,                /* storeTime */
      StoreDateTime,            /* storeDateTime */
      StorePoint,               /* storeRealPair */
      StorePointF,              /* storeRealPair */
      StoreSize,                /* storeRealPair */
      StoreSizeF,               /* storeRealPair */
      StoreRect,                /* storeRect */
      StoreRectF,               /* storeRect */
      StoreVector3D,            /* storeVector3D */
      StoreVariant,             /* storeString */
      StoreVariantInteger,      /* storeInteger */
      StoreVariantDouble,       /* storeDouble */
      StoreVariantBool,         /* storeBool */
      StoreObject,              /* storeObject */
      StoreVariantObject,       /* storeObject */
      StoreInterface,           /* storeObject */

      StoreSignal,              /* storeSignal */
      StoreImportedScript,      /* storeScript */
      StoreScriptString,        /* storeScriptString */

      //
      // Unresolved single assignment
      //
      AssignSignalObject,       /* assignSignalObject */
      AssignCustomType,         /* assignCustomType */

      StoreBinding,             /* assignBinding */
      StoreBindingOnAlias,      /* assignBinding */
      StoreCompiledBinding,     /* assignBinding */
      StoreValueSource,         /* assignValueSource */
      StoreValueInterceptor,    /* assignValueInterceptor */

      BeginObject,              /* begin */

      StoreObjectQList,         /* NA */
      AssignObjectList,         /* NA */

      FetchAttached,            /* fetchAttached */
      FetchQList,               /* fetch */
      FetchObject,              /* fetch */
      FetchValueType,           /* fetchValue */

      //
      // Stack manipulation
      //
      //    PopFetchedObject - Remove an object from the object stack
      //    PopQList - Remove a list from the list stack
      PopFetchedObject,
      PopQList,
      PopValueType,            /* fetchValue */

      //
      // Deferred creation
      //
      Defer                    /* defer */
   };
   QDeclarativeInstruction()
      : line(0) {}

   Type type;
   unsigned short line;

   struct InitInstruction {
      int bindingsSize;
      int parserStatusSize;
      int contextCache;
      int compiledBinding;
   };
   struct CreateInstruction {
      int type;
      int data;
      int bindingBits;
      ushort column;
   };
   struct CreateSimpleInstruction {
      void (*create)(void *);
      int typeSize;
      int type;
      ushort column;
   };
   struct StoreMetaInstruction {
      int data;
      int aliasData;
      int propertyCache;
   };
   struct SetIdInstruction {
      int value;
      int index;
   };
   struct AssignValueSourceInstruction {
      int property;
      int owner;
      int castValue;
   };
   struct AssignValueInterceptorInstruction {
      int property;
      int owner;
      int castValue;
   };
   struct AssignBindingInstruction {
      unsigned int property;
      int value;
      short context;
      short owner;
   };
   struct FetchInstruction {
      int property;
   };
   struct FetchValueInstruction {
      int property;
      int type;
      quint32 bindingSkipList;
   };
   struct FetchQmlListInstruction {
      int property;
      int type;
   };
   struct BeginInstruction {
      int castValue;
   };
   struct StoreFloatInstruction {
      int propertyIndex;
      float value;
   };
   struct StoreDoubleInstruction {
      int propertyIndex;
      double value;
   };
   struct StoreIntegerInstruction {
      int propertyIndex;
      int value;
   };
   struct StoreBoolInstruction {
      int propertyIndex;
      bool value;
   };
   struct StoreStringInstruction {
      int propertyIndex;
      int value;
   };
   struct StoreScriptStringInstruction {
      int propertyIndex;
      int value;
      int scope;
   };
   struct StoreScriptInstruction {
      int value;
   };
   struct StoreUrlInstruction {
      int propertyIndex;
      int value;
   };
   struct StoreColorInstruction {
      int propertyIndex;
      unsigned int value;
   };
   struct StoreDateInstruction {
      int propertyIndex;
      int value;
   };
   struct StoreTimeInstruction {
      int propertyIndex;
      int valueIndex;
   };
   struct StoreDateTimeInstruction {
      int propertyIndex;
      int valueIndex;
   };
   struct StoreRealPairInstruction {
      int propertyIndex;
      int valueIndex;
   };
   struct StoreRectInstruction {
      int propertyIndex;
      int valueIndex;
   };
   struct StoreVector3DInstruction {
      int propertyIndex;
      int valueIndex;
   };
   struct StoreObjectInstruction {
      int propertyIndex;
   };
   struct AssignCustomTypeInstruction {
      int propertyIndex;
      int valueIndex;
   };
   struct StoreSignalInstruction {
      int signalIndex;
      int value;
      short context;
      int name;
   };
   struct AssignSignalObjectInstruction {
      int signal;
   };
   struct CreateComponentInstruction {
      int count;
      ushort column;
      int endLine;
      int metaObject;
   };
   struct FetchAttachedInstruction {
      int id;
   };
   struct DeferInstruction {
      int deferCount;
   };

   union {
      InitInstruction init;
      CreateInstruction create;
      CreateSimpleInstruction createSimple;
      StoreMetaInstruction storeMeta;
      SetIdInstruction setId;
      AssignValueSourceInstruction assignValueSource;
      AssignValueInterceptorInstruction assignValueInterceptor;
      AssignBindingInstruction assignBinding;
      FetchInstruction fetch;
      FetchValueInstruction fetchValue;
      FetchQmlListInstruction fetchQmlList;
      BeginInstruction begin;
      StoreFloatInstruction storeFloat;
      StoreDoubleInstruction storeDouble;
      StoreIntegerInstruction storeInteger;
      StoreBoolInstruction storeBool;
      StoreStringInstruction storeString;
      StoreScriptStringInstruction storeScriptString;
      StoreScriptInstruction storeScript;
      StoreUrlInstruction storeUrl;
      StoreColorInstruction storeColor;
      StoreDateInstruction storeDate;
      StoreTimeInstruction storeTime;
      StoreDateTimeInstruction storeDateTime;
      StoreRealPairInstruction storeRealPair;
      StoreRectInstruction storeRect;
      StoreVector3DInstruction storeVector3D;
      StoreObjectInstruction storeObject;
      AssignCustomTypeInstruction assignCustomType;
      StoreSignalInstruction storeSignal;
      AssignSignalObjectInstruction assignSignalObject;
      CreateComponentInstruction createComponent;
      FetchAttachedInstruction fetchAttached;
      DeferInstruction defer;
   };

   void dump(QDeclarativeCompiledData *);
};

QT_END_NAMESPACE

#endif // QDECLARATIVEINSTRUCTION_P_H
