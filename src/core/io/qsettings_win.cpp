/***********************************************************************
*
* Copyright (c) 2012-2019 Barbara Geller
* Copyright (c) 2012-2019 Ansel Sermersheim
*
* Copyright (C) 2015 The Qt Company Ltd.
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

#ifndef QT_NO_SETTINGS

#include <qsettings_p.h>
#include <qvector.h>
#include <qmap.h>
#include <qt_windows.h>
#include <qdebug.h>

/*  Keys are stored in QStrings

    If the variable name starts with 'u', this is a "user" key
    ie. "foo/bar/alpha/beta"

    If the variable name starts with 'r', this is a "registry" key
    ie. "\foo\bar\alpha\beta"
*/


//  We do not use KEY_ALL_ACCESS because it gives more rights than what we need. See task 199061.
static const REGSAM registryPermissions = KEY_READ | KEY_WRITE;

static QString keyPath(const QString &rKey)
{
   int idx = rKey.lastIndexOf('\\');

   if (idx == -1) {
      return QString();
   }

   return rKey.left(idx + 1);
}

static QString keyName(const QString &rKey)
{
   QString retval;
   int idx = rKey.lastIndexOf('\\');

   if (idx == -1) {
      retval = rKey;
   } else {
      retval = rKey.mid(idx + 1);
   }

   if (retval == "Default" || retval == ".") {
      retval = "";
   }

   return retval;
}

static QString escapedKey(QString uKey)
{
   QString retval;

   for (auto c : uKey) {

      if (c == '\\') {
         c = '/';

      } else if (c == '/') {
         c = '\\';
      }

      retval.append(c);
   }

   return retval;
}

static QString unescapedKey(QString rKey)
{
   return escapedKey(rKey);
}

typedef QMap<QString, QString> NameSet;

static void mergeKeySets(NameSet *dest, const NameSet &src)
{
   NameSet::const_iterator it = src.constBegin();
   for (; it != src.constEnd(); ++it) {
      dest->insert(unescapedKey(it.key()), QString());
   }
}

static void mergeKeySets(NameSet *dest, const QStringList &src)
{
   QStringList::const_iterator it = src.constBegin();
   for (; it != src.constEnd(); ++it) {
      dest->insert(unescapedKey(*it), QString());
   }
}

//  Wrappers for the windows registry API
static QString errorCodeToString(DWORD errorCode)
{
   wchar_t *data = nullptr;
   FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, 0, errorCode, 0, (LPTSTR)&data, 0, 0);

   QString result;

   if (data != nullptr) {
      result = QString::fromStdWString(std::wstring(data));
      LocalFree(data);
   }

   if (result.endsWith('\n')) {
      result.truncate(result.length() - 1);
   }

   return result;
}

// Open a key with the specified perms
static HKEY openKey(HKEY parentHandle, REGSAM perms, const QString &rSubKey)
{
   HKEY resultHandle = 0;
   LONG res = RegOpenKeyEx(parentHandle, &rSubKey.toStdWString()[0], 0, perms, &resultHandle);

   if (res == ERROR_SUCCESS) {
      return resultHandle;
   }

   return 0;
}

// Open a key with the specified perms, create it if it does not exist
static HKEY createOrOpenKey(HKEY parentHandle, REGSAM perms, const QString &rSubKey)
{
   // try to open it
   HKEY resultHandle = openKey(parentHandle, perms, rSubKey);
   if (resultHandle != 0) {
      return resultHandle;
   }

   // try to create it
   LONG res = RegCreateKeyEx(parentHandle, &rSubKey.toStdWString()[0], 0, 0, REG_OPTION_NON_VOLATILE, perms, 0, &resultHandle, 0);

   if (res == ERROR_SUCCESS) {
      return resultHandle;
   }

   return 0;
}

// Open or create a key in read-write mode if possible, otherwise read-only
static HKEY createOrOpenKey(HKEY parentHandle, const QString &rSubKey, bool *readOnly)
{
   // try to open or create it read/write
   HKEY resultHandle = createOrOpenKey(parentHandle, registryPermissions, rSubKey);

   if (resultHandle != 0) {
      if (readOnly != 0) {
         *readOnly = false;
      }
      return resultHandle;
   }

   // try to open or create it read/only
   resultHandle = createOrOpenKey(parentHandle, KEY_READ, rSubKey);

   if (resultHandle != 0) {
      if (readOnly != 0) {
         *readOnly = true;
      }
      return resultHandle;
   }
   return 0;
}

