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

#include <qcoreapplication.h>
#include <qdatetime.h>
#include <qdir.h>
#include <qsettings.h>
#include <qtimezone.h>
#include <qvarlengtharray.h>

#include <qcore_mac_p.h>
#include <qsettings_p.h>

#include <Security/SecCode.h>
#include <Security/SecRequirement.h>

static const CFStringRef hostNames[2] = { kCFPreferencesCurrentHost, kCFPreferencesAnyHost };
static constexpr const int numHostNames = 2;

/*
    On the Mac, it is more natural to use '.' as the key separator
    than '/'. Therefore, it makes sense to replace '/' with '.' in
    keys. Then we replace '.' with middle dots (which we can't show
    here) and middle dots with '/'. A key like "4.0/BrowserCommand"
    becomes "4<middot>0.BrowserCommand".
*/

enum RotateShift {
   Macify = 1,
   Qtify = 2
};

static QString rotateSlashesDotsAndMiddots(const QString &key, int shift)
{
   static constexpr const int NumKnights = 3;
   static constexpr const char knightsOfTheRoundTable[NumKnights] = { '/', '.', '\xb7' };

   QString result = key;

   for (int i = 0; i < result.size(); ++i) {

      for (int j = 0; j < NumKnights; ++j) {

         if (result.at(i) == knightsOfTheRoundTable[j]) {
            QChar tmp = char32_t(knightsOfTheRoundTable[(j + shift) % NumKnights]);
            result.replace(i, 1, tmp);

            break;
         }

      }
   }

   return result;
}

static QCFType<CFStringRef> macKey(const QString &key)
{
   return QCFString::toCFStringRef(rotateSlashesDotsAndMiddots(key, Macify));
}

static QString qtKey(CFStringRef cfkey)
{
   return rotateSlashesDotsAndMiddots(QCFString::toQString(cfkey), Qtify);
}

static QCFType<CFPropertyListRef> macValue(const QVariant &value);

static CFArrayRef macList(const QList<QVariant> &list)
{
   int n = list.size();
   QVarLengthArray<QCFType<CFPropertyListRef>> cfvalues(n);

   for (int i = 0; i < n; ++i) {
      cfvalues[i] = macValue(list.at(i));
   }

   return CFArrayCreate(kCFAllocatorDefault, reinterpret_cast<const void **>(cfvalues.data()),
         CFIndex(n), &kCFTypeArrayCallBacks);
}

