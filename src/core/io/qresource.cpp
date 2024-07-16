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

#include <qresource.h>

#include <qbytearray.h>
#include <qdatetime.h>
#include <qdebug.h>
#include <qglobal.h>
#include <qhash.h>
#include <qlocale.h>
#include <qmutex.h>
#include <qplatformdefs.h>
#include <qset.h>
#include <qshareddata.h>
#include <qstringlist.h>
#include <qstringparser.h>
#include <qvector.h>

#include <qabstractfileengine_p.h>
#include <qresource_p.h>
#include <qresource_iterator_p.h>

#ifdef Q_OS_UNIX
# include <qcore_unix_p.h>
# define QT_USE_MMAP
#endif

#if defined(QT_USE_MMAP)
// for mmap

#include <sys/mman.h>
#include <errno.h>

#endif

class QResourceRoot
{
   enum Flags {
      Compressed = 0x01,
      Directory  = 0x02
   };

   const uchar *tree, *names, *payloads;

   int findOffset(int node) const {
      return node * 14;   //sizeof each tree element
   }

   int hash(int node) const;
   QString getName(int node) const;
   short flags(int node) const;

 public:
   enum ResourceRootType {
      Resource_Builtin,
      Resource_File,
      Resource_Buffer
   };

   QResourceRoot()
      : tree(nullptr), names(nullptr), payloads(nullptr)
   { }

   QResourceRoot(const uchar *t, const uchar *n, const uchar *d) {
      setSource(t, n, d);
   }

   virtual ~QResourceRoot() { }
   int findNode(const QString &path, const QLocale &locale = QLocale()) const;

   bool isContainer(int node) const {
      return flags(node) & Directory;
   }

   bool isCompressed(int node) const {
      return flags(node) & Compressed;
   }

   const uchar *data(int node, qint64 *size) const;
   QStringList children(int node) const;

   virtual QString mappingRoot() const {
      return QString();
   }

   bool mappingRootSubdir(const QString &path, QString *match = nullptr) const;

   bool operator==(const QResourceRoot &other) const {
      return tree == other.tree && names == other.names && payloads == other.payloads;
   }

   bool operator!=(const QResourceRoot &other) const {
      return !operator==(other);
   }

   virtual ResourceRootType type() const {
      return Resource_Builtin;
   }

   mutable QAtomicInt ref;

 protected:
   void setSource(const uchar *t, const uchar *n, const uchar *d) {
      tree     = t;
      names    = n;
      payloads = d;
   }
};

static QString cleanPath(const QString &_path)
{
   QString path = QDir::cleanPath(_path);

   // QDir::cleanPath does not remove two trailing slashes under _Windows_
   // due to support for UNC paths. Remove those manually.

   if (path.startsWith("//")) {
      path.remove(0, 1);
   }

   return path;
}

static QRecursiveMutex *resourceMutex()
{
   static QRecursiveMutex retval;
   return &retval;
}

using ResourceList = QList<QResourceRoot *>;

static ResourceList *resourceList()
{
   static ResourceList retval;
   return &retval;
}

static QStringList *resourceSearchPaths()
{
   static QStringList retval;
   return &retval;
}

class QResourcePrivate
{
 public:
   QResourcePrivate(QResource *q)
      : q_ptr(q)
   {
      clear();
   }

   ~QResourcePrivate() {
      clear();
   }

   void ensureInitialized() const;
   void ensureChildren() const;

   bool load(const QString &file);
   void clear();

   QLocale locale;
   QString fileName, absoluteFilePath;
   QList<QResourceRoot *> related;

   uint container : 1;
   mutable uint compressed : 1;
   mutable qint64 size;
   mutable const uchar *data;
   mutable QStringList children;

   QResource *q_ptr;
   Q_DECLARE_PUBLIC(QResource)
};

void QResourcePrivate::clear()
{
   absoluteFilePath.clear();
   compressed = 0;
   data       = nullptr;
   size       = 0;
   children.clear();
   container  = 0;

   for (int i = 0; i < related.size(); ++i) {
      QResourceRoot *root = related.at(i);

      if (! root->ref.deref()) {
         delete root;
      }
   }

   related.clear();
}

