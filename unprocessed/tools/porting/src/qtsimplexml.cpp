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

#include "qtsimplexml.h"
#include <QDomDocument>

QT_BEGIN_NAMESPACE

QtSimpleXml::QtSimpleXml(const QString &name)
{
    valid = false;
    n = name;
    parent = 0;
}

QtSimpleXml &QtSimpleXml::operator [](int index)
{
    if (index < 0)
	return *this;

    if (index > children.size()) {
        static QtSimpleXml NIL;
        qWarning("QtSimpleXml::operator[], Out of range access: size is %i, index is %i",
                 children.size(), index);
        return NIL;
    }

    if (index == children.size()) {
        QtSimpleXml *item = new QtSimpleXml(QLatin1String("item"));
        item->parent = this;
        children.insert(item->name(), item);
        return *item;
    }

    QMultiMap<QString, QtSimpleXml *>::Iterator it = children.begin();
    while (index--) ++it;
    return *it.value();
}

QtSimpleXml &QtSimpleXml::operator [](const QString &key)
{
    if (!children.contains(key)) {
	QtSimpleXml *item = new QtSimpleXml(key);
	item->parent = this;
	children.insert(item->name(), item);
        return *item;
    }

    return *children.find(key).value();
}

QtSimpleXml &QtSimpleXml::operator =(const QString &text)
{
    valid = true;
    QtSimpleXml *p = parent;
    while (p && !p->valid) {
	p->valid = true;
	p = p->parent;
    }

    s = text;
    return *this;
}

QDomDocument QtSimpleXml::toDomDocument() const
{
    QDomDocument doc;
    QString data = QLatin1String("version=\"1.0\" encoding=\"UTF-8\"");
    doc.appendChild(doc.createProcessingInstruction(QLatin1String("xml"), data));

    if (!valid)
	return doc;

    if(!s.isEmpty())
        doc.appendChild(doc.createTextNode(s));

    {
        QMultiMap<QString, QtSimpleXml *>::ConstIterator it = children.constBegin();
        for (; it != children.end(); ++it) {
            QtSimpleXml *item = it.value();
            if (item->valid) {
                QDomNode node = item->toDomElement(&doc);
                doc.appendChild(node);
            }
        }
    }

    return doc;
}

QDomElement QtSimpleXml::toDomElement(QDomDocument *doc) const
{
    QDomElement elem = doc->createElement(n);
    QMap<QString, QString>::ConstIterator ita = attr.constBegin();
    for (; ita != attr.constEnd(); ++ita)
	elem.setAttribute(ita.key(), ita.value());

    if(!s.isEmpty())
        elem.appendChild(doc->createTextNode(s));

    {
        QMultiMap<QString, QtSimpleXml *>::ConstIterator it = children.constBegin();
        for (; it != children.constEnd(); ++it) {
            QtSimpleXml *item = it.value();
            if (item->valid) {
                QDomNode node = item->toDomElement(doc);
                elem.appendChild(node);
            }
        }
    }

    return elem;
}

QString QtSimpleXml::name() const
{
    return n;
}

QString QtSimpleXml::text() const
{
    return s;
}

int QtSimpleXml::numChildren() const
{
    return children.count();
}

bool QtSimpleXml::isValid() const
{
    return valid;
}

void QtSimpleXml::setAttribute(const QString &key, const QString &value)
{
    attr.insert(key, QString(value));
}

QString QtSimpleXml::attribute(const QString &key)
{
    return attr[key];
}

bool QtSimpleXml::setContent(const QString &content)
{
    QDomDocument doc;
    QString errorMsg;
    int errorLine;
    int errorColumn;

    if (!doc.setContent(content, false, &errorMsg, &errorLine, &errorColumn)) {
        errorStr = errorMsg;
        errorStr += QLatin1String(" at ") + QString::number(errorLine) + QLatin1String(":") + QString::number(errorColumn);
        return false;
    }

    parse(doc);
    return true;
}

bool QtSimpleXml::setContent(QIODevice *device)
{
    QDomDocument doc;
    QString errorMsg;
    int errorLine;
    int errorColumn;
    if (!doc.setContent(device, false, &errorMsg, &errorLine, &errorColumn)) {
        errorStr = errorMsg;
        errorStr += QLatin1String(" at ") + QString::number(errorLine) + QLatin1String(":") + QString::number(errorColumn);
        return false;
    }

    QDomNode child = doc.firstChild();
    while (!child.isNull() && !child.isElement())
        child = child.nextSibling();

    while (!child.isNull()) {
        QtSimpleXml *xmlNode = new QtSimpleXml;
        xmlNode->parse(child);
        xmlNode->parent=this;
        children.insert(xmlNode->name(), xmlNode);
        do {
            child = child.nextSibling();
        } while (!child.isNull() && !child.isElement());
    }

    return true;
}


void QtSimpleXml::parse(QDomNode node)
{
 //   puts("parse");
    if (node.isNull())
        return;

    valid = true;
    n = node.nodeName();
    QDomElement element = node.toElement();

    QDomNamedNodeMap attrs = element.attributes();
    for (int i = 0; i < (int) attrs.count(); ++i) {
        QDomAttr attribute = attrs.item(i).toAttr();
        attr.insert(attribute.name(), attribute.value());
    }

    if (element.firstChild().isText()) {
  //      printf("Got text %s\n", element.text().stripWhiteSpace().latin1());
        s = element.text().trimmed();
        return;
    }

    if (node.hasChildNodes()) {

        // Skip to first element child
        QDomNode child = node.firstChild();
        while (!child.isNull() && !child.isElement())
            child = child.nextSibling();

        while (!child.isNull()) {
            QtSimpleXml *xmlNode = new QtSimpleXml;
            xmlNode->parse(child);
            children.insert(xmlNode->name(), xmlNode);

            node = node.nextSibling();

            do {
                child = child.nextSibling();
            } while (!child.isNull() && !child.isElement());
        }
    }
}

QString QtSimpleXml::errorString() const
{
    return errorStr;
}

QT_END_NAMESPACE
