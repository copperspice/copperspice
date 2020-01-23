/***********************************************************************
*
* Copyright (c) 2012-2020 Barbara Geller
* Copyright (c) 2012-2020 Ansel Sermersheim
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

#include "qscriptdebuggerscriptsmodel_p.h"
#include "qscriptscriptdata_p.h"
#include "qabstractitemmodel_p.h"
#include <QtCore/qfileinfo.h>
#include <QtCore/qpair.h>
#include <QtCore/qdebug.h>

QT_BEGIN_NAMESPACE

class QScriptDebuggerScriptsModelPrivate
   : public QAbstractItemModelPrivate
{
   Q_DECLARE_PUBLIC(QScriptDebuggerScriptsModel)

 public:
   struct Node {
      Node(qint64 sid, const QScriptScriptData &dt)
         : scriptId(sid), data(dt) {}

      qint64 scriptId;
      QScriptScriptData data;
      QList<QPair<QString, int> > functionsInfo;
      QSet<int> executableLineNumbers;
   };

   QScriptDebuggerScriptsModelPrivate();
   ~QScriptDebuggerScriptsModelPrivate();

   Node *findScriptNode(qint64 scriptId) const;

   int nextNodeId;
   QMap<int, Node *> nodes;
};

QScriptDebuggerScriptsModelPrivate::QScriptDebuggerScriptsModelPrivate()
{
   nextNodeId = 0;
}

QScriptDebuggerScriptsModelPrivate::~QScriptDebuggerScriptsModelPrivate()
{
   qDeleteAll(nodes);
}

QScriptDebuggerScriptsModelPrivate::Node *QScriptDebuggerScriptsModelPrivate::findScriptNode(qint64 scriptId) const
{
   QMap<int, Node *>::const_iterator it;
   for (it = nodes.constBegin(); it != nodes.constEnd(); ++it) {
      Node *n = it.value();
      if (n->scriptId == scriptId) {
         return n;
      }
   }
   return 0;
}

QScriptDebuggerScriptsModel::QScriptDebuggerScriptsModel(QObject *parent)
   : QAbstractItemModel(*new QScriptDebuggerScriptsModelPrivate, parent)
{
}

QScriptDebuggerScriptsModel::~QScriptDebuggerScriptsModel()
{
}

void QScriptDebuggerScriptsModel::removeScript(qint64 id)
{
   Q_D(QScriptDebuggerScriptsModel);
   QMap<int, QScriptDebuggerScriptsModelPrivate::Node *>::iterator it;
   for (it = d->nodes.begin(); it != d->nodes.end(); ++it) {
      QScriptDebuggerScriptsModelPrivate::Node *n = it.value();
      if (n->scriptId == id) {
         d->nodes.erase(it);
         delete n;
         break;
      }
   }
}

void QScriptDebuggerScriptsModel::addScript(qint64 sid, const QScriptScriptData &data)
{
   Q_D(QScriptDebuggerScriptsModel);
   int id = d->nextNodeId;
   ++d->nextNodeId;
   d->nodes.insert(id, new QScriptDebuggerScriptsModelPrivate::Node(sid, data));
}

void QScriptDebuggerScriptsModel::addExtraScriptInfo(
   qint64 sid, const QMap<QString, int> &functionsInfo,
   const QSet<int> &executableLineNumbers)
{
   Q_D(QScriptDebuggerScriptsModel);
   QScriptDebuggerScriptsModelPrivate::Node *node = d->findScriptNode(sid);
   if (!node) {
      return;
   }
   QList<QPair<QString, int> > lst;
   QMap<QString, int>::const_iterator it;
   for (it = functionsInfo.constBegin(); it != functionsInfo.constEnd(); ++it) {
      lst.append(qMakePair(it.key(), it.value()));
   }
   node->functionsInfo = lst;
   node->executableLineNumbers = executableLineNumbers;
}

void QScriptDebuggerScriptsModel::commit()
{
   layoutAboutToBeChanged();
   layoutChanged();
}

QScriptScriptData QScriptDebuggerScriptsModel::scriptData(qint64 sid) const
{
   Q_D(const QScriptDebuggerScriptsModel);
   QScriptDebuggerScriptsModelPrivate::Node *node = d->findScriptNode(sid);
   if (!node) {
      return QScriptScriptData();
   }
   return node->data;
}

QScriptScriptMap QScriptDebuggerScriptsModel::scripts() const
{
   Q_D(const QScriptDebuggerScriptsModel);
   QScriptScriptMap result;
   QMap<int, QScriptDebuggerScriptsModelPrivate::Node *>::const_iterator it;
   for (it = d->nodes.constBegin(); it != d->nodes.constEnd(); ++it) {
      QScriptDebuggerScriptsModelPrivate::Node *n = it.value();
      result.insert(n->scriptId, n->data);
   }
   return result;
}

qint64 QScriptDebuggerScriptsModel::resolveScript(const QString &fileName) const
{
   Q_D(const QScriptDebuggerScriptsModel);
   QMap<int, QScriptDebuggerScriptsModelPrivate::Node *>::const_iterator it;
   for (it = d->nodes.constBegin(); it != d->nodes.constEnd(); ++it) {
      QScriptDebuggerScriptsModelPrivate::Node *n = it.value();
      if (n->data.fileName() == fileName) {
         return n->scriptId;
      }
   }
   return -1;
}

QSet<int> QScriptDebuggerScriptsModel::executableLineNumbers(qint64 scriptId) const
{
   Q_D(const QScriptDebuggerScriptsModel);
   QScriptDebuggerScriptsModelPrivate::Node *node = d->findScriptNode(scriptId);
   if (!node) {
      return QSet<int>();
   }
   return node->executableLineNumbers;
}

QModelIndex QScriptDebuggerScriptsModel::indexFromScriptId(qint64 sid) const
{
   Q_D(const QScriptDebuggerScriptsModel);
   int row = 0;
   QMap<int, QScriptDebuggerScriptsModelPrivate::Node *>::const_iterator it;
   for (it = d->nodes.constBegin(); it != d->nodes.constEnd(); ++it, ++row) {
      QScriptDebuggerScriptsModelPrivate::Node *n = it.value();
      if (n->scriptId == sid) {
         return createIndex(row, 0, it.key() << 12);
      }
   }
   return QModelIndex();
}

qint64 QScriptDebuggerScriptsModel::scriptIdFromIndex(const QModelIndex &index) const
{
   Q_D(const QScriptDebuggerScriptsModel);
   if (!index.isValid()) {
      return -1;
   }
   int id = index.internalId();
   if (id & 1) {
      return -1;
   }
   QScriptDebuggerScriptsModelPrivate::Node *n = d->nodes.value(id >> 12);
   if (!n) {
      return -1;
   }
   return n->scriptId;
}

QPair<QString, int> QScriptDebuggerScriptsModel::scriptFunctionInfoFromIndex(const QModelIndex &index) const
{
   Q_D(const QScriptDebuggerScriptsModel);
   QPair<QString, int> result;
   if (!index.isValid()) {
      return result;
   }
   int id = index.internalId();
   if (!(id & 1)) {
      return result;
   }
   QScriptDebuggerScriptsModelPrivate::Node *node = d->nodes.value(id >> 12);
   if (!node) {
      return result;
   }
   int functionIndex = (id >> 1) & ((1 << 11) - 1);
   result = node->functionsInfo.at(functionIndex);
   return result;
}

/*!
  \reimp
*/
QModelIndex QScriptDebuggerScriptsModel::index(int row, int column, const QModelIndex &parent) const
{
   Q_D(const QScriptDebuggerScriptsModel);
   if (!parent.isValid()) {
      if ((row < 0) || (row >= d->nodes.size())) {
         return QModelIndex();
      }
      if (column != 0) {
         return QModelIndex();
      }
      return createIndex(row, column, d->nodes.keys().at(row) << 12);
   }
   int id = parent.internalId();
   if (id & 1) {
      return QModelIndex();
   }
   return createIndex(row, column, id | (row << 1) | 1);
}

