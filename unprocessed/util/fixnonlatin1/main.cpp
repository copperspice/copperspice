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

// Scans files for characters >127 and replaces them with the \nnn octal representation

int main(int argc, char *argv[])
{
    if (argc <= 1)
        qFatal("Usage: %s FILES", argc ? argv[0] : "fixnonlatin1");
    for (int i = 1; i < argc; ++i) {

        QString fileName = QString::fromLocal8Bit(argv[i]);
        if (   fileName.endsWith(".gif")
            || fileName.endsWith(".jpg")
            || fileName.endsWith(".tif")
            || fileName.endsWith(".tiff")
            || fileName.endsWith(".png")
            || fileName.endsWith(".mng")
            || fileName.endsWith(".ico")
            || fileName.endsWith(".zip")
            || fileName.endsWith(".gz")
            || fileName.endsWith(".qpf")
            || fileName.endsWith(".ttf")
            || fileName.endsWith(".pfb")
            || fileName.endsWith(".exe")
            || fileName.endsWith(".nib")
            || fileName.endsWith(".o")
            )
            continue;

        QFile file(fileName);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
            qFatal("Cannot open '%s': %s", argv[i], qPrintable(file.errorString()));

        QByteArray ba = file.readAll();
        bool mod = false;
        for (int j = 0; j < ba.count(); ++j) {
            uchar c = ba.at(j);
            if (c > 127) {
                ba[j] = '\\';
                ba.insert(j + 1, QByteArray::number(c, 8).rightJustified(3, '0', true));
                j += 3;
                mod = true;
            }
        }
        file.close();

        if (!mod)
            continue;

        qWarning("found non-latin1 characters in '%s'", argv[i]);
        if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            qWarning("Cannot open '%s' for writing: %s", argv[i], qPrintable(file.errorString()));
            continue;
        }
        if (file.write(ba) < 0)
            qFatal("Error while writing into '%s': %s", argv[i], qPrintable(file.errorString()));
        file.close();
    }

    return 0;
}

