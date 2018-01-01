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

#include <algorithm>

#include <qfilesystemmodel_p.h>
#include <qfilesystemmodel.h>
#include <qlocale.h>
#include <qmime.h>
#include <qurl.h>
#include <qdebug.h>
#include <qmessagebox.h>
#include <qapplication.h>

#ifdef Q_OS_WIN
#include <qt_windows.h>
#endif

#ifdef Q_OS_WIN32
#include <QtCore/QVarLengthArray>
#endif

QT_BEGIN_NAMESPACE

#ifndef QT_NO_FILESYSTEMMODEL

bool QFileSystemModel::remove(const QModelIndex &aindex) const
{
   //### TODO optim
   QString path = filePath(aindex);
   QFileSystemModelPrivate *d = const_cast<QFileSystemModelPrivate *>(d_func());

   d->fileInfoGatherer.removePath(path);

   if (QFileInfo(path).isFile()) {
      return QFile::remove(path);
   }

   return QDir(path).removeRecursively();
}

QFileSystemModel::QFileSystemModel(QObject *parent)
   : QAbstractItemModel(*new QFileSystemModelPrivate, parent)
{
   Q_D(QFileSystemModel);
   d->init();
}

QFileSystemModel::QFileSystemModel(QFileSystemModelPrivate &dd, QObject *parent)
   : QAbstractItemModel(dd, parent)
{
   Q_D(QFileSystemModel);
   d->init();
}

QFileSystemModel::~QFileSystemModel()
{
}

QModelIndex QFileSystemModel::index(int row, int column, const QModelIndex &parent) const
{
   Q_D(const QFileSystemModel);

   if (row < 0 || column < 0 || row >= rowCount(parent) || column >= columnCount(parent)) {
      return QModelIndex();
   }

   // get the parent node
   QFileSystemModelPrivate::QFileSystemNode *parentNode = (d->indexValid(parent) ? d->node(parent) :
         const_cast<QFileSystemModelPrivate::QFileSystemNode *>(&d->root));
   Q_ASSERT(parentNode);

   // now get the internal pointer for the index
   QString childName = parentNode->visibleChildren[d->translateVisibleLocation(parentNode, row)];
   const QFileSystemModelPrivate::QFileSystemNode *indexNode = parentNode->children.value(childName);

   Q_ASSERT(indexNode);

   return createIndex(row, column, const_cast<QFileSystemModelPrivate::QFileSystemNode *>(indexNode));
}

QModelIndex QFileSystemModel::index(const QString &path, int column) const
{
   Q_D(const QFileSystemModel);

   QFileSystemModelPrivate::QFileSystemNode *node = d->node(path, false);
   QModelIndex idx = d->index(node);

   if (idx.column() != column) {
      idx = idx.sibling(idx.row(), column);
   }

   return idx;
}

QFileSystemModelPrivate::QFileSystemNode *QFileSystemModelPrivate::node(const QModelIndex &index) const
{
   if (! index.isValid()) {
      return const_cast<QFileSystemNode *>(&root);
   }

   QFileSystemModelPrivate::QFileSystemNode *indexNode =
         static_cast<QFileSystemModelPrivate::QFileSystemNode *>(index.internalPointer());

   Q_ASSERT(indexNode);

   return indexNode;
}

#ifdef Q_OS_WIN32
static QString qt_GetLongPathName(const QString &strShortPath)
{
   if (strShortPath.isEmpty() || strShortPath == QLatin1String(".") || strShortPath == QLatin1String("..")) {
      return strShortPath;
   }

   if (strShortPath.length() == 2 && strShortPath.endsWith(QLatin1Char(':'))) {
      return strShortPath.toUpper();
   }

   const QString absPath = QDir(strShortPath).absolutePath();
   if (absPath.startsWith(QLatin1String("//")) || absPath.startsWith(QLatin1String("\\\\"))) {
      // unc
      return QDir::fromNativeSeparators(absPath);
   }

   if (absPath.startsWith(QLatin1Char('/'))) {
      return QString();
   }

   const QString inputString = QLatin1String("\\\\?\\") + QDir::toNativeSeparators(absPath);
   QVarLengthArray<TCHAR, MAX_PATH> buffer(MAX_PATH);
   DWORD result = ::GetLongPathName((wchar_t *)inputString.utf16(), buffer.data(),buffer.size());

   if (result > DWORD(buffer.size())) {
      buffer.resize(result);
      result = ::GetLongPathName((wchar_t *)inputString.utf16(), buffer.data(), buffer.size());
   }

   if (result > 4) {
      QString longPath = QString::fromWCharArray(buffer.data() + 4);

      // ignoring prefix
      longPath[0] = longPath.at(0).toUpper();

      // capital drive letters
      return QDir::fromNativeSeparators(longPath);

   } else {
      return QDir::fromNativeSeparators(strShortPath);

   }
}
#endif