static QStringList childKeysOrGroups(HKEY parentHandle, QSettingsPrivate::ChildSpec spec)
{
   QStringList result;

   DWORD numKeys;
   DWORD maxKeySize;
   DWORD numSubgroups;
   DWORD maxSubgroupSize;

   // Find the number of keys and subgroups, as well as the max of their lengths.
   LONG res = RegQueryInfoKey(parentHandle, 0, 0, 0, &numSubgroups, &maxSubgroupSize, 0,
                              &numKeys, &maxKeySize, 0, 0, 0);

   if (res != ERROR_SUCCESS) {
      qWarning("QSettings: RegQueryInfoKey() failed: %s", errorCodeToString(res).toLatin1().data());
      return result;
   }

   ++maxSubgroupSize;
   ++maxKeySize;

   int n;
   int m;

   if (spec == QSettingsPrivate::ChildKeys) {
      n = numKeys;
      m = maxKeySize;
   } else {
      n = numSubgroups;
      m = maxSubgroupSize;
   }

   /* The size does not include the terminating null character. */
   ++m;

   // Get the list
   std::wstring buffer(m, L'\0');

   for (int i = 0; i < n; ++i) {
      QString item;
      DWORD l = buffer.size();

      if (spec == QSettingsPrivate::ChildKeys) {
         res = RegEnumValue(parentHandle, i, &buffer[0], &l, 0, 0, 0, 0);
      } else {
         res = RegEnumKeyEx(parentHandle, i, &buffer[0], &l, 0, 0, 0, 0);
      }

      if (res == ERROR_SUCCESS) {
         item = QString::fromStdWString(buffer, l);
      }

      if (res != ERROR_SUCCESS) {
         qWarning("QSettings: RegEnumValue failed: %s", errorCodeToString(res).toLatin1().data());
         continue;
      }

      if (item.isEmpty()) {
         item = ".";
      }

      result.append(item);
   }

   return result;
}

static void allKeys(HKEY parentHandle, const QString &rSubKey, NameSet *result)
{
   HKEY handle = openKey(parentHandle, KEY_READ, rSubKey);
   if (handle == 0) {
      return;
   }

   QStringList childKeys   = childKeysOrGroups(handle, QSettingsPrivate::ChildKeys);
   QStringList childGroups = childKeysOrGroups(handle, QSettingsPrivate::ChildGroups);
   RegCloseKey(handle);

   for (int i = 0; i < childKeys.size(); ++i) {
      QString s = rSubKey;

      if (!s.isEmpty()) {
         s += '\\';
      }

      s += childKeys.at(i);
      result->insert(s, QString());
   }

   for (int i = 0; i < childGroups.size(); ++i) {
      QString s = rSubKey;

      if (!s.isEmpty()) {
         s += '\\';
      }

      s += childGroups.at(i);
      allKeys(parentHandle, s, result);
   }
}

static void deleteChildGroups(HKEY parentHandle)
{
   QStringList childGroups = childKeysOrGroups(parentHandle, QSettingsPrivate::ChildGroups);

   for (int i = 0; i < childGroups.size(); ++i) {
      QString group = childGroups.at(i);

      // delete subgroups in group
      HKEY childGroupHandle = openKey(parentHandle, registryPermissions, group);
      if (childGroupHandle == 0) {
         continue;
      }

      deleteChildGroups(childGroupHandle);
      RegCloseKey(childGroupHandle);

      // delete group itself
      LONG res = RegDeleteKey(parentHandle, &group.toStdWString()[0]);

      if (res != ERROR_SUCCESS) {
         qWarning("QSettings: RegDeleteKey failed on subkey \"%s\": %s",
                  group.toLatin1().data(), errorCodeToString(res).toLatin1().data());
         return;
      }
   }
}

class RegistryKey
{
 public:
   RegistryKey(HKEY parent_handle = 0, const QString &key = QString(), bool read_only = true);

   QString key() const;
   HKEY handle() const;
   HKEY parentHandle() const;
   bool readOnly() const;
   void close();

