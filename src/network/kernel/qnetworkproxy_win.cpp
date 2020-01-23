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

#include <qnetworkproxy.h>

#ifndef QT_NO_NETWORKPROXY

#include <qnetworkinterface.h>

#include <qmutex.h>
#include <qstringlist.h>
#include <qregularexpression.h>
#include <qurl.h>
#include <qvarlengtharray.h>

#include <qsystemlibrary_p.h>

#include <string.h>
#include <qt_windows.h>
#include <wininet.h>
#include <lmcons.h>

/*
 * Information on the WinHTTP DLL:
 *  http://msdn.microsoft.com/en-us/library/aa384122(VS.85).aspx example for WPAD
 *
 *  http://msdn.microsoft.com/en-us/library/aa384097(VS.85).aspx WinHttpGetProxyForUrl
 *  http://msdn.microsoft.com/en-us/library/aa384096(VS.85).aspx WinHttpGetIEProxyConfigForCurrentUs
 *  http://msdn.microsoft.com/en-us/library/aa384095(VS.85).aspx WinHttpGetDefaultProxyConfiguration
 */

// We do not want to include winhttp.h because it is not present in some Windows SDKs (I don't know why)
// Instead, copy the definitions here

typedef struct {
   DWORD dwFlags;
   DWORD dwAutoDetectFlags;
   LPCWSTR lpszAutoConfigUrl;
   LPVOID lpvReserved;
   DWORD dwReserved;
   BOOL fAutoLogonIfChallenged;
} WINHTTP_AUTOPROXY_OPTIONS;

typedef struct {
   DWORD dwAccessType;
   LPWSTR lpszProxy;
   LPWSTR lpszProxyBypass;
} WINHTTP_PROXY_INFO;

typedef struct {
   BOOL fAutoDetect;
   LPWSTR lpszAutoConfigUrl;
   LPWSTR lpszProxy;
   LPWSTR lpszProxyBypass;
} WINHTTP_CURRENT_USER_IE_PROXY_CONFIG;

#define WINHTTP_AUTOPROXY_AUTO_DETECT           0x00000001
#define WINHTTP_AUTOPROXY_CONFIG_URL            0x00000002

#define WINHTTP_AUTO_DETECT_TYPE_DHCP           0x00000001
#define WINHTTP_AUTO_DETECT_TYPE_DNS_A          0x00000002

#define WINHTTP_ACCESS_TYPE_DEFAULT_PROXY               0
#define WINHTTP_ACCESS_TYPE_NO_PROXY                    1
#define WINHTTP_ACCESS_TYPE_NAMED_PROXY                 3

#define WINHTTP_NO_PROXY_NAME     NULL
#define WINHTTP_NO_PROXY_BYPASS   NULL

#define WINHTTP_ERROR_BASE                      12000
#define ERROR_WINHTTP_LOGIN_FAILURE             (WINHTTP_ERROR_BASE + 15)
#define ERROR_WINHTTP_UNABLE_TO_DOWNLOAD_SCRIPT (WINHTTP_ERROR_BASE + 167)
#define ERROR_WINHTTP_AUTODETECTION_FAILED      (WINHTTP_ERROR_BASE + 180)

typedef BOOL (WINAPI *PtrWinHttpGetProxyForUrl)(HINTERNET, LPCWSTR, WINHTTP_AUTOPROXY_OPTIONS *, WINHTTP_PROXY_INFO *);
typedef HINTERNET (WINAPI *PtrWinHttpOpen)(LPCWSTR, DWORD, LPCWSTR, LPCWSTR, DWORD);
typedef BOOL (WINAPI *PtrWinHttpGetDefaultProxyConfiguration)(WINHTTP_PROXY_INFO *);
typedef BOOL (WINAPI *PtrWinHttpGetIEProxyConfigForCurrentUser)(WINHTTP_CURRENT_USER_IE_PROXY_CONFIG *);
typedef BOOL (WINAPI *PtrWinHttpCloseHandle)(HINTERNET);
typedef BOOL (WINAPI *PtrCloseServiceHandle)(SC_HANDLE hSCObject);
static PtrWinHttpGetProxyForUrl ptrWinHttpGetProxyForUrl = 0;
static PtrWinHttpOpen ptrWinHttpOpen = 0;
static PtrWinHttpGetDefaultProxyConfiguration ptrWinHttpGetDefaultProxyConfiguration = 0;
static PtrWinHttpGetIEProxyConfigForCurrentUser ptrWinHttpGetIEProxyConfigForCurrentUser = 0;
static PtrWinHttpCloseHandle ptrWinHttpCloseHandle = 0;

