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

#include "qscriptdebuggerevent_p.h"
#include "qscriptdebuggervalue_p.h"

#include <QtCore/qhash.h>
#include <QtCore/qdatastream.h>

Q_DECLARE_METATYPE(QScriptDebuggerValue)

QT_BEGIN_NAMESPACE

class QScriptDebuggerEventPrivate
{
 public:
   QScriptDebuggerEventPrivate();
   ~QScriptDebuggerEventPrivate();

   QScriptDebuggerEvent::Type type;
   QHash<QScriptDebuggerEvent::Attribute, QVariant> attributes;
};

QScriptDebuggerEventPrivate::QScriptDebuggerEventPrivate()
   : type(QScriptDebuggerEvent::None)
{
}

QScriptDebuggerEventPrivate::~QScriptDebuggerEventPrivate()
{
}

QScriptDebuggerEvent::QScriptDebuggerEvent()
   : d_ptr(new QScriptDebuggerEventPrivate)
{
   d_ptr->type = None;
}

QScriptDebuggerEvent::QScriptDebuggerEvent(Type type)
   : d_ptr(new QScriptDebuggerEventPrivate)
{
   d_ptr->type = type;
}

QScriptDebuggerEvent::QScriptDebuggerEvent(Type type, qint64 scriptId,
      int lineNumber, int columnNumber)
   : d_ptr(new QScriptDebuggerEventPrivate)
{
   d_ptr->type = type;
   d_ptr->attributes[ScriptID] = scriptId;
   d_ptr->attributes[LineNumber] = lineNumber;
   d_ptr->attributes[ColumnNumber] = columnNumber;
}

QScriptDebuggerEvent::QScriptDebuggerEvent(const QScriptDebuggerEvent &other)
   : d_ptr(new QScriptDebuggerEventPrivate)
{
   *d_ptr = *other.d_ptr;
}

QScriptDebuggerEvent::~QScriptDebuggerEvent()
{
}

QScriptDebuggerEvent &QScriptDebuggerEvent::operator=(const QScriptDebuggerEvent &other)
{
   *d_ptr = *other.d_ptr;
   return *this;
}

QScriptDebuggerEvent::Type QScriptDebuggerEvent::type() const
{
   Q_D(const QScriptDebuggerEvent);
   return d->type;
}

QVariant QScriptDebuggerEvent::attribute(Attribute attribute,
      const QVariant &defaultValue) const
{
   Q_D(const QScriptDebuggerEvent);
   return d->attributes.value(attribute, defaultValue);
}

void QScriptDebuggerEvent::setAttribute(Attribute attribute,
                                        const QVariant &value)
{
   Q_D(QScriptDebuggerEvent);
   if (!value.isValid()) {
      d->attributes.remove(attribute);
   } else {
      d->attributes[attribute] = value;
   }
}

QHash<QScriptDebuggerEvent::Attribute, QVariant> QScriptDebuggerEvent::attributes() const
{
   Q_D(const QScriptDebuggerEvent);
   return d->attributes;
}

qint64 QScriptDebuggerEvent::scriptId() const
{
   Q_D(const QScriptDebuggerEvent);
   return d->attributes.value(ScriptID, -1).toLongLong();
}

void QScriptDebuggerEvent::setScriptId(qint64 id)
{
   Q_D(QScriptDebuggerEvent);
   d->attributes[ScriptID] = id;
}

QString QScriptDebuggerEvent::fileName() const
{
   Q_D(const QScriptDebuggerEvent);
   return d->attributes.value(FileName).toString();
}

void QScriptDebuggerEvent::setFileName(const QString &fileName)
{
   Q_D(QScriptDebuggerEvent);
   d->attributes[FileName] = fileName;
}

int QScriptDebuggerEvent::lineNumber() const
{
   Q_D(const QScriptDebuggerEvent);
   return d->attributes.value(LineNumber, -1).toInt();
}

void QScriptDebuggerEvent::setLineNumber(int lineNumber)
{
   Q_D(QScriptDebuggerEvent);
   d->attributes[LineNumber] = lineNumber;
}

int QScriptDebuggerEvent::columnNumber() const
{
   Q_D(const QScriptDebuggerEvent);
   return d->attributes.value(ColumnNumber, -1).toInt();
}

void QScriptDebuggerEvent::setColumnNumber(int columnNumber)
{
   Q_D(QScriptDebuggerEvent);
   d->attributes[ColumnNumber] = columnNumber;
}

