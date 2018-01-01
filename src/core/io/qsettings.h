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

#ifndef QSETTINGS_H
#define QSETTINGS_H

#include <QtCore/qobject.h>
#include <QtCore/qvariant.h>
#include <QtCore/qstring.h>
#include <QScopedPointer>

#ifndef QT_NO_SETTINGS

#include <ctype.h>

QT_BEGIN_NAMESPACE

#ifdef Status // we seem to pick up a macro Status --> int somewhere
#undef Status
#endif

class QIODevice;
class QSettingsPrivate;

class Q_CORE_EXPORT QSettings : public QObject
{
   CORE_CS_OBJECT(QSettings)
   Q_DECLARE_PRIVATE(QSettings)

 public:
   enum Status {
      NoError = 0,
      AccessError,
      FormatError
   };

   enum Format {
      NativeFormat,
      IniFormat,

      InvalidFormat = 16,
      CustomFormat1,
      CustomFormat2,
      CustomFormat3,
      CustomFormat4,
      CustomFormat5,
      CustomFormat6,
      CustomFormat7,
      CustomFormat8,
      CustomFormat9,
      CustomFormat10,
      CustomFormat11,
      CustomFormat12,
      CustomFormat13,
      CustomFormat14,
      CustomFormat15,
      CustomFormat16
   };

   enum Scope {
      UserScope,
      SystemScope
   };

   explicit QSettings(const QString &organization,
                      const QString &application = QString(), QObject *parent = nullptr);
   QSettings(Scope scope, const QString &organization,
             const QString &application = QString(), QObject *parent = nullptr);
   QSettings(Format format, Scope scope, const QString &organization,
             const QString &application = QString(), QObject *parent = nullptr);
   QSettings(const QString &fileName, Format format, QObject *parent = nullptr);
   explicit QSettings(QObject *parent = nullptr);

   ~QSettings();

   void clear();
   void sync();
   Status status() const;

   void beginGroup(const QString &prefix);
   void endGroup();
   QString group() const;

   int beginReadArray(const QString &prefix);
   void beginWriteArray(const QString &prefix, int size = -1);
   void endArray();
   void setArrayIndex(int i);

   QStringList allKeys() const;
   QStringList childKeys() const;
   QStringList childGroups() const;
   bool isWritable() const;

   void setValue(const QString &key, const QVariant &value);
   QVariant value(const QString &key, const QVariant &defaultValue = QVariant()) const;

   void remove(const QString &key);
   bool contains(const QString &key) const;

   void setFallbacksEnabled(bool b);
   bool fallbacksEnabled() const;

   QString fileName() const;
   Format format() const;
   Scope scope() const;
   QString organizationName() const;
   QString applicationName() const;

#ifndef QT_NO_TEXTCODEC
   void setIniCodec(QTextCodec *codec);
   void setIniCodec(const char *codecName);
   QTextCodec *iniCodec() const;
#endif

   static void setDefaultFormat(Format format);
   static Format defaultFormat();
   static void setSystemIniPath(const QString &dir); // ### remove in 5.0 (use setPath() instead)
   static void setUserIniPath(const QString &dir);   // ### remove in 5.0 (use setPath() instead)
   static void setPath(Format format, Scope scope, const QString &path);

   typedef QMap<QString, QVariant> SettingsMap;
   typedef bool (*ReadFunc)(QIODevice &device, SettingsMap &map);
   typedef bool (*WriteFunc)(QIODevice &device, const SettingsMap &map);

   static Format registerFormat(const QString &extension, ReadFunc readFunc, WriteFunc writeFunc,
                                Qt::CaseSensitivity caseSensitivity = Qt::CaseSensitive);

 protected:
   bool event(QEvent *event) override;
   QScopedPointer<QSettingsPrivate> d_ptr;

 private:
   Q_DISABLE_COPY(QSettings)

};

QT_END_NAMESPACE

#endif // QT_NO_SETTINGS

#endif // QSETTINGS_H
