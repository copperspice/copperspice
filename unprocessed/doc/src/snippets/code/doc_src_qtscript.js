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

//! [2]
function myInterestingScriptFunction() {
    // ...
}
// ...
myQObject.somethingChanged.connect(myInterestingScriptFunction);
//! [2]


//! [3]
myQObject.somethingChanged.connect(myOtherQObject.doSomething);
//! [3]


//! [4]
myQObject.somethingChanged.disconnect(myInterestingFunction);
myQObject.somethingChanged.disconnect(myOtherQObject.doSomething);
//! [4]


//! [5]
var obj = { x: 123 };
var fun = function() { print(this.x); };
myQObject.somethingChanged.connect(obj, fun);
//! [5]


//! [6]
myQObject.somethingChanged.disconnect(obj, fun);
//! [6]


//! [7]
var obj = { x: 123, fun: function() { print(this.x); } };
myQObject.somethingChanged.connect(obj, "fun");
//! [7]


//! [8]
myQObject.somethingChanged.disconnect(obj, "fun");
//! [8]


//! [9]
try {
    myQObject.somethingChanged.connect(myQObject, "slotThatDoesntExist");
} catch (e) {
    print(e);
}
//! [9]


//! [10]
myQObject.somethingChanged("hello");
//! [10]


//! [11]
myQObject.myOverloadedSlot(10);   // will call the int overload
myQObject.myOverloadedSlot("10"); // will call the QString overload
//! [11]


//! [12]
myQObject['myOverloadedSlot(int)']("10");   // call int overload; the argument is converted to an int
myQObject['myOverloadedSlot(QString)'](10); // call QString overload; the argument is converted to a string
//! [12]


//! [14]
myQObject.enabled = true;

// ...

myQObject.enabled = !myQObject.enabled;
//! [14]


//! [15]
myDialog.okButton
//! [15]


//! [16]
myDialog.okButton.objectName = "cancelButton";
// from now on, myDialog.cancelButton references the button
//! [16]


//! [17]
var okButton = myDialog.findChild("okButton");
if (okButton != null) {
   // do something with the OK button
}

var buttons = myDialog.findChildren(RegExp("button[0-9]+"));
for (var i = 0; i < buttons.length; ++i) {
   // do something with buttons[i]
}
//! [17]


//! [21]
var obj = new MyObject;
obj.setEnabled( true );
print( "obj is enabled: " + obj.isEnabled() );
//! [21]


//! [22]
var obj = new MyObject;
obj.enabled = true;
print( "obj is enabled: " + obj.enabled );
//! [22]


//! [26]
function enabledChangedHandler( b )
{
    print( "state changed to: " + b );
}

function init()
{
    var obj = new MyObject();
    // connect a script function to the signal
    obj["enabledChanged(bool)"].connect(enabledChangedHandler);
    obj.enabled = true;
    print( "obj is enabled: " + obj.enabled );
}
//! [26]


//! [27]
var o = new Object();
o.foo = 123;
print(o.hasOwnProperty('foo')); // true
print(o.hasOwnProperty('bar')); // false
print(o); // calls o.toString(), which returns "[object Object]"
//! [27]


//! [28]
function Person(name)
{
  this.name = name;
}
//! [28]


//! [29]
Person.prototype.toString = function() { return "Person(name: " + this.name + ")"; }
//! [29]


//! [30]
var p1 = new Person("John Doe");
var p2 = new Person("G.I. Jane");
print(p1); // "Person(name: John Doe)"
print(p2); // "Person(name: G.I. Jane)"
//! [30]


//! [31]
print(p1.hasOwnProperty('name')); // 'name' is an instance variable, so this returns true
print(p1.hasOwnProperty('toString')); // returns false; inherited from prototype
print(p1 instanceof Person); // true
print(p1 instanceof Object); // true
//! [31]


//! [32]
function Employee(name, salary)
{
  Person.call(this, name); // call base constructor

  this.salary = salary;
}

// set the prototype to be an instance of the base class
Employee.prototype = new Person();

// initialize prototype
Employee.prototype.toString = function() {
    // ...
}
//! [32]


//! [33]
var e = new Employee("Johnny Bravo", 5000000);
print(e instanceof Employee); // true
print(e instanceof Person);   // true
print(e instanceof Object);   // true
print(e instanceof Array);    // false
//! [33]


//! [40]
var o = new Object();
(o.__proto__ === Object.prototype); // this evaluates to true
//! [40]


//! [41]
var o = new Object();
o.__defineGetter__("x", function() { return 123; });
var y = o.x; // 123
//! [41]


//! [42]
var o = new Object();
o.__defineSetter__("x", function(v) { print("and the value is:", v); });
o.x = 123; // will print "and the value is: 123"
//! [42]


