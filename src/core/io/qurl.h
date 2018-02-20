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

#ifndef QURL_H
#define QURL_H

#include <qbytearray.h>
#include <qpair.h>
#include <qstring.h>
#include <qhash.h>

class QUrlPrivate;
class QUrlQuery;
class QDataStream;

#ifdef Q_OS_DARWIN
using CFURLRef = const struct __CFURL *;

#ifdef __OBJC__
@class NSURL;
#endif

#endif

class QUrl;
Q_CORE_EXPORT uint qHash(const QUrl &url, uint seed = 0);

class Q_CORE_EXPORT QUrl
{
 public:
   enum ParsingMode {
      TolerantMode,
      StrictMode,
      DecodedMode
   };

   // encoding / toString values
   enum FormattingOption {
      None            = 0x0,
      RemoveScheme    = 0x1,
      RemovePassword  = 0x2,
      RemoveUserInfo  = RemovePassword | 0x4,
      RemovePort      = 0x8,
      RemoveAuthority = RemoveUserInfo | RemovePort | 0x10,
      RemovePath      = 0x20,
      RemoveQuery     = 0x40,
      RemoveFragment  = 0x80,

      // 0x100: private: normalized

      PreferLocalFile        = 0x200,
      StripTrailingSlash     = 0x400,
      RemoveFilename         = 0x800,
      NormalizePathSegments  = 0x1000,

      PrettyDecoded    = 0x000000,
      EncodeSpaces     = 0x100000,
      EncodeUnicode    = 0x200000,
      EncodeDelimiters = 0x400000 | 0x800000,
      EncodeReserved   = 0x1000000,
      DecodeReserved   = 0x2000000,
      // 0x4000000 used to indicate full-decode mode

      FullyEncoded = EncodeSpaces | EncodeUnicode | EncodeDelimiters | EncodeReserved,
      FullyDecoded = FullyEncoded | DecodeReserved | 0x4000000
   };
   using FormattingOptions = QFlags<FormattingOption>;

   enum UserInputResolutionOption {
      DefaultResolution,
      AssumeLocalFile
   };
   using UserInputResolutionOptions = QFlags<UserInputResolutionOption>;

   QUrl();
   QUrl(const QUrl &other);

   explicit QUrl(const QString &url, ParsingMode mode = TolerantMode);

   QUrl(QUrl &&other)
      : d(other.d) {
      other.d = nullptr;
   }

   QUrl &operator =(const QUrl &other);

   QUrl &operator=(QUrl &&other) {
      qSwap(d, other.d);
      return *this;
   }

   ~QUrl();

   QUrl adjusted(FormattingOptions options) const Q_REQUIRED_RESULT;

   void clear();
   void detach();

   QString errorString() const;

   QString fileName(FormattingOptions options = FullyDecoded) const;

   static QUrl fromEncoded(const QByteArray &url, ParsingMode mode = TolerantMode);
   static QString fromPercentEncoding(const QByteArray &);
   static QUrl fromUserInput(const QString &userInput);

   static QUrl fromUserInput(const QString &userInput, const QString &workingDirectory,
                  UserInputResolutionOptions options = DefaultResolution);

   static QUrl fromLocalFile(const QString &localfile);

   bool hasFragment() const;
   bool hasQuery() const;

   bool isLocalFile() const;
   bool isValid() const;
   bool isEmpty() const;
   bool isRelative() const;
   bool isDetached() const;
   bool isParentOf(const QUrl &url) const;

   bool matches(const QUrl &url, FormattingOptions options) const;

   QUrl resolved(const QUrl &relative) const Q_REQUIRED_RESULT;

   void swap(QUrl &other) {
      qSwap(d, other.d);
   }

   QString topLevelDomain(FormattingOptions options = FullyDecoded) const;

   static QByteArray toPercentEncoding(const QString &, const QByteArray &exclude = QByteArray(),
                  const QByteArray &include = QByteArray());

   QString toLocalFile() const;
   QString toString(FormattingOptions options = FormattingOptions(PrettyDecoded)) const;
   QString toDisplayString(FormattingOptions options = FormattingOptions(PrettyDecoded)) const;
   QByteArray toEncoded(FormattingOptions options = FullyEncoded) const;

   //
   void setScheme(const QString &scheme);
   QString scheme() const;

   void setAuthority(const QString &authority, ParsingMode mode = TolerantMode);
   QString authority(FormattingOptions options = PrettyDecoded) const;

   void setUserInfo(const QString &userInfo, ParsingMode mode = TolerantMode);
   QString userInfo(FormattingOptions options = PrettyDecoded) const;

   void setUserName(const QString &userName, ParsingMode mode = DecodedMode);
   QString userName(FormattingOptions options = FullyDecoded) const;

   void setPassword(const QString &password, ParsingMode mode = DecodedMode);
   QString password(FormattingOptions options = FullyDecoded) const;

   void setHost(const QString &host, ParsingMode mode = DecodedMode);
   QString host(FormattingOptions options = FullyDecoded) const;

   void setPort(int port);
   int port(int defaultPort = -1) const;

   void setPath(const QString &path, ParsingMode mode = DecodedMode);
   QString path(FormattingOptions options = FullyDecoded) const;