 private:
   HKEY m_parent_handle;
   mutable HKEY m_handle;
   QString m_key;
   mutable bool m_read_only;
};

RegistryKey::RegistryKey(HKEY parent_handle, const QString &key, bool read_only)
{
   m_parent_handle = parent_handle;
   m_handle = 0;
   m_read_only = read_only;
   m_key = key;
}

QString RegistryKey::key() const
{
   return m_key;
}

HKEY RegistryKey::handle() const
{
   if (m_handle != 0) {
      return m_handle;
   }

   if (m_read_only) {
      m_handle = openKey(m_parent_handle, KEY_READ, m_key);
   } else {
      m_handle = createOrOpenKey(m_parent_handle, m_key, &m_read_only);
   }

   return m_handle;
}

HKEY RegistryKey::parentHandle() const
{
   return m_parent_handle;
}

bool RegistryKey::readOnly() const
{
   return m_read_only;
}

void RegistryKey::close()
{
   if (m_handle != 0) {
      RegCloseKey(m_handle);
   }
   m_handle = 0;
}

typedef QVector<RegistryKey> RegistryKeyList;

/*******************************************************************************
** class QWinSettingsPrivate
*/

class QWinSettingsPrivate : public QSettingsPrivate
{
 public:
   QWinSettingsPrivate(QSettings::Scope scope, const QString &organization,
                       const QString &application);
   QWinSettingsPrivate(QString rKey);
   ~QWinSettingsPrivate();

   void remove(const QString &uKey) override;
   void set(const QString &uKey, const QVariant &value) override;
   bool get(const QString &uKey, QVariant *value) const override;
   QStringList children(const QString &uKey, ChildSpec spec) const override;
   void clear() override;
   void sync() override;
   void flush() override;
   bool isWritable() const override;

   HKEY writeHandle() const;
   bool readKey(HKEY parentHandle, const QString &rSubKey, QVariant *value) const;

   QString fileName() const override;

 private:
   RegistryKeyList regList; // list of registry locations to search for keys
   bool deleteWriteHandleOnExit;
};

QWinSettingsPrivate::QWinSettingsPrivate(QSettings::Scope scope, const QString &organization, const QString &application)
   : QSettingsPrivate(QSettings::NativeFormat, scope, organization, application)
{
   deleteWriteHandleOnExit = false;

   if (!organization.isEmpty()) {
      QString prefix    = "Software\\" + organization;
      QString orgPrefix = prefix + "\\OrganizationDefaults";
      QString appPrefix = prefix + '\\' + application;

      if (scope == QSettings::UserScope) {
         if (! application.isEmpty()) {
            regList.append(RegistryKey(HKEY_CURRENT_USER, appPrefix, ! regList.isEmpty()));
         }

         regList.append(RegistryKey(HKEY_CURRENT_USER, orgPrefix, ! regList.isEmpty()));
      }

      if (!application.isEmpty()) {
         regList.append(RegistryKey(HKEY_LOCAL_MACHINE, appPrefix, ! regList.isEmpty()));
      }

      regList.append(RegistryKey(HKEY_LOCAL_MACHINE, orgPrefix, ! regList.isEmpty()));
   }

   if (regList.isEmpty()) {
      setStatus(QSettings::AccessError);
   }
}

QWinSettingsPrivate::QWinSettingsPrivate(QString rPath)
   : QSettingsPrivate(QSettings::NativeFormat)
{
   deleteWriteHandleOnExit = false;

   if (rPath.startsWith("\\")) {
      rPath = rPath.mid(1);
   }

   if (rPath.startsWith("HKEY_CURRENT_USER\\")) {
      regList.append(RegistryKey(HKEY_CURRENT_USER, rPath.mid(18), false));

   } else if (rPath == "HKEY_CURRENT_USER") {
      regList.append(RegistryKey(HKEY_CURRENT_USER, QString(), false));

   } else if (rPath.startsWith("HKEY_LOCAL_MACHINE\\")) {
      regList.append(RegistryKey(HKEY_LOCAL_MACHINE, rPath.mid(19), false));

   } else if (rPath == "HKEY_LOCAL_MACHINE") {
      regList.append(RegistryKey(HKEY_LOCAL_MACHINE, QString(), false));

   } else if (rPath.startsWith("HKEY_CLASSES_ROOT\\")) {
      regList.append(RegistryKey(HKEY_CLASSES_ROOT, rPath.mid(18), false));

   } else if (rPath == "HKEY_CLASSES_ROOT") {
      regList.append(RegistryKey(HKEY_CLASSES_ROOT, QString(), false));

   } else if (rPath.startsWith("HKEY_USERS\\")) {
      regList.append(RegistryKey(HKEY_USERS, rPath.mid(11), false));

   } else if (rPath == "HKEY_USERS") {
      regList.append(RegistryKey(HKEY_USERS, QString(), false));

   } else {
      regList.append(RegistryKey(HKEY_LOCAL_MACHINE, rPath, false));
   }
}

