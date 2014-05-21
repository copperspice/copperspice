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

#include "qplatformdefs.h"
#include <QtCore/qatomic.h>

QT_BEGIN_NAMESPACE
static pthread_mutex_t qAtomicMutex = PTHREAD_MUTEX_INITIALIZER;

Q_CORE_EXPORT
bool QBasicAtomicInt_testAndSetOrdered(volatile int *_q_value, int expectedValue, int newValue)
{
    bool returnValue = false;
    pthread_mutex_lock(&qAtomicMutex);
    if (*_q_value == expectedValue) {
        *_q_value = newValue;
        returnValue = true;
    }
    pthread_mutex_unlock(&qAtomicMutex);
    return returnValue;
}

Q_CORE_EXPORT
int QBasicAtomicInt_fetchAndStoreOrdered(volatile int *_q_value, int newValue)
{
    int returnValue;
    pthread_mutex_lock(&qAtomicMutex);
    returnValue = *_q_value;
    *_q_value = newValue;
    pthread_mutex_unlock(&qAtomicMutex);
    return returnValue;
}

Q_CORE_EXPORT
int QBasicAtomicInt_fetchAndAddOrdered(volatile int *_q_value, int valueToAdd)
{
    int returnValue;
    pthread_mutex_lock(&qAtomicMutex);
    returnValue = *_q_value;
    *_q_value += valueToAdd;
    pthread_mutex_unlock(&qAtomicMutex);
    return returnValue;
}

Q_CORE_EXPORT
bool QBasicAtomicPointer_testAndSetOrdered(void * volatile *_q_value,
                                           void *expectedValue,
                                           void *newValue)
{
    bool returnValue = false;
    pthread_mutex_lock(&qAtomicMutex);
    if (*_q_value == expectedValue) {
        *_q_value = newValue;
        returnValue = true;
    }
    pthread_mutex_unlock(&qAtomicMutex);
    return returnValue;
}

Q_CORE_EXPORT
void *QBasicAtomicPointer_fetchAndStoreOrdered(void * volatile *_q_value, void *newValue)
{
    void *returnValue;
    pthread_mutex_lock(&qAtomicMutex);
    returnValue = *_q_value;
    *_q_value = newValue;
    pthread_mutex_unlock(&qAtomicMutex);
    return returnValue;
}

Q_CORE_EXPORT
void *QBasicAtomicPointer_fetchAndAddOrdered(void * volatile *_q_value, qptrdiff valueToAdd)
{
    void *returnValue;
    pthread_mutex_lock(&qAtomicMutex);
    returnValue = *_q_value;
    *_q_value = reinterpret_cast<char *>(returnValue) + valueToAdd;
    pthread_mutex_unlock(&qAtomicMutex);
    return returnValue;
}
QT_END_NAMESPACE
