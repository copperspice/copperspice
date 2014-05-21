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
//![document]
import QtQuick 1.0

//![parent begin]
Rectangle {
//![parent begin]

    id: screen
    width: 400; height: 500

//! [signal declaration]
    signal trigger
    signal send (string notice)
    signal perform (string task, variant object)
//! [signal declaration]

//! [signal handler declaration]
onTrigger: console.log("trigger signal emitted")

onSend: {
    console.log("send signal emitted with notice: " + notice)
}

onPerform: console.log("perform signal emitted")
//! [signal handler declaration]

//! [automatic signals]
Rectangle {
    id: sprite
    width: 25; height: 25
    x: 50; y: 15

    onXChanged: console.log("x property changed, emitted xChanged signal")
    onYChanged: console.log("y property changed, emitted yChanged signal")
}
//! [automatic signals]

//! [signal emit]
Rectangle {
    id: messenger

    signal send( string person, string notice)

    onSend: {
        console.log("For " + person + ", the notice is: " + notice)
    }

    Component.onCompleted: messenger.send("Tom", "the door is ajar.")
}
//! [signal emit]

//! [connect method]
Rectangle {
    id: relay

    signal send( string person, string notice)
    onSend: console.log("Send signal to: " + person + ", " + notice)

    Component.onCompleted: {
        relay.send.connect(sendToPost)
        relay.send.connect(sendToTelegraph)
        relay.send.connect(sendToEmail)
        relay.send("Tom", "Happy Birthday")
    }

    function sendToPost(person, notice) {
        console.log("Sending to post: " + person + ", " + notice)
    }
    function sendToTelegraph(person, notice) {
        console.log("Sending to telegraph: " + person + ", " + notice)
    }
    function sendToEmail(person, notice) {
        console.log("Sending to email: " + person + ", " + notice)
    }
}
//! [connect method]

//! [forward signal]
Rectangle {
    id: forwarder
    width: 100; height: 100

    signal send()
    onSend: console.log("Send clicked")

    MouseArea {
        id: mousearea
        anchors.fill: parent
        onClicked: console.log("MouseArea clicked")
    }
    Component.onCompleted: {
        mousearea.clicked.connect(send)
    }
}
//! [forward signal]

//! [connect method]
//![parent end]
}
//![parent end]

//![document]
