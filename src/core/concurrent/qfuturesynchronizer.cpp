/***********************************************************************
*
* Copyright (c) 2012-2019 Barbara Geller
* Copyright (c) 2012-2019 Ansel Sermersheim
*
* Copyright (C) 2015 The Qt Company Ltd.
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

/*! \class QFutureSynchronizer
    \since 4.4

    \brief The QFutureSynchronizer class is a convenience class that simplifies
    QFuture synchronization.

    \ingroup thread

    QFutureSynchronizer is a template class that simplifies synchronization of
    one or more QFuture objects. Futures are added using the addFuture() or
    setFuture() functions. The futures() function returns a list of futures.
    Use clearFutures() to remove all futures from the QFutureSynchronizer.

    The waitForFinished() function waits for all futures to finish.
    The destructor of QFutureSynchronizer calls waitForFinished(), providing
    an easy way to ensure that all futures have finished before returning from
    a function:

    \snippet doc/src/snippets/code/src_corelib_concurrent_qfuturesynchronizer.cpp 0

    The behavior of waitForFinished() can be changed using the
    setCancelOnWait() function. Calling setCancelOnWait(true) will cause
    waitForFinished() to cancel all futures before waiting for them to finish.
    You can query the status of the cancel-on-wait feature using the
    cancelOnWait() function.

    \sa QFuture, QFutureWatcher, {Concurrent Programming}{Qt Concurrent}
*/

/*!
    \fn QFutureSynchronizer::QFutureSynchronizer()

    Constructs a QFutureSynchronizer.
*/

/*!
    \fn QFutureSynchronizer::QFutureSynchronizer(const QFuture<T> &future)

    Constructs a QFutureSynchronizer and begins watching \a future by calling
    addFuture().

    \sa addFuture()
*/

/*!
    \fn QFutureSynchronizer::~QFutureSynchronizer()

    Calls waitForFinished() function to ensure that all futures have finished
    before destroying this QFutureSynchronizer.

    \sa waitForFinished()
*/

/*!
    \fn void QFutureSynchronizer::setFuture(const QFuture<T> &future)

    Sets \a future to be the only future managed by this QFutureSynchronizer.
    This is a convenience function that calls waitForFinished(),
    then clearFutures(), and finally passes \a future to addFuture().

    \sa addFuture(), waitForFinished(), clearFutures()
*/

/*!
    \fn void QFutureSynchronizer::addFuture(const QFuture<T> &future)

    Adds \a future to the list of managed futures.

    \sa futures()
*/

/*!
    \fn void QFutureSynchronizer::waitForFinished()

    Waits for all futures to finish. If cancelOnWait() returns true, each
    future is canceled before waiting for them to finish.

    \sa cancelOnWait(), setCancelOnWait()
*/

/*!
    \fn void QFutureSynchronizer::clearFutures()

    Removes all managed futures from this QFutureSynchronizer.

    \sa addFuture(), setFuture()
*/

/*!
    \fn QList<QFuture<T> > QFutureSynchronizer::futures() const

    Returns a list of all managed futures.

    \sa addFuture(), setFuture()
*/

/*!
    \fn void QFutureSynchronizer::setCancelOnWait(bool enabled)

    Enables or disables the cancel-on-wait feature based on the \a enabled
    argument. If \a enabled is true, the waitForFinished() function will cancel
    all futures before waiting for them to finish.

    \sa waitForFinished()
*/

/*!
    \fn bool QFutureSynchronizer::cancelOnWait() const

    Returns true if the cancel-on-wait feature is enabled; otherwise returns
    false. If cancel-on-wait is enabled, the waitForFinished() function will
    cancel all futures before waiting for them to finish.

    \sa waitForFinished()
*/
