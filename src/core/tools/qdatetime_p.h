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

#ifndef QDATETIME_P_H
#define QDATETIME_P_H

#include <qplatformdefs.h>
#include <qatomic.h>
#include <qdatetime.h>
#include <qstringlist.h>
#include <qlocale.h>
#include <qvariant.h>
#include <qvector.h>

#define QDATETIMEEDIT_TIME_MIN QTime(0, 0, 0, 0)
#define QDATETIMEEDIT_TIME_MAX QTime(23, 59, 59, 999)
#define QDATETIMEEDIT_DATE_MIN QDate(100, 1, 1)
#define QDATETIMEEDIT_COMPAT_DATE_MIN QDate(1752, 9, 14)
#define QDATETIMEEDIT_DATE_MAX QDate(7999, 12, 31)
#define QDATETIMEEDIT_DATETIME_MIN QDateTime(QDATETIMEEDIT_DATE_MIN, QDATETIMEEDIT_TIME_MIN)
#define QDATETIMEEDIT_COMPAT_DATETIME_MIN QDateTime(QDATETIMEEDIT_COMPAT_DATE_MIN, QDATETIMEEDIT_TIME_MIN)
#define QDATETIMEEDIT_DATETIME_MAX QDateTime(QDATETIMEEDIT_DATE_MAX, QDATETIMEEDIT_TIME_MAX)
#define QDATETIMEEDIT_DATE_INITIAL QDate(2000, 1, 1)

QT_BEGIN_NAMESPACE

class QDateTimePrivate
{
 public:
   enum Spec { LocalUnknown = -1, LocalStandard = 0, LocalDST = 1, UTC = 2, OffsetFromUTC = 3};

   QDateTimePrivate()
      : spec(LocalUnknown), m_offsetFromUtc(0)
   { }

   QDateTimePrivate(const QDate &toDate, const QTime &toTime, Qt::TimeSpec toSpec, int offsetSeconds);

   QDateTimePrivate(const QDateTimePrivate &other)
      : date(other.date), time(other.time), spec(other.spec),
        m_offsetFromUtc(other.m_offsetFromUtc)
   { }

   QAtomicInt ref;
   QDate date;
   QTime time;
   Spec spec;
   int m_offsetFromUtc;

   // Get current date/time in LocalTime and put result in outDate and outTime
   Spec getLocal(QDate &outDate, QTime &outTime) const;

   // Get current date/time in UTC and put result in outDate and outTime
   void getUTC(QDate &outDate, QTime &outTime) const;

   // Add msecs to given datetime and return result
   static QDateTime addMSecs(const QDateTime &dt, qint64 msecs);

   // Add msecs to given datetime and put result in utcDate and utcTime
   static void addMSecs(QDate &utcDate, QTime &utcTime, qint64 msecs);
};

class Q_CORE_EXPORT QDateTimeParser
{
 public:
   enum Context {
      FromString,
      DateTimeEdit
   };

   QDateTimeParser(QVariant::Type t, Context ctx)
      : currentSectionIndex(-1), display(0), cachedDay(-1), parserType(t),
        fixday(false), spec(Qt::LocalTime), context(ctx) {
      defaultLocale = QLocale::system();
      first.type = FirstSection;
      first.pos = -1;
      first.count = -1;
      first.zeroesAdded = 0;
      last.type = FirstSection;
      last.pos = -1;
      last.count = -1;
      last.zeroesAdded = 0;
      none.type = NoSection;
      none.pos = -1;
      none.count = -1;
      none.zeroesAdded = 0;
   }

   virtual ~QDateTimeParser() {}

   enum {
      Neither = -1,
      AM = 0,
      PM = 1,
      PossibleAM = 2,
      PossiblePM = 3,
      PossibleBoth = 4
   };

   enum Section {
      NoSection = 0x00000,
      AmPmSection = 0x00001,
      MSecSection = 0x00002,
      SecondSection = 0x00004,
      MinuteSection = 0x00008,
      Hour12Section   = 0x00010,
      Hour24Section   = 0x00020,
      TimeSectionMask = (AmPmSection | MSecSection | SecondSection | MinuteSection | Hour12Section | Hour24Section),
      Internal = 0x10000,
      DaySection = 0x00100,
      MonthSection = 0x00200,
      YearSection = 0x00400,
      YearSection2Digits = 0x00800,
      DayOfWeekSection = 0x01000,
      DateSectionMask = (DaySection | MonthSection | YearSection | YearSection2Digits | DayOfWeekSection),
      FirstSection = 0x02000 | Internal,
      LastSection = 0x04000 | Internal,
      CalendarPopupSection = 0x08000 | Internal,

