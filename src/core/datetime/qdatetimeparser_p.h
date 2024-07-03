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

#ifndef QDATETIMEPARSER_P_H
#define QDATETIMEPARSER_P_H

#include <qatomic.h>
#include <qcoreapplication.h>
#include <qdatetime.h>
#include <qlocale.h>
#include <qplatformdefs.h>
#include <qstringlist.h>
#include <qtimezone.h>
#include <qvariant.h>
#include <qvector.h>

#define QDATETIME_TIME_MIN            QTime(0, 0, 0, 0)
#define QDATETIME_TIME_MAX            QTime(23, 59, 59, 999)
#define QDATETIME_DATE_MIN            QDate(100, 1, 1)
#define QDATETIME_DATE_MAX            QDate(9999, 12, 31)

#define QDATETIME_DATETIME_MIN        QDateTime(QDATETIME_DATE_MIN, QDATETIME_TIME_MIN)
#define QDATETIME_DATETIME_MAX        QDateTime(QDATETIME_DATE_MAX, QDATETIME_TIME_MAX)

#define QDATETIME_GREGORIAN_DATE      QDate(1752, 9, 14)
#define QDATETIME_GREGORIAN_DATETIME  QDateTime(QDATETIME_GREGORIAN_DATE, QDATETIME_TIME_MIN)
#define QDATETIME_DATE_DEFAULT        QDate(2000, 1, 1)

class Q_CORE_EXPORT QDateTimeParser
{
   Q_DECLARE_TR_FUNCTIONS(QDateTimeParser)

 public:
   enum Context {
      FromString,
      DateTimeEdit
   };

   QDateTimeParser(QVariant::Type t, Context ctx)
      : currentSectionIndex(-1), display(Qt::EmptyFlag), cachedDay(-1), parserType(t), fixday(false),
        m_timeZone(QTimeZone::systemTimeZone()), context(ctx)
   {
      defaultLocale = QLocale::system();
      first.type        = FirstSection;
      first.pos         = -1;
      first.count       = -1;
      first.zeroesAdded = 0;
      last.type         = LastSection;
      last.pos          = -1;
      last.count        = -1;
      last.zeroesAdded  = 0;
      none.type         = NoSection;
      none.pos          = -1;
      none.count        = -1;
      none.zeroesAdded  = 0;
   }

   virtual ~QDateTimeParser()
   {
   }

   enum Section {
      NoSection       = 0x00000,
      AmPmSection     = 0x00001,
      MSecSection     = 0x00002,
      SecondSection   = 0x00004,
      MinuteSection   = 0x00008,
      Hour12Section   = 0x00010,
      Hour24Section   = 0x00020,
      HourSectionMask = (Hour12Section | Hour24Section),
      TimeSectionMask = (MSecSection | SecondSection | MinuteSection | HourSectionMask | AmPmSection),

      DaySection            = 0x00100,
      MonthSection          = 0x00200,
      YearSection           = 0x00400,
      YearSection2Digits    = 0x00800,
      YearSectionMask       = YearSection | YearSection2Digits,
      DayOfWeekSectionShort = 0x01000,
      DayOfWeekSectionLong  = 0x02000,
      DayOfWeekSectionMask  = DayOfWeekSectionShort | DayOfWeekSectionLong,
      DaySectionMask        = DaySection | DayOfWeekSectionMask,
      DateSectionMask       = DaySectionMask | MonthSection | YearSectionMask,

      Internal              = 0x10000,
      FirstSection          = 0x20000 | Internal,
      LastSection           = 0x40000 | Internal,
      CalendarPopupSection  = 0x80000 | Internal,

      NoSectionIndex        = -1,
      FirstSectionIndex     = -2,
      LastSectionIndex      = -3,
      CalendarPopupIndex    = -4
   };

   using Sections = QFlags<Section>;

   struct Q_CORE_EXPORT SectionNode {
      Section type;
      mutable int pos;
      int count;
      int zeroesAdded;

      static QString name(Section s);

      QString name() const {
         return name(type);
      }

      QString format() const;
      int maxChange() const;
   };

   enum State {
      Invalid,
      Intermediate,
      Acceptable
   };

