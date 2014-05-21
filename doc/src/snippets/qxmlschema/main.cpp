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

#include <QtCore>
#include <QtXmlPatterns>

class Schema
{
    public:
        void loadFromUrl() const;
        void loadFromFile() const;
        void loadFromData() const;
};

void Schema::loadFromUrl() const
{
//! [0]
    QUrl url("http://www.schema-example.org/myschema.xsd");

    QXmlSchema schema;
    if (schema.load(url) == true)
        qDebug() << "schema is valid";
    else
        qDebug() << "schema is invalid";
//! [0]
}

void Schema::loadFromFile() const
{
//! [1]
    QFile file("myschema.xsd");
    file.open(QIODevice::ReadOnly);

    QXmlSchema schema;
    schema.load(&file, QUrl::fromLocalFile(file.fileName()));

    if (schema.isValid())
        qDebug() << "schema is valid";
    else
        qDebug() << "schema is invalid";
//! [1]
}

void Schema::loadFromData() const
{
//! [2]
    QByteArray data( "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
                     "<xsd:schema"
                     "        xmlns:xsd=\"http://www.w3.org/2001/XMLSchema\""
                     "        xmlns=\"http://qt.nokia.com/xmlschematest\""
                     "        targetNamespace=\"http://qt.nokia.com/xmlschematest\""
                     "        version=\"1.0\""
                     "        elementFormDefault=\"qualified\">"
                     "</xsd:schema>" );

    QXmlSchema schema;
    schema.load(data);

    if (schema.isValid())
        qDebug() << "schema is valid";
    else
        qDebug() << "schema is invalid";
//! [2]
}

int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);

    Schema schema;

    schema.loadFromUrl();
    schema.loadFromFile();
    schema.loadFromData();

    return 0;
}
