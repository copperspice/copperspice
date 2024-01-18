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

#include "private/qdeclarativexmllistmodel_p.h"

#include <qdeclarativecontext.h>
#include <qdeclarativeengine_p.h>

#include <QDebug>
#include <QStringList>
#include <QMap>
#include <QApplication>
#include <QThread>
#include <QXmlQuery>
#include <QXmlResultItems>
#include <QXmlNodeModelIndex>
#include <QBuffer>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QTimer>
#include <QMutex>

Q_DECLARE_METATYPE(QDeclarativeXmlQueryResult)

QT_BEGIN_NAMESPACE

typedef QPair<int, int> QDeclarativeXmlListRange;

#define XMLLISTMODEL_CLEAR_ID 0

/*!
    \qmlclass XmlRole QDeclarativeXmlListModelRole
    \ingroup qml-working-with-data
  \since 4.7
    \brief The XmlRole element allows you to specify a role for an XmlListModel.

    \sa {QtDeclarative}
*/

/*!
    \qmlproperty string XmlRole::name

    The name for the role. This name is used to access the model data for this role.

    For example, the following model has a role named "title", which can be accessed
    from the view's delegate:

    \qml
    XmlListModel {
        id: xmlModel
        // ...
        XmlRole {
            name: "title"
            query: "title/string()"
        }
    }
    \endqml

    \qml
    ListView {
        model: xmlModel
        delegate: Text { text: title }
    }
    \endqml
*/

/*!
    \qmlproperty string XmlRole::query
    The relative XPath expression query for this role. The query must be relative; it cannot start
    with a '/'.

    For example, if there is an XML document like this:

    \quotefile doc/src/snippets/declarative/xmlrole.xml

    Here are some valid XPath expressions for XmlRole queries on this document:

    \snippet doc/src/snippets/declarative/xmlrole.qml 0
    \dots 4
    \snippet doc/src/snippets/declarative/xmlrole.qml 1

    See the \l{http://www.w3.org/TR/xpath20/}{W3C XPath 2.0 specification} for more information.
*/

/*!
    \qmlproperty bool XmlRole::isKey
    Defines whether this is a key role.

    Key roles are used to to determine whether a set of values should
    be updated or added to the XML list model when XmlListModel::reload()
    is called.

    \sa XmlListModel
*/

struct XmlQueryJob {
   int queryId;
   QByteArray data;
   QString query;
   QString namespaces;
   QStringList roleQueries;
   QList<void *> roleQueryErrorId; // the ptr to send back if there is an error
   QStringList keyRoleQueries;
   QStringList keyRoleResultsCache;
   QString prefix;
};


class QDeclarativeXmlQueryEngine;
class QDeclarativeXmlQueryThreadObject : public QObject
{
   DECL_CS_OBJECT(QDeclarativeXmlQueryThreadObject)

 public:
   QDeclarativeXmlQueryThreadObject(QDeclarativeXmlQueryEngine *);

   void processJobs();
   virtual bool event(QEvent *e);

 private:
   QDeclarativeXmlQueryEngine *m_queryEngine;
};


class QDeclarativeXmlQueryEngine : public QThread
{
   DECL_CS_OBJECT(QDeclarativeXmlQueryEngine)

 public:
   QDeclarativeXmlQueryEngine(QDeclarativeEngine *eng);
   ~QDeclarativeXmlQueryEngine();

   int doQuery(QString query, QString namespaces, QByteArray data, QList<QDeclarativeXmlListModelRole *> *roleObjects,
               QStringList keyRoleResultsCache);
   void abort(int id);

   void processJobs();

   static QDeclarativeXmlQueryEngine *instance(QDeclarativeEngine *engine);

   DECL_CS_SIGNAL_1(Public, void queryCompleted(const QDeclarativeXmlQueryResult &un_named_arg1))
   DECL_CS_SIGNAL_2(queryCompleted, un_named_arg1)

   DECL_CS_SIGNAL_1(Public, void error(void *un_named_arg1, const QString &un_named_arg2))
   DECL_CS_SIGNAL_2(error, un_named_arg1, un_named_arg2)

 protected:
   void run();

 private:
   void processQuery(XmlQueryJob *job);
   void doQueryJob(XmlQueryJob *job, QDeclarativeXmlQueryResult *currentResult);
   void doSubQueryJob(XmlQueryJob *job, QDeclarativeXmlQueryResult *currentResult);
   void getValuesOfKeyRoles(const XmlQueryJob &currentJob, QStringList *values, QXmlQuery *query) const;
   void addIndexToRangeList(QList<QDeclarativeXmlListRange> *ranges, int index) const;

