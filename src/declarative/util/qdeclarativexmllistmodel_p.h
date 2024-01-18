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

#ifndef QDECLARATIVEXMLLISTMODEL_P_H
#define QDECLARATIVEXMLLISTMODEL_P_H

#include <qdeclarative.h>
#include <qdeclarativeinfo.h>
#include <QtCore/qurl.h>
#include <QtCore/qstringlist.h>
#include <QtScript/qscriptvalue.h>
#include <qlistmodelinterface_p.h>

QT_BEGIN_NAMESPACE

class QDeclarativeContext;
class QDeclarativeXmlListModelRole;
class QDeclarativeXmlListModelPrivate;

struct QDeclarativeXmlQueryResult {
   int queryId;
   int size;
   QList<QList<QVariant> > data;
   QList<QPair<int, int> > inserted;
   QList<QPair<int, int> > removed;
   QStringList keyRoleResultsCache;
};

class QDeclarativeXmlListModel : public QListModelInterface, public QDeclarativeParserStatus
{
   DECL_CS_OBJECT(QDeclarativeXmlListModel)
   CS_INTERFACES(QDeclarativeParserStatus)
   CS_ENUM(Status)

   DECL_CS_PROPERTY_READ(status, status)
   DECL_CS_PROPERTY_NOTIFY(status, statusChanged)
   DECL_CS_PROPERTY_READ(progress, progress)
   DECL_CS_PROPERTY_NOTIFY(progress, progressChanged)
   DECL_CS_PROPERTY_READ(source, source)
   DECL_CS_PROPERTY_WRITE(source, setSource)
   DECL_CS_PROPERTY_NOTIFY(source, sourceChanged)
   DECL_CS_PROPERTY_READ(xml, xml)
   DECL_CS_PROPERTY_WRITE(xml, setXml)
   DECL_CS_PROPERTY_NOTIFY(xml, xmlChanged)
   DECL_CS_PROPERTY_READ(query, query)
   DECL_CS_PROPERTY_WRITE(query, setQuery)
   DECL_CS_PROPERTY_NOTIFY(query, queryChanged)
   DECL_CS_PROPERTY_READ(namespaceDeclarations, namespaceDeclarations)
   DECL_CS_PROPERTY_WRITE(namespaceDeclarations, setNamespaceDeclarations)
   DECL_CS_PROPERTY_NOTIFY(namespaceDeclarations, namespaceDeclarationsChanged)
   DECL_CS_PROPERTY_READ(roles, roleObjects)
   DECL_CS_PROPERTY_READ(count, count)
   DECL_CS_PROPERTY_NOTIFY(count, countChanged)
   DECL_CS_CLASSINFO("DefaultProperty", "roles")

 public:
   QDeclarativeXmlListModel(QObject *parent = nullptr);
   ~QDeclarativeXmlListModel();

   virtual QHash<int, QVariant> data(int index, const QList<int> &roles = (QList<int>())) const;
   virtual QVariant data(int index, int role) const;
   virtual int count() const;
   virtual QList<int> roles() const;
   virtual QString toString(int role) const;

   QDeclarativeListProperty<QDeclarativeXmlListModelRole> roleObjects();

   QUrl source() const;
   void setSource(const QUrl &);

   QString xml() const;
   void setXml(const QString &);

   QString query() const;
   void setQuery(const QString &);

   QString namespaceDeclarations() const;
   void setNamespaceDeclarations(const QString &);

   Q_INVOKABLE QScriptValue get(int index) const;

   enum Status { Null, Ready, Loading, Error };
   Status status() const;
   qreal progress() const;

   Q_INVOKABLE QString errorString() const;

   virtual void classBegin();
   virtual void componentComplete();

 public:
   DECL_CS_SIGNAL_1(Public, void statusChanged(QDeclarativeXmlListModel::Status un_named_arg1))
   DECL_CS_SIGNAL_2(statusChanged, un_named_arg1)
   DECL_CS_SIGNAL_1(Public, void progressChanged(qreal progress))
   DECL_CS_SIGNAL_2(progressChanged, progress)
   DECL_CS_SIGNAL_1(Public, void countChanged())
   DECL_CS_SIGNAL_2(countChanged)
   DECL_CS_SIGNAL_1(Public, void sourceChanged())
   DECL_CS_SIGNAL_2(sourceChanged)
   DECL_CS_SIGNAL_1(Public, void xmlChanged())
   DECL_CS_SIGNAL_2(xmlChanged)
   DECL_CS_SIGNAL_1(Public, void queryChanged())
   DECL_CS_SIGNAL_2(queryChanged)
   DECL_CS_SIGNAL_1(Public, void namespaceDeclarationsChanged())
   DECL_CS_SIGNAL_2(namespaceDeclarationsChanged)

