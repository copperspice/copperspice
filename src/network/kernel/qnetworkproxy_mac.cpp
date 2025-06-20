/***********************************************************************
*
* Copyright (c) 2012-2025 Barbara Geller
* Copyright (c) 2012-2025 Ansel Sermersheim
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

#include <qendian.h>
#include <qregularexpression.h>
#include <qstringlist.h>
#include <qurl.h>

#include <qcore_mac_p.h>

#include <CFNetwork/CFNetwork.h>
#include <CoreFoundation/CoreFoundation.h>
#include <SystemConfiguration/SystemConfiguration.h>

/*
 * MacOS X has a proxy configuration module in System Preferences (on
 * MacOS X 10.5, it's in Network, Advanced), where one can set the
 * proxy settings for:
 *
 * \list
 *   \o FTP proxy
 *   \o Web Proxy (HTTP)
 *   \o Secure Web Proxy (HTTPS)
 *   \o Streaming Proxy (RTSP)
 *   \o SOCKS Proxy
 *   \o Gopher Proxy
 *   \o URL for Automatic Proxy Configuration (PAC scripts)
 *   \o Bypass list (by default: *.local, 169.254/16)
 * \endlist
 *
 * The matching configuration can be obtained by calling SCDynamicStoreCopyProxies
 * (from <SystemConfiguration/SCDynamicStoreCopySpecific.h>). See
 * Apple's documentation:
 *
 * http://developer.apple.com/DOCUMENTATION/Networking/Reference/SysConfig/SCDynamicStoreCopySpecific/CompositePage.html#//apple_ref/c/func/SCDynamicStoreCopyProxies
 *
 */