//! [49]
var getProperty = function(name) { return this[name]; };

name = "Global Object"; // creates a global variable
print(getProperty("name")); // "Global Object"

var myObject = { name: 'My Object' };
print(getProperty.call(myObject, "name")); // "My Object"

myObject.getProperty = getProperty;
print(myObject.getProperty("name")); // "My Object"

getProperty.name = "The getProperty() function";
getProperty.getProperty = getProperty;
getProperty.getProperty("name"); // "The getProperty() function"
//! [49]

//! [50]
var o = { a: 1, b: 2, sum: function() { return a + b; } };
print(o.sum()); // reference error, or sum of global variables a and b!!
//! [50]

//! [51]
var o = { a: 1, b: 2, sum: function() { return this.a + this.b; } };
print(o.sum()); // 3
//! [51]

//! [56]
function add(a, b) {
    return a + b;
}
//! [56]

//! [57]
function add() {
    return arguments[0] + arguments[1];
}
//! [57]

//! [59]
function add() {
    if (arguments.length != 2)
        throw Error("add() takes exactly two arguments");
    return arguments[0] + arguments[1];
}
//! [59]

//! [60]
function add() {
    if (arguments.length != 2)
        throw Error("add() takes exactly two arguments");
    if (typeof arguments[0] != "number")
        throw TypeError("add(): first argument is not a number");
    if (typeof arguments[1] != "number")
        throw TypeError("add(): second argument is not a number");
    return arguments[0] + arguments[1];
}
//! [60]

//! [61]
function add() {
    if (arguments.length != 2)
        throw Error("add() takes exactly two arguments");
    return Number(arguments[0]) + Number(arguments[1]);
}
//! [61]

//! [64]
function concat() {
    var result = "";
    for (var i = 0; i < arguments.length; ++i)
        result += String(arguments[i]);
    return result;
}
//! [64]

//! [66]
function sort(comparefn) {
    if (comparefn == undefined)
        comparefn = fn; /* replace fn with the built-in comparison function */
    else if (typeof comparefn != "function")
        throw TypeError("sort(): argument must be a function");
    // ...
}
//! [66]

//! [68]
function foo() {
    // Let bar() take care of this.
    print("calling bar() with " + arguments.length + "arguments");
    var result = bar.apply(this, arguments);
    print("bar() returned" + result);
    return result;
}
//! [68]

//! [70]
function counter() {
    var count = 0;
    return function() {
        return count++;
    }
}
//! [70]

//! [71]
var c1 = counter(); // create a new counter function
var c2 = counter(); // create a new counter function
print(c1()); // 0
print(c1()); // 1
print(c2()); // 0
print(c2()); // 1
//! [71]

//! [75]
function Book(isbn) {
    this.isbn = isbn;
}

var coolBook1 = new Book("978-0131872493");
var coolBook2 = new Book("978-1593271473");
//! [75]

//! [80]
obj.x = "Roberta sent me";
print(obj.x); // "Ken sent me"
obj.x = "I sent the bill to Roberta";
print(obj.x); // "I sent the bill to Ken"
//! [80]

//! [81]
obj = {};
obj.__defineGetter__("x", function() { return this._x; });
obj.__defineSetter__("x", function(v) { print("setting x to", v); this._x = v; });
obj.x = 123;
//! [81]

//! [82]
myButton.text = qsTr("Hello world!");
//! [82]

//! [83]
myButton.text = qsTranslate("MyAwesomeScript", "Hello world!");
//! [83]

//! [84]
FriendlyConversation.prototype.greeting = function(type)
{
    if (FriendlyConversation['greeting_strings'] == undefined) {
        FriendlyConversation['greeting_strings'] = [
            QT_TR_NOOP("Hello"),
            QT_TR_NOOP("Goodbye")
        ];
    }
    return qsTr(FriendlyConversation.greeting_strings[type]);
}
//! [84]

//! [85]
FriendlyConversation.prototype.greeting = function(type)
{
    if (FriendlyConversation['greeting_strings'] == undefined) {
        FriendlyConversation['greeting_strings'] = [
            QT_TRANSLATE_NOOP("FriendlyConversation", "Hello"),
            QT_TRANSLATE_NOOP("FriendlyConversation", "Goodbye")
        ];
    }
    return qsTranslate("FriendlyConversation", FriendlyConversation.greeting_strings[type]);
}
//! [85]

//! [86]
FileCopier.prototype.showProgress = function(done, total, currentFileName)
{
    this.label.text = qsTr("%1 of %2 files copied.\nCopying: %3")
                      .arg(done)
                      .arg(total)
                      .arg(currentFileName);
}
//! [86]

//! [90]
({ unitName: "Celsius",
   toKelvin: function(x) { return x + 273; }
 })
//! [90]
