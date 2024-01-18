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

#ifndef QDECLARATIVEENGINEDEBUG_P_H
#define QDECLARATIVEENGINEDEBUG_P_H

#include <QtCore/qobject.h>
#include <QtCore/qurl.h>
#include <QtCore/qvariant.h>
#include <qdeclarativeglobal_p.h>
#include <QScopedPointer>

QT_BEGIN_NAMESPACE

class QDeclarativeDebugConnection;
class QDeclarativeDebugWatch;
class QDeclarativeDebugPropertyWatch;
class QDeclarativeDebugObjectExpressionWatch;
class QDeclarativeDebugEnginesQuery;
class QDeclarativeDebugRootContextQuery;
class QDeclarativeDebugObjectQuery;
class QDeclarativeDebugExpressionQuery;
class QDeclarativeDebugPropertyReference;
class QDeclarativeDebugContextReference;
class QDeclarativeDebugObjectReference;
class QDeclarativeDebugFileReference;
class QDeclarativeDebugEngineReference;
class QDeclarativeEngineDebugPrivate;

class Q_DECLARATIVE_PRIVATE_EXPORT QDeclarativeEngineDebug : public QObject
{
   DECL_CS_OBJECT(QDeclarativeEngineDebug)

 public:
   enum Status { NotConnected, Unavailable, Enabled };

   explicit QDeclarativeEngineDebug(QDeclarativeDebugConnection *, QObject * = 0);

   Status status() const;

   QDeclarativeDebugPropertyWatch *addWatch(const QDeclarativeDebugPropertyReference &, QObject *parent = nullptr);
   QDeclarativeDebugWatch *addWatch(const QDeclarativeDebugContextReference &, const QString &, QObject *parent = nullptr);
   QDeclarativeDebugObjectExpressionWatch *addWatch(const QDeclarativeDebugObjectReference &, const QString &,
         QObject *parent = nullptr);
   QDeclarativeDebugWatch *addWatch(const QDeclarativeDebugObjectReference &, QObject *parent = nullptr);
   QDeclarativeDebugWatch *addWatch(const QDeclarativeDebugFileReference &, QObject *parent = nullptr);

   void removeWatch(QDeclarativeDebugWatch *watch);

   QDeclarativeDebugEnginesQuery *queryAvailableEngines(QObject *parent = nullptr);

   QDeclarativeDebugRootContextQuery *queryRootContexts(const QDeclarativeDebugEngineReference &, QObject *parent = nullptr);
   QDeclarativeDebugObjectQuery *queryObject(const QDeclarativeDebugObjectReference &, QObject *parent = nullptr);
   QDeclarativeDebugObjectQuery *queryObjectRecursive(const QDeclarativeDebugObjectReference &, QObject *parent = nullptr);

   QDeclarativeDebugExpressionQuery *queryExpressionResult(int objectDebugId, const QString &expr, QObject *parent = nullptr);

   bool setBindingForObject(int objectDebugId, const QString &propertyName,
                            const QVariant &bindingExpression, bool isLiteralValue,
                            QString source = QString(), int line = -1);

   bool resetBindingForObject(int objectDebugId, const QString &propertyName);
   bool setMethodBody(int objectDebugId, const QString &methodName, const QString &methodBody);

   DECL_CS_SIGNAL_1(Public, void newObjects())
   DECL_CS_SIGNAL_2(newObjects)
   DECL_CS_SIGNAL_1(Public, void statusChanged(Status status))
   DECL_CS_SIGNAL_2(statusChanged, status)

 private:
   Q_DECLARE_PRIVATE(QDeclarativeEngineDebug)

 protected:
   QScopedPointer<QDeclarativeEngineDebugPrivate> d_ptr;

};

class Q_DECLARATIVE_PRIVATE_EXPORT QDeclarativeDebugWatch : public QObject
{
   DECL_CS_OBJECT(QDeclarativeDebugWatch)

 public:
   enum State { Waiting, Active, Inactive, Dead };

   QDeclarativeDebugWatch(QObject *);
   ~QDeclarativeDebugWatch();

   int queryId() const;
   int objectDebugId() const;
   State state() const;

 public:
   DECL_CS_SIGNAL_1(Public, void stateChanged(QDeclarativeDebugWatch::State un_named_arg1))
   DECL_CS_SIGNAL_2(stateChanged, un_named_arg1)
   //void objectChanged(int, const QDeclarativeDebugObjectReference &);
   //void valueChanged(int, const QVariant &);

