/****************************************************************************
**
** Copyright (C) 2012 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** This file is part of the documentation of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** You may use this file under the terms of the BSD license as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of Nokia Corporation and its Subsidiary(-ies) nor
**     the names of its contributors may be used to endorse or promote
**     products derived from this software without specific prior written
**     permission.
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
** $QT_END_LICENSE$
**
****************************************************************************/

//! [0]
QScriptEngine myEngine;
QScriptValue myObject = myEngine.newObject();
QScriptValue myOtherObject = myEngine.newObject();
myObject.setProperty("myChild", myOtherObject);
myObject.setProperty("name", "John Doe");
//! [0]


//! [1]
QScriptValue val(&myEngine, 123);
myObject.setProperty("myReadOnlyProperty", val, QScriptValue::ReadOnly);
//! [1]


//! [2]
QScriptEngine engine;
engine.evaluate("function fullName() { return this.firstName + ' ' + this.lastName; }");
engine.evaluate("somePerson = { firstName: 'John', lastName: 'Doe' }");

QScriptValue global = engine.globalObject();
QScriptValue fullName = global.property("fullName");
QScriptValue who = global.property("somePerson");
qDebug() << fullName.call(who).toString(); // "John Doe"

engine.evaluate("function cube(x) { return x * x * x; }");
QScriptValue cube = global.property("cube");
QScriptValueList args;
args << 3;
qDebug() << cube.call(QScriptValue(), args).toNumber(); // 27
//! [2]


//! [3]
QScriptValue myNativeFunction(QScriptContext *ctx, QScriptEngine *)
{
    QScriptValue otherFunction = ...;
    return otherFunction.call(ctx->thisObject(), ctx->argumentsObject());
}
//! [3]
