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

#include <qfilesystemmodel_p.h>

#include <qapplication.h>
#include <qdebug.h>
#include <qfilesystemmodel.h>
#include <qlocale.h>
#include <qmessagebox.h>
#include <qmimedata.h>
#include <qurl.h>

#include <algorithm>

#ifdef Q_OS_WIN
#include <qt_windows.h>
#include <qvarlengtharray.h>
#endif

#ifndef QT_NO_FILESYSTEMMODEL

QFileInfo QFileSystemModel::fileInfo(const QModelIndex &index) const
{
   Q_D(const QFileSystemModel);
   return d->node(index)->fileInfo();
}

bool QFileSystemModel::remove(const QModelIndex &index) const
{
   bool retval;

   QString path = filePath(index);

   if (QFileInfo(path).isFile()) {
      retval = QFile::remove(path);

   } else {
      retval = QDir(path).removeRecursively();
   }

#ifndef QT_NO_FILESYSTEMWATCHER
   if (retval) {
      QFileSystemModelPrivate *d = const_cast<QFileSystemModelPrivate *>(d_func());
      d->fileInfoGatherer.removePath(path);
   }
#endif

   return retval;
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

   // get the internal pointer for the index
   const QString &childName = parentNode->visibleChildren[d->translateVisibleLocation(parentNode, row)];
   const QFileSystemModelPrivate::QFileSystemNode *indexNode = parentNode->children.value(childName);

   Q_ASSERT(indexNode);

   return createIndex(row, column, const_cast<QFileSystemModelPrivate::QFileSystemNode *>(indexNode));
}