bool QResourcePrivate::load(const QString &file)
{
   related.clear();

   QRecursiveMutexLocker lock(resourceMutex());
   const ResourceList *list = resourceList();

   QString cleaned = cleanPath(file);

   for (int i = 0; i < list->size(); ++i) {
      QResourceRoot *res = list->at(i);
      const int node = res->findNode(cleaned, locale);

      if (node != -1) {
         if (related.isEmpty()) {
            container = res->isContainer(node);

            if (! container) {
               data = res->data(node, &size);
               compressed = res->isCompressed(node);

            } else {
               data = nullptr;
               size = 0;
               compressed = 0;
            }

         } else if (res->isContainer(node) != container) {
            qWarning("QResourceInfo::load() Resource %s has both data and child resources", csPrintable(file));

         }

         res->ref.ref();
         related.append(res);

      } else if (res->mappingRootSubdir(file)) {
         container  = true;
         data       = nullptr;
         size       = 0;
         compressed = 0;
         res->ref.ref();
         related.append(res);
      }
   }

   return ! related.isEmpty();
}

void QResourcePrivate::ensureInitialized() const
{
   if (! related.isEmpty()) {
      return;
   }

   QResourcePrivate *that = const_cast<QResourcePrivate *>(this);

   if (fileName == ":") {
      that->fileName += '/';
   }

   that->absoluteFilePath = fileName;

   if (! that->absoluteFilePath.startsWith(QChar(':'))) {
      that->absoluteFilePath.prepend(QChar(':'));
   }

   QString path = fileName;

   if (path.startsWith(':')) {
      path = path.mid(1);
   }

   if (path.startsWith('/')) {
      that->load(path);

   } else {
      QRecursiveMutexLocker lock(resourceMutex());

      QStringList searchPaths = *resourceSearchPaths();
      searchPaths << QString("");

      for (int i = 0; i < searchPaths.size(); ++i) {
         const QString searchPath(searchPaths.at(i) + QChar('/') + path);

         if (that->load(searchPath)) {
            that->absoluteFilePath = QChar(':') + searchPath;
            break;
         }
      }
   }
}

void QResourcePrivate::ensureChildren() const
{
   ensureInitialized();

   if (! children.isEmpty() || ! container || related.isEmpty()) {
      return;
   }

   QString path = absoluteFilePath, k;

   if (path.startsWith(QChar(':'))) {
      path = path.mid(1);
   }

   QSet<QString> kids;
   QString cleaned = cleanPath(path);

   for (int i = 0; i < related.size(); ++i) {
      QResourceRoot *res = related.at(i);

      if (res->mappingRootSubdir(path, &k) && ! k.isEmpty()) {
         if (!kids.contains(k)) {
            children += k;
            kids.insert(k);
         }

      } else {
         const int node = res->findNode(cleaned);

         if (node != -1) {
            QStringList related_children = res->children(node);

            for (int kid = 0; kid < related_children.size(); ++kid) {
               k = related_children.at(kid);

               if (! kids.contains(k)) {
                  children += k;
                  kids.insert(k);
               }
            }
         }
      }
   }
}

QResource::QResource(const QString &file, const QLocale &locale)
   : d_ptr(new QResourcePrivate(this))
{
   Q_D(QResource);
   d->fileName = file;
   d->locale = locale;
}

/*!
    Releases the resources of the QResource object.
*/
QResource::~QResource()
{
}

void QResource::setLocale(const QLocale &locale)
{
   Q_D(QResource);
   d->clear();
   d->locale = locale;
}

/*!
    Returns the locale used to locate the data for the QResource.
*/

QLocale QResource::locale() const
{
   Q_D(const QResource);
   return d->locale;
}

/*!
    Sets a QResource to point to \a file. \a file can either be absolute,
    in which case it is opened directly, if relative then the file will be
    tried to be found in QDir::searchPaths().

    \sa absoluteFilePath()
*/

void QResource::setFileName(const QString &file)
{
   Q_D(QResource);
   d->clear();
   d->fileName = file;
}

/*!
    Returns the full path to the file that this QResource represents as it
    was passed.

    \sa absoluteFilePath()
*/

QString QResource::fileName() const
{
   Q_D(const QResource);
   d->ensureInitialized();

   return d->fileName;
}

QString QResource::absoluteFilePath() const
{
   Q_D(const QResource);
   d->ensureInitialized();

   return d->absoluteFilePath;
}

