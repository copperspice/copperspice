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

#ifndef QHCPWRITER_H
#define QHCPWRITER_H

#include <QtXml/QXmlStreamWriter>
#include "adpreader.h"

QT_BEGIN_NAMESPACE

class QhcpWriter : public QXmlStreamWriter
{
public:
    QhcpWriter();
    bool writeFile(const QString &fileName);
    void setHelpProjectFile(const QString &qhpFile);
    void setProperties(const QMap<QString, QString> props);
    void setTitlePath(const QString &path);

private:
    void writeAssistantSettings();
    void writeDocuments();

    QString m_qhpFile;
    QMap<QString, QString> m_properties;
    QString m_titlePath;
};

QT_END_NAMESPACE

#endif
