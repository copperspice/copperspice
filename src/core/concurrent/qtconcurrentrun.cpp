/***********************************************************************
*
* Copyright (c) 2012-2018 Barbara Geller
* Copyright (c) 2012-2018 Ansel Sermersheim
* Copyright (c) 2012-2016 Digia Plc and/or its subsidiary(-ies).
* Copyright (c) 2008-2012 Nokia Corporation and/or its subsidiary(-ies).
* All rights reserved.
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
* <http://www.gnu.org/licenses/>.
*
***********************************************************************/

/*!
    \headerfile <QtConcurrentRun>
    \title Asynchronous Run

    \brief The <QtConcurrentRun> header provides a way to run a function in a
    separate thread.

    \ingroup thread

    This function is a part of the \l {Concurrent Programming}{Qt Concurrent} framework.

    The QtConcurrent::run() function runs a function in a separate thread.
    The return value of the function is made available through the QFuture API.

    \section1 Running a Function in a Separate Thread

    To run a function in another thread, use QtConcurrent::run():

    \snippet doc/src/snippets/code/src_corelib_concurrent_qtconcurrentrun.cpp 0

    This will run \e aFunction in a separate thread obtained from the default
    QThreadPool. You can use the QFuture and QFutureWatcher classes to monitor
    the status of the function.

    \section1 Passing Arguments to the Function

    Passing arguments to the function is done by adding them to the
    QtConcurrent::run() call immediately after the function name. For example:

    \snippet doc/src/snippets/code/src_corelib_concurrent_qtconcurrentrun.cpp 1

    A copy of each argument is made at the point where QtConcurrent::run() is
    called, and these values are passed to the thread when it begins executing
    the function. Changes made to the arguments after calling
    QtConcurrent::run() are \e not visible to the thread.

    \section1 Returning Values from the Function

    Any return value from the function is available via QFuture:

    \snippet doc/src/snippets/code/src_corelib_concurrent_qtconcurrentrun.cpp 2

    As documented above, passing arguments is done like this:

    \snippet doc/src/snippets/code/src_corelib_concurrent_qtconcurrentrun.cpp 3

    Note that the QFuture::result() function blocks and waits for the result
    to become available. Use QFutureWatcher to get notification when the
    function has finished execution and the result is available.

    \section1 Additional API Features

    \section2 Using Member Functions

    QtConcurrent::run() also accepts pointers to member functions. The first
    argument must be either a const reference or a pointer to an instance of
    the class. Passing by const reference is useful when calling const member
    functions; passing by pointer is useful for calling non-const member
    functions that modify the instance.

    For example, calling QByteArray::split() (a const member function) in a
    separate thread is done like this:

    \snippet doc/src/snippets/code/src_corelib_concurrent_qtconcurrentrun.cpp 4

    Calling a non-const member function is done like this:

    \snippet doc/src/snippets/code/src_corelib_concurrent_qtconcurrentrun.cpp 5

    \section2 Using Bound Function Arguments

    Note that Qt does not provide support for bound functions. This is
    provided by 3rd party libraries like
    \l{http://www.boost.org/libs/bind/bind.html}{Boost} or
    \l{http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2005/n1836.pdf}
    {C++ TR1 Library Extensions}.

    You can use boost::bind() or std::tr1::bind() to \e bind a number of
    arguments to a function when called. There are number of reasons for doing
    this:

    \list
    \o To call a function that takes more than 5 arguments.
    \o To simplify calling a function with constant arguments.
    \o Changing the order of arguments.
    \endlist

    See the documentation for the relevant functions for details on how to use
    the bind API.

    Calling a bound function is done like this:

    \snippet doc/src/snippets/code/src_corelib_concurrent_qtconcurrentrun.cpp 6
*/

/*!
    \fn QFuture<T> QtConcurrent::run(Function function, ...);
    \relates <QtConcurrentRun>

    Runs \a function in a separate thread. The thread is taken from the global
    QThreadPool. Note that the function may not run immediately; the function
    will only be run when a thread is available.

    T is the same type as the return value of \a function. Non-void return
    values can be accessed via the QFuture::result() function.

    Note that the QFuture returned by QtConcurrent::run() does not support
    canceling, pausing, or progress reporting. The QFuture returned can only
    be used to query for the running/finished status and the return value of
    the function.
*/