bool QResource::isValid() const
{
   Q_D(const QResource);
   d->ensureInitialized();

   return ! d->related.isEmpty();
}

bool QResource::isCompressed() const
{
   Q_D(const QResource);
   d->ensureInitialized();

   return d->compressed;
}

qint64 QResource::size() const
{
   Q_D(const QResource);
   d->ensureInitialized();

   return d->size;
}

const uchar *QResource::data() const
{
   Q_D(const QResource);
   d->ensureInitialized();
   return d->data;
}

bool QResource::isDir() const
{
   Q_D(const QResource);
   d->ensureInitialized();
   return d->container;
}

QStringList QResource::children() const
{
   Q_D(const QResource);
   d->ensureChildren();
   return d->children;
}

// obsolete
void QResource::addSearchPath(const QString &path)
{
   if (! path.startsWith(QChar('/'))) {
      qWarning("QResource::addResourceSearchPath() Search path %s must be absolute", csPrintable(path));
      return;
   }

   QRecursiveMutexLocker lock(resourceMutex());
   resourceSearchPaths()->prepend(path);
}

// obsolete
QStringList QResource::searchPaths()
{
   QRecursiveMutexLocker lock(resourceMutex());
   return *resourceSearchPaths();
}

inline int QResourceRoot::hash(int node) const
{
   if (! node) {
      return 0;
   }

   const int offset = findOffset(node);
   int name_offset = (tree[offset + 0] << 24) + (tree[offset + 1] << 16) +
         (tree[offset + 2] << 8) + (tree[offset + 3] << 0);

   name_offset += 2;    // jump past name length

   return (names[name_offset + 0] << 24) + (names[name_offset + 1] << 16) +
         (names[name_offset + 2] << 8) + (names[name_offset + 3] << 0);
}

inline QString QResourceRoot::getName(int node) const
{
   if (! node) {
      return QString();
   }

   QString retval;

   const int offset = findOffset(node);

   int name_offset  = (tree[offset + 0] << 24) + (tree[offset + 1] << 16)
         + (tree[offset + 2] << 8) + (tree[offset + 3] << 0);

   const short name_length = (names[name_offset + 0] << 8) + (names[name_offset + 1] << 0);

   name_offset += 2;
   name_offset += 4;    // jump past hash

   retval = QString::fromUtf8((const char *)names + name_offset, name_length);

   return retval;
}

int QResourceRoot::findNode(const QString &xPath, const QLocale &locale) const
{
   QString path = xPath;

   {
      QString root = mappingRoot();

      if (! root.isEmpty()) {

         if (root == path) {
            path = '/';

         } else {

            if (! root.endsWith('/')) {
               root += '/';
            }

            if (path.size() >= root.size() && path.startsWith(root)) {
               path = path.mid(root.length() - 1);
            }

            if (path.isEmpty()) {
               path = '/';
            }
         }
      }
   }

   if (path == "/") {
      return 0;
   }

   // root node is always first
   int child_count = (tree[6]  << 24) + (tree[7]  << 16) + (tree[8] << 8)  + (tree[9] << 0);
   int child       = (tree[10] << 24) + (tree[11] << 16) + (tree[12] << 8) + (tree[13] << 0);

   // now iterate up the tree
   int node = -1;

   QList<QString> segList = path.split('/', QStringParser::SkipEmptyParts);
   auto iter = segList.begin();

   while (child_count && iter != segList.end() ) {

      const QString &segment = *iter;
      ++iter;

      const int hashValue = cs_stable_hash(segment);

      // do the binary search for the hash
      int l = 0;
      int r = child_count - 1;

      int sub_node = (l + r + 1) / 2;

      while (r != l) {
         const int sub_node_hash = hash(child + sub_node);

         if (hashValue == sub_node_hash) {
            break;

         } else if (hashValue < sub_node_hash) {
            r = sub_node - 1;

         } else {
            l = sub_node;
         }

         sub_node = (l + r + 1) / 2;
      }

      sub_node += child;

      // now do the "harder" compares
      bool found = false;

      if (hash(sub_node) == hashValue) {
         while (sub_node > child && hash(sub_node - 1) == hashValue) {
            // backup for collisions
            --sub_node;
         }

         for (; sub_node < child + child_count && hash(sub_node) == hashValue; ++sub_node) {

            if (getName(sub_node) == segment) {
               found = true;

               int offset = findOffset(sub_node);
               offset += 4;                             // jump past name

               const short flags = (tree[offset + 0] << 8) + (tree[offset + 1] << 0);
               offset += 2;

               if (iter == segList.end()) {

                  if (! (flags & Directory)) {
                     const short country = (tree[offset + 0] << 8) + (tree[offset + 1] << 0);
                     offset += 2;

                     const short language = (tree[offset + 0] << 8) + (tree[offset + 1] << 0);
                     offset += 2;

                     if (country == locale.country() && language == locale.language()) {
                        return sub_node;

                     } else if ((country == QLocale::AnyCountry && language == locale.language()) ||
                           (country == QLocale::AnyCountry && language == QLocale::C && node == -1)) {
                        node = sub_node;
                     }

                     continue;

                  } else {
                     return sub_node;
                  }
               }

               if (! (flags & Directory)) {
                  return -1;
               }

               child_count = (tree[offset + 0] << 24) + (tree[offset + 1] << 16) +
                     (tree[offset + 2] << 8) + (tree[offset + 3] << 0);

               offset += 4;

               child = (tree[offset + 0] << 24) + (tree[offset + 1] << 16) +
                     (tree[offset + 2] << 8) + (tree[offset + 3] << 0);
               break;
            }
         }
      }

      if (! found) {
         break;
      }
   }

   return node;
}