QFileSystemModelPrivate::QFileSystemNode *QFileSystemModelPrivate::node(const QString &path, bool fetch) const
{
   Q_Q(const QFileSystemModel);
   Q_UNUSED(q);

   if (path.isEmpty() || path == myComputer() || path.startsWith(QLatin1Char(':'))) {
      return const_cast<QFileSystemModelPrivate::QFileSystemNode *>(&root);
   }

   // Construct the nodes up to the new root path if they need to be built
   QString absolutePath;

#ifdef Q_OS_WIN32
   QString longPath = qt_GetLongPathName(path);
#else
   QString longPath = path;
#endif

   if (longPath == rootDir.path()) {
      absolutePath = rootDir.absolutePath();
   } else {
      absolutePath = QDir(longPath).absolutePath();
   }

   // ### TODO can we use bool QAbstractFileEngine::caseSensitive() const?
   QStringList pathElements = absolutePath.split(QLatin1Char('/'), QString::SkipEmptyParts);

#if defined(Q_OS_WIN)
   if ((pathElements.isEmpty())) {

#else
   if ((pathElements.isEmpty()) && QDir::fromNativeSeparators(longPath) != QLatin1String("/")) {

#endif

      return const_cast<QFileSystemModelPrivate::QFileSystemNode *>(&root);
   }

   QModelIndex index = QModelIndex(); // start with "My Computer"

#if defined(Q_OS_WIN)
   if (absolutePath.startsWith(QLatin1String("//"))) {

      // UNC path
      QString host = QLatin1String("\\\\") + pathElements.first();

      if (absolutePath == QDir::fromNativeSeparators(host)) {
         absolutePath.append(QLatin1Char('/'));
      }

      if (longPath.endsWith(QLatin1Char('/')) && !absolutePath.endsWith(QLatin1Char('/'))) {
         absolutePath.append(QLatin1Char('/'));
      }

      int r = 0;
      QFileSystemModelPrivate::QFileSystemNode *rootNode = const_cast<QFileSystemModelPrivate::QFileSystemNode *>(&root);

      if (! root.children.contains(host.toLower())) {
         if (pathElements.count() == 1 && ! absolutePath.endsWith(QLatin1Char('/'))) {
            return rootNode;
         }

         QFileInfo info(host);

         if (! info.exists()) {
            return rootNode;
         }

         QFileSystemModelPrivate *p = const_cast<QFileSystemModelPrivate *>(this);
         p->addNode(rootNode, host, info);
         p->addVisibleFiles(rootNode, QStringList(host));
      }

      r = rootNode->visibleLocation(host);
      r = translateVisibleLocation(rootNode, r);
      index = q->index(r, 0, QModelIndex());
      pathElements.pop_front();

   } else

#endif

#if defined(Q_OS_WIN)
   {
      if (! pathElements.at(0).contains(QLatin1String(":"))) {
         QString rootPath = QDir(longPath).rootPath();
         pathElements.prepend(rootPath);
      }

      if (pathElements.at(0).endsWith(QLatin1Char('/'))) {
         pathElements[0].chop(1);
      }
   }
#else
      // add the "/" item, since it is a valid path element on Unix
      if (absolutePath[0] == QLatin1Char('/')) {
         pathElements.prepend(QLatin1String("/"));
      }
#endif

   QFileSystemModelPrivate::QFileSystemNode *parent = node(index);

   for (int i = 0; i < pathElements.count(); ++i) {
      QString element = pathElements.at(i);

#ifdef Q_OS_WIN
      // On Windows, "filename......." and "filename" are equivalent Task #133928

      while (element.endsWith(QLatin1Char('.'))) {
         element.chop(1);
      }
#endif

      bool alreadyExisted = parent->children.contains(element);

      // could not find the path element, we create a new node
      // since we  _know_ that the path is valid

      if (alreadyExisted) {
         if ((parent->children.count() == 0) || (parent->caseSensitive()
                   && parent->children.value(element)->fileName != element) || (!parent->caseSensitive()
                   && parent->children.value(element)->fileName.toLower() != element.toLower())) {
            alreadyExisted = false;
         }
      }

      QFileSystemModelPrivate::QFileSystemNode *node;
      if (! alreadyExisted) {
         // Someone might call ::index("file://cookie/monster/doesn't/like/veggies"),
         // a path that does npt exists, do not blindly create directories

         QFileInfo info(absolutePath);

         if (! info.exists()) {
            return const_cast<QFileSystemModelPrivate::QFileSystemNode *>(&root);
         }

         QFileSystemModelPrivate *p = const_cast<QFileSystemModelPrivate *>(this);
         node = p->addNode(parent, element, info);

#ifndef QT_NO_FILESYSTEMWATCHER
         node->populate(fileInfoGatherer.getInfo(info));
#endif

      } else {
         node = parent->children.value(element);
      }

      Q_ASSERT(node);

      if (! node->isVisible) {
         // It has been filtered out
         if (alreadyExisted && node->hasInformation() && ! fetch) {
            return const_cast<QFileSystemModelPrivate::QFileSystemNode *>(&root);
         }

         QFileSystemModelPrivate *p = const_cast<QFileSystemModelPrivate *>(this);
         p->addVisibleFiles(parent, QStringList(element));

         if (! p->bypassFilters.contains(node)) {
            p->bypassFilters[node] = 1;
         }

         QString dir = q->filePath(this->index(parent));
         if (! node->hasInformation() && fetch) {
            Fetching f;
            f.dir = dir;
            f.file = element;
            f.node = node;
            p->toFetch.append(f);
            p->fetchingTimer.start(0, const_cast<QFileSystemModel *>(q));
         }
      }
      parent = node;
   }

   return parent;
}

void QFileSystemModel::timerEvent(QTimerEvent *event)
{
   Q_D(QFileSystemModel);

   if (event->timerId() == d->fetchingTimer.timerId()) {
      d->fetchingTimer.stop();

#ifndef QT_NO_FILESYSTEMWATCHER
      for (int i = 0; i < d->toFetch.count(); ++i) {
         const QFileSystemModelPrivate::QFileSystemNode *node = d->toFetch.at(i).node;

         if (! node->hasInformation()) {
            d->fileInfoGatherer.fetchExtendedInformation(d->toFetch.at(i).dir,
                  QStringList(d->toFetch.at(i).file));
         }
      }
#endif
      d->toFetch.clear();
   }
}

bool QFileSystemModel::isDir(const QModelIndex &index) const
{
   // This function is for public usage only because it could create a file info
   Q_D(const QFileSystemModel);

   if (!index.isValid()) {
      return true;
   }

   QFileSystemModelPrivate::QFileSystemNode *n = d->node(index);
   if (n->hasInformation()) {
      return n->isDir();
   }

   return fileInfo(index).isDir();
}

qint64 QFileSystemModel::size(const QModelIndex &index) const
{
   Q_D(const QFileSystemModel);

   if (! index.isValid()) {
      return 0;
   }
   return d->node(index)->size();
}

QString QFileSystemModel::type(const QModelIndex &index) const
{
   Q_D(const QFileSystemModel);

   if (! index.isValid()) {
      return QString();
   }
   return d->node(index)->type();
}

QDateTime QFileSystemModel::lastModified(const QModelIndex &index) const
{
   Q_D(const QFileSystemModel);

   if (!index.isValid()) {
      return QDateTime();
   }
   return d->node(index)->lastModified();
}

QModelIndex QFileSystemModel::parent(const QModelIndex &index) const
{
   Q_D(const QFileSystemModel);

   if (! d->indexValid(index)) {
      return QModelIndex();
   }

   QFileSystemModelPrivate::QFileSystemNode *indexNode = d->node(index);
   Q_ASSERT(indexNode != 0);

   QFileSystemModelPrivate::QFileSystemNode *parentNode = (indexNode ? indexNode->parent : 0);
   if (parentNode == 0 || parentNode == &d->root) {
      return QModelIndex();
   }

   // get the parent's row
   QFileSystemModelPrivate::QFileSystemNode *grandParentNode = parentNode->parent;
   Q_ASSERT(grandParentNode->children.contains(parentNode->fileName));

   int visualRow = d->translateVisibleLocation(grandParentNode,
                   grandParentNode->visibleLocation(grandParentNode->children.value(parentNode->fileName)->fileName));

   if (visualRow == -1) {
      return QModelIndex();
   }

   return createIndex(visualRow, 0, parentNode);
}

QModelIndex QFileSystemModelPrivate::index(const QFileSystemModelPrivate::QFileSystemNode *node) const
{
   Q_Q(const QFileSystemModel);

   QFileSystemModelPrivate::QFileSystemNode *parentNode = (node ? node->parent : 0);

   if (node == &root || !parentNode) {
      return QModelIndex();
   }

   // get the parent's row
   Q_ASSERT(node);
   if (!node->isVisible) {
      return QModelIndex();
   }

   int visualRow = translateVisibleLocation(parentNode, parentNode->visibleLocation(node->fileName));

   return q->createIndex(visualRow, 0, const_cast<QFileSystemNode *>(node));
}

bool QFileSystemModel::hasChildren(const QModelIndex &parent) const
{
   Q_D(const QFileSystemModel);

   if (parent.column() > 0) {
      return false;
   }

   if (! parent.isValid()) {
      // drives
      return true;
   }

   const QFileSystemModelPrivate::QFileSystemNode *indexNode = d->node(parent);
   Q_ASSERT(indexNode);

   return (indexNode->isDir());
}

bool QFileSystemModel::canFetchMore(const QModelIndex &parent) const
{
   Q_D(const QFileSystemModel);

   const QFileSystemModelPrivate::QFileSystemNode *indexNode = d->node(parent);
   return (! indexNode->populatedChildren);
}

void QFileSystemModel::fetchMore(const QModelIndex &parent)
{
   Q_D(QFileSystemModel);

   if (! d->setRootPath) {
      return;
   }

   QFileSystemModelPrivate::QFileSystemNode *indexNode = d->node(parent);
   if (indexNode->populatedChildren) {
      return;
   }

   indexNode->populatedChildren = true;
   d->fileInfoGatherer.list(filePath(parent));
}

int QFileSystemModel::rowCount(const QModelIndex &parent) const
{
   Q_D(const QFileSystemModel);

   if (parent.column() > 0) {
      return 0;
   }

   if (! parent.isValid()) {
      return d->root.visibleChildren.count();
   }

   const QFileSystemModelPrivate::QFileSystemNode *parentNode = d->node(parent);

   return parentNode->visibleChildren.count();
}

int QFileSystemModel::columnCount(const QModelIndex &parent) const
{
   return (parent.column() > 0) ? 0 : 4;
}

QVariant QFileSystemModel::myComputer(int role) const
{
   Q_D(const QFileSystemModel);

   switch (role) {
      case Qt::DisplayRole:
         return d->myComputer();

      case Qt::DecorationRole:
         return d->fileInfoGatherer.iconProvider()->icon(QFileIconProvider::Computer);
   }

   return QVariant();
}

QVariant QFileSystemModel::data(const QModelIndex &index, int role) const
{
   Q_D(const QFileSystemModel);

   if (! index.isValid() || index.model() != this) {
      return QVariant();
   }

   switch (role) {
      case Qt::EditRole:
      case Qt::DisplayRole:
         switch (index.column()) {
            case 0:
               return d->displayName(index);
            case 1:
               return d->size(index);
            case 2:
               return d->type(index);
            case 3:
               return d->time(index);
            default:
               qWarning("QFileSystemModel:data() Invalid display value column %d", index.column());
               break;
         }
         break;

      case FilePathRole:
         return filePath(index);

      case FileNameRole:
         return d->name(index);

      case Qt::DecorationRole:
         if (index.column() == 0) {
            QIcon icon = d->icon(index);
            if (icon.isNull()) {
               if (d->node(index)->isDir()) {
                  icon = d->fileInfoGatherer.iconProvider()->icon(QFileIconProvider::Folder);
               } else {
                  icon = d->fileInfoGatherer.iconProvider()->icon(QFileIconProvider::File);
               }
            }
            return icon;
         }
         break;

      case Qt::TextAlignmentRole:
         if (index.column() == 1) {
            return Qt::AlignRight;
         }
         break;

      case FilePermissions:
         int p = permissions(index);
         return p;
   }

   return QVariant();
}

QString QFileSystemModelPrivate::size(const QModelIndex &index) const
{
   if (!index.isValid()) {
      return QString();
   }

   const QFileSystemNode *n = node(index);
   if (n->isDir()) {

#ifdef Q_OS_MAC
      return QLatin1String("--");
#else
      return QLatin1String("");
#endif

      // Windows   - ""
      // OS X      - "--"
      // Konqueror - "4 KB"
      // Nautilus  - "9 items" (the number of children)
   }

   return size(n->size());
}

QString QFileSystemModelPrivate::size(qint64 bytes)
{
   // According to the Si standard KB is 1000 bytes, KiB is 1024
   // but on windows sizes are calculated by dividing by 1024 so we do what they do

   const qint64 kb = 1024;
   const qint64 mb = 1024 * kb;
   const qint64 gb = 1024 * mb;
   const qint64 tb = 1024 * gb;

   if (bytes >= tb) {
      return QFileSystemModel::tr("%1 TB").arg(QLocale().toString(qreal(bytes) / tb, 'f', 3));
   }

   if (bytes >= gb) {
      return QFileSystemModel::tr("%1 GB").arg(QLocale().toString(qreal(bytes) / gb, 'f', 2));
   }

   if (bytes >= mb) {
      return QFileSystemModel::tr("%1 MB").arg(QLocale().toString(qreal(bytes) / mb, 'f', 1));
   }

   if (bytes >= kb) {
      return QFileSystemModel::tr("%1 KB").arg(QLocale().toString(bytes / kb));
   }

   return QFileSystemModel::tr("%1 bytes").arg(QLocale().toString(bytes));
}

QString QFileSystemModelPrivate::time(const QModelIndex &index) const
{
   if (! index.isValid()) {
      return QString();
   }

#ifndef QT_NO_DATESTRING
   return node(index)->lastModified().toString(Qt::SystemLocaleDate);
#else

   Q_UNUSED(index);
   return QString();

#endif
}

QString QFileSystemModelPrivate::type(const QModelIndex &index) const
{
   if (! index.isValid()) {
      return QString();
   }
   return node(index)->type();
}

QString QFileSystemModelPrivate::name(const QModelIndex &index) const
{
   if (! index.isValid()) {
      return QString();
   }

   QFileSystemNode *dirNode = node(index);

   if (fileInfoGatherer.resolveSymlinks() && ! resolvedSymLinks.isEmpty() && dirNode->isSymLink(true)) {
      QString fullPath = QDir::fromNativeSeparators(filePath(index));

      if (resolvedSymLinks.contains(fullPath)) {
         return resolvedSymLinks[fullPath];
      }
   }

   return dirNode->fileName;
}

QString QFileSystemModelPrivate::displayName(const QModelIndex &index) const
{
#if defined(Q_OS_WIN)
   QFileSystemNode *dirNode = node(index);

   if (! dirNode->volumeName.isNull()) {
      return dirNode->volumeName + QLatin1String(" (") + name(index) + QLatin1Char(')');
   }
#endif

  return name(index);
}

QIcon QFileSystemModelPrivate::icon(const QModelIndex &index) const
{
   if (!index.isValid()) {
      return QIcon();
   }

   return node(index)->icon();
}

bool QFileSystemModel::setData(const QModelIndex &idx, const QVariant &value, int role)
{
   Q_D(QFileSystemModel);

   if (! idx.isValid() || idx.column() != 0 || role != Qt::EditRole || (flags(idx) & Qt::ItemIsEditable) == 0) {
      return false;
   }

   QString newName = value.toString();
   QString oldName = idx.data().toString();

   if (newName == idx.data().toString()) {
      return true;
   }

   if (newName.isEmpty() || QDir::toNativeSeparators(newName).contains(QDir::separator())
         || ! QDir(filePath(parent(idx))).rename(oldName, newName)) {

#ifndef QT_NO_MESSAGEBOX
      QMessageBox::information(0, QFileSystemModel::tr("Invalid Filename"), QFileSystemModel::tr("<b>\"%1\" is invalid.</b><br>"
         "Use a filename with fewer characters or no punctuation.").arg(newName),QMessageBox::Ok);

#endif
      return false;

   } else {
      /*
          - After re-naming something we do not want the selection to change
          - can not remove rows and later insert
          - can no quickly remove and insert
          - index pointer can not change because treeview does not use persistant index's

          - if this get any more complicated think of changing it to just use layoutChanged
       */

      QFileSystemModelPrivate::QFileSystemNode *indexNode = d->node(idx);
      QFileSystemModelPrivate::QFileSystemNode *parentNode = indexNode->parent;
      int visibleLocation = parentNode->visibleLocation(parentNode->children.value(indexNode->fileName)->fileName);

      d->addNode(parentNode, newName, indexNode->info->fileInfo());
      parentNode->visibleChildren.removeAt(visibleLocation);

      QFileSystemModelPrivate::QFileSystemNode *oldValue = parentNode->children.value(oldName);
      parentNode->children[newName] = oldValue;

      QFileInfo info(d->rootDir, newName);
      oldValue->fileName = newName;
      oldValue->parent = parentNode;
      oldValue->populate(d->fileInfoGatherer.getInfo(info));
      oldValue->isVisible = true;

      parentNode->children.remove(oldName);
      parentNode->visibleChildren.insert(visibleLocation, newName);

      d->delayedSort();
      emit fileRenamed(filePath(idx.parent()), oldName, newName);
   }

   return true;
}

QVariant QFileSystemModel::headerData(int section, Qt::Orientation orientation, int role) const
{
   switch (role) {
      case Qt::DecorationRole:
         if (section == 0) {
            // ### TODO something is still 2 pixels off
            QImage pixmap(16, 1, QImage::Format_Mono);
            pixmap.fill(0);
            pixmap.setAlphaChannel(pixmap.createAlphaMask());
            return pixmap;
         }

         break;

      case Qt::TextAlignmentRole:
         return Qt::AlignLeft;
   }

   if (orientation != Qt::Horizontal || role != Qt::DisplayRole) {
      return QAbstractItemModel::headerData(section, orientation, role);
   }

   QString returnValue;

   switch (section) {
      case 0:
         returnValue = tr("Name");
         break;

      case 1:
         returnValue = tr("Size");
         break;

      case 2:

#ifdef Q_OS_MAC
         returnValue = tr("Kind", "Match OS X Finder");
#else
         returnValue = tr("Type", "All other platforms");
#endif
         break;

      case 3:
         returnValue = tr("Date Modified");
         break;

      default:
         return QVariant();
   }

   return returnValue;
}


Qt::ItemFlags QFileSystemModel::flags(const QModelIndex &index) const
{
   Q_D(const QFileSystemModel);

   Qt::ItemFlags flags = QAbstractItemModel::flags(index);
   if (!index.isValid()) {
      return flags;
   }

   QFileSystemModelPrivate::QFileSystemNode *indexNode = d->node(index);
   if (d->nameFilterDisables && !d->passNameFilters(indexNode)) {
      flags &= ~Qt::ItemIsEnabled;
      // ### TODO you should not be able to set this as the current item, task 119433
      return flags;
   }

   flags |= Qt::ItemIsDragEnabled;
   if (d->readOnly) {
      return flags;
   }
   if ((index.column() == 0) && indexNode->permissions() & QFile::WriteUser) {
      flags |= Qt::ItemIsEditable;
      if (indexNode->isDir()) {
         flags |= Qt::ItemIsDropEnabled;
      }
   }
   return flags;
}

void QFileSystemModelPrivate::_q_performDelayedSort()
{
   Q_Q(QFileSystemModel);
   q->sort(sortColumn, sortOrder);
}

static inline QChar getNextChar(const QString &s, int location)
{
   return (location < s.length()) ? s.at(location) : QChar();
}

/*!
    Natural number sort, skips spaces.

    Examples:
    1, 2, 10, 55, 100
    01.jpg, 2.jpg, 10.jpg

    Note on the algorithm:
    Only as many characters as necessary are looked at and at most they all
    are looked at once.

    Slower then QString::compare() (of course)
  */
int QFileSystemModelPrivate::naturalCompare(const QString &s1, const QString &s2,  Qt::CaseSensitivity cs)
{
   for (int l1 = 0, l2 = 0; l1 <= s1.count() && l2 <= s2.count(); ++l1, ++l2) {
      // skip spaces, tabs and 0's
      QChar c1 = getNextChar(s1, l1);
      while (c1.isSpace()) {
         c1 = getNextChar(s1, ++l1);
      }

      QChar c2 = getNextChar(s2, l2);
      while (c2.isSpace()) {
         c2 = getNextChar(s2, ++l2);
      }

      if (c1.isDigit() && c2.isDigit()) {
         while (c1.digitValue() == 0) {
            c1 = getNextChar(s1, ++l1);
         }
         while (c2.digitValue() == 0) {
            c2 = getNextChar(s2, ++l2);
         }

         int lookAheadLocation1 = l1;
         int lookAheadLocation2 = l2;
         int currentReturnValue = 0;
         // find the last digit, setting currentReturnValue as we go if it isn't equal
         for (
            QChar lookAhead1 = c1, lookAhead2 = c2;
            (lookAheadLocation1 <= s1.length() && lookAheadLocation2 <= s2.length());
            lookAhead1 = getNextChar(s1, ++lookAheadLocation1),
            lookAhead2 = getNextChar(s2, ++lookAheadLocation2)
         ) {
            bool is1ADigit = !lookAhead1.isNull() && lookAhead1.isDigit();
            bool is2ADigit = !lookAhead2.isNull() && lookAhead2.isDigit();
            if (!is1ADigit && !is2ADigit) {
               break;
            }
            if (!is1ADigit) {
               return -1;
            }
            if (!is2ADigit) {
               return 1;
            }
            if (currentReturnValue == 0) {
               if (lookAhead1 < lookAhead2) {
                  currentReturnValue = -1;
               } else if (lookAhead1 > lookAhead2) {
                  currentReturnValue = 1;
               }
            }
         }
         if (currentReturnValue != 0) {
            return currentReturnValue;
         }
      }

      if (cs == Qt::CaseInsensitive) {
         if (!c1.isLower()) {
            c1 = c1.toLower();
         }
         if (!c2.isLower()) {
            c2 = c2.toLower();
         }
      }
      int r = QString::localeAwareCompare(c1, c2);
      if (r < 0) {
         return -1;
      }
      if (r > 0) {
         return 1;
      }
   }
   // The two strings are the same (02 == 2) so fall back to the normal sort
   return QString::compare(s1, s2, cs);
}

/*
    \internal
    Helper functor used by sort()
*/
class QFileSystemModelSorter
{
 public:
   inline QFileSystemModelSorter(int column) : sortColumn(column) {}

   bool compareNodes(const QFileSystemModelPrivate::QFileSystemNode *l,
                     const QFileSystemModelPrivate::QFileSystemNode *r) const {
      switch (sortColumn) {
         case 0: {

#ifndef Q_OS_MAC
            // place directories before files
            bool left = l->isDir();
            bool right = r->isDir();
            if (left ^ right) {
               return left;
            }
#endif
            return QFileSystemModelPrivate::naturalCompare(l->fileName,
                   r->fileName, Qt::CaseInsensitive) < 0;
         }
      case 1: {
            // Directories go first
       bool left = l->isDir();
       bool right = r->isDir();
       if (left ^ right) {
          return left;
       }

       qint64 sizeDifference = l->size() - r->size();
       if (sizeDifference == 0) {
          return QFileSystemModelPrivate::naturalCompare(l->fileName, r->fileName, Qt::CaseInsensitive) < 0;
       }

       return sizeDifference < 0;
      }

         case 2: {
       int compare = QString::localeAwareCompare(l->type(), r->type());
       if (compare == 0) {
          return QFileSystemModelPrivate::naturalCompare(l->fileName, r->fileName, Qt::CaseInsensitive) < 0;
       }

       return compare < 0;
         }

         case 3:
       if (l->lastModified() == r->lastModified()) {
          return QFileSystemModelPrivate::naturalCompare(l->fileName, r->fileName, Qt::CaseInsensitive) < 0;
       }

            return l->lastModified() < r->lastModified();
      }

      Q_ASSERT(false);
      return false;
   }

   bool operator()(const QPair<QFileSystemModelPrivate::QFileSystemNode *, int> &l,
                   const QPair<QFileSystemModelPrivate::QFileSystemNode *, int> &r) const {
      return compareNodes(l.first, r.first);
   }


 private:
   int sortColumn;
};

void QFileSystemModelPrivate::sortChildren(int column, const QModelIndex &parent)
{
   Q_Q(QFileSystemModel);

   QFileSystemModelPrivate::QFileSystemNode *indexNode = node(parent);

   if (indexNode->children.count() == 0) {
      return;
   }

   QList<QPair<QFileSystemModelPrivate::QFileSystemNode *, int> > values;

   int i = 0;
   for (auto item: indexNode->children) {

      if (filtersAcceptsNode(item)) {
         values.append(QPair<QFileSystemModelPrivate::QFileSystemNode *, int>(item, i));

      } else {
         item->isVisible = false;
      }

      i++;
   }

   QFileSystemModelSorter ms(column);
   std::sort(values.begin(), values.end(), ms);

   // update the new visible list
   indexNode->visibleChildren.clear();

   // No more dirty item we reset our internal dirty index
   indexNode->dirtyChildrenIndex = -1;

   for (int i = 0; i < values.count(); ++i) {
      indexNode->visibleChildren.append(values.at(i).first->fileName);
      values.at(i).first->isVisible = true;
   }

   if (! disableRecursiveSort) {
      for (int i = 0; i < q->rowCount(parent); ++i) {
         const QModelIndex childIndex = q->index(i, 0, parent);
         QFileSystemModelPrivate::QFileSystemNode *indexNode = node(childIndex);

         // Only do a recursive sort on visible nodes
         if (indexNode->isVisible) {
            sortChildren(column, childIndex);
         }
      }
   }
}

/*!
    \reimp
*/
void QFileSystemModel::sort(int column, Qt::SortOrder order)
{
   Q_D(QFileSystemModel);
   if (d->sortOrder == order && d->sortColumn == column && !d->forceSort) {
      return;
   }

   emit layoutAboutToBeChanged();
   QModelIndexList oldList = persistentIndexList();
   QList<QPair<QFileSystemModelPrivate::QFileSystemNode *, int> > oldNodes;
   for (int i = 0; i < oldList.count(); ++i) {
      QPair<QFileSystemModelPrivate::QFileSystemNode *, int> pair(d->node(oldList.at(i)), oldList.at(i).column());
      oldNodes.append(pair);
   }

   if (!(d->sortColumn == column && d->sortOrder != order && !d->forceSort)) {
      //we sort only from where we are, don't need to sort all the model
      d->sortChildren(column, index(rootPath()));
      d->sortColumn = column;
      d->forceSort = false;
   }
   d->sortOrder = order;

   QModelIndexList newList;
   for (int i = 0; i < oldNodes.count(); ++i) {
      QModelIndex idx = d->index(oldNodes.at(i).first);
      idx = idx.sibling(idx.row(), oldNodes.at(i).second);
      newList.append(idx);
   }
   changePersistentIndexList(oldList, newList);
   emit layoutChanged();
}

/*!
    Returns a list of MIME types that can be used to describe a list of items
    in the model.
*/
QStringList QFileSystemModel::mimeTypes() const
{
   return QStringList(QLatin1String("text/uri-list"));
}

/*!
    Returns an object that contains a serialized description of the specified
    \a indexes. The format used to describe the items corresponding to the
    indexes is obtained from the mimeTypes() function.

    If the list of indexes is empty, 0 is returned rather than a serialized
    empty list.
*/
QMimeData *QFileSystemModel::mimeData(const QModelIndexList &indexes) const
{
   QList<QUrl> urls;
   QList<QModelIndex>::const_iterator it = indexes.begin();
   for (; it != indexes.end(); ++it)
      if ((*it).column() == 0) {
         urls << QUrl::fromLocalFile(filePath(*it));
      }
   QMimeData *data = new QMimeData();
   data->setUrls(urls);
   return data;
}

/*!
    Handles the \a data supplied by a drag and drop operation that ended with
    the given \a action over the row in the model specified by the \a row and
    \a column and by the \a parent index.

    \sa supportedDropActions()
*/
bool QFileSystemModel::dropMimeData(const QMimeData *data, Qt::DropAction action,
                                    int row, int column, const QModelIndex &parent)
{
   Q_UNUSED(row);
   Q_UNUSED(column);

   if (!parent.isValid() || isReadOnly()) {
      return false;
   }

   bool success = true;
   QString to = filePath(parent) + QDir::separator();

   QList<QUrl> urls = data->urls();
   QList<QUrl>::const_iterator it = urls.constBegin();

   switch (action) {
      case Qt::CopyAction:
         for (; it != urls.constEnd(); ++it) {
            QString path = (*it).toLocalFile();
            success = QFile::copy(path, to + QFileInfo(path).fileName()) && success;
         }
         break;
      case Qt::LinkAction:
         for (; it != urls.constEnd(); ++it) {
            QString path = (*it).toLocalFile();
            success = QFile::link(path, to + QFileInfo(path).fileName()) && success;
         }
         break;
      case Qt::MoveAction:
         for (; it != urls.constEnd(); ++it) {
            QString path = (*it).toLocalFile();
            success = QFile::rename(path, to + QFileInfo(path).fileName()) && success;
         }
         break;
      default:
         return false;
   }

   return success;
}

Qt::DropActions QFileSystemModel::supportedDropActions() const
{
   return Qt::CopyAction | Qt::MoveAction | Qt::LinkAction;
}

QString QFileSystemModel::filePath(const QModelIndex &index) const
{
   Q_D(const QFileSystemModel);

   QString fullPath = d->filePath(index);
   QFileSystemModelPrivate::QFileSystemNode *dirNode = d->node(index);

   if (dirNode->isSymLink() && d->fileInfoGatherer.resolveSymlinks() && d->resolvedSymLinks.contains(fullPath)
         && dirNode->isDir()) {

      QFileInfo resolvedInfo(fullPath);
      resolvedInfo = resolvedInfo.canonicalFilePath();

      if (resolvedInfo.exists()) {
         return resolvedInfo.filePath();
      }
   }
   return fullPath;
}

QString QFileSystemModelPrivate::filePath(const QModelIndex &index) const
{
   Q_Q(const QFileSystemModel);
   Q_UNUSED(q);

   if (!index.isValid()) {
      return QString();
   }

   Q_ASSERT(index.model() == q);

   QStringList path;
   QModelIndex idx = index;
   while (idx.isValid()) {
      QFileSystemModelPrivate::QFileSystemNode *dirNode = node(idx);
      if (dirNode) {
         path.prepend(dirNode->fileName);
      }
      idx = idx.parent();
   }

   QString fullPath = QDir::fromNativeSeparators(path.join(QDir::separator()));

#if ! defined(Q_OS_WIN)
   if ((fullPath.length() > 2) && fullPath[0] == QLatin1Char('/') && fullPath[1] == QLatin1Char('/')) {
      fullPath = fullPath.mid(1);
   }
#endif
#if defined(Q_OS_WIN)
   if (fullPath.length() == 2 && fullPath.endsWith(QLatin1Char(':'))) {
      fullPath.append(QLatin1Char('/'));
   }
#endif
   return fullPath;
}

/*!
    Create a directory with the \a name in the \a parent model index.
*/
QModelIndex QFileSystemModel::mkdir(const QModelIndex &parent, const QString &name)
{
   Q_D(QFileSystemModel);
   if (!parent.isValid()) {
      return parent;
   }

   QDir dir(filePath(parent));
   if (!dir.mkdir(name)) {
      return QModelIndex();
   }

   QFileSystemModelPrivate::QFileSystemNode *parentNode = d->node(parent);
   d->addNode(parentNode, name, QFileInfo());

   Q_ASSERT(parentNode->children.contains(name));

   QFileSystemModelPrivate::QFileSystemNode *node = parentNode->children[name];
   node->populate(d->fileInfoGatherer.getInfo(QFileInfo(dir.absolutePath() + QDir::separator() + name)));
   d->addVisibleFiles(parentNode, QStringList(name));
   return d->index(node);
}

/*!
    Returns the complete OR-ed together combination of QFile::Permission for the \a index.
 */
QFile::Permissions QFileSystemModel::permissions(const QModelIndex &index) const
{
   Q_D(const QFileSystemModel);

   QFile::Permissions p = d->node(index)->permissions();

   if (d->readOnly) {
      p ^= (QFile::WriteOwner | QFile::WriteUser | QFile::WriteGroup | QFile::WriteOther);
   }
   return p;
}

QModelIndex QFileSystemModel::setRootPath(const QString &newPath)
{
   Q_D(QFileSystemModel);

#if defined(Q_OS_WIN32)
   QString longNewPath = qt_GetLongPathName(newPath);

#elif defined(Q_OS_WIN)
   QString longNewPath = QDir::fromNativeSeparators(newPath);

#else
   QString longNewPath = newPath;

#endif

   QDir newPathDir(longNewPath);

   // remove .. and . from the given path if exist
   if (! newPath.isEmpty()) {
      longNewPath = QDir::cleanPath(longNewPath);
      newPathDir.setPath(longNewPath);
   }

   d->setRootPath = true;

   // newPath was not "" however longPath is ""
   if (! newPath.isEmpty() && longNewPath.isEmpty()) {
      return d->index(rootPath());
   }

   if (d->rootDir.path() == longNewPath) {
      return d->index(rootPath());
   }

   bool showDrives = (longNewPath.isEmpty() || longNewPath == d->myComputer());
   if (! showDrives && ! newPathDir.exists()) {
      return d->index(rootPath());
   }

   if (! rootPath().isEmpty() && rootPath() != QLatin1String(".")) {
      // remove the watcher for the old rootPath
      d->fileInfoGatherer.removePath(rootPath());

      // "marks" the node as dirty, so the next fetchMore call ask the gatherer to install a watcher again
      // this does not re-fetch everything (?)
      d->node(rootPath())->populatedChildren = false;
   }

   // new valid root path
   d->rootDir = newPathDir;
   QModelIndex newRootIndex;

   if (showDrives) {
      // otherwise dir will become '.'
      d->rootDir.setPath(QLatin1String(""));

   } else {
      newRootIndex = d->index(newPathDir.path());
   }

   fetchMore(newRootIndex);
   emit rootPathChanged(longNewPath);

   d->forceSort = true;
   d->delayedSort();

   return newRootIndex;
}

QString QFileSystemModel::rootPath() const
{
   Q_D(const QFileSystemModel);
   return d->rootDir.path();
}

QDir QFileSystemModel::rootDirectory() const
{
   Q_D(const QFileSystemModel);

   QDir dir(d->rootDir);
   dir.setNameFilters(nameFilters());
   dir.setFilter(filter());

   return dir;
}

void QFileSystemModel::setIconProvider(QFileIconProvider *provider)
{
   Q_D(QFileSystemModel);
   d->fileInfoGatherer.setIconProvider(provider);
   d->root.updateIcon(provider, QString());
}

QFileIconProvider *QFileSystemModel::iconProvider() const
{
   Q_D(const QFileSystemModel);
   return d->fileInfoGatherer.iconProvider();
}

void QFileSystemModel::setFilter(QDir::Filters filters)
{
   Q_D(QFileSystemModel);

   if (d->filters == filters) {
      return;
   }

   d->filters = filters;

   // CaseSensitivity might have changed
   setNameFilters(nameFilters());
   d->forceSort = true;
   d->delayedSort();
}


QDir::Filters QFileSystemModel::filter() const
{
   Q_D(const QFileSystemModel);
   return d->filters;
}

void QFileSystemModel::setResolveSymlinks(bool enable)
{
   Q_D(QFileSystemModel);
   d->fileInfoGatherer.setResolveSymlinks(enable);
}

bool QFileSystemModel::resolveSymlinks() const
{
   Q_D(const QFileSystemModel);
   return d->fileInfoGatherer.resolveSymlinks();
}

void QFileSystemModel::setReadOnly(bool enable)
{
   Q_D(QFileSystemModel);
   d->readOnly = enable;
}

bool QFileSystemModel::isReadOnly() const
{
   Q_D(const QFileSystemModel);
   return d->readOnly;
}

void QFileSystemModel::setNameFilterDisables(bool enable)
{
   Q_D(QFileSystemModel);

   if (d->nameFilterDisables == enable) {
      return;
   }

   d->nameFilterDisables = enable;
   d->forceSort = true;
   d->delayedSort();
}

bool QFileSystemModel::nameFilterDisables() const
{
   Q_D(const QFileSystemModel);
   return d->nameFilterDisables;
}


void QFileSystemModel::setNameFilters(const QStringList &filters)
{
   // Prep the regexp's ahead of time

#ifndef QT_NO_REGEXP
   Q_D(QFileSystemModel);

   if (!d->bypassFilters.isEmpty()) {
      // update the bypass filter to only bypass the stuff that must be kept around
      d->bypassFilters.clear();

      // We guarantee that rootPath will stick around
      QPersistentModelIndex root(index(rootPath()));
      QModelIndexList persistantList = persistentIndexList();
      for (int i = 0; i < persistantList.count(); ++i) {
         QFileSystemModelPrivate::QFileSystemNode *node;
         node = d->node(persistantList.at(i));
         while (node) {
            if (d->bypassFilters.contains(node)) {
               break;
            }
            if (node->isDir()) {
               d->bypassFilters[node] = true;
            }
            node = node->parent;
         }
      }
   }

   d->nameFilters.clear();
   const Qt::CaseSensitivity caseSensitive =
      (filter() & QDir::CaseSensitive) ? Qt::CaseSensitive : Qt::CaseInsensitive;
   for (int i = 0; i < filters.size(); ++i) {
      d->nameFilters << QRegExp(filters.at(i), caseSensitive, QRegExp::Wildcard);
   }
   d->forceSort = true;
   d->delayedSort();
#endif
}

/*!
    Returns a list of filters applied to the names in the model.
*/
QStringList QFileSystemModel::nameFilters() const
{
   Q_D(const QFileSystemModel);
   QStringList filters;
#ifndef QT_NO_REGEXP
   for (int i = 0; i < d->nameFilters.size(); ++i) {
      filters << d->nameFilters.at(i).pattern();
   }
#endif
   return filters;
}

/*!
    \reimp
*/
bool QFileSystemModel::event(QEvent *event)
{
   Q_D(QFileSystemModel);

   if (event->type() == QEvent::LanguageChange) {
      d->root.retranslateStrings(d->fileInfoGatherer.iconProvider(), QString());
      return true;
   }

   return QAbstractItemModel::event(event);
}

bool QFileSystemModel::rmdir(const QModelIndex &index)
{
   QString path = filePath(index);
   const bool retval = QDir().rmdir(path);

   if (retval) {
      QFileSystemModelPrivate *d = d_func();
      d->fileInfoGatherer.removePath(path);
   }

   return retval;
}

void QFileSystemModelPrivate::_q_directoryChanged(const QString &directory, const QStringList &files)
{
   QFileSystemModelPrivate::QFileSystemNode *parentNode = node(directory, false);

   if (parentNode->children.count() == 0) {
      return;
   }

   QStringList toRemove;
   QStringList newFiles = files;

   std::sort(newFiles.begin(), newFiles.end());

   for (auto childElement : parentNode->children) {
      QStringList::iterator iterator;

      iterator = std::lower_bound(newFiles.begin(), newFiles.end(), childElement->fileName);

      if (iterator == newFiles.end() || (childElement->fileName < *iterator)) {
         toRemove.append(childElement->fileName);
      }
   }

   for (int i = 0 ; i < toRemove.count() ; ++i ) {
      removeNode(parentNode, toRemove[i]);
   }
}

QFileSystemModelPrivate::QFileSystemNode *QFileSystemModelPrivate::addNode(QFileSystemNode *parentNode,
      const QString &fileName, const QFileInfo &info)
{
   // In the common case, itemLocation == count() so check there first
   QFileSystemModelPrivate::QFileSystemNode *node = new QFileSystemModelPrivate::QFileSystemNode(fileName, parentNode);

#ifndef QT_NO_FILESYSTEMWATCHER
   node->populate(info);
#endif

#if defined(Q_OS_WIN)
   //The parentNode is "" so we are listing the drives

   if (parentNode->fileName.isEmpty()) {
      wchar_t name[MAX_PATH + 1];

      //GetVolumeInformation requires to add trailing backslash
      const QString nodeName = fileName + QLatin1String("\\");
      BOOL success = ::GetVolumeInformation((wchar_t *)(nodeName.utf16()),
                                            name, MAX_PATH + 1, NULL, 0, NULL, NULL, 0);

      if (success && name[0]) {
         node->volumeName = QString::fromWCharArray(name);
      }
   }
#endif

   parentNode->children.insert(fileName, node);
   return node;
}

void QFileSystemModelPrivate::removeNode(QFileSystemModelPrivate::QFileSystemNode *parentNode, const QString &name)
{
   Q_Q(QFileSystemModel);

   QModelIndex parent = index(parentNode);
   bool indexHidden = isHiddenByFilter(parentNode, parent);

   int vLocation = parentNode->visibleLocation(name);

   if (vLocation >= 0 && !indexHidden)
      q->beginRemoveRows(parent, translateVisibleLocation(parentNode, vLocation),
                         translateVisibleLocation(parentNode, vLocation));

   QFileSystemNode *node = parentNode->children.take(name);
   delete node;

   // cleanup sort files after removing rather then re-sorting which is O(n)
   if (vLocation >= 0) {
      parentNode->visibleChildren.removeAt(vLocation);
   }

   if (vLocation >= 0 && !indexHidden) {
      q->endRemoveRows();
   }
}

/*
    \internal
    Helper functor used by addVisibleFiles()
*/
class QFileSystemModelVisibleFinder
{
 public:
   inline QFileSystemModelVisibleFinder(QFileSystemModelPrivate::QFileSystemNode *node,
                                        QFileSystemModelSorter *sorter) : parentNode(node), sorter(sorter) {}

   bool operator()(const QString &, QString r) const {
      return sorter->compareNodes(parentNode->children.value(name), parentNode->children.value(r));
   }

   QString name;

 private:
   QFileSystemModelPrivate::QFileSystemNode *parentNode;
   QFileSystemModelSorter *sorter;
};

void QFileSystemModelPrivate::addVisibleFiles(QFileSystemNode *parentNode, const QStringList &newFiles)
{
   Q_Q(QFileSystemModel);

   QModelIndex parent = index(parentNode);
   bool indexHidden   = isHiddenByFilter(parentNode, parent);

   if (! indexHidden) {
      q->beginInsertRows(parent, parentNode->visibleChildren.count() ,
                         parentNode->visibleChildren.count() + newFiles.count() - 1);
   }

   if (parentNode->dirtyChildrenIndex == -1) {
      parentNode->dirtyChildrenIndex = parentNode->visibleChildren.count();
   }

   for (int i = 0; i < newFiles.count(); ++i) {
      parentNode->visibleChildren.append(newFiles.at(i));
      parentNode->children[newFiles.at(i)]->isVisible = true;
   }

   if (! indexHidden) {
      q->endInsertRows();
   }
}

void QFileSystemModelPrivate::removeVisibleFile(QFileSystemNode *parentNode, int vLocation)
{
   Q_Q(QFileSystemModel);

   if (vLocation == -1) {
      return;
   }

   QModelIndex parent = index(parentNode);
   bool indexHidden   = isHiddenByFilter(parentNode, parent);

   if (! indexHidden)
      q->beginRemoveRows(parent, translateVisibleLocation(parentNode, vLocation),
                         translateVisibleLocation(parentNode, vLocation));

   parentNode->children[parentNode->visibleChildren.at(vLocation)]->isVisible = false;
   parentNode->visibleChildren.removeAt(vLocation);

   if (!indexHidden) {
      q->endRemoveRows();
   }
}

void QFileSystemModelPrivate::_q_fileSystemChanged(const QString &path, const QList<QPair<QString, QFileInfo> > &updates)
{
   Q_Q(QFileSystemModel);

   QVector<QString> rowsToUpdate;
   QStringList newFiles;

   QFileSystemModelPrivate::QFileSystemNode *parentNode = node(path, false);
   QModelIndex parentIndex = index(parentNode);

   for (int i = 0; i < updates.count(); ++i) {
      QString fileName = updates.at(i).first;
      Q_ASSERT(! fileName.isEmpty());

      QExtendedInformation info = fileInfoGatherer.getInfo(updates.at(i).second);
      bool previouslyHere = parentNode->children.contains(fileName);

      if (! previouslyHere) {
         addNode(parentNode, fileName, info.fileInfo());
      }

      QFileSystemModelPrivate::QFileSystemNode *node = parentNode->children.value(fileName);
      bool isCaseSensitive = parentNode->caseSensitive();

      if (isCaseSensitive) {
         if (node->fileName != fileName) {
            continue;
         }

      } else if (QString::compare(node->fileName, fileName, Qt::CaseInsensitive) != 0) {
         continue;

      }

      if (isCaseSensitive) {
         Q_ASSERT(node->fileName == fileName);

      } else {
         node->fileName = fileName;

      }

      if (info.size() == -1 && ! info.isSymLink()) {
         removeNode(parentNode, fileName);
         continue;
      }

      if (*node != info ) {
         node->populate(info);
         bypassFilters.remove(node);

         // brand new information
         if (filtersAcceptsNode(node)) {

            if (! node->isVisible) {
               newFiles.append(fileName);

            } else {
               rowsToUpdate.append(fileName);
            }

         } else {
            if (node->isVisible) {
               int visibleLocation = parentNode->visibleLocation(fileName);
               removeVisibleFile(parentNode, visibleLocation);
            } else {
               // The file is not visible, don't do anything
            }
         }
      }
   }

   // bundle up all of the changed signals into as few as possible
   std::sort(rowsToUpdate.begin(), rowsToUpdate.end());

   QString min;
   QString max;

   for (int i = 0; i < rowsToUpdate.count(); ++i) {
      QString value = rowsToUpdate.at(i);

      //##TODO is there a way to bundle signals with QString as the content of the list?
      /*if (min.isEmpty()) {
          min = value;
          if (i != rowsToUpdate.count() - 1)
              continue;
      }
      if (i != rowsToUpdate.count() - 1) {
          if ((value == min + 1 && max.isEmpty()) || value == max + 1) {
              max = value;
              continue;
          }
      }*/
      max = value;
      min = value;

      int visibleMin = parentNode->visibleLocation(min);
      int visibleMax = parentNode->visibleLocation(max);

      if (visibleMin >= 0 && visibleMin < parentNode->visibleChildren.count()
            && parentNode->visibleChildren.at(visibleMin) == min && visibleMax >= 0) {

         QModelIndex bottom = q->index(translateVisibleLocation(parentNode, visibleMin), 0, parentIndex);
         QModelIndex top = q->index(translateVisibleLocation(parentNode, visibleMax), 3, parentIndex);
         emit q->dataChanged(bottom, top);
      }

      /*min = QString();
      max = QString();*/
   }

   if (newFiles.count() > 0) {
      addVisibleFiles(parentNode, newFiles);
   }

   if (newFiles.count() > 0 || (sortColumn != 0 && rowsToUpdate.count() > 0)) {
      forceSort = true;
      delayedSort();
   }
}

void QFileSystemModelPrivate::_q_resolvedName(const QString &fileName, const QString &resolvedName)
{
   resolvedSymLinks[fileName] = resolvedName;
}

void QFileSystemModelPrivate::init()
{
   Q_Q(QFileSystemModel);

   qRegisterMetaType<QList<QPair<QString, QFileInfo> > >("QList<QPair<QString,QFileInfo> >");

   q->connect(&fileInfoGatherer, SIGNAL(newListOfFiles(const QString &, const QStringList &)),
              q, SLOT(_q_directoryChanged(const QString &, const QStringList &)));

   q->connect(&fileInfoGatherer, SIGNAL(updates(const QString &, const QList<QPair<QString, QFileInfo> > &)),
              q, SLOT(_q_fileSystemChanged(const QString &, const QList<QPair<QString, QFileInfo> > &)));

   q->connect(&fileInfoGatherer, SIGNAL(nameResolved(const QString &, const QString &)),
              q, SLOT(_q_resolvedName(const QString &, const QString &)));

   q->connect(&fileInfoGatherer, SIGNAL(directoryLoaded(const QString &)), q, SLOT(directoryLoaded(const QString &)));
   q->connect(&delayedSortTimer, SIGNAL(timeout()), q, SLOT(_q_performDelayedSort()), Qt::QueuedConnection);

   QHash<int, QByteArray> roles = q->roleNames();
   roles.insert(QFileSystemModel::FileIconRole, "fileIcon");       // == Qt::decoration

   roles.insert(QFileSystemModel::FilePathRole,      "filePath");
   roles.insert(QFileSystemModel::FileNameRole,      "fileName");
   roles.insert(QFileSystemModel::FilePermissions,   "filePermissions");

   q->setRoleNames(roles);
}

bool QFileSystemModelPrivate::filtersAcceptsNode(const QFileSystemNode *node) const
{
   // always accept drives
   if (node->parent == &root || bypassFilters.contains(node)) {
      return true;
   }

   // we do not know anything, so do not accept this node
   if (! node->hasInformation()) {
      return false;
   }

   const bool filterPermissions = ( (filters & QDir::PermissionMask) &&
                                    (filters & QDir::PermissionMask) != QDir::PermissionMask);

   const bool hideHidden        = ! (filters & QDir::Hidden);

   const bool hideDirs          = ! (filters & (QDir::Dirs | QDir::AllDirs));
   const bool hideFiles         = ! (filters & QDir::Files);
   const bool hideSystem        = ! (filters & QDir::System);

   const bool hideReadable      = ! (! filterPermissions || (filters & QDir::Readable));
   const bool hideWritable      = ! (! filterPermissions || (filters & QDir::Writable));
   const bool hideExecutable    = ! (! filterPermissions || (filters & QDir::Executable));

   const bool hideSymlinks      = (filters & QDir::NoSymLinks);

   // ### Qt5 simplify (because NoDotAndDotDot=NoDot|NoDotDot)
   const bool hideDot           = (filters & QDir::NoDot) || (filters & QDir::NoDotAndDotDot);

   // ### Qt5 simplify (because NoDotAndDotDot=NoDot|NoDotDot)
   const bool hideDotDot        = (filters & QDir::NoDotDot) || (filters & QDir::NoDotAndDotDot);

   bool isDot    = (node->fileName == QLatin1String("."));
   bool isDotDot = (node->fileName == QLatin1String(".."));

   if ( ( hideHidden && ! (isDot || isDotDot) && node->isHidden() )
          || (hideDirs  && node->isDir())
          || (hideFiles && node->isFile())
          || (hideSystem  && node->isSystem())
          || (hideReadable  && node->isReadable())
          || (hideWritable   && node->isWritable())
          || (hideExecutable && node->isExecutable())
          || (hideSymlinks  && node->isSymLink())
          || (hideDot && isDot)
          || (hideDotDot && isDotDot)) {

      return false;
   }

   return nameFilterDisables || passNameFilters(node);
}

bool QFileSystemModelPrivate::passNameFilters(const QFileSystemNode *node) const
{
#ifndef QT_NO_REGEXP
   if (nameFilters.isEmpty()) {
      return true;
   }

   // Check the name regularexpression filters
   if (! (node->isDir() && (filters & QDir::AllDirs))) {
      for (int i = 0; i < nameFilters.size(); ++i) {
         if (nameFilters.at(i).exactMatch(node->fileName)) {
            return true;
         }
      }
      return false;
   }
#endif
   return true;
}

void QFileSystemModel::_q_directoryChanged(const QString &directory, const QStringList &list)
{
   Q_D(QFileSystemModel);
   d->_q_directoryChanged(directory, list);
}

void QFileSystemModel::_q_performDelayedSort()
{
   Q_D(QFileSystemModel);
   d->_q_performDelayedSort();
}

void QFileSystemModel::_q_fileSystemChanged(const QString &path, const QList <QPair<QString, QFileInfo>> &un_named_arg2)
{
   Q_D(QFileSystemModel);
   d->_q_fileSystemChanged(path, un_named_arg2);
}

void QFileSystemModel::_q_resolvedName(const QString &fileName, const QString &resolvedName)
{
   Q_D(QFileSystemModel);
   d->_q_resolvedName(fileName, resolvedName);
}

QT_END_NAMESPACE

#endif // QT_NO_FILESYSTEMMODEL