/*!
  \reimp
*/
QModelIndex QScriptDebuggerScriptsModel::parent(const QModelIndex &index) const
{
   Q_D(const QScriptDebuggerScriptsModel);
   if (!index.isValid()) {
      return QModelIndex();
   }
   int id = index.internalId();
   if (!(id & 1)) {
      return QModelIndex();
   }
   QScriptDebuggerScriptsModelPrivate::Node *n = d->nodes.value(id >> 12);
   if (!n) {
      return QModelIndex();
   }
   return indexFromScriptId(n->scriptId);
}

/*!
  \reimp
*/
int QScriptDebuggerScriptsModel::columnCount(const QModelIndex &) const
{
   return 1;
}

/*!
  \reimp
*/
int QScriptDebuggerScriptsModel::rowCount(const QModelIndex &parent) const
{
   Q_D(const QScriptDebuggerScriptsModel);
   if (!parent.isValid()) {
      return d->nodes.size();
   }
   int id = parent.internalId();
   if (id & 1) {
      return 0;
   }
   QScriptDebuggerScriptsModelPrivate::Node *n = d->nodes.value(id >> 12);
   if (!n) {
      return 0;
   }
   return n->functionsInfo.size();
}

/*!
  \reimp
*/
QVariant QScriptDebuggerScriptsModel::data(const QModelIndex &index, int role) const
{
   Q_D(const QScriptDebuggerScriptsModel);
   if (!index.isValid()) {
      return QVariant();
   }
   int id = index.internalId();
   QScriptDebuggerScriptsModelPrivate::Node *node = d->nodes.value(id >> 12);
   if (!node) {
      return QVariant();
   }
   if (!(id & 1)) {
      if (role == Qt::DisplayRole) {
         QString fn = node->data.fileName();
         if (fn.isEmpty()) {
            fn = QString::fromLatin1("<anonymous script, id=%0>").arg(node->scriptId);
         }
         return fn;
      } else if (role == Qt::ToolTipRole) {
         QString fn = node->data.fileName();
         if (QFileInfo(fn).fileName() != fn) {
            return fn;
         }
      } else if (role == Qt::UserRole) {
         return node->scriptId;
      } else if (role == Qt::UserRole + 1) {
         return node->data.baseLineNumber();
      } else if (role == Qt::UserRole + 2) {
         return node->data.contents();
      }
   } else {
      int functionIndex = (id >> 1) & ((1 << 11) - 1);
      if (role == Qt::DisplayRole) {
         return node->functionsInfo[functionIndex].first;
      }
   }
   return QVariant();
}

QT_END_NAMESPACE
