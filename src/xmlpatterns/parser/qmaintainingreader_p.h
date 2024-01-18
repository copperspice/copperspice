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

#ifndef QMaintainingReader_P_H
#define QMaintainingReader_P_H

#include <qset.h>
#include <qsourcelocation.h>
#include <qstack.h>
#include <qstringlist.h>
#include <qxmlstreamreader.h>

#include <qxpathhelper_p.h>
#include <qxslttokenlookup_p.h>

class QUrl;

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

   inline typename TokenLookupClass::NodeName currentElementName() const;
   void error(const QString &message, const ReportContext::ErrorCode code) const;
   void warning(const QString &message) const;

   virtual QUrl documentURI() const = 0;
   virtual bool isAnyAttributeAllowed() const = 0;

   bool isWhitespace() const;
   void validateElement(const LookupKey name) const;
   QString readAttribute(const QString &localName, const QString &namespaceURI = QString()) const;
   bool hasAttribute(const QString &namespaceURI, const QString &localName) const;
   inline bool hasAttribute(const QString &localName) const;

   QXmlStreamAttributes m_currentAttributes;
   bool m_hasHandledStandardAttributes;
   QStack<bool> m_stripWhitespace;

 private:
   MaintainingReader(const MaintainingReader &) = delete;
   MaintainingReader &operator=(const MaintainingReader &) = delete;

   inline QSourceLocation currentLocation() const;

   typename TokenLookupClass::NodeName m_currentElementName;
   const ReportContext::Ptr m_context;
   const typename ElementDescription<TokenLookupClass, LookupKey>::Hash m_elementDescriptions;
   const QSet<typename TokenLookupClass::NodeName> m_standardAttributes;
};

#include "qmaintainingreader.cpp"

}

#endif

