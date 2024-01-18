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

#ifndef QSequenceMappingIterator_P_H
#define QSequenceMappingIterator_P_H

#include <qabstractxmlforwarditerator_p.h>
#include <qdynamiccontext_p.h>

namespace QPatternist {

template<typename TResult, typename TSource, typename TMapper>
class SequenceMappingIterator : public QAbstractXmlForwardIterator<TResult>
{
 public:
   /**
    * Constructs a SequenceMappingIterator.
    *
    * @param mapper the object that has the mapToItem() sequence.
    * @param sourceIterator the QAbstractXmlForwardIterator whose items should be mapped.
    * @param context the DynamicContext that will be passed to the map function.
    * May be null.
    */
   SequenceMappingIterator(const TMapper &mapper,
                           const typename QAbstractXmlForwardIterator<TSource>::Ptr &sourceIterator,
                           const DynamicContext::Ptr &context);

   TResult next() override;
   xsInteger count() override;
   TResult current() const override;
   xsInteger position() const override;

   /**
    * The reason the implementation is placed in line here, is due to a bug
    * in MSVC-2005 version 14.00.50727.762. Note that it works with version 14.00.50727.42.
    */
   typename QAbstractXmlForwardIterator<TResult>::Ptr copy() const override {
      return typename QAbstractXmlForwardIterator<TResult>::Ptr
             (new SequenceMappingIterator<TResult, TSource, TMapper>(m_mapper,
                   m_mainIterator->copy(), m_context));
   }

 private:
   xsInteger                                           m_position;
   TResult                                             m_current;
   typename QAbstractXmlForwardIterator<TSource>::Ptr  m_mainIterator;
   typename QAbstractXmlForwardIterator<TResult>::Ptr  m_currentIterator;
   const typename DynamicContext::Ptr                  m_context;
   const TMapper                                       m_mapper;
};

template<typename TResult, typename TSource, typename TMapper>
SequenceMappingIterator<TResult, TSource, TMapper>::SequenceMappingIterator(
   const TMapper &mapper, const typename QAbstractXmlForwardIterator<TSource>::Ptr &iterator,
   const DynamicContext::Ptr &context)
   : m_position(0), m_mainIterator(iterator), m_context(context), m_mapper(mapper)
{
   Q_ASSERT(mapper);
   Q_ASSERT(iterator);
}

template<typename TResult, typename TSource, typename TMapper>
TResult SequenceMappingIterator<TResult, TSource, TMapper>::next()
{
   /* This was once implemented with a recursive function, but the stack
    * got blown for some inputs by that approach. */
   while (true) {
      while (! m_currentIterator) {
         const TSource mainItem(m_mainIterator->next());

         if (qIsForwardIteratorEnd(mainItem)) { /* We've reached the very end. */
            m_position = -1;
            m_current = TResult();
            return TResult();
         } else {
            m_currentIterator = m_mapper->mapToSequence(mainItem, m_context);
         }
      }

      m_current = m_currentIterator->next();

      if (qIsForwardIteratorEnd(m_current)) {
         m_currentIterator.reset();
         continue;
      } else {
         ++m_position;
         return m_current;
      }
   }
}

template<typename TResult, typename TSource, typename TMapper>
xsInteger SequenceMappingIterator<TResult, TSource, TMapper>::count()
{
   TSource unit(m_mainIterator->next());
   xsInteger c = 0;

   while (!qIsForwardIteratorEnd(unit)) {
      const typename QAbstractXmlForwardIterator<TResult>::Ptr sit(m_mapper->mapToSequence(unit, m_context));
      c += sit->count();
      unit = m_mainIterator->next();
   }

   return c;
}

template<typename TResult, typename TSource, typename TMapper>
TResult SequenceMappingIterator<TResult, TSource, TMapper>::current() const
{
   return m_current;
}

template<typename TResult, typename TSource, typename TMapper>
xsInteger SequenceMappingIterator<TResult, TSource, TMapper>::position() const
{
   return m_position;
}

template<typename TResult, typename TSource, typename TMapper>
static inline typename QAbstractXmlForwardIterator<TResult>::Ptr
makeSequenceMappingIterator(const TMapper &mapper,
                  const QExplicitlySharedDataPointer<QAbstractXmlForwardIterator<TSource> > &source,
                  const DynamicContext::Ptr &context)
{
   return typename QAbstractXmlForwardIterator<TResult>::Ptr
          (new SequenceMappingIterator<TResult, TSource, TMapper>(mapper, source, context));
}

}

#endif