   QMutex m_mutex;
   QDeclarativeXmlQueryThreadObject *m_threadObject;
   QList<XmlQueryJob> m_jobs;
   QSet<int> m_cancelledJobs;
   QAtomicInt m_queryIds;

   QDeclarativeEngine *m_engine;
   QObject *m_eventLoopQuitHack;

   static QHash<QDeclarativeEngine *, QDeclarativeXmlQueryEngine *> queryEngines;
   static QMutex queryEnginesMutex;
};
QHash<QDeclarativeEngine *, QDeclarativeXmlQueryEngine *> QDeclarativeXmlQueryEngine::queryEngines;
QMutex QDeclarativeXmlQueryEngine::queryEnginesMutex;


QDeclarativeXmlQueryThreadObject::QDeclarativeXmlQueryThreadObject(QDeclarativeXmlQueryEngine *e)
   : m_queryEngine(e)
{
}

void QDeclarativeXmlQueryThreadObject::processJobs()
{
   QCoreApplication::postEvent(this, new QEvent(QEvent::User));
}

bool QDeclarativeXmlQueryThreadObject::event(QEvent *e)
{
   if (e->type() == QEvent::User) {
      m_queryEngine->processJobs();
      return true;
   } else {
      return QObject::event(e);
   }
}



QDeclarativeXmlQueryEngine::QDeclarativeXmlQueryEngine(QDeclarativeEngine *eng)
   : QThread(eng), m_threadObject(0), m_queryIds(XMLLISTMODEL_CLEAR_ID + 1), m_engine(eng), m_eventLoopQuitHack(0)
{
   qRegisterMetaType<QDeclarativeXmlQueryResult>("QDeclarativeXmlQueryResult");

   m_eventLoopQuitHack = new QObject;
   m_eventLoopQuitHack->moveToThread(this);
   connect(m_eventLoopQuitHack, SIGNAL(destroyed(QObject *)), SLOT(quit()), Qt::DirectConnection);
   start(QThread::IdlePriority);
}

QDeclarativeXmlQueryEngine::~QDeclarativeXmlQueryEngine()
{
   queryEnginesMutex.lock();
   queryEngines.remove(m_engine);
   queryEnginesMutex.unlock();

   m_eventLoopQuitHack->deleteLater();
   wait();
}

int QDeclarativeXmlQueryEngine::doQuery(QString query, QString namespaces, QByteArray data,
                                        QList<QDeclarativeXmlListModelRole *> *roleObjects, QStringList keyRoleResultsCache)
{
   {
      QMutexLocker m1(&m_mutex);
      m_queryIds.ref();
      if (m_queryIds.load() <= 0) {
         m_queryIds.store(1);
      }
   }

   XmlQueryJob job;
   job.queryId = m_queryIds.load();
   job.data = data;
   job.query = QLatin1String("doc($src)") + query;
   job.namespaces = namespaces;
   job.keyRoleResultsCache = keyRoleResultsCache;

   for (int i = 0; i < roleObjects->count(); i++) {
      if (!roleObjects->at(i)->isValid()) {
         job.roleQueries << QString();
         continue;
      }
      job.roleQueries << roleObjects->at(i)->query();
      job.roleQueryErrorId << static_cast<void *>(roleObjects->at(i));
      if (roleObjects->at(i)->isKey()) {
         job.keyRoleQueries << job.roleQueries.last();
      }
   }

   {
      QMutexLocker ml(&m_mutex);
      m_jobs.append(job);
      if (m_threadObject) {
         m_threadObject->processJobs();
      }
   }

   return job.queryId;
}

void QDeclarativeXmlQueryEngine::abort(int id)
{
   QMutexLocker ml(&m_mutex);
   if (id != -1) {
      m_cancelledJobs.insert(id);
   }
}

void QDeclarativeXmlQueryEngine::run()
{
   m_mutex.lock();
   m_threadObject = new QDeclarativeXmlQueryThreadObject(this);
   m_mutex.unlock();

   processJobs();
   exec();

   delete m_threadObject;
   m_threadObject = 0;
}

void QDeclarativeXmlQueryEngine::processJobs()
{
   QMutexLocker locker(&m_mutex);

   while (true) {
      if (m_jobs.isEmpty()) {
         return;
      }

      XmlQueryJob currentJob = m_jobs.takeLast();
      while (m_cancelledJobs.remove(currentJob.queryId)) {
         if (m_jobs.isEmpty()) {
            return;
         }
         currentJob = m_jobs.takeLast();
      }

      locker.unlock();
      processQuery(&currentJob);
      locker.relock();
   }
}

