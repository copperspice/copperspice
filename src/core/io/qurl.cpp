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

#include <qurl.h>

#include <qbytearray.h>
#include <qdebug.h>
#include <qdir.h>
#include <qplatformdefs.h>
#include <qstring.h>
#include <qstringlist.h>
#include <qurlquery.h>

#include <qipaddress_p.h>
#include <qtldurl_p.h>
#include <qurl_p.h>

#include <algorithm>

// source code located in qdir.cpp
extern QString cs_internal_normalizePath(const QString &name, bool allowUncPaths);

int minPositive(int value1, int value2)
{
   int retval = value1;

   if (value2 >= 0 && (value2 < value1 || retval < 0) ) {
      retval = value2;
   }

   return retval;
}

static inline bool isHex(char c)
{
   c |= 0x20;
   return (c >= '0' && c <= '9') || (c >= 'a' && c <= 'f');
}

static inline QString ftpScheme()
{
   return QString("ftp");
}

static inline QString fileScheme()
{
   return QString("file");
}

static inline QString webDavScheme()
{
   return QString("webdavs");
}

static inline QString webDavSslTag()
{
   return QString("@SSL");
}

class QUrlPrivate
{
 public:

   enum Section : uchar {
      Scheme    = 0x01,
      UserName  = 0x02,
      Password  = 0x04,
      UserInfo  = UserName | Password,
      Host      = 0x08,
      Port      = 0x10,
      Authority = UserInfo | Host | Port,
      Path      = 0x20,
      Hierarchy = Authority | Path,
      Query     = 0x40,
      Fragment  = 0x80,
      FullUrl   = 0xff
   };

   enum Flags : uchar {
      IsLocalFile = 0x01
   };

   enum ErrorCode {
      // the high byte of the error code matches the Section
      // the first item in each value must be the generic "Invalid xxx Error"
      InvalidSchemeError   = Scheme << 8,
      InvalidUserNameError = UserName << 8,
      InvalidPasswordError = Password << 8,

      InvalidRegNameError = Host << 8,
      InvalidIPv4AddressError,
      InvalidIPv6AddressError,
      InvalidCharacterInIPv6Error,
      InvalidIPvFutureError,
      HostMissingEndBracket,

      InvalidPortError = Port << 8,
      PortEmptyError,

      InvalidPathError = Path << 8,

      InvalidQueryError = Query << 8,

      InvalidFragmentError = Fragment << 8,

      // the following two cases are only possible in combination
      // with presence/absence of the authority and scheme. See validityError().

      AuthorityPresentAndPathIsRelative       = Authority << 8 | Path << 8 | 0x10000,
      RelativeUrlPathContainsColonBeforeSlash = Scheme << 8 | Authority << 8 | Path << 8 | 0x10000,

      NoError = 0
   };

   struct Error {
      QString source;
      ErrorCode code;
      int position;
   };

   QUrlPrivate();
   QUrlPrivate(const QUrlPrivate &other);

   ~QUrlPrivate();

   void parse(const QString &url, QUrl::ParsingMode parsingMode);
   bool isEmpty() const {
      return sectionIsPresent == 0 && port == -1 && path.isEmpty();
   }

   Error *cloneError() const;
   void clearError();
   void setError(ErrorCode errorCode, const QString &source, int supplement = -1);
   ErrorCode validityError(QString *source = nullptr, int *position = nullptr) const;

   bool validateComponent(Section section, const QString &input, int begin, int end);
   bool validateComponent(Section section, const QString &input) {
      return validateComponent(section, input, 0, uint(input.length()));
   }

   // no QString scheme() const;
   void appendAuthority(QString &appendTo, QUrl::FormattingOptions options, Section appendingTo) const;
   void appendUserInfo(QString &appendTo, QUrl::FormattingOptions options, Section appendingTo) const;
   void appendUserName(QString &appendTo, QUrl::FormattingOptions options) const;
   void appendPassword(QString &appendTo, QUrl::FormattingOptions options) const;
   void appendHost(QString &appendTo, QUrl::FormattingOptions options) const;
   void appendPath(QString &appendTo, QUrl::FormattingOptions options, Section appendingTo) const;
   void appendQuery(QString &appendTo, QUrl::FormattingOptions options, Section appendingTo) const;
   void appendFragment(QString &appendTo, QUrl::FormattingOptions options, Section appendingTo) const;

   // "end" parameters are like STL iterators: they point to one past the last valid element
   bool setScheme(const QString &value, int len, bool doSetError);
   void setAuthority(const QString &auth, int from, int end, QUrl::ParsingMode mode);
   void setUserInfo(const QString &userInfo, int from, int end);
   void setUserName(const QString &value, int from, int end);
   void setPassword(const QString &value, int from, int end);
   bool setHost(const QString &value, int from, int end, QUrl::ParsingMode mode);
   void setPath(const QString &value, int from, int end);
   void setQuery(const QString &value, int from, int end);
   void setFragment(const QString &value, int from, int end);

   bool hasScheme() const {
      return sectionIsPresent & Scheme;
   }

   bool hasAuthority() const {
      return sectionIsPresent & Authority;
   }

   bool hasUserInfo() const {
      return sectionIsPresent & UserInfo;
   }

   bool hasUserName() const {
      return sectionIsPresent & UserName;
   }

   bool hasPassword() const {
      return sectionIsPresent & Password;
   }

   bool hasHost() const {
      return sectionIsPresent & Host;
   }

   bool hasPort() const {
      return port != -1;
   }

   bool hasPath() const {
      return !path.isEmpty();
   }

   bool hasQuery() const {
      return sectionIsPresent & Query;
   }

   bool hasFragment() const {
      return sectionIsPresent & Fragment;
   }

   bool isLocalFile() const {
      return flags & IsLocalFile;
   }

   QString toLocalFile(QUrl::FormattingOptions options) const;
   QString mergePaths(const QString &relativePath) const;

   QAtomicInt ref;
   int port;

   QString scheme;
   QString userName;
   QString password;
   QString host;
   QString path;
   QString query;
   QString fragment;

   Error *m_error;

   // not used for:
   //  - Port (port == -1 means absence)
   //  - Path (there's no path delimiter, so we optimize its use out of existence)
   // Schemes are never supposed to be empty, but we keep the flag anyway

   uchar sectionIsPresent;
   uchar flags;

   // 32-bit: 2 bytes tail padding available
   // 64-bit: 6 bytes tail padding available
};

inline QUrlPrivate::QUrlPrivate()
   : ref(1), port(-1), m_error(nullptr), sectionIsPresent(0), flags(0)
{
}

inline QUrlPrivate::QUrlPrivate(const QUrlPrivate &copy)
   : ref(1), port(copy.port), scheme(copy.scheme), userName(copy.userName),
     password(copy.password), host(copy.host), path(copy.path), query(copy.query),
     fragment(copy.fragment), m_error(copy.cloneError()),
     sectionIsPresent(copy.sectionIsPresent), flags(copy.flags)
{
}

inline QUrlPrivate::~QUrlPrivate()
{
   delete m_error;
}

inline QUrlPrivate::Error *QUrlPrivate::cloneError() const
{
   return m_error ? new Error(*m_error) : nullptr;
}

inline void QUrlPrivate::clearError()
{
   delete m_error;
   m_error = nullptr;
}

inline void QUrlPrivate::setError(ErrorCode errorCode, const QString &source, int supplement)
{
   if (m_error) {
      // do not overwrite an error set in a previous section during parsing
      return;
   }

   m_error = new Error;
   m_error->code     = errorCode;
   m_error->source   = source;
   m_error->position = supplement;
}

#define decode(x) ushort(x)
#define leave(x)  ushort(0x100 | (x))
#define encode(x) ushort(0x200 | (x))

static const ushort userNameInIsolation[] = {
   decode(':'), // 0
   decode('@'), // 1
   decode(']'), // 2
   decode('['), // 3
   decode('/'), // 4
   decode('?'), // 5
   decode('#'), // 6

   decode('"'), // 7
   decode('<'),
   decode('>'),
   decode('^'),
   decode('\\'),
   decode('|'),
   decode('{'),
   decode('}'),
   0
};

static const ushort *const passwordInIsolation = userNameInIsolation + 1;
static const ushort *const pathInIsolation = userNameInIsolation + 5;
static const ushort *const queryInIsolation = userNameInIsolation + 6;
static const ushort *const fragmentInIsolation = userNameInIsolation + 7;

static const ushort userNameInUserInfo[] =  {
   encode(':'), // 0
   decode('@'), // 1
   decode(']'), // 2
   decode('['), // 3
   decode('/'), // 4
   decode('?'), // 5
   decode('#'), // 6

   decode('"'), // 7
   decode('<'),
   decode('>'),
   decode('^'),
   decode('\\'),
   decode('|'),
   decode('{'),
   decode('}'),
   0
};
static const ushort *const passwordInUserInfo = userNameInUserInfo + 1;

