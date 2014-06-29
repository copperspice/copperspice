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
#include <QtScript>
//! [0]

//! [13]
Q_PROPERTY(bool enabled READ enabled WRITE setEnabled)
//! [13]

//! [18]
QScriptValue myQObjectConstructor(QScriptContext *context, QScriptEngine *engine)
{
  // let the engine manage the new object's lifetime.
  return engine->newQObject(new MyQObject(), QScriptEngine::ScriptOwnership);
}
//! [18]


//! [19]
class MyObject : public QObject
{
    Q_OBJECT

public:
    MyObject( ... );

    void aNonScriptableFunction();

public slots: // these functions (slots) will be available in QtScript
    void calculate( ... );
    void setEnabled( bool enabled );
    bool isEnabled() const;

private:
   ....

};
//! [19]


//! [20]
class MyObject : public QObject
{
    Q_OBJECT

    public:
    Q_INVOKABLE void thisMethodIsInvokableInQtScript();
    void thisMethodIsNotInvokableInQtScript();

    ...
};
//! [20]


//! [23]
class MyObject : public QObject
{
    Q_OBJECT
    // define the enabled property
    Q_PROPERTY( bool enabled WRITE setEnabled READ isEnabled )

public:
    MyObject( ... );

    void aNonScriptableFunction();

public slots: // these functions (slots) will be available in QtScript
    void calculate( ... );
    void setEnabled( bool enabled );
    bool isEnabled() const;

private:
   ....

};
//! [23]


//! [24]
Q_PROPERTY(int nonScriptableProperty READ foo WRITE bar SCRIPTABLE false)
//! [24]


//! [25]
class MyObject : public QObject
{
    Q_OBJECT
    // define the enabled property
    Q_PROPERTY( bool enabled WRITE setEnabled READ isEnabled )

public:
    MyObject( ... );

    void aNonScriptableFunction();

public slots: // these functions (slots) will be available in QtScript
    void calculate( ... );
    void setEnabled( bool enabled );
    bool isEnabled() const;

signals: // the signals
    void enabledChanged( bool newState );

private:
   ....

};
//! [25]


//! [34]
QScriptValue Person_ctor(QScriptContext *context, QScriptEngine *engine)
{
  QString name = context->argument(0).toString();
  context->thisObject().setProperty("name", name);
  return engine->undefinedValue();
}
//! [34]


//! [35]
QScriptValue Person_prototype_toString(QScriptContext *context, QScriptEngine *engine)
{
  QString name = context->thisObject().property("name").toString();
  QString result = QString::fromLatin1("Person(name: %0)").arg(name);
  return result;
}
//! [35]


//! [36]
QScriptEngine engine;
QScriptValue ctor = engine.newFunction(Person_ctor);
ctor.property("prototype").setProperty("toString", engine.newFunction(Person_prototype_toString));
QScriptValue global = engine.globalObject();
global.setProperty("Person", ctor);
//! [36]


//! [37]
QScriptValue Employee_ctor(QScriptContext *context, QScriptEngine *engine)
{
  QScriptValue super = context->callee().property("prototype").property("constructor");
  super.call(context->thisObject(), QScriptValueList() << context->argument(0));
  context->thisObject().setProperty("salary", context->argument(1));
  return engine->undefinedValue();
}
//! [37]


//! [38]
QScriptValue empCtor = engine.newFunction(Employee_ctor);
empCtor.setProperty("prototype", global.property("Person").construct());
global.setProperty("Employee", empCtor);
//! [38]


//! [39]
Q_DECLARE_METATYPE(QPointF)
Q_DECLARE_METATYPE(QPointF*)

QScriptValue QPointF_prototype_x(QScriptContext *context, QScriptEngine *engine)
{
  // Since the point is not to be modified, it's OK to cast to a value here
    QPointF point = qscriptvalue_cast<QPointF>(context->thisObject());
    return point.x();
}

