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
//! [document]
import QtQuick 1.0


//! [parent begin]
Rectangle {
//! [parent begin]
    width: 200; height: 600
    id: screen

Column {
    spacing: 12
//! [direct property change]
Rectangle {
    id: blob
    width: 75; height: 75
    color: "blue"

    MouseArea {
        anchors.fill: parent
        onClicked: blob.color = "green"
    }
}
//! [direct property change]

//! [property animation]
Rectangle {
    id: flashingblob
    width: 75; height: 75
    color: "blue"
    opacity: 1.0

    MouseArea {
        anchors.fill: parent
        onClicked: {
            animateColor.start()
            animateOpacity.start()
        }
    }

    PropertyAnimation {id: animateColor; target: flashingblob; properties: "color"; to: "green"; duration: 100}

    NumberAnimation {
        id: animateOpacity
        target: flashingblob
        properties: "opacity"
        from: 0.99
        to: 1.0
        loops: Animation.Infinite
        easing {type: Easing.OutBack; overshoot: 500}
   }
}
//! [property animation]

//! [transition animation]
Rectangle {
    width: 75; height: 75
    id: button
    state: "RELEASED"

    MouseArea {
        anchors.fill: parent
        onPressed: button.state = "PRESSED"
        onReleased: button.state = "RELEASED"
    }

    states: [
        State {
            name: "PRESSED"
            PropertyChanges { target: button; color: "lightblue"}
        },
        State {
            name: "RELEASED"
            PropertyChanges { target: button; color: "lightsteelblue"}
        }
    ]

    transitions: [
        Transition {
            from: "PRESSED"
            to: "RELEASED"
            ColorAnimation { target: button; duration: 100}
        },
        Transition {
            from: "RELEASED"
            to: "PRESSED"
            ColorAnimation { target: button; duration: 100}
        }
    ]
}
//! [transition animation]

Rectangle {
    width: 75; height: 75
    id: wildcard
    color: "green"
//! [wildcard animation]
    transitions:
        Transition {
            to: "*"
            ColorAnimation { target: button; duration: 100}
        }
//! [wildcard animation]

    MouseArea {
        anchors.fill: parent
        onPressed: {
            ball.x = 10
            ball.color = "red"
        }
        onReleased: {
            ball.x = screen.width / 2
            ball.color = "salmon"
        }
    }
}

//! [behavior animation]
Rectangle {
    width: 75; height: 75; radius: width
    id: ball
    color: "salmon"

    Behavior on x {
        NumberAnimation {
            id: bouncebehavior
            easing {
                type: Easing.OutElastic
                amplitude: 1.0
                period: 0.5
            }
        }
    }
    Behavior on y {
        animation: bouncebehavior
    }
    Behavior {
        ColorAnimation { target: ball; duration: 100 }
    }
}
//! [behavior animation]

//! [sequential animation]
Rectangle {
    id: banner
    width: 150; height: 100; border.color: "black"

    Column {
        anchors.centerIn: parent
        Text {
            id: code
            text: "Code less."
            opacity: 0.01
        }
        Text {
            id: create
            text: "Create more."
            opacity: 0.01
        }
        Text {
            id: deploy
            text: "Deploy everywhere."
            opacity: 0.01
        }
    }

    MouseArea {
        anchors.fill: parent
        onPressed: playbanner.start()
    }

    SequentialAnimation {
        id: playbanner
        running: false
        NumberAnimation { target: code; property: "opacity"; to: 1.0; duration: 200}
        NumberAnimation { target: create; property: "opacity"; to: 1.0; duration: 200}
        NumberAnimation { target: deploy; property: "opacity"; to: 1.0; duration: 200}
    }
}
//! [sequential animation]

}//end of col
//! [parent end]
}
//! [parent end]

//! [document]
