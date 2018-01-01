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

#ifndef RCC_H
#define RCC_H

#include <QtCore/QStringList>
#include <QtCore/QHash>
#include <QtCore/QString>

QT_BEGIN_NAMESPACE

class RCCFileInfo;
class QIODevice;
class QTextStream;


class RCCResourceLibrary
{
   RCCResourceLibrary(const RCCResourceLibrary &);
   RCCResourceLibrary &operator=(const RCCResourceLibrary &);

 public:
   RCCResourceLibrary();
   ~RCCResourceLibrary();

   bool output(QIODevice &out, QIODevice &errorDevice);

   bool readFiles(bool ignoreErrors, QIODevice &errorDevice);

   enum Format { Binary, C_Code };
   void setFormat(Format f) {
      m_format = f;
   }
   Format format() const {
      return m_format;
   }

   void setInputFiles(const QStringList &files) {
      m_fileNames = files;
   }
   QStringList inputFiles() const {
      return m_fileNames;
   }

   QStringList dataFiles() const;

   // Return a map of resource identifier (':/newPrefix/images/p1.png') to file.
   typedef QHash<QString, QString> ResourceDataFileMap;
   ResourceDataFileMap resourceDataFileMap() const;

   void setVerbose(bool b) {
      m_verbose = b;
   }
   bool verbose() const {
      return m_verbose;
   }

   void setInitName(const QString &name) {
      m_initName = name;
   }
   QString initName() const {
      return m_initName;
   }

   void setCompressLevel(int c) {
      m_compressLevel = c;
   }
   int compressLevel() const {
      return m_compressLevel;
   }

   void setCompressThreshold(int t) {
      m_compressThreshold = t;
   }
   int compressThreshold() const {
      return m_compressThreshold;
   }

   void setResourceRoot(const QString &root) {
      m_resourceRoot = root;
   }
   QString resourceRoot() const {
      return m_resourceRoot;
   }

   void setUseNameSpace(bool v) {
      m_useNameSpace = v;
   }
   bool useNameSpace() const {
      return m_useNameSpace;
   }

   QStringList failedResources() const {
      return m_failedResources;
   }

 private:
   struct Strings {
      Strings();
      const QString TAG_RCC;
      const QString TAG_RESOURCE;
      const QString TAG_FILE;
      const QString ATTRIBUTE_LANG;
      const QString ATTRIBUTE_PREFIX;
      const QString ATTRIBUTE_ALIAS;
      const QString ATTRIBUTE_THRESHOLD;
      const QString ATTRIBUTE_COMPRESS;
   };

   friend class RCCFileInfo;

   void reset();
   bool addFile(const QString &alias, const RCCFileInfo &file);

   bool interpretResourceFile(QIODevice *inputDevice, const QString &file,
                  QString currentPath = QString(), bool ignoreErrors = false);

   bool writeHeader();
   bool writeDataBlobs();
   bool writeDataNames();
   bool writeDataStructure();
   bool writeInitializer();
   void writeMangleNamespaceFunction(const QByteArray &name);
   void writeAddNamespaceFunction(const QByteArray &name);
   void writeHex(quint8 number);
   void writeNumber2(quint16 number);
   void writeNumber4(quint32 number);

   void writeChar(char c) {
      m_out.append(c);
   }

   void writeByteArray(const QByteArray &);
   void write(const char *, int len);

   const Strings m_strings;
   RCCFileInfo *m_root;
   QStringList m_fileNames;
   QString m_resourceRoot;
   QString m_initName;
   Format m_format;
   bool m_verbose;
   int m_compressLevel;
   int m_compressThreshold;
   int m_treeOffset;
   int m_namesOffset;
   int m_dataOffset;
   bool m_useNameSpace;
   QStringList m_failedResources;
   QIODevice *m_errorDevice;
   QByteArray m_out;
};

QT_END_NAMESPACE

#endif