QScriptValue QPointF_prototype_setX(QScriptContext *context, QScriptEngine *engine)
{
    // Cast to a pointer to be able to modify the underlying C++ value
    QPointF *point = qscriptvalue_cast<QPointF*>(context->thisObject());
    if (!point)
        return context->throwError(QScriptContext::TypeError, "QPointF.prototype.setX: this object is not a QPointF");
    point->setX(context->argument(0).toNumber());
    return engine->undefinedValue();
}
//! [39]


//! [43]
class MyObject : public QObject
{
    Q_OBJECT
    ...
};

Q_DECLARE_METATYPE(MyObject*)

QScriptValue myObjectToScriptValue(QScriptEngine *engine, MyObject* const &in)
{ return engine->newQObject(in); }

void myObjectFromScriptValue(const QScriptValue &object, MyObject* &out)
{ out = qobject_cast<MyObject*>(object.toQObject()); }

...

qScriptRegisterMetaType(&engine, myObjectToScriptValue, myObjectFromScriptValue);
//! [43]

//! [44]
QScriptValue QPoint_ctor(QScriptContext *context, QScriptEngine *engine)
{
    int x = context->argument(0).toInt32();
    int y = context->argument(1).toInt32();
    return engine->toScriptValue(QPoint(x, y));
}

...

engine.globalObject().setProperty("QPoint", engine.newFunction(QPoint_ctor));
//! [44]

//! [45]
QScriptValue myPrintFunction(QScriptContext *context, QScriptEngine *engine)
{
    QString result;
    for (int i = 0; i < context->argumentCount(); ++i) {
        if (i > 0)
            result.append(" ");
        result.append(context->argument(i).toString());
    }

    QScriptValue calleeData = context->callee().data();
    QPlainTextEdit *edit = qobject_cast<QPlainTextEdit*>(calleeData.toQObject());
    edit->appendPlainText(result);

    return engine->undefinedValue();
}
//! [45]

//! [46]
int main(int argc, char **argv)
{
    QApplication app(argc, argv);

    QScriptEngine eng;
    QPlainTextEdit edit;

    QScriptValue fun = eng.newFunction(myPrintFunction);
    fun.setData(eng.newQObject(&edit));
    eng.globalObject().setProperty("print", fun);

    eng.evaluate("print('hello', 'world')");

    edit.show();
    return app.exec();
}
//! [46]


//! [47]
QScriptEngine eng;
QLineEdit *edit = new QLineEdit(...);
QScriptValue handler = eng.evaluate("(function(text) { print('text was changed to', text); })");
qScriptConnect(edit, SIGNAL(textChanged(const QString &)), QScriptValue(), handler);
//! [47]

//! [48]
QLineEdit *edit1 = new QLineEdit(...);
QLineEdit *edit2 = new QLineEdit(...);

QScriptValue handler = eng.evaluate("(function() { print('I am', this.name); })");
QScriptValue obj1 = eng.newObject();
obj1.setProperty("name", "the walrus");
QScriptValue obj2 = eng.newObject();
obj2.setProperty("name", "Sam");

qScriptConnect(edit1, SIGNAL(returnPressed()), obj1, handler);
qScriptConnect(edit2, SIGNAL(returnPressed()), obj2, handler);
//! [48]

//! [52]
QScriptValue getProperty(QScriptContext *ctx, QScriptEngine *eng)
{
    QString name = ctx->argument(0).toString();
    return ctx->thisObject().property(name);
}
//! [52]

//! [53]
QScriptValue myCompare(QScriptContext *ctx, QScriptEngine *eng)
{
    double first = ctx->argument(0).toNumber();
    double second = ctx->argument(1).toNumber();
    int result;
    if (first == second)
        result = 0;
    else if (first < second)
        result = -1;
    else
        result = 1;
    return result;
}
//! [53]

