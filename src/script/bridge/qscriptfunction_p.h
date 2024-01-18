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

#ifndef QSCRIPTFUNCTION_P_H
#define QSCRIPTFUNCTION_P_H

#include <qglobal.h>
#include "qscriptengine.h"
#include "PrototypeFunction.h"


namespace QScript {

class FunctionWrapper : public JSC::PrototypeFunction // ### subclass InternalFunction instead
{
 public:
   // work around CELL_SIZE limitation
   struct Data {
      QScriptEngine::FunctionSignature function;
   };

   FunctionWrapper(JSC::ExecState *, int length, const JSC::Identifier &,
      QScriptEngine::FunctionSignature);
   ~FunctionWrapper();

   virtual const JSC::ClassInfo *classInfo() const {
      return &info;
   }

   static const JSC::ClassInfo info;

   QScriptEngine::FunctionSignature function() const {
      return data->function;
   }

 private:
   virtual JSC::ConstructType getConstructData(JSC::ConstructData &);

   static JSC::JSValue JSC_HOST_CALL proxyCall(JSC::ExecState *, JSC::JSObject *,
      JSC::JSValue, const JSC::ArgList &);
   static JSC::JSObject *proxyConstruct(JSC::ExecState *, JSC::JSObject *,
      const JSC::ArgList &);

 private:
   Data *data;
};

class FunctionWithArgWrapper : public JSC::PrototypeFunction
{
 public:
   // work around CELL_SIZE limitation
   struct Data {
      QScriptEngine::FunctionWithArgSignature function;
      void *arg;
   };

   FunctionWithArgWrapper(JSC::ExecState *, int length, const JSC::Identifier &,
      QScriptEngine::FunctionWithArgSignature, void *);
   ~FunctionWithArgWrapper();

   virtual const JSC::ClassInfo *classInfo() const {
      return &info;
   }
   static const JSC::ClassInfo info;

   QScriptEngine::FunctionWithArgSignature function() const {
      return data->function;
   }

   void *arg() const {
      return data->arg;
   }

 private:
   virtual JSC::ConstructType getConstructData(JSC::ConstructData &);

   static JSC::JSValue JSC_HOST_CALL proxyCall(JSC::ExecState *, JSC::JSObject *,
      JSC::JSValue, const JSC::ArgList &);
   static JSC::JSObject *proxyConstruct(JSC::ExecState *, JSC::JSObject *,
      const JSC::ArgList &);

 private:
   Data *data;
};

} // namespace QScript


#endif
