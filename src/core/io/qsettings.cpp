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

#include <qdebug.h>
#include <qplatformdefs.h>
#include <qsettings.h>

#ifndef QT_NO_SETTINGS

#include <qsettings_p.h>
#include <qcache.h>
#include <qfile.h>
#include <qdir.h>
#include <qfileinfo.h>
#include <qmutex.h>
#include <qlibraryinfo.h>
#include <qtemporaryfile.h>

#ifndef QT_NO_TEXTCODEC
#  include <qtextcodec.h>
#endif

#include <qsize.h>
#include <qpoint.h>
#include <qrect.h>
#include <qcoreapplication.h>

#ifdef Q_OS_WIN                // for homedirpath reading from registry
#include <qt_windows.h>
#include <qsystemlibrary_p.h>
#endif

#include <stdlib.h>

#ifndef CSIDL_COMMON_APPDATA
#define CSIDL_COMMON_APPDATA	0x0023  // All Users\Application Data
#endif

#ifndef CSIDL_APPDATA
#define CSIDL_APPDATA		0x001a	// <username>\Application Data
#endif

// ************************************************************************
// QConfFile

/*
    QConfFile objects are explicitly shared within the application.
    This ensures that modification to the settings done through one
    QSettings object are immediately reflected in other setting
    objects of the same application.
*/

QT_BEGIN_NAMESPACE

struct QConfFileCustomFormat {
   QString extension;
   QSettings::ReadFunc readFunc;
   QSettings::WriteFunc writeFunc;
   Qt::CaseSensitivity caseSensitivity;
};

typedef QHash<QString, QConfFile *> ConfFileHash;
typedef QCache<QString, QConfFile> ConfFileCache;
typedef QHash<int, QString> PathHash;
typedef QVector<QConfFileCustomFormat> CustomFormatVector;

Q_GLOBAL_STATIC(ConfFileHash, usedHashFunc)
Q_GLOBAL_STATIC(ConfFileCache, unusedCacheFunc)
Q_GLOBAL_STATIC(PathHash, pathHashFunc)
Q_GLOBAL_STATIC(CustomFormatVector, customFormatVectorFunc)
Q_GLOBAL_STATIC(QMutex, globalMutex)
static QSettings::Format globalDefaultFormat = QSettings::NativeFormat;

#ifndef Q_OS_WIN
inline bool qt_isEvilFsTypeName(const char *name)
{
   return (qstrncmp(name, "nfs", 3) == 0
           || qstrncmp(name, "autofs", 6) == 0
           || qstrncmp(name, "cachefs", 7) == 0);
}

#if defined(Q_OS_BSD4) && !defined(Q_OS_NETBSD)
QT_BEGIN_INCLUDE_NAMESPACE
# include <sys/param.h>
# include <sys/mount.h>
QT_END_INCLUDE_NAMESPACE

static bool qIsLikelyToBeNfs(int handle)
{
   struct statfs buf;
   if (fstatfs(handle, &buf) != 0) {
      return false;
   }
   return qt_isEvilFsTypeName(buf.f_fstypename);
}

#elif defined(Q_OS_LINUX)
QT_BEGIN_INCLUDE_NAMESPACE
# include <sys/vfs.h>
# ifdef QT_LINUXBASE
// LSB 3.2 has fstatfs in sys/statfs.h, sys/vfs.h is just an empty dummy header
#  include <sys/statfs.h>
# endif

QT_END_INCLUDE_NAMESPACE
# ifndef NFS_SUPER_MAGIC
#  define NFS_SUPER_MAGIC       0x00006969
# endif
# ifndef AUTOFS_SUPER_MAGIC
#  define AUTOFS_SUPER_MAGIC    0x00000187
# endif
# ifndef AUTOFSNG_SUPER_MAGIC
#  define AUTOFSNG_SUPER_MAGIC  0x7d92b1a0
# endif

static bool qIsLikelyToBeNfs(int handle)
{
   struct statfs buf;
   if (fstatfs(handle, &buf) != 0) {
      return false;
   }
   return buf.f_type == NFS_SUPER_MAGIC
          || buf.f_type == AUTOFS_SUPER_MAGIC
          || buf.f_type == AUTOFSNG_SUPER_MAGIC;
}

#elif defined(Q_OS_SOLARIS) || defined(Q_OS_HPUX) || defined(Q_OS_UNIXWARE)  || defined(Q_OS_NETBSD)
QT_BEGIN_INCLUDE_NAMESPACE
# include <sys/statvfs.h>
QT_END_INCLUDE_NAMESPACE

static bool qIsLikelyToBeNfs(int handle)
{
   struct statvfs buf;
   if (fstatvfs(handle, &buf) != 0) {
      return false;
   }
#if defined(Q_OS_NETBSD)
   return qt_isEvilFsTypeName(buf.f_fstypename);
#else
   return qt_isEvilFsTypeName(buf.f_basetype);
#endif
}
#else
static inline bool qIsLikelyToBeNfs(int /* handle */)
{
   return true;
}
#endif

static bool unixLock(int handle, int lockType)
{
   /*
       NFS hangs on the fcntl() call below when statd or lockd isn't
       running. There's no way to detect this. Our work-around for
       now is to disable locking when we detect NFS (or AutoFS or
       CacheFS, which are probably wrapping NFS).
   */
   if (qIsLikelyToBeNfs(handle)) {
      return false;
   }

   struct flock fl;
   fl.l_whence = SEEK_SET;
   fl.l_start = 0;
   fl.l_len = 0;
   fl.l_type = lockType;
   return fcntl(handle, F_SETLKW, &fl) == 0;
}
#endif

QConfFile::QConfFile(const QString &fileName, bool _userPerms)
   : name(fileName), size(0), ref(1), userPerms(_userPerms)
{
   usedHashFunc()->insert(name, this);
}

QConfFile::~QConfFile()
{
   if (usedHashFunc()) {
      usedHashFunc()->remove(name);
   }
}

ParsedSettingsMap QConfFile::mergedKeyMap() const
{
   ParsedSettingsMap result = originalKeys;
   ParsedSettingsMap::const_iterator i;

   for (i = removedKeys.begin(); i != removedKeys.end(); ++i) {
      result.remove(i.key());
   }
   for (i = addedKeys.begin(); i != addedKeys.end(); ++i) {
      result.insert(i.key(), i.value());
   }
   return result;
}

bool QConfFile::isWritable() const
{
   QFileInfo fileInfo(name);

#ifndef QT_NO_TEMPORARYFILE
   if (fileInfo.exists()) {
#endif
      QFile file(name);
      return file.open(QFile::ReadWrite);
#ifndef QT_NO_TEMPORARYFILE
   } else {
      // Create the directories to the file.
      QDir dir(fileInfo.absolutePath());
      if (!dir.exists()) {
         if (!dir.mkpath(dir.absolutePath())) {
            return false;
         }
      }

      // we use a temporary file to avoid race conditions
      QTemporaryFile file(name);
      return file.open();
   }
#endif
}

QConfFile *QConfFile::fromName(const QString &fileName, bool _userPerms)
{
   QString absPath = QFileInfo(fileName).absoluteFilePath();

   ConfFileHash *usedHash = usedHashFunc();
   ConfFileCache *unusedCache = unusedCacheFunc();

   QConfFile *confFile = 0;
   QMutexLocker locker(globalMutex());

   if (!(confFile = usedHash->value(absPath))) {
      if ((confFile = unusedCache->take(absPath))) {
         usedHash->insert(absPath, confFile);
      }
   }
   if (confFile) {
      confFile->ref.ref();
      return confFile;
   }
   return new QConfFile(absPath, _userPerms);
}

void QConfFile::clearCache()
{
   QMutexLocker locker(globalMutex());
   unusedCacheFunc()->clear();
}

// ************************************************************************
// QSettingsPrivate

QSettingsPrivate::QSettingsPrivate(QSettings::Format format)
   : format(format), scope(QSettings::UserScope /* nothing better to put */), iniCodec(0), spec(0), fallbacks(true),
     pendingChanges(false), status(QSettings::NoError)
{
}

QSettingsPrivate::QSettingsPrivate(QSettings::Format format, QSettings::Scope scope,
                                   const QString &organization, const QString &application)
   : format(format), scope(scope), organizationName(organization), applicationName(application),
     iniCodec(0), spec(0), fallbacks(true), pendingChanges(false), status(QSettings::NoError)
{
}

QSettingsPrivate::~QSettingsPrivate()
{
}

QString QSettingsPrivate::actualKey(const QString &key) const
{
   QString n = normalizedKey(key);
   Q_ASSERT_X(!n.isEmpty(), "QSettings", "empty key");
   n.prepend(groupPrefix);
   return n;
}

/*
    Returns a string that never starts nor ends with a slash (or an
    empty string). Examples:

            "foo"            becomes   "foo"
            "/foo//bar///"   becomes   "foo/bar"
            "///"            becomes   ""

    This function is optimized to avoid a QString deep copy in the
    common case where the key is already normalized.
*/
QString QSettingsPrivate::normalizedKey(const QString &key)
{
   QString result = key;

   int i = 0;
   while (i < result.size()) {
      while (result.at(i) == QLatin1Char('/')) {
         result.remove(i, 1);
         if (i == result.size()) {
            goto after_loop;
         }
      }
      while (result.at(i) != QLatin1Char('/')) {
         ++i;
         if (i == result.size()) {
            return result;
         }
      }
      ++i; // leave the slash alone
   }

after_loop:
   if (!result.isEmpty()) {
      result.truncate(i - 1);   // remove the trailing slash
   }
   return result;
}

// see also qsettings_win.cpp and qsettings_mac.cpp

#if !defined(Q_OS_WIN) && !defined(Q_OS_MAC)
QSettingsPrivate *QSettingsPrivate::create(QSettings::Format format, QSettings::Scope scope,
      const QString &organization, const QString &application)
{
   return new QConfFileSettingsPrivate(format, scope, organization, application);
}
#endif

#if !defined(Q_OS_WIN)
QSettingsPrivate *QSettingsPrivate::create(const QString &fileName, QSettings::Format format)
{
   return new QConfFileSettingsPrivate(fileName, format);
}
#endif

