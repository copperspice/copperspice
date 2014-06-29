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

import QtQuick 1.1
import "righttoleft"

Column {
    width: 200
//![0]
// automatically aligned to the left
Text {
    text: "Phone"
    width: 200
}

// automatically aligned to the right
Text {
    text: "خامل"
    width: 200
}

// aligned to the left
Text {
    text: "خامل"
    horizontalAlignment: Text.AlignLeft
    width: 200
}

// aligned to the right
Text {
    text: "خامل"
    horizontalAlignment: Text.AlignLeft
    LayoutMirroring.enabled: true
    width: 200
}
//![0]

//![1]
// by default child items are positioned from left to right
Row {
    Child {}
    Child {}
}

// position child items from right to left
Row {
    layoutDirection: Qt.RightToLeft
    Child {}
    Child {}
}

// position child items from left to right
Row {
    LayoutMirroring.enabled: true
    layoutDirection: Qt.RightToLeft
    Child {}
    Child {}
}
//![1]

//![2]
Item {
    height: 50; width: 150

    LayoutMirroring.enabled: true
    anchors.left: parent.left   // anchor left becomes right

    Row {
        // items flow from left to right (as per default)
        Child {}
        Child {}
        Child {}
    }
}
//![2]

//![3]
Item {
    height: 50; width: 150

    LayoutMirroring.enabled: true
    LayoutMirroring.childrenInherit: true
    anchors.left: parent.left   // anchor left becomes right

    Row {
        // setting childrenInherit in the parent causes these
        // items to flow from right to left instead
        Child {}
        Child {}
        Child {}
    }
}
//![3]

//![4]
Rectangle {
    color: "black"
    height: 50; width: 50
    x: mirror(10)
    function mirror(value) {
        return LayoutMirroring.enabled ? (parent.width - width - value) : value;
    }
}
//![4]

//![5]
Image {
    source: "arrow.png"
    mirror: true
}
//![5]
}
