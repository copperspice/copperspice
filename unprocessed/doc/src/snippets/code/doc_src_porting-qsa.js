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
point = new Object();
point.x = 12;
point.y = 35;
//! [0]


//! [1]
function manhattanLength(point) {  
    return point.x + point.y;
}
//! [1]


//! [2]
manhattanLength = function(point) {
   return point.x + point.y;
}
//! [2]


//! [3]
point.manhattanLength = function() {
    return this.x + this.y;
}
print(point.manhattanLength()); // prints 47
//! [3]


//! [5]
point.manhattanLength = function() {
    return this.x + this.y;
}
print(point.manhattanLength()); // prints 47
//! [5]


//! [8]
var car = new Object();
car.constructor = function(regnr) {
    // ...
}
car.constructor();
//! [8]


//! [10]
function Car(regnr) {
    this.regNumber = regnr;
    this.toString = function() { return this.regNumber; }
}
//! [10]


//! [11]
function Car(regnr) {
    this.regNumber = regnr;
}
Car.prototype.toString = function() { return this.regNumber; }
//! [11]


//! [13]
function GasolineCar(regnr) {
    Car(regnr); 
}
GasolineCar.prototype = new Car();
GasolineCar.prototype.toString = function() { 
    return "GasolineCar(" + this.regNumber + ")"; 
}
//! [13]


//! [15]
Car.globalCount = 0;
print(Car.globalCount);
//! [15]
