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

#ifndef QTIMEZONE_P_H
#define QTIMEZONE_P_H

#include <qtimezone.h>
#include <qlocale_p.h>
#include <qvector.h>

#ifdef Q_OS_DARWIN
#ifdef __OBJC__
@class NSTimeZone;
#else
class NSTimeZone;
#endif
#endif

#ifdef Q_OS_WIN
#include <qt_windows.h>
#endif

#ifdef Q_OS_ANDROID
#include <qjni_p.h>
#endif

class Q_CORE_EXPORT QTimeZonePrivate : public QSharedData
{
 public:
   //Version of QTimeZone::OffsetData struct using msecs for efficiency
   struct Data {
      QString abbreviation;
      qint64 atMSecsSinceEpoch;
      int offsetFromUtc;
      int standardTimeOffset;
      int daylightTimeOffset;
   };
   typedef QVector<Data> DataList;

   // Create null time zone
   QTimeZonePrivate();
   QTimeZonePrivate(const QTimeZonePrivate &other);
   virtual ~QTimeZonePrivate();

   virtual QTimeZonePrivate *clone();

   bool operator==(const QTimeZonePrivate &other) const;
   bool operator!=(const QTimeZonePrivate &other) const;

   bool isValid() const;

   QByteArray id() const;
   virtual QLocale::Country country() const;
   virtual QString comment() const;

   virtual QString displayName(qint64 atMSecsSinceEpoch,
      QTimeZone::NameType nameType,
      const QLocale &locale) const;
   virtual QString displayName(QTimeZone::TimeType timeType,
      QTimeZone::NameType nameType,
      const QLocale &locale) const;
   virtual QString abbreviation(qint64 atMSecsSinceEpoch) const;

   virtual int offsetFromUtc(qint64 atMSecsSinceEpoch) const;
   virtual int standardTimeOffset(qint64 atMSecsSinceEpoch) const;
   virtual int daylightTimeOffset(qint64 atMSecsSinceEpoch) const;

   virtual bool hasDaylightTime() const;
   virtual bool isDaylightTime(qint64 atMSecsSinceEpoch) const;

   virtual Data data(qint64 forMSecsSinceEpoch) const;
   virtual Data dataForLocalTime(qint64 forLocalMSecs) const;

   virtual bool hasTransitions() const;
   virtual Data nextTransition(qint64 afterMSecsSinceEpoch) const;
   virtual Data previousTransition(qint64 beforeMSecsSinceEpoch) const;
   DataList transitions(qint64 fromMSecsSinceEpoch, qint64 toMSecsSinceEpoch) const;

   virtual QByteArray systemTimeZoneId() const;

   virtual QList<QByteArray> availableTimeZoneIds() const;
   virtual QList<QByteArray> availableTimeZoneIds(QLocale::Country country) const;
   virtual QList<QByteArray> availableTimeZoneIds(int utcOffset) const;

   virtual void serialize(QDataStream &ds) const;

   // Static Utility Methods
   static inline qint64 maxMSecs() {
      return std::numeric_limits<qint64>::max();
   }
   static inline qint64 minMSecs() {
      return std::numeric_limits<qint64>::min() + 1;
   }
   static inline qint64 invalidMSecs() {
      return std::numeric_limits<qint64>::min();
   }
   static inline qint64 invalidSeconds() {
      return std::numeric_limits<int>::min();
   }
   static Data invalidData();
   static QTimeZone::OffsetData invalidOffsetData();
   static QTimeZone::OffsetData toOffsetData(const Data &data);
   static bool isValidId(const QByteArray &ianaId);
   static QString isoOffsetFormat(int offsetFromUtc);

   static QByteArray ianaIdToWindowsId(const QByteArray &ianaId);
   static QByteArray windowsIdToDefaultIanaId(const QByteArray &windowsId);
   static QByteArray windowsIdToDefaultIanaId(const QByteArray &windowsId, QLocale::Country country);
   static QList<QByteArray> windowsIdToIanaIds(const QByteArray &windowsId);
   static QList<QByteArray> windowsIdToIanaIds(const QByteArray &windowsId, QLocale::Country country);

   // returns "UTC" QString and QByteArray
   static inline QString utcQString() {
      return QString("UTC");
   }

   static inline QByteArray utcQByteArray() {
      return QByteArray("UTC");
   }

 protected:
   QByteArray m_id;
};
Q_DECLARE_TYPEINFO(QTimeZonePrivate::Data, Q_MOVABLE_TYPE);

template<>
QTimeZonePrivate *QSharedDataPointer<QTimeZonePrivate>::clone();

class  QUtcTimeZonePrivate final : public QTimeZonePrivate
{
 public:
   // Create default UTC time zone
   QUtcTimeZonePrivate();

   // Create named time zone
   QUtcTimeZonePrivate(const QByteArray &utcId);

   // Create offset from UTC
   QUtcTimeZonePrivate(int offsetSeconds);

