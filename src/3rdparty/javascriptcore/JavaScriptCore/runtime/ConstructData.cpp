/*
 * Copyright (C) 2008 Apple Inc. All Rights Reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. 
 */

#include "config.h"
#include "ConstructData.h"

#include "JSFunction.h"

#ifdef QT_BUILD_SCRIPT_LIB
#include "Debugger.h"
#include "DebuggerCallFrame.h"
#include "JSGlobalObject.h"
#endif

namespace JSC {

#ifdef QT_BUILD_SCRIPT_LIB
JSObject* JSC::NativeConstrWrapper::operator() (ExecState* exec, JSObject* jsobj, const ArgList& argList) const
{
    Debugger* debugger = exec->lexicalGlobalObject()->debugger();
    if (debugger)
        debugger->callEvent(DebuggerCallFrame(exec), -1, -1);

    JSObject* returnValue = ptr(exec, jsobj, argList);

    if ((debugger) && (callDebuggerFunctionExit))
        debugger->functionExit(JSValue(returnValue), -1);

    return returnValue;
}
#endif

JSObject* construct(ExecState* exec, JSValue object, ConstructType constructType, const ConstructData& constructData, const ArgList& args)
{
    if (constructType == ConstructTypeHost)
        return constructData.native.function(exec, asObject(object), args);
    ASSERT(constructType == ConstructTypeJS);
    // FIXME: Can this be done more efficiently using the constructData?
    return asFunction(object)->construct(exec, args);
}

} // namespace JSC
