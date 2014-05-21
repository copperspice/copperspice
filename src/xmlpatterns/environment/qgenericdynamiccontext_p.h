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

#ifndef Patternist_GenericDynamicContext_P_H
#define Patternist_GenericDynamicContext_P_H

#include <QDateTime>
#include <QVector>

#include "qdaytimeduration_p.h"
#include "qstackcontextbase_p.h"
#include "qexpression_p.h"

QT_BEGIN_NAMESPACE

namespace QPatternist
{
  
    class GenericDynamicContext : public StackContextBase<DynamicContext>
    {
    public:
        typedef QExplicitlySharedDataPointer<GenericDynamicContext> Ptr;

        GenericDynamicContext(const NamePool::Ptr &np,
                              QAbstractMessageHandler *const messageHandler,
                              const LocationHash &locations);

        virtual xsInteger contextPosition() const;
        /**
         * @returns always @c null, the focus is always undefined when an GenericDynamicContext
         * is used.
         */
        virtual Item contextItem() const;
        virtual xsInteger contextSize();

        virtual void setFocusIterator(const Item::Iterator::Ptr &it);
        virtual Item::Iterator::Ptr focusIterator() const;

        virtual QAbstractMessageHandler * messageHandler() const;
        virtual QExplicitlySharedDataPointer<DayTimeDuration> implicitTimezone() const;
        virtual QDateTime currentDateTime() const;

        virtual QAbstractXmlReceiver *outputReceiver() const;
        void setOutputReceiver(QAbstractXmlReceiver *const receiver);

        virtual NodeBuilder::Ptr nodeBuilder(const QUrl &baseURI) const;
        void setNodeBuilder(NodeBuilder::Ptr &builder);

        virtual ResourceLoader::Ptr resourceLoader() const;
        void setResourceLoader(const ResourceLoader::Ptr &loader);

        virtual ExternalVariableLoader::Ptr externalVariableLoader() const;
        void setExternalVariableLoader(const ExternalVariableLoader::Ptr &loader);
        virtual NamePool::Ptr namePool() const;
        virtual QSourceLocation locationFor(const SourceLocationReflection *const reflection) const;
        virtual void addNodeModel(const QAbstractXmlNodeModel::Ptr &nm);
        virtual const QAbstractUriResolver *uriResolver() const;
        virtual ItemCacheCell &globalItemCacheCell(const VariableSlotID slot);
        virtual ItemSequenceCacheCell::Vector &globalItemSequenceCacheCells(const VariableSlotID slot);

        void setUriResolver(const QAbstractUriResolver *const resolver);

        /**
         * We return a null item, we have no focus.
         */
        virtual Item currentItem() const;

        /**
         * @short Returns always @c null, since we're always
         * a top-level context.
         */
        virtual DynamicContext::Ptr previousContext() const;

        virtual QExplicitlySharedDataPointer<TemplateMode> currentTemplateMode() const;

    private:
        QAbstractMessageHandler *       m_messageHandler;
        const QDateTime                 m_currentDateTime;
        const DayTimeDuration::Ptr      m_zoneOffset;
        QAbstractXmlReceiver *          m_outputReceiver;
        mutable NodeBuilder::Ptr        m_nodeBuilder;
        ExternalVariableLoader::Ptr     m_externalVariableLoader;
        ResourceLoader::Ptr             m_resourceLoader;
        NamePool::Ptr                   m_namePool;
        const LocationHash              m_locations;
        QAbstractXmlNodeModel::List     m_nodeModels;
        const QAbstractUriResolver *    m_uriResolver;
        ItemCacheCell::Vector           m_globalItemCacheCells;
        ItemSequenceCacheCell::Vector   m_globalItemSequenceCacheCells;
    };
}

QT_END_NAMESPACE

#endif
