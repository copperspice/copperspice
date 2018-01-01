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

#include "main.h"

#include <QtCore/QDebug>
#include <QtCore/QFile>
#include <QtCore/QStringList>
#include <QtCore/QUrl>
#include <QtXmlPatterns/QXmlSchema>
#include <QtXmlPatterns/QXmlSchemaValidator>

QT_USE_NAMESPACE

int main(int argc, char **argv)
{
    enum ExitCode
    {
        Valid = 0,
        Invalid,
        ParseError
    };

    enum ExecutionMode
    {
        InvalidMode,
        SchemaOnlyMode,
        InstanceOnlyMode,
        SchemaAndInstanceMode
    };

    const QCoreApplication app(argc, argv);
    QCoreApplication::setApplicationName(QLatin1String("xmlpatternsvalidator"));

    QStringList arguments = QCoreApplication::arguments();
    if (arguments.size() != 2 && arguments.size() != 3) {
        qDebug() << QXmlPatternistCLI::tr("usage: xmlpatternsvalidator (<schema url> | <instance url> <schema url> | <instance url>)");
        return ParseError;
    }

    // parse command line arguments
    ExecutionMode mode = InvalidMode;

    QUrl schemaUri;
    QUrl instanceUri;

    {
        QUrl url = arguments[1];

        if (url.isRelative())
            url = QUrl::fromLocalFile(arguments[1]);

        if (arguments.size() == 2) {
            // either it is a schema or instance document

            if (arguments[1].toLower().endsWith(QLatin1String(".xsd"))) {
                schemaUri = url;
                mode = SchemaOnlyMode;
            } else {
                // as we could validate all types of xml documents, don't check the extension here
                instanceUri = url;
                mode = InstanceOnlyMode;
            }
        } else if (arguments.size() == 3) {
            instanceUri = url;
            schemaUri = arguments[2];

            if (schemaUri.isRelative())
                schemaUri = QUrl::fromLocalFile(schemaUri.toString());

            mode = SchemaAndInstanceMode;
        }
    }

    // Validate schema
    QXmlSchema schema;
    if (InstanceOnlyMode != mode) {
        schema.load(schemaUri);
        if (!schema.isValid())
            return Invalid;
    }

    if (SchemaOnlyMode == mode)
        return Valid;

    // Validate instance
    QXmlSchemaValidator validator(schema);
    if (validator.validate(instanceUri))
        return Valid;

    return Invalid;
}
