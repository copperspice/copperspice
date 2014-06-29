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

//! [0]
    QXmlNamePool namePool(query.namePool());
    query.bindVariable(QXmlName(namePool, localName), value);
//! [0]


{
//! [1]
    QByteArray myDocument;
    QBuffer buffer(&myDocument); // This is a QIODevice.
    buffer.open(QIODevice::ReadOnly);
    QXmlQuery query;
    query.bindVariable("myDocument", &buffer);
    query.setQuery("doc($myDocument)");
//! [1]
}


{
    QIODevice *device = 0;
//! [2]
    QXmlNamePool namePool(query.namePool());
    query.bindVariable(QXmlName(namePool, localName), device);
//! [2]

}

{
    QIODevice *myOutputDevice = 0;
//! [3]
    QFile xq("myquery.xq");

    QXmlQuery query;
    query.setQuery(&xq, QUrl::fromLocalFile(xq.fileName()));

    QXmlSerializer serializer(query, myOutputDevice);
    query.evaluateTo(&serializer);
//! [3]
}

{
    QIODevice *myOutputDevice = 0;
//! [4]
    QFile xq("myquery.xq");
    QString fileName("the filename");
    QString publisherName("the publisher");
    qlonglong year = 1234;

    QXmlQuery query;

    query.bindVariable("file", QVariant(fileName));
    query.bindVariable("publisher", QVariant(publisherName));
    query.bindVariable("year", QVariant(year));

    query.setQuery(&xq, QUrl::fromLocalFile(xq.fileName()));

    QXmlSerializer serializer(query, myOutputDevice);
    query.evaluateTo(&serializer);
//! [4]
}

{
//! [5]
    QFile xq("myquery.xq");
    QString fileName("the filename");
    QString publisherName("the publisher");
    qlonglong year = 1234;

    QXmlQuery query;

    query.bindVariable("file", QVariant(fileName));
    query.bindVariable("publisher", QVariant(publisherName));
    query.bindVariable("year", QVariant(year));

    query.setQuery(&xq, QUrl::fromLocalFile(xq.fileName()));

    QXmlResultItems result;
    query.evaluateTo(&result);
    QXmlItem item(result.next());
    while (!item.isNull()) {
        if (item.isAtomicValue()) {
            QVariant v = item.toAtomicValue();
            switch (v.type()) {
                case QVariant::LongLong:
                    // xs:integer
                    break;
                case QVariant::String:
                    // xs:string
                    break;
                default:
                    // error
                    break;
            }
        }
        else if (item.isNode()) {
            QXmlNodeModelIndex i = item.toNodeModelIndex();
            // process node
        }
        item = result.next();
    }
//! [5]
}

{
//! [6]
    QFile xq("myquery.xq");

    QXmlQuery query;
    query.setQuery(&xq, QUrl::fromLocalFile(xq.fileName()));

    QXmlResultItems result;
    query.evaluateTo(&result);
    QXmlItem item(result.next());
    while (!item.isNull()) {
        if (item.isAtomicValue()) {
            QVariant v = item.toAtomicValue();
            switch (v.type()) {
                case QVariant::LongLong:
                    // xs:integer
                    break;
                case QVariant::String:
                    // xs:string
                    break;
                default:
                    if (v.userType() == qMetaTypeId<QXmlName>()) {
                        QXmlName n = qvariant_cast<QXmlName>(v);
                        // process QXmlName n...
                    }
                    else {
                        // error
                    }
                    break;
            }
        }
        else if (item.isNode()) {
            QXmlNodeModelIndex i = item.toNodeModelIndex();
            // process node
        }
        item = result.next();
    }
//! [6]
}

{
    QIODevice *out = 0;
//! [7]
    QXmlQuery query(QXmlQuery::XSLT20);
    query.setFocus(QUrl("myInput.xml"));
    query.setQuery(QUrl("myStylesheet.xsl"));
    query.evaluateTo(out);
//! [7]
}
