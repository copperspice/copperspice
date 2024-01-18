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

#ifndef QDECLARATIVEENGINEDEBUGSERVICE_P_H
#define QDECLARATIVEENGINEDEBUGSERVICE_P_H

#include <qdeclarativedebugservice_p.h>
#include <qurl.h>
#include <qvariant.h>
#include <QWeakPointer>

QT_BEGIN_NAMESPACE

class QDeclarativeEngine;
class QDeclarativeContext;
class QDeclarativeWatcher;
class QDataStream;
class QDeclarativeState;

class QDeclarativeEngineDebugService : public QDeclarativeDebugService
{
   DECL_CS_OBJECT(QDeclarativeEngineDebugService)
 public:
   QDeclarativeEngineDebugService(QObject * = 0);

   struct QDeclarativeObjectData {
      QUrl url;
      int lineNumber;
      int columnNumber;
      QString idString;
      QString objectName;
      QString objectType;
      int objectId;
      int contextId;
      int parentId;
   };

   struct QDeclarativeObjectProperty {
      enum Type { Unknown, Basic, Object, List, SignalProperty, Variant };
      Type type;
      QString name;
      QVariant value;
      QString valueTypeName;
      QString binding;
      bool hasNotifySignal;
   };

   void addEngine(QDeclarativeEngine *);
   void remEngine(QDeclarativeEngine *);
   void objectCreated(QDeclarativeEngine *, QObject *);

   static QDeclarativeEngineDebugService *instance();

 protected:
   virtual void messageReceived(const QByteArray &);

 private :
   DECL_CS_SLOT_1(Private, void propertyChanged(int id, int objectId, const QMetaProperty &property, const QVariant &value))
   DECL_CS_SLOT_2(propertyChanged)

 private:
   void prepareDeferredObjects(QObject *);
   void buildObjectList(QDataStream &, QDeclarativeContext *);
   void buildObjectDump(QDataStream &, QObject *, bool, bool);
   void buildStatesList(QDeclarativeContext *, bool);
   void buildStatesList(QObject *obj);
   QDeclarativeObjectData objectData(QObject *);
   QDeclarativeObjectProperty propertyData(QObject *, int);
   QVariant valueContents(const QVariant &defaultValue) const;
   void setBinding(int objectId, const QString &propertyName, const QVariant &expression, bool isLiteralValue,
                   QString filename = QString(), int line = -1);
   void resetBinding(int objectId, const QString &propertyName);
   void setMethodBody(int objectId, const QString &method, const QString &body);

   QList<QDeclarativeEngine *> m_engines;
   QDeclarativeWatcher *m_watch;
   QList<QWeakPointer<QDeclarativeState> > m_allStates;
};
Q_DECLARATIVE_PRIVATE_EXPORT QDataStream &operator<<(QDataStream &,
      const QDeclarativeEngineDebugService::QDeclarativeObjectData &);
Q_DECLARATIVE_PRIVATE_EXPORT QDataStream &operator>>(QDataStream &,
      QDeclarativeEngineDebugService::QDeclarativeObjectData &);
Q_DECLARATIVE_PRIVATE_EXPORT QDataStream &operator<<(QDataStream &,
      const QDeclarativeEngineDebugService::QDeclarativeObjectProperty &);
Q_DECLARATIVE_PRIVATE_EXPORT QDataStream &operator>>(QDataStream &,
      QDeclarativeEngineDebugService::QDeclarativeObjectProperty &);

QT_END_NAMESPACE

#endif // QDECLARATIVEENGINEDEBUGSERVICE_P_H