 public :
   // ### need to use/expose Expiry to guess when to call this?
   // ### property to auto-call this on reasonable Expiry?
   // ### LastModified/Age also useful to guess.
   // ### Probably also applies to other network-requesting types.
   DECL_CS_SLOT_1(Public, void reload())
   DECL_CS_SLOT_2(reload)

 private :
   DECL_CS_SLOT_1(Private, void requestFinished())
   DECL_CS_SLOT_2(requestFinished)
   DECL_CS_SLOT_1(Private, void requestProgress(qint64 un_named_arg1, qint64 un_named_arg2))
   DECL_CS_SLOT_2(requestProgress)
   DECL_CS_SLOT_1(Private, void dataCleared())
   DECL_CS_SLOT_2(dataCleared)
   DECL_CS_SLOT_1(Private, void queryCompleted(const QDeclarativeXmlQueryResult &un_named_arg1))
   DECL_CS_SLOT_2(queryCompleted)
   DECL_CS_SLOT_1(Private, void queryError(void *object, const QString &error))
   DECL_CS_SLOT_2(queryError)

 private:
   Q_DECLARE_PRIVATE(QDeclarativeXmlListModel)
   Q_DISABLE_COPY(QDeclarativeXmlListModel)
};

class QDeclarativeXmlListModelRole : public QObject
{
   DECL_CS_OBJECT(QDeclarativeXmlListModelRole)
   DECL_CS_PROPERTY_READ(name, name)
   DECL_CS_PROPERTY_WRITE(name, setName)
   DECL_CS_PROPERTY_NOTIFY(name, nameChanged)
   DECL_CS_PROPERTY_READ(query, query)
   DECL_CS_PROPERTY_WRITE(query, setQuery)
   DECL_CS_PROPERTY_NOTIFY(query, queryChanged)
   DECL_CS_PROPERTY_READ(isKey, isKey)
   DECL_CS_PROPERTY_WRITE(isKey, setIsKey)
   DECL_CS_PROPERTY_NOTIFY(isKey, isKeyChanged)
 public:
   QDeclarativeXmlListModelRole() : m_isKey(false) {}
   ~QDeclarativeXmlListModelRole() {}

   QString name() const {
      return m_name;
   }
   void setName(const QString &name) {
      if (name == m_name) {
         return;
      }
      m_name = name;
      emit nameChanged();
   }

   QString query() const {
      return m_query;
   }
   void setQuery(const QString &query) {
      if (query.startsWith(QLatin1Char('/'))) {
         qmlInfo(this) << tr("An XmlRole query must not start with '/'");
         return;
      }
      if (m_query == query) {
         return;
      }
      m_query = query;
      emit queryChanged();
   }

   bool isKey() const {
      return m_isKey;
   }
   void setIsKey(bool b) {
      if (m_isKey == b) {
         return;
      }
      m_isKey = b;
      emit isKeyChanged();
   }

   bool isValid() {
      return !m_name.isEmpty() && !m_query.isEmpty();
   }

 public:
   DECL_CS_SIGNAL_1(Public, void nameChanged())
   DECL_CS_SIGNAL_2(nameChanged)
   DECL_CS_SIGNAL_1(Public, void queryChanged())
   DECL_CS_SIGNAL_2(queryChanged)
   DECL_CS_SIGNAL_1(Public, void isKeyChanged())
   DECL_CS_SIGNAL_2(isKeyChanged)

 private:
   QString m_name;
   QString m_query;
   bool m_isKey;
};

QT_END_NAMESPACE

QML_DECLARE_TYPE(QDeclarativeXmlListModel)
QML_DECLARE_TYPE(QDeclarativeXmlListModelRole)

#endif // QDECLARATIVEXMLLISTMODEL_H