bool QWinSettingsPrivate::readKey(HKEY parentHandle, const QString &rSubKey, QVariant *value) const
{
   QString rSubkeyName = keyName(rSubKey);
   QString rSubkeyPath = keyPath(rSubKey);

   // open a handle on the subkey
   HKEY handle = openKey(parentHandle, KEY_READ, rSubkeyPath);
   if (handle == 0) {
      return false;
   }

   // get the size and type of the value
   DWORD dataType;
   DWORD dataSize;
   LONG res = RegQueryValueEx(handle, &rSubkeyName.toStdWString()[0], 0, &dataType, 0, &dataSize);

   if (res != ERROR_SUCCESS) {
      RegCloseKey(handle);
      return false;
   }

   // get the value
   QByteArray data(dataSize, 0);
   res = RegQueryValueEx(handle, &rSubkeyName.toStdWString()[0], 0, 0, reinterpret_cast<unsigned char *>(data.data()), &dataSize);

   if (res != ERROR_SUCCESS) {
      RegCloseKey(handle);
      return false;
   }

   switch (dataType) {
      case REG_EXPAND_SZ:
      case REG_SZ: {
         QString s;

         if (dataSize) {
            s = QString::fromUtf16((const char16_t *)data.constData());
         }

         if (value != 0) {
            *value = stringToVariant(s);
         }
         break;
      }

      case REG_MULTI_SZ: {
         QStringList list;

         if (dataSize) {
            int i = 0;

            for (;;) {
               QString s = QString::fromUtf16((const char16_t *)data.constData() + i);
               i += s.length() + 1;

               if (s.isEmpty()) {
                  break;
               }

               list.append(s);
            }
         }

         if (value != 0) {
            *value = stringListToVariantList(list);
         }
         break;
      }

      case REG_NONE:
      case REG_BINARY: {
         QString s;

         if (dataSize) {
            s = QString::fromUtf16((const char16_t *)data.constData(), data.size() / 2);
         }

         if (value != 0) {
            *value = stringToVariant(s);
         }
         break;
      }

      case REG_DWORD_BIG_ENDIAN:
      case REG_DWORD: {
         Q_ASSERT(data.size() == sizeof(int));
         int i;

         memcpy((char *)&i, data.constData(), sizeof(int));

         if (value != 0) {
            *value = i;
         }
         break;
      }

      case REG_QWORD: {
         Q_ASSERT(data.size() == sizeof(qint64));
         qint64 i;
         memcpy((char *)&i, data.constData(), sizeof(qint64));
         if (value != 0) {
            *value = i;
         }
         break;
      }

      default:
         qWarning("QSettings: Unknown data %d type in Windows registry", static_cast<int>(dataType));
         if (value != 0) {
            *value = QVariant();
         }
         break;
   }

   RegCloseKey(handle);
   return true;
}

HKEY QWinSettingsPrivate::writeHandle() const
{
   if (regList.isEmpty()) {
      return 0;
   }
   const RegistryKey &key = regList.at(0);
   if (key.handle() == 0 || key.readOnly()) {
      return 0;
   }
   return key.handle();
}

QWinSettingsPrivate::~QWinSettingsPrivate()
{
   if (deleteWriteHandleOnExit && writeHandle() != 0) {

      QString emptyKey;
      DWORD res = RegDeleteKey(writeHandle(), &emptyKey.toStdWString()[0]);

      if (res != ERROR_SUCCESS) {
         qWarning("QSettings: Failed to delete key \"%s\": %s",
                  regList.at(0).key().toLatin1().data(), errorCodeToString(res).toLatin1().data());
      }
   }

   for (int i = 0; i < regList.size(); ++i) {
      regList[i].close();
   }
}

