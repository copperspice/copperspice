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

#include "qdynamiccontext_p.h"
#include "qpatternistlocale_p.h"
#include "qitem_p.h"
#include "qxmlquery_p.h"
#include "qxmlserializer_p.h"
#include "qxmlserializer.h"

using namespace QPatternist;

QXmlSerializerPrivate::QXmlSerializerPrivate(const QXmlQuery &query,
      QIODevice *outputDevice)
   : isPreviousAtomic(false),
     state(QXmlSerializer::BeforeDocumentElement),
     np(query.namePool().d),
     device(outputDevice),
     codec(QTextCodec::codecForMib(106)), /* UTF-8 */
     query(query)
{
   hasClosedElement.reserve(EstimatedTreeDepth);
   namespaces.reserve(EstimatedTreeDepth);
   nameCache.reserve(EstimatedNameCount);

   hasClosedElement.push(qMakePair(QXmlName(), true));

   /*
     We push the empty namespace such that first of all
     namespaceBinding() won't assert on an empty QStack,
     and such that the empty namespace is in-scope and
     that the code doesn't attempt to declare it.

     We push the XML namespace. Although we won't receive
     declarations for it, we may output attributes by that
     name.
   */
   QVector<QXmlName> defNss;
   defNss.resize(2);
   defNss[0] = QXmlName(StandardNamespaces::empty,
                        StandardLocalNames::empty,
                        StandardPrefixes::empty);
   defNss[1] = QXmlName(StandardNamespaces::xml,
                        StandardLocalNames::empty,
                        StandardPrefixes::xml);

   namespaces.push(defNss);

   /* If we don't set this flag, QTextCodec will generate a BOM. */
   converterState.m_flags = QTextCodec::IgnoreHeader;
}

QXmlSerializer::QXmlSerializer(const QXmlQuery &query,
                               QIODevice *outputDevice) : QAbstractXmlReceiver(new QXmlSerializerPrivate(query, outputDevice))
{
   if (!outputDevice) {
      qWarning("outputDevice cannot be null.");
      return;
   }

   if (!outputDevice->isWritable()) {
      qWarning("outputDevice must be opened in write mode.");
      return;
   }
}

QXmlSerializer::QXmlSerializer(QAbstractXmlReceiverPrivate *d) : QAbstractXmlReceiver(d)
{
}

bool QXmlSerializer::atDocumentRoot() const
{
   Q_D(const QXmlSerializer);
   return d->state == BeforeDocumentElement ||
          (d->state == InsideDocumentElement && d->hasClosedElement.size() == 1);
}

void QXmlSerializer::startContent()
{
   Q_D(QXmlSerializer);
   if (!d->hasClosedElement.top().second) {
      d->write('>');
      d->hasClosedElement.top().second = true;
   }
}

void QXmlSerializer::writeEscaped(const QString &toEscape)
{
   if (toEscape.isEmpty()) { /* Early exit. */
      return;
   }

   QString result;
   const int length = toEscape.length();

   for (int i = 0; i < length; ++i) {
      const QChar c(toEscape.at(i));

      if (c == QLatin1Char('<')) {
         result += QLatin1String("&lt;");
      } else if (c == QLatin1Char('>')) {
         result += QLatin1String("&gt;");
      } else if (c == QLatin1Char('&')) {
         result += QLatin1String("&amp;");
      } else {
         result += toEscape.at(i);
      }
   }

   write(result);
}

void QXmlSerializer::writeEscapedAttribute(const QString &toEscape)
{
   if (toEscape.isEmpty()) { /* Early exit. */
      return;
   }

   QString result;
   const int length = toEscape.length();

   for (int i = 0; i < length; ++i) {
      const QChar c(toEscape.at(i));

      if (c == QLatin1Char('<')) {
         result += QLatin1String("&lt;");
      } else if (c == QLatin1Char('>')) {
         result += QLatin1String("&gt;");
      } else if (c == QLatin1Char('&')) {
         result += QLatin1String("&amp;");
      } else if (c == QLatin1Char('"')) {
         result += QLatin1String("&quot;");
      } else {
         result += toEscape.at(i);
      }
   }

   write(result);
}