QDeclarativeXmlQueryEngine *QDeclarativeXmlQueryEngine::instance(QDeclarativeEngine *engine)
{
   queryEnginesMutex.lock();
   QDeclarativeXmlQueryEngine *queryEng = queryEngines.value(engine);
   if (!queryEng) {
      queryEng = new QDeclarativeXmlQueryEngine(engine);
      queryEngines.insert(engine, queryEng);
   }
   queryEnginesMutex.unlock();

   return queryEng;
}

void QDeclarativeXmlQueryEngine::processQuery(XmlQueryJob *job)
{
   QDeclarativeXmlQueryResult result;
   result.queryId = job->queryId;
   doQueryJob(job, &result);
   doSubQueryJob(job, &result);

   {
      QMutexLocker ml(&m_mutex);
      if (m_cancelledJobs.contains(job->queryId)) {
         m_cancelledJobs.remove(job->queryId);
      } else {
         emit queryCompleted(result);
      }
   }
}

void QDeclarativeXmlQueryEngine::doQueryJob(XmlQueryJob *currentJob, QDeclarativeXmlQueryResult *currentResult)
{
   Q_ASSERT(currentJob->queryId != -1);

   QString r;
   QXmlQuery query;
   QBuffer buffer(&currentJob->data);
   buffer.open(QIODevice::ReadOnly);
   query.bindVariable(QLatin1String("src"), &buffer);
   query.setQuery(currentJob->namespaces + currentJob->query);
   query.evaluateTo(&r);

   //always need a single root element
   QByteArray xml = "<dummy:items xmlns:dummy=\"http://qtsotware.com/dummy\">\n" + r.toUtf8() + "</dummy:items>";
   QBuffer b(&xml);
   b.open(QIODevice::ReadOnly);

   QString namespaces = QLatin1String("declare namespace dummy=\"http://qtsotware.com/dummy\";\n") +
                        currentJob->namespaces;
   QString prefix = QLatin1String("doc($inputDocument)/dummy:items") +
                    currentJob->query.mid(currentJob->query.lastIndexOf(QLatin1Char('/')));

   //figure out how many items we are dealing with
   int count = -1;
   {
      QXmlResultItems result;
      QXmlQuery countquery;
      countquery.bindVariable(QLatin1String("inputDocument"), &b);
      countquery.setQuery(namespaces + QLatin1String("count(") + prefix + QLatin1Char(')'));
      countquery.evaluateTo(&result);
      QXmlItem item(result.next());
      if (item.isAtomicValue()) {
         count = item.toAtomicValue().toInt();
      }
   }

   currentJob->data = xml;
   currentJob->prefix = namespaces + prefix + QLatin1Char('/');
   currentResult->size = (count > 0 ? count : 0);
}

void QDeclarativeXmlQueryEngine::getValuesOfKeyRoles(const XmlQueryJob &currentJob, QStringList *values,
      QXmlQuery *query) const
{
   const QStringList &keysQueries = currentJob.keyRoleQueries;
   QString keysQuery;
   if (keysQueries.count() == 1) {
      keysQuery = currentJob.prefix + keysQueries[0];
   } else if (keysQueries.count() > 1) {
      keysQuery = currentJob.prefix + QLatin1String("concat(") + keysQueries.join(QLatin1String(",")) + QLatin1String(")");
   }

   if (!keysQuery.isEmpty()) {
      query->setQuery(keysQuery);
      QXmlResultItems resultItems;
      query->evaluateTo(&resultItems);
      QXmlItem item(resultItems.next());
      while (!item.isNull()) {
         values->append(item.toAtomicValue().toString());
         item = resultItems.next();
      }
   }
}

void QDeclarativeXmlQueryEngine::addIndexToRangeList(QList<QDeclarativeXmlListRange> *ranges, int index) const
{
   if (ranges->isEmpty()) {
      ranges->append(qMakePair(index, 1));
   } else if (ranges->last().first + ranges->last().second == index) {
      ranges->last().second += 1;
   } else {
      ranges->append(qMakePair(index, 1));
   }
}