void QWinSettingsPrivate::remove(const QString &uKey)
{
   if (writeHandle() == 0) {
      setStatus(QSettings::AccessError);
      return;
   }

   QString rKey = escapedKey(uKey);

   // try to delete value bar in key foo
   LONG res;
   HKEY handle = openKey(writeHandle(), registryPermissions, keyPath(rKey));

   if (handle != 0) {
      res = RegDeleteValue(handle, &keyName(rKey).toStdWString()[0]);
      RegCloseKey(handle);
   }

   // try to delete key foo/bar and all subkeys
   handle = openKey(writeHandle(), registryPermissions, rKey);

   if (handle != 0) {
      deleteChildGroups(handle);

      if (rKey.isEmpty()) {
         QStringList childKeys = childKeysOrGroups(handle, QSettingsPrivate::ChildKeys);

         for (int i = 0; i < childKeys.size(); ++i) {
            QString group = childKeys.at(i);

            LONG res = RegDeleteValue(handle, &group.toStdWString()[0]);

            if (res != ERROR_SUCCESS) {
               qWarning("QSettings: RegDeleteValue failed on subkey \"%s\": %s",
                        group.toLatin1().data(), errorCodeToString(res).toLatin1().data());
            }
         }

      } else {
         res = RegDeleteKey(writeHandle(), &rKey.toStdWString()[0]);

         if (res != ERROR_SUCCESS) {
            qWarning("QSettings: RegDeleteKey failed on key \"%s\": %s",
                     rKey.toLatin1().data(), errorCodeToString(res).toLatin1().data());
         }
      }

      RegCloseKey(handle);
   }
}

static bool stringContainsNullChar(const QString &s)
{
   for (int i = 0; i < s.length(); ++i) {
      if (s.at(i).unicode() == 0) {
         return true;
      }
   }

   return false;
}

void QWinSettingsPrivate::set(const QString &uKey, const QVariant &value)
{
   if (writeHandle() == 0) {
      setStatus(QSettings::AccessError);
      return;
   }

   QString rKey = escapedKey(uKey);

   HKEY handle = createOrOpenKey(writeHandle(), registryPermissions, keyPath(rKey));
   if (handle == 0) {
      setStatus(QSettings::AccessError);
      return;
   }

   DWORD type;
   QByteArray regValueBuff;

   // Determine the type
   switch (value.type()) {
      case QVariant::List:
      case QVariant::StringList: {
         // If none of the elements contains '\0', we can use REG_MULTI_SZ, the
         // native registry string list type. Otherwise we use REG_BINARY.

         type = REG_MULTI_SZ;
         QStringList l = variantListToStringList(value.toList());
         QStringList::const_iterator it = l.constBegin();

         for (; it != l.constEnd(); ++it) {
            if ((*it).length() == 0 || stringContainsNullChar(*it)) {
               type = REG_BINARY;
               break;
            }
         }

         if (type == REG_BINARY) {
            QString s = variantToString(value);

            std::wstring tmp = s.toStdWString();

            QByteArray tmpArray;
            tmpArray.resize(tmp.size() * 2);
            memcpy(tmpArray.data(), tmp.data(), tmp.size() * 2);

            regValueBuff = tmpArray;

         } else {
            QStringList::const_iterator it = l.constBegin();

            for (; it != l.constEnd(); ++it) {
               const QString &s = *it;

               std::wstring tmp = s.toStdWString();

               QByteArray tmpArray;
               tmpArray.resize((tmp.size() + 1) * 2);
               memcpy(tmpArray.data(), tmp.data(), (tmp.size() + 1) * 2);

               regValueBuff += tmpArray;
            }

            regValueBuff.append((char)0);
            regValueBuff.append((char)0);
         }
         break;
      }

      case QVariant::Int:
      case QVariant::UInt: {
         type = REG_DWORD;
         qint32 i = value.toInt();
         regValueBuff = QByteArray((const char *)&i, sizeof(qint32));
         break;
      }

      case QVariant::LongLong:
      case QVariant::ULongLong: {
         type = REG_QWORD;
         qint64 i = value.toLongLong();
         regValueBuff = QByteArray((const char *)&i, sizeof(qint64));
         break;
      }

      case QVariant::ByteArray:
      // fallthrough intended

      default: {
         // If the string does not contain '\0', we can use REG_SZ, the native registry
         // string type. Otherwise we use REG_BINARY.
         QString s = variantToString(value);
         type      = stringContainsNullChar(s) ? REG_BINARY : REG_SZ;

         std::wstring tmp = s.toStdWString();

         QByteArray tmpArray;

         if (type == REG_BINARY) {
             tmpArray.resize(tmp.size() * 2);
             memcpy(tmpArray.data(), tmp.data(), tmp.size() * 2);

             regValueBuff == tmpArray;

         } else {
            tmpArray.resize((tmp.size() + 1) * 2);
            memcpy(tmpArray.data(), tmp.data(), (tmp.size() + 1) * 2);

            regValueBuff = tmpArray;
         }
         break;
      }
   }

   // set the value
   LONG res = RegSetValueEx(handle, &keyName(rKey).toStdWString()[0], 0, type,
                  reinterpret_cast<const unsigned char *>(regValueBuff.constData()), regValueBuff.size());

   if (res == ERROR_SUCCESS) {
      deleteWriteHandleOnExit = false;
   } else {
      qWarning("QSettings: failed to set subkey \"%s\": %s", rKey.toLatin1().data(), errorCodeToString(res).toLatin1().data());
      setStatus(QSettings::AccessError);
   }

   RegCloseKey(handle);
}