static bool currentProcessIsService()
{
   typedef BOOL (WINAPI * PtrGetUserName)(LPTSTR lpBuffer, LPDWORD lpnSize);
   typedef BOOL (WINAPI * PtrLookupAccountName)(LPCTSTR lpSystemName, LPCTSTR lpAccountName, PSID Sid,
         LPDWORD cbSid, LPTSTR ReferencedDomainName, LPDWORD cchReferencedDomainName, PSID_NAME_USE peUse);

   static PtrGetUserName ptrGetUserName = (PtrGetUserName)QSystemLibrary::resolve(QLatin1String("Advapi32"), "GetUserNameW");
   static PtrLookupAccountName ptrLookupAccountName = (PtrLookupAccountName)QSystemLibrary::resolve(QLatin1String("Advapi32"), "LookupAccountNameW");

   if (ptrGetUserName && ptrLookupAccountName) {
      wchar_t userName[UNLEN + 1] = L"";
      DWORD size = UNLEN;

      if (ptrGetUserName(userName, &size)) {
         SID_NAME_USE type = SidTypeUser;
         DWORD sidSize     = 0;
         DWORD domainSize  = 0;

         // first call is to get the correct size
         bool bRet = ptrLookupAccountName(NULL, userName, NULL, &sidSize, NULL, &domainSize, &type);
         if (bRet == FALSE && ERROR_INSUFFICIENT_BUFFER != GetLastError()) {
            return false;
         }

         QVarLengthArray<BYTE, 68> buff(sidSize);
         QVarLengthArray<wchar_t, MAX_PATH> domainName(domainSize);

         // second call to LookupAccountNameW actually gets the SID
         // both the pointer to the buffer and the pointer to the domain name should not be NULL
         if (ptrLookupAccountName(NULL, userName, buff.data(), &sidSize, domainName.data(), &domainSize, &type)) {
            return type != SidTypeUser;   //returns true if the current user is not a user
         }
      }
   }

   return false;
}

static QStringList splitSpaceSemicolon(const QString &source)
{
   QStringList list;
   int start = 0;
   int end;

   while (true) {
      int space = source.indexOf(QLatin1Char(' '), start);
      int semicolon = source.indexOf(QLatin1Char(';'), start);
      end = space;

      if (semicolon != -1 && (end == -1 || semicolon < end)) {
         end = semicolon;
      }

      if (end == -1) {
         if (start != source.length()) {
            list.append(source.mid(start));
         }
         return list;
      }

      if (start != end) {
         list.append(source.mid(start, end - start));
      }
      start = end + 1;
   }
   return list;
}

static bool isBypassed(const QString &host, const QStringList &bypassList)
{
   if (host.isEmpty()) {
      return false;
   }

   bool isSimple = !host.contains(QLatin1Char('.')) && !host.contains(QLatin1Char(':'));

   QHostAddress ipAddress;
   bool isIpAddress = ipAddress.setAddress(host);

   // always exclude loopback
   if (isIpAddress && ipAddress.isLoopback()) {
      return true;
   }

   // does it match the list of exclusions?
   for (const QString &entry : bypassList) {
      if (entry == "<local>") {
         if (isSimple) {
            return true;
         }

         if (isIpAddress) {
            //exclude all local subnets
            for (const QNetworkInterface &iface : QNetworkInterface::allInterfaces()) {
               for (const QNetworkAddressEntry netaddr : iface.addressEntries()) {
                  if (ipAddress.isInSubnet(netaddr.ip(), netaddr.prefixLength())) {
                     return true;
                  }
               }
            }
         }
      }
      if (isIpAddress && ipAddress.isInSubnet(QHostAddress::parseSubnet(entry))) {
         return true;        // excluded

      } else {
         // do wildcard matching
         QRegularExpression rx(entry, QPatternOption::CaseInsensitiveOption | QPatternOption::WildcardOption
                     | QPatternOption::ExactMatchOption);

         QRegularExpressionMatch match = rx.match(host);

         if (match.hasMatch()) {
            return true;
         }
      }
   }

   // host was not excluded
   return false;
}