short QResourceRoot::flags(int node) const
{
   if (node == -1) {
      return 0;
   }

   const int offset = findOffset(node) + 4;                   // jump past name
   return (tree[offset + 0] << 8) + (tree[offset + 1] << 0);
}

const uchar *QResourceRoot::data(int node, qint64 *size) const
{
   if (node == -1) {
      *size = 0;
      return nullptr;
   }

   int offset = findOffset(node) + 4; //jump past name

   const short flags = (tree[offset + 0] << 8) + (tree[offset + 1] << 0);
   offset += 2;

   offset += 4; //jump past locale

   if (! (flags & Directory)) {
      const int data_offset = (tree[offset + 0] << 24) + (tree[offset + 1] << 16) +
            (tree[offset + 2] << 8) + (tree[offset + 3] << 0);

      const uint data_length = (payloads[data_offset + 0] << 24) + (payloads[data_offset + 1] << 16) +
            (payloads[data_offset + 2] << 8) + (payloads[data_offset + 3] << 0);

      const uchar *ret = payloads + data_offset + 4;
      *size = data_length;

      return ret;
   }

   *size = 0;
   return nullptr;
}

QStringList QResourceRoot::children(int node) const
{
   if (node == -1) {
      return QStringList();
   }

   int offset = findOffset(node) + 4; //jump past name

   const short flags = (tree[offset + 0] << 8) + (tree[offset + 1] << 0);
   offset += 2;

   QStringList ret;

   if (flags & Directory) {
      const int child_count = (tree[offset + 0] << 24) + (tree[offset + 1] << 16) +
            (tree[offset + 2] << 8) + (tree[offset + 3] << 0);

      offset += 4;

      const int child_off = (tree[offset + 0] << 24) + (tree[offset + 1] << 16) +
            (tree[offset + 2] << 8) + (tree[offset + 3] << 0);

      for (int i = child_off; i < child_off + child_count; ++i) {
         ret << getName(i);
      }
   }

   return ret;
}

bool QResourceRoot::mappingRootSubdir(const QString &path, QString *match) const
{
   const QString root = mappingRoot();

   if (! root.isEmpty()) {

      const QStringList root_segments = root.split('/', QStringParser::SkipEmptyParts),
            path_segments = path.split('/', QStringParser::SkipEmptyParts);

      if (path_segments.size() <= root_segments.size()) {
         int matched = 0;

         for (int i = 0; i < path_segments.size(); ++i) {
            if (root_segments[i] != path_segments[i]) {
               break;
            }

            ++matched;
         }

         if (matched == path_segments.size()) {
            if (match && root_segments.size() > matched) {
               *match = root_segments.at(matched);
            }

            return true;
         }
      }
   }

   return false;
}

