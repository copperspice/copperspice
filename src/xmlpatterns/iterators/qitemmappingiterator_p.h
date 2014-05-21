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

#ifndef Patternist_ItemMappingIterator_P_H
#define Patternist_ItemMappingIterator_P_H

#include "qabstractxmlforwarditerator_p.h"
#include "qdynamiccontext_p.h"

QT_BEGIN_NAMESPACE

namespace QPatternist
{

    template<typename TResult, typename TSource, typename TMapper, typename Context = DynamicContext::Ptr>
    class ItemMappingIterator : public QAbstractXmlForwardIterator<TResult>
    {
    public:
        /**
         * Constructs an ItemMappingIterator.
         *
         * @param mapper the object that has the mapToItem() sequence.
         * @param iterator the QAbstractXmlForwardIterator whose items should be mapped.
         * @param context the context that will be passed to the map function.
         * May be null.
         */
        ItemMappingIterator(const TMapper &mapper,
                            const typename QAbstractXmlForwardIterator<TSource>::Ptr &iterator,
                            const Context &context) : m_mapper(mapper)
                                                    , m_it(iterator)
                                                    , m_context(context)
                                                    , m_position(0)
        {
            Q_ASSERT(mapper);
            Q_ASSERT(iterator);
        }

        /**
         * @returns the next item in the sequence, or
         * @c null if the end have been reached.
         */
        virtual TResult next()
        {
            const TSource sourceItem(m_it->next());

            if(qIsForwardIteratorEnd(sourceItem))
            {
                m_current = TResult();
                m_position = -1;
                return TResult();
            }
            else
            {
                m_current = m_mapper->mapToItem(sourceItem, m_context);
                if(qIsForwardIteratorEnd(m_current))
                    return next(); /* The mapper returned null, so continue with the next in the source. */
                else
                {
                    ++m_position;
                    return m_current;
                }
            }
        }

        virtual TResult current() const
        {
            return m_current;
        }

        virtual xsInteger position() const
        {
            return m_position;
        }

        virtual typename QAbstractXmlForwardIterator<TResult>::Ptr copy() const
        {
            return typename QAbstractXmlForwardIterator<TResult>::Ptr
                (new ItemMappingIterator<TResult, TSource, TMapper, Context>(m_mapper, m_it->copy(), m_context));
        }

    private:
        const TMapper                                               m_mapper;
        const typename QAbstractXmlForwardIterator<TSource>::Ptr    m_it;
        const Context                                               m_context;
        TResult                                                     m_current;
        xsInteger                                                   m_position;
    };

    /**
     * @short An object generator for ItemMappingIterator.
     *
     * makeItemMappingIterator() is a convenience function for avoiding specifying
     * the full template instantiation for ItemMappingIterator. Conceptually, it
     * is identical to Qt's qMakePair().
     *
     * @relates ItemMappingIterator
     */
    template<typename TResult, typename TSource, typename TMapper, typename Context>
    static inline
    typename QAbstractXmlForwardIterator<TResult>::Ptr
    makeItemMappingIterator(const TMapper &mapper,
                            const QExplicitlySharedDataPointer<QAbstractXmlForwardIterator<TSource> > &source,
                            const Context &context)
    {
        return typename QAbstractXmlForwardIterator<TResult>::Ptr
            (new ItemMappingIterator<TResult, TSource, TMapper, Context>(mapper, source, context));
    }
}

QT_END_NAMESPACE

#endif
