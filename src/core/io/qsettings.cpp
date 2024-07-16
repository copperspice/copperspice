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

#include <qsettings.h>

#include <qdebug.h>
#include <qplatformdefs.h>

#ifndef QT_NO_SETTINGS

#include <qcache.h>
#include <qcoreapplication.h>
#include <qdir.h>
#include <qfile.h>
#include <qfileinfo.h>
#include <qlibraryinfo.h>
#include <qlockfile.h>
#include <qmutex.h>
#include <qpoint.h>
#include <qrect.h>
#include <qsavefile.h>
#include <qsize.h>
#include <qstringparser.h>
#include <qtemporaryfile.h>

#ifndef QT_NO_TEXTCODEC
#  include <qtextcodec.h>
#endif

#include <qsettings_p.h>

#ifdef Q_OS_WIN                        // for homedirpath reading from registry
#include <qt_windows.h>
#include <qsystemlibrary_p.h>
#include <shlobj.h>
#endif

#include <algorithm>
#include <stdlib.h>

#ifndef CSIDL_COMMON_APPDATA
#define CSIDL_COMMON_APPDATA  0x0023   // All Users\Application Data
#endif

#ifndef CSIDL_APPDATA
#define CSIDL_APPDATA      0x001a      // <username>\Application Data
#endif

#if defined(Q_OS_UNIX) && ! defined(Q_OS_DARWIN)
#define Q_XDG_PLATFORM
#endif

struct QConfFileCustomFormat {
   QString extension;
   QSettings::ReadFunc readFunc;
   QSettings::WriteFunc writeFunc;
   Qt::CaseSensitivity caseSensitivity;
};

using ConfFileHash       = QHash<QString, QConfFile *>;
using ConfFileCache      = QCache<QString, QConfFile>;
using PathHash           = QHash<int, QString>;
using CustomFormatVector = QVector<QConfFileCustomFormat>;

static ConfFileHash *usedHashFunc()
{
   static ConfFileHash retval;
   return &retval;
}

static ConfFileCache *unusedCacheFunc()
{
   static ConfFileCache retval;
   return &retval;
}

static PathHash *pathHashFunc()
{
   static PathHash retval;
   return &retval;
}

static CustomFormatVector *customFormatVectorFunc()
{
   static CustomFormatVector retval;
   return &retval;
}

static QMutex *globalMutex()
{
   static QMutex retval;
   return &retval;
}

static QSettings::Format globalDefaultFormat = QSettings::NativeFormat;

static constexpr const int Space   = 0x1;
static constexpr const int Special = 0x2;

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
      // Create the directories to the file
      QDir dir(fileInfo.absolutePath());

      if (! dir.exists()) {
         if (! dir.mkpath(dir.absolutePath())) {
            return false;
         }
      }

      // use a temporary file to avoid race conditions
      QTemporaryFile file(name);
      return file.open();
   }

#endif
}

QConfFile *QConfFile::fromName(const QString &fileName, bool userPermission)
{
   QString absPath = QFileInfo(fileName).absoluteFilePath();

   ConfFileHash *usedHash     = usedHashFunc();
   ConfFileCache *unusedCache = unusedCacheFunc();

   QConfFile *filePtr = nullptr;
   QMutexLocker locker(globalMutex());

   if (! (filePtr = usedHash->value(absPath))) {
      if ((filePtr = unusedCache->take(absPath))) {
         usedHash->insert(absPath, filePtr);
      }
   }

   if (filePtr != nullptr) {
      filePtr->ref.ref();
      return filePtr;
   }

   return new QConfFile(absPath, userPermission);
}

void QConfFile::clearCache()
{
   QMutexLocker locker(globalMutex());
   unusedCacheFunc()->clear();
}

QSettingsPrivate::QSettingsPrivate(QSettings::Format format)
   : m_format(format), m_scope(QSettings::UserScope), iniCodec(nullptr), m_spec(0), fallbacks(true),
     pendingChanges(false), m_status(QSettings::NoError)
{
}

QSettingsPrivate::QSettingsPrivate(QSettings::Format format, QSettings::Scope scope,
      const QString &organization, const QString &application)
   : m_format(format), m_scope(scope), organizationName(organization), applicationName(application),
     iniCodec(nullptr), m_spec(0), fallbacks(true), pendingChanges(false), m_status(QSettings::NoError)
{
}

QSettingsPrivate::~QSettingsPrivate()
{
}

QString QSettingsPrivate::actualKey(const QString &key) const
{
   QString n = normalizedKey(key);

   Q_ASSERT_X(! n.isEmpty(), "QSettings", "empty key");
   n.prepend(groupPrefix);

   return n;
}

QString QSettingsPrivate::normalizedKey(const QString &key)
{
   QString result = key;
   int i = 0;

   while (i < result.size()) {
      while (result.at(i) == '/') {
         result.remove(i, 1);

         if (i == result.size()) {
            goto after_loop;
         }
      }

      while (result.at(i) != '/') {
         ++i;

         if (i == result.size()) {
            return result;
         }
      }

      ++i; // leave the slash alone
   }

after_loop:

   if (! result.isEmpty()) {
      result.truncate(i - 1);   // remove the trailing slash
   }

   return result;
}

// see also qsettings_win.cpp and qsettings_mac.cpp

#if ! defined(Q_OS_WIN) && ! defined(Q_OS_DARWIN)
QSettingsPrivate *QSettingsPrivate::create(QSettings::Format format, QSettings::Scope scope,
      const QString &organization, const QString &application)
{
   return new QConfFileSettingsPrivate(format, scope, organization, application);
}
#endif

#if ! defined(Q_OS_WIN)
QSettingsPrivate *QSettingsPrivate::create(const QString &fileName, QSettings::Format format)
{
   return new QConfFileSettingsPrivate(fileName, format);
}
#endif