bool QWinSettingsPrivate::get(const QString &uKey, QVariant *value) const
{
   QString rKey = escapedKey(uKey);

   for (int i = 0; i < regList.size(); ++i) {
      HKEY handle = regList.at(i).handle();
      if (handle != 0 && readKey(handle, rKey, value)) {
         return true;
      }

      if (!fallbacks) {
         return false;
      }
   }

   return false;
}

QStringList QWinSettingsPrivate::children(const QString &uKey, ChildSpec spec) const
{
   NameSet result;
   QString rKey = escapedKey(uKey);

   for (int i = 0; i < regList.size(); ++i) {
      HKEY parent_handle = regList.at(i).handle();
      if (parent_handle == 0) {
         continue;
      }
      HKEY handle = openKey(parent_handle, KEY_READ, rKey);
      if (handle == 0) {
         continue;
      }

      if (spec == AllKeys) {
         NameSet keys;
         allKeys(handle, "", &keys);
         mergeKeySets(&result, keys);
      } else { // ChildGroups or ChildKeys
         QStringList names = childKeysOrGroups(handle, spec);
         mergeKeySets(&result, names);
      }

      RegCloseKey(handle);

      if (!fallbacks) {
         return result.keys();
      }
   }

   return result.keys();
}

void QWinSettingsPrivate::clear()
{
   remove(QString());
   deleteWriteHandleOnExit = true;
}

void QWinSettingsPrivate::sync()
{
   RegFlushKey(writeHandle());
}

void QWinSettingsPrivate::flush()
{
   // Windows does this for us.
}

QString QWinSettingsPrivate::fileName() const
{
   if (regList.isEmpty()) {
      return QString();
   }

   const RegistryKey &key = regList.at(0);
   QString result;

   if (key.parentHandle() == HKEY_CURRENT_USER) {
      result = "\\HKEY_CURRENT_USER\\";
   } else {
      result = "\\HKEY_LOCAL_MACHINE\\";
   }

   return result + regList.at(0).key();
}

bool QWinSettingsPrivate::isWritable() const
{
   return writeHandle() != 0;
}

QSettingsPrivate *QSettingsPrivate::create(QSettings::Format format, QSettings::Scope scope,
      const QString &organization, const QString &application)
{
   if (format == QSettings::NativeFormat) {
      return new QWinSettingsPrivate(scope, organization, application);
   } else {
      return new QConfFileSettingsPrivate(format, scope, organization, application);
   }
}

QSettingsPrivate *QSettingsPrivate::create(const QString &fileName, QSettings::Format format)
{
   if (format == QSettings::NativeFormat) {
      return new QWinSettingsPrivate(fileName);
   } else {
      return new QConfFileSettingsPrivate(fileName, format);
   }
}

#endif // QT_NO_SETTINGS
