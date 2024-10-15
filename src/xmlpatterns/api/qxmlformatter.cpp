/***********************************************************************
*
* Copyright (c) 2012-2024 Barbara Geller
* Copyright (c) 2012-2024 Ansel Sermersheim
*
* Copyright (c) 2015 The Qt Company Ltd.
* Copyright (c) 2012-2016 Digia Plc and/or its subsidiary(-ies).
* Copyright (c) 2008-2012 Nokia Corporation and/or its subsidiary(-ies).
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
* https://www.gnu.org/licenses/
*
***********************************************************************/

#include <QDebug>

#include "qxmlformatter.h"
#include "qxpathhelper_p.h"
#include "qxmlserializer_p.h"

using namespace QPatternist;

class QXmlFormatterPrivate : public QXmlSerializerPrivate
{
 public:
   inline QXmlFormatterPrivate(const QXmlQuery &q,
                               QIODevice *const outputDevice);

   int             indentationDepth;
   int             currentDepth;
   QString         characterBuffer;
   QString         indentString;
   QStack<bool>    canIndent;
};

QXmlFormatterPrivate::QXmlFormatterPrivate(const QXmlQuery &query, QIODevice *const outputDevice)
   : QXmlSerializerPrivate(query, outputDevice), indentationDepth(4), currentDepth(0)
{
   indentString = '\n';

   canIndent.push(false);
}

QXmlFormatter::QXmlFormatter(const QXmlQuery &query, QIODevice *outputDevice)
   : QXmlSerializer(new QXmlFormatterPrivate(query, outputDevice))
{
}

void QXmlFormatter::startFormattingContent()
{
   Q_D(QXmlFormatter);

   if (QPatternist::XPathHelper::isWhitespaceOnly(d->characterBuffer)) {
      if (d->canIndent.top()) {
         QXmlSerializer::characters(QStringView(d->indentString));
      }

   } else {
      if (!d->characterBuffer.isEmpty()) {
         /* Significant data, we don't touch it. */
         QXmlSerializer::characters(QStringView(d->characterBuffer));
      }
   }

   d->characterBuffer.clear();
}

void QXmlFormatter::startElement(const QXmlName &name)
{
   Q_D(QXmlFormatter);
   startFormattingContent();
   ++d->currentDepth;
   d->indentString.append(QString(d->indentationDepth, QLatin1Char(' ')));
   d->canIndent.push(true);

   QXmlSerializer::startElement(name);
}

void QXmlFormatter::endElement()
{
   Q_D(QXmlFormatter);
   --d->currentDepth;
   d->indentString.chop(d->indentationDepth);

   if (!d->hasClosedElement.top().second) {
      d->canIndent.top() = false;
   }

   startFormattingContent();

   d->canIndent.pop();
   d->canIndent.top() = true;
   QXmlSerializer::endElement();
}

void QXmlFormatter::attribute(const QXmlName &name, QStringView value)
{
   QXmlSerializer::attribute(name, value);
}

void QXmlFormatter::comment(const QString &value)
{
   Q_D(QXmlFormatter);
   startFormattingContent();
   QXmlSerializer::comment(value);
   d->canIndent.top() = true;
}

void QXmlFormatter::characters(QStringView value)
{
   Q_D(QXmlFormatter);
   d->isPreviousAtomic = false;
   d->characterBuffer += value.toString();
}

void QXmlFormatter::processingInstruction(const QXmlName &name,
      const QString &value)
{
   Q_D(QXmlFormatter);
   startFormattingContent();
   QXmlSerializer::processingInstruction(name, value);
   d->canIndent.top() = true;
}

void QXmlFormatter::atomicValue(const QVariant &value)
{
   Q_D(QXmlFormatter);
   d->canIndent.top() = false;
   QXmlSerializer::atomicValue(value);
}

void QXmlFormatter::startDocument()
{
   QXmlSerializer::startDocument();
}

void QXmlFormatter::endDocument()
{
   QXmlSerializer::endDocument();
}

void QXmlFormatter::startOfSequence()
{
   QXmlSerializer::startOfSequence();
}

void QXmlFormatter::endOfSequence()
{
   Q_D(QXmlFormatter);

   /* Flush any buffered content. */
   if (! d->characterBuffer.isEmpty()) {
      QXmlSerializer::characters(QStringView(d->characterBuffer));
   }

   d->write('\n');
   QXmlSerializer::endOfSequence();
}

void QXmlFormatter::item(const QPatternist::Item &item)
{
   Q_D(QXmlFormatter);

   if (item.isAtomicValue()) {
      if (QPatternist::XPathHelper::isWhitespaceOnly(item.stringValue())) {
         return;
      } else {
         d->canIndent.top() = false;
         startFormattingContent();
      }
   }

   QXmlSerializer::item(item);
}

int QXmlFormatter::indentationDepth() const
{
   Q_D(const QXmlFormatter);
   return d->indentationDepth;
}

void QXmlFormatter::setIndentationDepth(int depth)
{
   Q_D(QXmlFormatter);
   d->indentationDepth = depth;
}