void QSettingsPrivate::processChild(QString key, ChildSpec spec, QMap<QString, QString> &result)
{
   if (spec != AllKeys) {
      int slashPos = key.indexOf(QLatin1Char('/'));
      if (slashPos == -1) {
         if (spec != ChildKeys) {
            return;
         }
      } else {
         if (spec != ChildGroups) {
            return;
         }
         key.truncate(slashPos);
      }
   }
   result.insert(key, QString());
}

void QSettingsPrivate::beginGroupOrArray(const QSettingsGroup &group)
{
   groupStack.push(group);
   if (!group.name().isEmpty()) {
      groupPrefix += group.name();
      groupPrefix += QLatin1Char('/');
   }
}

/*
    We only set an error if there isn't one set already. This way the user always gets the
    first error that occurred. We always allow clearing errors.
*/

void QSettingsPrivate::setStatus(QSettings::Status status) const
{
   if (status == QSettings::NoError || this->status == QSettings::NoError) {
      this->status = status;
   }
}

void QSettingsPrivate::update()
{
   flush();
   pendingChanges = false;
}

void QSettingsPrivate::requestUpdate()
{
   if (!pendingChanges) {
      pendingChanges = true;

      Q_Q(QSettings);
      QCoreApplication::postEvent(q, new QEvent(QEvent::UpdateRequest));

   }
}

QStringList QSettingsPrivate::variantListToStringList(const QVariantList &l)
{
   QStringList result;
   QVariantList::const_iterator it = l.constBegin();
   for (; it != l.constEnd(); ++it) {
      result.append(variantToString(*it));
   }
   return result;
}

QVariant QSettingsPrivate::stringListToVariantList(const QStringList &l)
{
   QStringList outStringList = l;
   for (int i = 0; i < outStringList.count(); ++i) {
      const QString &str = outStringList.at(i);

      if (str.startsWith(QLatin1Char('@'))) {
         if (str.length() >= 2 && str.at(1) == QLatin1Char('@')) {
            outStringList[i].remove(0, 1);
         } else {
            QVariantList variantList;
            for (int j = 0; j < l.count(); ++j) {
               variantList.append(stringToVariant(l.at(j)));
            }
            return variantList;
         }
      }
   }
   return outStringList;
}

QString QSettingsPrivate::variantToString(const QVariant &v)
{
   QString result;

   switch (v.type()) {
      case QVariant::Invalid:
         result = QLatin1String("@Invalid()");
         break;

      case QVariant::ByteArray: {
         QByteArray a = v.toByteArray();
         result = QLatin1String("@ByteArray(");
         result += QString::fromLatin1(a.constData(), a.size());
         result += QLatin1Char(')');
         break;
      }

      case QVariant::String:
      case QVariant::LongLong:
      case QVariant::ULongLong:
      case QVariant::Int:
      case QVariant::UInt:
      case QVariant::Bool:
      case QVariant::Double:
      case QVariant::KeySequence: {
         result = v.toString();
         if (result.startsWith(QLatin1Char('@'))) {
            result.prepend(QLatin1Char('@'));
         }
         break;
      }

      case QVariant::Rect: {
         QRect r = qvariant_cast<QRect>(v);
         result += QLatin1String("@Rect(");
         result += QString::number(r.x());
         result += QLatin1Char(' ');
         result += QString::number(r.y());
         result += QLatin1Char(' ');
         result += QString::number(r.width());
         result += QLatin1Char(' ');
         result += QString::number(r.height());
         result += QLatin1Char(')');
         break;
      }
      case QVariant::Size: {
         QSize s = qvariant_cast<QSize>(v);
         result += QLatin1String("@Size(");
         result += QString::number(s.width());
         result += QLatin1Char(' ');
         result += QString::number(s.height());
         result += QLatin1Char(')');
         break;
      }
      case QVariant::Point: {
         QPoint p = qvariant_cast<QPoint>(v);
         result += QLatin1String("@Point(");
         result += QString::number(p.x());
         result += QLatin1Char(' ');
         result += QString::number(p.y());
         result += QLatin1Char(')');
         break;
      }

      default: {
         QByteArray a;

         {
            QDataStream s(&a, QIODevice::WriteOnly);
            s.setVersion(QDataStream::Qt_4_0);
            s << v;
         }

         result = QLatin1String("@Variant(");
         result += QString::fromLatin1(a.constData(), a.size());
         result += QLatin1Char(')');

         break;
      }
   }

   return result;
}


QVariant QSettingsPrivate::stringToVariant(const QString &s)
{
   if (s.startsWith(QLatin1Char('@'))) {
      if (s.endsWith(QLatin1Char(')'))) {
         if (s.startsWith(QLatin1String("@ByteArray("))) {
            return QVariant(s.toLatin1().mid(11, s.size() - 12));
         } else if (s.startsWith(QLatin1String("@Variant("))) {
#ifndef QT_NO_DATASTREAM
            QByteArray a(s.toLatin1().mid(9));
            QDataStream stream(&a, QIODevice::ReadOnly);
            stream.setVersion(QDataStream::Qt_4_0);
            QVariant result;
            stream >> result;
            return result;
#else
            Q_ASSERT(!"QSettings: Cannot load custom types without QDataStream support");
#endif

         } else if (s.startsWith(QLatin1String("@Rect("))) {
            QStringList args = QSettingsPrivate::splitArgs(s, 5);
            if (args.size() == 4) {
               return QVariant(QRect(args[0].toInt(), args[1].toInt(), args[2].toInt(), args[3].toInt()));
            }
         } else if (s.startsWith(QLatin1String("@Size("))) {
            QStringList args = QSettingsPrivate::splitArgs(s, 5);
            if (args.size() == 2) {
               return QVariant(QSize(args[0].toInt(), args[1].toInt()));
            }
         } else if (s.startsWith(QLatin1String("@Point("))) {
            QStringList args = QSettingsPrivate::splitArgs(s, 6);
            if (args.size() == 2) {
               return QVariant(QPoint(args[0].toInt(), args[1].toInt()));
            }

         } else if (s == QLatin1String("@Invalid()")) {
            return QVariant();
         }

      }
      if (s.startsWith(QLatin1String("@@"))) {
         return QVariant(s.mid(1));
      }
   }

   return QVariant(s);
}

static const char hexDigits[] = "0123456789ABCDEF";

