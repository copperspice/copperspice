/***********************************************************************
*
* Copyright (c) 2012-2018 Barbara Geller
* Copyright (c) 2012-2018 Ansel Sermersheim
* Copyright (c) 2012-2016 Digia Plc and/or its subsidiary(-ies).
* Copyright (c) 2008-2012 Nokia Corporation and/or its subsidiary(-ies).
* All rights reserved.
*
* This file is part of CopperSpice.
*
* CopperSpice is free software. You can redistribute it and/or
* modify it under the terms of the GNU Lesser General Public License
* version 2.1 as published by the Free Software Foundation.
*
* CopperSpice is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
*
* <http://www.gnu.org/licenses/>.
*
***********************************************************************/

#include <QtCore/QtCore>

#include <stdio.h>

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    const QStringList args = app.arguments().mid(1);
    if (args.isEmpty()) {
        printf("Usage: ./plugintest libplugin.so...\nThis tool loads a plugin and displays whether QPluginLoader could load it or not.\nIf the plugin could not be loaded, it'll display the error string.\n");
        return 1;
    }

    foreach (QString plugin, args) {
        printf("%s: ", qPrintable(plugin));
        QPluginLoader loader(plugin);
        if (loader.load())
            printf("success!\n");
        else
            printf("failure: %s\n", qPrintable(loader.errorString()));
    }

    return 0;
}