void QDeclarativeXmlQueryEngine::doSubQueryJob(XmlQueryJob *currentJob, QDeclarativeXmlQueryResult *currentResult)
{
   Q_ASSERT(currentJob->queryId != -1);

   QBuffer b(&currentJob->data);
   b.open(QIODevice::ReadOnly);

   QXmlQuery subquery;
   subquery.bindVariable(QLatin1String("inputDocument"), &b);

   QStringList keyRoleResults;
   getValuesOfKeyRoles(*currentJob, &keyRoleResults, &subquery);

   // See if any values of key roles have been inserted or removed.

   if (currentJob->keyRoleResultsCache.isEmpty()) {
      currentResult->inserted << qMakePair(0, currentResult->size);
   } else {
      if (keyRoleResults != currentJob->keyRoleResultsCache) {
         QStringList temp;
         for (int i = 0; i < currentJob->keyRoleResultsCache.count(); i++) {
            if (!keyRoleResults.contains(currentJob->keyRoleResultsCache[i])) {
               addIndexToRangeList(&currentResult->removed, i);
            } else {
               temp << currentJob->keyRoleResultsCache[i];
            }
         }

         for (int i = 0; i < keyRoleResults.count(); i++) {
            if (temp.count() == i || keyRoleResults[i] != temp[i]) {
               temp.insert(i, keyRoleResults[i]);
               addIndexToRangeList(&currentResult->inserted, i);
            }
         }
      }
   }
   currentResult->keyRoleResultsCache = keyRoleResults;

   // Get the new values for each role.
   //### we might be able to condense even further (query for everything in one go)
   const QStringList &queries = currentJob->roleQueries;
   for (int i = 0; i < queries.size(); ++i) {
      QList<QVariant> resultList;
      if (!queries[i].isEmpty()) {
         subquery.setQuery(currentJob->prefix + QLatin1String("(let $v := string(") + queries[i] +
                           QLatin1String(") return if ($v) then ") + queries[i] + QLatin1String(" else \"\")"));
         if (subquery.isValid()) {
            QXmlResultItems resultItems;
            subquery.evaluateTo(&resultItems);
            QXmlItem item(resultItems.next());
            while (!item.isNull()) {
               resultList << item.toAtomicValue(); //### we used to trim strings
               item = resultItems.next();
            }
         } else {
            emit error(currentJob->roleQueryErrorId.at(i), queries[i]);
         }
      }
      //### should warn here if things have gone wrong.
      while (resultList.count() < currentResult->size) {
         resultList << QVariant();
      }
      currentResult->data << resultList;
      b.seek(0);
   }

   //this method is much slower, but works better for incremental loading
   /*for (int j = 0; j < m_size; ++j) {
       QList<QVariant> resultList;
       for (int i = 0; i < m_roleObjects->size(); ++i) {
           QDeclarativeXmlListModelRole *role = m_roleObjects->at(i);
           subquery.setQuery(m_prefix.arg(j+1) + role->query());
           if (role->isStringList()) {
               QStringList data;
               subquery.evaluateTo(&data);
               resultList << QVariant(data);
               //qDebug() << data;
           } else {
               QString s;
               subquery.evaluateTo(&s);
               if (role->isCData()) {
                   //un-escape
                   s.replace(QLatin1String("&lt;"), QLatin1String("<"));
                   s.replace(QLatin1String("&gt;"), QLatin1String(">"));
                   s.replace(QLatin1String("&amp;"), QLatin1String("&"));
               }
               resultList << s.trimmed();
               //qDebug() << s;
           }
           b.seek(0);
       }
       m_modelData << resultList;
   }*/
}

class QDeclarativeXmlListModelPrivate
{
   Q_DECLARE_PUBLIC(QDeclarativeXmlListModel)

 public:
   QDeclarativeXmlListModelPrivate()
      : isComponentComplete(true), size(-1), highestRole(Qt::UserRole)
      , reply(0), status(QDeclarativeXmlListModel::Null), progress(0.0)
      , queryId(-1), roleObjects(), redirectCount(0) {}


   void notifyQueryStarted(bool remoteSource) {
      Q_Q(QDeclarativeXmlListModel);
      progress = remoteSource ? qreal(0.0) : qreal(1.0);
      status = QDeclarativeXmlListModel::Loading;
      errorString.clear();
      emit q->progressChanged(progress);
      emit q->statusChanged(status);
   }

   void deleteReply() {
      Q_Q(QDeclarativeXmlListModel);
      if (reply) {
         QObject::disconnect(reply, 0, q, 0);
         reply->deleteLater();
         reply = 0;
      }
   }

   bool isComponentComplete;
   QUrl src;
   QString xml;
   QString query;
   QString namespaces;
   int size;
   QList<int> roles;
   QStringList roleNames;
   int highestRole;

   QNetworkReply *reply;
   QDeclarativeXmlListModel::Status status;
   QString errorString;
   qreal progress;
   int queryId;
   QStringList keyRoleResultsCache;
   QList<QDeclarativeXmlListModelRole *> roleObjects;

   static void append_role(QDeclarativeListProperty<QDeclarativeXmlListModelRole> *list,
                           QDeclarativeXmlListModelRole *role);
   static void clear_role(QDeclarativeListProperty<QDeclarativeXmlListModelRole> *list);
   QList<QList<QVariant> > data;
   int redirectCount;
};


