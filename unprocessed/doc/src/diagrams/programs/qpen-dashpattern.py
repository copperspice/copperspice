#!/usr/bin/env python
#############################################################################
##
## Copyright (C) 2012 Nokia Corporation and/or its subsidiary(-ies).
## All rights reserved.
## Contact: Nokia Corporation (qt-info@nokia.com)
##
## This file is part of the documentation of the Qt Toolkit.
##
## $QT_BEGIN_LICENSE:LGPL$
## GNU Lesser General Public License Usage
## This file may be used under the terms of the GNU Lesser General Public
## License version 2.1 as published by the Free Software Foundation and
## appearing in the file LICENSE.LGPL included in the packaging of this
## file. Please review the following information to ensure the GNU Lesser
## General Public License version 2.1 requirements will be met:
## http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
##
## In addition, as a special exception, Nokia gives you certain additional
## rights. These rights are described in the Nokia Qt LGPL Exception
## version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
##
## GNU General Public License Usage
## Alternatively, this file may be used under the terms of the GNU General
## Public License version 3.0 as published by the Free Software Foundation
## and appearing in the file LICENSE.GPL included in the packaging of this
## file. Please review the following information to ensure the GNU General
## Public License version 3.0 requirements will be met:
## http://www.gnu.org/copyleft/gpl.html.
##
## Other Usage
## Alternatively, this file may be used in accordance with the terms and
## conditions contained in a signed written agreement between you and Nokia.
##
##
##
##
##
## $QT_END_LICENSE$
##
#############################################################################

import sys
from PyQt4.QtCore import *
from PyQt4.QtGui import *
from PyQt4.QtSvg import QSvgGenerator

if __name__ == "__main__":

    app = QApplication(sys.argv)
    
    #device = QSvgGenerator()
    #device.setFileName("qpen-dashpattern.svg")
    #device.setSize(QSize(216, 144))
    #device.setResolution(72)
    
    device = QImage(192, 144, QImage.Format_ARGB32)
    device.fill(qRgba(0, 0, 0, 0))
    
    #resolution = device.resolution() # dpi
    #dpp = resolution / 72.0
    
    p = QPainter()
    p.begin(device)
    
    width = 8
    
    pen = QPen()
    pen.setWidth(width)
    pen.setDashPattern([4, 2])
    pen.setCapStyle(Qt.FlatCap)
    
    faded_pen = QPen()
    faded_pen.setWidth(width)
    faded_pen.setDashPattern([4, 2])
    faded_pen.setColor(QColor(160, 160, 160))
    faded_pen.setCapStyle(Qt.FlatCap)
    
    font = QFont("Monospace")
    font.setPointSize(12)
    p.setFont(font)
    p.setBrush(QColor(160, 0, 0))
    
    for x in range(-6, 9):
    
        if x % 4 == 0:
            length = 6
        else:
            length = 2
        
        p.drawLine(64 + x * width, 4, 64 + x * width, 4 + length)
        p.drawLine(64 + x * width, 136, 64 + x * width, 136 - length)
    
    offsets = (0, 2, 3.5, 4, 5, 6)
    for i in range(len(offsets)):
    
        offset = offsets[i]
        pen.setDashOffset(offset)
        
        p.setPen(faded_pen)
        p.drawLine(64 - offset * width, 20 + (i * 20), 64, 20 + (i * 20))
        
        p.setPen(pen)
        p.drawLine(64, 20 + (i * 20), 128, 20 + (i * 20))
        
        p.drawText(150, 25 + (i * 20), str(offset))
    
    p.end()
    device.save("qpen-dashpattern.png")
    sys.exit()