Q_CORE_EXPORT bool qRegisterResourceData(int version, const unsigned char *tree,
      const unsigned char *name, const unsigned char *data)
{
   QRecursiveMutexLocker lock(resourceMutex());

   if (version == 0x01 && resourceList()) {
      bool found = false;
      QResourceRoot res(tree, name, data);

      for (int i = 0; i < resourceList()->size(); ++i) {
         if (*resourceList()->at(i) == res) {
            found = true;
            break;
         }
      }

      if (! found) {
         QResourceRoot *root = new QResourceRoot(tree, name, data);
         root->ref.ref();
         resourceList()->append(root);
      }

      return true;
   }

   return false;
}

Q_CORE_EXPORT bool qUnregisterResourceData(int version, const unsigned char *tree,
      const unsigned char *name, const unsigned char *data)
{
   QRecursiveMutexLocker lock(resourceMutex());

   if (version == 0x01 && resourceList()) {
      QResourceRoot res(tree, name, data);

      for (int i = 0; i < resourceList()->size(); ) {
         if (*resourceList()->at(i) == res) {
            QResourceRoot *root = resourceList()->takeAt(i);

            if (! root->ref.deref()) {
               delete root;
            }

         } else {
            ++i;
         }
      }

      return true;
   }

   return false;
}

//run time resource creation

class QDynamicBufferResourceRoot: public QResourceRoot
{
 private:
   QString root;
   const uchar *buffer;

 public:
   QDynamicBufferResourceRoot(const QString &_root)
      : root(_root), buffer(nullptr)
   { }

   ~QDynamicBufferResourceRoot()
   { }

   const uchar *mappingBuffer() const {
      return buffer;
   }

   QString mappingRoot() const override {
      return root;
   }

   ResourceRootType type() const override {
      return Resource_Buffer;
   }

   bool registerSelf(const uchar *b) {
      //setup the data now
      int offset = 0;

      //magic number
      if (b[offset + 0] != 'q' || b[offset + 1] != 'r' ||
            b[offset + 2] != 'e' || b[offset + 3] != 's') {
         return false;
      }

      offset += 4;

      const int version = (b[offset + 0] << 24) + (b[offset + 1] << 16) +
            (b[offset + 2] << 8) + (b[offset + 3] << 0);
      offset += 4;

      const int tree_offset = (b[offset + 0] << 24) + (b[offset + 1] << 16) +
            (b[offset + 2] << 8) + (b[offset + 3] << 0);
      offset += 4;

      const int data_offset = (b[offset + 0] << 24) + (b[offset + 1] << 16) +
            (b[offset + 2] << 8) + (b[offset + 3] << 0);
      offset += 4;

      const int name_offset = (b[offset + 0] << 24) + (b[offset + 1] << 16) +
            (b[offset + 2] << 8) + (b[offset + 3] << 0);
      offset += 4;

      if (version == 0x01) {
         buffer = b;
         setSource(b + tree_offset, b + name_offset, b + data_offset);
         return true;
      }

      return false;
   }
};

class QDynamicFileResourceRoot: public QDynamicBufferResourceRoot
{
   QString fileName;

   // for mmap'ed files, this is what needs to be unmapped.
   uchar *unmapPointer;
   unsigned int unmapLength;

 public:
   QDynamicFileResourceRoot(const QString &_root)
      : QDynamicBufferResourceRoot(_root), unmapPointer(nullptr), unmapLength(0)
   {
   }

   ~QDynamicFileResourceRoot() {
#if defined(QT_USE_MMAP)

      if (unmapPointer) {
         munmap((char *)unmapPointer, unmapLength);
         unmapPointer = nullptr;
         unmapLength = 0;
      } else
#endif
      {
         delete [] (uchar *)mappingBuffer();
      }
   }

   QString mappingFile() const {
      return fileName;
   }

   ResourceRootType type() const override {
      return Resource_File;
   }