static QList<QNetworkProxy> filterProxyListByCapabilities(const QList<QNetworkProxy> &proxyList,
      const QNetworkProxyQuery &query)
{
   QNetworkProxy::Capabilities requiredCaps;
   switch (query.queryType()) {
      case QNetworkProxyQuery::TcpSocket:
         requiredCaps = QNetworkProxy::TunnelingCapability;
         break;
      case QNetworkProxyQuery::UdpSocket:
         requiredCaps = QNetworkProxy::UdpTunnelingCapability;
         break;
      case QNetworkProxyQuery::TcpServer:
         requiredCaps = QNetworkProxy::ListeningCapability;
         break;
      default:
         return proxyList;
         break;
   }

   QList<QNetworkProxy> result;

   for (const QNetworkProxy &proxy : proxyList) {
      if (proxy.capabilities() & requiredCaps) {
         result.append(proxy);
      }
   }

   return result;
}

static QList<QNetworkProxy> removeDuplicateProxies(const QList<QNetworkProxy> &proxyList)
{
   QList<QNetworkProxy> result;
   for (QNetworkProxy proxy : proxyList) {
      bool append = true;
      for (int i = 0; i < result.count(); i++) {
         if (proxy.hostName() == result.at(i).hostName()
               && proxy.port() == result.at(i).port()) {
            append = false;
            // HttpProxy trumps FtpCachingProxy or HttpCachingProxy on the same host/port
            if (proxy.type() == QNetworkProxy::HttpProxy) {
               result[i] = proxy;
            }
         }
      }
      if (append) {
         result.append(proxy);
      }
   }
   return result;
}

