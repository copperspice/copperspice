/***********************************************************************
*
* Copyright (c) 2012-2020 Barbara Geller
* Copyright (c) 2012-2020 Ansel Sermersheim
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

template<typename TokenLookupClass, typename LookupKey>
MaintainingReader<TokenLookupClass, LookupKey>::MaintainingReader(const typename ElementDescription<TokenLookupClass,
                  LookupKey>::Hash &elementDescriptions, const QSet<typename TokenLookupClass::NodeName> &standardAttributes,
                  const ReportContext::Ptr &context, QIODevice *const queryDevice)
   : QXmlStreamReader(queryDevice), m_hasHandledStandardAttributes(false), m_context(context),
                  m_elementDescriptions(elementDescriptions), m_standardAttributes(standardAttributes)
{
   Q_ASSERT(m_context);
   Q_ASSERT(!m_elementDescriptions.isEmpty());

   /* We start with stripping. */
   m_stripWhitespace.push(true);
}

template<typename TokenLookupClass, typename LookupKey>
MaintainingReader<TokenLookupClass, LookupKey>::~MaintainingReader()
{
}

template<typename TokenLookupClass, typename LookupKey>
QSourceLocation MaintainingReader<TokenLookupClass, LookupKey>::currentLocation() const
{
   return QSourceLocation(documentURI(), lineNumber(), columnNumber());
}

template<typename TokenLookupClass, typename LookupKey>
QXmlStreamReader::TokenType MaintainingReader<TokenLookupClass, LookupKey>::readNext()
{
   const TokenType retval = QXmlStreamReader::readNext();

   switch (retval) {
      case StartElement: {
         m_currentElementName = TokenLookupClass::toToken(name());
         m_currentAttributes = attributes();
         m_hasHandledStandardAttributes = false;

         if (!m_currentAttributes.hasAttribute(QLatin1String("xml:space"))) {
            m_stripWhitespace.push(m_stripWhitespace.top());
         }
         break;
      }
      case EndElement:
         m_currentElementName = TokenLookupClass::toToken(name());
         m_stripWhitespace.pop();
         break;
      default:
         break;
   }

   return retval;
}

template<typename TokenLookupClass, typename LookupKey>
bool MaintainingReader<TokenLookupClass, LookupKey>::isWhitespace() const
{
   return QXmlStreamReader::isWhitespace() || XPathHelper::isWhitespaceOnly(text());
}

template<typename TokenLookupClass, typename LookupKey>
void MaintainingReader<TokenLookupClass, LookupKey>::error(const QString &message,
      const ReportContext::ErrorCode code) const
{
   m_context->error(message, code, currentLocation());
}

template<typename TokenLookupClass, typename LookupKey>
void MaintainingReader<TokenLookupClass, LookupKey>::warning(const QString &message) const
{
   m_context->warning(message, currentLocation());
}

template<typename TokenLookupClass, typename LookupKey>
typename TokenLookupClass::NodeName MaintainingReader<TokenLookupClass, LookupKey>::currentElementName() const
{
   return m_currentElementName;
}