void QDeclarativeXmlListModelPrivate::append_role(QDeclarativeListProperty<QDeclarativeXmlListModelRole> *list,
      QDeclarativeXmlListModelRole *role)
{
   QDeclarativeXmlListModel *_this = qobject_cast<QDeclarativeXmlListModel *>(list->object);
   if (_this && role) {
      int i = _this->d_func()->roleObjects.count();
      _this->d_func()->roleObjects.append(role);
      if (_this->d_func()->roleNames.contains(role->name())) {
         qmlInfo(role) << QObject::tr("\"%1\" duplicates a previous role name and will be disabled.").arg(role->name());
         return;
      }
      _this->d_func()->roles.insert(i, _this->d_func()->highestRole);
      _this->d_func()->roleNames.insert(i, role->name());
      ++_this->d_func()->highestRole;
   }
}

//### clear needs to invalidate any cached data (in data table) as well
//    (and the model should emit the appropriate signals)
void QDeclarativeXmlListModelPrivate::clear_role(QDeclarativeListProperty<QDeclarativeXmlListModelRole> *list)
{
   QDeclarativeXmlListModel *_this = static_cast<QDeclarativeXmlListModel *>(list->object);
   _this->d_func()->roles.clear();
   _this->d_func()->roleNames.clear();
   _this->d_func()->roleObjects.clear();
}

/*!
    \qmlclass XmlListModel QDeclarativeXmlListModel
    \ingroup qml-working-with-data
  \since 4.7
    \brief The XmlListModel element is used to specify a read-only model using XPath expressions.

    XmlListModel is used to create a read-only model from XML data. It can be used as a data source
    for view elements (such as ListView, PathView, GridView) and other elements that interact with model
    data (such as \l Repeater).

    For example, if there is a XML document at http://www.mysite.com/feed.xml like this:

    \code
    <?xml version="1.0" encoding="utf-8"?>
    <rss version="2.0">
        ...
        <channel>
            <item>
                <title>A blog post</title>
                <pubDate>Sat, 07 Sep 2010 10:00:01 GMT</pubDate>
            </item>
            <item>
                <title>Another blog post</title>
                <pubDate>Sat, 07 Sep 2010 15:35:01 GMT</pubDate>
            </item>
        </channel>
    </rss>
    \endcode

    A XmlListModel could create a model from this data, like this:

    \qml
    import QtQuick 1.0

    XmlListModel {
        id: xmlModel
        source: "http://www.mysite.com/feed.xml"
        query: "/rss/channel/item"

        XmlRole { name: "title"; query: "title/string()" }
        XmlRole { name: "pubDate"; query: "pubDate/string()" }
    }
    \endqml

    The \l {XmlListModel::query}{query} value of "/rss/channel/item" specifies that the XmlListModel should generate
    a model item for each \c <item> in the XML document.

    The XmlRole objects define the
    model item attributes. Here, each model item will have \c title and \c pubDate
    attributes that match the \c title and \c pubDate values of its corresponding \c <item>.
    (See \l XmlRole::query for more examples of valid XPath expressions for XmlRole.)

    The model could be used in a ListView, like this:

    \qml
    ListView {
        width: 180; height: 300
        model: xmlModel
        delegate: Text { text: title + ": " + pubDate }
    }
    \endqml

    \image qml-xmllistmodel-example.png

    The XmlListModel data is loaded asynchronously, and \l status
    is set to \c XmlListModel.Ready when loading is complete.
    Note this means when XmlListModel is used for a view, the view is not
    populated until the model is loaded.


    \section2 Using key XML roles

    You can define certain roles as "keys" so that when reload() is called,
    the model will only add and refresh data that contains new values for
    these keys.

    For example, if above role for "pubDate" was defined like this instead:

    \qml
        XmlRole { name: "pubDate"; query: "pubDate/string()"; isKey: true }
    \endqml

    Then when reload() is called, the model will only add and reload
    items with a "pubDate" value that is not already
    present in the model.

    This is useful when displaying the contents of XML documents that
    are incrementally updated (such as RSS feeds) to avoid repainting the
    entire contents of a model in a view.

    If multiple key roles are specified, the model only adds and reload items
    with a combined value of all key roles that is not already present in
    the model.

    \sa {RSS News}
*/

QDeclarativeXmlListModel::QDeclarativeXmlListModel(QObject *parent)
   : QListModelInterface(*(new QDeclarativeXmlListModelPrivate), parent)
{
}

QDeclarativeXmlListModel::~QDeclarativeXmlListModel()
{
}

/*!
    \qmlproperty list<XmlRole> XmlListModel::roles

    The roles to make available for this model.
*/
QDeclarativeListProperty<QDeclarativeXmlListModelRole> QDeclarativeXmlListModel::roleObjects()
{
   Q_D(QDeclarativeXmlListModel);
   QDeclarativeListProperty<QDeclarativeXmlListModelRole> list(this, d->roleObjects);
   list.append = &QDeclarativeXmlListModelPrivate::append_role;
   list.clear = &QDeclarativeXmlListModelPrivate::clear_role;
   return list;
}

