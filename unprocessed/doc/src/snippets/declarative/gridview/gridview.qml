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

//![import]
import QtQuick 1.0
//![import]

Rectangle {
    width: childrenRect.width; height: childrenRect.height

Row {

//![classdocs simple]
GridView {
    width: 300; height: 200

    model: ContactModel {}
    delegate: Column {
        Image { source: portrait; anchors.horizontalCenter: parent.horizontalCenter }
        Text { text: name; anchors.horizontalCenter: parent.horizontalCenter }
    }
}
//![classdocs simple]


//![classdocs advanced]
Rectangle {
    width: 300; height: 200

    Component {
        id: contactDelegate
        Item {
            width: grid.cellWidth; height: grid.cellHeight
            Column {
                anchors.fill: parent
                Image { source: portrait; anchors.horizontalCenter: parent.horizontalCenter }
                Text { text: name; anchors.horizontalCenter: parent.horizontalCenter }
            }
        }
    }

    GridView {
        id: grid
        anchors.fill: parent
        cellWidth: 80; cellHeight: 80

        model: ContactModel {}
        delegate: contactDelegate
        highlight: Rectangle { color: "lightsteelblue"; radius: 5 }
        focus: true
    }
}
//![classdocs advanced]

//![delayRemove]
Component {
    id: delegate
    Item {
        GridView.onRemove: SequentialAnimation {
            PropertyAction { target: wrapper; property: "GridView.delayRemove"; value: true }
            NumberAnimation { target: wrapper; property: "scale"; to: 0; duration: 250; easing.type: Easing.InOutQuad }
            PropertyAction { target: wrapper; property: "GridView.delayRemove"; value: false }
        }
    }
}
//![delayRemove]

//![highlightFollowsCurrentItem]
Component {
    id: highlight
    Rectangle {
        width: view.cellWidth; height: view.cellHeight
        color: "lightsteelblue"; radius: 5
        x: view.currentItem.x
        y: view.currentItem.y
        Behavior on x { SpringAnimation { spring: 3; damping: 0.2 } }
        Behavior on y { SpringAnimation { spring: 3; damping: 0.2 } }
    }
}

GridView {
    id: view
    width: 300; height: 200
    cellWidth: 80; cellHeight: 80

    model: ContactModel {}
    delegate: Column {
        Image { source: portrait; anchors.horizontalCenter: parent.horizontalCenter }
        Text { text: name; anchors.horizontalCenter: parent.horizontalCenter }
    }

    highlight: highlight
    highlightFollowsCurrentItem: false
    focus: true
}
//![highlightFollowsCurrentItem]

//![isCurrentItem]
GridView {
    width: 300; height: 200
    cellWidth: 80; cellHeight: 80

    Component {
        id: contactsDelegate
        Rectangle {
            id: wrapper
            width: 80
            height: 80
            color: GridView.isCurrentItem ? "black" : "red"
            Text {
                id: contactInfo
                text: name + ": " + number
                color: wrapper.GridView.isCurrentItem ? "red" : "black"
            }
        }
    }

    model: ContactModel {}
    delegate: contactsDelegate
    focus: true
}
//![isCurrentItem]

}

}