template<typename TokenLookupClass, typename LookupKey>
void MaintainingReader<TokenLookupClass, LookupKey>::validateElement(const LookupKey elementName) const
{
   Q_ASSERT(tokenType() == QXmlStreamReader::StartElement);

   if (m_elementDescriptions.contains(elementName)) {
      // QHash::value breaks in Metrowerks Compiler
      const ElementDescription<TokenLookupClass, LookupKey> &desc = *m_elementDescriptions.find(elementName);
      const int attCount = m_currentAttributes.count();

      QSet<typename TokenLookupClass::NodeName> encounteredXSLTAtts;

      for (int i = 0; i < attCount; ++i) {
         const QXmlStreamAttribute &attr = m_currentAttributes.at(i);
         if (attr.namespaceUri().isEmpty()) {
            const typename TokenLookupClass::NodeName attrName(TokenLookupClass::toToken(attr.name()));
            encounteredXSLTAtts.insert(attrName);

            if (!desc.requiredAttributes.contains(attrName) &&
                  !desc.optionalAttributes.contains(attrName) &&
                  !m_standardAttributes.contains(attrName) &&
                  !isAnyAttributeAllowed()) {
               QString translationString;

               QList<typename TokenLookupClass::NodeName> all(desc.requiredAttributes.toList() + desc.optionalAttributes.toList());
               const int totalCount = all.count();
               QStringList allowed;

               for (int i = 0; i < totalCount; ++i) {
                  allowed.append(QPatternist::formatKeyword(TokenLookupClass::toString(all.at(i))));
               }

               /* Note, we can't run toString() on attrName, because we're in this branch,
                * the token lookup doesn't have the string(!).*/
               const QString stringedName(attr.name().toString());

               if (totalCount == 0) {
                  translationString =
                     QtXmlPatterns::tr("Attribute %1 can not appear on the element %2. Only the standard attributes can appear.")
                     .formatArgs(formatKeyword(stringedName), formatKeyword(name()));

               } else if (totalCount == 1) {
                  translationString =
                     QtXmlPatterns::tr("Attribute %1 can not appear on the element %2. Only %3 is allowed, and the standard attributes.")
                     .formatArgs(formatKeyword(stringedName), formatKeyword(name()), allowed.first());

               } else if (totalCount == 1) {
                  /* Note, allowed has already had formatKeyword() applied. */
                  translationString =
                     QtXmlPatterns::tr("Attribute %1 can not appear on the element %2. Allowed is %3, %4, and the standard attributes.")
                     .formatArgs(formatKeyword(stringedName), formatKeyword(name()), allowed.first(), allowed.last());

               } else {
                  /* Note, allowed has already had formatKeyword() applied. */
                  translationString =
                     QtXmlPatterns::tr("Attribute %1 can not appear on the element %2. Allowed is %3, and the standard attributes.")
                     .formatArgs(formatKeyword(stringedName), formatKeyword(name()), allowed.join(QLatin1String(", ")));
               }

               m_context->error(translationString, ReportContext::XTSE0090, currentLocation());
            }

         } else if (attr.namespaceUri() == namespaceUri()) {
            m_context->error(
               QtXmlPatterns::tr("XSL-T attributes on XSL-T elements must be in the null namespace, not in the XSL-T namespace which %1 is.")
               .formatArg(formatKeyword(attr.name())), ReportContext::XTSE0090, currentLocation());
         }

         /* Else, attributes in other namespaces are allowed, continue. */
      }

      const QSet<typename TokenLookupClass::NodeName> requiredButMissing(QSet<typename TokenLookupClass::NodeName>
            (desc.requiredAttributes).subtract(encounteredXSLTAtts));

      if (!requiredButMissing.isEmpty()) {
         error(QtXmlPatterns::tr("The attribute %1 must appear on element %2.")
               .formatArgs(QPatternist::formatKeyword(TokenLookupClass::toString(*requiredButMissing.constBegin())), formatKeyword(name())),
               ReportContext::XTSE0010);
      }

   } else {
      error(QtXmlPatterns::tr("The element with local name %1 does not exist in XSL-T.").formatArg(formatKeyword(name())),
            ReportContext::XTSE0010);
   }
}

template<typename TokenLookupClass,
         typename LookupKey>
bool MaintainingReader<TokenLookupClass, LookupKey>::hasAttribute(const QString &namespaceURI,
      const QString &localName) const
{
   Q_ASSERT(tokenType() == QXmlStreamReader::StartElement);
   return m_currentAttributes.hasAttribute(namespaceURI, localName);
}

template<typename TokenLookupClass,
         typename LookupKey>
bool MaintainingReader<TokenLookupClass, LookupKey>::hasAttribute(const QString &localName) const
{
   return hasAttribute(QString(), localName);
}

template<typename TokenLookupClass,
         typename LookupKey>
QString MaintainingReader<TokenLookupClass, LookupKey>::readAttribute(const QString &localName,
      const QString &namespaceURI) const
{
   Q_ASSERT(tokenType() == QXmlStreamReader::StartElement);

   Q_ASSERT_X(m_currentAttributes.hasAttribute(namespaceURI, localName),
              Q_FUNC_INFO,
              "Validation must be done before this function is called.");

   return m_currentAttributes.value(namespaceURI, localName).toString();
}