QHash<int, QVariant> QDeclarativeXmlListModel::data(int index, const QList<int> &roles) const
{
   Q_D(const QDeclarativeXmlListModel);
   QHash<int, QVariant> rv;
   for (int i = 0; i < roles.size(); ++i) {
      int role = roles.at(i);
      int roleIndex = d->roles.indexOf(role);
      rv.insert(role, roleIndex == -1 ? QVariant() : d->data.value(roleIndex).value(index));
   }
   return rv;
}

QVariant QDeclarativeXmlListModel::data(int index, int role) const
{
   Q_D(const QDeclarativeXmlListModel);
   int roleIndex = d->roles.indexOf(role);
   return (roleIndex == -1) ? QVariant() : d->data.value(roleIndex).value(index);
}

/*!
    \qmlproperty int XmlListModel::count
    The number of data entries in the model.
*/
int QDeclarativeXmlListModel::count() const
{
   Q_D(const QDeclarativeXmlListModel);
   return d->size;
}

QList<int> QDeclarativeXmlListModel::roles() const
{
   Q_D(const QDeclarativeXmlListModel);
   return d->roles;
}

QString QDeclarativeXmlListModel::toString(int role) const
{
   Q_D(const QDeclarativeXmlListModel);
   int index = d->roles.indexOf(role);
   if (index == -1) {
      return QString();
   }
   return d->roleNames.at(index);
}

/*!
    \qmlproperty url XmlListModel::source
    The location of the XML data source.

    If both \c source and \l xml are set, \l xml is used.
*/
QUrl QDeclarativeXmlListModel::source() const
{
   Q_D(const QDeclarativeXmlListModel);
   return d->src;
}

void QDeclarativeXmlListModel::setSource(const QUrl &src)
{
   Q_D(QDeclarativeXmlListModel);
   if (d->src != src) {
      d->src = src;
      if (d->xml.isEmpty()) { // src is only used if d->xml is not set
         reload();
      }
      emit sourceChanged();
   }
}

/*!
    \qmlproperty string XmlListModel::xml
    This property holds the XML data for this model, if set.

    The text is assumed to be UTF-8 encoded.

    If both \l source and \c xml are set, \c xml is used.
*/
QString QDeclarativeXmlListModel::xml() const
{
   Q_D(const QDeclarativeXmlListModel);
   return d->xml;
}

void QDeclarativeXmlListModel::setXml(const QString &xml)
{
   Q_D(QDeclarativeXmlListModel);
   if (d->xml != xml) {
      d->xml = xml;
      reload();
      emit xmlChanged();
   }
}

/*!
    \qmlproperty string XmlListModel::query
    An absolute XPath query representing the base query for creating model items
    from this model's XmlRole objects. The query should start with '/' or '//'.
*/
QString QDeclarativeXmlListModel::query() const
{
   Q_D(const QDeclarativeXmlListModel);
   return d->query;
}

void QDeclarativeXmlListModel::setQuery(const QString &query)
{
   Q_D(QDeclarativeXmlListModel);
   if (!query.startsWith(QLatin1Char('/'))) {
      qmlInfo(this) << QCoreApplication::translate("QDeclarativeXmlRoleList",
                    "An XmlListModel query must start with '/' or \"//\"");
      return;
   }

   if (d->query != query) {
      d->query = query;
      reload();
      emit queryChanged();
   }
}

/*!
    \qmlproperty string XmlListModel::namespaceDeclarations
    The namespace declarations to be used in the XPath queries.

    The namespaces should be declared as in XQuery. For example, if a requested document
    at http://mysite.com/feed.xml uses the namespace "http://www.w3.org/2005/Atom",
    this can be declared as the default namespace:

    \qml
    XmlListModel {
        source: "http://mysite.com/feed.xml"
        query: "/feed/entry"
        namespaceDeclarations: "declare default element namespace 'http://www.w3.org/2005/Atom';"

        XmlRole { name: "title"; query: "title/string()" }
    }
    \endqml
*/
QString QDeclarativeXmlListModel::namespaceDeclarations() const
{
   Q_D(const QDeclarativeXmlListModel);
   return d->namespaces;
}

void QDeclarativeXmlListModel::setNamespaceDeclarations(const QString &declarations)
{
   Q_D(QDeclarativeXmlListModel);
   if (d->namespaces != declarations) {
      d->namespaces = declarations;
      reload();
      emit namespaceDeclarationsChanged();
   }
}

