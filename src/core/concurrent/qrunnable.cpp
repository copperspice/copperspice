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
    \class QRunnable
    \since 4.4
    \brief The QRunnable class is the base class for all runnable objects.

    \ingroup thread

    The QRunnable class is an interface for representing a task or
    piece of code that needs to be executed, represented by your
    reimplementation of the run() function.

    You can use QThreadPool to execute your code in a separate
    thread. QThreadPool deletes the QRunnable automatically if
    autoDelete() returns true (the default). Use setAutoDelete() to
    change the auto-deletion flag.

    QThreadPool supports executing the same QRunnable more than once
    by calling QThreadPool::tryStart(this) from within the run() function.
    If autoDelete is enabled the QRunnable will be deleted when
    the last thread exits the run function. Calling QThreadPool::start()
    multiple times with the same QRunnable when autoDelete is enabled
    creates a race condition and is not recommended.

    \sa QThreadPool
*/

/*! \fn QRunnable::run()
    Implement this pure virtual function in your subclass.
*/

/*! \fn QRunnable::QRunnable()
    Constructs a QRunnable. Auto-deletion is enabled by default.

    \sa autoDelete(), setAutoDelete()
*/

/*! \fn QRunnable::~QRunnable()
    QRunnable virtual destructor.
*/

/*! \fn bool QRunnable::autoDelete() const

    Returns true is auto-deletion is enabled; false otherwise.

    If auto-deletion is enabled, QThreadPool will automatically delete
    this runnable after calling run(); otherwise, ownership remains
    with the application programmer.

    \sa setAutoDelete(), QThreadPool
*/

/*! \fn bool QRunnable::setAutoDelete(bool autoDelete)

    Enables auto-deletion if \a autoDelete is true; otherwise
    auto-deletion is disabled.

    If auto-deletion is enabled, QThreadPool will automatically delete
    this runnable after calling run(); otherwise, ownership remains
    with the application programmer.

    Note that this flag must be set before calling
    QThreadPool::start(). Calling this function after
    QThreadPool::start() results in undefined behavior.

    \sa autoDelete(), QThreadPool
*/
