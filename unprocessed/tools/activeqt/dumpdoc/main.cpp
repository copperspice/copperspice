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

#include <QAxObject>
#include <QFile>
#include <QTextStream>
#include <qt_windows.h>

QT_USE_NAMESPACE

int main(int argc, char **argv)
{
    CoInitialize(0);

    enum State {
        Default = 0,
            OutOption
    } state;
    state = Default;
    
    QByteArray outname;
    QByteArray object;
    
    for (int a = 1; a < argc; ++a) {
        QByteArray arg(argv[a]);
        const char first = arg[0];
        switch(state) {
        case Default:
            if (first == '-' || first == '/') {
                arg = arg.mid(1);
                arg.toLower();
                if (arg == "o")
                    state = OutOption;
                else if (arg == "v") {
                    qWarning("dumpdoc: Version 1.0");
                    return 0;
                } else if (arg == "h") {
                    qWarning("dumpdoc Usage:\n\tdumpdoc object [-o <file>]"
                        "              \n\tobject   : object[/subobject]*"
                        "              \n\tsubobject: property\n"
                        "      \nexample:\n\tdumpdoc Outlook.Application/Session/CurrentUser -o outlook.html");
                    return 0;
                }
            } else {
                object = arg;
            }
            break;
        case OutOption:
            outname = arg;
            state = Default;
            break;
            
        default:
            break;
        }
    }
    
    if (object.isEmpty()) {
        qWarning("dumpdoc: No object name provided.\n"
            "         Use -h for help.");
        return -1;
    }
    QFile outfile;
    if (!outname.isEmpty()) {
        outfile.setFileName(QString::fromLatin1(outname.constData()));
        if (!outfile.open(QIODevice::WriteOnly | QIODevice::Text)) {
            qWarning("dumpdoc: Could not open output file '%s'", outname.data());
        }
    } else {
        outfile.open(stdout, QIODevice::WriteOnly);
    }
    QTextStream out(&outfile);
    
    QByteArray subobject = object;
    int index = subobject.indexOf('/');
    if (index != -1)
        subobject = subobject.left(index);
    
    QAxObject topobject(QString::fromLatin1(subobject.constData()));
    
    if (topobject.isNull()) {
        qWarning("dumpdoc: Could not instantiate COM object '%s'", subobject.data());
        return -2;
    }
    
    QAxObject *axobject = &topobject;
    while (index != -1 && axobject) {
        index++;
        subobject = object.mid(index);
        if (object.indexOf('/', index) != -1) {
            int oldindex = index;
            index = object.indexOf('/', index);
            subobject = object.mid(oldindex, index-oldindex);	    
        } else {
            index = -1;
        }
        
        axobject = axobject->querySubObject(subobject);
    }
    if (!axobject || axobject->isNull()) {
        qWarning("dumpdoc: Subobject '%s' does not exist in '%s'", subobject.data(), object.data());
        return -3;
    }
    
    QString docu = axobject->generateDocumentation();
    out << docu;
    return 0;
}
