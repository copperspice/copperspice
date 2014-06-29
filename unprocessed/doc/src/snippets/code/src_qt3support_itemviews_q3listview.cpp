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
(void) new Q3ListViewItem(listView, "Column 1", "Column 2");
(void) new Q3ListViewItem(listView->firstChild(), "A", "B", "C");
//! [0]


//! [1]
Q3ListViewItem * myChild = myItem->firstChild();
while(myChild) {
    doSomething(myChild);
    myChild = myChild->nextSibling();
}
//! [1]


//! [2]
Q3ListViewItemIterator it(listview);
while (it.current()) {
    Q3ListViewItem *item = it.current();
    doSomething(item);
    ++it;
}
//! [2]


//! [3]
int MyListViewItem::compare(Q3ListViewItem *i, int col,
                             bool ascending) const
{
    return key(col, ascending).compare(i->key(col, ascending));
}
//! [3]


//! [4]
Q3ListViewItem *i = itemAt(p);
if (i) {
    if (p.x() > header()->sectionPos(header()->mapToIndex(0)) +
            treeStepSize() * (i->depth() + (rootIsDecorated() ? 1 : 0)) + itemMargin() ||
            p.x() < header()->sectionPos(header()->mapToIndex(0))) {
        ; // p is not on root decoration
    else
        ; // p is on the root decoration
}
//! [4]


//! [5]
QRect r(listView->itemRect(item));
r.setHeight(qMin(item->totalHeight(),
                 listView->viewport->height() - r.y()))
//! [5]


//! [6]
QList<Q3ListViewItem *> lst;
Q3ListViewItemIterator it(myListView);
while (it.current()) {
    if (it.current()->isSelected())
        lst.append(it.current());
    ++it;
}
//! [6]


//! [7]
QList<Q3ListViewItem *> lst;
Q3ListViewItemIterator it(myListView, Q3ListViewItemIterator::Selected);
while (it.current()) {
    lst.append(it.current());
    ++it;
}
//! [7]