void QXmlSerializer::write(const QString &content)
{
   Q_D(QXmlSerializer);
   d->device->write(d->codec->fromUnicode(content, &d->converterState));
}

void QXmlSerializer::write(const QXmlName &name)
{
   Q_D(QXmlSerializer);
   const QByteArray &cell = d->nameCache[name.code()];

   if (cell.isNull()) {
      QByteArray &mutableCell = d->nameCache[name.code()];

      const QString content(d->np->toLexical(name));
      mutableCell = d->codec->fromUnicode(content, &d->converterState);
      d->device->write(mutableCell);

   } else {
      d->device->write(cell);
   }
}

void QXmlSerializer::write(const char *const chars)
{
   Q_D(QXmlSerializer);
   d->device->write(chars);
}

void QXmlSerializer::startElement(const QXmlName &name)
{
   Q_D(QXmlSerializer);
   Q_ASSERT(d->device);
   Q_ASSERT(d->device->isWritable());
   Q_ASSERT(d->codec);
   Q_ASSERT(!name.isNull());

   d->namespaces.push(QVector<QXmlName>());

   if (atDocumentRoot()) {
      if (d->state == BeforeDocumentElement) {
         d->state = InsideDocumentElement;

      } else if (d->state != InsideDocumentElement) {
         d->query.d->staticContext()->error(QtXmlPatterns::tr("Element %1 can not be serialized because it appears outside "
            "the document element.").formatArgs(formatKeyword(d->np, name)), ReportContext::SENR0001,
            d->query.d->expression().data());
      }
   }

   startContent();
   d->write('<');
   write(name);

   /* Ensure that the namespace URI used in the name gets outputted. */
   namespaceBinding(name);

   d->hasClosedElement.push(qMakePair(name, false));
   d->isPreviousAtomic = false;
}

void QXmlSerializer::endElement()
{
   Q_D(QXmlSerializer);
   const QPair<QXmlName, bool> e(d->hasClosedElement.pop());
   d->namespaces.pop();

   if (e.second) {
      write("</");
      write(e.first);
      d->write('>');
   } else {
      write("/>");
   }

   d->isPreviousAtomic = false;
}

void QXmlSerializer::attribute(const QXmlName &name, QStringView value)
{
   Q_D(QXmlSerializer);
   Q_ASSERT(!name.isNull());

   /* Ensure that the namespace URI used in the name gets outputted. */
   {
      /* Since attributes doesn't pick up the default namespace, a
       * namespace declaration would cause trouble if we output it. */
      if (name.prefix() != StandardPrefixes::empty) {
         namespaceBinding(name);
      }
   }

   if (atDocumentRoot()) {
      d->query.d->staticContext()->error(QtXmlPatterns::tr("Attribute %1 can not be serialized because it appears at "
            "the top level.").formatArg(formatKeyword(d->np, name)),
            ReportContext::SENR0001, d->query.d->expression().data());

   } else {
      d->write(' ');
      write(name);
      write("=\"");
      writeEscapedAttribute(value.toString());
      d->write('"');
   }
}

bool QXmlSerializer::isBindingInScope(const QXmlName nb) const
{
   Q_D(const QXmlSerializer);
   const int levelLen = d->namespaces.size();

   if (nb.prefix() == StandardPrefixes::empty) {
      for (int lvl = levelLen - 1; lvl >= 0; --lvl) {
         const QVector<QXmlName> &scope = d->namespaces.at(lvl);
         const int vectorLen = scope.size();

         for (int s = vectorLen - 1; s >= 0; --s) {
            const QXmlName &nsb = scope.at(s);

            if (nsb.prefix() == StandardPrefixes::empty) {
               return nsb.namespaceURI() == nb.namespaceURI();
            }
         }
      }

   } else {
      for (int lvl = 0; lvl < levelLen; ++lvl) {
         const QVector<QXmlName> &scope = d->namespaces.at(lvl);
         const int vectorLen = scope.size();

         for (int s = 0; s < vectorLen; ++s) {
            const QXmlName &n = scope.at(s);
            if (n.prefix() == nb.prefix() &&
                  n.namespaceURI() == nb.namespaceURI()) {
               return true;
            }
         }
      }
   }

   return false;
}

