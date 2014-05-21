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
void MyButton::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    drawButton(&painter);
}
//! [0]


//! [1]
ba.at(0) = 'X';
//! [1]


//! [2]
ba[0] = 'X';
//! [2]


//! [3]
if (!cache.insert(key, object))
    delete object;
//! [3]


//! [4]
cache.insert(key, object);
//! [4]


//! [5]
Q3Cache<QWidget> cache;
cache.insert(widget->name(), widget);
...
QWidget *foo = cache.take("foo");
if (foo)
    foo->show();
//! [5]


//! [6]
typedef QWidget *QWidgetPtr;
QCache<QString, QWidgetPtr> cache;
cache.insert(widget->name(), new QWidgetPtr(widget));
...
QWidgetPtr *ptr = cache.take("foo");
if (ptr) {
    QWidget *foo = *ptr;
    delete ptr;
    foo->show();
}
//! [6]


//! [7]
painter.setBrush(palette().brush(QPalette::Text));
//! [7]


//! [8]
QByteArray ba("Hello");
ba.size();             // returns 5 (the '\0' is not counted)
ba.length();           // returns 5
ba.data()[5];          // returns '\0'
//! [8]


//! [9]
dict.replace(key, value);
//! [9]


//! [10]
delete hash.take(key);
hash.insert(key, value);
//! [10]


//! [11]
dict.remove(key, value);
//! [11]


//! [12]
delete hash.take(key);
//! [12]


//! [13]
dict.clear();
//! [13]


//! [14]
while (!hash.isEmpty()) {
    T *value = *hash.begin();
    hash.erase(hash.begin());
    delete value;
}
//! [14]


//! [15]
qDeleteAll(hash);
hash.clear();
//! [15]


//! [16]
Q3DictIterator<QWidget> i(dict);
while (i.current() != 0) {
    do_something(i.currentKey(), i.current());
    ++i;
}
//! [16]


//! [17]
QHashIterator<QString, QWidget *> i(hash);
while (i.hasNext()) {
    i.next();                   // must come first
    do_something(i.key(), i.value());
}
//! [17]


//! [18]
QList<QWidget *> myWidgets = myParent->findChildren<QWidget *>();
//! [18]


//! [19]
list.replace(index, value);
//! [19]


//! [20]
delete list[index];
list[index] = value;
//! [20]


//! [21]
list.removeFirst();
//! [21]


//! [22]
delete list.takeFirst();
//! [22]


//! [23]
list.removeLast();
//! [23]


//! [24]
delete list.takeLast();
//! [24]


//! [25]
list.remove(index);
//! [25]


//! [26]
delete list.takeAt(index);
//! [26]


//! [27]
list.remove(value);
//! [27]


//! [28]
int i = list.indexOf(value);
if (i != -1)
    delete list.takeAt(i);
//! [28]


//! [29]
list.remove();
//! [29]


//! [30]
QMutableListIterator<T *> i;
...
delete i.value();
i.remove();
//! [30]


//! [31]
list.clear();
//! [31]


//! [32]
while (!list.isEmpty())
    delete list.takeFirst();
//! [32]


//! [33]
qDeleteAll(list);
list.clear();
//! [33]


//! [34]
QPtrList<QWidget> list;
...
while (list.current() != 0) {
    do_something(list.current());
    list.next();
}
//! [34]


//! [35]
QList<QWidget *> list;
...
QListIterator<QWidget *> i(list);
while (i.hasNext())
    do_something(i.next());
//! [35]


//! [36]
QPtrList<QWidget> list;
...
QPtrListIterator<QWidget> i;
while (i.current() != 0) {
    do_something(i.current());
    i.next();
}
//! [36]


//! [37]
QList<QWidget *> list;
...
QListIterator<QWidget *> i(list);
while (i.hasNext())
    do_something(i.next());
//! [37]


//! [38]
queue.dequeue();
//! [38]


//! [39]
delete queue.dequeue();
//! [39]


//! [40]
queue.remove();
//! [40]


//! [41]
delete queue.dequeue();
//! [41]


//! [42]
queue.clear();
//! [42]


//! [43]
while (!queue.isEmpty())
    delete queue.dequeue();
//! [43]


//! [44]
qDeleteAll(queue);
queue.clear();
//! [44]


//! [45]
stack.pop();
//! [45]


//! [46]
delete stack.pop();
//! [46]


//! [47]
stack.remove();
//! [47]


//! [48]
delete stack.pop();
//! [48]


//! [49]
stack.clear();
//! [49]


//! [50]
while (!stack.isEmpty())
    delete stack.pop();
//! [50]


//! [51]
qDeleteAll(stack);
stack.clear();
//! [51]


//! [52]
vect.insert(i, ptr);
//! [52]


//! [53]
delete vect[i];
vect[i] = ptr;
//! [53]


//! [54]
vect.remove(i);
//! [54]


//! [55]
delete vect[i];
vect[i] = 0;
//! [55]


//! [56]
T *ptr = vect.take(i);
//! [56]


//! [57]
T *ptr = vect[i];
vect[i] = 0;
//! [57]


//! [58]
vect.resize(n)
//! [58]


//! [59]
while (n > vect.size())
    vect.append(0);
while (n < vect.size() {
    T *ptr = vect.last();
    vect.remove(vect.size() - 1);
    delete ptr;
}
//! [59]


//! [60]
vect.clear();
//! [60]


//! [61]
for (int i = 0; i < vect.size(); ++i)
    T *ptr = vect[i];
    vect[i] = 0;
    delete ptr;
}
//! [61]


//! [62]
qDeleteAll(vect);
vect.clear();
//! [62]


//! [63]
struct Shared
{
    Shared() : count(1) {}
    void ref() { ++count; }
    bool deref() { return !--count; }
    uint count;
};
//! [63]

//! [63a]
// Declare the object
QSimpleRichText richText(text, font);

// Set the width of the paragraph to w
richText.setWidth(w);

// Or set a reasonable default size
richText.adjustSize();

// Query for its used size
int width = richText.widthUsed();
int height = richText.height();

// Draw
richText.draw(painter, x, y, clipRect, colorGroup);
//! [63a]


//! [63b]
// Declare the object
QTextDocument doc;

// If text is rich text, use setHtml()
doc.setHtml(text);

// Otherwise, use setPlainText()
doc.setPlainText(text);

// Set the width of the paragraph of text to w
doc.setTextWidth(w);

// Query for the used size
int width = doc.idealWidth();
int height = doc.size().height();

// Draw
painter.translate(x, y);
doc.drawContents(painter, clipRect);

// If you have a palette/colorgroup you can draw using lower-level functions:
QAbstractTextDocumentLayout::PaintContext context;
context.palette = myPalette;
doc.documentLayout()->draw(painter, context);
//! [63b]

//! [63c]
QSlider *slider;
slider->style()->subControlRect(CC_Slider, sliderOption, SC_SliderHandle, slider);
//! [63c]

//! [64]
QString greeting = "Hello";
const char *badData = greeting.toAscii().constData(); // data is invalid
QByteArray asciiData = greeting.toAscii();
const char *goodData = asciiData.constData();
//! [64]


//! [65]
str.at(0) = 'X';
//! [65]


//! [66]
str[0] = 'X';
//! [66]