void QSettingsPrivate::iniEscapedKey(const QString &key, QByteArray &result)
{
   result.reserve(result.length() + key.length() * 3 / 2);
   for (int i = 0; i < key.size(); ++i) {
      uint ch = key.at(i).unicode();

      if (ch == '/') {
         result += '\\';
      } else if ((ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z') || (ch >= '0' && ch <= '9')
                 || ch == '_' || ch == '-' || ch == '.') {
         result += (char)ch;
      } else if (ch <= 0xFF) {
         result += '%';
         result += hexDigits[ch / 16];
         result += hexDigits[ch % 16];
      } else {
         result += "%U";
         QByteArray hexCode;
         for (int i = 0; i < 4; ++i) {
            hexCode.prepend(hexDigits[ch % 16]);
            ch >>= 4;
         }
         result += hexCode;
      }
   }
}

bool QSettingsPrivate::iniUnescapedKey(const QByteArray &key, int from, int to, QString &result)
{
   bool lowercaseOnly = true;
   int i = from;
   result.reserve(result.length() + (to - from));
   while (i < to) {
      int ch = (uchar)key.at(i);

      if (ch == '\\') {
         result += QLatin1Char('/');
         ++i;
         continue;
      }

      if (ch != '%' || i == to - 1) {
         if (uint(ch - 'A') <= 'Z' - 'A') { // only for ASCII
            lowercaseOnly = false;
         }
         result += QLatin1Char(ch);
         ++i;
         continue;
      }

      int numDigits = 2;
      int firstDigitPos = i + 1;

      ch = key.at(i + 1);
      if (ch == 'U') {
         ++firstDigitPos;
         numDigits = 4;
      }

      if (firstDigitPos + numDigits > to) {
         result += QLatin1Char('%');
         // ### missing U
         ++i;
         continue;
      }

      bool ok;
      ch = key.mid(firstDigitPos, numDigits).toInt(&ok, 16);
      if (!ok) {
         result += QLatin1Char('%');
         // ### missing U
         ++i;
         continue;
      }

      QChar qch(ch);
      if (qch.isUpper()) {
         lowercaseOnly = false;
      }
      result += qch;
      i = firstDigitPos + numDigits;
   }
   return lowercaseOnly;
}

void QSettingsPrivate::iniEscapedString(const QString &str, QByteArray &result, QTextCodec *codec)
{
   bool needsQuotes = false;
   bool escapeNextIfDigit = false;
   int i;
   bool useCodec = codec && !str.startsWith(QLatin1String("@ByteArray("))
		   && !str.startsWith(QLatin1String("@Variant("));
   int startPos = result.size();

   result.reserve(startPos + str.size() * 3 / 2);
   for (i = 0; i < str.size(); ++i) {
      uint ch = str.at(i).unicode();
      if (ch == ';' || ch == ',' || ch == '=') {
         needsQuotes = true;
      }

      if (escapeNextIfDigit
            && ((ch >= '0' && ch <= '9')
                || (ch >= 'a' && ch <= 'f')
                || (ch >= 'A' && ch <= 'F'))) {
         result += "\\x";
         result += QByteArray::number(ch, 16);
         continue;
      }

      escapeNextIfDigit = false;

      switch (ch) {
         case '\0':
            result += "\\0";
            escapeNextIfDigit = true;
            break;
         case '\a':
            result += "\\a";
            break;
         case '\b':
            result += "\\b";
            break;
         case '\f':
            result += "\\f";
            break;
         case '\n':
            result += "\\n";
            break;
         case '\r':
            result += "\\r";
            break;
         case '\t':
            result += "\\t";
            break;
         case '\v':
            result += "\\v";
            break;
         case '"':
         case '\\':
            result += '\\';
            result += (char)ch;
            break;
	 default:
	    if (ch <= 0x1F || (ch >= 0x7F && !useCodec)) {
	       result += "\\x";
	       result += QByteArray::number(ch, 16);
	       escapeNextIfDigit = true;
#ifndef QT_NO_TEXTCODEC
	    } else if (useCodec) {
	       // slow
	       result += codec->fromUnicode(str.at(i));
#endif
            } else {
               result += (char)ch;
            }
      }
   }

   if (needsQuotes
         || (startPos < result.size() && (result.at(startPos) == ' '
                                          || result.at(result.size() - 1) == ' '))) {
      result.insert(startPos, '"');
      result += '"';
   }
}

inline static void iniChopTrailingSpaces(QString &str)
{
   int n = str.size() - 1;
   QChar ch;
   while (n >= 0 && ((ch = str.at(n)) == QLatin1Char(' ') || ch == QLatin1Char('\t'))) {
      str.truncate(n--);
   }
}

void QSettingsPrivate::iniEscapedStringList(const QStringList &strs, QByteArray &result, QTextCodec *codec)
{
   if (strs.isEmpty()) {
      /*
          We need to distinguish between empty lists and one-item
          lists that contain an empty string. Ideally, we'd have a
          @EmptyList() symbol but that would break compatibility
          with Qt 4.0. @Invalid() stands for QVariant(), and
          QVariant().toStringList() returns an empty QStringList,
          so we're in good shape.

          ### Qt5: Use a nicer syntax, e.g. @List, for variant lists
      */
      result += "@Invalid()";

   } else {
      for (int i = 0; i < strs.size(); ++i) {
         if (i != 0) {
            result += ", ";
         }
         iniEscapedString(strs.at(i), result, codec);
      }
   }
}

bool QSettingsPrivate::iniUnescapedStringList(const QByteArray &str, int from, int to,
      QString &stringResult, QStringList &stringListResult,
      QTextCodec *codec)
{
#ifdef QT_NO_TEXTCODE
   Q_UNUSED(codec);
#endif
   static const char escapeCodes[][2] = {
      { 'a', '\a' },
      { 'b', '\b' },
      { 'f', '\f' },
      { 'n', '\n' },
      { 'r', '\r' },
      { 't', '\t' },
      { 'v', '\v' },
      { '"', '"' },
      { '?', '?' },
      { '\'', '\'' },
      { '\\', '\\' }
   };
   static const int numEscapeCodes = sizeof(escapeCodes) / sizeof(escapeCodes[0]);

   bool isStringList = false;
   bool inQuotedString = false;
   bool currentValueIsQuoted = false;
   int escapeVal = 0;
   int i = from;
   char ch;

StSkipSpaces:
   while (i < to && ((ch = str.at(i)) == ' ' || ch == '\t')) {
      ++i;
   }
   // fallthrough

StNormal:
   while (i < to) {
      switch (str.at(i)) {
         case '\\':
            ++i;
            if (i >= to) {
               goto end;
            }

            ch = str.at(i++);
            for (int j = 0; j < numEscapeCodes; ++j) {
               if (ch == escapeCodes[j][0]) {
                  stringResult += QLatin1Char(escapeCodes[j][1]);
                  goto StNormal;
               }
            }

            if (ch == 'x') {
               escapeVal = 0;

               if (i >= to) {
                  goto end;
               }

               ch = str.at(i);
               if ((ch >= '0' && ch <= '9') || (ch >= 'A' && ch <= 'F') || (ch >= 'a' && ch <= 'f')) {
                  goto StHexEscape;
               }
            } else if (ch >= '0' && ch <= '7') {
               escapeVal = ch - '0';
               goto StOctEscape;
            } else if (ch == '\n' || ch == '\r') {
               if (i < to) {
                  char ch2 = str.at(i);
                  // \n, \r, \r\n, and \n\r are legitimate line terminators in INI files
                  if ((ch2 == '\n' || ch2 == '\r') && ch2 != ch) {
                     ++i;
                  }
               }
            } else {
               // the character is skipped
            }
            break;
         case '"':
            ++i;
            currentValueIsQuoted = true;
            inQuotedString = !inQuotedString;
            if (!inQuotedString) {
               goto StSkipSpaces;
            }
            break;
         case ',':
            if (!inQuotedString) {
               if (!currentValueIsQuoted) {
                  iniChopTrailingSpaces(stringResult);
               }
               if (!isStringList) {
                  isStringList = true;
                  stringListResult.clear();
                  stringResult.squeeze();
               }
               stringListResult.append(stringResult);
               stringResult.clear();
               currentValueIsQuoted = false;
               ++i;
               goto StSkipSpaces;
            }
         // fallthrough
         default: {
            int j = i + 1;
            while (j < to) {
               ch = str.at(j);
               if (ch == '\\' || ch == '"' || ch == ',') {
                  break;
               }
               ++j;
            }

#ifndef QT_NO_TEXTCODEC
            if (codec) {
               stringResult += codec->toUnicode(str.constData() + i, j - i);
            } else
#endif
            {
               int n = stringResult.size();
               stringResult.resize(n + (j - i));
               QChar *resultData = stringResult.data() + n;
               for (int k = i; k < j; ++k) {
                  *resultData++ = QLatin1Char(str.at(k));
               }
            }
            i = j;
         }
      }
   }
   goto end;

StHexEscape:
   if (i >= to) {
      stringResult += QChar(escapeVal);
      goto end;
   }

   ch = str.at(i);
   if (ch >= 'a') {
      ch -= 'a' - 'A';
   }
   if ((ch >= '0' && ch <= '9') || (ch >= 'A' && ch <= 'F')) {
      escapeVal <<= 4;
      escapeVal += strchr(hexDigits, ch) - hexDigits;
      ++i;
      goto StHexEscape;
   } else {
      stringResult += QChar(escapeVal);
      goto StNormal;
   }

StOctEscape:
   if (i >= to) {
      stringResult += QChar(escapeVal);
      goto end;
   }

   ch = str.at(i);
   if (ch >= '0' && ch <= '7') {
      escapeVal <<= 3;
      escapeVal += ch - '0';
      ++i;
      goto StOctEscape;
   } else {
      stringResult += QChar(escapeVal);
      goto StNormal;
   }

end:
   if (!currentValueIsQuoted) {
      iniChopTrailingSpaces(stringResult);
   }
   if (isStringList) {
      stringListResult.append(stringResult);
   }
   return isStringList;
}

QStringList QSettingsPrivate::splitArgs(const QString &s, int idx)
{
   int l = s.length();
   Q_ASSERT(l > 0);
   Q_ASSERT(s.at(idx) == QLatin1Char('('));
   Q_ASSERT(s.at(l - 1) == QLatin1Char(')'));

   QStringList result;
   QString item;

   for (++idx; idx < l; ++idx) {
      QChar c = s.at(idx);
      if (c == QLatin1Char(')')) {
         Q_ASSERT(idx == l - 1);
         result.append(item);
      } else if (c == QLatin1Char(' ')) {
         result.append(item);
         item.clear();
      } else {
         item.append(c);
      }
   }

   return result;
}

// ************************************************************************
// QConfFileSettingsPrivate

void QConfFileSettingsPrivate::initFormat()
{
   extension = (format == QSettings::NativeFormat) ? QLatin1String(".conf") : QLatin1String(".ini");
   readFunc = 0;
   writeFunc = 0;
#if defined(Q_OS_MAC)
   caseSensitivity = (format == QSettings::NativeFormat) ? Qt::CaseSensitive : IniCaseSensitivity;
#else
   caseSensitivity = IniCaseSensitivity;
#endif

   if (format > QSettings::IniFormat) {
      QMutexLocker locker(globalMutex());
      const CustomFormatVector *customFormatVector = customFormatVectorFunc();

      int i = (int)format - (int)QSettings::CustomFormat1;
      if (i >= 0 && i < customFormatVector->size()) {
         QConfFileCustomFormat info = customFormatVector->at(i);
         extension = info.extension;
         readFunc = info.readFunc;
         writeFunc = info.writeFunc;
         caseSensitivity = info.caseSensitivity;
      }
   }
}

void QConfFileSettingsPrivate::initAccess()
{
   if (confFiles[spec]) {
      if (format > QSettings::IniFormat) {
         if (!readFunc) {
            setStatus(QSettings::AccessError);
         }
      }
   }

   sync();       // loads the files the first time
}

#ifdef Q_OS_WIN
static QString windowsConfigPath(int type)
{
   QString result;

   QSystemLibrary library(QLatin1String("shell32"));

   typedef BOOL (WINAPI * GetSpecialFolderPath)(HWND, LPWSTR, int, BOOL);
   GetSpecialFolderPath SHGetSpecialFolderPath = (GetSpecialFolderPath)library.resolve("SHGetSpecialFolderPathW");
   if (SHGetSpecialFolderPath) {
      wchar_t path[MAX_PATH];
      SHGetSpecialFolderPath(0, path, type, FALSE);
      result = QString::fromWCharArray(path);
   }

   if (result.isEmpty()) {
      switch (type) {


         case CSIDL_COMMON_APPDATA:
            result = QLatin1String("C:\\temp\\qt-common");
            break;
         case CSIDL_APPDATA:
            result = QLatin1String("C:\\temp\\qt-user");
            break;
         default:
            ;
      }
   }

   return result;
}
#endif // Q_OS_WIN

static inline int pathHashKey(QSettings::Format format, QSettings::Scope scope)
{
   return int((uint(format) << 1) | uint(scope == QSettings::SystemScope));
}

static void initDefaultPaths(QMutexLocker *locker)
{
   PathHash *pathHash = pathHashFunc();
   QString homePath = QDir::homePath();
   QString systemPath;

   locker->unlock();

   /*
      QLibraryInfo::location() uses QSettings, so in order to
      avoid a dead-lock, we can't hold the global mutex while
      calling it.
   */
   systemPath = QLibraryInfo::location(QLibraryInfo::SettingsPath);
   systemPath += QLatin1Char('/');

   locker->relock();
   if (pathHash->isEmpty()) {
      /*
         Lazy initialization of pathHash. We initialize the
         IniFormat paths and (on Unix) the NativeFormat paths.
         (The NativeFormat paths are not configurable for the
         Windows registry and the Mac CFPreferences.)
      */
#ifdef Q_OS_WIN
      pathHash->insert(pathHashKey(QSettings::IniFormat, QSettings::UserScope),
                       windowsConfigPath(CSIDL_APPDATA) + QDir::separator());
      pathHash->insert(pathHashKey(QSettings::IniFormat, QSettings::SystemScope),
                       windowsConfigPath(CSIDL_COMMON_APPDATA) + QDir::separator());
#else
      QString userPath;
      char *env = getenv("XDG_CONFIG_HOME");
      if (env == 0) {
         userPath = homePath;
         userPath += QLatin1Char('/');
#if defined(Q_WS_QWS) || defined(Q_WS_QPA)
         userPath += QLatin1String("Settings");
#else
         userPath += QLatin1String(".config");
#endif
      } else if (*env == '/') {
         userPath = QFile::decodeName(env);
      } else {
         userPath = homePath;
         userPath += QLatin1Char('/');
         userPath += QFile::decodeName(env);
      }
      userPath += QLatin1Char('/');

      pathHash->insert(pathHashKey(QSettings::IniFormat, QSettings::UserScope), userPath);
      pathHash->insert(pathHashKey(QSettings::IniFormat, QSettings::SystemScope), systemPath);
#ifndef Q_OS_MAC
      pathHash->insert(pathHashKey(QSettings::NativeFormat, QSettings::UserScope), userPath);
      pathHash->insert(pathHashKey(QSettings::NativeFormat, QSettings::SystemScope), systemPath);
#endif
#endif
   }
}

static QString getPath(QSettings::Format format, QSettings::Scope scope)
{
   Q_ASSERT((int)QSettings::NativeFormat == 0);
   Q_ASSERT((int)QSettings::IniFormat == 1);

   QMutexLocker locker(globalMutex());
   PathHash *pathHash = pathHashFunc();
   if (pathHash->isEmpty()) {
      initDefaultPaths(&locker);
   }

   QString result = pathHash->value(pathHashKey(format, scope));
   if (!result.isEmpty()) {
      return result;
   }

   // fall back on INI path
   return pathHash->value(pathHashKey(QSettings::IniFormat, scope));
}

QConfFileSettingsPrivate::QConfFileSettingsPrivate(QSettings::Format format,
      QSettings::Scope scope,
      const QString &organization,
      const QString &application)
   : QSettingsPrivate(format, scope, organization, application),
     nextPosition(0x40000000) // big positive number
{
   int i;
   initFormat();

   QString org = organization;
   if (org.isEmpty()) {
      setStatus(QSettings::AccessError);
      org = QLatin1String("Unknown Organization");
   }

   QString appFile = org + QDir::separator() + application + extension;
   QString orgFile = org + extension;

   if (scope == QSettings::UserScope) {
      QString userPath = getPath(format, QSettings::UserScope);
      if (!application.isEmpty()) {
         confFiles[F_User | F_Application].reset(QConfFile::fromName(userPath + appFile, true));
      }
      confFiles[F_User | F_Organization].reset(QConfFile::fromName(userPath + orgFile, true));
   }

   QString systemPath = getPath(format, QSettings::SystemScope);
   if (!application.isEmpty()) {
      confFiles[F_System | F_Application].reset(QConfFile::fromName(systemPath + appFile, false));
   }
   confFiles[F_System | F_Organization].reset(QConfFile::fromName(systemPath + orgFile, false));

   for (i = 0; i < NumConfFiles; ++i) {
      if (confFiles[i]) {
         spec = i;
         break;
      }
   }

   initAccess();
}

QConfFileSettingsPrivate::QConfFileSettingsPrivate(const QString &fileName,
      QSettings::Format format)
   : QSettingsPrivate(format),
     nextPosition(0x40000000) // big positive number
{
   initFormat();

   confFiles[0].reset(QConfFile::fromName(fileName, true));

   initAccess();
}

QConfFileSettingsPrivate::~QConfFileSettingsPrivate()
{
   QMutexLocker locker(globalMutex());
   ConfFileHash *usedHash = usedHashFunc();
   ConfFileCache *unusedCache = unusedCacheFunc();

   for (int i = 0; i < NumConfFiles; ++i) {
      if (confFiles[i] && !confFiles[i]->ref.deref()) {
         if (confFiles[i]->size == 0) {
            delete confFiles[i].take();
         } else {
            if (usedHash) {
               usedHash->remove(confFiles[i]->name);
            }
            if (unusedCache) {
               QT_TRY {
                  // compute a better size?
                  unusedCache->insert(confFiles[i]->name, confFiles[i].data(),
                  10 + (confFiles[i]->originalKeys.size() / 4));
                  confFiles[i].take();
               } QT_CATCH(...) {
                  // out of memory. Do not cache the file.
                  delete confFiles[i].take();
               }
            } else {
               // unusedCache is gone - delete the entry to prevent a memory leak
               delete confFiles[i].take();
            }
         }
      }
      // prevent the ScopedPointer to deref it again.
      confFiles[i].take();
   }
}

void QConfFileSettingsPrivate::remove(const QString &key)
{
   QConfFile *confFile = confFiles[spec].data();
   if (!confFile) {
      return;
   }

   QSettingsKey theKey(key, caseSensitivity);
   QSettingsKey prefix(key + QLatin1Char('/'), caseSensitivity);
   QMutexLocker locker(&confFile->mutex);

   ensureSectionParsed(confFile, theKey);
   ensureSectionParsed(confFile, prefix);

   ParsedSettingsMap::iterator i = confFile->addedKeys.lowerBound(prefix);
   while (i != confFile->addedKeys.end() && i.key().startsWith(prefix)) {
      i = confFile->addedKeys.erase(i);
   }
   confFile->addedKeys.remove(theKey);

   ParsedSettingsMap::const_iterator j = const_cast<const ParsedSettingsMap *>(&confFile->originalKeys)->lowerBound(
         prefix);
   while (j != confFile->originalKeys.constEnd() && j.key().startsWith(prefix)) {
      confFile->removedKeys.insert(j.key(), QVariant());
      ++j;
   }
   if (confFile->originalKeys.contains(theKey)) {
      confFile->removedKeys.insert(theKey, QVariant());
   }
}

void QConfFileSettingsPrivate::set(const QString &key, const QVariant &value)
{
   QConfFile *confFile = confFiles[spec].data();
   if (!confFile) {
      return;
   }

   QSettingsKey theKey(key, caseSensitivity, nextPosition++);
   QMutexLocker locker(&confFile->mutex);
   confFile->removedKeys.remove(theKey);
   confFile->addedKeys.insert(theKey, value);
}

bool QConfFileSettingsPrivate::get(const QString &key, QVariant *value) const
{
   QSettingsKey theKey(key, caseSensitivity);
   ParsedSettingsMap::const_iterator j;
   bool found = false;

   for (int i = 0; i < NumConfFiles; ++i) {
      if (QConfFile *confFile = confFiles[i].data()) {
         QMutexLocker locker(&confFile->mutex);

         if (!confFile->addedKeys.isEmpty()) {
            j = confFile->addedKeys.constFind(theKey);
            found = (j != confFile->addedKeys.constEnd());
         }
         if (!found) {
            ensureSectionParsed(confFile, theKey);
            j = confFile->originalKeys.constFind(theKey);
            found = (j != confFile->originalKeys.constEnd()
                     && !confFile->removedKeys.contains(theKey));
         }

         if (found && value) {
            *value = *j;
         }

         if (found) {
            return true;
         }
         if (!fallbacks) {
            break;
         }
      }
   }
   return false;
}

QStringList QConfFileSettingsPrivate::children(const QString &prefix, ChildSpec spec) const
{
   QMap<QString, QString> result;
   ParsedSettingsMap::const_iterator j;

   QSettingsKey thePrefix(prefix, caseSensitivity);
   int startPos = prefix.size();

   for (int i = 0; i < NumConfFiles; ++i) {
      if (QConfFile *confFile = confFiles[i].data()) {
         QMutexLocker locker(&confFile->mutex);

         if (thePrefix.isEmpty()) {
            ensureAllSectionsParsed(confFile);
         } else {
            ensureSectionParsed(confFile, thePrefix);
         }

         j = const_cast<const ParsedSettingsMap *>(
                &confFile->originalKeys)->lowerBound( thePrefix);
         while (j != confFile->originalKeys.constEnd() && j.key().startsWith(thePrefix)) {
            if (!confFile->removedKeys.contains(j.key())) {
               processChild(j.key().originalCaseKey().mid(startPos), spec, result);
            }
            ++j;
         }

         j = const_cast<const ParsedSettingsMap *>(
                &confFile->addedKeys)->lowerBound(thePrefix);
         while (j != confFile->addedKeys.constEnd() && j.key().startsWith(thePrefix)) {
            processChild(j.key().originalCaseKey().mid(startPos), spec, result);
            ++j;
         }

         if (!fallbacks) {
            break;
         }
      }
   }
   return result.keys();
}

void QConfFileSettingsPrivate::clear()
{
   QConfFile *confFile = confFiles[spec].data();
   if (!confFile) {
      return;
   }

   QMutexLocker locker(&confFile->mutex);
   ensureAllSectionsParsed(confFile);
   confFile->addedKeys.clear();
   confFile->removedKeys = confFile->originalKeys;
}

void QConfFileSettingsPrivate::sync()
{
   // people probably won't be checking the status a whole lot, so in case of
   // error we just try to go on and make the best of it

   for (int i = 0; i < NumConfFiles; ++i) {
      QConfFile *confFile = confFiles[i].data();
      if (confFile) {
         QMutexLocker locker(&confFile->mutex);
         syncConfFile(i);
      }
   }
}

void QConfFileSettingsPrivate::flush()
{
   sync();
}

QString QConfFileSettingsPrivate::fileName() const
{
   QConfFile *confFile = confFiles[spec].data();
   if (!confFile) {
      return QString();
   }
   return confFile->name;
}

bool QConfFileSettingsPrivate::isWritable() const
{
   if (format > QSettings::IniFormat && !writeFunc) {
      return false;
   }

   QConfFile *confFile = confFiles[spec].data();
   if (!confFile) {
      return false;
   }

   return confFile->isWritable();
}

void QConfFileSettingsPrivate::syncConfFile(int confFileNo)
{
   QConfFile *confFile = confFiles[confFileNo].data();
   bool readOnly = confFile->addedKeys.isEmpty() && confFile->removedKeys.isEmpty();
   bool ok;

   /*
       We can often optimize the read-only case, if the file on disk
       hasn't changed.
   */
   if (readOnly && confFile->size > 0) {
      QFileInfo fileInfo(confFile->name);
      if (confFile->size == fileInfo.size() && confFile->timeStamp == fileInfo.lastModified()) {
         return;
      }
   }

   /*
       Open the configuration file and try to use it using a named
       semaphore on Windows and an advisory lock on Unix-based
       systems. This protect us against other QSettings instances
       trying to access the same file from other threads or
       processes.

       As it stands now, the locking mechanism doesn't work for
       .plist files.
   */
   QFile file(confFile->name);
   bool createFile = !file.exists();
   if ((!readOnly && confFile->isWritable()) || createFile) {
      file.open(QFile::ReadWrite);
   }
   if (!file.isOpen()) {
      file.open(QFile::ReadOnly);
   }

   if (!createFile && !file.isOpen()) {
      setStatus(QSettings::AccessError);
   }

#ifdef Q_OS_WIN
   HANDLE readSemaphore = 0;
   HANDLE writeSemaphore = 0;
   static const int FileLockSemMax = 50;
   int numReadLocks = readOnly ? 1 : FileLockSemMax;

   if (file.isOpen()) {
      // Acquire the write lock if we will be writing
      if (!readOnly) {
         QString writeSemName = QLatin1String("QSettingsWriteSem ");
         writeSemName.append(file.fileName());

         writeSemaphore = CreateSemaphore(0, 1, 1, reinterpret_cast<const wchar_t *>(writeSemName.utf16()));

         if (writeSemaphore) {
            WaitForSingleObject(writeSemaphore, INFINITE);
         } else {
            setStatus(QSettings::AccessError);
            return;
         }
      }

      // Acquire all the read locks if we will be writing, to make sure nobody
      // reads while we're writing. If we are only reading, acquire a single
      // read lock.
      QString readSemName(QLatin1String("QSettingsReadSem "));
      readSemName.append(file.fileName());

      readSemaphore = CreateSemaphore(0, FileLockSemMax, FileLockSemMax,
                                      reinterpret_cast<const wchar_t *>(readSemName.utf16()));

      if (readSemaphore) {
         for (int i = 0; i < numReadLocks; ++i) {
            WaitForSingleObject(readSemaphore, INFINITE);
         }
      } else {
         setStatus(QSettings::AccessError);
         if (writeSemaphore != 0) {
            ReleaseSemaphore(writeSemaphore, 1, 0);
            CloseHandle(writeSemaphore);
         }
         return;
      }
   }
#else
   if (file.isOpen()) {
      unixLock(file.handle(), readOnly ? F_RDLCK : F_WRLCK);
   }
#endif

   // If we have created the file, apply the file perms
   if (file.isOpen()) {
      if (createFile) {
         QFile::Permissions perms = file.permissions() | QFile::ReadOwner | QFile::WriteOwner;
         if (!confFile->userPerms) {
            perms |= QFile::ReadGroup | QFile::ReadOther;
         }
         file.setPermissions(perms);
      }
   }

   /*
       We hold the lock. Let's reread the file if it has changed
       since last time we read it.
   */
   QFileInfo fileInfo(confFile->name);
   bool mustReadFile = true;

   if (!readOnly)
      mustReadFile = (confFile->size != fileInfo.size()
                      || (confFile->size != 0 && confFile->timeStamp != fileInfo.lastModified()));

   if (mustReadFile) {
      confFile->unparsedIniSections.clear();
      confFile->originalKeys.clear();

      /*
          Files that we can't read (because of permissions or
          because they don't exist) are treated as empty files.
      */
      if (file.isReadable() && fileInfo.size() != 0) {
#ifdef Q_OS_MAC
         if (format == QSettings::NativeFormat) {
            ok = readPlistFile(confFile->name, &confFile->originalKeys);
         } else
#endif
         {
            if (format <= QSettings::IniFormat) {
               QByteArray data = file.readAll();
               ok = readIniFile(data, &confFile->unparsedIniSections);
            } else {
               if (readFunc) {
                  QSettings::SettingsMap tempNewKeys;
                  ok = readFunc(file, tempNewKeys);

                  if (ok) {
                     QSettings::SettingsMap::const_iterator i = tempNewKeys.constBegin();
                     while (i != tempNewKeys.constEnd()) {
                        confFile->originalKeys.insert(QSettingsKey(i.key(),
                                                      caseSensitivity),
                                                      i.value());
                        ++i;
                     }
                  }
               } else {
                  ok = false;
               }
            }
         }

         if (!ok) {
            setStatus(QSettings::FormatError);
         }
      }

      confFile->size = fileInfo.size();
      confFile->timeStamp = fileInfo.lastModified();
   }

   /*
       We also need to save the file. We still hold the file lock,
       so everything is under control.
   */
   if (!readOnly) {
      ensureAllSectionsParsed(confFile);
      ParsedSettingsMap mergedKeys = confFile->mergedKeyMap();

      if (file.isWritable()) {
#ifdef Q_OS_MAC
         if (format == QSettings::NativeFormat) {
            ok = writePlistFile(confFile->name, mergedKeys);
         } else
#endif
         {
            file.seek(0);
            file.resize(0);

            if (format <= QSettings::IniFormat) {
               ok = writeIniFile(file, mergedKeys);
               if (!ok) {
                  // try to restore old data; might work if the disk was full and the new data
                  // was larger than the old data
                  file.seek(0);
                  file.resize(0);
                  writeIniFile(file, confFile->originalKeys);
               }
            } else {
               if (writeFunc) {
                  QSettings::SettingsMap tempOriginalKeys;

                  ParsedSettingsMap::const_iterator i = mergedKeys.constBegin();
                  while (i != mergedKeys.constEnd()) {
                     tempOriginalKeys.insert(i.key(), i.value());
                     ++i;
                  }
                  ok = writeFunc(file, tempOriginalKeys);
               } else {
                  ok = false;
               }
            }
         }
      } else {
         ok = false;
      }

      if (ok) {
         confFile->unparsedIniSections.clear();
         confFile->originalKeys = mergedKeys;
         confFile->addedKeys.clear();
         confFile->removedKeys.clear();

         QFileInfo fileInfo(confFile->name);
         confFile->size = fileInfo.size();
         confFile->timeStamp = fileInfo.lastModified();
      } else {
         setStatus(QSettings::AccessError);
      }
   }

   /*
       Release the file lock.
   */
#ifdef Q_OS_WIN
   if (readSemaphore != 0) {
      ReleaseSemaphore(readSemaphore, numReadLocks, 0);
      CloseHandle(readSemaphore);
   }
   if (writeSemaphore != 0) {
      ReleaseSemaphore(writeSemaphore, 1, 0);
      CloseHandle(writeSemaphore);
   }
#endif
}

enum { Space = 0x1, Special = 0x2 };

static const char charTraits[256] = {
   // Space: '\t', '\n', '\r', ' '
   // Special: '\n', '\r', '"', ';', '=', '\\'

   0, 0, 0, 0, 0, 0, 0, 0, 0, Space, Space | Special, 0, 0, Space | Special, 0, 0,
   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
   Space, 0, Special, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, Special, 0, Special, 0, 0,
   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, Special, 0, 0, 0,
   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,

   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

bool QConfFileSettingsPrivate::readIniLine(const QByteArray &data, int &dataPos,
      int &lineStart, int &lineLen, int &equalsPos)
{
   int dataLen = data.length();
   bool inQuotes = false;

   equalsPos = -1;

   lineStart = dataPos;
   while (lineStart < dataLen && (charTraits[uint(uchar(data.at(lineStart)))] & Space)) {
      ++lineStart;
   }

   int i = lineStart;
   while (i < dataLen) {
      while (!(charTraits[uint(uchar(data.at(i)))] & Special)) {
         if (++i == dataLen) {
            goto break_out_of_outer_loop;
         }
      }

      char ch = data.at(i++);
      if (ch == '=') {
         if (!inQuotes && equalsPos == -1) {
            equalsPos = i - 1;
         }
      } else if (ch == '\n' || ch == '\r') {
         if (i == lineStart + 1) {
            ++lineStart;
         } else if (!inQuotes) {
            --i;
            goto break_out_of_outer_loop;
         }
      } else if (ch == '\\') {
         if (i < dataLen) {
            char ch = data.at(i++);
            if (i < dataLen) {
               char ch2 = data.at(i);
               // \n, \r, \r\n, and \n\r are legitimate line terminators in INI files
               if ((ch == '\n' && ch2 == '\r') || (ch == '\r' && ch2 == '\n')) {
                  ++i;
               }
            }
         }
      } else if (ch == '"') {
         inQuotes = !inQuotes;
      } else {
         Q_ASSERT(ch == ';');

         if (i == lineStart + 1) {
            char ch;
            while (i < dataLen && ((ch = data.at(i) != '\n') && ch != '\r')) {
               ++i;
            }
            lineStart = i;
         } else if (!inQuotes) {
            --i;
            goto break_out_of_outer_loop;
         }
      }
   }

break_out_of_outer_loop:
   dataPos = i;
   lineLen = i - lineStart;
   return lineLen > 0;
}

/*
    Returns false on parse error. However, as many keys are read as
    possible, so if the user doesn't check the status he will get the
    most out of the file anyway.
*/
bool QConfFileSettingsPrivate::readIniFile(const QByteArray &data,
      UnparsedSettingsMap *unparsedIniSections)
{
#define FLUSH_CURRENT_SECTION() \
    { \
        QByteArray &sectionData = (*unparsedIniSections)[QSettingsKey(currentSection, \
                                                                      IniCaseSensitivity, \
                                                                      sectionPosition)]; \
        if (!sectionData.isEmpty()) \
            sectionData.append('\n'); \
        sectionData += data.mid(currentSectionStart, lineStart - currentSectionStart); \
        sectionPosition = ++position; \
    }

   QString currentSection;
   int currentSectionStart = 0;
   int dataPos = 0;
   int lineStart;
   int lineLen;
   int equalsPos;
   int position = 0;
   int sectionPosition = 0;
   bool ok = true;

   while (readIniLine(data, dataPos, lineStart, lineLen, equalsPos)) {
      char ch = data.at(lineStart);
      if (ch == '[') {
         FLUSH_CURRENT_SECTION();

         // this is a section
         QByteArray iniSection;
         int idx = data.indexOf(']', lineStart);
         if (idx == -1 || idx >= lineStart + lineLen) {
            ok = false;
            iniSection = data.mid(lineStart + 1, lineLen - 1);
         } else {
            iniSection = data.mid(lineStart + 1, idx - lineStart - 1);
         }

         iniSection = iniSection.trimmed();

         if (qstricmp(iniSection.constData(), "general") == 0) {
            currentSection.clear();
         } else {
            if (qstricmp(iniSection.constData(), "%general") == 0) {
               currentSection = QLatin1String(iniSection.constData() + 1);
            } else {
               currentSection.clear();
               iniUnescapedKey(iniSection, 0, iniSection.size(), currentSection);
            }
            currentSection += QLatin1Char('/');
         }
         currentSectionStart = dataPos;
      }
      ++position;
   }

   Q_ASSERT(lineStart == data.length());
   FLUSH_CURRENT_SECTION();

   return ok;

#undef FLUSH_CURRENT_SECTION
}

bool QConfFileSettingsPrivate::readIniSection(const QSettingsKey &section, const QByteArray &data,
      ParsedSettingsMap *settingsMap, QTextCodec *codec)
{
   QStringList strListValue;
   bool sectionIsLowercase = (section == section.originalCaseKey());
   int equalsPos;

   bool ok = true;
   int dataPos = 0;
   int lineStart;
   int lineLen;
   int position = section.originalKeyPosition();

   while (readIniLine(data, dataPos, lineStart, lineLen, equalsPos)) {
      char ch = data.at(lineStart);
      Q_ASSERT(ch != '[');

      if (equalsPos == -1) {
         if (ch != ';') {
            ok = false;
         }
         continue;
      }

      int keyEnd = equalsPos;
      while (keyEnd > lineStart && ((ch = data.at(keyEnd - 1)) == ' ' || ch == '\t')) {
         --keyEnd;
      }
      int valueStart = equalsPos + 1;

      QString key = section.originalCaseKey();
      bool keyIsLowercase = (iniUnescapedKey(data, lineStart, keyEnd, key) && sectionIsLowercase);

      QString strValue;
      strValue.reserve(lineLen - (valueStart - lineStart));
      bool isStringList = iniUnescapedStringList(data, valueStart, lineStart + lineLen,
                          strValue, strListValue, codec);
      QVariant variant;
      if (isStringList) {
         variant = stringListToVariantList(strListValue);
      } else {
         variant = stringToVariant(strValue);
      }

      /*
          We try to avoid the expensive toLower() call in
          QSettingsKey by passing Qt::CaseSensitive when the
          key is already in lowercase.
      */
      settingsMap->insert(QSettingsKey(key, keyIsLowercase ? Qt::CaseSensitive
                                       : IniCaseSensitivity,
                                       position),
                          variant);
      ++position;
   }

   return ok;
}

class QSettingsIniKey : public QString
{
 public:
   inline QSettingsIniKey() : position(-1) {}
   inline QSettingsIniKey(const QString &str, int pos = -1) : QString(str), position(pos) {}

   int position;
};

static bool operator<(const QSettingsIniKey &k1, const QSettingsIniKey &k2)
{
   if (k1.position != k2.position) {
      return k1.position < k2.position;
   }
   return static_cast<const QString &>(k1) < static_cast<const QString &>(k2);
}

typedef QMap<QSettingsIniKey, QVariant> IniKeyMap;

struct QSettingsIniSection {
   int position;
   IniKeyMap keyMap;

   inline QSettingsIniSection() : position(-1) {}
};

typedef QMap<QString, QSettingsIniSection> IniMap;

/*
    This would be more straightforward if we didn't try to remember the original
    key order in the .ini file, but we do.
*/
bool QConfFileSettingsPrivate::writeIniFile(QIODevice &device, const ParsedSettingsMap &map)
{
   IniMap iniMap;
   IniMap::const_iterator i;

#ifdef Q_OS_WIN
   const char *const eol = "\r\n";
#else
   const char eol = '\n';
#endif

   for (ParsedSettingsMap::const_iterator j = map.constBegin(); j != map.constEnd(); ++j) {
      QString section;
      QSettingsIniKey key(j.key().originalCaseKey(), j.key().originalKeyPosition());
      int slashPos;

      if ((slashPos = key.indexOf(QLatin1Char('/'))) != -1) {
         section = key.left(slashPos);
         key.remove(0, slashPos + 1);
      }

      QSettingsIniSection &iniSection = iniMap[section];

      // -1 means infinity
      if (uint(key.position) < uint(iniSection.position)) {
         iniSection.position = key.position;
      }
      iniSection.keyMap[key] = j.value();
   }

   const int sectionCount = iniMap.size();
   QVector<QSettingsIniKey> sections;
   sections.reserve(sectionCount);
   for (i = iniMap.constBegin(); i != iniMap.constEnd(); ++i) {
      sections.append(QSettingsIniKey(i.key(), i.value().position));
   }
   std::sort(sections.begin(), sections.end());

   bool writeError = false;
   for (int j = 0; !writeError && j < sectionCount; ++j) {
      i = iniMap.constFind(sections.at(j));
      Q_ASSERT(i != iniMap.constEnd());

      QByteArray realSection;

      iniEscapedKey(i.key(), realSection);

      if (realSection.isEmpty()) {
         realSection = "[General]";
      } else if (qstricmp(realSection.constData(), "general") == 0) {
         realSection = "[%General]";
      } else {
         realSection.prepend('[');
         realSection.append(']');
      }

      if (j != 0) {
         realSection.prepend(eol);
      }
      realSection += eol;

      device.write(realSection);

      const IniKeyMap &ents = i.value().keyMap;
      for (IniKeyMap::const_iterator j = ents.constBegin(); j != ents.constEnd(); ++j) {
         QByteArray block;
         iniEscapedKey(j.key(), block);
         block += '=';

         const QVariant &value = j.value();

         /*
             The size() != 1 trick is necessary because
             QVariant(QString("foo")).toList() returns an empty
             list, not a list containing "foo".
         */
         if (value.type() == QVariant::StringList
               || (value.type() == QVariant::List && value.toList().size() != 1)) {
            iniEscapedStringList(variantListToStringList(value.toList()), block, iniCodec);
         } else {
            iniEscapedString(variantToString(value), block, iniCodec);
         }
         block += eol;
         if (device.write(block) == -1) {
            writeError = true;
            break;
         }
      }
   }
   return !writeError;
}

void QConfFileSettingsPrivate::ensureAllSectionsParsed(QConfFile *confFile) const
{
   UnparsedSettingsMap::const_iterator i = confFile->unparsedIniSections.constBegin();
   const UnparsedSettingsMap::const_iterator end = confFile->unparsedIniSections.constEnd();

   for (; i != end; ++i) {
      if (!QConfFileSettingsPrivate::readIniSection(i.key(), i.value(), &confFile->originalKeys, iniCodec)) {
         setStatus(QSettings::FormatError);
      }
   }
   confFile->unparsedIniSections.clear();
}

void QConfFileSettingsPrivate::ensureSectionParsed(QConfFile *confFile,
      const QSettingsKey &key) const
{
   if (confFile->unparsedIniSections.isEmpty()) {
      return;
   }

   UnparsedSettingsMap::iterator i;

   int indexOfSlash = key.indexOf(QLatin1Char('/'));
   if (indexOfSlash != -1) {
      i = confFile->unparsedIniSections.upperBound(key);
      if (i == confFile->unparsedIniSections.begin()) {
         return;
      }
      --i;
      if (i.key().isEmpty() || !key.startsWith(i.key())) {
         return;
      }
   } else {
      i = confFile->unparsedIniSections.begin();
      if (i == confFile->unparsedIniSections.end() || !i.key().isEmpty()) {
         return;
      }
   }

   if (!QConfFileSettingsPrivate::readIniSection(i.key(), i.value(), &confFile->originalKeys, iniCodec)) {
      setStatus(QSettings::FormatError);
   }
   confFile->unparsedIniSections.erase(i);
}

QSettings::QSettings(const QString &organization, const QString &application, QObject *parent)
   : QObject(parent), d_ptr(QSettingsPrivate::create(NativeFormat, UserScope, organization, application))
{
   d_ptr->q_ptr = this;
}

/*!
    Constructs a QSettings object for accessing settings of the
    application called \a application from the organization called \a
    organization, and with parent \a parent.

    If \a scope is QSettings::UserScope, the QSettings object searches
    user-specific settings first, before it searches system-wide
    settings as a fallback. If \a scope is QSettings::SystemScope, the
    QSettings object ignores user-specific settings and provides
    access to system-wide settings.

    The storage format is set to QSettings::NativeFormat (i.e. calling
    setDefaultFormat() before calling this constructor has no effect).

    If no application name is given, the QSettings object will
    only access the organization-wide \l{Fallback Mechanism}{locations}.

    \sa setDefaultFormat()
*/
QSettings::QSettings(Scope scope, const QString &organization, const QString &application, QObject *parent)
   : QObject(parent), d_ptr(QSettingsPrivate::create(NativeFormat, scope, organization, application))
{
   d_ptr->q_ptr = this;
}

/*!
    Constructs a QSettings object for accessing settings of the
    application called \a application from the organization called
    \a organization, and with parent \a parent.

    If \a scope is QSettings::UserScope, the QSettings object searches
    user-specific settings first, before it searches system-wide
    settings as a fallback. If \a scope is
    QSettings::SystemScope, the QSettings object ignores user-specific
    settings and provides access to system-wide settings.

    If \a format is QSettings::NativeFormat, the native API is used for
    storing settings. If \a format is QSettings::IniFormat, the INI format
    is used.

    If no application name is given, the QSettings object will
    only access the organization-wide \l{Fallback Mechanism}{locations}.
*/
QSettings::QSettings(Format format, Scope scope, const QString &organization,
                     const QString &application, QObject *parent)
   : QObject(parent), d_ptr(QSettingsPrivate::create(format, scope, organization, application))
{
   d_ptr->q_ptr = this;
}

/*!
    Constructs a QSettings object for accessing the settings
    stored in the file called \a fileName, with parent \a parent. If
    the file doesn't already exist, it is created.

    If \a format is QSettings::NativeFormat, the meaning of \a
    fileName depends on the platform. On Unix, \a fileName is the
    name of an INI file. On Mac OS X, \a fileName is the name of a
    \c .plist file. On Windows, \a fileName is a path in the system
    registry.

    If \a format is QSettings::IniFormat, \a fileName is the name of an INI
    file.

    \warning This function is provided for convenience. It works well for
    accessing INI or \c .plist files generated by Qt, but might fail on some
    syntaxes found in such files originated by other programs. In particular,
    be aware of the following limitations:

    \list
    \o QSettings provides no way of reading INI "path" entries, i.e., entries
       with unescaped slash characters. (This is because these entries are
       ambiguous and cannot be resolved automatically.)
    \o In INI files, QSettings uses the \c @ character as a metacharacter in some
       contexts, to encode Qt-specific data types (e.g., \c @Rect), and might
       therefore misinterpret it when it occurs in pure INI files.
    \endlist

    \sa fileName()
*/
QSettings::QSettings(const QString &fileName, Format format, QObject *parent)
   : QObject(parent), d_ptr(QSettingsPrivate::create(fileName, format))
{
   d_ptr->q_ptr = this;
}

/*!
    Constructs a QSettings object for accessing settings of the
    application and organization set previously with a call to
    QCoreApplication::setOrganizationName(),
    QCoreApplication::setOrganizationDomain(), and
    QCoreApplication::setApplicationName().

    The scope is QSettings::UserScope and the format is
    defaultFormat() (QSettings::NativeFormat by default).
    Use setDefaultFormat() before calling this constructor
    to change the default format used by this constructor.

    The code

    \snippet doc/src/snippets/code/src_corelib_io_qsettings.cpp 11

    is equivalent to

    \snippet doc/src/snippets/code/src_corelib_io_qsettings.cpp 12

    If QCoreApplication::setOrganizationName() and
    QCoreApplication::setApplicationName() has not been previously
    called, the QSettings object will not be able to read or write
    any settings, and status() will return AccessError.

    On Mac OS X, if both a name and an Internet domain are specified
    for the organization, the domain is preferred over the name. On
    other platforms, the name is preferred over the domain.

    \sa QCoreApplication::setOrganizationName(),
        QCoreApplication::setOrganizationDomain(),
        QCoreApplication::setApplicationName(),
        setDefaultFormat()
*/
QSettings::QSettings(QObject *parent)
   : QObject(parent), d_ptr(QSettingsPrivate::create(globalDefaultFormat, UserScope,

#ifdef Q_OS_MAC
                            QCoreApplication::organizationDomain().isEmpty() ? QCoreApplication::organizationName()
                            : QCoreApplication::organizationDomain(),
#else
                            QCoreApplication::organizationName().isEmpty() ? QCoreApplication::organizationDomain()
                            : QCoreApplication::organizationName(),
#endif

                            QCoreApplication::applicationName() ))
{
   d_ptr->q_ptr = this;
}


/*!
    Destroys the QSettings object.

    Any unsaved changes will eventually be written to permanent
    storage.

    \sa sync()
*/
QSettings::~QSettings()
{
   Q_D(QSettings);
   if (d->pendingChanges) {
      QT_TRY {
         d->flush();
      } QT_CATCH(...) {
         ; // ok. then don't flush but at least don't throw in the destructor
      }
   }
}

/*!
    Removes all entries in the primary location associated to this
    QSettings object.

    Entries in fallback locations are not removed.

    If you only want to remove the entries in the current group(),
    use remove("") instead.

    \sa remove(), setFallbacksEnabled()
*/
void QSettings::clear()
{
   Q_D(QSettings);
   d->clear();
   d->requestUpdate();
}

/*!
    Writes any unsaved changes to permanent storage, and reloads any
    settings that have been changed in the meantime by another
    application.

    This function is called automatically from QSettings's destructor and
    by the event loop at regular intervals, so you normally don't need to
    call it yourself.

    \sa status()
*/
void QSettings::sync()
{
   Q_D(QSettings);
   d->sync();
}

/*!
    Returns the path where settings written using this QSettings
    object are stored.

    On Windows, if the format is QSettings::NativeFormat, the return value
    is a system registry path, not a file path.

    \sa isWritable(), format()
*/
QString QSettings::fileName() const
{
   Q_D(const QSettings);
   return d->fileName();
}

/*!
    \since 4.4

    Returns the format used for storing the settings.

    \sa defaultFormat(), fileName(), scope(), organizationName(), applicationName()
*/
QSettings::Format QSettings::format() const
{
   Q_D(const QSettings);
   return d->format;
}

/*!
    \since 4.4

    Returns the scope used for storing the settings.

    \sa format(), organizationName(), applicationName()
*/
QSettings::Scope QSettings::scope() const
{
   Q_D(const QSettings);
   return d->scope;
}

/*!
    \since 4.4

    Returns the organization name used for storing the settings.

    \sa QCoreApplication::organizationName(), format(), scope(), applicationName()
*/
QString QSettings::organizationName() const
{
   Q_D(const QSettings);
   return d->organizationName;
}

/*!
    \since 4.4

    Returns the application name used for storing the settings.

    \sa QCoreApplication::applicationName(), format(), scope(), organizationName()
*/
QString QSettings::applicationName() const
{
   Q_D(const QSettings);
   return d->applicationName;
}

#ifndef QT_NO_TEXTCODEC

/*!
    \since 4.5

    Sets the codec for accessing INI files (including \c .conf files on Unix)
    to \a codec. The codec is used for decoding any data that is read from
    the INI file, and for encoding any data that is written to the file. By
    default, no codec is used, and non-ASCII characters are encoded using
    standard INI escape sequences.

    \warning The codec must be set immediately after creating the QSettings
    object, before accessing any data.

    \sa iniCodec()
*/
void QSettings::setIniCodec(QTextCodec *codec)
{
   Q_D(QSettings);
   d->iniCodec = codec;
}

/*!
    \since 4.5
    \overload

    Sets the codec for accessing INI files (including \c .conf files on Unix)
    to the QTextCodec for the encoding specified by \a codecName. Common
    values for \c codecName include "ISO 8859-1", "UTF-8", and "UTF-16".
    If the encoding isn't recognized, nothing happens.

    \sa QTextCodec::codecForName()
*/
void QSettings::setIniCodec(const char *codecName)
{
   Q_D(QSettings);
   if (QTextCodec *codec = QTextCodec::codecForName(codecName)) {
      d->iniCodec = codec;
   }
}

/*!
    \since 4.5

    Returns the codec that is used for accessing INI files. By default,
    no codec is used, so a null pointer is returned.
*/

QTextCodec *QSettings::iniCodec() const
{
   Q_D(const QSettings);
   return d->iniCodec;
}

#endif // QT_NO_TEXTCODEC

/*!
    Returns a status code indicating the first error that was met by
    QSettings, or QSettings::NoError if no error occurred.

    Be aware that QSettings delays performing some operations. For this
    reason, you might want to call sync() to ensure that the data stored
    in QSettings is written to disk before calling status().

    \sa sync()
*/
QSettings::Status QSettings::status() const
{
   Q_D(const QSettings);
   return d->status;
}

/*!
    Appends \a prefix to the current group.

    The current group is automatically prepended to all keys
    specified to QSettings. In addition, query functions such as
    childGroups(), childKeys(), and allKeys() are based on the group.
    By default, no group is set.

    Groups are useful to avoid typing in the same setting paths over
    and over. For example:

    \snippet doc/src/snippets/code/src_corelib_io_qsettings.cpp 13

    This will set the value of three settings:

    \list
    \o \c mainwindow/size
    \o \c mainwindow/fullScreen
    \o \c outputpanel/visible
    \endlist

    Call endGroup() to reset the current group to what it was before
    the corresponding beginGroup() call. Groups can be nested.

    \sa endGroup(), group()
*/
void QSettings::beginGroup(const QString &prefix)
{
   Q_D(QSettings);
   d->beginGroupOrArray(QSettingsGroup(d->normalizedKey(prefix)));
}

/*!
    Resets the group to what it was before the corresponding
    beginGroup() call.

    Example:

    \snippet doc/src/snippets/code/src_corelib_io_qsettings.cpp 14

    \sa beginGroup(), group()
*/
void QSettings::endGroup()
{
   Q_D(QSettings);
   if (d->groupStack.isEmpty()) {
      qWarning("QSettings::endGroup: No matching beginGroup()");
      return;
   }

   QSettingsGroup group = d->groupStack.pop();
   int len = group.toString().size();
   if (len > 0) {
      d->groupPrefix.truncate(d->groupPrefix.size() - (len + 1));
   }

   if (group.isArray()) {
      qWarning("QSettings::endGroup: Expected endArray() instead");
   }
}

/*!
    Returns the current group.

    \sa beginGroup(), endGroup()
*/
QString QSettings::group() const
{
   Q_D(const QSettings);
   return d->groupPrefix.left(d->groupPrefix.size() - 1);
}

/*!
    Adds \a prefix to the current group and starts reading from an
    array. Returns the size of the array.

    Example:

    \snippet doc/src/snippets/code/src_corelib_io_qsettings.cpp 15

    Use beginWriteArray() to write the array in the first place.

    \sa beginWriteArray(), endArray(), setArrayIndex()
*/
int QSettings::beginReadArray(const QString &prefix)
{
   Q_D(QSettings);
   d->beginGroupOrArray(QSettingsGroup(d->normalizedKey(prefix), false));
   return value(QLatin1String("size")).toInt();
}

/*!
    Adds \a prefix to the current group and starts writing an array
    of size \a size. If \a size is -1 (the default), it is automatically
    determined based on the indexes of the entries written.

    If you have many occurrences of a certain set of keys, you can
    use arrays to make your life easier. For example, let's suppose
    that you want to save a variable-length list of user names and
    passwords. You could then write:

    \snippet doc/src/snippets/code/src_corelib_io_qsettings.cpp 16

    The generated keys will have the form

    \list
    \o \c logins/size
    \o \c logins/1/userName
    \o \c logins/1/password
    \o \c logins/2/userName
    \o \c logins/2/password
    \o \c logins/3/userName
    \o \c logins/3/password
    \o ...
    \endlist

    To read back an array, use beginReadArray().

    \sa beginReadArray(), endArray(), setArrayIndex()
*/
void QSettings::beginWriteArray(const QString &prefix, int size)
{
   Q_D(QSettings);
   d->beginGroupOrArray(QSettingsGroup(d->normalizedKey(prefix), size < 0));

   if (size < 0) {
      remove(QLatin1String("size"));
   } else {
      setValue(QLatin1String("size"), size);
   }
}

/*!
    Closes the array that was started using beginReadArray() or
    beginWriteArray().

    \sa beginReadArray(), beginWriteArray()
*/
void QSettings::endArray()
{
   Q_D(QSettings);
   if (d->groupStack.isEmpty()) {
      qWarning("QSettings::endArray: No matching beginArray()");
      return;
   }

   QSettingsGroup group = d->groupStack.top();
   int len = group.toString().size();
   d->groupStack.pop();
   if (len > 0) {
      d->groupPrefix.truncate(d->groupPrefix.size() - (len + 1));
   }

   if (group.arraySizeGuess() != -1) {
      setValue(group.name() + QLatin1String("/size"), group.arraySizeGuess());
   }

   if (!group.isArray()) {
      qWarning("QSettings::endArray: Expected endGroup() instead");
   }
}

/*!
    Sets the current array index to \a i. Calls to functions such as
    setValue(), value(), remove(), and contains() will operate on the
    array entry at that index.

    You must call beginReadArray() or beginWriteArray() before you
    can call this function.
*/
void QSettings::setArrayIndex(int i)
{
   Q_D(QSettings);
   if (d->groupStack.isEmpty() || !d->groupStack.top().isArray()) {
      qWarning("QSettings::setArrayIndex: Missing beginArray()");
      return;
   }

   QSettingsGroup &top = d->groupStack.top();
   int len = top.toString().size();
   top.setArrayIndex(qMax(i, 0));
   d->groupPrefix.replace(d->groupPrefix.size() - len - 1, len, top.toString());
}

/*!
    Returns a list of all keys, including subkeys, that can be read
    using the QSettings object.

    Example:

    \snippet doc/src/snippets/code/src_corelib_io_qsettings.cpp 17

    If a group is set using beginGroup(), only the keys in the group
    are returned, without the group prefix:

    \snippet doc/src/snippets/code/src_corelib_io_qsettings.cpp 18

    \sa childGroups(), childKeys()
*/
QStringList QSettings::allKeys() const
{
   Q_D(const QSettings);
   return d->children(d->groupPrefix, QSettingsPrivate::AllKeys);
}

/*!
    Returns a list of all top-level keys that can be read using the
    QSettings object.

    Example:

    \snippet doc/src/snippets/code/src_corelib_io_qsettings.cpp 19

    If a group is set using beginGroup(), the top-level keys in that
    group are returned, without the group prefix:

    \snippet doc/src/snippets/code/src_corelib_io_qsettings.cpp 20

    You can navigate through the entire setting hierarchy using
    childKeys() and childGroups() recursively.

    \sa childGroups(), allKeys()
*/
QStringList QSettings::childKeys() const
{
   Q_D(const QSettings);
   return d->children(d->groupPrefix, QSettingsPrivate::ChildKeys);
}

/*!
    Returns a list of all key top-level groups that contain keys that
    can be read using the QSettings object.

    Example:

    \snippet doc/src/snippets/code/src_corelib_io_qsettings.cpp 21

    If a group is set using beginGroup(), the first-level keys in
    that group are returned, without the group prefix.

    \snippet doc/src/snippets/code/src_corelib_io_qsettings.cpp 22

    You can navigate through the entire setting hierarchy using
    childKeys() and childGroups() recursively.

    \sa childKeys(), allKeys()
*/
QStringList QSettings::childGroups() const
{
   Q_D(const QSettings);
   return d->children(d->groupPrefix, QSettingsPrivate::ChildGroups);
}

/*!
    Returns true if settings can be written using this QSettings
    object; returns false otherwise.

    One reason why isWritable() might return false is if
    QSettings operates on a read-only file.

    \warning This function is not perfectly reliable, because the
    file permissions can change at any time.

    \sa fileName(), status(), sync()
*/
bool QSettings::isWritable() const
{
   Q_D(const QSettings);
   return d->isWritable();
}

/*!

  Sets the value of setting \a key to \a value. If the \a key already
  exists, the previous value is overwritten.

  Note that the Windows registry and INI files use case-insensitive
  keys, whereas the Carbon Preferences API on Mac OS X uses
  case-sensitive keys. To avoid portability problems, see the
  \l{Section and Key Syntax} rules.

  Example:

  \snippet doc/src/snippets/code/src_corelib_io_qsettings.cpp 23

  \sa value(), remove(), contains()
*/
void QSettings::setValue(const QString &key, const QVariant &value)
{
   Q_D(QSettings);
   QString k = d->actualKey(key);
   d->set(k, value);
   d->requestUpdate();
}

/*!
    Removes the setting \a key and any sub-settings of \a key.

    Example:

    \snippet doc/src/snippets/code/src_corelib_io_qsettings.cpp 24

    Be aware that if one of the fallback locations contains a setting
    with the same key, that setting will be visible after calling
    remove().

    If \a key is an empty string, all keys in the current group() are
    removed. For example:

    \snippet doc/src/snippets/code/src_corelib_io_qsettings.cpp 25

    Note that the Windows registry and INI files use case-insensitive
    keys, whereas the Carbon Preferences API on Mac OS X uses
    case-sensitive keys. To avoid portability problems, see the
    \l{Section and Key Syntax} rules.

    \sa setValue(), value(), contains()
*/
void QSettings::remove(const QString &key)
{
   Q_D(QSettings);
   /*
       We cannot use actualKey(), because remove() supports empty
       keys. The code is also tricky because of slash handling.
   */
   QString theKey = d->normalizedKey(key);
   if (theKey.isEmpty()) {
      theKey = group();
   } else {
      theKey.prepend(d->groupPrefix);
   }

   if (theKey.isEmpty()) {
      d->clear();
   } else {
      d->remove(theKey);
   }
   d->requestUpdate();
}

/*!
    Returns true if there exists a setting called \a key; returns
    false otherwise.

    If a group is set using beginGroup(), \a key is taken to be
    relative to that group.

    Note that the Windows registry and INI files use case-insensitive
    keys, whereas the Carbon Preferences API on Mac OS X uses
    case-sensitive keys. To avoid portability problems, see the
    \l{Section and Key Syntax} rules.

    \sa value(), setValue()
*/
bool QSettings::contains(const QString &key) const
{
   Q_D(const QSettings);
   QString k = d->actualKey(key);
   return d->get(k, 0);
}

/*!
    Sets whether fallbacks are enabled to \a b.

    By default, fallbacks are enabled.

    \sa fallbacksEnabled()
*/
void QSettings::setFallbacksEnabled(bool b)
{
   Q_D(QSettings);
   d->fallbacks = !!b;
}

/*!
    Returns true if fallbacks are enabled; returns false otherwise.

    By default, fallbacks are enabled.

    \sa setFallbacksEnabled()
*/
bool QSettings::fallbacksEnabled() const
{
   Q_D(const QSettings);
   return d->fallbacks;
}


/*!
    \reimp
*/
bool QSettings::event(QEvent *event)
{
   Q_D(QSettings);
   if (event->type() == QEvent::UpdateRequest) {
      d->update();
      return true;
   }
   return QObject::event(event);
}


/*!
    Returns the value for setting \a key. If the setting doesn't
    exist, returns \a defaultValue.

    If no default value is specified, a default QVariant is
    returned.

    Note that the Windows registry and INI files use case-insensitive
    keys, whereas the Carbon Preferences API on Mac OS X uses
    case-sensitive keys. To avoid portability problems, see the
    \l{Section and Key Syntax} rules.

    Example:

    \snippet doc/src/snippets/code/src_corelib_io_qsettings.cpp 26

    \sa setValue(), contains(), remove()
*/
QVariant QSettings::value(const QString &key, const QVariant &defaultValue) const
{
   Q_D(const QSettings);
   QVariant result = defaultValue;
   QString k = d->actualKey(key);
   d->get(k, &result);
   return result;
}

/*!
    \since 4.4

    Sets the default file format to the given \a format, which is used
    for storing settings for the QSettings(QObject *) constructor.

    If no default format is set, QSettings::NativeFormat is used. See
    the documentation for the QSettings constructor you are using to
    see if that constructor will ignore this function.

    \sa format()
*/
void QSettings::setDefaultFormat(Format format)
{
   globalDefaultFormat = format;
}

/*!
    \since 4.4

    Returns default file format used for storing settings for the QSettings(QObject *) constructor.
    If no default format is set, QSettings::NativeFormat is used.

    \sa format()
*/
QSettings::Format QSettings::defaultFormat()
{
   return globalDefaultFormat;
}

/*!
    \obsolete

    Use setPath() instead.

    \oldcode
        setSystemIniPath(path);
    \newcode
        setPath(QSettings::NativeFormat, QSettings::SystemScope, path);
        setPath(QSettings::IniFormat, QSettings::SystemScope, path);
    \endcode
*/
void QSettings::setSystemIniPath(const QString &dir)
{
   setPath(IniFormat, SystemScope, dir);
#if !defined(Q_OS_WIN) && !defined(Q_OS_MAC)
   setPath(NativeFormat, SystemScope, dir);
#endif
}

/*!
    \obsolete

    Use setPath() instead.
*/

void QSettings::setUserIniPath(const QString &dir)
{
   setPath(IniFormat, UserScope, dir);
#if !defined(Q_OS_WIN) && !defined(Q_OS_MAC)
   setPath(NativeFormat, UserScope, dir);
#endif
}

void QSettings::setPath(Format format, Scope scope, const QString &path)
{
   QMutexLocker locker(globalMutex());
   PathHash *pathHash = pathHashFunc();
   if (pathHash->isEmpty()) {
      initDefaultPaths(&locker);
   }
   pathHash->insert(pathHashKey(format, scope), path + QDir::separator());
}

QSettings::Format QSettings::registerFormat(const QString &extension, ReadFunc readFunc,
      WriteFunc writeFunc,
      Qt::CaseSensitivity caseSensitivity)
{
#ifdef QT_QSETTINGS_ALWAYS_CASE_SENSITIVE_AND_FORGET_ORIGINAL_KEY_ORDER
   Q_ASSERT(caseSensitivity == Qt::CaseSensitive);
#endif

   QMutexLocker locker(globalMutex());
   CustomFormatVector *customFormatVector = customFormatVectorFunc();
   int index = customFormatVector->size();
   if (index == 16) { // the QSettings::Format enum has room for 16 custom formats
      return QSettings::InvalidFormat;
   }

   QConfFileCustomFormat info;
   info.extension = QLatin1Char('.');
   info.extension += extension;
   info.readFunc = readFunc;
   info.writeFunc = writeFunc;
   info.caseSensitivity = caseSensitivity;
   customFormatVector->append(info);

   return QSettings::Format((int)QSettings::CustomFormat1 + index);
}


QT_END_NAMESPACE

#endif // QT_NO_SETTINGS
