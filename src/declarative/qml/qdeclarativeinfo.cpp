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

#include "qdeclarativeinfo.h"

#include "private/qdeclarativedata_p.h"
#include "qdeclarativecontext.h"
#include "private/qdeclarativecontext_p.h"
#include "private/qdeclarativemetatype_p.h"
#include "private/qdeclarativeengine_p.h"

#include <QCoreApplication>

QT_BEGIN_NAMESPACE

/*!
    \fn QDeclarativeInfo qmlInfo(const QObject *object)
    \relates QDeclarativeEngine

    Prints warning messages that include the file and line number for the
    specified QML \a object.

    When QML types display warning messages, it improves traceability
    if they include the QML file and line number on which the
    particular instance was instantiated.

    To include the file and line number, an object must be passed.  If
    the file and line number is not available for that instance
    (either it was not instantiated by the QML engine or location
    information is disabled), "unknown location" will be used instead.

    For example,

    \code
    qmlInfo(object) << tr("component property is a write-once property");
    \endcode

    prints

    \code
    QML MyCustomType (unknown location): component property is a write-once property
    \endcode
*/

class QDeclarativeInfoPrivate
{
 public:
   QDeclarativeInfoPrivate() : ref (1), object(0) {}

   int ref;
   const QObject *object;
   QString buffer;
   QList<QDeclarativeError> errors;
};

QDeclarativeInfo::QDeclarativeInfo(QDeclarativeInfoPrivate *p)
   : QDebug(&p->buffer), d(p)
{
   nospace();
}

QDeclarativeInfo::QDeclarativeInfo(const QDeclarativeInfo &other)
   : QDebug(other), d(other.d)
{
   d->ref++;
}

QDeclarativeInfo::~QDeclarativeInfo()
{
   if (0 == --d->ref) {
      QList<QDeclarativeError> errors = d->errors;

      QDeclarativeEngine *engine = 0;

      if (!d->buffer.isEmpty()) {
         QDeclarativeError error;

         QObject *object = const_cast<QObject *>(d->object);

         if (object) {
            engine = qmlEngine(d->object);
            QString typeName;
            QDeclarativeType *type = QDeclarativeMetaType::qmlType(object->metaObject());
            if (type) {
               typeName = QLatin1String(type->qmlTypeName());
               int lastSlash = typeName.lastIndexOf(QLatin1Char('/'));
               if (lastSlash != -1) {
                  typeName = typeName.mid(lastSlash + 1);
               }
            } else {
               typeName = QString::fromUtf8(object->metaObject()->className());
               int marker = typeName.indexOf(QLatin1String("_QMLTYPE_"));
               if (marker != -1) {
                  typeName = typeName.left(marker);
               }
            }

            d->buffer.prepend(QLatin1String("QML ") + typeName + QLatin1String(": "));

            QDeclarativeData *ddata = QDeclarativeData::get(object, false);
            if (ddata && ddata->outerContext && !ddata->outerContext->url.isEmpty()) {
               error.setUrl(ddata->outerContext->url);
               error.setLine(ddata->lineNumber);
               error.setColumn(ddata->columnNumber);
            }
         }

         error.setDescription(d->buffer);

         errors.prepend(error);
      }

      QDeclarativeEnginePrivate::warning(engine, errors);

      delete d;
   }
}

QDeclarativeInfo qmlInfo(const QObject *me)
{
   QDeclarativeInfoPrivate *d = new QDeclarativeInfoPrivate;
   d->object = me;
   return QDeclarativeInfo(d);
}

QDeclarativeInfo qmlInfo(const QObject *me, const QDeclarativeError &error)
{
   QDeclarativeInfoPrivate *d = new QDeclarativeInfoPrivate;
   d->object = me;
   d->errors << error;
   return QDeclarativeInfo(d);
}

QDeclarativeInfo qmlInfo(const QObject *me, const QList<QDeclarativeError> &errors)
{
   QDeclarativeInfoPrivate *d = new QDeclarativeInfoPrivate;
   d->object = me;
   d->errors = errors;
   return QDeclarativeInfo(d);
}


QT_END_NAMESPACE
