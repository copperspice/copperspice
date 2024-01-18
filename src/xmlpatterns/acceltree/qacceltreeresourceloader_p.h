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

#ifndef QAccelTreeResourceLoader_P_H
#define QAccelTreeResourceLoader_P_H

#include <qeventloop.h>
#include <qnetwork_reply.h>

#include <qabstractxmlreceiver.h>
#include <qacceltree_p.h>
#include <qacceltreebuilder_p.h>
#include <qdeviceresourceloader_p.h>
#include <qnamepool_p.h>
#include <qnetworkaccessdelegator_p.h>
#include <qreportcontext_p.h>

class QIODevice;

namespace QPatternist {

class NetworkLoop : public QEventLoop
{
   XMLP_CS_OBJECT(NetworkLoop)

 public:
   NetworkLoop() : m_hasReceivedError(false) { }

   XMLP_CS_SLOT_1(Public, void error(QNetworkReply::NetworkError code))
   XMLP_CS_SLOT_2(error)

   XMLP_CS_SLOT_1(Public, void finished())
   XMLP_CS_SLOT_2(finished)

 private:
   bool m_hasReceivedError;
};

class AccelTreeResourceLoader : public DeviceResourceLoader
{
 public:
   /**
    * Describes the behaviour of the resource loader in case of an
    * error.
    */
   enum ErrorHandling {
      FailOnError,        ///< The resource loader will report the error via the report context.
      ContinueOnError     ///< The resource loader will report no error and return an empty QNetworkReply.
   };

   /**
    * AccelTreeResourceLoader does not own @p context.
    */
   AccelTreeResourceLoader(const NamePool::Ptr &np, const NetworkAccessDelegator::Ptr &networkDelegator,
                  AccelTreeBuilder<true>::Features = AccelTreeBuilder<true>::NoneFeature);

   Item openDocument(const QUrl &uri, const ReportContext::Ptr &context) override;
   virtual Item openDocument(QIODevice *source, const QUrl &documentUri, const ReportContext::Ptr &context);

   SequenceType::Ptr announceDocument(const QUrl &uri, const Usage usageHint) override;
   bool isDocumentAvailable(const QUrl &uri) override;

   bool isUnparsedTextAvailable(const QUrl &uri, const QString &encoding) override;

   Item openUnparsedText(const QUrl &uri, const QString &encoding, const ReportContext::Ptr &context,
                  const SourceLocationReflection *const where) override;

   static QNetworkReply *load(const QUrl &uri, QNetworkAccessManager *const networkManager,
                  const ReportContext::Ptr &context, ErrorHandling handling = FailOnError);

   /**
    * @overload
    */
   static QNetworkReply *load(const QUrl &uri, const NetworkAccessDelegator::Ptr &networkDelegator,
                  const ReportContext::Ptr &context, ErrorHandling handling = FailOnError);

   /**
    * @short Returns the URIs this AccelTreeResourceLoader has loaded
    * which are for devices through variable bindings.
    */
   QSet<QUrl> deviceURIs() const override;

   void clear(const QUrl &uri) override;

 private:
   static bool streamToReceiver(QIODevice *const dev, AccelTreeBuilder<true> *const receiver,
                  const NamePool::Ptr &np, const ReportContext::Ptr &context, const QUrl &uri);

   bool retrieveDocument(const QUrl &uri, const ReportContext::Ptr &context);
   bool retrieveDocument(QIODevice *source, const QUrl &documentUri, const ReportContext::Ptr &context);
   /**
    * If @p context is @c null, no error reporting should be done.
    */
   bool retrieveUnparsedText(const QUrl &uri, const QString &encoding, const ReportContext::Ptr &context,
                  const SourceLocationReflection *const where);

   QHash<QUrl, AccelTree::Ptr>             m_loadedDocuments;
   const NamePool::Ptr                     m_namePool;
   const NetworkAccessDelegator::Ptr       m_networkAccessDelegator;
   QHash<QPair<QUrl, QString>, QString>    m_unparsedTexts;
   AccelTreeBuilder<true>::Features        m_features;
};

}

#endif