QModelIndex QFileSystemModel::index(const QString &path, int column) const
{
   Q_D(const QFileSystemModel);

   QFileSystemModelPrivate::QFileSystemNode *node = d->node(path, false);

   return d->index(node, column);
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

#ifdef Q_OS_WIN

static QString qt_GetLongPathName(const QString &strShortPath)
{
   if (strShortPath.isEmpty() || strShortPath == "." || strShortPath == "..") {
      return strShortPath;
   }

   if (strShortPath.length() == 2 && strShortPath.endsWith(':')) {
      return strShortPath.toUpper();
   }

   const QString absPath = QDir(strShortPath).absolutePath();

   if (absPath.startsWith("//") || absPath.startsWith("\\\\")) {
      // unc
      return QDir::fromNativeSeparators(absPath);
   }

   if (absPath.startsWith(QChar('/'))) {
      return QString();
   }

   const QString inputString = "\\\\?\\" + QDir::toNativeSeparators(absPath);
   std::wstring buffer(MAX_PATH, L'\0');

   DWORD result = ::GetLongPathName(&inputString.toStdWString()[0], &buffer[0], buffer.size());

   if (result > DWORD(buffer.size())) {
      buffer.resize(result);
      result = ::GetLongPathName(&inputString.toStdWString()[0], &buffer[0], buffer.size());
   }

   if (result > 4) {
      buffer.erase(0, 4);
      QString longPath = QString::fromStdWString(buffer);

      // ignoring prefix
      longPath.replace(0, 1, longPath.at(0).toUpper());

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

   if (path.isEmpty() || path == myComputer() || path.startsWith(':')) {
      return const_cast<QFileSystemModelPrivate::QFileSystemNode *>(&root);
   }

   // Construct the nodes up to the new root path if they need to be built
   QString absolutePath;

#ifdef Q_OS_WIN
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
   QStringList pathElements = absolutePath.split(QChar('/'), QStringParser::SkipEmptyParts);

#if defined(Q_OS_WIN)
   if ((pathElements.isEmpty())) {

#else
   if ((pathElements.isEmpty()) && QDir::fromNativeSeparators(longPath) != QString("/")) {

#endif

      return const_cast<QFileSystemModelPrivate::QFileSystemNode *>(&root);
   }

   QModelIndex index = QModelIndex(); // start with "My Computer"

   QString elementPath;
   QChar separator = '/';
   QString trailingSeparator;

#if defined(Q_OS_WIN)
   if (absolutePath.startsWith("//")) {

      // UNC path
      QString host = QString("\\\\") + pathElements.first();

      if (absolutePath == QDir::fromNativeSeparators(host)) {
         absolutePath.append(QChar('/'));
      }

      if (longPath.endsWith('/') && ! absolutePath.endsWith('/')) {
         absolutePath.append('/');
      }

      if (absolutePath.endsWith('/')) {
         trailingSeparator = "\\";
      }

      int r = 0;
      QFileSystemModelPrivate::QFileSystemNode *rootNode = const_cast<QFileSystemModelPrivate::QFileSystemNode *>(&root);

      if (! root.children.contains(host.toLower())) {
         if (pathElements.count() == 1 && ! absolutePath.endsWith('/')) {
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

      separator = QChar('\\');
      elementPath = host;
      elementPath.append(separator);

   } else {

      if (! pathElements.at(0).contains(":")) {
         QString rootPath = QDir(longPath).rootPath();
         pathElements.prepend(rootPath);
      }

      if (pathElements.at(0).endsWith(QChar('/'))) {
         pathElements[0].chop(1);
      }
   }

#else
   // add the "/" item, since it is a valid path element on Unix
   if (absolutePath[0] == QChar('/')) {
      pathElements.prepend(QString("/"));
   }
#endif

   QFileSystemModelPrivate::QFileSystemNode *parent = node(index);

   for (int i = 0; i < pathElements.count(); ++i) {

      QString element = pathElements.at(i);

      if (i != 0) {
         elementPath.append(separator);
      }

      elementPath.append(element);
      if (i == pathElements.count() - 1) {
         elementPath.append(trailingSeparator);
      }

#ifdef Q_OS_WIN
      while (element.endsWith(QChar('.')) || element.endsWith(QChar(' '))) {
         element.chop(1);
      }

      if (element.isEmpty()) {
         return parent;
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

         QFileInfo info(elementPath);

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

   if (! index.isValid()) {
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
   Q_ASSERT(indexNode != nullptr);

   QFileSystemModelPrivate::QFileSystemNode *parentNode = (indexNode ? indexNode->parent : nullptr);
   if (parentNode == nullptr || parentNode == &d->root) {
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

QModelIndex QFileSystemModelPrivate::index(const QFileSystemModelPrivate::QFileSystemNode *node, int column) const
{
   Q_Q(const QFileSystemModel);

   QFileSystemModelPrivate::QFileSystemNode *parentNode = (node ? node->parent : nullptr);

   if (node == &root || !parentNode) {
      return QModelIndex();
   }

   // get the parent's row
   Q_ASSERT(node);
   if (!node->isVisible) {
      return QModelIndex();
   }

   int visualRow = translateVisibleLocation(parentNode, parentNode->visibleLocation(node->fileName));

   return q->createIndex(visualRow, column, const_cast<QFileSystemNode *>(node));
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

#ifndef QT_NO_FILESYSTEMWATCHER
   d->fileInfoGatherer.list(filePath(parent));
#endif
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
   return (parent.column() > 0) ? 0 : QFileSystemModelPrivate::NumColumns;
}

QVariant QFileSystemModel::myComputer(int role) const
{
#ifndef QT_NO_FILESYSTEMWATCHER
   Q_D(const QFileSystemModel);
#endif

   switch (role) {
      case Qt::DisplayRole:
         return QFileSystemModelPrivate::myComputer();

#ifndef QT_NO_FILESYSTEMWATCHER
      case Qt::DecorationRole:
         return d->fileInfoGatherer.iconProvider()->icon(QFileIconProvider::Computer);
#endif
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
               qWarning("QFileSystemModel::data() Invalid display role for column %d", index.column());
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

#ifndef QT_NO_FILESYSTEMWATCHER
            if (icon.isNull()) {
               if (d->node(index)->isDir()) {
                  icon = d->fileInfoGatherer.iconProvider()->icon(QFileIconProvider::Folder);
               } else {
                  icon = d->fileInfoGatherer.iconProvider()->icon(QFileIconProvider::File);
               }
            }
#endif
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
   if (! index.isValid()) {
      return QString();
   }

   const QFileSystemNode *n = node(index);
   if (n->isDir()) {

#ifdef Q_OS_DARWIN
      return QString("--");
#else
      return QString("");
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
      return QFileSystemModel::tr("%1 TB").formatArg(QLocale().toString(qreal(bytes) / tb, 'f', 3));
   }

   if (bytes >= gb) {
      return QFileSystemModel::tr("%1 GB").formatArg(QLocale().toString(qreal(bytes) / gb, 'f', 2));
   }

   if (bytes >= mb) {
      return QFileSystemModel::tr("%1 MB").formatArg(QLocale().toString(qreal(bytes) / mb, 'f', 1));
   }

   if (bytes >= kb) {
      return QFileSystemModel::tr("%1 KB").formatArg(QLocale().toString(bytes / kb));
   }

   return QFileSystemModel::tr("%1 bytes").formatArg(QLocale().toString(bytes));
}

QString QFileSystemModelPrivate::time(const QModelIndex &index) const
{
   if (! index.isValid()) {
      return QString();
   }

   return node(index)->lastModified().toString(Qt::SystemLocaleDate);
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


#ifndef QT_NO_FILESYSTEMWATCHER
   if (fileInfoGatherer.resolveSymlinks() && ! resolvedSymLinks.isEmpty() && dirNode->isSymLink(true)) {

#else
   if (! resolvedSymLinks.isEmpty() && dirNode->isSymLink(true)) {

#endif

      QString fullPath = QDir::fromNativeSeparators(filePath(index));
      return resolvedSymLinks.value(fullPath, dirNode->fileName);
   }

   return dirNode->fileName;
}

QString QFileSystemModelPrivate::displayName(const QModelIndex &index) const
{
#if defined(Q_OS_WIN)
   QFileSystemNode *dirNode = node(index);

   if (! dirNode->volumeName.isEmpty()) {
      return dirNode->volumeName + " (" + name(index) + ')';
   }
#endif

   return name(index);
}

QIcon QFileSystemModelPrivate::icon(const QModelIndex &index) const
{
   if (! index.isValid()) {
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

   const QString parentPath = filePath(parent(idx));

   if (newName.isEmpty() || QDir::toNativeSeparators(newName).contains(QDir::separator())
      || ! QDir(filePath(parent(idx))).rename(oldName, newName)) {

#ifndef QT_NO_MESSAGEBOX
      QMessageBox::information(nullptr, QFileSystemModel::tr("Invalid Filename"), QFileSystemModel::tr("<b>\"%1\" is invalid.</b><br>"
            "Use a filename with fewer characters or no punctuation.").formatArg(newName), QMessageBox::Ok);

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

      parentNode->visibleChildren.removeAt(visibleLocation);

      QFileSystemModelPrivate::QFileSystemNode *oldValue = parentNode->children.value(oldName);
      parentNode->children[newName] = oldValue;

      oldValue->fileName = newName;
      oldValue->parent = parentNode;

#ifndef QT_NO_FILESYSTEMWATCHER
      oldValue->populate(d->fileInfoGatherer.getInfo(QFileInfo(parentPath, newName)));
#endif
      oldValue->isVisible = true;

      parentNode->children.remove(oldName);
      parentNode->visibleChildren.insert(visibleLocation, newName);

      d->delayedSort();
      emit fileRenamed(parentPath, oldName, newName);
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

#ifdef Q_OS_DARWIN
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
      } else {
         flags |= Qt::ItemNeverHasChildren;
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
         for (QChar lookAhead1 = c1, lookAhead2 = c2;
            (lookAheadLocation1 <= s1.length() && lookAheadLocation2 <= s2.length());
            lookAhead1 = getNextChar(s1, ++lookAheadLocation1),
            lookAhead2 = getNextChar(s2, ++lookAheadLocation2)) {

            bool is1ADigit = !lookAhead1.isNull() && lookAhead1.isDigit();
            bool is2ADigit = !lookAhead2.isNull() && lookAhead2.isDigit();

            if (!is1ADigit && !is2ADigit) {
               break;
            }

            if (! is1ADigit) {
               return -1;
            }

            if (! is2ADigit) {
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
         if (! c1.isLower()) {
            c1 = c1.toLower()[0];
         }

         if (! c2.isLower()) {
            c2 = c2.toLower()[0];
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

class QFileSystemModelSorter
{
 public:
   inline QFileSystemModelSorter(int column) : sortColumn(column) { }

   bool compareNodes(const QFileSystemModelPrivate::QFileSystemNode *l,
      const QFileSystemModelPrivate::QFileSystemNode *r) const {
      switch (sortColumn) {
         case 0: {

#ifndef Q_OS_DARWIN
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

         case 3:   {
            if (l->lastModified() == r->lastModified()) {
               return QFileSystemModelPrivate::naturalCompare(l->fileName, r->fileName, Qt::CaseInsensitive) < 0;
            }

            return l->lastModified() < r->lastModified();
         }

      }

      Q_ASSERT(false);
      return false;
   }

   bool operator()(const QFileSystemModelPrivate::QFileSystemNode *l,
      const QFileSystemModelPrivate::QFileSystemNode *r) const {
      return compareNodes(l, r);
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

   QVector<QFileSystemModelPrivate::QFileSystemNode *> values;


   for (auto item : indexNode->children) {

      if (filtersAcceptsNode(item)) {
         values.append(item);

      } else {
         item->isVisible = false;
      }
   }

   QFileSystemModelSorter ms(column);
   std::sort(values.begin(), values.end(), ms);

   // update the new visible list
   indexNode->visibleChildren.clear();

   // No more dirty item we reset our internal dirty index
   indexNode->dirtyChildrenIndex = -1;

   for (int i = 0; i < values.count(); ++i) {
      indexNode->visibleChildren.append(values.at(i)->fileName);
      values.at(i)->isVisible = true;
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

void QFileSystemModel::sort(int column, Qt::SortOrder order)
{
   Q_D(QFileSystemModel);
   if (d->sortOrder == order && d->sortColumn == column && !d->forceSort) {
      return;
   }

   emit layoutAboutToBeChanged();
   QModelIndexList oldList = persistentIndexList();
   QVector<QPair<QFileSystemModelPrivate::QFileSystemNode *, int>> oldNodes;

   for (int i = 0; i < oldList.count(); ++i) {
      const QModelIndex &oldNode = oldList.at(i);

      QPair<QFileSystemModelPrivate::QFileSystemNode *, int> pair(d->node(oldNode), oldNode.column());
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
      const QPair<QFileSystemModelPrivate::QFileSystemNode *, int> &oldNode = oldNodes.at(i);
      newList.append(d->index(oldNode.first, oldNode.second));
   }

   changePersistentIndexList(oldList, newList);
   emit layoutChanged();
}

QStringList QFileSystemModel::mimeTypes() const
{
   return QStringList(QString("text/uri-list"));
}

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

bool QFileSystemModel::dropMimeData(const QMimeData *data, Qt::DropAction action,
   int row, int column, const QModelIndex &parent)
{
   (void) row;
   (void) column;

   if (! parent.isValid() || isReadOnly()) {
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

   if (dirNode->isSymLink()

#ifndef QT_NO_FILESYSTEMWATCHER
      && d->fileInfoGatherer.resolveSymlinks()
#endif
      && d->resolvedSymLinks.contains(fullPath) && dirNode->isDir()) {

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
   (void) q;

   if (! index.isValid()) {
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
   if ((fullPath.length() > 2) && fullPath[0] == QChar('/') && fullPath[1] == QChar('/')) {
      fullPath = fullPath.mid(1);
   }
#endif

#if defined(Q_OS_WIN)
   if (fullPath.length() == 2 && fullPath.endsWith(QChar(':'))) {
      fullPath.append(QChar('/'));
   }
#endif
   return fullPath;
}

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

#ifndef QT_NO_FILESYSTEMWATCHER
   node->populate(d->fileInfoGatherer.getInfo(QFileInfo(dir.absolutePath() + QDir::separator() + name)));
#endif

   d->addVisibleFiles(parentNode, QStringList(name));
   return d->index(node);
}

QFile::Permissions QFileSystemModel::permissions(const QModelIndex &index) const
{
   Q_D(const QFileSystemModel);
   return d->node(index)->permissions();
}

QModelIndex QFileSystemModel::setRootPath(const QString &newPath)
{
   Q_D(QFileSystemModel);

#if defined(Q_OS_WIN)
   QString longNewPath = qt_GetLongPathName(newPath);

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

   if (! rootPath().isEmpty() && rootPath() != QString(".")) {
      // remove the watcher for the old rootPath

#ifndef QT_NO_FILESYSTEMWATCHER
      d->fileInfoGatherer.removePath(rootPath());
#endif

      // "marks" the node as dirty, so the next fetchMore call ask the gatherer to install a watcher again
      // this does not re-fetch everything (?)
      d->node(rootPath())->populatedChildren = false;
   }

   // new valid root path
   d->rootDir = newPathDir;
   QModelIndex newRootIndex;

   if (showDrives) {
      // otherwise dir will become '.'
      d->rootDir.setPath(QString(""));

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
#ifndef QT_NO_FILESYSTEMWATCHER
   d->fileInfoGatherer.setIconProvider(provider);
#endif
   d->root.updateIcon(provider, QString());
}

QFileIconProvider *QFileSystemModel::iconProvider() const
{
#ifndef QT_NO_FILESYSTEMWATCHER
   Q_D(const QFileSystemModel);
   return d->fileInfoGatherer.iconProvider();
#else
   return 0;
#endif
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
#ifndef QT_NO_FILESYSTEMWATCHER
   Q_D(QFileSystemModel);
   d->fileInfoGatherer.setResolveSymlinks(enable);
#endif
}

bool QFileSystemModel::resolveSymlinks() const
{
#ifndef QT_NO_FILESYSTEMWATCHER
   Q_D(const QFileSystemModel);
   return d->fileInfoGatherer.resolveSymlinks();
#else
   return false;
#endif
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

   Q_D(QFileSystemModel);

   if (! d->bypassFilters.isEmpty()) {
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

   QPatternOptionFlags flags = QPatternOption::WildcardOption;

   if (filter() & QDir::CaseSensitive) {
      // do nothing

   } else {
      flags |= QPatternOption::CaseInsensitiveOption;

   }

   for (int i = 0; i < filters.size(); ++i) {
      d->nameFilters << QRegularExpression(filters.at(i), flags);
   }

   d->forceSort = true;
   d->delayedSort();
}

QStringList QFileSystemModel::nameFilters() const
{
   Q_D(const QFileSystemModel);
   QStringList filters;

   for (int i = 0; i < d->nameFilters.size(); ++i) {
      filters << d->nameFilters.at(i).pattern();
   }

   return filters;
}

bool QFileSystemModel::event(QEvent *event)
{
#ifndef QT_NO_FILESYSTEMWATCHER
   Q_D(QFileSystemModel);

   if (event->type() == QEvent::LanguageChange) {
      d->root.retranslateStrings(d->fileInfoGatherer.iconProvider(), QString());
      return true;
   }
#endif
   return QAbstractItemModel::event(event);
}

bool QFileSystemModel::rmdir(const QModelIndex &index)
{
   QString path = filePath(index);
   const bool retval = QDir().rmdir(path);

#ifndef QT_NO_FILESYSTEMWATCHER
   if (retval) {
      QFileSystemModelPrivate *d = const_cast<QFileSystemModelPrivate *>(d_func());
      d->fileInfoGatherer.removePath(path);
   }
#endif

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
   // parentNode is "" so we are listing the drives

   if (parentNode->fileName.isEmpty()) {
      std::wstring name(MAX_PATH + 1, L'\0');

      // GetVolumeInformation requires to add trailing backslash
      const QString nodeName = fileName + "\\";
      BOOL success = ::GetVolumeInformation(&nodeName.toStdWString()[0], &name[0], MAX_PATH + 1,
                  nullptr, nullptr, nullptr, nullptr, 0);

      if (success && name[0]) {
         node->volumeName = QString::fromStdWString(name);
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

   if (vLocation >= 0 && ! indexHidden) {
      q->beginRemoveRows(parent, translateVisibleLocation(parentNode, vLocation), translateVisibleLocation(parentNode, vLocation));
   }

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


void QFileSystemModelPrivate::addVisibleFiles(QFileSystemNode *parentNode, const QStringList &newFiles)
{
   Q_Q(QFileSystemModel);

   QModelIndex parent = index(parentNode);
   bool indexHidden   = isHiddenByFilter(parentNode, parent);

   if (! indexHidden) {
      q->beginInsertRows(parent, parentNode->visibleChildren.count(),
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

void QFileSystemModelPrivate::_q_fileSystemChanged(const QString &path, const QVector<QPair<QString, QFileInfo>> &updates)
{
#ifndef QT_NO_FILESYSTEMWATCHER
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

      // is there a way to bundle signals with QString as the content of the list?
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

      /* min = QString();
         max = QString();
      */
   }

   if (newFiles.count() > 0) {
      addVisibleFiles(parentNode, newFiles);
   }

   if (newFiles.count() > 0 || (sortColumn != 0 && rowsToUpdate.count() > 0)) {
      forceSort = true;
      delayedSort();
   }
#endif
}

void QFileSystemModelPrivate::_q_resolvedName(const QString &fileName, const QString &resolvedName)
{
   resolvedSymLinks[fileName] = resolvedName;
}

void QFileSystemModelPrivate::init()
{
   Q_Q(QFileSystemModel);

#ifndef QT_NO_FILESYSTEMWATCHER
   q->connect(&fileInfoGatherer, &QFileInfoGatherer::newListOfFiles,  q, &QFileSystemModel::_q_directoryChanged);
   q->connect(&fileInfoGatherer, &QFileInfoGatherer::updates,         q, &QFileSystemModel::_q_fileSystemChanged);
   q->connect(&fileInfoGatherer, &QFileInfoGatherer::nameResolved,    q, &QFileSystemModel::_q_resolvedName);
   q->connect(&fileInfoGatherer, &QFileInfoGatherer::directoryLoaded, q, &QFileSystemModel::directoryLoaded);
#endif

   q->connect(&delayedSortTimer, &QTimer::timeout, q, &QFileSystemModel::_q_performDelayedSort, Qt::QueuedConnection);

   roleNames.insertMulti(QFileSystemModel::FileIconRole, "fileIcon");
   roleNames.insert(QFileSystemModel::FilePathRole,      "filePath");
   roleNames.insert(QFileSystemModel::FileNameRole,      "fileName");
   roleNames.insert(QFileSystemModel::FilePermissions,   "filePermissions");
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

   const bool hideDot           = (filters & QDir::NoDot);
   const bool hideDotDot        = (filters & QDir::NoDotDot);

   bool isDot    = (node->fileName == QString("."));
   bool isDotDot = (node->fileName == QString(".."));

   if ( ( hideHidden && ! (isDot || isDotDot) && node->isHidden() )
      || (hideDirs       && node->isDir())
      || (hideFiles      && node->isFile())
      || (hideSystem     && node->isSystem())
      || (hideReadable   && node->isReadable())
      || (hideWritable   && node->isWritable())
      || (hideExecutable && node->isExecutable())
      || (hideSymlinks   && node->isSymLink())
      || (hideDot        && isDot)
      || (hideDotDot     && isDotDot)) {

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
         if (nameFilters.at(i).match(node->fileName).hasMatch()) {
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

void QFileSystemModel::_q_fileSystemChanged(const QString &path, const QVector<QPair<QString, QFileInfo>> &data)
{
   Q_D(QFileSystemModel);
   d->_q_fileSystemChanged(path, data);
}

void QFileSystemModel::_q_resolvedName(const QString &fileName, const QString &resolvedName)
{
   Q_D(QFileSystemModel);
   d->_q_resolvedName(fileName, resolvedName);
}

#endif // QT_NO_FILESYSTEMMODEL