int QScriptDebuggerEvent::breakpointId() const
{
   Q_D(const QScriptDebuggerEvent);
   return d->attributes.value(BreakpointID, -1).toInt();
}

void QScriptDebuggerEvent::setBreakpointId(int id)
{
   Q_D(QScriptDebuggerEvent);
   d->attributes[BreakpointID] = id;
}

QString QScriptDebuggerEvent::message() const
{
   Q_D(const QScriptDebuggerEvent);
   return d->attributes.value(Message).toString();
}

void QScriptDebuggerEvent::setMessage(const QString &message)
{
   Q_D(QScriptDebuggerEvent);
   d->attributes[Message] = message;
}

QScriptDebuggerValue QScriptDebuggerEvent::scriptValue() const
{
   Q_D(const QScriptDebuggerEvent);
   return qvariant_cast<QScriptDebuggerValue>(d->attributes[Value]);
}

void QScriptDebuggerEvent::setScriptValue(const QScriptDebuggerValue &value)
{
   Q_D(QScriptDebuggerEvent);
   d->attributes[Value] = QVariant::fromValue(value);
}

void QScriptDebuggerEvent::setNestedEvaluate(bool nested)
{
   Q_D(QScriptDebuggerEvent);
   d->attributes[IsNestedEvaluate] = nested;
}

bool QScriptDebuggerEvent::isNestedEvaluate() const
{
   Q_D(const QScriptDebuggerEvent);
   return d->attributes.value(IsNestedEvaluate).toBool();
}

void QScriptDebuggerEvent::setHasExceptionHandler(bool hasHandler)
{
   Q_D(QScriptDebuggerEvent);
   d->attributes[HasExceptionHandler] = hasHandler;
}

bool QScriptDebuggerEvent::hasExceptionHandler() const
{
   Q_D(const QScriptDebuggerEvent);
   return d->attributes.value(HasExceptionHandler).toBool();
}

/*!
  Returns true if this QScriptDebuggerEvent is equal to the \a other
  event, otherwise returns false.
*/
bool QScriptDebuggerEvent::operator==(const QScriptDebuggerEvent &other) const
{
   Q_D(const QScriptDebuggerEvent);
   const QScriptDebuggerEventPrivate *od = other.d_func();
   if (d == od) {
      return true;
   }
   if (!d || !od) {
      return false;
   }
   return ((d->type == od->type)
           && (d->attributes == od->attributes));
}

/*!
  Returns true if this QScriptDebuggerEvent is not equal to the \a
  other event, otherwise returns false.
*/
bool QScriptDebuggerEvent::operator!=(const QScriptDebuggerEvent &other) const
{
   return !(*this == other);
}

/*!
  \fn QDataStream &operator<<(QDataStream &stream, const QScriptDebuggerEvent &event)
  \relates QScriptDebuggerEvent

  Writes the given \a event to the specified \a stream.
*/
QDataStream &operator<<(QDataStream &out, const QScriptDebuggerEvent &event)
{
   const QScriptDebuggerEventPrivate *d = event.d_ptr.data();
   out << (quint32)d->type;
   out << (qint32)d->attributes.size();
   QHash<QScriptDebuggerEvent::Attribute, QVariant>::const_iterator it;
   for (it = d->attributes.constBegin(); it != d->attributes.constEnd(); ++it) {
      out << (quint32)it.key();
      out << it.value();
   }
   return out;
}

/*!
  \fn QDataStream &operator>>(QDataStream &stream, QScriptDebuggerEvent &event)
  \relates QScriptDebuggerEvent

  Reads a QScriptDebuggerEvent from the specified \a stream into the
  given \a event.
*/
QDataStream &operator>>(QDataStream &in, QScriptDebuggerEvent &event)
{
   QScriptDebuggerEventPrivate *d = event.d_ptr.data();

   quint32 type;
   in >> type;
   d->type = QScriptDebuggerEvent::Type(type);

   qint32 attribCount;
   in >> attribCount;
   QHash<QScriptDebuggerEvent::Attribute, QVariant> attribs;
   for (qint32 i = 0; i < attribCount; ++i) {
      quint32 key;
      in >> key;
      QVariant value;
      in >> value;
      attribs[QScriptDebuggerEvent::Attribute(key)] = value;
   }
   d->attributes = attribs;

   return in;
}

QT_END_NAMESPACE