   // Server sends value as string if it is a user-type variant
   DECL_CS_SIGNAL_1(Public, void valueChanged(const QByteArray &name, const QVariant &value))
   DECL_CS_SIGNAL_2(valueChanged, name, value)

 private:
   friend class QDeclarativeEngineDebug;
   friend class QDeclarativeEngineDebugPrivate;
   void setState(State);
   State m_state;
   int m_queryId;
   QDeclarativeEngineDebug *m_client;
   int m_objectDebugId;
};

class Q_DECLARATIVE_PRIVATE_EXPORT QDeclarativeDebugPropertyWatch : public QDeclarativeDebugWatch
{
   DECL_CS_OBJECT(QDeclarativeDebugPropertyWatch)

 public:
   QDeclarativeDebugPropertyWatch(QObject *parent);

   QString name() const;

 private:
   friend class QDeclarativeEngineDebug;
   QString m_name;
};

class Q_DECLARATIVE_PRIVATE_EXPORT QDeclarativeDebugObjectExpressionWatch : public QDeclarativeDebugWatch
{
   DECL_CS_OBJECT(QDeclarativeDebugObjectExpressionWatch)

 public:
   QDeclarativeDebugObjectExpressionWatch(QObject *parent);

   QString expression() const;

 private:
   friend class QDeclarativeEngineDebug;
   QString m_expr;
   int m_debugId;
};


class Q_DECLARATIVE_PRIVATE_EXPORT QDeclarativeDebugQuery : public QObject
{
   DECL_CS_OBJECT(QDeclarativeDebugQuery)

 public:
   enum State { Waiting, Error, Completed };

   State state() const;
   bool isWaiting() const;

   //    bool waitUntilCompleted();

 public:
   DECL_CS_SIGNAL_1(Public, void stateChanged(QDeclarativeDebugQuery::State un_named_arg1))
   DECL_CS_SIGNAL_2(stateChanged, un_named_arg1)

 protected:
   QDeclarativeDebugQuery(QObject *);

 private:
   friend class QDeclarativeEngineDebug;
   friend class QDeclarativeEngineDebugPrivate;
   void setState(State);
   State m_state;
};

class Q_DECLARATIVE_PRIVATE_EXPORT QDeclarativeDebugFileReference
{
 public:
   QDeclarativeDebugFileReference();
   QDeclarativeDebugFileReference(const QDeclarativeDebugFileReference &);
   QDeclarativeDebugFileReference &operator=(const QDeclarativeDebugFileReference &);

   QUrl url() const;
   void setUrl(const QUrl &);
   int lineNumber() const;
   void setLineNumber(int);
   int columnNumber() const;
   void setColumnNumber(int);

 private:
   friend class QDeclarativeEngineDebugPrivate;
   QUrl m_url;
   int m_lineNumber;
   int m_columnNumber;
};

class Q_DECLARATIVE_PRIVATE_EXPORT QDeclarativeDebugEngineReference
{
 public:
   QDeclarativeDebugEngineReference();
   QDeclarativeDebugEngineReference(int);
   QDeclarativeDebugEngineReference(const QDeclarativeDebugEngineReference &);
   QDeclarativeDebugEngineReference &operator=(const QDeclarativeDebugEngineReference &);

   int debugId() const;
   QString name() const;

 private:
   friend class QDeclarativeEngineDebugPrivate;
   int m_debugId;
   QString m_name;
};

class Q_DECLARATIVE_PRIVATE_EXPORT QDeclarativeDebugObjectReference
{
 public:
   QDeclarativeDebugObjectReference();
   QDeclarativeDebugObjectReference(int);
   QDeclarativeDebugObjectReference(const QDeclarativeDebugObjectReference &);
   QDeclarativeDebugObjectReference &operator=(const QDeclarativeDebugObjectReference &);

   int debugId() const;
   int parentId() const;
   QString className() const;
   QString idString() const;
   QString name() const;

   QDeclarativeDebugFileReference source() const;
   int contextDebugId() const;
   bool needsMoreData() const;

   QList<QDeclarativeDebugPropertyReference> properties() const;
   QList<QDeclarativeDebugObjectReference> children() const;

 private:
   friend class QDeclarativeEngineDebugPrivate;
   int m_debugId;
   int m_parentId;
   QString m_class;
   QString m_idString;
   QString m_name;
   QDeclarativeDebugFileReference m_source;
   int m_contextDebugId;
   bool m_needsMoreData;
   QList<QDeclarativeDebugPropertyReference> m_properties;
   QList<QDeclarativeDebugObjectReference> m_children;
};