static const ushort userNameInAuthority[] = {
   encode(':'), // 0
   encode('@'), // 1
   encode(']'), // 2
   encode('['), // 3
   decode('/'), // 4
   decode('?'), // 5
   decode('#'), // 6

   decode('"'), // 7
   decode('<'),
   decode('>'),
   decode('^'),
   decode('\\'),
   decode('|'),
   decode('{'),
   decode('}'),
   0
};
static const ushort *const passwordInAuthority = userNameInAuthority + 1;

static const ushort userNameInUrl[] = {
   encode(':'), // 0
   encode('@'), // 1
   encode(']'), // 2
   encode('['), // 3
   encode('/'), // 4
   encode('?'), // 5
   encode('#'), // 6

   // no need to list encode(x) for the other characters
   0
};

static const ushort *const passwordInUrl = userNameInUrl + 1;
static const ushort *const pathInUrl = userNameInUrl + 5;
static const ushort *const queryInUrl = userNameInUrl + 6;
static const ushort *const fragmentInUrl = userNameInUrl + 6;

static inline void parseDecodedComponent(QString &data)
{
   data.replace('%', "%25");
}

static inline QString recodeFromUser(const QString &input, const ushort *actions, int from, int to)
{
   QString output;

   QString::const_iterator begin = input.begin() + from;
   QString::const_iterator end   = input.begin() + to;

   if (qt_urlRecode(output, begin, end, Qt::EmptyFlag, actions)) {
      return output;
   }

   return input.mid(from, to - from);
}

// appendXXXX functions: copy from the internal form to the external, user form
// the internal value is stored in its PrettyDecoded form so that case is easy
static inline void appendToUser(QString &appendTo, const QString &value, QUrl::FormattingOptions options,
      const ushort *actions)
{
   if (options == QUrl::PrettyDecoded) {
      appendTo += value;
      return;
   }

   if (! qt_urlRecode(appendTo, value.constBegin(), value.constEnd(), options, actions)) {
      appendTo += value;
   }
}

inline void QUrlPrivate::appendAuthority(QString &appendTo, QUrl::FormattingOptions options,
      Section appendingTo) const
{
   if ((options & QUrl::RemoveUserInfo) != QUrl::RemoveUserInfo) {
      appendUserInfo(appendTo, options, appendingTo);

      // add '@' only if we added anything
      if (hasUserName() || (hasPassword() && (options & QUrl::RemovePassword) == 0)) {
         appendTo += '@';
      }
   }

   appendHost(appendTo, options);

   if (! (options & QUrl::RemovePort) && port != -1) {
      appendTo += ':' + QString::number(port);
   }
}

inline void QUrlPrivate::appendUserInfo(QString &appendTo, QUrl::FormattingOptions options,
      Section appendingTo) const
{
   if (! hasUserInfo()) {
      return;
   }

   const ushort *userNameActions = nullptr;
   const ushort *passwordActions = nullptr;

   if (options & QUrl::EncodeDelimiters) {
      userNameActions = userNameInUrl;
      passwordActions = passwordInUrl;

   } else {
      switch (appendingTo) {
         case UserInfo:
            userNameActions = userNameInUserInfo;
            passwordActions = passwordInUserInfo;
            break;

         case Authority:
            userNameActions = userNameInAuthority;
            passwordActions = passwordInAuthority;
            break;

         case FullUrl:
            userNameActions = userNameInUrl;
            passwordActions = passwordInUrl;
            break;

         default:
            // can not happen
            break;
      }
   }

   if (! qt_urlRecode(appendTo, userName.constBegin(), userName.constEnd(), options, userNameActions)) {
      appendTo += userName;
   }

   if (options & QUrl::RemovePassword || !hasPassword()) {
      return;

   } else {
      appendTo += QLatin1Char(':');

      if (! qt_urlRecode(appendTo, password.constBegin(), password.constEnd(), options, passwordActions)) {
         appendTo += password;
      }
   }
}

inline void QUrlPrivate::appendUserName(QString &appendTo, QUrl::FormattingOptions options) const
{
   // only called from QUrl::userName()
   appendToUser(appendTo, userName, options,
         options & QUrl::EncodeDelimiters ? userNameInUrl : userNameInIsolation);
}

inline void QUrlPrivate::appendPassword(QString &appendTo, QUrl::FormattingOptions options) const
{
   // only called from QUrl::password()
   appendToUser(appendTo, password, options,
         options & QUrl::EncodeDelimiters ? passwordInUrl : passwordInIsolation);
}

inline void QUrlPrivate::appendPath(QString &appendTo, QUrl::FormattingOptions options, Section appendingTo) const
{
   QString thePath = path;

   if (options & QUrl::NormalizePathSegments) {
      thePath = cs_internal_normalizePath(path, false);
   }

   if (options & QUrl::RemoveFilename) {
      const int slash = path.lastIndexOf('/');

      if (slash == -1) {
         return;
      }

      thePath = path.left(slash + 1);
   }

   // check if we need to remove trailing slashes
   if (options & QUrl::StripTrailingSlash) {
      while (thePath.length() > 1 && thePath.endsWith(QLatin1Char('/'))) {
         thePath.chop(1);
      }
   }

   appendToUser(appendTo, thePath, options,
         appendingTo == FullUrl || options & QUrl::EncodeDelimiters ? pathInUrl : pathInIsolation);
}

inline void QUrlPrivate::appendFragment(QString &appendTo, QUrl::FormattingOptions options,
      Section appendingTo) const
{
   appendToUser(appendTo, fragment, options,
         options & QUrl::EncodeDelimiters ? fragmentInUrl :
         appendingTo == FullUrl ? nullptr : fragmentInIsolation);
}

inline void QUrlPrivate::appendQuery(QString &appendTo, QUrl::FormattingOptions options, Section appendingTo) const
{
   appendToUser(appendTo, query, options,
         appendingTo == FullUrl || options & QUrl::EncodeDelimiters ? queryInUrl : queryInIsolation);
}

