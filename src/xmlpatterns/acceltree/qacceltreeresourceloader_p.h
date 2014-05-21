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

#ifndef QAccelTreeResourceLoader_P_H
#define QAccelTreeResourceLoader_P_H

#include <QtCore/QEventLoop>
#include <QtNetwork/QNetworkReply>

#include "qabstractxmlreceiver.h"
#include "qacceltree_p.h"
#include "qacceltreebuilder_p.h"
#include "qdeviceresourceloader_p.h"
#include "qnamepool_p.h"
#include "qnetworkaccessdelegator_p.h"
#include "qreportcontext_p.h"

QT_BEGIN_NAMESPACE

class QIODevice;

namespace QPatternist
{

    class NetworkLoop : public QEventLoop
    {
        CS_OBJECT(NetworkLoop)

    public:
        NetworkLoop() : m_hasReceivedError(false)
        {
        }
    
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
        enum ErrorHandling
        {
            FailOnError,        ///< The resource loader will report the error via the report context.
            ContinueOnError     ///< The resource loader will report no error and return an empty QNetworkReply.
        };

        /**
         * AccelTreeResourceLoader does not own @p context.
         */
        AccelTreeResourceLoader(const NamePool::Ptr &np,
                                const NetworkAccessDelegator::Ptr &networkDelegator, AccelTreeBuilder<true>::Features = AccelTreeBuilder<true>::NoneFeature);

        virtual Item openDocument(const QUrl &uri,
                                  const ReportContext::Ptr &context);
        virtual Item openDocument(QIODevice *source, const QUrl &documentUri,
                                  const ReportContext::Ptr &context);
        virtual SequenceType::Ptr announceDocument(const QUrl &uri, const Usage usageHint);
        virtual bool isDocumentAvailable(const QUrl &uri);

        virtual bool isUnparsedTextAvailable(const QUrl &uri,
                                             const QString &encoding);

        virtual Item openUnparsedText(const QUrl &uri,
                                      const QString &encoding,
                                      const ReportContext::Ptr &context,
                                      const SourceLocationReflection *const where);

        /**
         * @short Helper function that do NetworkAccessDelegator::get(), but
         * does it blocked.
         *
         * The returned QNetworkReply has emitted QNetworkReply::finished().
         *
         * The caller owns the return QIODevice instance.
         *
         * @p context may be @c null or valid. If @c null, no error reporting
         * is done and @c null is returned.
         *
         * @see NetworkAccessDelegator
         */
        static QNetworkReply *load(const QUrl &uri,
                                   QNetworkAccessManager *const networkManager,
                                   const ReportContext::Ptr &context, ErrorHandling handling = FailOnError);

        /**
         * @overload
         */
        static QNetworkReply *load(const QUrl &uri,
                                   const NetworkAccessDelegator::Ptr &networkDelegator,
                                   const ReportContext::Ptr &context, ErrorHandling handling = FailOnError);

        /**
         * @short Returns the URIs this AccelTreeResourceLoader has loaded
         * which are for devices through variable bindings.
         */
        virtual QSet<QUrl> deviceURIs() const;

        virtual void clear(const QUrl &uri);

    private:
        static bool streamToReceiver(QIODevice *const dev,
                                     AccelTreeBuilder<true> *const receiver,
                                     const NamePool::Ptr &np,
                                     const ReportContext::Ptr &context,
                                     const QUrl &uri);
        bool retrieveDocument(const QUrl &uri,
                              const ReportContext::Ptr &context);
        bool retrieveDocument(QIODevice *source, const QUrl &documentUri,
                              const ReportContext::Ptr &context);
        /**
         * If @p context is @c null, no error reporting should be done.
         */
        bool retrieveUnparsedText(const QUrl &uri,
                                  const QString &encoding,
                                  const ReportContext::Ptr &context,
                                  const SourceLocationReflection *const where);

        QHash<QUrl, AccelTree::Ptr>             m_loadedDocuments;
        const NamePool::Ptr                     m_namePool;
        const NetworkAccessDelegator::Ptr       m_networkAccessDelegator;
        QHash<QPair<QUrl, QString>, QString>    m_unparsedTexts;
        AccelTreeBuilder<true>::Features        m_features;
    };
}

QT_END_NAMESPACE

#endif