   // Create custom offset from UTC
   QUtcTimeZonePrivate(const QByteArray &zoneId, int offsetSeconds, const QString &name,
      const QString &abbreviation, QLocale::Country country,
      const QString &comment);

   QUtcTimeZonePrivate(const QUtcTimeZonePrivate &other);
   virtual ~QUtcTimeZonePrivate();

   QTimeZonePrivate *clone() override;

   Data data(qint64 forMSecsSinceEpoch) const override;

   QLocale::Country country() const override;
   QString comment() const override;

   QString displayName(QTimeZone::TimeType timeType,
      QTimeZone::NameType nameType,
      const QLocale &locale) const override;
   QString abbreviation(qint64 atMSecsSinceEpoch) const override;

   int standardTimeOffset(qint64 atMSecsSinceEpoch) const override;
   int daylightTimeOffset(qint64 atMSecsSinceEpoch) const override;

   QByteArray systemTimeZoneId() const override;

   QList<QByteArray> availableTimeZoneIds() const override;
   QList<QByteArray> availableTimeZoneIds(QLocale::Country country) const override;
   QList<QByteArray> availableTimeZoneIds(int utcOffset) const override;

   void serialize(QDataStream &ds) const override;

 private:
   void init(const QByteArray &zoneId);
   void init(const QByteArray &zoneId, int offsetSeconds, const QString &name,
      const QString &abbreviation, QLocale::Country country,
      const QString &comment);

   QString m_name;
   QString m_abbreviation;
   QString m_comment;
   QLocale::Country m_country;
   int m_offsetFromUtc;
};


#if defined Q_OS_UNIX && !defined Q_OS_DARWIN && !defined Q_OS_ANDROID
struct QTzTransitionTime {
   qint64 atMSecsSinceEpoch;
   quint8 ruleIndex;
};

Q_DECLARE_TYPEINFO(QTzTransitionTime, Q_PRIMITIVE_TYPE);
struct QTzTransitionRule {
   int stdOffset;
   int dstOffset;
   quint8 abbreviationIndex;
};

Q_DECLARE_TYPEINFO(QTzTransitionRule, Q_PRIMITIVE_TYPE);

inline bool operator==(const QTzTransitionRule &lhs, const QTzTransitionRule &rhs)
{
   return lhs.stdOffset == rhs.stdOffset && lhs.dstOffset == rhs.dstOffset &&
      lhs.abbreviationIndex == rhs.abbreviationIndex;
}

inline bool operator!=(const QTzTransitionRule &lhs, const QTzTransitionRule &rhs)
{
   return !operator==(lhs, rhs);
}

class QTzTimeZonePrivate final : public QTimeZonePrivate
{
 public:
   // Create default time zone
   QTzTimeZonePrivate();
   // Create named time zone
   QTzTimeZonePrivate(const QByteArray &ianaId);
   QTzTimeZonePrivate(const QTzTimeZonePrivate &other);
   ~QTzTimeZonePrivate();

   QTimeZonePrivate *clone() override;

   QLocale::Country country() const override;
   QString comment() const override;

   QString displayName(qint64 atMSecsSinceEpoch,
      QTimeZone::NameType nameType,
      const QLocale &locale) const override;
   QString displayName(QTimeZone::TimeType timeType,
      QTimeZone::NameType nameType,
      const QLocale &locale) const override;
   QString abbreviation(qint64 atMSecsSinceEpoch) const override;

   int offsetFromUtc(qint64 atMSecsSinceEpoch) const override;
   int standardTimeOffset(qint64 atMSecsSinceEpoch) const override;
   int daylightTimeOffset(qint64 atMSecsSinceEpoch) const override;

   bool hasDaylightTime() const override;
   bool isDaylightTime(qint64 atMSecsSinceEpoch) const override;

   Data data(qint64 forMSecsSinceEpoch) const override;

   bool hasTransitions() const override;
   Data nextTransition(qint64 afterMSecsSinceEpoch) const override;
   Data previousTransition(qint64 beforeMSecsSinceEpoch) const override;

   QByteArray systemTimeZoneId() const override;

   QList<QByteArray> availableTimeZoneIds() const override;
   QList<QByteArray> availableTimeZoneIds(QLocale::Country country) const override;

 private:
   void init(const QByteArray &ianaId);

   Data dataForTzTransition(QTzTransitionTime tran) const;
   QVector<QTzTransitionTime> m_tranTimes;
   QVector<QTzTransitionRule> m_tranRules;
   QList<QByteArray> m_abbreviations;

   QByteArray m_posixRule;
};
#endif // Q_OS_UNIX

#ifdef Q_OS_DARWIN
class QMacTimeZonePrivate final : public QTimeZonePrivate
{
 public:
   // Create default time zone
   QMacTimeZonePrivate();
   // Create named time zone
   QMacTimeZonePrivate(const QByteArray &ianaId);
   QMacTimeZonePrivate(const QMacTimeZonePrivate &other);
   ~QMacTimeZonePrivate();