void QXmlSerializer::namespaceBinding(const QXmlName &nb)
{
   /*
    * Writes out \a nb.
    *
    * Namespace bindings aren't looked up in a cache, because
    * we typically receive very few.
    */

   Q_D(QXmlSerializer);
   Q_ASSERT_X(! nb.isNull(), Q_FUNC_INFO, "Unable to bind a null QXmlName.");

   Q_ASSERT_X((nb.namespaceURI() != StandardNamespaces::empty) || (nb.prefix() == StandardPrefixes::empty),
              Q_FUNC_INFO, "Undeclarations of prefixes are not allowed in XML 1.0.");

   if (nb.namespaceURI() == QPatternist::StandardNamespaces::StopNamespaceInheritance) {
      return;
   }

   if (isBindingInScope(nb)) {
      return;
   }

   d->namespaces.top().append(nb);

   if (nb.prefix() == StandardPrefixes::empty) {
      write(" xmlns");
   } else {
      write(" xmlns:");
      write(d->np->stringForPrefix(nb.prefix()));
   }

   write("=\"");
   writeEscapedAttribute(d->np->stringForNamespace(nb.namespaceURI()));
   d->write('"');
}

void QXmlSerializer::comment(const QString &value)
{
   Q_D(QXmlSerializer);

   Q_ASSERT_X(!value.contains(QLatin1String("--")), Q_FUNC_INFO,
                  "Invalid input, it is the caller's responsibility to ensure the input is correct.");

   startContent();
   write("<!--");
   write(value);
   write("-->");
   d->isPreviousAtomic = false;
}

void QXmlSerializer::characters(QStringView value)
{
   Q_D(QXmlSerializer);

   d->isPreviousAtomic = false;
   startContent();
   writeEscaped(QString(value));
}

void QXmlSerializer::processingInstruction(const QXmlName &name, const QString &value)
{
   Q_D(QXmlSerializer);
   Q_ASSERT_X(! value.contains(QLatin1String("?>")), Q_FUNC_INFO,
                  "Invalid input, it is the caller's responsibility to ensure the input is correct.");

   startContent();
   write("<?");
   write(name);
   d->write(' ');
   write(value);
   write("?>");

   d->isPreviousAtomic = false;
}

void QXmlSerializer::item(const QPatternist::Item &outputItem)
{
   Q_D(QXmlSerializer);

   if (outputItem.isAtomicValue()) {
      if (d->isPreviousAtomic) {
         startContent();
         d->write(' ');
         writeEscaped(outputItem.stringValue());
      } else {
         d->isPreviousAtomic = true;
         const QString value(outputItem.stringValue());

         if (!value.isEmpty()) {
            startContent();
            writeEscaped(value);
         }
      }
   } else {
      startContent();
      Q_ASSERT(outputItem.isNode());
      sendAsNode(outputItem);
   }
}

void QXmlSerializer::atomicValue(const QVariant &value)
{
   (void) value;
}

void QXmlSerializer::startDocument()
{
   Q_D(QXmlSerializer);
   d->isPreviousAtomic = false;
}

void QXmlSerializer::endDocument()
{
   Q_D(QXmlSerializer);
   d->isPreviousAtomic = false;
}

QIODevice *QXmlSerializer::outputDevice() const
{
   Q_D(const QXmlSerializer);
   return d->device;
}

void QXmlSerializer::setCodec(const QTextCodec *outputCodec)
{
   Q_D(QXmlSerializer);
   d->codec = outputCodec;
}

const QTextCodec *QXmlSerializer::codec() const
{
   Q_D(const QXmlSerializer);
   return d->codec;
}

void QXmlSerializer::startOfSequence()
{
}

void QXmlSerializer::endOfSequence()
{
   /* If this function is changed to flush or close or something like that,
    * take into consideration QXmlFormatter, especially
    * QXmlFormatter::endOfSequence().
    */
}