static QList<QNetworkProxy> parseServerList(const QNetworkProxyQuery &query, const QStringList &proxyList)
{
   // Reference documentation from Microsoft:
   // http://msdn.microsoft.com/en-us/library/aa383912(VS.85).aspx
   //
   // According to the website, the proxy server list is
   // one or more of the space- or semicolon-separated strings in the format:
   //   ([<scheme>=][<scheme>"://"]<server>[":"<port>])
   // The first scheme relates to the protocol tag
   // The second scheme, if present, overrides the proxy type

   QList<QNetworkProxy> result;
   QHash<QString, QNetworkProxy> taggedProxies;
   const QString requiredTag = query.protocolTag();

   bool checkTags = ! requiredTag.isEmpty() && query.queryType() != QNetworkProxyQuery::TcpServer; //windows tags are only for clients

   for (const QString &entry : proxyList) {
      int server = 0;

      QNetworkProxy::ProxyType proxyType = QNetworkProxy::HttpProxy;
      quint16 port = 8080;

      int pos = entry.indexOf(QLatin1Char('='));

      QStringView scheme;
      QStringView protocolTag;

      if (pos != -1) {
         scheme = protocolTag = entry.leftView(pos);
         server = pos + 1;
      }
      pos = entry.indexOf("://", server);
      if (pos != -1) {
         scheme = entry.midView(server, pos - server);
         server = pos + 3;
      }

      if (!scheme.isEmpty()) {
         if (scheme == "http" || scheme == "https") {
            // no-op, defaults are above

         } else if (scheme == "socks" || scheme == "socks5") {
            proxyType = QNetworkProxy::Socks5Proxy;
            port = 1080;

         } else if (scheme == "ftp") {
            proxyType = QNetworkProxy::FtpCachingProxy;
            port = 2121;

         } else {
            // unknown proxy type
            continue;
         }
      }

      pos = entry.indexOf(':', server);
      if (pos != -1) {
         bool ok;
         uint value = entry.mid(pos + 1).toInteger<uint>(&ok);

         if (! ok || value > 65535) {
            continue;   // invalid port number
         }

         port = value;
      } else {
         pos = entry.length();
      }

      result << QNetworkProxy(proxyType, entry.mid(server, pos - server), port);
      if (!protocolTag.isEmpty()) {
         taggedProxies.insert(protocolTag.toString(), result.last());
      }
   }

   if (checkTags && taggedProxies.contains(requiredTag)) {
      if (query.queryType() == QNetworkProxyQuery::UrlRequest) {
         result.clear();
         result.append(taggedProxies.value(requiredTag));
         return result;
      } else {
         result.prepend(taggedProxies.value(requiredTag));
      }
   }
   if (!checkTags || requiredTag != QLatin1String("http")) {
      // if there are different http proxies for http and https, prefer the https one (more likely to be capable of CONNECT)
      QNetworkProxy httpProxy  = taggedProxies.value(QLatin1String("http"));
      QNetworkProxy httpsProxy = taggedProxies.value(QLatin1String("http"));

      if (httpProxy != httpsProxy && httpProxy.type() == QNetworkProxy::HttpProxy &&
            httpsProxy.type() == QNetworkProxy::HttpProxy) {
         for (int i = 0; i < result.count(); i++) {
            if (httpProxy == result.at(i)) {
               result[i].setType(QNetworkProxy::HttpCachingProxy);
            }
         }
      }
   }

   result = filterProxyListByCapabilities(result, query);
   return removeDuplicateProxies(result);
}

namespace {

class QRegistryWatcher
{
 public:
   void addLocation(HKEY hive, const QString &path) {
      HKEY openedKey;
      std::wstring tmp = path.toStdWString();

      if (RegOpenKeyEx(hive, tmp.c_str(), 0, KEY_READ, &openedKey) != ERROR_SUCCESS) {
         return;
      }

      const DWORD filter = REG_NOTIFY_CHANGE_NAME | REG_NOTIFY_CHANGE_ATTRIBUTES |
                           REG_NOTIFY_CHANGE_LAST_SET | REG_NOTIFY_CHANGE_SECURITY;

      // Watch the registry key for a change of value.
      HANDLE handle = CreateEvent(NULL, true, false, NULL);
      if (RegNotifyChangeKeyValue(openedKey, true, filter, handle, true) != ERROR_SUCCESS) {
         CloseHandle(handle);
         return;
      }

      m_watchEvents.append(handle);
      m_registryHandles.append(openedKey);
   }

   bool hasChanged() const {
      return !isEmpty() &&
             WaitForMultipleObjects(m_watchEvents.size(), m_watchEvents.data(), false, 0) < WAIT_OBJECT_0 + m_watchEvents.size();
   }

   bool isEmpty() const {
      return m_watchEvents.isEmpty();
   }

   void clear() {
      for (HANDLE event : m_watchEvents) {
         CloseHandle(event);
      }

      for (HKEY key : m_registryHandles) {
         RegCloseKey(key);
      }

      m_watchEvents.clear();
      m_registryHandles.clear();
   }

   ~QRegistryWatcher() {
      clear();
   }

 private:
   QVector<HANDLE> m_watchEvents;
   QVector<HKEY> m_registryHandles;
};
} // namespace

class QWindowsSystemProxy
{
 public:
   QWindowsSystemProxy();
   ~QWindowsSystemProxy();

   void init();
   void reset();

   QMutex mutex;

   HINTERNET hHttpSession;
   WINHTTP_AUTOPROXY_OPTIONS m_autoProxyOptions;

   std::wstring m_proxyString;