class Q_DECLARATIVE_PRIVATE_EXPORT QDeclarativeDebugContextReference
{
 public:
   QDeclarativeDebugContextReference();
   QDeclarativeDebugContextReference(const QDeclarativeDebugContextReference &);
   QDeclarativeDebugContextReference &operator=(const QDeclarativeDebugContextReference &);

   int debugId() const;
   QString name() const;

   QList<QDeclarativeDebugObjectReference> objects() const;
   QList<QDeclarativeDebugContextReference> contexts() const;

 private:
   friend class QDeclarativeEngineDebugPrivate;
   int m_debugId;
   QString m_name;
   QList<QDeclarativeDebugObjectReference> m_objects;
   QList<QDeclarativeDebugContextReference> m_contexts;
};

class Q_DECLARATIVE_PRIVATE_EXPORT QDeclarativeDebugPropertyReference
{
 public:
   QDeclarativeDebugPropertyReference();
   QDeclarativeDebugPropertyReference(const QDeclarativeDebugPropertyReference &);
   QDeclarativeDebugPropertyReference &operator=(const QDeclarativeDebugPropertyReference &);

   int objectDebugId() const;
   QString name() const;
   QVariant value() const;
   QString valueTypeName() const;
   QString binding() const;
   bool hasNotifySignal() const;

 private:
   friend class QDeclarativeEngineDebugPrivate;
   int m_objectDebugId;
   QString m_name;
   QVariant m_value;
   QString m_valueTypeName;
   QString m_binding;
   bool m_hasNotifySignal;
};


class Q_DECLARATIVE_PRIVATE_EXPORT QDeclarativeDebugEnginesQuery : public QDeclarativeDebugQuery
{
   DECL_CS_OBJECT(QDeclarativeDebugEnginesQuery)
 public:
   virtual ~QDeclarativeDebugEnginesQuery();
   QList<QDeclarativeDebugEngineReference> engines() const;
 private:
   friend class QDeclarativeEngineDebug;
   friend class QDeclarativeEngineDebugPrivate;
   QDeclarativeDebugEnginesQuery(QObject *);
   QDeclarativeEngineDebug *m_client;
   int m_queryId;
   QList<QDeclarativeDebugEngineReference> m_engines;
};

class Q_DECLARATIVE_PRIVATE_EXPORT QDeclarativeDebugRootContextQuery : public QDeclarativeDebugQuery
{
   DECL_CS_OBJECT(QDeclarativeDebugRootContextQuery)
 public:
   virtual ~QDeclarativeDebugRootContextQuery();
   QDeclarativeDebugContextReference rootContext() const;
 private:
   friend class QDeclarativeEngineDebug;
   friend class QDeclarativeEngineDebugPrivate;
   QDeclarativeDebugRootContextQuery(QObject *);
   QDeclarativeEngineDebug *m_client;
   int m_queryId;
   QDeclarativeDebugContextReference m_context;
};

class Q_DECLARATIVE_PRIVATE_EXPORT QDeclarativeDebugObjectQuery : public QDeclarativeDebugQuery
{
   DECL_CS_OBJECT(QDeclarativeDebugObjectQuery)
 public:
   virtual ~QDeclarativeDebugObjectQuery();
   QDeclarativeDebugObjectReference object() const;
 private:
   friend class QDeclarativeEngineDebug;
   friend class QDeclarativeEngineDebugPrivate;
   QDeclarativeDebugObjectQuery(QObject *);
   QDeclarativeEngineDebug *m_client;
   int m_queryId;
   QDeclarativeDebugObjectReference m_object;

};

class Q_DECLARATIVE_PRIVATE_EXPORT QDeclarativeDebugExpressionQuery : public QDeclarativeDebugQuery
{
   DECL_CS_OBJECT(QDeclarativeDebugExpressionQuery)
 public:
   virtual ~QDeclarativeDebugExpressionQuery();
   QVariant expression() const;
   QVariant result() const;
 private:
   friend class QDeclarativeEngineDebug;
   friend class QDeclarativeEngineDebugPrivate;
   QDeclarativeDebugExpressionQuery(QObject *);
   QDeclarativeEngineDebug *m_client;
   int m_queryId;
   QVariant m_expr;
   QVariant m_result;
};

QT_END_NAMESPACE

Q_DECLARE_METATYPE(QDeclarativeDebugEngineReference)
Q_DECLARE_METATYPE(QDeclarativeDebugObjectReference)
Q_DECLARE_METATYPE(QDeclarativeDebugContextReference)
Q_DECLARE_METATYPE(QDeclarativeDebugPropertyReference)

#endif // QDECLARATIVEENGINEDEBUG_H