static bool isHostExcluded(CFDictionaryRef dict, const QString &host)
{
   if (host.isEmpty()) {
      return true;
   }

   bool isSimple = ! host.contains(QChar('.')) && ! host.contains(QChar(':'));
   CFNumberRef excludeSimples;

   if (isSimple && (excludeSimples = (CFNumberRef)CFDictionaryGetValue(dict, kSCPropNetProxiesExcludeSimpleHostnames))) {
      int enabled;

      if (CFNumberGetValue(excludeSimples, kCFNumberIntType, &enabled) && enabled) {
         return true;
      }
   }

   QHostAddress ipAddress;
   bool isIpAddress = ipAddress.setAddress(host);

   // not a simple host name
   // does it match the list of exclusions?
   CFArrayRef exclusionList = (CFArrayRef)CFDictionaryGetValue(dict, kSCPropNetProxiesExceptionsList);
   if (! exclusionList) {
      return false;
   }

   CFIndex size = CFArrayGetCount(exclusionList);
   for (CFIndex i = 0; i < size; ++i) {
      CFStringRef cfentry = (CFStringRef)CFArrayGetValueAtIndex(exclusionList, i);
      QString entry = QCFString::toQString(cfentry);

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

static QNetworkProxy proxyFromDictionary(CFDictionaryRef dict, QNetworkProxy::ProxyType type,
      CFStringRef enableKey, CFStringRef hostKey, CFStringRef portKey)
{
   CFNumberRef protoEnabled;
   CFNumberRef protoPort;
   CFStringRef protoHost;

   if (enableKey
         && (protoEnabled = (CFNumberRef)CFDictionaryGetValue(dict, enableKey))
         && (protoHost = (CFStringRef)CFDictionaryGetValue(dict, hostKey))
         && (protoPort = (CFNumberRef)CFDictionaryGetValue(dict, portKey))) {

      int enabled;

      if (CFNumberGetValue(protoEnabled, kCFNumberIntType, &enabled) && enabled) {
         QString host = QCFString::toQString(protoHost);

         int port;
         CFNumberGetValue(protoPort, kCFNumberIntType, &port);

         return QNetworkProxy(type, host, port);
      }
   }

   // proxy not enabled
   return QNetworkProxy();
}

static QNetworkProxy proxyFromDictionary(CFDictionaryRef dict)
{
   QNetworkProxy::ProxyType proxyType = QNetworkProxy::DefaultProxy;

   QString hostName;
   quint16 port = 0;
   QString user;
   QString password;

   CFStringRef cfProxyType = (CFStringRef)CFDictionaryGetValue(dict, kCFProxyTypeKey);
   if (CFStringCompare(cfProxyType, kCFProxyTypeNone, 0) == kCFCompareEqualTo) {
      proxyType = QNetworkProxy::NoProxy;
   } else if (CFStringCompare(cfProxyType, kCFProxyTypeFTP, 0) == kCFCompareEqualTo) {
      proxyType = QNetworkProxy::FtpCachingProxy;
   } else if (CFStringCompare(cfProxyType, kCFProxyTypeHTTP, 0) == kCFCompareEqualTo) {
      proxyType = QNetworkProxy::HttpProxy;
   } else if (CFStringCompare(cfProxyType, kCFProxyTypeHTTPS, 0) == kCFCompareEqualTo) {
      proxyType = QNetworkProxy::HttpProxy;
   } else if (CFStringCompare(cfProxyType, kCFProxyTypeSOCKS, 0) == kCFCompareEqualTo) {
      proxyType = QNetworkProxy::Socks5Proxy;
   }

   hostName = QCFString::toQString((CFStringRef)CFDictionaryGetValue(dict, kCFProxyHostNameKey));
   user     = QCFString::toQString((CFStringRef)CFDictionaryGetValue(dict, kCFProxyUsernameKey));
   password = QCFString::toQString((CFStringRef)CFDictionaryGetValue(dict, kCFProxyPasswordKey));

   CFNumberRef portNumber = (CFNumberRef)CFDictionaryGetValue(dict, kCFProxyPortNumberKey);
   if (portNumber) {
      CFNumberGetValue(portNumber, kCFNumberSInt16Type, &port);
   }

   return QNetworkProxy(proxyType, hostName, port, user, password);
}

const char *cfurlErrorDescription(SInt32 errorCode)
{
   switch (errorCode) {
      case kCFURLUnknownError:
         return "Unknown Error";

      case kCFURLUnknownSchemeError:
         return "Unknown Scheme";

      case kCFURLResourceNotFoundError:
         return "Resource Not Found";

      case kCFURLResourceAccessViolationError:
         return "Resource Access Violation";

      case kCFURLRemoteHostUnavailableError:
         return "Remote Host Unavailable";

      case kCFURLImproperArgumentsError:
         return "Improper Arguments";

      case kCFURLUnknownPropertyKeyError:
         return "Unknown Property Key";

      case kCFURLPropertyKeyUnavailableError:
         return "Property Key Unavailable";

      case kCFURLTimeoutError:
         return "Timeout";

      default:
         return "Really Unknown Error";
   }
}

QList<QNetworkProxy> macQueryInternal(const QNetworkProxyQuery &query)
{
   QList<QNetworkProxy> result;

   // obtain a dictionary to the proxy settings:
   CFDictionaryRef dict = SCDynamicStoreCopyProxies(nullptr);
   if (! dict) {
      qWarning("macQueryInternal() SCDynamicStoreCopyProxies invalid (nullptr)");
      return result;          // failed
   }

   if (isHostExcluded(dict, query.peerHostName())) {
      CFRelease(dict);
      return result;          // no proxy for this host
   }

   // is there a PAC enabled? If so, use it first.
   CFNumberRef pacEnabled;

   if ((pacEnabled = (CFNumberRef)CFDictionaryGetValue(dict, kSCPropNetProxiesProxyAutoConfigEnable))) {
      int enabled;

      if (CFNumberGetValue(pacEnabled, kCFNumberIntType, &enabled) && enabled) {
         // PAC is enabled

         CFStringRef pacLocationSetting = (CFStringRef)CFDictionaryGetValue(dict, kSCPropNetProxiesProxyAutoConfigURLString);
         QCFType<CFStringRef> cfPacLocation = CFURLCreateStringByAddingPercentEscapes(kCFAllocatorDefault, pacLocationSetting,
               nullptr, nullptr, kCFStringEncodingUTF8);

         QCFType<CFDataRef> pacData;
         QCFType<CFURLRef> pacUrl = CFURLCreateWithString(kCFAllocatorDefault, cfPacLocation, nullptr);

         if (! pacUrl) {
            qWarning("macQueryInternal() Invalid PAC url %s", csPrintable(QCFString::toQString(cfPacLocation)));
            return result;
         }

         SInt32 errorCode;
         if (! CFURLCreateDataAndPropertiesFromResource(kCFAllocatorDefault, pacUrl, &pacData, nullptr, nullptr, &errorCode)) {
            QString pacLocation = QCFString::toQString(cfPacLocation);

            qWarning("macQueryInternal() Unable to get the PAC script at %s (%s)", csPrintable(pacLocation), cfurlErrorDescription(errorCode));
            return result;
         }

         if (! pacData) {
            qWarning("macQueryInternal() %s returned an empty PAC script", csPrintable(QCFString::toQString(cfPacLocation)));
            return result;
         }

         QCFType<CFStringRef> pacScript = CFStringCreateFromExternalRepresentation(kCFAllocatorDefault, pacData, kCFStringEncodingISOLatin1);

         if (! pacScript) {
            // This should never happen, but the documentation says it may return nullptr if there was a problem creating the object.
            QString pacLocation = QCFString::toQString(cfPacLocation);
            qWarning("macQueryInternal() Unable to read the PAC script at %s", csPrintable(pacLocation));
            return result;
         }

         QByteArray encodedURL = query.url().toEncoded(); // converted to UTF-8
         if (encodedURL.isEmpty()) {
            return result; // Invalid URL, abort
         }

         QCFType<CFURLRef> targetURL = CFURLCreateWithBytes(kCFAllocatorDefault, (UInt8 *)encodedURL.data(),
               encodedURL.size(), kCFStringEncodingUTF8, nullptr);

         if (! targetURL) {
            return result; // URL creation problem, abort
         }

         QCFType<CFErrorRef> pacError;
         QCFType<CFArrayRef> proxies = CFNetworkCopyProxiesForAutoConfigurationScript(pacScript, targetURL, &pacError);

         if (! proxies) {
            QString pacLocation = QCFString::toQString(cfPacLocation);
            QCFType<CFStringRef> pacErrorDescription = CFErrorCopyDescription(pacError);
            qWarning("macQueryInternal() Execution of PAC script at %s failed, %s", csPrintable(pacLocation),
                     csPrintable(QCFString::toQString(pacErrorDescription)));

            return result;
         }

         CFIndex size = CFArrayGetCount(proxies);
         for (CFIndex i = 0; i < size; ++i) {
            CFDictionaryRef proxy = (CFDictionaryRef)CFArrayGetValueAtIndex(proxies, i);
            result << proxyFromDictionary(proxy);
         }

         return result;
      }
   }

   // no PAC, decide which proxy we're looking for based on the query
   bool isHttps = false;
   QString protocol = query.protocolTag().toLower();

   // try the protocol-specific proxy
   QNetworkProxy protocolSpecificProxy;

   if (protocol == "ftp") {
      protocolSpecificProxy = proxyFromDictionary(dict, QNetworkProxy::FtpCachingProxy,
            kSCPropNetProxiesFTPEnable, kSCPropNetProxiesFTPProxy, kSCPropNetProxiesFTPPort);

   } else if (protocol == "http") {
      protocolSpecificProxy = proxyFromDictionary(dict, QNetworkProxy::HttpProxy,
            kSCPropNetProxiesHTTPEnable, kSCPropNetProxiesHTTPProxy, kSCPropNetProxiesHTTPPort);

   } else if (protocol == "https") {
      isHttps = true;

      protocolSpecificProxy = proxyFromDictionary(dict, QNetworkProxy::HttpProxy,
            kSCPropNetProxiesHTTPSEnable, kSCPropNetProxiesHTTPSProxy, kSCPropNetProxiesHTTPSPort);
   }

   if (protocolSpecificProxy.type() != QNetworkProxy::DefaultProxy) {
      result << protocolSpecificProxy;
   }

   // let's add SOCKSv5 if present too
   QNetworkProxy socks5 = proxyFromDictionary(dict, QNetworkProxy::Socks5Proxy,
         kSCPropNetProxiesSOCKSEnable, kSCPropNetProxiesSOCKSProxy, kSCPropNetProxiesSOCKSPort);

   if (socks5.type() != QNetworkProxy::DefaultProxy) {
      result << socks5;
   }

   // add the HTTPS proxy if present (and if we haven't added yet)
   if (! isHttps) {
      QNetworkProxy https = proxyFromDictionary(dict, QNetworkProxy::HttpProxy,
            kSCPropNetProxiesHTTPSEnable, kSCPropNetProxiesHTTPSProxy, kSCPropNetProxiesHTTPSPort);

      if (https.type() != QNetworkProxy::DefaultProxy && https != protocolSpecificProxy) {
         result << https;
      }
   }

   CFRelease(dict);

   return result;
}

QList<QNetworkProxy> QNetworkProxyFactory::systemProxyForQuery(const QNetworkProxyQuery &query)
{
   QList<QNetworkProxy> result = macQueryInternal(query);

   if (result.isEmpty()) {
      result << QNetworkProxy::NoProxy;
   }

   return result;
}

#endif
