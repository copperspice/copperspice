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

#ifndef QMaintainingReader_P_H
#define QMaintainingReader_P_H

#include <QSet>
#include <QSourceLocation>
#include <QStack>
#include <QStringList>
#include <QXmlStreamReader>

#include <qxpathhelper_p.h>
#include <qxslttokenlookup_p.h>

class QUrl;

QT_BEGIN_NAMESPACE

namespace QPatternist {

template<typename TokenLookupClass, typename LookupKey = typename TokenLookupClass::NodeName>
class ElementDescription
{
 public:
   typedef QHash<LookupKey, ElementDescription<TokenLookupClass, LookupKey> > Hash;
   QSet<typename TokenLookupClass::NodeName> requiredAttributes;
   QSet<typename TokenLookupClass::NodeName> optionalAttributes;
};


template<typename TokenLookupClass, typename LookupKey = typename TokenLookupClass::NodeName>
class MaintainingReader : public QXmlStreamReader, protected TokenLookupClass
{
 protected:

   MaintainingReader(const typename ElementDescription<TokenLookupClass, LookupKey>::Hash &elementDescriptions,
                     const QSet<typename TokenLookupClass::NodeName> &standardAttributes,
                     const ReportContext::Ptr &context, QIODevice *const queryDevice);

   virtual ~MaintainingReader();

   QXmlStreamReader::TokenType readNext();

   /**
    * Returns the name of the current element.
    */
   inline typename TokenLookupClass::NodeName currentElementName() const;

   /**
    * @short Convenience function for calling ReportContext::error().
    */
   void error(const QString &message, const ReportContext::ErrorCode code) const;

   /**
    * @short Convenience function for calling ReportContext::warning().
    */
   void warning(const QString &message) const;

   /**
    * @short Returns the location of the document that MaintainingReader
    * is parsing. Used for error reporting
    */
   virtual QUrl documentURI() const = 0;

   /**
    * @short Returns @c true, if any attribute is allowed on the
    * element currently being validated.
    */
   virtual bool isAnyAttributeAllowed() const = 0;

   /**
    * QXmlStreamReader::isWhitespace() returns true for whitespace that is
    * not expressed as character references, while XSL-T operatates ontop
    * of the XDM, which means we needs to return true for those too.
    *
    * @see <a href="http://www.w3.org/TR/xslt20/#data-model">4 Data Model</a>
    */
   bool isWhitespace() const;

   /**
    * This function is not merged with handleStandardAttributes() because
    * handleStandardAttributes() needs to be called for all elements,
    * while validateElement() only applies to XSL-T elements.
    *
    * @see handleStandardAttributes()
    */
   void validateElement(const LookupKey name) const;

   QXmlStreamAttributes                                                    m_currentAttributes;

   bool                                                                    m_hasHandledStandardAttributes;

   /**
    * This stack mirrors the depth of elements in the parsed document. If
    * no @c xml:space is present on the current element, MaintainingReader
    * simply pushes the current top(). However, it never sets the value
    * depending on @c xml:space's value.
    */
   QStack<bool>                                                            m_stripWhitespace;

   /**
    * @short Returns the value for attribute by name \a name.
    *
    * If it doesn't exist, an error is raised.
    *
    * It is assumed that m_reader's current state is
    * QXmlStreamReader::StartElement.
    */
   QString readAttribute(const QString &localName,
                         const QString &namespaceURI = QString()) const;

   /**
    * @short Returns @c true if the current element has an attribute whose
    * name is @p namespaceURI and local name is @p localName.
    */
   bool hasAttribute(const QString &namespaceURI, const QString &localName) const;

   /**
    * @short Returns @c true if the current element has an attribute whose
    * local name is @p localName and namespace URI is null.
    */
   inline bool hasAttribute(const QString &localName) const;

 private:
   typename TokenLookupClass::NodeName                                     m_currentElementName;

   /**
    * This member is private, see the error() and warning() functions in
    * this class.
    */
   const ReportContext::Ptr                                                m_context;

   /**
    * Returns the current location that QXmlStreamReader has.
    */
   inline QSourceLocation currentLocation() const;

   const typename ElementDescription<TokenLookupClass, LookupKey>::Hash    m_elementDescriptions;
   const QSet<typename TokenLookupClass::NodeName>                         m_standardAttributes;
   Q_DISABLE_COPY(MaintainingReader)
};

#include "qmaintainingreader.cpp"

}

QT_END_NAMESPACE

#endif

