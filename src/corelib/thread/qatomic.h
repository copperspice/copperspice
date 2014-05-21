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

#include <QtCore/qglobal.h>

#ifndef QATOMIC_H
#define QATOMIC_H

#include <QtCore/qbasicatomic.h>

QT_BEGIN_NAMESPACE

#if defined(__GNUC__) && (__GNUC__ * 100 + __GNUC_MINOR__ >= 406)
# pragma GCC diagnostic push
# pragma GCC diagnostic ignored "-Wextra"
#endif

// High-level atomic integer operations
class Q_CORE_EXPORT QAtomicInt : public QBasicAtomicInt
{
public:
    // Non-atomic API
    inline QAtomicInt(int value = 0)
    {
#ifdef QT_ARCH_PARISC
        this->_q_lock[0] = this->_q_lock[1] = this->_q_lock[2] = this->_q_lock[3] = -1;
#endif
        _q_value = value;
    }

    inline QAtomicInt(const QAtomicInt &other)
    {
#ifdef QT_ARCH_PARISC
        this->_q_lock[0] = this->_q_lock[1] = this->_q_lock[2] = this->_q_lock[3] = -1;
#endif
        this->store(other.load());
    }

    inline QAtomicInt &operator=(const QAtomicInt &other)
    {
        this->store(other.load());
        return *this;
    }
};

// High-level atomic pointer operations
template <typename T>
class QAtomicPointer : public QBasicAtomicPointer<T>
{
public:
    inline QAtomicPointer(T *value = 0)
    {
#ifdef QT_ARCH_PARISC
        this->_q_lock[0] = this->_q_lock[1] = this->_q_lock[2] = this->_q_lock[3] = -1;
#endif
        this->store(value);
    }
    inline QAtomicPointer(const QAtomicPointer<T> &other)
    {
#ifdef QT_ARCH_PARISC
        this->_q_lock[0] = this->_q_lock[1] = this->_q_lock[2] = this->_q_lock[3] = -1;
#endif
        this->store(other.load());
    }

    inline QAtomicPointer<T> &operator=(const QAtomicPointer<T> &other)
    {
        this->store(other.load());
        return *this;
    }
};

#if defined(__GNUC__) && (__GNUC__ * 100 + __GNUC_MINOR__ >= 406)
# pragma GCC diagnostic pop
#endif

/*!
    This is a helper for the assignment operators of implicitly
    shared classes. Your assignment operator should look like this:

    \snippet doc/src/snippets/code/src.corelib.thread.qatomic.h 0
*/
template <typename T>
inline void qAtomicAssign(T *&d, T *x)
{
    if (d == x)
        return;
    x->ref.ref();
    if (!d->ref.deref())
        delete d;
    d = x;
}

/*!
    This is a helper for the detach method of implicitly shared
    classes. Your private class needs a copy constructor which copies
    the members and sets the refcount to 1. After that, your detach
    function should look like this:

    \snippet doc/src/snippets/code/src.corelib.thread.qatomic.h 1
*/
template <typename T>
inline void qAtomicDetach(T *&d)
{
    if (d->ref.load() == 1)
        return;
    T *x = d;
    d = new T(*d);
    if (!x->ref.deref())
        delete x;
}

QT_END_NAMESPACE

#endif // QATOMIC_H