//! [54]
QScriptEngine eng;
QScriptValue comparefn = eng.newFunction(myCompare);
QScriptValue array = eng.evaluate("new Array(10, 5, 20, 15, 30)");
array.property("sort").call(array, QScriptValueList() << comparefn);

// prints "5,10,15,20,30"
qDebug() << array.toString();
//! [54]

//! [55]
QScriptValue rectifier(QScriptContext *ctx, QScriptEngine *eng)
{
    QRectF magicRect = qscriptvalue_cast<QRectF>(ctx->callee().data());
    QRectF sourceRect = qscriptvalue_cast<QRectF>(ctx->argument(0));
    return eng->toScriptValue(sourceRect.intersected(magicRect));
}

...

QScriptValue fun = eng.newFunction(rectifier);
QRectF magicRect = QRectF(10, 20, 30, 40);
fun.setData(eng.toScriptValue(magicRect));
eng.globalObject().setProperty("rectifier", fun);
//! [55]

//! [58]
QScriptValue add(QScriptContext *ctx, QScriptEngine *eng)
{
    double a = ctx->argument(0).toNumber();
    double b = ctx->argument(1).toNumber();
    return a + b;
}
//! [58]

//! [62]
QScriptValue add(QScriptContext *ctx, QScriptEngine *eng)
{
    if (ctx->argumentCount() != 2)
        return ctx->throwError("add() takes exactly two arguments");
    double a = ctx->argument(0).toNumber();
    double b = ctx->argument(1).toNumber();
    return a + b;
}
//! [62]

//! [63]
QScriptValue add(QScriptContext *ctx, QScriptEngine *eng)
{
    if (ctx->argumentCount() != 2)
        return ctx->throwError("add() takes exactly two arguments");
    if (!ctx->argument(0).isNumber())
        return ctx->throwError(QScriptContext::TypeError, "add(): first argument is not a number");
    if (!ctx->argument(1).isNumber())
        return ctx->throwError(QScriptContext::TypeError, "add(): second argument is not a number");
    double a = ctx->argument(0).toNumber();
    double b = ctx->argument(1).toNumber();
    return a + b;
}
//! [63]

//! [65]
QScriptValue concat(QScriptContext *ctx, QScriptEngine *eng)
{
    QString result = "";
    for (int i = 0; i < ctx->argumentCount(); ++i)
        result += ctx->argument(i).toString();
    return result;
}
//! [65]

//! [67]
QScriptValue sort(QScriptContext *ctx, QScriptEngine *eng)
{
    QScriptValue comparefn = ctx->argument(0);
    if (comparefn.isUndefined())
        comparefn = /* the built-in comparison function */;
    else if (!comparefn.isFunction())
        return ctx->throwError(QScriptContext::TypeError, "sort(): argument is not a function");
    ...
}
//! [67]

//! [69]
QScriptValue foo(QScriptContext *ctx, QScriptEngine *eng)
{
    QScriptValue bar = eng->globalObject().property("bar");
    QScriptValue arguments = ctx->argumentsObject();
    qDebug() << "calling bar() with" << arguments.property("length").toInt32() << "arguments";
    QScriptValue result = bar.apply(ctx->thisObject(), arguments);
    qDebug() << "bar() returned" << result.toString();
    return result;
}
//! [69]

//! [72]
QScriptValue counter(QScriptContext *ctx, QScriptEngine *eng)
{
    QScriptValue act = ctx->activationObject();
    act.setProperty("count", 0);
    QScriptValue result = eng->newFunction(counter_inner);
    result.setScope(act);
    return result;
}
//! [72]

//! [73]
QScriptValue counter_inner(QScriptContext *ctx, QScriptEngine *eng)
{
    QScriptValue outerAct = ctx->callee().scope();
    double count = outerAct.property("count").toNumber();
    outerAct.setProperty("count", count+1);
    return count;
}
//! [73]

//! [74]
QScriptValue counter_hybrid(QScriptContext *ctx, QScriptEngine *eng)
{
    QScriptValue act = ctx->activationObject();
    act.setProperty("count", 0);
    return eng->evaluate("(function() { return count++; })");
}
//! [74]

