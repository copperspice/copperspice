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

import QtQuick 1.0

Rectangle {
    width: 160; height: 250
    
    Image {
        width: 160; height: 200
        source: "pics/checker.svg"
        fillMode: Image.Tile

        //! [colors]
        Rectangle {
            color: "steelblue"
            width: 40; height: 40
        }
        Rectangle {
            color: "transparent"
            y: 40; width: 40; height: 40
        }
        Rectangle {
            color: "#FF0000"
            y: 80; width: 40; height: 40
        }
        Rectangle {
            color: "#800000FF"
            y: 120; width: 40; height: 40
        }
        Rectangle {
            color: "#00000000"    // ARGB fully transparent
            y: 160
            width: 40; height: 40
        }
        //! [colors]

        Rectangle {
            x: 40
            width: 120; height: 200

            Text {
                font.pixelSize: 16
                text: "steelblue"
                x: 10; height: 40
                verticalAlignment: Text.AlignVCenter
            }
            Text {
                font.pixelSize: 16
                text: "transparent"
                x: 10; y: 40; height: 40
                verticalAlignment: Text.AlignVCenter
            }
            Text {
                font.pixelSize: 16
                text: "FF0000"
                x: 10; y: 80; height: 40
                verticalAlignment: Text.AlignVCenter
            }
            Text {
                font.pixelSize: 16
                text: "800000FF"
                x: 10; y: 120; height: 40
                verticalAlignment: Text.AlignVCenter
            }
            Text {
                font.pixelSize: 16
                text: "00000000"
                x: 10; y: 160; height: 40
                verticalAlignment: Text.AlignVCenter
            }
        }
    }

    Image {
        y: 210
        width: 40; height: 40
        source: "pics/checker.svg"
        fillMode: Image.Tile
    }

    Text {
        font.pixelSize: 16
        text: "(background)"
        x: 50; y: 210; height: 40
        verticalAlignment: Text.AlignVCenter
    }
}