   QTimeZonePrivate *clone() override;

   QString comment() const override;

   QString displayName(QTimeZone::TimeType timeType, QTimeZone::NameType nameType,
      const QLocale &locale) const override;
   QString abbreviation(qint64 atMSecsSinceEpoch) const override;

   int offsetFromUtc(qint64 atMSecsSinceEpoch) const override;
   int standardTimeOffset(qint64 atMSecsSinceEpoch) const override;
   int daylightTimeOffset(qint64 atMSecsSinceEpoch) const override;

   bool hasDaylightTime() const override;
   bool isDaylightTime(qint64 atMSecsSinceEpoch) const override;

   Data data(qint64 forMSecsSinceEpoch) const override;

   bool hasTransitions() const override;
   Data nextTransition(qint64 afterMSecsSinceEpoch) const override;
   Data previousTransition(qint64 beforeMSecsSinceEpoch) const override;

   QByteArray systemTimeZoneId() const override;

   QList<QByteArray> availableTimeZoneIds() const override;

 private:
   void init(const QByteArray &zoneId);

   NSTimeZone *m_nstz;
};
#endif

#ifdef Q_OS_WIN
class QWinTimeZonePrivate final : public QTimeZonePrivate
{
 public:
   struct QWinTransitionRule {
      int startYear;
      int standardTimeBias;
      int daylightTimeBias;
      SYSTEMTIME standardTimeRule;
      SYSTEMTIME daylightTimeRule;
   };

   // Create default time zone
   QWinTimeZonePrivate();

   // Create named time zone
   QWinTimeZonePrivate(const QByteArray &ianaId);
   QWinTimeZonePrivate(const QWinTimeZonePrivate &other);
   ~QWinTimeZonePrivate();

   QTimeZonePrivate *clone() override;

   QString comment() const override;

   QString displayName(QTimeZone::TimeType timeType, QTimeZone::NameType nameType,
      const QLocale &locale) const override;
   QString abbreviation(qint64 atMSecsSinceEpoch) const override;

   int offsetFromUtc(qint64 atMSecsSinceEpoch) const override;
   int standardTimeOffset(qint64 atMSecsSinceEpoch) const override;
   int daylightTimeOffset(qint64 atMSecsSinceEpoch) const override;

   bool hasDaylightTime() const override;
   bool isDaylightTime(qint64 atMSecsSinceEpoch) const override;

   Data data(qint64 forMSecsSinceEpoch) const override;

   bool hasTransitions() const override;
   Data nextTransition(qint64 afterMSecsSinceEpoch) const override;
   Data previousTransition(qint64 beforeMSecsSinceEpoch) const override;

   QByteArray systemTimeZoneId() const override;

   QList<QByteArray> availableTimeZoneIds() const override;

 private:
   void init(const QByteArray &ianaId);
   QWinTransitionRule ruleForYear(int year) const;
   QTimeZonePrivate::Data ruleToData(const QWinTransitionRule &rule, qint64 atMSecsSinceEpoch, QTimeZone::TimeType type) const;

   QByteArray m_windowsId;
   QString m_displayName;
   QString m_standardName;
   QString m_daylightName;
   QList<QWinTransitionRule> m_tranRules;
};
#endif // Q_OS_WIN

#ifdef Q_OS_ANDROID
class QAndroidTimeZonePrivate final : public QTimeZonePrivate

{
 public:
   // Create default time zone
   QAndroidTimeZonePrivate();
   // Create named time zone
   QAndroidTimeZonePrivate(const QByteArray &ianaId);
   QAndroidTimeZonePrivate(const QAndroidTimeZonePrivate &other);
   ~QAndroidTimeZonePrivate();

   QTimeZonePrivate *clone() override;

   QString displayName(QTimeZone::TimeType timeType, QTimeZone::NameType nameType, const QLocale &locale) const override;
   QString abbreviation(qint64 atMSecsSinceEpoch) const override

   int offsetFromUtc(qint64 atMSecsSinceEpoch) const override;
   int standardTimeOffset(qint64 atMSecsSinceEpoch) const override;
   int daylightTimeOffset(qint64 atMSecsSinceEpoch) const override;

   bool hasDaylightTime() const override;
   bool isDaylightTime(qint64 atMSecsSinceEpoch) const override;

   Data data(qint64 forMSecsSinceEpoch) const override;

   bool hasTransitions() const override;
   Data nextTransition(qint64 afterMSecsSinceEpoch) const override;
   Data previousTransition(qint64 beforeMSecsSinceEpoch) const override;

   Data dataForLocalTime(qint64 forLocalMSecs) const override;

   QByteArray systemTimeZoneId() const override;

   QList<QByteArray> availableTimeZoneIds() const override;

 private:
   void init(const QByteArray &zoneId);

   QJNIObjectPrivate androidTimeZone;

};
#endif // Q_OS_ANDROID


#endif
