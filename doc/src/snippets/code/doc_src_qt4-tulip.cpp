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
foreach (variable, container)
    statement;
//! [0]


//! [1]
QList<QString> list;
...
foreach (const QString &str, list)
    cout << str.ascii() << endl;
//! [1]


//! [2]
QString str;
foreach (str, list)
    cout << str.ascii() << endl;
//! [2]


//! [3]
// forward                                  // backward
QList<QString> list;                        QList<QString> list;
...                                         ...
QListIterator<QString> i(list);             QListIterator<QString> i(list);        
while (i.hasNext())                         i.toBack();                            
    cout << i.next().ascii() << endl;       while (i.hasPrev())                    
                                                cout << i.prev().ascii() << endl;
//! [3]


//! [4]
// forward                                  // backward
QMutableListIterator<int> i(list);          QMutableListIterator<int> i(list);  
while (i.hasNext())                         i.toBack();                         
    if (i.next() > 128)                     while (i.hasPrev())                 
        i.setValue(128);                        if (i.prev() > 128)             
                                                    i.setValue(128);           
//! [4]


//! [5]
// forward                                  // backward
QMutableListIterator<int> i(list);          QMutableListIterator<int> i(list);                 
while (i.hasNext())                         i.toBack();                         
    if (i.next() % 2 != 0)                  while (i.hasPrev())                        
        i.remove();                             if (i.prev() % 2 != 0)          
                                                    i.remove();                 
//! [5]


//! [6]
// STL-style                                // Java-style
QMap<int, QWidget *>::const_iterator i;     QMapIterator<int, QWidget *> i(map);
for (i = map.begin(); i != map.end(); ++i)  while (i.findNext(widget))
    if (i.value() == widget)                    cout << "Found widget " << widget
        cout << "Found widget " << widget            << " under key "
             << " under key "                        << i.key() << endl;
             << i.key() << endl;
//! [6]


//! [7]
// STL-style                                // Java-style
QList<int>::iterator i = list.begin();      QMutableListIterator<int> i(list);
while (i != list.end()) {                   while (i.hasNext()) {
    if (*i == 0) {                              int val = i.next();
        i = list.erase(i);                      if (val < 0)
    } else {                                        i.setValue(-val);
        if (*i < 0)                             else if (val == 0)
            *i = -*i;                               i.remove();
        ++i;                                }
    }
}
//! [7]


//! [8]
QList<double> list;
...
for (int i = 0; i < list.size(); ++i) {
    if (list[i] < 0.0)
        list[i] = 0.0;
}
//! [8]


//! [9]
QMap<QString, int> map;
...
map.value("TIMEOUT", 30);  // returns 30 if "TIMEOUT" isn't in the map
//! [9]


//! [10]
QMultiMap<QString, int> map;
...
QList<int> values = map.values("TIMEOUT");
//! [10]