   void setQuery(const QString &query, ParsingMode mode = TolerantMode);
   void setQuery(const QUrlQuery &query);
   QString query(FormattingOptions options = PrettyDecoded) const;

   void setUrl(const QString &url, ParsingMode mode = TolerantMode);
   QString url(FormattingOptions options = FormattingOptions(PrettyDecoded)) const;

   void setFragment(const QString &fragment, ParsingMode mode = TolerantMode);
   QString fragment(FormattingOptions options = PrettyDecoded) const;

   // operators
   bool operator <(const QUrl &url) const;
   bool operator ==(const QUrl &url) const;
   bool operator !=(const QUrl &url) const;

# ifdef Q_OS_DARWIN
   static QUrl fromCFURL(CFURLRef url);
   CFURLRef    toCFURL() const;

#if defined(__OBJC__)
   static QUrl fromNSURL(const NSURL *url);
   NSURL      *toNSURL() const;
#endif

#endif

   static QString fromPunycode(const QByteArray &punycode) {
      return fromAce(punycode);
   }

   static QByteArray toPunycode(const QString &string) {
      return toAce(string);
   }

   void addQueryItem(const QString &key, const QString &value);
   void setQueryItems(const QList<QPair<QString, QString> > &query);
   QList<QPair<QString, QString> > queryItems() const;

   bool hasQueryItem(const QString &key) const;
   QString queryItemValue(const QString &key) const;

   QStringList allQueryItemValues(const QString &key) const;

   void removeQueryItem(const QString &key);
   void removeAllQueryItems(const QString &key);

   void setEncodedQueryItems(const QList<QPair<QByteArray, QByteArray> > &query);
   void addEncodedQueryItem(const QByteArray &key, const QByteArray &value);
   QList<QPair<QByteArray, QByteArray> > encodedQueryItems() const;
   bool hasEncodedQueryItem(const QByteArray &key) const;
   QByteArray encodedQueryItemValue(const QByteArray &key) const;
   QList<QByteArray> allEncodedQueryItemValues(const QByteArray &key) const;
   void removeEncodedQueryItem(const QByteArray &key);
   void removeAllEncodedQueryItems(const QByteArray &key);

   void setEncodedUrl(const QByteArray &u, ParsingMode mode = TolerantMode) {
      setUrl(fromEncodedComponent_helper(u), mode);
   }

   QByteArray encodedUserName() const {
      return userName(FullyEncoded).toLatin1();
   }
   void setEncodedUserName(const QByteArray &value) {
      setUserName(fromEncodedComponent_helper(value));
   }

   QByteArray encodedPassword() const {
      return password(FullyEncoded).toLatin1();
   }
   void setEncodedPassword(const QByteArray &value) {
      setPassword(fromEncodedComponent_helper(value));
   }

   QByteArray encodedHost() const {
      return host(FullyEncoded).toLatin1();
   }
   void setEncodedHost(const QByteArray &value) {
      setHost(fromEncodedComponent_helper(value));
   }

   QByteArray encodedPath() const {
      return path(FullyEncoded).toLatin1();
   }
   void setEncodedPath(const QByteArray &value) {
      setPath(fromEncodedComponent_helper(value));
   }

   QByteArray encodedQuery() const {
      return toLatin1_helper(query(FullyEncoded));
   }
   void setEncodedQuery(const QByteArray &query) {
      setQuery(fromEncodedComponent_helper(query));
   }

   QByteArray encodedFragment() const {
      return toLatin1_helper(fragment(FullyEncoded));
   }
   void setEncodedFragment(const QByteArray &fragment) {
      setFragment(fromEncodedComponent_helper(fragment));
   }

   static QString fromAce(const QByteArray &);
   static QByteArray toAce(const QString &);
   static QStringList idnWhitelist();
   static QStringList toStringList(const QList<QUrl> &urls, FormattingOptions options = FormattingOptions(PrettyDecoded));
   static QList<QUrl> fromStringList(const QStringList &urls, ParsingMode mode = TolerantMode);
   static void setIdnWhitelist(const QStringList &);

   using DataPtr = QUrlPrivate *;

   inline DataPtr &data_ptr() {
      return d;
   }

 private:
   // helper function for the encodedQuery and encodedFragment functions
   static QByteArray toLatin1_helper(const QString &string) {
      if (string.isEmpty()) {
         return string.isNull() ? QByteArray() : QByteArray("");
      }

      return string.toLatin1();
   }

   static QString fromEncodedComponent_helper(const QByteArray &ba);

   QUrlPrivate *d;

   friend class QUrlQuery;
   friend Q_CORE_EXPORT uint qHash(const QUrl &url, uint seed);
};

Q_DECLARE_TYPEINFO(QUrl, Q_MOVABLE_TYPE);
Q_DECLARE_SHARED(QUrl)
Q_DECLARE_OPERATORS_FOR_FLAGS(QUrl::FormattingOptions)

Q_CORE_EXPORT QDataStream &operator<<(QDataStream &, const QUrl &);
Q_CORE_EXPORT QDataStream &operator>>(QDataStream &, QUrl &);

#endif
