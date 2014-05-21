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
from PyQt4.QtCore import QDir, Qt
from PyQt4.QtGui import *

app = QApplication(sys.argv)

background = QWidget()
palette = QPalette()
palette.setColor(QPalette.Window, QColor(Qt.white))
background.setPalette(palette)

model = QFileSystemModel()
model.setRootPath(QDir.currentPath())

treeView = QTreeView(background)
treeView.setModel(model)
treeView.setRootIndex(model.index(QDir.currentPath()))

listView = QListView(background)
listView.setModel(model)
listView.setRootIndex(model.index(QDir.currentPath()))

tableView = QTableView(background)
tableView.setModel(model)
tableView.setRootIndex(model.index(QDir.currentPath()))

selection = QItemSelectionModel(model)
treeView.setSelectionModel(selection)
listView.setSelectionModel(selection)
tableView.setSelectionModel(selection)

layout = QHBoxLayout(background)
layout.addWidget(listView)
layout.addSpacing(24)
layout.addWidget(treeView, 1)
layout.addSpacing(24)
layout.addWidget(tableView)
background.show()

sys.exit(app.exec_())