/*!
    \qmlmethod object XmlListModel::get(int index)

    Returns the item at \a index in the model.

    For example, for a model like this:

    \qml
    XmlListModel {
        id: model
        source: "http://mysite.com/feed.xml"
        query: "/feed/entry"
        XmlRole { name: "title"; query: "title/string()" }
    }
    \endqml

    This will access the \c title value for the first item in the model:

    \js
    var title = model.get(0).title;
    \endjs
*/
QScriptValue QDeclarativeXmlListModel::get(int index) const
{
   Q_D(const QDeclarativeXmlListModel);

   QScriptEngine *sengine = QDeclarativeEnginePrivate::getScriptEngine(qmlContext(this)->engine());
   if (index < 0 || index >= count()) {
      return sengine->undefinedValue();
   }

   QScriptValue sv = sengine->newObject();
   for (int i = 0; i < d->roleObjects.count(); i++) {
      sv.setProperty(d->roleObjects[i]->name(), sengine->toScriptValue(d->data.value(i).value(index)));
   }
   return sv;
}

/*!
    \qmlproperty enumeration XmlListModel::status
    Specifies the model loading status, which can be one of the following:

    \list
    \o XmlListModel.Null - No XML data has been set for this model.
    \o XmlListModel.Ready - The XML data has been loaded into the model.
    \o XmlListModel.Loading - The model is in the process of reading and loading XML data.
    \o XmlListModel.Error - An error occurred while the model was loading. See errorString() for details
       about the error.
    \endlist

    \sa progress

*/
QDeclarativeXmlListModel::Status QDeclarativeXmlListModel::status() const
{
   Q_D(const QDeclarativeXmlListModel);
   return d->status;
}

/*!
    \qmlproperty real XmlListModel::progress

    This indicates the current progress of the downloading of the XML data
    source. This value ranges from 0.0 (no data downloaded) to
    1.0 (all data downloaded). If the XML data is not from a remote source,
    the progress becomes 1.0 as soon as the data is read.

    Note that when the progress is 1.0, the XML data has been downloaded, but
    it is yet to be loaded into the model at this point. Use the status
    property to find out when the XML data has been read and loaded into
    the model.

    \sa status, source
*/
qreal QDeclarativeXmlListModel::progress() const
{
   Q_D(const QDeclarativeXmlListModel);
   return d->progress;
}

/*!
    \qmlmethod void XmlListModel::errorString()

    Returns a string description of the last error that occurred
    if \l status is XmlListModel::Error.
*/
QString QDeclarativeXmlListModel::errorString() const
{
   Q_D(const QDeclarativeXmlListModel);
   return d->errorString;
}

void QDeclarativeXmlListModel::classBegin()
{
   Q_D(QDeclarativeXmlListModel);
   d->isComponentComplete = false;

   QDeclarativeXmlQueryEngine *queryEngine = QDeclarativeXmlQueryEngine::instance(qmlEngine(this));
   connect(queryEngine, SIGNAL(queryCompleted(QDeclarativeXmlQueryResult)),
           SLOT(queryCompleted(QDeclarativeXmlQueryResult)));
   connect(queryEngine, SIGNAL(error(void *, QString)),
           SLOT(queryError(void *, QString)));
}

void QDeclarativeXmlListModel::componentComplete()
{
   Q_D(QDeclarativeXmlListModel);
   d->isComponentComplete = true;
   reload();
}

/*!
    \qmlmethod XmlListModel::reload()

    Reloads the model.

    If no key roles have been specified, all existing model
    data is removed, and the model is rebuilt from scratch.

    Otherwise, items are only added if the model does not already
    contain items with matching key role values.

    \sa {Using key XML roles}, XmlRole::isKey
*/
void QDeclarativeXmlListModel::reload()
{
   Q_D(QDeclarativeXmlListModel);

   if (!d->isComponentComplete) {
      return;
   }

   QDeclarativeXmlQueryEngine::instance(qmlEngine(this))->abort(d->queryId);
   d->queryId = -1;

   if (d->size < 0) {
      d->size = 0;
   }

   if (d->reply) {
      d->reply->abort();
      d->deleteReply();
   }

   if (!d->xml.isEmpty()) {
      d->queryId = QDeclarativeXmlQueryEngine::instance(qmlEngine(this))->doQuery(d->query, d->namespaces, d->xml.toUtf8(),
                   &d->roleObjects, d->keyRoleResultsCache);
      d->notifyQueryStarted(false);

   } else if (d->src.isEmpty()) {
      d->queryId = XMLLISTMODEL_CLEAR_ID;
      d->notifyQueryStarted(false);
      QTimer::singleShot(0, this, SLOT(dataCleared()));

   } else {
      d->notifyQueryStarted(true);
      QNetworkRequest req(d->src);
      req.setRawHeader("Accept", "application/xml,*/*");
      d->reply = qmlContext(this)->engine()->networkAccessManager()->get(req);
      QObject::connect(d->reply, SIGNAL(finished()), this, SLOT(requestFinished()));
      QObject::connect(d->reply, SIGNAL(downloadProgress(qint64, qint64)),
                       this, SLOT(requestProgress(qint64, qint64)));
   }
}