   QString autoConfigUrl;
   QStringList proxyServerList;
   QStringList proxyBypass;
   QList<QNetworkProxy> defaultResult;

   QRegistryWatcher proxySettingsWatcher;
   bool initialized;
   bool functional;
   bool isAutoConfig;
};

Q_GLOBAL_STATIC(QWindowsSystemProxy, systemProxy)

QWindowsSystemProxy::QWindowsSystemProxy()
   : hHttpSession(0), initialized(false), functional(false), isAutoConfig(false)
{
   defaultResult << QNetworkProxy::NoProxy;
}

QWindowsSystemProxy::~QWindowsSystemProxy()
{
   if (hHttpSession) {
      ptrWinHttpCloseHandle(hHttpSession);
   }
}

void QWindowsSystemProxy::reset()
{
   autoConfigUrl.clear();
   proxyServerList.clear();
   proxyBypass.clear();
   defaultResult.clear();
   defaultResult << QNetworkProxy::NoProxy;
   functional = false;
   isAutoConfig = false;
}

void QWindowsSystemProxy::init()
{
   bool proxySettingsChanged = false;

   proxySettingsChanged = proxySettingsWatcher.hasChanged();

   if (initialized && ! proxySettingsChanged) {
      return;
   }

   initialized = true;

   reset();
   proxySettingsWatcher.clear(); // needs reset to trigger a new detection
   proxySettingsWatcher.addLocation(HKEY_CURRENT_USER,  "Software\\Microsoft\\Windows\\CurrentVersion\\Internet Settings");
   proxySettingsWatcher.addLocation(HKEY_LOCAL_MACHINE, "Software\\Microsoft\\Windows\\CurrentVersion\\Internet Settings");
   proxySettingsWatcher.addLocation(HKEY_LOCAL_MACHINE, "Software\\Policies\\Microsoft\\Windows\\CurrentVersion\\Internet Settings");

   // load the winhttp.dll library
   QSystemLibrary lib("winhttp");

   if (! lib.load()) {
      return;   // failed to load
   }

   ptrWinHttpOpen = (PtrWinHttpOpen)lib.resolve("WinHttpOpen");
   ptrWinHttpCloseHandle = (PtrWinHttpCloseHandle)lib.resolve("WinHttpCloseHandle");
   ptrWinHttpGetProxyForUrl = (PtrWinHttpGetProxyForUrl)lib.resolve("WinHttpGetProxyForUrl");

   ptrWinHttpGetDefaultProxyConfiguration   = (PtrWinHttpGetDefaultProxyConfiguration)lib.resolve("WinHttpGetDefaultProxyConfiguration");
   ptrWinHttpGetIEProxyConfigForCurrentUser = (PtrWinHttpGetIEProxyConfigForCurrentUser)lib.resolve("WinHttpGetIEProxyConfigForCurrentUser");

   // Try to obtain the Internet Explorer configuration.
   WINHTTP_CURRENT_USER_IE_PROXY_CONFIG ieProxyConfig;
   const bool hasIEConfig = ptrWinHttpGetIEProxyConfigForCurrentUser(&ieProxyConfig);

   if (hasIEConfig) {
      if (ieProxyConfig.lpszAutoConfigUrl) {
         autoConfigUrl = QString::fromStdWString(std::wstring(ieProxyConfig.lpszAutoConfigUrl));
         GlobalFree(ieProxyConfig.lpszAutoConfigUrl);
      }

      if (ieProxyConfig.lpszProxy) {
         // http://msdn.microsoft.com/en-us/library/aa384250%28VS.85%29.aspx speaks only about a "proxy URL",
         // not multiple URLs. However we tested this and it can return multiple URLs. So we use splitSpaceSemicolon
         // on it.
         proxyServerList = splitSpaceSemicolon(QString::fromStdWString(std::wstring(ieProxyConfig.lpszProxy)));
         GlobalFree(ieProxyConfig.lpszProxy);
      }

      if (ieProxyConfig.lpszProxyBypass) {
         proxyBypass = splitSpaceSemicolon(QString::fromStdWString(std::wstring(ieProxyConfig.lpszProxyBypass)));
         GlobalFree(ieProxyConfig.lpszProxyBypass);
      }
   }

   if (! hasIEConfig ||
         (currentProcessIsService() && proxyServerList.isEmpty() && proxyBypass.isEmpty()))  {
      // no user configuration
      // attempt to get the default configuration instead
      // that config will serve as default if WPAD fails

      WINHTTP_PROXY_INFO proxyInfo;
      if (ptrWinHttpGetDefaultProxyConfiguration(&proxyInfo) &&
            proxyInfo.dwAccessType == WINHTTP_ACCESS_TYPE_NAMED_PROXY) {
         // we got information from the registry
         // overwrite the IE configuration, if any

         proxyBypass = splitSpaceSemicolon(QString::fromStdWString(std::wstring(proxyInfo.lpszProxyBypass)));
         proxyServerList = splitSpaceSemicolon(QString::fromStdWString(std::wstring(proxyInfo.lpszProxy)));
      }

      if (proxyInfo.lpszProxy) {
         GlobalFree(proxyInfo.lpszProxy);
      }

      if (proxyInfo.lpszProxyBypass) {
         GlobalFree(proxyInfo.lpszProxyBypass);
      }
   }

   hHttpSession = NULL;

   if (ieProxyConfig.fAutoDetect || !autoConfigUrl.isEmpty())  {
      // open the handle and obtain the options
      hHttpSession = ptrWinHttpOpen(L"CopperSpice System Proxy access/1.0",
                                 WINHTTP_ACCESS_TYPE_NO_PROXY,
                                 WINHTTP_NO_PROXY_NAME,
                                 WINHTTP_NO_PROXY_BYPASS, 0);
      if (! hHttpSession) {
         return;
      }

      isAutoConfig = true;
      memset(&m_autoProxyOptions, 0, sizeof(m_autoProxyOptions));
      m_autoProxyOptions.fAutoLogonIfChallenged = false;

      // Although it is possible to specify dwFlags = WINHTTP_AUTOPROXY_AUTO_DETECT | WINHTTP_AUTOPROXY_CONFIG_URL
      // this has poor performance (WPAD is attempted for every url, taking 2.5 seconds per interface,
      // before the configured pac file is used)

      if (ieProxyConfig.fAutoDetect) {
         m_autoProxyOptions.dwFlags = WINHTTP_AUTOPROXY_AUTO_DETECT;
         m_autoProxyOptions.dwAutoDetectFlags = WINHTTP_AUTO_DETECT_TYPE_DHCP | WINHTTP_AUTO_DETECT_TYPE_DNS_A;
      } else {
         m_autoProxyOptions.dwFlags = WINHTTP_AUTOPROXY_CONFIG_URL;

         m_proxyString = autoConfigUrl.toStdWString();
         m_autoProxyOptions.lpszAutoConfigUrl = m_proxyString.c_str();
      }
   }

   functional = isAutoConfig || !proxyServerList.isEmpty();
}

