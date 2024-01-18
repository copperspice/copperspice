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

#include "qscriptdebuggerfrontend_p.h"
#include "qscriptdebuggerfrontend_p_p.h"
#include "qscriptdebuggercommand_p.h"
#include "qscriptdebuggerevent_p.h"
#include "qscriptdebuggerresponse_p.h"
#include "qscriptdebuggereventhandlerinterface_p.h"
#include "qscriptdebuggerresponsehandlerinterface_p.h"

#include <QtCore/qcoreevent.h>
#include <QtCore/qcoreapplication.h>

QT_BEGIN_NAMESPACE

/*!
  \class QScriptDebuggerFrontend
  \since 4.5
  \internal

  \brief The QScriptDebuggerFrontend class is the base class of debugger front-ends.
*/

// helper class that's used to handle our custom Qt events
class QScriptDebuggerFrontendEventReceiver : public QObject
{
 public:
   QScriptDebuggerFrontendEventReceiver(QScriptDebuggerFrontendPrivate *frontend,
                                        QObject *parent = nullptr);
   ~QScriptDebuggerFrontendEventReceiver();

   bool event(QEvent *);

 private:
   QScriptDebuggerFrontendPrivate *m_frontend;
};

QScriptDebuggerFrontendEventReceiver::QScriptDebuggerFrontendEventReceiver(
   QScriptDebuggerFrontendPrivate *frontend, QObject *parent)
   : QObject(parent), m_frontend(frontend)
{
}

QScriptDebuggerFrontendEventReceiver::~QScriptDebuggerFrontendEventReceiver()
{
}

bool QScriptDebuggerFrontendEventReceiver::event(QEvent *e)
{
   return m_frontend->event(e);
}


QScriptDebuggerFrontendPrivate::QScriptDebuggerFrontendPrivate()
{
   eventHandler = 0;
   nextCommandId = 0;
   eventReceiver = new QScriptDebuggerFrontendEventReceiver(this);
}

QScriptDebuggerFrontendPrivate::~QScriptDebuggerFrontendPrivate()
{
   delete eventReceiver;
}

void QScriptDebuggerFrontendPrivate::postEvent(QEvent *e)
{
   QCoreApplication::postEvent(eventReceiver, e);
}

bool QScriptDebuggerFrontendPrivate::event(QEvent *e)
{
   Q_Q(QScriptDebuggerFrontend);
   if (e->type() == QEvent::User + 1) {
      QScriptDebuggerEventEvent *de = static_cast<QScriptDebuggerEventEvent *>(e);
      bool handled = q->notifyEvent(de->event());
      if (handled) {
         q->scheduleCommand(QScriptDebuggerCommand::resumeCommand(),
                            /*responseHandler=*/0);
      }
      return true;
   } else if (e->type() == QEvent::User + 2) {
      processCommands();
      return true;
   }
   return false;
}

void QScriptDebuggerFrontendPrivate::processCommands()
{
   Q_Q(QScriptDebuggerFrontend);
   while (!pendingCommands.isEmpty()) {
      QScriptDebuggerCommand command(pendingCommands.takeFirst());
      int id = pendingCommandIds.takeFirst();
      q->processCommand(id, command);
   }
}

QScriptDebuggerFrontend::QScriptDebuggerFrontend()
   : d_ptr(new QScriptDebuggerFrontendPrivate())
{
   d_ptr->q_ptr = this;
}

QScriptDebuggerFrontend::~QScriptDebuggerFrontend()
{
}

QScriptDebuggerFrontend::QScriptDebuggerFrontend(QScriptDebuggerFrontendPrivate &dd)
   : d_ptr(&dd)
{
   d_ptr->q_ptr = this;
}

QScriptDebuggerEventHandlerInterface *QScriptDebuggerFrontend::eventHandler() const
{
   Q_D(const QScriptDebuggerFrontend);
   return d->eventHandler;
}

void QScriptDebuggerFrontend::setEventHandler(QScriptDebuggerEventHandlerInterface *eventHandler)
{
   Q_D(QScriptDebuggerFrontend);
   d->eventHandler = eventHandler;
}

/*!
  Schedules the given \a command for execution by this front-end,
  and returns a unique identifier associated with this command.

  Subclasses can call this function to schedule custom commands.

  \sa notifyCommandFinished()
*/
int QScriptDebuggerFrontend::scheduleCommand(
   const QScriptDebuggerCommand &command,
   QScriptDebuggerResponseHandlerInterface *responseHandler)
{
   Q_D(QScriptDebuggerFrontend);
   int id = ++d->nextCommandId;
   d->pendingCommands.append(command);
   d->pendingCommandIds.append(id);
   if (responseHandler) {
      d->responseHandlers.insert(id, responseHandler);
   }
   if (d->pendingCommands.size() == 1) {
      QEvent *e = new QEvent(QEvent::Type(QEvent::User + 2)); // ProcessCommands
      d->postEvent(e);
   }
   return id;
}

/*!
  Subclasses should call this function when the command identified by
  the given \a id has finished and produced the given \a response.

  \sa processCommand(), notifyEvent()
*/
void QScriptDebuggerFrontend::notifyCommandFinished(int id, const QScriptDebuggerResponse &response)
{
   Q_D(QScriptDebuggerFrontend);
   if (d->responseHandlers.contains(id)) {
      QScriptDebuggerResponseHandlerInterface *handler = d->responseHandlers.take(id);
      Q_ASSERT(handler != 0);
      handler->handleResponse(response, id);
   }
}

/*!
  Subclasses should call this function when the given \a event is
  received from the back-end.

  \sa notifyCommandFinished(), QScriptDebuggerBackend::event()
*/
bool QScriptDebuggerFrontend::notifyEvent(const QScriptDebuggerEvent &event)
{
   Q_D(QScriptDebuggerFrontend);
   if (d->eventHandler) {
      return d->eventHandler->debuggerEvent(event);
   }
   return false;
}

int QScriptDebuggerFrontend::scheduledCommandCount() const
{
   Q_D(const QScriptDebuggerFrontend);
   return d->nextCommandId;
}

/*!
  \fn void QScriptDebuggerFrontend::processCommand(int id, const QScriptDebuggerCommand &command)

  Subclasses must reimplement this function to process the given \a command
  identified by \a id. Call notifyCommandFinished() when processing is
  complete.
*/

QT_END_NAMESPACE
