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

class SchemaValidator
{
    public:
        void validateFromUrl() const;
        void validateFromFile() const;
        void validateFromData() const;
        void validateComplete() const;

    private:
        QXmlSchema getSchema() const;
};

void SchemaValidator::validateFromUrl() const
{
//! [0]
    const QXmlSchema schema = getSchema();

    const QUrl url("http://www.schema-example.org/test.xml");

    QXmlSchemaValidator validator(schema);
    if (validator.validate(url))
        qDebug() << "instance document is valid";
    else
        qDebug() << "instance document is invalid";
//! [0]
}

void SchemaValidator::validateFromFile() const
{
//! [1]
    const QXmlSchema schema = getSchema();

    QFile file("test.xml");
    file.open(QIODevice::ReadOnly);

    QXmlSchemaValidator validator(schema);
    if (validator.validate(&file, QUrl::fromLocalFile(file.fileName())))
        qDebug() << "instance document is valid";
    else
        qDebug() << "instance document is invalid";
//! [1]
}

void SchemaValidator::validateFromData() const
{
//! [2]
    const QXmlSchema schema = getSchema();

    QByteArray data("<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
                    "<test></test>");

    QBuffer buffer(&data);
    buffer.open(QIODevice::ReadOnly);

    QXmlSchemaValidator validator(schema);
    if (validator.validate(&buffer))
        qDebug() << "instance document is valid";
    else
        qDebug() << "instance document is invalid";
//! [2]
}

QXmlSchema SchemaValidator::getSchema() const
{
    QByteArray data("<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
                    "<xsd:schema"
                    "        xmlns:xsd=\"http://www.w3.org/2001/XMLSchema\""
                    "        xmlns=\"http://qt.nokia.com/xmlschematest\""
                    "        targetNamespace=\"http://qt.nokia.com/xmlschematest\""
                    "        version=\"1.0\""
                    "        elementFormDefault=\"qualified\">"
                    "</xsd:schema>");

    QBuffer buffer(&data);
    buffer.open(QIODevice::ReadOnly);

    QXmlSchema schema;
    schema.load(&buffer);

    return schema;
}

void SchemaValidator::validateComplete() const
{
//! [3]
    QUrl schemaUrl("file:///home/user/schema.xsd");

    QXmlSchema schema;
    schema.load(schemaUrl);

    if (schema.isValid()) {
        QFile file("test.xml");
        file.open(QIODevice::ReadOnly);

        QXmlSchemaValidator validator(schema);
        if (validator.validate(&file, QUrl::fromLocalFile(file.fileName())))
            qDebug() << "instance document is valid";
        else
            qDebug() << "instance document is invalid";
    }
//! [3]
}

int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);

    SchemaValidator validator;

    validator.validateFromUrl();
    validator.validateFromFile();
    validator.validateFromData();
    validator.validateComplete();

    return 0;
}
