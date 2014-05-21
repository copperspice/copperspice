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

#include <QCoreApplication>
#include <QFile>
#include <QHash>
#include <QPair>
#include <QStringList>
#include <QTextStream>
#include <QXmlStreamReader>

/*
    This class exists for the sole purpose of creating a translation context.
 */
class PrettyPrint
{
    Q_DECLARE_TR_FUNCTIONS(PrettyPrint)
};

int main(int argc, char *argv[])
{
    enum ExitCode
    {
        Success,
        ParseFailure,
        ArgumentError,
        WriteError,
        FileFailure
    };

    QCoreApplication app(argc, argv);

    QTextStream errorStream(stderr);

    if (argc != 2)
    {
        errorStream << PrettyPrint::tr(
                       "Usage: prettyprint <path to XML file>\n");
        return ArgumentError;
    }

    QString inputFilePath(QCoreApplication::arguments().at(1));
    QFile inputFile(inputFilePath);

    if (!QFile::exists(inputFilePath))
    {
        errorStream << PrettyPrint::tr(
                       "File %1 does not exist.\n").arg(inputFilePath);
        return FileFailure;

    } else if (!inputFile.open(QIODevice::ReadOnly)) {
        errorStream << PrettyPrint::tr(
                       "Failed to open file %1.\n").arg(inputFilePath);
        return FileFailure;
    }

    QFile outputFile;
    if (!outputFile.open(stdout, QIODevice::WriteOnly))
    {
        QTextStream(stderr) << PrettyPrint::tr("Failed to open stdout.");
        return WriteError;
    }

    QXmlStreamReader reader(&inputFile);
    int indentation = 0;
    QHash<int,QPair<int, int> > indentationStack;

    while (!reader.atEnd())
    {
        reader.readNext();
        if (reader.isStartElement()) {
            indentationStack[indentation] = QPair<int,int>(
                reader.lineNumber(), reader.columnNumber());
            indentation += 1;
        } else if (reader.isEndElement()) {
            indentationStack.remove(indentation);
            indentation -= 1;
        }

        if (reader.error())
        {
            errorStream << PrettyPrint::tr(
                           "Error: %1 in file %2 at line %3, column %4.\n").arg(
                               reader.errorString(), inputFilePath,
                               QString::number(reader.lineNumber()),
                               QString::number(reader.columnNumber()));
            if (indentationStack.contains(indentation-1)) {
                int line = indentationStack[indentation-1].first;
                int column = indentationStack[indentation-1].second;
                errorStream << PrettyPrint::tr(
                               "Opened at line %1, column %2.\n").arg(
                                   QString::number(line),
                                   QString::number(column));
            }
            return ParseFailure;

        } else if (reader.isStartElement() && !reader.name().isEmpty()) {
            outputFile.write(QByteArray().fill(' ', indentation));
            outputFile.write(reader.name().toString().toLocal8Bit());
            outputFile.write(QString(" line %1, column %2\n").arg(
                reader.lineNumber()).arg(reader.columnNumber()).toLocal8Bit());
        }
    }

    return Success;
}
