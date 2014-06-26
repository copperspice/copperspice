/***********************************************************************
*
* Copyright (c) 2012-2014 Barbara Geller
* Copyright (c) 2012-2014 Ansel Sermersheim
* Copyright (c) 2012-2014 Digia Plc and/or its subsidiary(-ies).
* Copyright (c) 2008-2012 Nokia Corporation and/or its subsidiary(-ies).
* All rights reserved.
*
* This file is part of CopperSpice.
*
* CopperSpice is free software: you can redistribute it and/or
* modify it under the terms of the GNU Lesser General Public License
* version 2.1 as published by the Free Software Foundation.
*
* CopperSpice is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
* Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public
* License along with CopperSpice.  If not, see
* <http://www.gnu.org/licenses/>.
*
***********************************************************************/

#ifndef QResourceDelegator_P_H
#define QResourceDelegator_P_H

#include <QSet>
#include <QUrl>
#include <qdeviceresourceloader_p.h>

QT_BEGIN_NAMESPACE

namespace QPatternist {
class ResourceDelegator : public DeviceResourceLoader
{
 public:
   ResourceDelegator(const QSet<QUrl> &needsOverride,
                     const ResourceLoader::Ptr &parentLoader,
                     const ResourceLoader::Ptr &forDeviceLoader) : m_needsOverride(needsOverride)
      , m_parentLoader(parentLoader)
      , m_forDeviceLoader(forDeviceLoader)

   {
      Q_ASSERT(m_parentLoader);
   }

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

   /**
    * Returns the union of the deviceURIs() that ResourceDelegator's two
    * resource loaders has.
    */
   virtual QSet<QUrl> deviceURIs() const;

 private:
   const QSet<QUrl> m_needsOverride;
   const ResourceLoader::Ptr m_parentLoader;
   const ResourceDelegator::Ptr m_forDeviceLoader;
};
}

QT_END_NAMESPACE

#endif
