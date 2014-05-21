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
//contents of focusbutton.qml
import QtQuick 1.0

//! [parent begin]
FocusScope {
//! [parent begin]

    //! [expose visuals]
    //FocusScope needs to bind to visual properties of the children
    property alias color: button.color
    x: button.x; y: button.y
    width: button.width; height: button.height
    //! [expose visuals]

    //! [rectangle begin]
    Rectangle {
    //! [rectangle begin]
        id: button
    //! [properties]
        width: 145; height: 60
        color: "blue"
        smooth: true; radius: 9
        property alias text: label.text
    //! [properties]
        border {color: "#B9C5D0"; width: 1}

        gradient: Gradient {
            GradientStop {color: "#CFF7FF"; position: 0.0}
            GradientStop {color: "#99C0E5"; position: 0.57}
            GradientStop {color: "#719FCB"; position: 0.9}
        }

        Text {
            id: label
            anchors.centerIn: parent
            text: "Click Me!"
            font.pointSize: 12
            color: "blue"
        }

        MouseArea {
            anchors.fill: parent
            onClicked: console.log(text + " clicked")
        }
    //! [rectangle end]
    }
    //! [rectangle end]
//! [parent end]
}
//! [parent end]

//! [document]

//! [ellipses]
    //...
//! [ellipses]


