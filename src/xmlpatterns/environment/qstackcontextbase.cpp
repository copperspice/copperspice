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

template<typename TSuperClass>
StackContextBase<TSuperClass>::StackContextBase() : m_rangeVariables(10),
   m_expressionVariables(10),
   m_positionIterators(5),
   m_itemCacheCells(5),
   m_itemSequenceCacheCells(5)
{
   /* The m_* containers are initialized with default sizes. Estimated guesses on usage patterns. */
}

template<typename TSuperClass>
StackContextBase<TSuperClass>::StackContextBase(const DynamicContext::Ptr &prevContext)
   : TSuperClass(prevContext),
     m_rangeVariables(10),
     m_expressionVariables(10),
     m_positionIterators(5),
     m_itemCacheCells(5),
     m_itemSequenceCacheCells(5)
{
   Q_ASSERT(prevContext);
}

template<typename TSuperClass>
ItemCacheCell &StackContextBase<TSuperClass>::itemCacheCell(const VariableSlotID slot)
{
   if (slot >= m_itemCacheCells.size()) {
      m_itemCacheCells.resize(qMax(slot + 1, m_itemCacheCells.size()));
   }

   return m_itemCacheCells[slot];
}

template<typename TSuperClass>
ItemSequenceCacheCell::Vector &StackContextBase<TSuperClass>::itemSequenceCacheCells(const VariableSlotID slot)
{
   if (slot >= m_itemSequenceCacheCells.size()) {
      m_itemSequenceCacheCells.resize(qMax(slot + 1, m_itemSequenceCacheCells.size()));
   }

   return m_itemSequenceCacheCells;
}

template<typename TSuperClass>
Item StackContextBase<TSuperClass>::rangeVariable(const VariableSlotID slot) const
{
   Q_ASSERT(slot < m_rangeVariables.size());
   Q_ASSERT(m_rangeVariables.at(slot));
   return m_rangeVariables.at(slot);
}

template<typename TSuperClass>
Expression::Ptr StackContextBase<TSuperClass>::expressionVariable(const VariableSlotID slot) const
{
   Q_ASSERT(slot < m_expressionVariables.size());
   Q_ASSERT(m_expressionVariables.at(slot));
   return m_expressionVariables.at(slot);
}

template<typename TSuperClass>
Item::Iterator::Ptr StackContextBase<TSuperClass>::positionIterator(const VariableSlotID slot) const
{
   Q_ASSERT(slot < m_positionIterators.size());
   return m_positionIterators.at(slot);
}

template<typename TSuperClass>
template<typename VectorType, typename UnitType>
inline
void StackContextBase<TSuperClass>::setSlotVariable(const VariableSlotID slot,
      const UnitType &newValue,
      VectorType &container) const
{
   if (slot < container.size()) {
      container.replace(slot, newValue);
   } else {
      container.resize(slot + 1);
      container.replace(slot, newValue);
   }
}

template<typename TSuperClass>
void StackContextBase<TSuperClass>::setRangeVariable(const VariableSlotID slot, const Item &newValue)
{
   setSlotVariable(slot, newValue, m_rangeVariables);
}

template<typename TSuperClass>
void StackContextBase<TSuperClass>::setExpressionVariable(const VariableSlotID slot,
      const Expression::Ptr &newValue)
{
   setSlotVariable(slot, newValue, m_expressionVariables);
}

template<typename TSuperClass>
void StackContextBase<TSuperClass>::setPositionIterator(const VariableSlotID slot,
      const Item::Iterator::Ptr &newValue)
{
   setSlotVariable(slot, newValue, m_positionIterators);
}

template<typename TSuperClass>
DynamicContext::TemplateParameterHash &StackContextBase<TSuperClass>::templateParameterStore()
{
   return m_templateParameterStore;
}