      NoSectionIndex = -1,
      FirstSectionIndex = -2,
      LastSectionIndex = -3,
      CalendarPopupIndex = -4
   }; // duplicated from qdatetimeedit.h
   using Sections = QFlags<Section>;

   struct SectionNode {
      Section type;
      mutable int pos;
      int count;
      int zeroesAdded;
   };

   enum State { // duplicated from QValidator
      Invalid,
      Intermediate,
      Acceptable
   };

   struct StateNode {
      StateNode() : state(Invalid), conflicts(false) {}
      QString input;
      State state;
      bool conflicts;
      QDateTime value;
   };

   enum AmPm {
      AmText,
      PmText
   };

   enum Case {
      UpperCase,
      LowerCase
   };

#ifndef QT_NO_DATESTRING
   StateNode parse(QString &input, int &cursorPosition, const QDateTime &currentValue, bool fixup) const;
#endif

   int sectionMaxSize(int index) const;
   int sectionSize(int index) const;
   int sectionMaxSize(Section s, int count) const;
   int sectionPos(int index) const;
   int sectionPos(const SectionNode &sn) const;

   const SectionNode &sectionNode(int index) const;
   Section sectionType(int index) const;
   QString sectionText(int sectionIndex) const;
   QString sectionText(const QString &text, int sectionIndex, int index) const;
   int getDigit(const QDateTime &dt, int index) const;
   bool setDigit(QDateTime &t, int index, int newval) const;

   int parseSection(const QDateTime &currentValue, int sectionIndex, QString &txt, int &cursorPosition,
                    int index, QDateTimeParser::State &state, int *used = 0) const;

   int absoluteMax(int index, const QDateTime &value = QDateTime()) const;
   int absoluteMin(int index) const;
   bool parseFormat(const QString &format);

#ifndef QT_NO_DATESTRING
   bool fromString(const QString &text, QDate *date, QTime *time) const;
#endif

#ifndef QT_NO_TEXTDATE
   int findMonth(const QString &str1, int monthstart, int sectionIndex, QString *monthName = 0, int *used = 0) const;

   int findDay(const QString &str1, int intDaystart, int sectionIndex, QString *dayName = 0, int *used = 0) const;
#endif

   int findAmPm(QString &str1, int index, int *used = 0) const;
   int maxChange(int s) const;
   bool potentialValue(const QString &str, int min, int max, int index, const QDateTime &currentValue, int insert) const;
   bool skipToNextSection(int section, const QDateTime &current, const QString &sectionText) const;
   QString sectionName(int s) const;
   QString stateName(int s) const;

   QString sectionFormat(int index) const;
   QString sectionFormat(Section s, int count) const;

   enum FieldInfoFlag {
      Numeric = 0x01,
      FixedWidth = 0x02,
      AllowPartial = 0x04,
      Fraction = 0x08
   };
   using FieldInfo = QFlags<FieldInfoFlag>;

   FieldInfo fieldInfo(int index) const;

   virtual QDateTime getMinimum() const;
   virtual QDateTime getMaximum() const;

   virtual int cursorPosition() const {
      return -1;
   }
   virtual QString displayText() const {
      return text;
   }

   virtual QString getAmPmText(AmPm ap, Case cs) const;
   virtual QLocale locale() const {
      return defaultLocale;
   }

   mutable int currentSectionIndex;
   Sections display;
   mutable int cachedDay;
   mutable QString text;
   QVector<SectionNode> sectionNodes;
   SectionNode first, last, none, popup;
   QStringList separators;
   QString displayFormat;
   QLocale defaultLocale;
   QVariant::Type parserType;

   bool fixday;

   Qt::TimeSpec spec; // spec if used by QDateTimeEdit
   Context context;
};

Q_CORE_EXPORT bool operator==(const QDateTimeParser::SectionNode &s1, const QDateTimeParser::SectionNode &s2);

Q_DECLARE_OPERATORS_FOR_FLAGS(QDateTimeParser::Sections)
Q_DECLARE_OPERATORS_FOR_FLAGS(QDateTimeParser::FieldInfo)

QT_END_NAMESPACE

#endif // QDATETIME_P_H