   bool registerSelf(const QString &f) {
      bool fromMM = false;
      uchar *data = nullptr;
      unsigned int data_len = 0;

#ifdef QT_USE_MMAP

#ifndef MAP_FILE
#define MAP_FILE 0
#endif

#ifndef MAP_FAILED
#define MAP_FAILED -1
#endif

#if defined(Q_OS_WIN)
      int fd = QT_OPEN(QFile::encodeName(f).constData(), O_RDONLY, _S_IREAD | _S_IWRITE);
#else
      int fd = QT_OPEN(QFile::encodeName(f).constData(), O_RDONLY, 0666);
#endif

      if (fd >= 0) {
         QT_STATBUF st;

         if (! QT_FSTAT(fd, &st)) {
            uchar *ptr;
            ptr = reinterpret_cast<uchar *>(mmap(nullptr, st.st_size, PROT_READ, MAP_FILE | MAP_PRIVATE, fd, 0));

            if (ptr && ptr != reinterpret_cast<uchar *>(MAP_FAILED)) {
               data = ptr;
               data_len = st.st_size;
               fromMM = true;
            }
         }

         ::close(fd);
      }

#endif // QT_USE_MMAP

      if (! data) {
         QFile file(f);

         if (!file.exists()) {
            return false;
         }

         data_len = file.size();
         data = new uchar[data_len];

         bool ok = false;

         if (file.open(QIODevice::ReadOnly)) {
            ok = (data_len == (uint)file.read((char *)data, data_len));
         }

         if (!ok) {
            delete [] data;
            data = nullptr;
            data_len = 0;
            return false;
         }

         fromMM = false;
      }

      if (data && QDynamicBufferResourceRoot::registerSelf(data)) {
         if (fromMM) {
            unmapPointer = data;
            unmapLength = data_len;
         }

         fileName = f;
         return true;
      }

      return false;
   }
};

static QString qt_resource_fixResourceRoot(QString r)
{
   if (! r.isEmpty()) {
      if (r.startsWith(':')) {
         r = r.mid(1);
      }

      if (!r.isEmpty()) {
         r = QDir::cleanPath(r);
      }
   }

   return r;
}

bool QResource::registerResource(const QString &rccFilename, const QString &resourceRoot)
{
   QString r = qt_resource_fixResourceRoot(resourceRoot);

   if (! r.isEmpty() && r[0] != '/') {
      qWarning("QDir::registerResource() Registering resource %s requires an absolute path start with '/', current root is %s",
            csPrintable(rccFilename), csPrintable(resourceRoot));

      return false;
   }

   QDynamicFileResourceRoot *root = new QDynamicFileResourceRoot(r);

   if (root->registerSelf(rccFilename)) {
      root->ref.ref();

      QRecursiveMutexLocker lock(resourceMutex());
      resourceList()->append(root);
      return true;
   }

   delete root;
   return false;
}

bool QResource::unregisterResource(const QString &rccFilename, const QString &resourceRoot)
{
   QString r = qt_resource_fixResourceRoot(resourceRoot);

   QRecursiveMutexLocker lock(resourceMutex());
   ResourceList *list = resourceList();

   for (int i = 0; i < list->size(); ++i) {
      QResourceRoot *res = list->at(i);

      if (res->type() == QResourceRoot::Resource_File) {
         QDynamicFileResourceRoot *root = reinterpret_cast<QDynamicFileResourceRoot *>(res);

         if (root->mappingFile() == rccFilename && root->mappingRoot() == r) {
            resourceList()->removeAt(i);

            if (!root->ref.deref()) {
               delete root;
               return true;
            }

            return false;
         }
      }
   }

   return false;
}

bool QResource::registerResource(const uchar *rccData, const QString &resourceRoot)
{
   QString r = qt_resource_fixResourceRoot(resourceRoot);

   if (! r.isEmpty() && r[0] != QLatin1Char('/')) {
      qWarning("QDir::registerResource() Registering resource %s requires an absolute path start with '/', current root is %s",
            rccData, csPrintable(resourceRoot));
      return false;
   }

   QDynamicBufferResourceRoot *root = new QDynamicBufferResourceRoot(r);

   if (root->registerSelf(rccData)) {
      root->ref.ref();
      QRecursiveMutexLocker lock(resourceMutex());
      resourceList()->append(root);
      return true;
   }

   delete root;

   return false;
}

bool QResource::unregisterResource(const uchar *rccData, const QString &resourceRoot)
{
   QString r = qt_resource_fixResourceRoot(resourceRoot);

   QRecursiveMutexLocker lock(resourceMutex());
   ResourceList *list = resourceList();

   for (int i = 0; i < list->size(); ++i) {
      QResourceRoot *res = list->at(i);

      if (res->type() == QResourceRoot::Resource_Buffer) {
         QDynamicBufferResourceRoot *root = reinterpret_cast<QDynamicBufferResourceRoot *>(res);

         if (root->mappingBuffer() == rccData && root->mappingRoot() == r) {
            resourceList()->removeAt(i);

            if (!root->ref.deref()) {
               delete root;
               return true;
            }

            return false;
         }
      }
   }

   return false;
}

