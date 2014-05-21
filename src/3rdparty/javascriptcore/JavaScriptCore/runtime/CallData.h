/*
 * Copyright (C) 2008 Apple Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1.  Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 * 2.  Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 * 3.  Neither the name of Apple Computer, Inc. ("Apple") nor the names of
 *     its contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE AND ITS CONTRIBUTORS "AS IS" AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL APPLE OR ITS CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef CallData_h
#define CallData_h

#include "NativeFunctionWrapper.h"

namespace JSC {

    class ArgList;
    class ExecState;
    class FunctionExecutable;
    class JSObject;
    class JSValue;
    class ScopeChainNode;

    enum CallType {
        CallTypeNone,
        CallTypeHost,
        CallTypeJS
    };

    typedef JSValue (JSC_HOST_CALL *NativeFunction)(ExecState*, JSObject*, JSValue thisValue, const ArgList&);

#ifdef QT_BUILD_SCRIPT_LIB
    class NativeFuncWrapper
    {
        NativeFunction ptr;
    public:
        inline NativeFuncWrapper& operator=(NativeFunction func)
        {
            ptr = func;
            return *this;
        }
        inline operator NativeFunction() const {return ptr;}
        inline operator bool() const {return ptr;}

        JSValue operator()(ExecState* exec, JSObject* jsobj, JSValue thisValue, const ArgList& argList) const;
    };
#endif

#if defined(QT_BUILD_SCRIPT_LIB) && OS(SOLARIS)
    struct
#else
    union
#endif
    CallData {
        struct {
#ifndef QT_BUILD_SCRIPT_LIB
            NativeFunction function;
#else
            NativeFuncWrapper function;
#endif
        } native;
        struct {
            FunctionExecutable* functionExecutable;
            ScopeChainNode* scopeChain;
        } js;
    };

    JSValue call(ExecState*, JSValue functionObject, CallType, const CallData&, JSValue thisValue, const ArgList&);

} // namespace JSC

#endif // CallData_h