#define XMLLISTMODEL_MAX_REDIRECT 16

void QDeclarativeXmlListModel::requestFinished()
{
   Q_D(QDeclarativeXmlListModel);

   d->redirectCount++;
   if (d->redirectCount < XMLLISTMODEL_MAX_REDIRECT) {
      QVariant redirect = d->reply->attribute(QNetworkRequest::RedirectionTargetAttribute);
      if (redirect.isValid()) {
         QUrl url = d->reply->url().resolved(redirect.toUrl());
         d->deleteReply();
         setSource(url);
         return;
      }
   }
   d->redirectCount = 0;

   if (d->reply->error() != QNetworkReply::NoError) {
      d->errorString = d->reply->errorString();
      d->deleteReply();

      int count = this->count();
      d->data.clear();
      d->size = 0;
      if (count > 0) {
         emit itemsRemoved(0, count);
         emit countChanged();
      }

      d->status = Error;
      d->queryId = -1;
      emit statusChanged(d->status);
   } else {
      QByteArray data = d->reply->readAll();
      if (data.isEmpty()) {
         d->queryId = XMLLISTMODEL_CLEAR_ID;
         QTimer::singleShot(0, this, SLOT(dataCleared()));
      } else {
         d->queryId = QDeclarativeXmlQueryEngine::instance(qmlEngine(this))->doQuery(d->query, d->namespaces, data,
                      &d->roleObjects, d->keyRoleResultsCache);
      }
      d->deleteReply();

      d->progress = 1.0;
      emit progressChanged(d->progress);
   }
}

void QDeclarativeXmlListModel::requestProgress(qint64 received, qint64 total)
{
   Q_D(QDeclarativeXmlListModel);
   if (d->status == Loading && total > 0) {
      d->progress = qreal(received) / total;
      emit progressChanged(d->progress);
   }
}

void QDeclarativeXmlListModel::dataCleared()
{
   Q_D(QDeclarativeXmlListModel);
   QDeclarativeXmlQueryResult r;
   r.queryId = XMLLISTMODEL_CLEAR_ID;
   r.size = 0;
   r.removed << qMakePair(0, count());
   r.keyRoleResultsCache = d->keyRoleResultsCache;
   queryCompleted(r);
}

void QDeclarativeXmlListModel::queryError(void *object, const QString &error)
{
   // Be extra careful, object may no longer exist, it's just an ID.
   Q_D(QDeclarativeXmlListModel);
   for (int i = 0; i < d->roleObjects.count(); i++) {
      if (d->roleObjects.at(i) == static_cast<QDeclarativeXmlListModelRole *>(object)) {
         qmlInfo(d->roleObjects.at(i)) << QObject::tr("invalid query: \"%1\"").arg(error);
         return;
      }
   }
   qmlInfo(this) << QObject::tr("invalid query: \"%1\"").arg(error);
}

void QDeclarativeXmlListModel::queryCompleted(const QDeclarativeXmlQueryResult &result)
{
   Q_D(QDeclarativeXmlListModel);
   if (result.queryId != d->queryId) {
      return;
   }

   int origCount = d->size;
   bool sizeChanged = result.size != d->size;

   d->size = result.size;
   d->data = result.data;
   d->keyRoleResultsCache = result.keyRoleResultsCache;
   d->status = Ready;
   d->errorString.clear();
   d->queryId = -1;

   bool hasKeys = false;
   for (int i = 0; i < d->roleObjects.count(); i++) {
      if (d->roleObjects[i]->isKey()) {
         hasKeys = true;
         break;
      }
   }
   if (!hasKeys) {
      if (!(origCount == 0 && d->size == 0)) {
         emit itemsRemoved(0, origCount);
         emit itemsInserted(0, d->size);
         emit countChanged();
      }

   } else {
      for (int i = 0; i < result.removed.count(); i++) {
         emit itemsRemoved(result.removed[i].first, result.removed[i].second);
      }
      for (int i = 0; i < result.inserted.count(); i++) {
         emit itemsInserted(result.inserted[i].first, result.inserted[i].second);
      }

      if (sizeChanged) {
         emit countChanged();
      }
   }

   emit statusChanged(d->status);
}

QT_END_NAMESPACE