static QCFType<CFPropertyListRef> macValue(const QVariant &value)
{
   CFPropertyListRef result = nullptr;

   switch (value.type()) {
      case QVariant::ByteArray: {
         QByteArray ba = value.toByteArray();
         result = CFDataCreate(kCFAllocatorDefault, reinterpret_cast<const UInt8 *>(ba.data()), CFIndex(ba.size()));
      }
      break;

      // should be same as below (look for LIST)
      case QVariant::List:
      case QVariant::StringList:
      case QVariant::Polygon:
         result = macList(value.toList());
         break;

      case QVariant::Map: {
         /*
             QMap<QString, QVariant> is potentially a multimap,
             whereas CFDictionary is a single-valued map. To allow
             for multiple values with the same key, we store
             multiple values in a CFArray. To avoid ambiguities,
             we also wrap lists in a CFArray singleton.
         */
         QMap<QString, QVariant> map = value.toMap();
         QMap<QString, QVariant>::const_iterator i = map.constBegin();

         int maxUniqueKeys = map.size();
         int numUniqueKeys = 0;

         QVarLengthArray<QCFType<CFPropertyListRef>> cfkeys(maxUniqueKeys);
         QVarLengthArray<QCFType<CFPropertyListRef>> cfvalues(maxUniqueKeys);

         while (i != map.constEnd()) {
            const QString &key = i.key();
            QList<QVariant> values;

            do {
               values << i.value();
               ++i;
            } while (i != map.constEnd() && i.key() == key);

            bool singleton = (values.count() == 1);

            if (singleton) {
               switch (values.first().type()) {
                  // should be same as above (look for LIST)
                  case QVariant::List:
                  case QVariant::StringList:
                  case QVariant::Polygon:
                     singleton = false;

                  default:
                     ;
               }
            }

            cfkeys[numUniqueKeys] = QCFString::toCFStringRef(key);
            cfvalues[numUniqueKeys] = singleton ? macValue(values.first()) : macList(values);
            ++numUniqueKeys;
         }

         result = CFDictionaryCreate(kCFAllocatorDefault,
               reinterpret_cast<const void **>(cfkeys.data()), reinterpret_cast<const void **>(cfvalues.data()),
               CFIndex(numUniqueKeys), &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
      }
      break;

      case QVariant::DateTime: {
         //    CFDate, unlike QDateTime, doesn't store timezone information

         QDateTime dt = value.toDateTime();

         if (dt.timeZone() == QTimeZone::systemTimeZone()) {
            QDateTime reference;
            reference.setTime_t((uint)kCFAbsoluteTimeIntervalSince1970);
            result = CFDateCreate(kCFAllocatorDefault, CFAbsoluteTime(reference.secsTo(dt)));
         } else {
            goto string_case;
         }
      }
      break;

      case QVariant::Bool:
         result = value.toBool() ? kCFBooleanTrue : kCFBooleanFalse;
         break;

      case QVariant::Int:
      case QVariant::UInt: {
         int n  = value.toInt();
         result = CFNumberCreate(kCFAllocatorDefault, kCFNumberIntType, &n);
      }

      break;

      case QVariant::Double: {
         double n = value.toDouble();
         result = CFNumberCreate(kCFAllocatorDefault, kCFNumberDoubleType, &n);
      }
      break;

      case QVariant::LongLong:
      case QVariant::ULongLong: {
         qint64 n = value.toLongLong();
         result = CFNumberCreate(nullptr, kCFNumberLongLongType, &n);
      }
      break;

      case QVariant::String:
string_case:
      default:
         QString string = QSettingsPrivate::variantToString(value);
         result = QCFString::toCFStringRef(string);
   }

   return result;
}

static QVariant qtValue(CFPropertyListRef cfvalue)
{
   if (! cfvalue) {
      return QVariant();
   }

   CFTypeID typeId = CFGetTypeID(cfvalue);

   // Sorted grossly from most to least frequent type
   if (typeId == CFStringGetTypeID()) {
      return QSettingsPrivate::stringToVariant(QCFString::toQString(static_cast<CFStringRef>(cfvalue)));

   } else if (typeId == CFNumberGetTypeID()) {
      CFNumberRef cfnumber = static_cast<CFNumberRef>(cfvalue);

      if (CFNumberIsFloatType(cfnumber)) {
         double d;
         CFNumberGetValue(cfnumber, kCFNumberDoubleType, &d);
         return d;

      } else {
         int i;
         qint64 ll;

         if (CFNumberGetType(cfnumber) == kCFNumberIntType) {
            CFNumberGetValue(cfnumber, kCFNumberIntType, &i);
            return i;
         }

         CFNumberGetValue(cfnumber, kCFNumberLongLongType, &ll);
         return ll;
      }

   } else if (typeId == CFArrayGetTypeID()) {
      CFArrayRef cfarray = static_cast<CFArrayRef>(cfvalue);
      QList<QVariant> list;
      CFIndex size = CFArrayGetCount(cfarray);
      bool metNonString = false;

      for (CFIndex i = 0; i < size; ++i) {
         QVariant value = qtValue(CFArrayGetValueAtIndex(cfarray, i));

         if (value.type() != QVariant::String) {
            metNonString = true;
         }

         list << value;
      }

      if (metNonString) {
         return list;
      } else {
         return QVariant(list).toStringList();
      }

   } else if (typeId == CFBooleanGetTypeID()) {
      return (bool)CFBooleanGetValue(static_cast<CFBooleanRef>(cfvalue));

   } else if (typeId == CFDataGetTypeID()) {
      CFDataRef cfdata = static_cast<CFDataRef>(cfvalue);

      return QByteArray(reinterpret_cast<const char *>(CFDataGetBytePtr(cfdata)),
            CFDataGetLength(cfdata));

   } else if (typeId == CFDictionaryGetTypeID()) {
      CFDictionaryRef cfdict = static_cast<CFDictionaryRef>(cfvalue);
      CFTypeID arrayTypeId = CFArrayGetTypeID();
      int size = (int)CFDictionaryGetCount(cfdict);
      QVarLengthArray<CFPropertyListRef> keys(size);
      QVarLengthArray<CFPropertyListRef> values(size);
      CFDictionaryGetKeysAndValues(cfdict, keys.data(), values.data());

      QMultiMap<QString, QVariant> map;

      for (int i = 0; i < size; ++i) {
         QString key = QCFString::toQString(static_cast<CFStringRef>(keys[i]));

         if (CFGetTypeID(values[i]) == arrayTypeId) {
            CFArrayRef cfarray = static_cast<CFArrayRef>(values[i]);
            CFIndex arraySize = CFArrayGetCount(cfarray);

            for (CFIndex j = arraySize - 1; j >= 0; --j) {
               map.insert(key, qtValue(CFArrayGetValueAtIndex(cfarray, j)));
            }

         } else {
            map.insert(key, qtValue(values[i]));
         }
      }

      return map;

   } else if (typeId == CFDateGetTypeID()) {
      QDateTime dt;
      dt.setTime_t((uint)kCFAbsoluteTimeIntervalSince1970);

      return dt.addSecs((int)CFDateGetAbsoluteTime(static_cast<CFDateRef>(cfvalue)));
   }

   return QVariant();
}

static QString comify(const QString &organization)
{
   for (int i = organization.size() - 1; i >= 0; --i) {
      QChar ch = organization.at(i);

      if (ch == QChar('.') || ch == QChar(0x3002) || ch == QChar(0xff0e) || ch == QChar(0xff61)) {
         QString suffix = organization.mid(i + 1).toLower();

         if (suffix.size() == 2 || suffix == "com"  || suffix == "org" || suffix == "net"   ||
               suffix == "edu"  || suffix == "gov"  || suffix == "mil"  || suffix == "biz"  ||
               suffix == "info" || suffix == "name" || suffix == "pro"  || suffix == "aero" ||
               suffix == "coop" || suffix == "museum") {

            QString result = organization;
            result.replace(QChar('/'), QChar(' '));

            return result;
         }

         break;
      }

      int uc = ch.unicode();

      if ((uc < 'a' || uc > 'z') && (uc < 'A' || uc > 'Z')) {
         break;
      }
   }

   QString domain;

   for (int i = 0; i < organization.size(); ++i) {
      QChar ch = organization.at(i);
      int uc   = ch.unicode();

      if ((uc >= 'a' && uc <= 'z') || (uc >= '0' && uc <= '9')) {
         domain += ch;

      } else if (uc >= 'A' && uc <= 'Z') {
         domain += ch.toLower();

      } else {
         domain += QChar(' ');

      }
   }

   domain = domain.simplified();
   domain.replace(QChar(' '), QChar('-'));

   if (! domain.isEmpty()) {
      domain.append(".com");
   }

   return domain;
}

class QMacSettingsPrivate : public QSettingsPrivate
{
 public:
   QMacSettingsPrivate(QSettings::Scope scope, const QString &organization, const QString &application);

   ~QMacSettingsPrivate();

   void remove(const QString &key);
   void set(const QString &key, const QVariant &value);
   bool get(const QString &key, QVariant *value) const;
   QStringList children(const QString &prefix, ChildSpec spec) const;
   void clear();
   void sync();
   void flush();
   bool isWritable() const;
   QString fileName() const;

 private:
   struct SearchDomain {
      CFStringRef userName;
      CFStringRef applicationOrSuiteId;
   };

   QCFString applicationId;
   QCFString suiteId;
   QCFString hostName;
   SearchDomain domains[6];
   int numDomains;
};

QMacSettingsPrivate::QMacSettingsPrivate(QSettings::Scope scope, const QString &organization, const QString &application)
   : QSettingsPrivate(QSettings::NativeFormat, scope, organization, application)
{
   QString javaPackageName;

   int curPos = 0;
   int nextDot;

   QString domainName = comify(organization);

   if (domainName.isEmpty()) {
      CFBundleRef main_bundle = CFBundleGetMainBundle();

      if (main_bundle != nullptr) {
         CFStringRef main_bundle_identifier = CFBundleGetIdentifier(main_bundle);

         if (main_bundle_identifier != nullptr) {
            QString bundle_identifier(qtKey(main_bundle_identifier));

            // CFBundleGetIdentifier returns identifier separated by slashes rather than periods.
            QStringList bundle_identifier_components = bundle_identifier.split('/');

            // pre-reverse them so that when they get reversed again below, they are in the com.company.product format.
            QStringList bundle_identifier_components_reversed;

            for (int i = 0; i < bundle_identifier_components.size(); ++i) {
               const QString &bundle_identifier_component = bundle_identifier_components.at(i);
               bundle_identifier_components_reversed.push_front(bundle_identifier_component);
            }

            domainName = bundle_identifier_components_reversed.join(".");
         }
      }
   }

   // if no bundle identifier yet. use a hard coded string.
   if (domainName.isEmpty()) {
      domainName = "unknown-organization.copperspice.com";
   }

   while ((nextDot = domainName.indexOf('.', curPos)) != -1) {
      javaPackageName.prepend(domainName.midView(curPos, nextDot - curPos));
      javaPackageName.prepend('.');
      curPos = nextDot + 1;
   }

   javaPackageName.prepend(domainName.midView(curPos));
   javaPackageName = javaPackageName.toLower();

   if (curPos == 0) {
      javaPackageName.prepend("com.");
   }

   suiteId = javaPackageName;

   if (scope == QSettings::SystemScope) {
      m_spec |= F_System;
   }

   if (application.isEmpty()) {
      m_spec |= F_Organization;
   } else {
      javaPackageName += '.';
      javaPackageName += application;
      applicationId = javaPackageName;
   }

   numDomains = 0;

   for (int i = (m_spec & F_System) ? 1 : 0; i < 2; ++i) {
      for (int j = (m_spec & F_Organization) ? 1 : 0; j < 3; ++j) {
         SearchDomain &domain = domains[numDomains++];
         domain.userName = (i == 0) ? kCFPreferencesCurrentUser : kCFPreferencesAnyUser;

         if (j == 0) {
            domain.applicationOrSuiteId = applicationId.toCFStringRef();
         } else if (j == 1) {
            domain.applicationOrSuiteId = suiteId.toCFStringRef();
         } else {
            domain.applicationOrSuiteId = kCFPreferencesAnyApplication;
         }
      }
   }

   hostName = (scope == QSettings::SystemScope) ? kCFPreferencesCurrentHost : kCFPreferencesAnyHost;
   sync();
}

QMacSettingsPrivate::~QMacSettingsPrivate()
{
}

void QMacSettingsPrivate::remove(const QString &key)
{
   QStringList keys = children(key + QChar('/'), AllKeys);

   // If i == -1, then delete "key" itself.
   for (int i = -1; i < keys.size(); ++i) {
      QString subKey = key;

      if (i >= 0) {
         subKey += QChar('/');
         subKey += keys.at(i);
      }

      CFPreferencesSetValue(macKey(subKey), nullptr, domains[0].applicationOrSuiteId,
            domains[0].userName, hostName.toCFStringRef());
   }
}

void QMacSettingsPrivate::set(const QString &key, const QVariant &value)
{
   CFPreferencesSetValue(macKey(key), macValue(value), domains[0].applicationOrSuiteId,
         domains[0].userName, hostName.toCFStringRef());
}

bool QMacSettingsPrivate::get(const QString &key, QVariant *value) const
{
   QCFString k = macKey(key);

   for (int i = 0; i < numDomains; ++i) {

      for (int j = 0; j < numHostNames; ++j) {

         QCFType<CFPropertyListRef> ret = CFPreferencesCopyValue(k.toCFStringRef(), domains[i].applicationOrSuiteId,
               domains[i].userName, hostNames[j]);

         if (ret) {
            if (value) {
               *value = qtValue(ret);
            }

            return true;
         }
      }

      if (!fallbacks) {
         break;
      }
   }

   return false;
}

QStringList QMacSettingsPrivate::children(const QString &prefix, ChildSpec spec) const
{
   QMap<QString, QString> result;
   int startPos = prefix.size();

   for (int i = 0; i < numDomains; ++i) {
      for (int j = 0; j < numHostNames; ++j) {

         QCFType<CFArrayRef> cfarray = CFPreferencesCopyKeyList(domains[i].applicationOrSuiteId,
               domains[i].userName, hostNames[j]);

         if (cfarray) {
            CFIndex size = CFArrayGetCount(cfarray);

            for (CFIndex k = 0; k < size; ++k) {
               QString currentKey = qtKey(static_cast<CFStringRef>(CFArrayGetValueAtIndex(cfarray, k)));

               if (currentKey.startsWith(prefix)) {
                  processChild(currentKey.midView(startPos), spec, result);
               }
            }
         }
      }

      if (! fallbacks) {
         break;
      }
   }

   return result.keys();
}

void QMacSettingsPrivate::clear()
{
   QCFType<CFArrayRef> cfarray = CFPreferencesCopyKeyList(domains[0].applicationOrSuiteId,
         domains[0].userName, hostName.toCFStringRef());

   CFPreferencesSetMultiple(nullptr, cfarray, domains[0].applicationOrSuiteId, domains[0].userName,
         hostName.toCFStringRef());
}

void QMacSettingsPrivate::sync()
{
   for (int i = 0; i < numDomains; ++i) {

      for (int j = 0; j < numHostNames; ++j) {
         Boolean ok = CFPreferencesSynchronize(domains[i].applicationOrSuiteId,
               domains[i].userName, hostNames[j]);

         // only report failures for the primary file (the one we write to)
         if (! ok && i == 0 && hostNames[j] == hostName.toCFStringRef() && m_status == QSettings::NoError) {
            setStatus(QSettings::AccessError);
         }
      }
   }
}

void QMacSettingsPrivate::flush()
{
   sync();
}

bool QMacSettingsPrivate::isWritable() const
{
   QMacSettingsPrivate *that = const_cast<QMacSettingsPrivate *>(this);
   QString impossibleKey("qt_internal/");

   QSettings::Status oldStatus = that->m_status;
   that->m_status = QSettings::NoError;

   that->set(impossibleKey, QVariant());
   that->sync();
   bool writable = (m_status == QSettings::NoError) && that->get(impossibleKey, nullptr);
   that->remove(impossibleKey);
   that->sync();

   that->m_status = oldStatus;

   return writable;
}

QString QMacSettingsPrivate::fileName() const
{
   QString result;

   if ((m_spec & F_System) == 0) {
      result = QDir::homePath();
   }

   result += "/Library/Preferences/";
   result += QCFString::toQString(domains[0].applicationOrSuiteId);
   result += ".plist";

   return result;
}

QSettingsPrivate *QSettingsPrivate::create(QSettings::Format format,
      QSettings::Scope scope,
      const QString &organization,
      const QString &application)
{
   if (organization == "CS")   {
      QString organizationDomain = QCoreApplication::organizationDomain();
      QString applicationName    = QCoreApplication::applicationName();

      QSettingsPrivate *newSettings;

      if (format == QSettings::NativeFormat) {
         newSettings = new QMacSettingsPrivate(scope, organizationDomain, applicationName);
      } else {
         newSettings = new QConfFileSettingsPrivate(format, scope, organizationDomain, applicationName);
      }

      newSettings->beginGroupOrArray(QSettingsGroup(normalizedKey(organization)));

      if (! application.isEmpty()) {
         newSettings->beginGroupOrArray(QSettingsGroup(normalizedKey(application)));
      }

      return newSettings;
   }

   if (format == QSettings::NativeFormat) {
      return new QMacSettingsPrivate(scope, organization, application);
   } else {
      return new QConfFileSettingsPrivate(format, scope, organization, application);
   }
}

bool QConfFileSettingsPrivate::readPlistFile(const QString &fileName, ParsedSettingsMap *map) const
{
   QFile file(fileName);

   if (! file.open(QIODevice::ReadOnly)) {
      return false;
   }

   QByteArray data = file.readAll();

   QCFType<CFDataRef> resource = CFDataCreateWithBytesNoCopy(kCFAllocatorDefault,
         reinterpret_cast<const UInt8 *>(data.constData()), data.length(), kCFAllocatorNull);

   QCFType<CFPropertyListRef> propertyList = CFPropertyListCreateWithData(kCFAllocatorDefault, resource,
         kCFPropertyListImmutable, nullptr, nullptr);

   if (! propertyList) {
      return true;
   }

   if (CFGetTypeID(propertyList) != CFDictionaryGetTypeID()) {
      return false;
   }

   CFDictionaryRef cfdict = static_cast<CFDictionaryRef>(static_cast<CFPropertyListRef>(propertyList));
   int size = (int)CFDictionaryGetCount(cfdict);
   QVarLengthArray<CFPropertyListRef> keys(size);
   QVarLengthArray<CFPropertyListRef> values(size);
   CFDictionaryGetKeysAndValues(cfdict, keys.data(), values.data());

   for (int i = 0; i < size; ++i) {
      QString key = qtKey(static_cast<CFStringRef>(keys[i]));
      map->insert(QSettingsKey(key, Qt::CaseSensitive), qtValue(values[i]));
   }

   return true;
}

bool QConfFileSettingsPrivate::writePlistFile(const QString &fileName, const ParsedSettingsMap &map) const
{
   QVarLengthArray<QCFType<CFStringRef>> cfkeys(map.size());
   QVarLengthArray<QCFType<CFPropertyListRef>> cfvalues(map.size());
   int i = 0;

   ParsedSettingsMap::const_iterator j;

   for (j = map.constBegin(); j != map.constEnd(); ++j) {
      cfkeys[i] = macKey(j.key());
      cfvalues[i] = macValue(j.value());
      ++i;
   }

   QCFType<CFDictionaryRef> propertyList = CFDictionaryCreate(kCFAllocatorDefault,
         reinterpret_cast<const void **>(cfkeys.data()),
         reinterpret_cast<const void **>(cfvalues.data()),
         CFIndex(map.size()), &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);

   QCFType<CFDataRef> xmlData = CFPropertyListCreateData(kCFAllocatorDefault, propertyList,
         kCFPropertyListXMLFormat_v1_0, 0, nullptr);

   auto len = CFDataGetLength(xmlData);
   QByteArray data(reinterpret_cast<const char *>(CFDataGetBytePtr(xmlData)), len);

   QFile file(fileName);

   if (file.open(QIODevice::WriteOnly)) {
      return file.write(data) == len;
   } else {
      return false;
   }
}