// resource engine
class QResourceFileEnginePrivate : public QAbstractFileEnginePrivate
{
 protected:
   Q_DECLARE_PUBLIC(QResourceFileEngine)

 private:
   uchar *map(qint64 offset, qint64 size, QFile::MemoryMapFlags flags);
   bool unmap(uchar *ptr);

   qint64 m_offset;

   QResource resource;
   QByteArray uncompressed;

 protected:
   QResourceFileEnginePrivate()
      : m_offset(0)
   { }
};

bool QResourceFileEngine::mkdir(const QString &, bool) const
{
   return false;
}

bool QResourceFileEngine::rmdir(const QString &, bool) const
{
   return false;
}

bool QResourceFileEngine::setSize(qint64)
{
   return false;
}

QStringList QResourceFileEngine::entryList(QDir::Filters filters, const QStringList &filterNames) const
{
   return QAbstractFileEngine::entryList(filters, filterNames);
}

bool QResourceFileEngine::caseSensitive() const
{
   return true;
}

QResourceFileEngine::QResourceFileEngine(const QString &file)
   : QAbstractFileEngine(*new QResourceFileEnginePrivate)
{
   Q_D(QResourceFileEngine);
   d->resource.setFileName(file);

   if (d->resource.isCompressed() && d->resource.size()) {
      d->uncompressed = qUncompress(d->resource.data(), d->resource.size());
   }
}

QResourceFileEngine::~QResourceFileEngine()
{
}

void QResourceFileEngine::setFileName(const QString &file)
{
   Q_D(QResourceFileEngine);
   d->resource.setFileName(file);
}

bool QResourceFileEngine::open(QIODevice::OpenMode flags)
{
   Q_D(QResourceFileEngine);

   if (d->resource.fileName().isEmpty()) {
      qWarning("QResourceFileEngine::open() Missing file name");
      return false;
   }

   if (flags & QIODevice::WriteOnly) {
      return false;
   }

   if (! d->resource.isValid()) {
      return false;
   }

   return true;
}

bool QResourceFileEngine::close()
{
   Q_D(QResourceFileEngine);
   d->m_offset = 0;
   d->uncompressed.clear();

   return true;
}

bool QResourceFileEngine::flush()
{
   return true;
}

qint64 QResourceFileEngine::read(char *data, qint64 len)
{
   Q_D(QResourceFileEngine);

   if (len > size() - d->m_offset) {
      len = size() - d->m_offset;
   }

   if (len <= 0) {
      return 0;
   }

   if (d->resource.isCompressed()) {
      memcpy(data, d->uncompressed.constData() + d->m_offset, len);
   } else {
      memcpy(data, d->resource.data() + d->m_offset, len);
   }

   d->m_offset += len;

   return len;
}

qint64 QResourceFileEngine::write(const char *, qint64)
{
   return -1;
}

bool QResourceFileEngine::remove()
{
   return false;
}

bool QResourceFileEngine::copy(const QString &)
{
   return false;
}

bool QResourceFileEngine::rename(const QString &)
{
   return false;
}

bool QResourceFileEngine::link(const QString &)
{
   return false;
}

qint64 QResourceFileEngine::size() const
{
   Q_D(const QResourceFileEngine);

   if (! d->resource.isValid()) {
      return 0;
   }

   if (d->resource.isCompressed()) {
      return d->uncompressed.size();
   }

   return d->resource.size();
}

qint64 QResourceFileEngine::pos() const
{
   Q_D(const QResourceFileEngine);
   return d->m_offset;
}

bool QResourceFileEngine::atEnd() const
{
   Q_D(const QResourceFileEngine);

   if (! d->resource.isValid()) {
      return true;
   }

   return d->m_offset == size();
}

bool QResourceFileEngine::seek(qint64 pos)
{
   Q_D(QResourceFileEngine);

   if (! d->resource.isValid()) {
      return false;
   }

   if (d->m_offset > size()) {
      return false;
   }

   d->m_offset = pos;

   return true;
}

