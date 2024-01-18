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

   /**
    * Whether we /have/ sent nodes like processing instructions and comments
    * to QXmlSerializer.
    */
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

/*!
  \internal
 */
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

/*!
  \reimp
 */
void QXmlFormatter::startElement(const QXmlName &name)
{
   Q_D(QXmlFormatter);
   startFormattingContent();
   ++d->currentDepth;
   d->indentString.append(QString(d->indentationDepth, QLatin1Char(' ')));
   d->canIndent.push(true);

   QXmlSerializer::startElement(name);
}

/*!
  \reimp
 */
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

/*!
  \reimp
 */
void QXmlFormatter::attribute(const QXmlName &name, QStringView value)
{
   QXmlSerializer::attribute(name, value);
}

/*!
 \reimp
 */
void QXmlFormatter::comment(const QString &value)
{
   Q_D(QXmlFormatter);
   startFormattingContent();
   QXmlSerializer::comment(value);
   d->canIndent.top() = true;
}

/*!
 \reimp
 */
void QXmlFormatter::characters(QStringView value)
{
   Q_D(QXmlFormatter);
   d->isPreviousAtomic = false;
   d->characterBuffer += value.toString();
}

/*!
 \reimp
 */
void QXmlFormatter::processingInstruction(const QXmlName &name,
      const QString &value)
{
   Q_D(QXmlFormatter);
   startFormattingContent();
   QXmlSerializer::processingInstruction(name, value);
   d->canIndent.top() = true;
}

/*!
 \reimp
 */
void QXmlFormatter::atomicValue(const QVariant &value)
{
   Q_D(QXmlFormatter);
   d->canIndent.top() = false;
   QXmlSerializer::atomicValue(value);
}

/*!
 \reimp
 */
void QXmlFormatter::startDocument()
{
   QXmlSerializer::startDocument();
}

/*!
 \reimp
 */
void QXmlFormatter::endDocument()
{
   QXmlSerializer::endDocument();
}

/*!
 \reimp
 */
void QXmlFormatter::startOfSequence()
{
   QXmlSerializer::startOfSequence();
}

/*!
 \reimp
 */
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

/*!
 \internal
 */
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

/*!
  Returns the number of spaces QXmlFormatter will output for each
  indentation level. The default is four.

 \sa setIndentationDepth()
 */
int QXmlFormatter::indentationDepth() const
{
   Q_D(const QXmlFormatter);
   return d->indentationDepth;
}

/*!
  Sets \a depth to be the number of spaces QXmlFormatter will
  output for level of indentation. The default is four.

 \sa indentationDepth()
 */
void QXmlFormatter::setIndentationDepth(int depth)
{
   Q_D(QXmlFormatter);
   d->indentationDepth = depth;
}
