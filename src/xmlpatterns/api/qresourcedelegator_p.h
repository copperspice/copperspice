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

#ifndef QResourceDelegator_P_H
#define QResourceDelegator_P_H

#include <QSet>
#include <QUrl>
#include <qdeviceresourceloader_p.h>

namespace QPatternist {

class ResourceDelegator : public DeviceResourceLoader
{
 public:
   ResourceDelegator(const QSet<QUrl> &needsOverride, const ResourceLoader::Ptr &parentLoader,
                  const ResourceLoader::Ptr &forDeviceLoader) : m_needsOverride(needsOverride),
                  m_parentLoader(parentLoader), m_forDeviceLoader(forDeviceLoader)
   {
      Q_ASSERT(m_parentLoader);
   }

   bool isUnparsedTextAvailable(const QUrl &uri, const QString &encoding) override;
   ItemType::Ptr announceUnparsedText(const QUrl &uri) override;
   Item openUnparsedText(const QUrl &uri, const QString &encoding, const ReportContext::Ptr &context,
                  const SourceLocationReflection *const where) override;

   Item openDocument(const QUrl &uri, const ReportContext::Ptr &context) override;
   SequenceType::Ptr announceDocument(const QUrl &uri, const Usage usageHint) override;
   bool isDocumentAvailable(const QUrl &uri) override;
   Item::Iterator::Ptr openCollection(const QUrl &uri) override;
   SequenceType::Ptr announceCollection(const QUrl &uri) override;

   /**
    * Returns the union of the deviceURIs() that ResourceDelegator's two
    * resource loaders has.
    */
   QSet<QUrl> deviceURIs() const override;

 private:
   const QSet<QUrl> m_needsOverride;
   const ResourceLoader::Ptr m_parentLoader;
   const ResourceDelegator::Ptr m_forDeviceLoader;
};

}

#endif
