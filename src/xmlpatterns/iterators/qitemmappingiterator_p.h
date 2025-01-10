/***********************************************************************
*
* Copyright (c) 2012-2025 Barbara Geller
* Copyright (c) 2012-2025 Ansel Sermersheim
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

#ifndef QItemMappingIterator_P_H
#define QItemMappingIterator_P_H

#include <qabstractxmlforwarditerator_p.h>
#include <qdynamiccontext_p.h>

namespace QPatternist {

template<typename TResult, typename TSource, typename TMapper, typename Context = DynamicContext::Ptr>
class ItemMappingIterator : public QAbstractXmlForwardIterator<TResult>
{
 public:
   ItemMappingIterator(const TMapper &mapper, const typename QAbstractXmlForwardIterator<TSource>::Ptr &iterator, const Context &context)
      : m_mapper(mapper), m_it(iterator), m_context(context), m_position(0)
   {
      Q_ASSERT(mapper);
      Q_ASSERT(iterator);
   }

   TResult next() override {
      const TSource sourceItem(m_it->next());

      if (qIsForwardIteratorEnd(sourceItem)) {
         m_current = TResult();
         m_position = -1;
         return TResult();
      } else {
         m_current = m_mapper->mapToItem(sourceItem, m_context);
         if (qIsForwardIteratorEnd(m_current)) {
            return next();   /* The mapper returned null, so continue with the next in the source. */
         } else {
            ++m_position;
            return m_current;
         }
      }
   }

   TResult current() const  override {
      return m_current;
   }

   xsInteger position() const override {
      return m_position;
   }

   typename QAbstractXmlForwardIterator<TResult>::Ptr copy() const  override {
      return typename QAbstractXmlForwardIterator<TResult>::Ptr
             (new ItemMappingIterator<TResult, TSource, TMapper, Context>(m_mapper, m_it->copy(), m_context));
   }

 private:
   const TMapper m_mapper;
   const typename QAbstractXmlForwardIterator<TSource>::Ptr m_it;
   const Context m_context;

   TResult m_current;
   xsInteger m_position;
};

template<typename TResult, typename TSource, typename TMapper, typename Context>
static inline typename QAbstractXmlForwardIterator<TResult>::Ptr makeItemMappingIterator(const TMapper &mapper,
      const QExplicitlySharedDataPointer<QAbstractXmlForwardIterator<TSource> > &source, const Context &context)
{
   return typename QAbstractXmlForwardIterator<TResult>::Ptr
          (new ItemMappingIterator<TResult, TSource, TMapper, Context>(mapper, source, context));
}

}

#endif