inline bool QUrlPrivate::setScheme(const QString &value, int len, bool doSetError)
{
   // schemes are strictly RFC-compliant:
   //    scheme        = ALPHA *( ALPHA / DIGIT / "+" / "-" / "." )
   // we also lowercase the scheme

   // schemes in URLs are not allowed to be empty, but they can be in
   // "Relative URIs" which QUrl also supports. QUrl::setScheme does
   // not call us with len == 0, so this can only be from parse()

   scheme.clear();

   if (len == 0) {
      return false;
   }

   sectionIsPresent |= Scheme;

   // validate it
   int needsLowercasing = -1;
   QString::const_iterator p = value.begin();

   for (int i = 0; i < len; ++i, ++p) {

      if (*p >= 'a' && *p <= 'z') {
         continue;
      }

      if (*p >= 'A' && *p <= 'Z') {
         needsLowercasing = i;
         continue;
      }

      if (i) {
         if (*p >= '0' && *p <= '9') {
            continue;
         }

         if (*p == '+' || *p == '-' || *p == '.') {
            continue;
         }
      }

      // found something else, do not call setError needlessly
      // if called from parse(), it will try to recover

      if (doSetError) {
         setError(InvalidSchemeError, value, i);
      }

      return false;
   }

   scheme = value.left(len);

   if (needsLowercasing != -1) {
      scheme = scheme.toLower();
   }

   // did we set to the file protocol?
#ifdef Q_OS_WIN

   if (scheme == fileScheme() || scheme == webDavScheme()) {

#else

   if (scheme == fileScheme()) {

#endif
      flags |= IsLocalFile;

   } else {
      flags &= ~IsLocalFile;

   }

   return true;
}

inline void QUrlPrivate::setAuthority(const QString &auth, int from, int end, QUrl::ParsingMode mode)
{
   sectionIsPresent &= ~Authority;
   sectionIsPresent |= Host;

   // we never actually loop

   while (from != end) {
      int userInfoIndex = auth.indexOf('@', from);

      if (uint(userInfoIndex) < uint(end)) {
         setUserInfo(auth, from, userInfoIndex);

         if (mode == QUrl::StrictMode && !validateComponent(UserInfo, auth, from, userInfoIndex)) {
            break;
         }

         from = userInfoIndex + 1;
      }

      int colonIndex = auth.lastIndexOf(':', end - 1);

      if (colonIndex < from) {
         colonIndex = -1;
      }

      if (uint(colonIndex) < uint(end)) {
         if (auth.at(from).unicode() == '[') {
            // check if colonIndex isn't inside the "[...]" part
            int closingBracket = auth.indexOf(QLatin1Char(']'), from);

            if (uint(closingBracket) > uint(colonIndex)) {
               colonIndex = -1;
            }
         }
      }

      if (colonIndex == end - 1) {
         // found a colon but no digits after it
         port = -1;

      } else if (uint(colonIndex) < uint(end)) {
         unsigned long x = 0;

         for (int i = colonIndex + 1; i < end; ++i) {
            ushort c = auth.at(i).unicode();

            if (c >= '0' && c <= '9') {
               x *= 10;
               x += c - '0';
            } else {
               x = ulong(-1); // x != ushort(x)
               break;
            }
         }

         if (x == ushort(x)) {
            port = ushort(x);

         } else {
            setError(InvalidPortError, auth, colonIndex + 1);

            if (mode == QUrl::StrictMode) {
               break;
            }
         }

      } else {
         port = -1;
      }

      int tmpValue = minPositive(end, colonIndex);
      setHost(auth, from, tmpValue, mode);

      if (mode == QUrl::StrictMode && ! validateComponent(Host, auth, from, tmpValue)) {
         // mark host as not present
         sectionIsPresent &= ~Authority;
         break;
      }

      // success
      return;
   }

   // clear all sections but host
   sectionIsPresent &= ~Authority | Host;
   userName.clear();
   password.clear();
   host.clear();
   port = -1;
}

inline void QUrlPrivate::setUserInfo(const QString &userInfo, int from, int end)
{
   int delimIndex = userInfo.indexOf(':', from);

   setUserName(userInfo, from, minPositive(end, delimIndex));

   if (uint(delimIndex) >= uint(end)) {
      password.clear();
      sectionIsPresent &= ~Password;

   } else {
      setPassword(userInfo, delimIndex + 1, end);
   }
}

inline void QUrlPrivate::setUserName(const QString &value, int from, int end)
{
   sectionIsPresent |= UserName;
   userName = recodeFromUser(value, userNameInIsolation, from, end);
}

inline void QUrlPrivate::setPassword(const QString &value, int from, int end)
{
   sectionIsPresent |= Password;
   password = recodeFromUser(value, passwordInIsolation, from, end);
}

inline void QUrlPrivate::setPath(const QString &value, int from, int end)
{
   // sectionIsPresent |= Path; // not used, save some cycles
   path = recodeFromUser(value, pathInIsolation, from, end);
}

inline void QUrlPrivate::setFragment(const QString &value, int from, int end)
{
   sectionIsPresent |= Fragment;
   fragment = recodeFromUser(value, fragmentInIsolation, from, end);
}

inline void QUrlPrivate::setQuery(const QString &value, int from, int iend)
{
   sectionIsPresent |= Query;
   query = recodeFromUser(value, queryInIsolation, from, iend);
}

inline void QUrlPrivate::appendHost(QString &appendTo, QUrl::FormattingOptions options) const
{
   // EncodeUnicode is the only flag that matters
   if ((options & QUrl::FullyDecoded) == QUrl::FullyDecoded) {
      options = Qt::EmptyFlag;
   } else {
      options &= QUrl::EncodeUnicode;
   }

   if (host.isEmpty()) {
      return;
   }

   if (host.at(0).unicode() == '[') {
      // IPv6Address and IPvFuture address never require any transformation
      appendTo += host;

   } else {
      // this is either an IPv4Address or a reg-name
      // if it is a reg-name, it is already stored in Unicode form
      if (options & QUrl::EncodeUnicode && !(options & 0x4000000)) {
         appendTo += qt_ACE_do(host, ToAceOnly, AllowLeadingDot);
      } else {
         appendTo += host;
      }
   }
}

static QString::const_iterator parseIpFuture(QString &host, QString::const_iterator begin,
   QString::const_iterator end, QUrl::ParsingMode mode)
{
   //    IPvFuture     = "v" 1*HEXDIG "." 1*( unreserved / sub-delims / ":" )

   static const QString acceptable =
         "!$&'()*+,;="          // sub-delims
         ":"                    // ":"
         "-._~";                // unreserved

   // the brackets and the "v" have been checked
   QString::const_iterator origBegin = begin;

   if (begin[3] != '.') {
      return begin + 3;
   }

   if ((begin[2] >= 'A' && begin[2] <= 'F') || (begin[2] >= 'a' && begin[2] <= 'f') ||
         (begin[2] >= '0' && begin[2] <= '9')) {

      // very unlikely, decode the whole string skipping the "[vH." and "]" which we already know to be there
      host += QString(begin, begin + 4);

      // uppercase the version if necessary
      if (begin[2] >= 'a') {
         host.replace(host.end() - 2, host.end() - 1, begin[2].toLower() );
      }

      begin += 4;
      --end;

      QString decoded;

      if (mode == QUrl::TolerantMode && qt_urlRecode(decoded, begin, end, QUrl::FullyDecoded, nullptr)) {
         begin = decoded.constBegin();
         end   = decoded.constEnd();
      }

      for ( ; begin != end; ++begin) {
         if (*begin >= 'A' && *begin <= 'Z') {
            host += *begin;

         } else if (*begin >= 'a' && *begin <= 'z') {
            host += *begin;

         } else if (*begin >= '0' && *begin <= '9') {
            host += *begin;

         } else if (*begin < 0x80 && acceptable.contains(*begin)) {
            host += *begin;

         } else {
            return decoded.isEmpty() ? begin : origBegin + 2;
         }
      }

      host += ']';

      return end;
   }

   return origBegin + 2;
}

// ONLY the IPv6 address is parsed here, WITHOUT the brackets
static QString::const_iterator parseIp6(QString &host, QString::const_iterator begin, QString::const_iterator end, QUrl::ParsingMode mode)
{
   QIPAddressUtils::IPv6Address address;
   QString::const_iterator iter = QIPAddressUtils::parseIp6(address, begin, end);

   if (iter != end) {
      // this struct is kept in automatic storage because it's only 4 bytes
      const ushort decodeColon[] = { decode(':'), 0 };

      // IPv6 failed parsing, check if it was a percent-encoded character in the middle and try again
      QString decoded;

      if (mode == QUrl::TolerantMode && qt_urlRecode(decoded, begin, end, Qt::EmptyFlag, decodeColon)) {
         // recurse if the parsing fails again, the qt_urlRecode above will return end
         return parseIp6(host, decoded.constBegin(), decoded.constEnd(), mode);
      }

      // no transformation, nothing to re-parse
      return iter;
   }

   host += '[';
   QIPAddressUtils::toString(host, address);
   host += ']';

   return end;
}

inline bool QUrlPrivate::setHost(const QString &value, int xfrom, int xend, QUrl::ParsingMode mode)
{
   QString::const_iterator begin = value.constBegin() + xfrom;
   QString::const_iterator end   = value.constBegin() + xend;

   const int len = end - begin;

   host.clear();
   sectionIsPresent |= Host;

   if (len == 0) {
      return true;
   }

   if (begin[0] == '[') {
      // IPv6Address or IPvFuture
      // smallest IPv6 address is      "[::]"   (len = 4)
      // smallest IPvFuture address is "[v7.X]" (len = 6)

      QString::const_iterator iter;

      if (end[-1] != ']') {
         setError(HostMissingEndBracket, value);
         return false;
      }

      if (len > 5 && begin[1] == 'v') {
         iter = parseIpFuture(host, begin, end, mode);

         if (iter == end) {
            return true;

         } else {
            setError(InvalidIPvFutureError, value, iter - value.begin());
            return false;
         }

      } else if (begin[1].unicode() == 'v') {
         setError(InvalidIPvFutureError, value, xfrom);

      }

      iter = parseIp6(host, begin + 1, end - 1, mode);

      if (iter == end) {
         return true;
      }

      if (iter == end - 1) {
         setError(InvalidIPv6AddressError, value, xfrom);

      } else {
         setError(InvalidCharacterInIPv6Error, value, iter - value.begin());

      }

      return false;
   }

   // check if it is an IPv4 address
   QIPAddressUtils::IPv4Address ip4;

   if (QIPAddressUtils::parseIp4(ip4, begin, end)) {
      QIPAddressUtils::toString(host, ip4);
      return true;
   }

   // This is probably a regular name.
   // Can also be an encoded string when decoded becomes one of the types above.
   //
   // Two types of encoding are possible:
   //  percent encoding (e.g., "%31%30%2E%30%2E%30%2E%31" -> "10.0.0.1")
   //  Unicode encoding (some non-ASCII characters case-fold to digits
   //                    when nameprepping is done)
   //
   // The qt_ACE_do function below applies nameprepping and the STD3 check.
   // That means a Unicode string may become an IPv4 address, but it cannot
   // produce a '[' or a '%'.

   // check for percent-encoding first
   QString s;

   if (mode == QUrl::TolerantMode && qt_urlRecode(s, begin, end, Qt::EmptyFlag, nullptr)) {
      // something was decoded, anything encoded left?
      int pos = s.indexOf('%'); // '%'

      if (pos != -1) {
         setError(InvalidRegNameError, s, pos);
         return false;
      }

      // recurse
      return setHost(s, 0, s.length(), QUrl::StrictMode);
   }

   s = qt_ACE_do(QStringView(begin, end), NormalizeAce, ForbidLeadingDot);

   if (s.isEmpty()) {
      setError(InvalidRegNameError, value);
      return false;
   }

   // check IPv4 again
   if (QIPAddressUtils::parseIp4(ip4, s.constBegin(), s.constEnd())) {
      QIPAddressUtils::toString(host, ip4);
   } else {
      host = s;
   }

   return true;
}

inline void QUrlPrivate::parse(const QString &url, QUrl::ParsingMode parsingMode)
{
   //   URI-reference = URI / relative-ref
   //   URI           = scheme ":" hier-part [ "?" query ] [ "#" fragment ]
   //   relative-ref  = relative-part [ "?" query ] [ "#" fragment ]
   //   hier-part     = "//" authority path-abempty
   //                 / other path types
   //   relative-part = "//" authority path-abempty
   //                 /  other path types here

   sectionIsPresent = 0;
   flags = 0;

   clearError();

   // find the important delimiters
   int colon    = -1;
   int question = -1;
   int hash     = -1;

   const int len = url.length();

   QString::const_iterator iter = url.begin();

   for (int i = 0; i < len; ++i) {
      QChar32 uc = iter[i];

      if (uc == '#' && hash == -1) {
         hash = i;

         // nothing more to be found
         break;
      }

      if (question == -1) {
         if (uc == ':' && colon == -1) {
            colon = i;

         } else if (uc == '?') {
            question = i;
         }
      }
   }

   // check if we have a scheme
   int hierStart;

   if (colon != -1 && setScheme(url, colon, false)) {
      hierStart = colon + 1;

   } else {
      // recover from a failed scheme, it might not have been a scheme at all
      scheme.clear();
      sectionIsPresent = 0;
      hierStart = 0;
   }

   int pathStart;
   int hierEnd = len;

   if (question != -1 && question < hierEnd) {
      hierEnd = question;
   }

   if (hash != -1 && hash < hierEnd) {
      hierEnd = hash;
   }

   if (hierEnd - hierStart >= 2 && url[hierStart] == '/' && url[hierStart + 1] == '/') {
      // we have an authority, it ends at the first slash after these
      int authorityEnd = hierEnd;

      for (int i = hierStart + 2; i < authorityEnd ; ++i) {
         if (url[i] == '/') {
            authorityEnd = i;
            break;
         }
      }

      setAuthority(url, hierStart + 2, authorityEnd, parsingMode);

      // even if we failed to set the authority properly, try to recover
      pathStart = authorityEnd;
      setPath(url, pathStart, hierEnd);

   } else {
      userName.clear();
      password.clear();
      host.clear();

      port      = -1;
      pathStart = hierStart;

      if (hierStart < hierEnd) {
         setPath(url, hierStart, hierEnd);
      } else {
         path.clear();
      }
   }

   if (uint(question) < uint(hash)) {
      setQuery(url, question + 1, minPositive(hash, len));
   }

   if (hash != -1) {
      setFragment(url, hash + 1, len);
   }

   if (m_error || parsingMode == QUrl::TolerantMode) {
      return;
   }

   // The parsing so far was partially tolerant of errors, except for the scheme parser
   // (which is always strict) and the authority (which was executed in strict mode).
   // If we have not found any errors so far, continue the strict-mode parsing from the path component onwards.

   if (! validateComponent(Path, url, pathStart, hierEnd)) {
      return;
   }

   if (uint(question) < uint(hash) && ! validateComponent(Query, url, question + 1, minPositive(hash, len))) {
      return;
   }

   if (hash != -1) {
      validateComponent(Fragment, url, hash + 1, len);
   }
}

QString QUrlPrivate::toLocalFile(QUrl::FormattingOptions options) const
{
   QString retval;

   QString ourPath;
   appendPath(ourPath, options, QUrlPrivate::Path);

   // magic for shared drive on windows
   if (! host.isEmpty()) {
      retval = "//" + host;

#ifdef Q_OS_WIN

      if (scheme == webDavScheme()) {
         retval += webDavSslTag();
      }

#endif

      if (! ourPath.isEmpty() && ! ourPath.startsWith('/')) {
         retval += '/';
      }

      retval += ourPath;

   } else {
      retval = ourPath;

#ifdef Q_OS_WIN

      // magic for drives on windows
      if (ourPath.length() > 2 && ourPath.at(0) == '/' && ourPath.at(2) == ':') {
         retval.remove(0, 1);
      }

#endif

   }

   return retval;
}

inline QString QUrlPrivate::mergePaths(const QString &relativePath) const
{
   // If the base URI has a defined authority component and an empty
   // path, then return a string consisting of "/" concatenated with
   // the reference's path; otherwise

   if (!host.isEmpty() && path.isEmpty()) {
      return QLatin1Char('/') + relativePath;
   }

   // Return a string consisting of the reference's path component
   // appended to all but the last segment of the base URI's path
   // (i.e., excluding any characters after the right-most "/" in the
   // base URI path, or excluding the entire base URI path if it does
   // not contain any "/" characters).

   QString newPath;

   if (! path.contains('/')) {
      newPath = relativePath;
   } else {
      newPath = path.left(path.lastIndexOf('/') + 1) + relativePath;
   }

   return newPath;
}

static void removeDotsFromPath(QString &path)
{
   // From http://www.ietf.org/rfc/rfc3986.txt, 5.2.4

   // input buffer is initialized with the appended path components
   // output buffer is initialized to the empty string

   QString retval;

   QString::const_iterator in  = path.begin();
   QString::const_iterator end = path.end();

   // If the input buffer consists only of "." or "..", then remove that from the input buffer
   if (path == ".") {
      ++in;

   } else if (path == "..") {
      in += 2;
   }

   while (in < end) {

      // if the input buffer begins with a prefix of "../" or "./", remove that prefix from the input buffer;
      QStringView tmp(in, end);

      if (tmp.startsWith("./")) {
         in += 2;

      } else if (tmp.startsWith("../")) {
         in += 3;
      }

      // if the input buffer begins with a prefix of "/./" or "/.", where "." is a complete path segment
      // then replace that prefix with "/" in the input buffer;

      tmp = QStringView(in, end);

      if (tmp.startsWith("/./")) {
         in += 2;
         continue;

      } else if (tmp.startsWith("/.")) {
         retval.append("/");
         in += 2;
         break;
      }

      // if the input buffer begins with a prefix of "/../" or "/..", where ".." is a complete path segment
      // then replace that prefix with "/" in the input buffer and remove the last segment and its
      // preceding "/" (if any) from the output buffer

      if (tmp.startsWith("/../")) {

         while (retval.size() > 1 && ! retval.endsWith('/')) {
            retval.chop(1);
         }

         if (retval.size() == 1 && retval != "/") {
            ++in;
         }

         in += 3;
         continue;

      } else if (tmp == "/..") {

         while (retval.size() > 1 && ! retval.endsWith('/')) {

            if (retval.startsWith("/") && retval.size() == 2) {
               break;
            }

            retval.chop(1);
         }

         in += 3;
         break;
      }

      // move the first path segment in the input buffer to the end of the output buffer,
      // including the initial "/" character (if any) and any subsequent characters up to,
      // but not including, the next "/" character or the end of the input buffer.

      retval.append(*in);
      ++in;

      while (in != end && *in != '/') {
         retval.append(*in);
         ++in;
      }
   }

   path = retval;
}

inline QUrlPrivate::ErrorCode QUrlPrivate::validityError(QString *source, int *position) const
{
   Q_ASSERT(! source == ! position);

   if (m_error) {
      if (source != nullptr) {
         *source   = m_error->source;
         *position = m_error->position;
      }

      return m_error->code;
   }

   // There are two more cases of invalid URLs that QUrl recognizes and they are only possible with
   //  constructed URLs (setXXX methods), not with parsing. Therefore, they are tested here.
   //
   // The two cases are a non-empty path that does not start with a slash and:
   //  - with an authority
   //  - without an authority, without scheme but the path with a colon before the first slash
   //
   // Those cases are considered invalid because toString() would produce a URL
   // that would not be parsed back to the same QUrl.

   if (path.isEmpty() || path.at(0) == '/') {
      return NoError;
   }

   if (sectionIsPresent & QUrlPrivate::Host) {
      if (source) {
         *source = path;
         *position = 0;
      }

      return AuthorityPresentAndPathIsRelative;
   }

   if (sectionIsPresent & QUrlPrivate::Scheme) {
      return NoError;
   }

   // check for a path of "text:text/"
   for (int i = 0; i < path.length(); ++i) {
      ushort c = path.at(i).unicode();

      if (c == '/') {
         // found the slash before the colon
         return NoError;
      }

      if (c == ':') {
         // found the colon before the slash, it is invalid
         if (source) {
            *source = path;
            *position = i;
         }

         return RelativeUrlPathContainsColonBeforeSlash;
      }
   }

   return NoError;
}

bool QUrlPrivate::validateComponent(QUrlPrivate::Section section, const QString &input, int begin, int end)
{
   // What we need to look out for, that the regular parser tolerates:
   //  - percent signs not followed by two hex digits
   //  - forbidden characters, which should always appear encoded
   //    '"' / '<' / '>' / '\' / '^' / '`' / '{' / '|' / '}' / BKSP
   //    control characters
   //  - delimiters not allowed in certain positions
   //    . scheme: parser is already strict
   //    . user info: gen-delims except ":" disallowed ("/" / "?" / "#" / "[" / "]" / "@")
   //    . host: parser is stricter than the standard
   //    . port: parser is stricter than the standard
   //    . path: all delimiters allowed
   //    . fragment: all delimiters allowed
   //    . query: all delimiters allowed

   static const char forbidden[] = "\"<>\\^`{|}\x7F";
   static const char forbiddenUserInfo[] = ":/?#[]@";

   Q_ASSERT(section != Authority && section != Hierarchy && section != FullUrl);

   QString::const_iterator iter_begin = input.constBegin() + begin;
   QString::const_iterator iter_end   = iter_begin + (end - begin);

   for (auto iter = iter_begin; iter != iter_end; ++iter) {
      char32_t uc = iter->unicode();

      if (uc >= 0x80) {
         continue;
      }

      bool error = false;

      if ((uc == '%' && (iter + 1 != iter_end || iter + 2 != iter_end || ! isHex(iter[1].unicode()) ||
            ! isHex(iter[2].unicode()))) || uc <= 0x20 || strchr(forbidden, uc)) {
         // found an error
         error = true;

      } else if (section & UserInfo) {
         if (section == UserInfo && strchr(forbiddenUserInfo + 1, uc)) {
            error = true;

         } else if (section != UserInfo && strchr(forbiddenUserInfo, uc)) {
            error = true;
         }
      }

      if (! error) {
         continue;
      }

      ErrorCode errorCode = ErrorCode(int(section) << 8);

      if (section == UserInfo) {
         // is it the user name or the password?
         errorCode = InvalidUserNameError;

         for (auto temp = iter_begin; temp != iter; ++temp) {

            if (*temp == ':') {
               errorCode = InvalidPasswordError;
               break;
            }
         }
      }

      setError(errorCode, input, iter - input.constBegin());
      return false;
   }

   // no errors
   return true;
}

QUrl::QUrl(const QString &url, ParsingMode parsingMode)
   : d(nullptr)
{
   setUrl(url, parsingMode);
}

QUrl::QUrl()
   : d(nullptr)
{
}

QUrl::QUrl(const QUrl &other) : d(other.d)
{
   if (d) {
      d->ref.ref();
   }
}

QUrl::~QUrl()
{
   if (d && ! d->ref.deref()) {
      delete d;
   }
}

bool QUrl::isValid() const
{
   if (isEmpty()) {
      // also catches d == 0
      return false;
   }

   return d->validityError() == QUrlPrivate::NoError;
}

bool QUrl::isEmpty() const
{
   if (! d) {
      return true;
   }

   return d->isEmpty();
}

void QUrl::clear()
{
   if (d && !d->ref.deref()) {
      delete d;
   }

   d = nullptr;
}

void QUrl::setUrl(const QString &url, ParsingMode parsingMode)
{
   if (parsingMode == DecodedMode) {
      qWarning("QUrl::setUrl() Unable to parse a full URL in QUrl::DecodedMode");

   } else {
      detach();
      d->parse(url, parsingMode);
   }
}

void QUrl::setScheme(const QString &scheme)
{
   detach();
   d->clearError();

   if (scheme.isEmpty()) {
      // schemes are not allowed to be empty
      d->sectionIsPresent &= ~QUrlPrivate::Scheme;
      d->flags &= ~QUrlPrivate::IsLocalFile;
      d->scheme.clear();

   } else {
      // do set error
      d->setScheme(scheme, scheme.length(), true);
   }
}

QString QUrl::scheme() const
{
   if (! d) {
      return QString();
   }

   return d->scheme;
}

void QUrl::setAuthority(const QString &authority, ParsingMode mode)
{
   detach();
   d->clearError();

   if (mode == DecodedMode) {
      qWarning("QUrl::setAuthority() Unable to set authority in QUrl::DecodedMode");
      return;
   }

   d->setAuthority(authority, 0, authority.length(), mode);

   if (authority.isEmpty()) {
      // QUrlPrivate::setAuthority cleared almost everything
      // but it leaves the Host bit set
      d->sectionIsPresent &= ~QUrlPrivate::Authority;
   }
}

QString QUrl::authority(FormattingOptions options) const
{
   if (! d) {
      return QString();
   }

   if (options == QUrl::FullyDecoded) {
      qWarning("QUrl::authority() Unable to set authority in QUrl::DecodedMode");

      return QString();
   }

   QString result;
   d->appendAuthority(result, options, QUrlPrivate::Authority);

   return result;
}

void QUrl::setUserInfo(const QString &userInfo, ParsingMode mode)
{
   detach();
   d->clearError();

   QString trimmed = userInfo.trimmed();

   if (mode == DecodedMode) {
      qWarning("QUrl::setUserInfo() Unable to set user information in QUrl::DecodedMode");
      return;
   }

   d->setUserInfo(trimmed, 0, trimmed.length());

   if (userInfo.isEmpty()) {
      // QUrlPrivate::setUserInfo cleared almost everything
      // but it leaves the UserName bit set
      d->sectionIsPresent &= ~QUrlPrivate::UserInfo;

   } else if (mode == StrictMode && !d->validateComponent(QUrlPrivate::UserInfo, userInfo)) {
      d->sectionIsPresent &= ~QUrlPrivate::UserInfo;
      d->userName.clear();
      d->password.clear();
   }
}

QString QUrl::userInfo(FormattingOptions options) const
{
   if (! d) {
      return QString();
   }

   if (options == QUrl::FullyDecoded) {
      qWarning("QUrl::userInfo() Unable to set user information in QUrl::DecodedMode");
      return QString();
   }

   QString result;
   d->appendUserInfo(result, options, QUrlPrivate::UserInfo);
   return result;
}

void QUrl::setUserName(const QString &userName, ParsingMode mode)
{
   detach();
   d->clearError();

   QString data = userName;

   if (mode == DecodedMode) {
      parseDecodedComponent(data);
      mode = TolerantMode;
   }

   d->setUserName(data, 0, data.length());

   if (userName.isEmpty()) {
      d->sectionIsPresent &= ~QUrlPrivate::UserName;
   } else if (mode == StrictMode && !d->validateComponent(QUrlPrivate::UserName, userName)) {
      d->userName.clear();
   }
}

QString QUrl::userName(FormattingOptions options) const
{
   if (!d) {
      return QString();
   }

   QString result;
   d->appendUserName(result, options);
   return result;
}

void QUrl::setPassword(const QString &password, ParsingMode mode)
{
   detach();
   d->clearError();

   QString data = password;

   if (mode == DecodedMode) {
      parseDecodedComponent(data);
      mode = TolerantMode;
   }

   d->setPassword(data, 0, data.length());

   if (password.isEmpty()) {
      d->sectionIsPresent &= ~QUrlPrivate::Password;
   } else if (mode == StrictMode && !d->validateComponent(QUrlPrivate::Password, password)) {
      d->password.clear();
   }
}

QString QUrl::password(FormattingOptions options) const
{
   if (!d) {
      return QString();
   }

   QString result;
   d->appendPassword(result, options);
   return result;
}

void QUrl::setHost(const QString &host, ParsingMode mode)
{
   detach();
   d->clearError();

   QString data = host;

   if (mode == DecodedMode) {
      parseDecodedComponent(data);
      mode = TolerantMode;
   }

   if (d->setHost(data, 0, data.length(), mode)) {
      if (host.isEmpty()) {
         d->sectionIsPresent &= ~QUrlPrivate::Host;
      }

   } else if (! data.startsWith('[')) {
      // setHost failed, it might be IPv6 or IPvFuture in need of bracketing
      Q_ASSERT(d->m_error);

      data.prepend(QLatin1Char('['));
      data.append(QLatin1Char(']'));

      if (!d->setHost(data, 0, data.length(), mode)) {
         // failed again
         if (data.contains(QLatin1Char(':'))) {
            // source data contains ':', so it's an IPv6 error
            d->m_error->code = QUrlPrivate::InvalidIPv6AddressError;
         }

      } else {
         // succeeded
         d->clearError();
      }
   }
}

QString QUrl::host(FormattingOptions options) const
{
   if (! d) {
      return QString();
   }

   QString result;
   d->appendHost(result, options);

   if (result.startsWith(QChar('['))) {
      return result.mid(1, result.length() - 2);
   }

   return result;
}

void QUrl::setPort(int port)
{
   detach();
   d->clearError();

   if (port < -1 || port > 65535) {
      d->setError(QUrlPrivate::InvalidPortError, QString::number(port), 0);
      port = -1;
   }

   d->port = port;
}

int QUrl::port(int defaultPort) const
{
   if (! d) {
      return defaultPort;
   }

   return d->port == -1 ? defaultPort : d->port;
}

void QUrl::setPath(const QString &path, ParsingMode mode)
{
   detach();
   d->clearError();

   QString data = path;

   if (mode == DecodedMode) {
      parseDecodedComponent(data);
      mode = TolerantMode;
   }

   int from = 0;

   while (from < data.length() - 2 && data.midView(from, 2) == "//") {
      ++from;
   }

   d->setPath(data, from, data.length());

   if (mode == StrictMode && !d->validateComponent(QUrlPrivate::Path, path)) {
      d->path.clear();
   }
}

QString QUrl::path(FormattingOptions options) const
{
   if (! d) {
      return QString();
   }

   QString result;
   d->appendPath(result, options, QUrlPrivate::Path);
   return result;
}

QString QUrl::fileName(FormattingOptions options) const
{
   const QString ourPath = path(options);
   const int slash = ourPath.lastIndexOf(QChar('/'));

   if (slash == -1) {
      return ourPath;
   }

   return ourPath.mid(slash + 1);
}

bool QUrl::hasQuery() const
{
   if (! d) {
      return false;
   }

   return d->hasQuery();
}

void QUrl::setQuery(const QString &query, ParsingMode mode)
{
   detach();
   d->clearError();

   QString data = query;

   if (mode == DecodedMode) {
      parseDecodedComponent(data);
      mode = TolerantMode;
   }

   d->setQuery(data, 0, data.length());

   if (query.isEmpty()) {
      d->sectionIsPresent &= ~QUrlPrivate::Query;
   } else if (mode == StrictMode && ! d->validateComponent(QUrlPrivate::Query, query)) {
      d->query.clear();
   }
}

void QUrl::setQuery(const QUrlQuery &query)
{
   detach();
   d->clearError();

   // we know the data is in the right format
   d->query = query.toString();

   if (query.isEmpty()) {
      d->sectionIsPresent &= ~QUrlPrivate::Query;
   } else {
      d->sectionIsPresent |= QUrlPrivate::Query;
   }
}

QString QUrl::query(FormattingOptions options) const
{
   if (! d) {
      return QString();
   }

   QString result;
   d->appendQuery(result, options, QUrlPrivate::Query);

   return result;
}

void QUrl::setFragment(const QString &fragment, ParsingMode mode)
{
   detach();
   d->clearError();

   QString data = fragment;

   if (mode == DecodedMode) {
      parseDecodedComponent(data);
      mode = TolerantMode;
   }

   d->setFragment(data, 0, data.length());

   if (fragment.isEmpty()) {
      d->sectionIsPresent &= ~QUrlPrivate::Fragment;
   } else if (mode == StrictMode && !d->validateComponent(QUrlPrivate::Fragment, fragment)) {
      d->fragment.clear();
   }
}

QString QUrl::fragment(FormattingOptions options) const
{
   if (!d) {
      return QString();
   }

   QString result;
   d->appendFragment(result, options, QUrlPrivate::Fragment);

   return result;
}

bool QUrl::hasFragment() const
{
   if (!d) {
      return false;
   }

   return d->hasFragment();
}

QString QUrl::topLevelDomain(FormattingOptions options) const
{
   QString tld = qTopLevelDomain(host());

   if (options & EncodeUnicode) {
      return qt_ACE_do(tld, ToAceOnly, AllowLeadingDot);
   }

   return tld;
}

QUrl QUrl::resolved(const QUrl &relative) const
{
   if (! d) {
      return relative;
   }

   if (! relative.d) {
      return *this;
   }

   QUrl t;

   // Compatibility (mostly for qtdeclarative), treat "file:relative.txt" as relative even
   // though QUrl::isRelative() says false

   if (! relative.d->scheme.isEmpty() && (! relative.isLocalFile() || QDir::isAbsolutePath(relative.d->path))) {
      t = relative;
      t.detach();

   } else {
      if (relative.d->hasAuthority()) {
         t = relative;
         t.detach();

      } else {
         t.d = new QUrlPrivate;

         // copy the authority
         t.d->userName = d->userName;
         t.d->password = d->password;
         t.d->host = d->host;
         t.d->port = d->port;
         t.d->sectionIsPresent = d->sectionIsPresent & QUrlPrivate::Authority;

         if (relative.d->path.isEmpty()) {
            t.d->path = d->path;

            if (relative.d->hasQuery()) {
               t.d->query = relative.d->query;
               t.d->sectionIsPresent |= QUrlPrivate::Query;

            } else if (d->hasQuery()) {
               t.d->query = d->query;
               t.d->sectionIsPresent |= QUrlPrivate::Query;
            }

         } else {
            t.d->path = relative.d->path.startsWith('/') ? relative.d->path : d->mergePaths(relative.d->path);

            if (relative.d->hasQuery()) {
               t.d->query = relative.d->query;
               t.d->sectionIsPresent |= QUrlPrivate::Query;
            }
         }
      }

      t.d->scheme = d->scheme;

      if (d->hasScheme()) {
         t.d->sectionIsPresent |= QUrlPrivate::Scheme;
      } else {
         t.d->sectionIsPresent &= ~QUrlPrivate::Scheme;
      }

      t.d->flags |= d->flags & QUrlPrivate::IsLocalFile;
   }

   t.d->fragment = relative.d->fragment;

   if (relative.d->hasFragment()) {
      t.d->sectionIsPresent |= QUrlPrivate::Fragment;
   } else {
      t.d->sectionIsPresent &= ~QUrlPrivate::Fragment;
   }

   removeDotsFromPath(t.d->path);

   return t;
}

bool QUrl::isRelative() const
{
   if (! d) {
      return true;
   }

   return ! d->hasScheme();
}

QString QUrl::url(FormattingOptions options) const
{
   return toString(options);
}

QString QUrl::toString(FormattingOptions options) const
{
   if (! isValid()) {
      // also catches isEmpty()
      return QString();
   }

   if (options == QUrl::FullyDecoded) {
      qWarning("QUrl::toString() Unable to generate string in QUrl::FullyDecoded mode");
      options = QUrl::PrettyDecoded;
   }

   // return just the path if:
   //  - QUrl::PreferLocalFile is passed
   //  - QUrl::RemovePath is not passed
   //  - there is no query or fragment to return
   //  - it is a local file

   if (options.testFlag(QUrl::PreferLocalFile) && ! options.testFlag(QUrl::RemovePath)
         && (! d->hasQuery()    || options.testFlag(QUrl::RemoveQuery))
         && (! d->hasFragment() || options.testFlag(QUrl::RemoveFragment))
         && isLocalFile()) {

      return d->toLocalFile(options);
   }

   QString url;

   // for the full URL consider that reserved characters are prettier if encoded
   if (options & DecodeReserved) {
      options &= ~EncodeReserved;
   } else {
      options |= EncodeReserved;
   }

   if (! (options & QUrl::RemoveScheme) && d->hasScheme()) {
      url += d->scheme + ':';
   }

   bool pathIsAbsolute = d->path.startsWith('/');

   if (! ((options & QUrl::RemoveAuthority) == QUrl::RemoveAuthority) && d->hasAuthority()) {
      url += "//";
      d->appendAuthority(url, options, QUrlPrivate::FullUrl);

   } else if (isLocalFile() && pathIsAbsolute) {
      // comply with the XDG file URI spec which requires triple slashes
      url += "//";
   }

   if (! (options & QUrl::RemovePath)) {
      d->appendPath(url, options, QUrlPrivate::FullUrl);
   }

   if (! (options & QUrl::RemoveQuery) && d->hasQuery()) {
      url += '?';
      d->appendQuery(url, options, QUrlPrivate::FullUrl);
   }

   if (! (options & QUrl::RemoveFragment) && d->hasFragment()) {
      url += '#';
      d->appendFragment(url, options, QUrlPrivate::FullUrl);
   }

   return url;
}

QString QUrl::toDisplayString(FormattingOptions options) const
{
   return toString(options | RemovePassword);
}

QUrl QUrl::adjusted(QUrl::FormattingOptions options) const
{
   if (!isValid()) {
      // also catches isEmpty()
      return QUrl();
   }

   QUrl that = *this;

   if (options & RemoveScheme) {
      that.setScheme(QString());
   }

   if ((options & RemoveAuthority) == RemoveAuthority) {
      that.setAuthority(QString());

   } else {
      if ((options & RemoveUserInfo) == RemoveUserInfo) {
         that.setUserInfo(QString());
      } else if (options & RemovePassword) {
         that.setPassword(QString());
      }

      if (options & RemovePort) {
         that.setPort(-1);
      }
   }

   if (options & RemoveQuery) {
      that.setQuery(QString());
   }

   if (options & RemoveFragment) {
      that.setFragment(QString());
   }

   if (options & RemovePath) {
      that.setPath(QString());
   } else if (options & (StripTrailingSlash | RemoveFilename | NormalizePathSegments)) {
      that.detach();
      QString path;
      d->appendPath(path, options | FullyEncoded, QUrlPrivate::Path);
      that.d->setPath(path, 0, path.length());
   }

   return that;
}

QByteArray QUrl::toEncoded(FormattingOptions options) const
{
   options &= ~(FullyDecoded | FullyEncoded);
   QString stringForm = toString(options | FullyEncoded);
   return stringForm.toLatin1();
}

QUrl QUrl::fromEncoded(const QByteArray &input, ParsingMode mode)
{
   return QUrl(QString::fromUtf8(input.constData(), input.size()), mode);
}

QString QUrl::fromPercentEncoding(const QByteArray &input)
{
   QByteArray ba = QByteArray::fromPercentEncoding(input);
   return QString::fromUtf8(ba.constData(), ba.size());
}

QByteArray QUrl::toPercentEncoding(const QString &input, const QByteArray &exclude, const QByteArray &include)
{
   return input.toUtf8().toPercentEncoding(exclude, include);
}

QString QUrl::fromEncodedComponent_helper(const QByteArray &ba)
{
   return qt_urlRecodeByteArray(ba);
}

QString QUrl::fromAce(const QString &domain)
{
   return qt_ACE_do(domain, NormalizeAce, ForbidLeadingDot /*FIXME: make configurable*/);
}

QString QUrl::fromAce(const QByteArray &domain)
{
   QString str = QString::fromLatin1(domain);
   return qt_ACE_do(str, NormalizeAce, ForbidLeadingDot /*FIXME: make configurable*/);
}

QByteArray QUrl::toAce(const QString &domain)
{
   QString result = qt_ACE_do(domain, ToAceOnly, ForbidLeadingDot /*FIXME: make configurable*/);
   return result.toLatin1();
}

bool QUrl::operator<(const QUrl &url) const
{
   if (! d || !url.d) {
      bool thisIsEmpty = !d || d->isEmpty();
      bool thatIsEmpty = !url.d || url.d->isEmpty();

      // sort an empty URL first
      return thisIsEmpty && !thatIsEmpty;
   }

   int cmp;
   cmp = d->scheme.compare(url.d->scheme);

   if (cmp != 0) {
      return cmp < 0;
   }

   cmp = d->userName.compare(url.d->userName);

   if (cmp != 0) {
      return cmp < 0;
   }

   cmp = d->password.compare(url.d->password);

   if (cmp != 0) {
      return cmp < 0;
   }

   cmp = d->host.compare(url.d->host);

   if (cmp != 0) {
      return cmp < 0;
   }

   if (d->port != url.d->port) {
      return d->port < url.d->port;
   }

   cmp = d->path.compare(url.d->path);

   if (cmp != 0) {
      return cmp < 0;
   }

   if (d->hasQuery() != url.d->hasQuery()) {
      return url.d->hasQuery();
   }

   cmp = d->query.compare(url.d->query);

   if (cmp != 0) {
      return cmp < 0;
   }

   if (d->hasFragment() != url.d->hasFragment()) {
      return url.d->hasFragment();
   }

   cmp = d->fragment.compare(url.d->fragment);
   return cmp < 0;
}

bool QUrl::operator==(const QUrl &url) const
{
   if (!d && !url.d) {
      return true;
   }

   if (!d) {
      return url.d->isEmpty();
   }

   if (!url.d) {
      return d->isEmpty();
   }

   // First, compare which sections are present, since it speeds up the
   // processing considerably. We just have to ignore the host-is-present flag
   // for local files (the "file" protocol), due to the requirements of the
   // XDG file URI specification.

   int mask = QUrlPrivate::FullUrl;

   if (isLocalFile()) {
      mask &= ~QUrlPrivate::Host;
   }

   return (d->sectionIsPresent & mask) == (url.d->sectionIsPresent & mask) &&
         d->scheme == url.d->scheme &&
         d->userName == url.d->userName &&
         d->password == url.d->password &&
         d->host == url.d->host &&
         d->port == url.d->port &&
         d->path == url.d->path &&
         d->query == url.d->query &&
         d->fragment == url.d->fragment;
}

bool QUrl::matches(const QUrl &url, FormattingOptions options) const
{
   if (! d && !url.d) {
      return true;
   }

   if (! d) {
      return url.d->isEmpty();
   }

   if (! url.d) {
      return d->isEmpty();
   }

   // First, compare which sections are present, since it speeds up the
   // processing considerably. We just have to ignore the host-is-present flag
   // for local files (the "file" protocol), due to the requirements of the
   // XDG file URI specification.

   int mask = QUrlPrivate::FullUrl;

   if (isLocalFile()) {
      mask &= ~QUrlPrivate::Host;
   }

   if (options & QUrl::RemoveScheme) {
      mask &= ~QUrlPrivate::Scheme;
   } else if (d->scheme != url.d->scheme) {
      return false;
   }

   if (options & QUrl::RemovePassword) {
      mask &= ~QUrlPrivate::Password;
   } else if (d->password != url.d->password) {
      return false;
   }

   if (options & QUrl::RemoveUserInfo) {
      mask &= ~QUrlPrivate::UserName;
   } else if (d->userName != url.d->userName) {
      return false;
   }

   if (options & QUrl::RemovePort) {
      mask &= ~QUrlPrivate::Port;
   } else if (d->port != url.d->port) {
      return false;
   }

   if (options & QUrl::RemoveAuthority) {
      mask &= ~QUrlPrivate::Host;
   } else if (d->host != url.d->host) {
      return false;
   }

   if (options & QUrl::RemoveQuery) {
      mask &= ~QUrlPrivate::Query;
   } else if (d->query != url.d->query) {
      return false;
   }

   if (options & QUrl::RemoveFragment) {
      mask &= ~QUrlPrivate::Fragment;
   } else if (d->fragment != url.d->fragment) {
      return false;
   }

   if ((d->sectionIsPresent & mask) != (url.d->sectionIsPresent & mask)) {
      return false;
   }

   if (options & QUrl::RemovePath) {
      return true;
   }

   // Compare paths, after applying path-related options
   QString path1;
   d->appendPath(path1, options, QUrlPrivate::Path);

   QString path2;
   url.d->appendPath(path2, options, QUrlPrivate::Path);

   return path1 == path2;
}

bool QUrl::operator!=(const QUrl &url) const
{
   return !(*this == url);
}

QUrl &QUrl::operator=(const QUrl &url)
{
   if (! d) {
      if (url.d) {
         url.d->ref.ref();
         d = url.d;
      }

   } else {
      if (url.d) {
         qAtomicAssign(d, url.d);
      } else {
         clear();
      }
   }

   return *this;
}

void QUrl::detach()
{
   if (! d) {
      d = new QUrlPrivate;
   } else {
      qAtomicDetach(d);
   }
}

bool QUrl::isDetached() const
{
   return !d || d->ref.load() == 1;
}

QUrl QUrl::fromLocalFile(const QString &localFile)
{
   QUrl url;

   if (localFile.isEmpty()) {
      return url;
   }

   QString scheme = fileScheme();
   QString deslashified = QDir::fromNativeSeparators(localFile);

   // magic for drives on windows
   if (deslashified.length() > 1 && deslashified.at(1) == ':' && deslashified.at(0) != '/') {
      deslashified.prepend('/');

   } else if (deslashified.startsWith("//")) {
      // magic for shared drive on windows
      int indexOfPath = deslashified.indexOf('/', 2);

      QString hostSpec = deslashified.mid(2, indexOfPath - 2);

      // Check for Windows-specific WebDAV specification: "//host@SSL/path".
      if (hostSpec.endsWith(webDavSslTag(), Qt::CaseInsensitive)) {
         hostSpec.chop(4);
         scheme = webDavScheme();
      }

      url.setHost(hostSpec);

      if (indexOfPath > 2) {
         deslashified = deslashified.right(deslashified.length() - indexOfPath);
      } else {
         deslashified.clear();
      }
   }

   url.setScheme(scheme);
   url.setPath(deslashified, DecodedMode);
   return url;
}

QString QUrl::toLocalFile() const
{
   // call to isLocalFile() also ensures the url is parsed
   if (! isLocalFile()) {
      return QString();
   }

   return d->toLocalFile(QUrl::FullyDecoded);
}

bool QUrl::isLocalFile() const
{
   return d && d->isLocalFile();
}

bool QUrl::isParentOf(const QUrl &childUrl) const
{
   QString childPath = childUrl.path();

   if (! d) {
      return ((childUrl.scheme().isEmpty()) && (childUrl.authority().isEmpty())
            && childPath.length() > 0 && childPath.at(0) == QChar('/'));
   }

   QString ourPath = path();

   return ((childUrl.scheme().isEmpty() || d->scheme == childUrl.scheme())  && (childUrl.authority().isEmpty() ||
         authority() == childUrl.authority()) &&  childPath.startsWith(ourPath) && ((ourPath.endsWith(QChar('/')) &&
         childPath.length() > ourPath.length()) || (! ourPath.endsWith(QChar('/')) &&
         childPath.length() > ourPath.length() && childPath.at(ourPath.length()) == QChar('/'))));
}

QDataStream &operator<<(QDataStream &stream, const QUrl &url)
{
   QByteArray u;

   if (url.isValid()) {
      u = url.toEncoded();
   }

   stream << u;
   return stream;
}

QDataStream &operator>>(QDataStream &stream, QUrl &url)
{
   QByteArray u;
   stream >> u;
   url.setUrl(QString::fromLatin1(u));

   return stream;
}

static QString errorMessage(QUrlPrivate::ErrorCode errorCode, const QString &errorSource, int errorPosition)
{
   QChar c = uint(errorPosition) < uint(errorSource.length()) ?
         errorSource.at(errorPosition) : QChar(QChar::Null);

   switch (errorCode) {

      case QUrlPrivate::NoError:
         Q_ASSERT_X(false, "QUrl::errorString", "Impossible: QUrl::errorString should have treated this condition");

         return QString();

      case QUrlPrivate::InvalidSchemeError: {
         QString msg = QString("Invalid scheme (character '%1' not permitted)");
         return msg.formatArg(c);
      }

      case QUrlPrivate::InvalidUserNameError:
         return QString("Invalid user name (character '%1' not permitted)").formatArg(c);

      case QUrlPrivate::InvalidPasswordError:
         return QString("Invalid password (character '%1' not permitted)").formatArg(c);

      case QUrlPrivate::InvalidRegNameError:
         if (errorPosition != -1) {
            return QString("Invalid hostname (character '%1' not permitted)").formatArg(c);
         } else {
            return QString("Invalid hostname (contains invalid characters)");
         }

      case QUrlPrivate::InvalidIPv4AddressError:
         return QString(); // doesn't happen yet

      case QUrlPrivate::InvalidIPv6AddressError:
         return QString("Invalid IPv6 address");

      case QUrlPrivate::InvalidCharacterInIPv6Error:
         return QString("Invalid IPv6 address (character '%1' not permitted)").formatArg(c);

      case QUrlPrivate::InvalidIPvFutureError:
         return QString("Invalid IPvFuture address (character '%1' not permitted)").formatArg(c);

      case QUrlPrivate::HostMissingEndBracket:
         return QString("Expected ']' to match '[' in hostname");

      case QUrlPrivate::InvalidPortError:
         return QString("Invalid port or port number out of range");

      case QUrlPrivate::PortEmptyError:
         return QString("Port field was empty");

      case QUrlPrivate::InvalidPathError:
         return QString("Invalid path (character '%1' not permitted)").formatArg(c);

      case QUrlPrivate::InvalidQueryError:
         return QString("Invalid query (character '%1' not permitted)").formatArg(c);

      case QUrlPrivate::InvalidFragmentError:
         return QString("Invalid fragment (character '%1' not permitted)").formatArg(c);

      case QUrlPrivate::AuthorityPresentAndPathIsRelative:
         return QString("Path component is relative and authority is present");

      case QUrlPrivate::RelativeUrlPathContainsColonBeforeSlash:
         return QString("Relative URL's path component contains ':' before any '/'");
   }

   Q_ASSERT_X(false, "QUrl::errorString", "Can not happen, unknown error");

   return QString();
}

static inline void appendComponentIfPresent(QString &msg, bool present, const char *componentName, const QString &component)
{
   if (present) {
      msg += QString::fromUtf8(componentName);
      msg += '"' + msg + component + "\",";
   }
}

QString QUrl::errorString() const
{
   if (! d) {
      return QString();
   }

   QString errorSource;
   int errorPosition = 0;

   QUrlPrivate::ErrorCode errorCode = d->validityError(&errorSource, &errorPosition);

   if (errorCode == QUrlPrivate::NoError) {
      return QString();
   }

   QString msg = errorMessage(errorCode, errorSource, errorPosition);
   msg += QLatin1String("; source was \"");
   msg += errorSource;
   msg += "\";";

   appendComponentIfPresent(msg, d->sectionIsPresent & QUrlPrivate::Scheme,   " scheme = ", d->scheme);
   appendComponentIfPresent(msg, d->sectionIsPresent & QUrlPrivate::UserInfo, " userinfo = ", userInfo());
   appendComponentIfPresent(msg, d->sectionIsPresent & QUrlPrivate::Host,     " host = ", d->host);

   appendComponentIfPresent(msg, d->port != -1,      " port = ", QString::number(d->port));
   appendComponentIfPresent(msg, !d->path.isEmpty(), " path = ", d->path);

   appendComponentIfPresent(msg, d->sectionIsPresent & QUrlPrivate::Query,    " query = ", d->query);
   appendComponentIfPresent(msg, d->sectionIsPresent & QUrlPrivate::Fragment, " fragment = ", d->fragment);

   if (msg.endsWith(',')) {
      msg.chop(1);
   }

   return msg;
}

QStringList QUrl::toStringList(const QList<QUrl> &urls, FormattingOptions options)
{
   QStringList lst;

   for (const QUrl &url : urls) {
      lst.append(url.toString(options));
   }

   return lst;
}

QList<QUrl> QUrl::fromStringList(const QStringList &urls, ParsingMode mode)
{
   QList<QUrl> lst;

   for (const QString &str : urls) {
      lst.append(QUrl(str, mode));
   }

   return lst;
}

uint qHash(const QUrl &url, uint seed)
{
   if (! url.d) {
      return qHash(-1, seed);   // the hash of an unset port (-1)
   }

   return qHash(url.d->scheme) ^
         qHash(url.d->userName) ^
         qHash(url.d->password) ^
         qHash(url.d->host) ^
         qHash(url.d->port, seed) ^
         qHash(url.d->path) ^
         qHash(url.d->query) ^
         qHash(url.d->fragment);
}

static QUrl adjustFtpPath(QUrl url)
{
   if (url.scheme() == ftpScheme()) {
      QString path = url.path(QUrl::PrettyDecoded);

      if (path.startsWith("//")) {
         url.setPath("/%2F" + path.mid(2), QUrl::TolerantMode);
      }
   }

   return url;
}

static bool isIp6(const QString &text)
{
   QIPAddressUtils::IPv6Address address;
   return ! text.isEmpty() && QIPAddressUtils::parseIp6(address, text.begin(), text.end()) == text.end();
}

QUrl QUrl::fromUserInput(const QString &userInput, const QString &workingDirectory,
      UserInputResolutionOptions options)
{
   QString trimmedString = userInput.trimmed();

   if (trimmedString.isEmpty()) {
      return QUrl();
   }

   // Check for IPv6 addresses, since a path starting with ":" is absolute (a resource)
   // and IPv6 addresses can start with "c:" too
   if (isIp6(trimmedString)) {
      QUrl url;
      url.setHost(trimmedString);
      url.setScheme(QString("http"));
      return url;
   }

   QUrl url = QUrl(trimmedString, QUrl::TolerantMode);

   // Check both QUrl::isRelative (to detect full URLs) and QDir::isAbsolutePath
   // (since on Windows drive letters can be interpreted as schemes)
   if (url.isRelative() && !QDir::isAbsolutePath(trimmedString)) {
      QFileInfo fileInfo(QDir(workingDirectory), trimmedString);

      if ((options & AssumeLocalFile) || fileInfo.exists()) {
         return QUrl::fromLocalFile(fileInfo.absoluteFilePath());
      }
   }

   return fromUserInput(trimmedString);
}

QUrl QUrl::fromUserInput(const QString &userInput)
{
   QString trimmedString = userInput.trimmed();

   // Check for IPv6 addresses, since a path starting with ":" is absolute (a resource)
   // and IPv6 addresses can start with "c:" too
   if (isIp6(trimmedString)) {
      QUrl url;
      url.setHost(trimmedString);
      url.setScheme(QString("http"));
      return url;
   }

   // Check first for files, since on Windows drive letters can be interpretted as schemes
   if (QDir::isAbsolutePath(trimmedString)) {
      return QUrl::fromLocalFile(trimmedString);
   }

   QUrl url = QUrl(trimmedString, QUrl::TolerantMode);
   QUrl urlPrepended = QUrl(QString("http://") + trimmedString, QUrl::TolerantMode);

   // Check the most common case of a valid url with a scheme
   // We check if the port would be valid by adding the scheme to handle the case host:port
   // where the host would be interpretted as the scheme
   if (url.isValid() && !url.scheme().isEmpty() && urlPrepended.port() == -1) {
      return adjustFtpPath(url);
   }

   // Else, try the prepended one and adjust the scheme from the host name
   if (urlPrepended.isValid() && (!urlPrepended.host().isEmpty() || !urlPrepended.path().isEmpty())) {
      int dotIndex = trimmedString.indexOf(QLatin1Char('.'));
      const QString hostscheme = trimmedString.left(dotIndex).toLower();

      if (hostscheme == ftpScheme()) {
         urlPrepended.setScheme(ftpScheme());
      }

      return adjustFtpPath(urlPrepended);
   }

   return QUrl();
}
