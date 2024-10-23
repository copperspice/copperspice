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

#ifndef QResourceLoader_P_H
#define QResourceLoader_P_H

#include "qitem_p.h"
#include "qreportcontext_p.h"
#include "qsequencetype_p.h"
#include "qsourcelocationreflection_p.h"

class QUrl;

namespace QPatternist {
class ResourceLoader : public QSharedData
{
 public:
   enum Usage {
      // Communicates the URI may be used during query evaluation.
      // Typically this hint is given when the URI is available at
      // compile-time, but it is used inside a conditional statement
      // whose branching cannot be determined at compile time.
      MayUse,

      // Communicates the URI will always be used at query evaluation.
      WillUse
   };

   typedef QExplicitlySharedDataPointer<ResourceLoader> Ptr;
   virtual ~ResourceLoader();

   ResourceLoader()
   { }
   virtual bool isUnparsedTextAvailable(const QUrl &uri,
                                        const QString &encoding);

   virtual ItemType::Ptr announceUnparsedText(const QUrl &uri);

   virtual Item openUnparsedText(const QUrl &uri,
                                 const QString &encoding,
                                 const ReportContext::Ptr &context,
                                 const SourceLocationReflection *const where);

   virtual Item openDocument(const QUrl &uri,
                             const ReportContext::Ptr &context);

   virtual SequenceType::Ptr announceDocument(const QUrl &uri, const Usage usageHint);

   virtual bool isDocumentAvailable(const QUrl &uri);

   virtual Item::Iterator::Ptr openCollection(const QUrl &uri);

   virtual SequenceType::Ptr announceCollection(const QUrl &uri);

   virtual void clear(const QUrl &uri);
};
}

#endif