QList<QNetworkProxy> QNetworkProxyFactory::systemProxyForQuery(const QNetworkProxyQuery &query)
{
   QWindowsSystemProxy *sp = systemProxy();

   if (! sp) {
      return QList<QNetworkProxy>() << QNetworkProxy();
   }

   QMutexLocker locker(&sp->mutex);
   sp->init();

   if (! sp->functional) {
      return sp->defaultResult;
   }

   if (sp->isAutoConfig) {
      WINHTTP_PROXY_INFO proxyInfo;

      // try to get the proxy config for the URL
      QUrl url = query.url();

      // url could be empty, e.g. from QNetworkProxy::applicationProxy(), that's fine,
      // we'll still ask for the proxy.
      // But for a file url, we know we don't need one.
      if (url.scheme() == QLatin1String("file") || url.scheme() == QLatin1String("qrc")) {
         return sp->defaultResult;
      }

      if (query.queryType() != QNetworkProxyQuery::UrlRequest) {
         // change the scheme to https, maybe it'll work
         url.setScheme(QLatin1String("https"));
      }

      QString urlQueryString = url.toString();
      if (urlQueryString.size() > 2083) {
         qWarning("Proxy query URL too long for windows API, try with truncated URL");
         urlQueryString = url.toString().left(2083);
      }

      std::wstring tmp = urlQueryString.toStdWString();
      bool getProxySucceeded = ptrWinHttpGetProxyForUrl(sp->hHttpSession, tmp.c_str(), &sp->m_autoProxyOptions,  &proxyInfo);

      DWORD getProxyError = GetLastError();

      if (!getProxySucceeded
            && (ERROR_WINHTTP_AUTODETECTION_FAILED == getProxyError)) {
         // WPAD failed
         if (sp->autoConfigUrl.isEmpty()) {
            //No config file could be retrieved on the network.
            //Don't search for it next time again.
            sp->isAutoConfig = false;

         } else {
            //pac file URL is specified as well, try using that
            sp->m_autoProxyOptions.dwFlags = WINHTTP_AUTOPROXY_CONFIG_URL;

            sp->m_proxyString = sp->autoConfigUrl.toStdWString();
            sp->m_autoProxyOptions.lpszAutoConfigUrl = sp->m_proxyString.c_str();

            std::wstring tmp  = urlQueryString.toStdWString();
            getProxySucceeded = ptrWinHttpGetProxyForUrl(sp->hHttpSession, tmp.c_str(), &sp->m_autoProxyOptions, &proxyInfo);

            getProxyError = GetLastError();
         }
      }

      if (!getProxySucceeded
            && (ERROR_WINHTTP_LOGIN_FAILURE == getProxyError)) {
         // We first tried without AutoLogon, because this might prevent caching the result.
         // But now we've to enable it (http://msdn.microsoft.com/en-us/library/aa383153%28v=VS.85%29.aspx)
         sp->m_autoProxyOptions.fAutoLogonIfChallenged = TRUE;

         std::wstring tmp  = urlQueryString.toStdWString();
         getProxySucceeded = ptrWinHttpGetProxyForUrl(sp->hHttpSession, tmp.c_str(), &sp->m_autoProxyOptions, &proxyInfo);

         getProxyError = GetLastError();
      }

      if (!getProxySucceeded
            && (ERROR_WINHTTP_UNABLE_TO_DOWNLOAD_SCRIPT == getProxyError)) {
         // PAC file url is not connectable, or server returned error (e.g. http 404)
         //Don't search for it next time again.
         sp->isAutoConfig = false;
      }

      if (getProxySucceeded) {
         // yes, we got a config for this URL
         QString proxyBypass = QString::fromStdWString(std::wstring(proxyInfo.lpszProxyBypass));
         QStringList proxyServerList = splitSpaceSemicolon(QString::fromStdWString(std::wstring(proxyInfo.lpszProxy)));

         if (proxyInfo.lpszProxy) {
            GlobalFree(proxyInfo.lpszProxy);
         }
         if (proxyInfo.lpszProxyBypass) {
            GlobalFree(proxyInfo.lpszProxyBypass);
         }

         if (proxyInfo.dwAccessType == WINHTTP_ACCESS_TYPE_NO_PROXY) {
            return sp->defaultResult;   //i.e. the PAC file result was "DIRECT"
         }
         if (isBypassed(query.peerHostName(), splitSpaceSemicolon(proxyBypass))) {
            return sp->defaultResult;
         }
         return parseServerList(query, proxyServerList);
      }

      // GetProxyForUrl failed, fall back to static configuration
   }

   // static configuration
   if (isBypassed(query.peerHostName(), sp->proxyBypass)) {
      return sp->defaultResult;
   }

   QList<QNetworkProxy> result = parseServerList(query, sp->proxyServerList);
   // In some cases, this was empty. See SF task 00062670
   if (result.isEmpty()) {
      return sp->defaultResult;
   }

   return result;
}


#endif