bool QResourceFileEngine::isSequential() const
{
   return false;
}

QAbstractFileEngine::FileFlags QResourceFileEngine::fileFlags(QAbstractFileEngine::FileFlags type) const
{
   Q_D(const QResourceFileEngine);

   QAbstractFileEngine::FileFlags ret = Qt::EmptyFlag;

   if (! d->resource.isValid()) {
      return ret;
   }

   if (type & PermsMask) {
      ret |= QAbstractFileEngine::FileFlags(ReadOwnerPerm | ReadUserPerm | ReadGroupPerm | ReadOtherPerm);
   }

   if (type & TypesMask) {
      if (d->resource.isDir()) {
         ret |= DirectoryType;
      } else {
         ret |= FileType;
      }
   }

   if (type & FlagsMask) {
      ret |= ExistsFlag;

      if (d->resource.absoluteFilePath() == ":/") {
         ret |= RootFlag;
      }
   }

   return ret;
}

bool QResourceFileEngine::setPermissions(uint)
{
   return false;
}

QString QResourceFileEngine::fileName(FileName file) const
{
   Q_D(const QResourceFileEngine);

   if (file == BaseName) {
      int slash = d->resource.fileName().lastIndexOf(QLatin1Char('/'));

      if (slash == -1) {
         return d->resource.fileName();
      }

      return d->resource.fileName().mid(slash + 1);

   } else if (file == PathName || file == AbsolutePathName) {
      const QString path = (file == AbsolutePathName) ? d->resource.absoluteFilePath() : d->resource.fileName();
      const int slash = path.lastIndexOf(QLatin1Char('/'));

      if (slash == -1) {
         return QLatin1String(":");
      } else if (slash <= 1) {
         return QLatin1String(":/");
      }

      return path.left(slash);

   } else if (file == CanonicalName || file == CanonicalPathName) {
      const QString absoluteFilePath = d->resource.absoluteFilePath();

      if (file == CanonicalPathName) {
         const int slash = absoluteFilePath.lastIndexOf(QLatin1Char('/'));

         if (slash != -1) {
            return absoluteFilePath.left(slash);
         }
      }

      return absoluteFilePath;
   }

   return d->resource.fileName();
}

bool QResourceFileEngine::isRelativePath() const
{
   return false;
}

uint QResourceFileEngine::ownerId(FileOwner) const
{
   static constexpr const uint nobodyID = (uint) - 2;
   return nobodyID;
}

QString QResourceFileEngine::owner(FileOwner) const
{
   return QString();
}

QDateTime QResourceFileEngine::fileTime(FileTime) const
{
   return QDateTime();
}

QAbstractFileEngineIterator *QResourceFileEngine::beginEntryList(QDir::Filters filters,
      const QStringList &filterNames)
{
   return new QResourceFileEngineIterator(filters, filterNames);
}

QAbstractFileEngineIterator *QResourceFileEngine::endEntryList()
{
   return nullptr;
}

bool QResourceFileEngine::extension(Extension extension, const ExtensionOption *option, ExtensionReturn *output)
{
   Q_D(QResourceFileEngine);

   if (extension == MapExtension) {
      const MapExtensionOption *options = (MapExtensionOption *)(option);
      MapExtensionReturn *returnValue = static_cast<MapExtensionReturn *>(output);
      returnValue->address = d->map(options->offset, options->size, options->flags);
      return (returnValue->address != nullptr);
   }

   if (extension == UnMapExtension) {
      UnMapExtensionOption *options = (UnMapExtensionOption *)option;
      return d->unmap(options->address);
   }

   return false;
}

bool QResourceFileEngine::supportsExtension(Extension extension) const
{
   return (extension == UnMapExtension || extension == MapExtension);
}

uchar *QResourceFileEnginePrivate::map(qint64 offset, qint64 size, QFile::MemoryMapFlags flags)
{
   Q_Q(QResourceFileEngine);

   (void) flags;

   if (offset < 0 || size <= 0 || !resource.isValid() || offset + size > resource.size()) {
      q->setError(QFile::UnspecifiedError, QString());
      return nullptr;
   }

   uchar *address = const_cast<uchar *>(resource.data());

   return (address + offset);
}

bool QResourceFileEnginePrivate::unmap(uchar *ptr)
{
   (void) ptr;
   return true;
}