void QSettingsPrivate::processChild(QString key, ChildSpec spec, QMap<QString, QString> &result)
{
   if (spec != AllKeys) {
      int slashPos = key.indexOf('/');

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

   if (! group.name().isEmpty()) {
      groupPrefix += group.name();
      groupPrefix += '/';
   }
}

// only set an error if one is not set, user always gets the first error that occurred.
// always allow clearing errors
void QSettingsPrivate::setStatus(QSettings::Status status) const
{
   if (status == QSettings::NoError || m_status == QSettings::NoError) {
      m_status = status;
   }
}

void QSettingsPrivate::update()
{
   flush();
   pendingChanges = false;
}

void QSettingsPrivate::requestUpdate()
{
   if (! pendingChanges) {
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

      if (str.startsWith('@')) {
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
         result = "@Invalid()";
         break;

      case QVariant::ByteArray: {
         QByteArray a = v.toByteArray();
         result  = QString("@ByteArray(");
         result += QString::fromLatin1(a.constData(), a.size());
         result += QChar(')');
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

         if (result.startsWith('@')) {
            result.prepend('@');
         }

         break;
      }

      case QVariant::Rect: {
         QRect r = v.value<QRect>();

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
         QSize s = v.value<QSize>();

         result += QLatin1String("@Size(");
         result += QString::number(s.width());
         result += QLatin1Char(' ');
         result += QString::number(s.height());
         result += QLatin1Char(')');
         break;
      }

      case QVariant::Point: {
         QPoint p = v.value<QPoint>();

         result += QLatin1String("@Point(");
         result += QString::number(p.x());
         result += QLatin1Char(' ');
         result += QString::number(p.y());
         result += QLatin1Char(')');
         break;
      }

      default: {
         QByteArray data;

         {
            QDataStream s(&data, QIODevice::WriteOnly);
            s << v;
         }

         result = "@Variant(";
         result += QString::fromLatin1(data.constData(), data.size());
         result += ')';

         break;
      }
   }

   return result;
}

QVariant QSettingsPrivate::stringToVariant(const QString &s)
{
   if (s.startsWith('@')) {

      if (s.endsWith(')')) {

         if (s.startsWith("@ByteArray(")) {
            return QVariant(s.toLatin1().mid(11, s.size() - 12));

         } else if (s.startsWith("@String(")) {
            return QVariant(s.mid(8, s.size() - 9));

         } else if (s.startsWith("@Variant(")) {

            QByteArray a(s.toLatin1().mid(9));
            QDataStream stream(&a, QIODevice::ReadOnly);

            QVariant result;
            stream >> result;

            return result;

         } else if (s.startsWith("@Rect(")) {
            QStringList args = QSettingsPrivate::splitArgs(s, 5);

            if (args.size() == 4) {
               return QVariant(QRect(args[0].toInteger<int>(), args[1].toInteger<int>(), args[2].toInteger<int>(), args[3].toInteger<int>()));
            }

         } else if (s.startsWith("@Size(")) {
            QStringList args = QSettingsPrivate::splitArgs(s, 5);

            if (args.size() == 2) {
               return QVariant(QSize(args[0].toInteger<int>(), args[1].toInteger<int>()));
            }

         } else if (s.startsWith("@Point(")) {
            QStringList args = QSettingsPrivate::splitArgs(s, 6);

            if (args.size() == 2) {
               return QVariant(QPoint(args[0].toInteger<int>(), args[1].toInteger<int>()));
            }

         } else if (s == "@Invalid()") {
            return QVariant();
         }

      }

      if (s.startsWith("@@")) {
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

         for (int j = 0; j < 4; ++j) {
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

   while (i < to) {
      int ch = (uchar)key.at(i);

      if (ch == '\\') {
         result += '/';
         ++i;
         continue;
      }

      if (ch != '%' || i == to - 1) {
         if (uint(ch - 'A') <= 'Z' - 'A') {
            // only for ASCII
            lowercaseOnly = false;
         }

         result += ch;
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
         result += '%';
         // ### missing U
         ++i;
         continue;
      }

      bool ok;
      ch = key.mid(firstDigitPos, numDigits).toInt(&ok, 16);

      if (! ok) {
         result += '%';
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
   bool needsQuotes       = false;
   bool escapeNextIfDigit = false;
   int i;

   bool useCodec = codec && !str.startsWith("@ByteArray(") && ! str.startsWith("@Variant(");
   int startPos = result.size();

   result.reserve(startPos + str.size() * 3 / 2);

   for (i = 0; i < str.size(); ++i) {
      uint ch = str.at(i).unicode();

      if (ch == ';' || ch == ',' || ch == '=') {
         needsQuotes = true;
      }

      if (escapeNextIfDigit
            && ((ch >= '0' && ch <= '9') || (ch >= 'a' && ch <= 'f') || (ch >= 'A' && ch <= 'F'))) {
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
            if (ch <= 0x1F || (ch >= 0x7F && ! useCodec)) {
               result += "\\x";
               result += QByteArray::number(ch, 16);
               escapeNextIfDigit = true;

            } else if (useCodec) {
               result += codec->fromUnicode(str.mid(i, 1));

            } else {
               result += (char)ch;

            }
      }
   }

   if (needsQuotes || (startPos < result.size() && (result.at(startPos) == ' ' || result.at(result.size() - 1) == ' '))) {
      result.insert(startPos, '"');
      result += '"';
   }
}

static inline void iniChopTrailingSpaces(QString &str, int limit)
{
   int n = str.size() - 1;
   QChar ch;

   while (n >= limit && ((ch = str.at(n)) == ' ' || ch == '\t')) {
      str.truncate(n--);
   }
}

void QSettingsPrivate::iniEscapedStringList(const QStringList &strs, QByteArray &result, QTextCodec *codec)
{
   if (strs.isEmpty()) {

      //  need to distinguish between empty lists and one-item lists that contain an empty string.
      //  nice to have  @EmptyList() symbol but that would break compatibility
      //  @Invalid() stands for QVariant() and QVariant().toStringList() returns an empty QStringList
      //  use a nicer syntax like @List, for variant lists

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
      QString &stringResult, QStringList &stringListResult, QTextCodec *codec)
{
   static const char escapeCodes[][2] = {
      { 'a', '\a'  },
      { 'b', '\b'  },
      { 'f', '\f'  },
      { 'n', '\n'  },
      { 'r', '\r'  },
      { 't', '\t'  },
      { 'v', '\v'  },
      { '"', '"'   },
      { '?', '?'   },
      { '\'', '\'' },
      { '\\', '\\' }
   };

   static constexpr const int numEscapeCodes = sizeof(escapeCodes) / sizeof(escapeCodes[0]);

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

StNormal:
   int chopLimit = stringResult.length();

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
                  stringResult += escapeCodes[j][1];
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

            chopLimit = stringResult.length();
            break;

         case '"':
            ++i;
            currentValueIsQuoted = true;
            inQuotedString = ! inQuotedString;

            if (! inQuotedString) {
               goto StSkipSpaces;
            }

            break;

         case ',':
            if (! inQuotedString) {
               if (! currentValueIsQuoted) {
                  iniChopTrailingSpaces(stringResult, chopLimit);
               }

               if (! isStringList) {
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

         [[fallthrough]];

         default: {
            int j = i + 1;

            while (j < to) {
               ch = str.at(j);

               if (ch == '\\' || ch == '"' || ch == ',') {
                  break;
               }

               ++j;
            }

            if (codec) {
               stringResult += codec->toUnicode(str.constData() + i, j - i);

            } else  {
               stringResult.append(QString::fromUtf8(str.mid(i, j - i)));

            }

            i = j;
         }
      }
   }

   if (! currentValueIsQuoted) {
      iniChopTrailingSpaces(stringResult, chopLimit);
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

void QConfFileSettingsPrivate::initFormat()
{
   extension = (m_format == QSettings::NativeFormat) ? QString(".conf") : QString(".ini");
   readFunc  = nullptr;
   writeFunc = nullptr;

#if defined(Q_OS_DARWIN)
   caseSensitivity = (m_format == QSettings::NativeFormat) ? Qt::CaseSensitive : IniCaseSensitivity;
#else
   caseSensitivity = IniCaseSensitivity;
#endif

   if (m_format > QSettings::IniFormat) {
      QMutexLocker locker(globalMutex());
      const CustomFormatVector *customFormatVector = customFormatVectorFunc();

      int i = (int)m_format - (int)QSettings::CustomFormat1;

      if (i >= 0 && i < customFormatVector->size()) {
         QConfFileCustomFormat info = customFormatVector->at(i);
         extension = info.extension;
         readFunc  = info.readFunc;
         writeFunc = info.writeFunc;
         caseSensitivity = info.caseSensitivity;
      }
   }
}

void QConfFileSettingsPrivate::initAccess()
{
   if (! m_confFiles.isEmpty()) {
      if (m_format > QSettings::IniFormat) {
         if (! readFunc) {
            setStatus(QSettings::AccessError);
         }
      }
   }

   // loads the files the first time
   sync();
}

#ifdef Q_OS_WIN
static QString windowsConfigPath(int type)
{
   QString result;

   std::wstring path(MAX_PATH, L'\0');

   if (SHGetSpecialFolderPath(nullptr, &path[0], type, FALSE)) {
      result = QString::fromStdWString(path);
   }

   if (result.isEmpty()) {
      switch (type) {
         case CSIDL_COMMON_APPDATA:
            result = "C:\\temp\\qt-common";
            break;

         case CSIDL_APPDATA:
            result = "C:\\temp\\qt-user";
            break;

         default:
            ;
      }
   }

   return result;
}
#endif

static inline int pathHashKey(QSettings::Format format, QSettings::Scope scope)
{
   return int((uint(format) << 1) | uint(scope == QSettings::SystemScope));
}

static void initDefaultPaths(QMutexLocker *locker)
{
   PathHash *pathHash = pathHashFunc();
   QString homePath   = QDir::homePath();
   QString systemPath;

   locker->unlock();

   // QLibraryInfo::location() uses QSettings, in order to
   // avoid a dead lock, we can not hold the global mutex while calling it

   systemPath = QLibraryInfo::location(QLibraryInfo::SettingsPath);
   systemPath += '/';

   locker->relock();

   if (pathHash->isEmpty()) {
      // Lazy initialization of pathHash, initialize the IniFormat paths and (on Unix) the NativeFormat paths.
      // The NativeFormat paths are not configurable for the Windows registry and the Mac CFPreferences.

#ifdef Q_OS_WIN
      pathHash->insert(pathHashKey(QSettings::IniFormat, QSettings::UserScope),
            windowsConfigPath(CSIDL_APPDATA) + QDir::separator());

      pathHash->insert(pathHashKey(QSettings::IniFormat, QSettings::SystemScope),
            windowsConfigPath(CSIDL_COMMON_APPDATA) + QDir::separator());
#else
      QString userPath;
      QByteArray env = qgetenv("XDG_CONFIG_HOME");

      if (env.isEmpty()) {
         userPath = homePath;
         userPath += QString("/.config");

      } else if (env.startsWith('/')) {
         userPath = QFile::decodeName(env);

      } else {
         userPath  = homePath;
         userPath += '/';
         userPath += QFile::decodeName(env);
      }

      userPath += '/';

      pathHash->insert(pathHashKey(QSettings::IniFormat, QSettings::UserScope), userPath);
      pathHash->insert(pathHashKey(QSettings::IniFormat, QSettings::SystemScope), systemPath);

#ifndef Q_OS_DARWIN
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

   if (! result.isEmpty()) {
      return result;
   }

   // fall back on INI path
   return pathHash->value(pathHashKey(QSettings::IniFormat, scope));
}

QConfFileSettingsPrivate::QConfFileSettingsPrivate(QSettings::Format format, QSettings::Scope scope,
      const QString &organization, const QString &application)
   : QSettingsPrivate(format, scope, organization, application), nextPosition(0x40000000)
{
   initFormat();

   QString org = organization;

   if (org.isEmpty()) {
      setStatus(QSettings::AccessError);
      org = "Unknown Organization";
   }

   QString appFile = org + QDir::separator() + application + extension;
   QString orgFile = org + extension;

   if (scope == QSettings::UserScope) {
      QString userPath = getPath(format, QSettings::UserScope);

      if (! application.isEmpty()) {
         constexpr auto index_A = F_User | F_Application;

         if (m_confFiles.size() < index_A + 1) {
            m_confFiles.resize(index_A + 1);
         }

         m_confFiles[index_A].reset(QConfFile::fromName(userPath + appFile, true));
      }

      constexpr auto index_B = F_User | F_Organization;

      if (m_confFiles.size() < index_B + 1) {
         m_confFiles.resize(index_B + 1);
      }

      m_confFiles[index_B].reset(QConfFile::fromName(userPath + orgFile, true));
   }

   QString systemPath = getPath(format, QSettings::SystemScope);

   if (! application.isEmpty()) {

      constexpr auto index_C = F_System | F_Application;

      if (m_confFiles.size() < index_C + 1) {
         m_confFiles.resize(index_C + 1);
      }

      m_confFiles[index_C].reset(QConfFile::fromName(systemPath + appFile, false));
   }

   constexpr auto index_D = F_System | F_Organization;

   if (m_confFiles.size() < index_D + 1) {
      m_confFiles.resize(index_D + 1);
   }

   m_confFiles[index_D].reset(QConfFile::fromName(systemPath + orgFile, false));

   int index = 0;

   for (const auto &ptr : m_confFiles) {
      if (ptr != nullptr) {
         m_spec = index;
         break;
      }

      ++index;
   }

   initAccess();
}

QConfFileSettingsPrivate::QConfFileSettingsPrivate(const QString &fileName, QSettings::Format format)
   : QSettingsPrivate(format), nextPosition(0x40000000) // big positive number
{
   initFormat();

   if (m_confFiles.isEmpty()) {
      m_confFiles.resize(1);
   }

   m_confFiles[0].reset(QConfFile::fromName(fileName, true));

   initAccess();
}

QConfFileSettingsPrivate::~QConfFileSettingsPrivate()
{
   QMutexLocker locker(globalMutex());
   ConfFileHash *usedHash     = usedHashFunc();
   ConfFileCache *unusedCache = unusedCacheFunc();

   for (auto &ptr : m_confFiles) {

      if (ptr != nullptr && ! ptr->ref.deref()) {

         if (ptr->size == 0) {
            ptr.reset();

         } else {
            if (usedHash) {
               usedHash->remove(ptr->name);
            }

            if (unusedCache) {
               try {
                  // compute a better size?
                  unusedCache->insert(ptr->name, ptr.get(), 10 + (ptr->originalKeys.size() / 4));
                  ptr.release();

               } catch (...) {
                  // out of memory, do not cache the file
                  ptr.reset();
               }

            } else {
               // unusedCache is gone, destroy the objet and set the pointer to nullptr
               ptr.reset();
            }
         }
      }

      // prevent using the ptr again
      ptr.release();
   }
}

void QConfFileSettingsPrivate::remove(const QString &key)
{
   if (m_confFiles.size() <= m_spec) {
      return;
   }

   QConfFile *filePtr = m_confFiles[m_spec].get();

   if (filePtr == nullptr) {
      return;
   }

   QSettingsKey theKey(key, caseSensitivity);
   QSettingsKey prefix(key + '/', caseSensitivity);
   QMutexLocker locker(&filePtr->mutex);

   ensureSectionParsed(filePtr, theKey);
   ensureSectionParsed(filePtr, prefix);

   ParsedSettingsMap::iterator i = filePtr->addedKeys.lowerBound(prefix);

   while (i != filePtr->addedKeys.end() && i.key().startsWith(prefix)) {
      i = filePtr->addedKeys.erase(i);
   }

   filePtr->addedKeys.remove(theKey);

   ParsedSettingsMap::const_iterator j = const_cast<const ParsedSettingsMap *>(&filePtr->originalKeys)->lowerBound(prefix);

   while (j != filePtr->originalKeys.constEnd() && j.key().startsWith(prefix)) {
      filePtr->removedKeys.insert(j.key(), QVariant());
      ++j;
   }

   if (filePtr->originalKeys.contains(theKey)) {
      filePtr->removedKeys.insert(theKey, QVariant());
   }
}

void QConfFileSettingsPrivate::set(const QString &key, const QVariant &value)
{
   if (m_confFiles.size() <= m_spec) {
      return;
   }

   QConfFile *filePtr = m_confFiles[m_spec].get();

   if (filePtr == nullptr) {
      return;
   }

   QSettingsKey theKey(key, caseSensitivity, nextPosition);
   ++nextPosition;

   QMutexLocker locker(&filePtr->mutex);

   filePtr->removedKeys.remove(theKey);
   filePtr->addedKeys.insert(theKey, value);
}

bool QConfFileSettingsPrivate::get(const QString &key, QVariant *value) const
{
   QSettingsKey theKey(key, caseSensitivity);
   ParsedSettingsMap::const_iterator j;

   bool found = false;

   for (auto &ptr : m_confFiles) {
      QConfFile *filePtr = ptr.get();

      if (filePtr != nullptr) {
         QMutexLocker locker(&filePtr->mutex);

         if (! filePtr->addedKeys.isEmpty()) {
            j = filePtr->addedKeys.constFind(theKey);
            found = (j != filePtr->addedKeys.constEnd());
         }

         if (! found) {
            ensureSectionParsed(filePtr, theKey);
            j = filePtr->originalKeys.constFind(theKey);
            found = (j != filePtr->originalKeys.constEnd() && ! filePtr->removedKeys.contains(theKey));
         }

         if (found && value) {
            *value = *j;
         }

         if (found) {
            return true;
         }

         if (! fallbacks) {
            break;
         }
      }
   }

   return false;
}

QStringList QConfFileSettingsPrivate::children(const QString &prefix, ChildSpec spec) const
{
   QMap<QString, QString> retval;
   ParsedSettingsMap::const_iterator j;

   QSettingsKey thePrefix(prefix, caseSensitivity);
   int startPos = prefix.size();

   for (auto &ptr : m_confFiles) {
      QConfFile *filePtr = ptr.get();

      if (filePtr != nullptr) {
         QMutexLocker locker(&filePtr->mutex);

         if (thePrefix.isEmpty()) {
            ensureAllSectionsParsed(filePtr);
         } else {
            ensureSectionParsed(filePtr, thePrefix);
         }

         j = const_cast<const ParsedSettingsMap *>(&filePtr->originalKeys)->lowerBound(thePrefix);

         while (j != filePtr->originalKeys.constEnd() && j.key().startsWith(thePrefix)) {
            if (! filePtr->removedKeys.contains(j.key())) {
               processChild(j.key().originalCaseKey().mid(startPos), spec, retval);
            }

            ++j;
         }

         j = const_cast<const ParsedSettingsMap *>(&filePtr->addedKeys)->lowerBound(thePrefix);

         while (j != filePtr->addedKeys.constEnd() && j.key().startsWith(thePrefix)) {
            processChild(j.key().originalCaseKey().mid(startPos), spec, retval);

            ++j;
         }

         if (! fallbacks) {
            break;
         }
      }
   }

   return retval.keys();
}

void QConfFileSettingsPrivate::clear()
{
   if (m_confFiles.size() <= m_spec) {
      return;
   }

   QConfFile *filePtr = m_confFiles[m_spec].get();

   if (filePtr == nullptr) {
      return;
   }

   QMutexLocker locker(&filePtr->mutex);
   ensureAllSectionsParsed(filePtr);

   filePtr->addedKeys.clear();
   filePtr->removedKeys = filePtr->originalKeys;
}

void QConfFileSettingsPrivate::sync()
{
   // in case of an error try to go on

   int index = 0;

   for (auto &ptr : m_confFiles) {
      QConfFile *filePtr = ptr.get();

      if (filePtr != nullptr) {
         QMutexLocker locker(&filePtr->mutex);
         syncConfFile(index);
      }

      ++index;
   }
}

void QConfFileSettingsPrivate::flush()
{
   sync();
}

QString QConfFileSettingsPrivate::fileName() const
{
   if (m_confFiles.size() <= m_spec) {
      return QString();
   }

   QConfFile *filePtr = m_confFiles[m_spec].get();

   if (filePtr == nullptr) {
      return QString();
   }

   return filePtr->name;
}

bool QConfFileSettingsPrivate::isWritable() const
{
   if (m_format > QSettings::IniFormat && ! writeFunc) {
      return false;
   }

   if (m_confFiles.size() <= m_spec) {
      return false;
   }

   QConfFile *filePtr = m_confFiles[m_spec].get();

   if (filePtr == nullptr) {
      return false;
   }

   return filePtr->isWritable();
}

void QConfFileSettingsPrivate::syncConfFile(int confFileNo)
{
   if (m_confFiles.size() <= confFileNo) {
      return;
   }

   QConfFile *filePtr = m_confFiles[confFileNo].get();

   bool readOnly = filePtr->addedKeys.isEmpty() && filePtr->removedKeys.isEmpty();
   bool ok;

   // can often optimize the read-only case if the file on disk has not changed

   if (readOnly && filePtr->size > 0) {
      QFileInfo fileInfo(filePtr->name);

      if (filePtr->size == fileInfo.size() && filePtr->timeStamp == fileInfo.lastModified()) {
         return;
      }
   }

   //  Use a lockfile in order to protect us against other QSettings instances
   //  trying to write the same settings at the same time.

   //  We only need to lock if we are actually writing as only concurrent writes are a problem.
   //  Concurrent read and write are not a problem because the writing operation is atomic.

   QLockFile lockFile(filePtr->name + ".lock");

   if (! readOnly) {
      if (! filePtr->isWritable() || ! lockFile.lock() ) {
         setStatus(QSettings::AccessError);
         return;
      }
   }

   // hold the lock, reread the file if it has changed since last time we read it

   QFileInfo fileInfo(filePtr->name);
   bool mustReadFile = true;
   bool createFile   = ! fileInfo.exists();

   if (! readOnly) {
      mustReadFile = (filePtr->size != fileInfo.size()
            || (filePtr->size != 0 && filePtr->timeStamp != fileInfo.lastModified()));
   }

   if (mustReadFile) {
      filePtr->unparsedIniSections.clear();
      filePtr->originalKeys.clear();

      QFile file(filePtr->name);

      if (! createFile && ! file.open(QFile::ReadOnly)) {
         setStatus(QSettings::AccessError);
         return;
      }

      // Files that we ca not read because of permissions or
      // because they do not exist are treated as empty files

      if (file.isReadable() && fileInfo.size() != 0) {

#ifdef Q_OS_DARWIN

         if (m_format == QSettings::NativeFormat) {
            ok = readPlistFile(filePtr->name, &filePtr->originalKeys);

         } else
#endif
         {
            if (m_format <= QSettings::IniFormat) {
               QByteArray data = file.readAll();
               ok = readIniFile(data, &filePtr->unparsedIniSections);

            } else {
               if (readFunc) {
                  QSettings::SettingsMap tempNewKeys;
                  ok = readFunc(file, tempNewKeys);

                  if (ok) {
                     QSettings::SettingsMap::const_iterator i = tempNewKeys.constBegin();

                     while (i != tempNewKeys.constEnd()) {
                        filePtr->originalKeys.insert(QSettingsKey(i.key(), caseSensitivity), i.value());
                        ++i;
                     }
                  }

               } else {
                  ok = false;
               }
            }
         }

         if (! ok) {
            setStatus(QSettings::FormatError);
         }
      }

      filePtr->size = fileInfo.size();
      filePtr->timeStamp = fileInfo.lastModified();
   }

   // need to save the file, keep the file lock

   if (! readOnly) {
      ensureAllSectionsParsed(filePtr);
      ParsedSettingsMap mergedKeys = filePtr->mergedKeyMap();

#ifdef Q_OS_DARWIN
      if (m_format == QSettings::NativeFormat) {
         ok = writePlistFile(filePtr->name, mergedKeys);

      } else
#endif

      {
         QSaveFile sf(filePtr->name);

         if (! sf.open(QIODevice::WriteOnly)) {
            setStatus(QSettings::AccessError);
            ok = false;

         } else if (m_format <= QSettings::IniFormat) {
            ok = writeIniFile(sf, mergedKeys);

         } else {
            if (writeFunc) {
               QSettings::SettingsMap tempOriginalKeys;

               ParsedSettingsMap::const_iterator i = mergedKeys.constBegin();

               while (i != mergedKeys.constEnd()) {
                  tempOriginalKeys.insert(i.key(), i.value());
                  ++i;
               }

               ok = writeFunc(sf, tempOriginalKeys);

            } else {
               ok = false;
            }
         }

         if (ok) {
            ok = sf.commit();
         }
      }

      if (ok) {
         filePtr->unparsedIniSections.clear();
         filePtr->originalKeys = mergedKeys;
         filePtr->addedKeys.clear();
         filePtr->removedKeys.clear();

         QFileInfo tmpFileInfo(filePtr->name);
         filePtr->size      = tmpFileInfo.size();
         filePtr->timeStamp = tmpFileInfo.lastModified();

         if (createFile) {
            QFile::Permissions perms = tmpFileInfo.permissions() | QFile::ReadOwner | QFile::WriteOwner;

            if (! filePtr->userPerms) {
               perms |= QFile::ReadGroup | QFile::ReadOther;
            }

            QFile(filePtr->name).setPermissions(perms);
         }

      } else {
         setStatus(QSettings::AccessError);
      }
   }
}

bool QConfFileSettingsPrivate::readIniLine(const QByteArray &data, int &dataPos,
      int &lineStart, int &lineLen, int &equalsPos)
{
   int dataLen   = data.length();
   bool inQuotes = false;

   equalsPos = -1;

   lineStart = dataPos;

   while (lineStart < dataLen && (charTraits[uint(uchar(data.at(lineStart)))] & Space)) {
      ++lineStart;
   }

   int i = lineStart;

   while (i < dataLen) {
      while (! (charTraits[uint(uchar(data.at(i)))] & Special)) {
         if (++i == dataLen) {
            goto break_out_of_outer_loop;
         }
      }

      char ch = data.at(i++);

      if (ch == '=') {
         if (! inQuotes && equalsPos == -1) {
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
            char ch2 = data.at(i++);

            if (i < dataLen) {
               char ch3 = data.at(i);

               // \n, \r, \r\n, and \n\r are legitimate line terminators in INI files
               if ((ch2 == '\n' && ch3 == '\r') || (ch2 == '\r' && ch3 == '\n')) {
                  ++i;
               }
            }
         }

      } else if (ch == '"') {
         inQuotes = !inQuotes;

      } else {
         Q_ASSERT(ch == ';');

         if (i == lineStart + 1) {
            char ch4;

            while (i < dataLen && ((ch4 = data.at(i) != '\n') && ch4 != '\r')) {
               ++i;
            }

            lineStart = i;

         } else if (! inQuotes) {
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
bool QConfFileSettingsPrivate::readIniFile(const QByteArray &data,
      UnparsedSettingsMap *unparsedIniSections)
{
#define FLUSH_CURRENT_SECTION() \
   { \
      QByteArray &sectionData = (*unparsedIniSections)[QSettingsKey(currentSection, \
                        IniCaseSensitivity, \
                        sectionPosition)]; \
      if (! sectionData.isEmpty()) \
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
               currentSection = iniSection.constData() + 1;

            } else {
               currentSection.clear();
               iniUnescapedKey(iniSection, 0, iniSection.size(), currentSection);
            }

            currentSection += '/';
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
            : IniCaseSensitivity, position), variant);

      ++position;
   }

   return ok;
}

class QSettingsIniKey : public QString
{
 public:
   QSettingsIniKey()
      : position(-1)
   {
   }

   QSettingsIniKey(const QString &str, int pos = -1)
      : QString(str), position(pos)
   {
   }

   int position;
};

static bool operator<(const QSettingsIniKey &k1, const QSettingsIniKey &k2)
{
   if (k1.position != k2.position) {
      return k1.position < k2.position;
   }

   return static_cast<const QString &>(k1) < static_cast<const QString &>(k2);
}

struct QSettingsIniSection {
   int position;
   QMap<QSettingsIniKey, QVariant> keyMap;

   QSettingsIniSection()
      : position(-1)
   {
   }
};

bool QConfFileSettingsPrivate::writeIniFile(QIODevice &device, const ParsedSettingsMap &map)
{
   QMap<QString, QSettingsIniSection> tmpMap;
   QMap<QString, QSettingsIniSection>::const_iterator iterMap;

#ifdef Q_OS_WIN
   const char *const eol = "\r\n";
#else
   const char eol = '\n';
#endif

   for (ParsedSettingsMap::const_iterator j = map.constBegin(); j != map.constEnd(); ++j) {
      QString section;

      QSettingsIniKey key(j.key().originalCaseKey(), j.key().originalKeyPosition());
      int slashPos;

      if ((slashPos = key.indexOf(QChar('/'))) != -1) {
         section = key.left(slashPos);
         key.remove(0, slashPos + 1);
      }

      QSettingsIniSection &iniSection = tmpMap[section];

      // -1 means infinity
      if (uint(key.position) < uint(iniSection.position)) {
         iniSection.position = key.position;
      }

      iniSection.keyMap[key] = j.value();
   }

   const int sectionCount = tmpMap.size();

   QVector<QSettingsIniKey> sections;
   sections.reserve(sectionCount);

   for (iterMap = tmpMap.constBegin(); iterMap != tmpMap.constEnd(); ++iterMap) {
      sections.append(QSettingsIniKey(iterMap.key(), iterMap.value().position));
   }

   std::sort(sections.begin(), sections.end());

   bool writeError = false;

   for (int cnt = 0; ! writeError && cnt < sectionCount; ++cnt) {
      iterMap = tmpMap.constFind(sections.at(cnt));
      Q_ASSERT(iterMap != tmpMap.constEnd());

      QByteArray realSection;

      iniEscapedKey(iterMap.key(), realSection);

      if (realSection.isEmpty()) {
         realSection = "[General]";

      } else if (qstricmp(realSection.constData(), "general") == 0) {
         realSection = "[%General]";

      } else {
         realSection.prepend('[');
         realSection.append(']');
      }

      if (cnt != 0) {
         realSection.prepend(eol);
      }

      realSection += eol;

      device.write(realSection);

      const QMap<QSettingsIniKey, QVariant> &ents = iterMap.value().keyMap;

      for (auto i = ents.constBegin(); i != ents.constEnd(); ++i) {
         QByteArray block;
         iniEscapedKey(i.key(), block);
         block += '=';

         const QVariant &value = i.value();

         //  The size() != 1 trick is necessary because
         //  QVariant(QString("foo")).toList() returns an empty
         //  list, not a list containing "foo".

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

   return ! writeError;
}

void QConfFileSettingsPrivate::ensureAllSectionsParsed(QConfFile *filePtr) const
{
   auto iter      = filePtr->unparsedIniSections.constBegin();
   const auto end = filePtr->unparsedIniSections.constEnd();

   for (; iter != end; ++iter) {
      if (! QConfFileSettingsPrivate::readIniSection(iter.key(), iter.value(), &filePtr->originalKeys, iniCodec)) {
         setStatus(QSettings::FormatError);
      }
   }

   filePtr->unparsedIniSections.clear();
}

void QConfFileSettingsPrivate::ensureSectionParsed(QConfFile *filePtr, const QSettingsKey &searchKey) const
{
   if (filePtr->unparsedIniSections.isEmpty()) {
      return;
   }

   UnparsedSettingsMap::iterator iter;

   int indexOfSlash = searchKey.indexOf('/');

   if (indexOfSlash != -1) {
      iter = filePtr->unparsedIniSections.upperBound(searchKey);

      if (iter == filePtr->unparsedIniSections.begin()) {
         return;
      }

      --iter;

      if (iter.key().isEmpty() || ! searchKey.startsWith(iter.key())) {
         return;
      }

   } else {
      iter = filePtr->unparsedIniSections.begin();

      if (iter == filePtr->unparsedIniSections.end() || ! iter.key().isEmpty()) {
         return;
      }
   }

   if (! QConfFileSettingsPrivate::readIniSection(iter.key(), iter.value(), &filePtr->originalKeys, iniCodec)) {
      setStatus(QSettings::FormatError);
   }

   filePtr->unparsedIniSections.erase(iter);
}

QSettings::QSettings(const QString &organization, const QString &application, QObject *parent)
   : QObject(parent), d_ptr(QSettingsPrivate::create(NativeFormat, UserScope, organization, application))
{
   d_ptr->q_ptr = this;
}

QSettings::QSettings(Scope scope, const QString &organization, const QString &application, QObject *parent)
   : QObject(parent), d_ptr(QSettingsPrivate::create(NativeFormat, scope, organization, application))
{
   d_ptr->q_ptr = this;
}

QSettings::QSettings(Format format, Scope scope, const QString &organization,
      const QString &application, QObject *parent)
   : QObject(parent), d_ptr(QSettingsPrivate::create(format, scope, organization, application))
{
   d_ptr->q_ptr = this;
}

QSettings::QSettings(const QString &fileName, Format format, QObject *parent)
   : QObject(parent), d_ptr(QSettingsPrivate::create(fileName, format))
{
   d_ptr->q_ptr = this;
}

QSettings::QSettings(QObject *parent)
   : QObject(parent), d_ptr(QSettingsPrivate::create(globalDefaultFormat, UserScope,

#ifdef Q_OS_DARWIN
     QCoreApplication::organizationDomain().isEmpty() ? QCoreApplication::organizationName()
        : QCoreApplication::organizationDomain(), QCoreApplication::applicationName() ))
#else
     QCoreApplication::organizationName().isEmpty() ? QCoreApplication::organizationDomain()
        : QCoreApplication::organizationName(), QCoreApplication::applicationName() ))
#endif

{
   d_ptr->q_ptr = this;
}

QSettings::~QSettings()
{
   Q_D(QSettings);

   if (d->pendingChanges) {
      try {
         d->flush();

      } catch (...) {
         // do not flush and do not throw in the destructor
         ;
      }
   }
}

void QSettings::clear()
{
   Q_D(QSettings);
   d->clear();
   d->requestUpdate();
}

void QSettings::sync()
{
   Q_D(QSettings);
   d->sync();
}

QString QSettings::fileName() const
{
   Q_D(const QSettings);
   return d->fileName();
}

QSettings::Format QSettings::format() const
{
   Q_D(const QSettings);
   return d->m_format;
}

QSettings::Scope QSettings::scope() const
{
   Q_D(const QSettings);
   return d->m_scope;
}

QString QSettings::organizationName() const
{
   Q_D(const QSettings);
   return d->organizationName;
}

QString QSettings::applicationName() const
{
   Q_D(const QSettings);
   return d->applicationName;
}

#ifndef QT_NO_TEXTCODEC

void QSettings::setIniCodec(QTextCodec *codec)
{
   Q_D(QSettings);
   d->iniCodec = codec;
}

void QSettings::setIniCodec(const char *codecName)
{
   Q_D(QSettings);

   if (QTextCodec *codec = QTextCodec::codecForName(codecName)) {
      d->iniCodec = codec;
   }
}

QTextCodec *QSettings::iniCodec() const
{
   Q_D(const QSettings);
   return d->iniCodec;
}

#endif

QSettings::Status QSettings::status() const
{
   Q_D(const QSettings);
   return d->m_status;
}

void QSettings::beginGroup(const QString &prefix)
{
   Q_D(QSettings);
   d->beginGroupOrArray(QSettingsGroup(d->normalizedKey(prefix)));
}

void QSettings::endGroup()
{
   Q_D(QSettings);

   if (d->groupStack.isEmpty()) {
      qWarning("QSettings::endGroup() No matching beginGroup()");
      return;
   }

   QSettingsGroup group = d->groupStack.pop();
   int len = group.toString().size();

   if (len > 0) {
      d->groupPrefix.truncate(d->groupPrefix.size() - (len + 1));
   }

   if (group.isArray()) {
      qWarning("QSettings::endGroup() Expected endArray()");
   }
}

QString QSettings::group() const
{
   Q_D(const QSettings);
   return d->groupPrefix.left(d->groupPrefix.size() - 1);
}

int QSettings::beginReadArray(const QString &prefix)
{
   Q_D(QSettings);
   d->beginGroupOrArray(QSettingsGroup(d->normalizedKey(prefix), false));

   return value(QLatin1String("size")).toInt();
}

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

void QSettings::endArray()
{
   Q_D(QSettings);

   if (d->groupStack.isEmpty()) {
      qWarning("QSettings::endArray() No matching beginArray()");
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

   if (! group.isArray()) {
      qWarning("QSettings::endArray() Expected endGroup()");
   }
}

void QSettings::setArrayIndex(int i)
{
   Q_D(QSettings);

   if (d->groupStack.isEmpty() || !d->groupStack.top().isArray()) {
      qWarning("QSettings::setArrayIndex() Missing beginArray()");
      return;
   }

   QSettingsGroup &top = d->groupStack.top();
   int len = top.toString().size();
   top.setArrayIndex(qMax(i, 0));
   d->groupPrefix.replace(d->groupPrefix.size() - len - 1, len, top.toString());
}

QStringList QSettings::allKeys() const
{
   Q_D(const QSettings);
   return d->children(d->groupPrefix, QSettingsPrivate::AllKeys);
}

QStringList QSettings::childKeys() const
{
   Q_D(const QSettings);
   return d->children(d->groupPrefix, QSettingsPrivate::ChildKeys);
}

QStringList QSettings::childGroups() const
{
   Q_D(const QSettings);
   return d->children(d->groupPrefix, QSettingsPrivate::ChildGroups);
}

bool QSettings::isWritable() const
{
   Q_D(const QSettings);
   return d->isWritable();
}

void QSettings::setValue(const QString &key, const QVariant &value)
{
   Q_D(QSettings);

   if (key.isEmpty()) {
      qWarning("QSettings::setValue() Key can not be empty");
      return;
   }

   QString k = d->actualKey(key);
   d->set(k, value);
   d->requestUpdate();
}

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

bool QSettings::contains(const QString &key) const
{
   Q_D(const QSettings);
   QString k = d->actualKey(key);
   return d->get(k, nullptr);
}

void QSettings::setFallbacksEnabled(bool b)
{
   Q_D(QSettings);
   d->fallbacks = !!b;
}

bool QSettings::fallbacksEnabled() const
{
   Q_D(const QSettings);
   return d->fallbacks;
}

bool QSettings::event(QEvent *event)
{
   Q_D(QSettings);

   if (event->type() == QEvent::UpdateRequest) {
      d->update();
      return true;
   }

   return QObject::event(event);
}

QVariant QSettings::value(const QString &key, const QVariant &defaultValue) const
{
   Q_D(const QSettings);

   if (key.isEmpty()) {
      qWarning("QSettings::value() Key can not be empty");
      return QVariant();
   }

   QVariant result = defaultValue;
   QString k = d->actualKey(key);
   d->get(k, &result);

   return result;
}

void QSettings::setDefaultFormat(Format format)
{
   globalDefaultFormat = format;
}

QSettings::Format QSettings::defaultFormat()
{
   return globalDefaultFormat;
}

// obsolete
void QSettings::setSystemIniPath(const QString &dir)
{
   setPath(IniFormat, SystemScope, dir);

#if ! defined(Q_OS_WIN) && ! defined(Q_OS_DARWIN)
   setPath(NativeFormat, SystemScope, dir);
#endif
}

// obsolete
void QSettings::setUserIniPath(const QString &dir)
{
   setPath(IniFormat, UserScope, dir);

#if ! defined(Q_OS_WIN) && ! defined(Q_OS_DARWIN)
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
      WriteFunc writeFunc, Qt::CaseSensitivity caseSensitivity)
{
#ifdef QT_QSETTINGS_ALWAYS_CASE_SENSITIVE_AND_FORGET_ORIGINAL_KEY_ORDER
   Q_ASSERT(caseSensitivity == Qt::CaseSensitive);
#endif

   QMutexLocker locker(globalMutex());
   CustomFormatVector *customFormatVector = customFormatVectorFunc();
   int index = customFormatVector->size();

   if (index == 16) {
      // the QSettings::Format enum has room for 16 custom formats
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

#endif // QT_NO_SETTINGS