   struct StateNode {
      StateNode()
         : state(Invalid), conflicts(false)
      { }

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

   StateNode parse(QString &input, int &cursorPosition, const QDateTime &currentValue, bool fixup) const;

   bool parseFormat(const QString &format);
   bool fromString(const QString &text, QDate *date, QTime *time) const;

   enum FieldInfoFlag {
      Numeric      = 0x01,
      FixedWidth   = 0x02,
      AllowPartial = 0x04,
      Fraction     = 0x08
   };
   using FieldInfo = QFlags<FieldInfoFlag>;

   FieldInfo fieldInfo(int index) const;

   void setDefaultLocale(const QLocale &loc) {
      defaultLocale = loc;
   }

   virtual QString displayText() const {
      return m_text;
   }

 protected:
   // for the benefit of QDateTimeEditPrivate
   int sectionSize(int index) const;
   int sectionMaxSize(int index) const;
   int sectionPos(int index) const;
   int sectionPos(const SectionNode &sn) const;

   const SectionNode &sectionNode(int index) const;
   Section sectionType(int index) const;
   QString sectionText(int sectionIndex) const;
   int getDigit(const QDateTime &dt, int index) const;
   bool setDigit(QDateTime &t, int index, int newval) const;

   int absoluteMax(int index, const QDateTime &value = QDateTime()) const;
   int absoluteMin(int index) const;

   bool skipToNextSection(int section, const QDateTime &current, const QString &sectionText) const;
   QString stateName(State s) const;

   virtual QDateTime getMinimum() const;
   virtual QDateTime getMaximum() const;

   virtual int cursorPosition() const {
      return -1;
   }

   virtual QString getAmPmText(AmPm ap, Case cs) const;
   virtual QLocale locale() const {
      return defaultLocale;
   }

   mutable int currentSectionIndex;

   Sections display;
   /*
       stores the most recently selected day, useful when considering the following scenario:

       1. Date is: 31/01/2000
       2. User increments month: 29/02/2000
       3. User increments month: 31/03/2000

       At step 1, cachedDay stores 31. At step 2, the 31 is invalid for February, so the cachedDay is not updated.
       At step 3, the month is changed to March, for which 31 is a valid day. Since 29 < 31, the day is set to cachedDay.
       This is good for when users have selected their desired day and are scrolling up or down in the month or year section
       and do not want smaller months (or non-leap years) to alter the day that they chose.
   */

   mutable int cachedDay;
   mutable QString m_text;

   QVector<SectionNode> sectionNodes;

   SectionNode first;
   SectionNode last;
   SectionNode none;
   SectionNode popup;

   QStringList separators;
   QString displayFormat;
   QLocale defaultLocale;
   QVariant::Type parserType;
   bool fixday;
   QTimeZone m_timeZone;
   Context context;

 private:
   enum AmPmFinder {
      Neither      = -1,
      AM           = 0,
      PM           = 1,
      PossibleAM   = 2,
      PossiblePM   = 3,
      PossibleBoth = 4
   };

   int sectionMaxSize(Section s, int count) const;
   QString sectionText(const QString &text, int sectionIndex, int index) const;
   int parseSection(const QDateTime &currentValue, int sectionIndex, QString &txt, int &cursorPosition,
         int index, QDateTimeParser::State &state, int *used = nullptr) const;

   int findMonth(const QString &str1, int monthstart, int sectionIndex,
         QString *monthName = nullptr, int *used = nullptr) const;

   int findDay(const QString &str1, int intDaystart, int sectionIndex,
         QString *dayName = nullptr, int *used = nullptr) const;

   AmPmFinder findAmPm(QString &str, int index, int *used = nullptr) const;
   bool potentialValue(const QString &str, int min, int max, int index,
         const QDateTime &currentValue, int insert) const;
};

Q_CORE_EXPORT bool operator==(const QDateTimeParser::SectionNode &s1, const QDateTimeParser::SectionNode &s2);

Q_DECLARE_OPERATORS_FOR_FLAGS(QDateTimeParser::Sections)
Q_DECLARE_OPERATORS_FOR_FLAGS(QDateTimeParser::FieldInfo)

#endif