//! [76]
QScriptValue Person_ctor(QScriptContext *ctx, QScriptEngine *eng)
{
    QScriptValue object;
    if (ctx->isCalledAsConstructor()) {
        object = ctx->thisObject();
    } else {
        object = eng->newObject();
        object.setPrototype(ctx->callee().property("prototype"));
    }
    object.setProperty("name", ctx->argument(0));
    return object;
}
//! [76]

//! [77]
QScriptContext *ctx = eng.pushContext();
QScriptValue act = ctx->activationObject();
act.setProperty("digit", 7);

qDebug() << eng.evaluate("digit + 1").toNumber(); // 8

eng.popContext();
//! [77]

//! [78]
QScriptValue getSet(QScriptContext *ctx, QScriptEngine *eng)
{
    QScriptValue obj = ctx->thisObject();
    QScriptValue data = obj.data();
    if (!data.isValid()) {
        data = eng->newObject();
        obj.setData(data);
    }
    QScriptValue result;
    if (ctx->argumentCount() == 1) {
        QString str = ctx->argument(0).toString();
        str.replace("Roberta", "Ken");
        result = str;
        data.setProperty("x", result);
    } else {
        result = data.property("x");
    }
    return result;
}
//! [78]

//! [79]
QScriptEngine eng;
QScriptValue obj = eng.newObject();
obj.setProperty("x", eng.newFunction(getSet),
                QScriptValue::PropertyGetter|QScriptValue::PropertySetter);
//! [79]

//! [91]
QScriptValue object = engine.evaluate("({ unitName: 'Celsius', toKelvin: function(x) { return x + 273; } })");
QScriptValue toKelvin = object.property("toKelvin");
QScriptValue result = toKelvin.call(object, QScriptValueList() << 100);
qDebug() << result.toNumber(); // 373
//! [91]

//! [92]
QScriptValue add = engine.globalObject().property("add");
qDebug() << add.call(QScriptValue(), QScriptValueList() << 1 << 2).toNumber(); // 3
//! [92]

//! [93]
typedef QSharedPointer<QXmlStreamReader> XmlStreamReaderPointer;

Q_DECLARE_METATYPE(XmlStreamReaderPointer)

QScriptValue constructXmlStreamReader(QScriptContext *context, QScriptEngine *engine)
{
    if (!context->isCalledAsConstructor())
        return context->throwError(QScriptContext::SyntaxError, "please use the 'new' operator");

    QIODevice *device = qobject_cast<QIODevice*>(context->argument(0).toQObject());
    if (!device)
        return context->throwError(QScriptContext::TypeError, "please supply a QIODevice as first argument");

    // Create the C++ object
    QXmlStreamReader *reader = new QXmlStreamReader(device);

    XmlStreamReaderPointer pointer(reader);

    // store the shared pointer in the script object that we are constructing
    return engine->newVariant(context->thisObject(), QVariant::fromValue(pointer));
}
//! [93]

//! [94]
QScriptValue xmlStreamReader_atEnd(QScriptContext *context, QScriptEngine *)
{
    XmlStreamReaderPointer reader = qscriptvalue_cast<XmlStreamReaderPointer>(context->thisObject());
    if (!reader)
        return context->throwError(QScriptContext::TypeError, "this object is not an XmlStreamReader");
    return reader->atEnd();
}
//! [94]

//! [95]
    QScriptEngine engine;
    QScriptValue xmlStreamReaderProto = engine.newObject();
    xmlStreamReaderProto.setProperty("atEnd", engine.newFunction(xmlStreamReader_atEnd));

    QScriptValue xmlStreamReaderCtor = engine.newFunction(constructXmlStreamReader, xmlStreamReaderProto);
    engine.globalObject().setProperty("XmlStreamReader", xmlStreamReaderCtor);
//! [95]
