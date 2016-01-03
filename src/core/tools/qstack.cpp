/***********************************************************************
*
* Copyright (c) 2012-2016 Barbara Geller
* Copyright (c) 2012-2016 Ansel Sermersheim
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

/*!
    \class QStack
    \brief The QStack class is a template class that provides a stack.

    \ingroup tools
    \ingroup shared

    \reentrant

    QStack\<T\> is one of Qt's generic \l{container classes}. It implements
    a stack data structure for items of a same type.

    A stack is a last in, first out (LIFO) structure. Items are added
    to the top of the stack using push() and retrieved from the top
    using pop(). The top() function provides access to the topmost
    item without removing it.

    Example:

    \snippet doc/src/snippets/qstack/main.cpp 0

    The example will output 3, 2, 1 in that order.

    QStack inherits from QVector. All of QVector's functionality also
    applies to QStack. For example, you can use isEmpty() to test
    whether the stack is empty, and you can traverse a QStack using
    QVector's iterator classes (for example, QVectorIterator). But in
    addition, QStack provides three convenience functions that make
    it easy to implement LIFO semantics: push(), pop(), and top().

    QStack's value type must be an \l{assignable data type}. This
    covers most data types that are commonly used, but the compiler
    won't let you, for example, store a QWidget as a value; instead,
    store a QWidget *.

    \sa QVector, QQueue
*/

/*!
    \fn QStack::QStack()

    Constructs an empty stack.
*/

/*!
    \fn QStack::~QStack()

    Destroys the stack. References to the values in the stack, and all
    iterators over this stack, become invalid.
*/

/*!
    \fn void QStack::swap(QStack<T> &other)
    \since 4.8

    Swaps stack \a other with this stack. This operation is very fast and
    never fails.
*/

/*!
    \fn void QStack::push(const T& t)

    Adds element \a t to the top of the stack.

    This is the same as QVector::append().

    \sa pop(), top()
*/

/*!
    \fn T& QStack::top()

    Returns a reference to the stack's top item. This function
    assumes that the stack isn't empty.

    This is the same as QVector::last().

    \sa pop(), push(), isEmpty()
*/

/*!
    \fn const T& QStack::top() const

    \overload

    \sa pop(), push()
*/

/*!
    \fn T QStack::pop()

    Removes the top item from the stack and returns it. This function
    assumes that the stack isn't empty.

    \sa top(), push(), isEmpty()
*/
