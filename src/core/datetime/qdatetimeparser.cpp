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

#include <qdatetimeparser_p.h>

#include <qdatastream.h>
#include <qdatetime.h>
#include <qdebug.h>
#include <qlocale.h>
#include <qplatformdefs.h>
#include <qset.h>

int QDateTimeParser::getDigit(const QDateTime &t, int index) const
{
   if (index < 0 || index >= sectionNodes.size()) {
      qWarning("QDateTimeParser::getDigit() Internal error (%s %d)", csPrintable(t.toString()), index);
      return -1;
   }

   const SectionNode &node = sectionNodes.at(index);

   switch (node.type) {
      case Hour24Section:
      case Hour12Section:
         return t.time().hour();

      case MinuteSection:
         return t.time().minute();

      case SecondSection:
         return t.time().second();

      case MSecSection:
         return t.time().msec();

      case YearSection2Digits:
      case YearSection:
         return t.date().year();

      case MonthSection:
         return t.date().month();

      case DaySection:
         return t.date().day();

      case DayOfWeekSectionShort:
      case DayOfWeekSectionLong:
         return t.date().day();

      case AmPmSection:
         return t.time().hour() > 11 ? 1 : 0;

      default:
         break;
   }

   qWarning("QDateTimeParser::getDigit() Internal error 2 (%s %d)", csPrintable(t.toString()), index);

   return -1;
}

bool QDateTimeParser::setDigit(QDateTime &v, int index, int newVal) const
{
   if (index < 0 || index >= sectionNodes.size()) {
      qWarning("QDateTimeParser::setDigit() Internal error (%s %d %d)", csPrintable(v.toString()), index, newVal);
      return false;
   }

   const SectionNode &node = sectionNodes.at(index);

   int year   = v.date().year();
   int month  = v.date().month();
   int day    = v.date().day();
   int hour   = v.time().hour();
   int minute = v.time().minute();
   int second = v.time().second();
   int msec   = v.time().msec();

   switch (node.type) {
      case Hour24Section:
      case Hour12Section:
         hour = newVal;
         break;

      case MinuteSection:
         minute = newVal;
         break;

      case SecondSection:
         second = newVal;
         break;

      case MSecSection:
         msec = newVal;
         break;

      case YearSection2Digits:
      case YearSection:
         year = newVal;
         break;

      case MonthSection:
         month = newVal;
         break;

      case DaySection:
      case DayOfWeekSectionShort:
      case DayOfWeekSectionLong:
         if (newVal > 31) {
            // have to keep legacy behavior. setting the
            // date to 32 should return false. Setting it
            // to 31 for february should return true
            return false;
         }

         day = newVal;
         break;

      case AmPmSection:
         hour = (newVal == 0 ? hour % 12 : (hour % 12) + 12);
         break;

      default:
         qWarning("QDateTimeParser::setDigit() Internal error (%s)", csPrintable(node.name()));
         break;
   }

   if (! (node.type & DaySectionMask)) {
      if (day < cachedDay) {
         day = cachedDay;
      }

      const int max = QDate(year, month, 1).daysInMonth();

      if (day > max) {
         day = max;
      }
   }

   if (QDate::isValid(year, month, day) && QTime::isValid(hour, minute, second, msec)) {
      v = QDateTime(QDate(year, month, day), QTime(hour, minute, second, msec), m_timeZone);
      return true;
   }

   return false;
}

int QDateTimeParser::absoluteMax(int s, const QDateTime &cur) const
{
   const SectionNode &sn = sectionNode(s);

   switch (sn.type) {
      case Hour24Section:
      case Hour12Section:
         return 23; // special-cased in parseSection, want it to be 23 for the stepBy case.

      case MinuteSection:
      case SecondSection:
         return 59;

      case MSecSection:
         return 999;

      case YearSection2Digits:
      case YearSection:
         return 9999;

      // sectionMaxSize will prevent people from typing in a larger number in count == 2 sections.
      // stepBy() will work on real years anyway

      case MonthSection:
         return 12;

      case DaySection:
      case DayOfWeekSectionShort:
      case DayOfWeekSectionLong:
         return cur.isValid() ? cur.date().daysInMonth() : 31;

      case AmPmSection:
         return 1;

      default:
         break;
   }

   qWarning("QDateTimeParser::absoluteMax() Internal error (%s)", csPrintable(sn.name()));

   return -1;
}

// internal
int QDateTimeParser::absoluteMin(int s) const
{
   const SectionNode &sn = sectionNode(s);

   switch (sn.type) {
      case Hour24Section:
      case Hour12Section:
      case MinuteSection:
      case SecondSection:
      case MSecSection:
      case YearSection2Digits:
      case YearSection:
         return 0;

      case MonthSection:
      case DaySection:
      case DayOfWeekSectionShort:
      case DayOfWeekSectionLong:
         return 1;

      case AmPmSection:
         return 0;

      default:
         break;
   }

   qWarning("QDateTimeParser::absoluteMin() Internal error (%s, %0x)", csPrintable(sn.name()), sn.type);

   return -1;
}

const QDateTimeParser::SectionNode &QDateTimeParser::sectionNode(int sectionIndex) const
{
   if (sectionIndex < 0) {
      switch (sectionIndex) {
         case FirstSectionIndex:
            return first;

         case LastSectionIndex:
            return last;

         case NoSectionIndex:
            return none;
      }

   } else if (sectionIndex < sectionNodes.size()) {
      return sectionNodes.at(sectionIndex);
   }

   qWarning("QDateTimeParser::sectionNode() Internal error (%d)", sectionIndex);

   return none;
}

QDateTimeParser::Section QDateTimeParser::sectionType(int sectionIndex) const
{
   return sectionNode(sectionIndex).type;
}

int QDateTimeParser::sectionPos(int sectionIndex) const
{
   return sectionPos(sectionNode(sectionIndex));
}

int QDateTimeParser::sectionPos(const SectionNode &sn) const
{
   switch (sn.type) {
      case FirstSection:
         return 0;

      case LastSection:
         return displayText().size() - 1;

      default:
         break;
   }

   if (sn.pos == -1) {
      qWarning("QDateTimeParser::sectionPos() Internal error (%s)", csPrintable(sn.name()));
      return -1;
   }

   return sn.pos;
}

static QString unquote(const QString &str)
{
   const QChar quote('\'');
   const QChar slash('\\');
   const QChar zero('0');

   QString retval;
   QChar status(zero);
   const int max = str.size();

   for (int i = 0; i < max; ++i) {
      if (str.at(i) == quote) {
         if (status != quote) {
            status = quote;

         } else if (! retval.isEmpty() && str.at(i - 1) == slash) {
            retval.chop(1);
            retval.append(quote);

         } else {
            status = zero;
         }

      } else {
         retval.append(str.at(i));
      }
   }

   return retval;
}

static inline int countRepeat(const QString &str, int index, int maxCount)
{
   int count = 1;
   const QChar ch(str.at(index));
   const int max = qMin(index + maxCount, str.size());

   while (index + count < max && str.at(index + count) == ch) {
      ++count;
   }

   return count;
}

static inline void appendSeparator(QStringList *list, const QString &string, int from, int size, int lastQuote)
{
   QString str(string.mid(from, size));

   if (lastQuote >= from) {
      str = unquote(str);
   }

   list->append(str);
}

bool QDateTimeParser::parseFormat(const QString &newFormat)
{
   const QChar quote('\'');
   const QChar slash('\\');
   const QChar zero('0');

   if (newFormat == displayFormat && !newFormat.isEmpty()) {
      return true;
   }

   QVector<SectionNode> newSectionNodes;
   Sections newDisplay = Qt::EmptyFlag;

   QStringList newSeparators;
   int i;
   int index = 0;
   int add = 0;

   QChar status(zero);
   const int max = newFormat.size();
   int lastQuote = -1;

   for (i = 0; i < max; ++i) {
      if (newFormat.at(i) == quote) {
         lastQuote = i;
         ++add;

         if (status != quote) {
            status = quote;

         } else if (i > 0 && newFormat.at(i - 1) != slash) {
            status = zero;
         }

      } else if (status != quote) {
         const char sect = newFormat.at(i).toLatin1();

         switch (sect) {
            case 'H':
            case 'h':
               if (parserType != QVariant::Date) {
                  const Section hour = (sect == 'h') ? Hour12Section : Hour24Section;
                  const SectionNode sn = { hour, i - add, countRepeat(newFormat, i, 2), 0 };
                  newSectionNodes.append(sn);
                  appendSeparator(&newSeparators, newFormat, index, i - index, lastQuote);
                  i += sn.count - 1;
                  index = i + 1;
                  newDisplay |= hour;
               }

               break;

            case 'm':
               if (parserType != QVariant::Date) {
                  const SectionNode sn = { MinuteSection, i - add, countRepeat(newFormat, i, 2), 0 };
                  newSectionNodes.append(sn);
                  appendSeparator(&newSeparators, newFormat, index, i - index, lastQuote);
                  i += sn.count - 1;
                  index = i + 1;
                  newDisplay |= MinuteSection;
               }

               break;

            case 's':
               if (parserType != QVariant::Date) {
                  const SectionNode sn = { SecondSection, i - add, countRepeat(newFormat, i, 2), 0 };
                  newSectionNodes.append(sn);
                  appendSeparator(&newSeparators, newFormat, index, i - index, lastQuote);
                  i += sn.count - 1;
                  index = i + 1;
                  newDisplay |= SecondSection;
               }

               break;

            case 'z':
               if (parserType != QVariant::Date) {
                  const SectionNode sn = { MSecSection, i - add, countRepeat(newFormat, i, 3) < 3 ? 1 : 3, 0 };
                  newSectionNodes.append(sn);
                  appendSeparator(&newSeparators, newFormat, index, i - index, lastQuote);
                  i += sn.count - 1;
                  index = i + 1;
                  newDisplay |= MSecSection;
               }

               break;

            case 'A':
            case 'a':
               if (parserType != QVariant::Date) {
                  const bool cap = (sect == 'A');
                  const SectionNode sn = { AmPmSection, i - add, (cap ? 1 : 0), 0 };
                  newSectionNodes.append(sn);
                  appendSeparator(&newSeparators, newFormat, index, i - index, lastQuote);
                  newDisplay |= AmPmSection;

                  if (i + 1 < newFormat.size() && newFormat.at(i + 1) == (cap ? QChar('P') : QChar('p'))) {
                     ++i;
                  }

                  index = i + 1;
               }

               break;

            case 'y':
               if (parserType != QVariant::Time) {
                  const int repeat = countRepeat(newFormat, i, 4);

                  if (repeat >= 2) {
                     const SectionNode sn = { repeat == 4 ? YearSection : YearSection2Digits,
                           i - add, repeat == 4 ? 4 : 2, 0 };

                     newSectionNodes.append(sn);
                     appendSeparator(&newSeparators, newFormat, index, i - index, lastQuote);
                     i += sn.count - 1;
                     index = i + 1;
                     newDisplay |= sn.type;
                  }
               }

               break;

            case 'M':
               if (parserType != QVariant::Time) {
                  const SectionNode sn = { MonthSection, i - add, countRepeat(newFormat, i, 4), 0 };
                  newSectionNodes.append(sn);
                  newSeparators.append(unquote(newFormat.mid(index, i - index)));
                  i += sn.count - 1;
                  index = i + 1;
                  newDisplay |= MonthSection;
               }

               break;

            case 'd':
               if (parserType != QVariant::Time) {
                  const int repeat = countRepeat(newFormat, i, 4);
                  const Section sectionType = (repeat == 4 ? DayOfWeekSectionLong
                        : (repeat == 3 ? DayOfWeekSectionShort : DaySection));

                  const SectionNode sn = { sectionType, i - add, repeat, 0 };
                  newSectionNodes.append(sn);
                  appendSeparator(&newSeparators, newFormat, index, i - index, lastQuote);
                  i += sn.count - 1;
                  index = i + 1;
                  newDisplay |= sn.type;
               }

               break;

            default:
               break;
         }
      }
   }

   if (newSectionNodes.isEmpty() && context == DateTimeEdit) {
      return false;
   }

   if ((newDisplay & (AmPmSection | Hour12Section)) == Hour12Section) {
      const int count = newSectionNodes.size();

      for (int j = 0; j < count; ++j) {
         SectionNode &node = newSectionNodes[j];

         if (node.type == Hour12Section) {
            node.type = Hour24Section;
         }
      }
   }

   if (index < max) {
      appendSeparator(&newSeparators, newFormat, index, index - max, lastQuote);
   } else {
      newSeparators.append(QString());
   }

   displayFormat = newFormat;
   separators    = newSeparators;
   sectionNodes  = newSectionNodes;
   display       = newDisplay;
   last.pos      = -1;

   return true;
}

int QDateTimeParser::sectionSize(int sectionIndex) const
{
   if (sectionIndex < 0) {
      return 0;
   }

   if (sectionIndex >= sectionNodes.size()) {
      qWarning("QDateTimeParser::sectionSize() Internal error (%d)", sectionIndex);
      return -1;
   }

   if (sectionIndex == sectionNodes.size() - 1) {
      // In some cases there is a difference between displayText() and text.
      // e.g. when text is 2000/01/31 and displayText() is "2000/2/31" - text
      // is the previous value and displayText() is the new value.
      // The size difference is always due to leading zeroes.
      int sizeAdjustment = 0;

      if (displayText().size() != m_text.size()) {
         // Any zeroes added before this section will affect our size.
         int preceedingZeroesAdded = 0;

         if (sectionNodes.size() > 1 && context == DateTimeEdit) {
            for (QVector<SectionNode>::const_iterator sectionIt = sectionNodes.constBegin();
                  sectionIt != sectionNodes.constBegin() + sectionIndex; ++sectionIt) {
               preceedingZeroesAdded += sectionIt->zeroesAdded;
            }
         }

         sizeAdjustment = preceedingZeroesAdded;
      }

      return displayText().size() + sizeAdjustment - sectionPos(sectionIndex) - separators.last().size();

   } else {
      return sectionPos(sectionIndex + 1) - sectionPos(sectionIndex) - separators.at(sectionIndex + 1).size();
   }
}

int QDateTimeParser::sectionMaxSize(Section s, int count) const
{
   int mcount = 12;

   switch (s) {
      case FirstSection:
      case NoSection:
      case LastSection:
         return 0;

      case AmPmSection: {
         const int lowerMax = qMin(getAmPmText(AmText, LowerCase).size(),
               getAmPmText(PmText, LowerCase).size());

         const int upperMax = qMin(getAmPmText(AmText, UpperCase).size(),
               getAmPmText(PmText, UpperCase).size());

         return qMin(4, qMin(lowerMax, upperMax));
      }

      case Hour24Section:
      case Hour12Section:
      case MinuteSection:
      case SecondSection:
      case DaySection:
         return 2;

      case DayOfWeekSectionShort:
      case DayOfWeekSectionLong:
         mcount = 7;

         [[fallthrough]];

      case MonthSection:

         if (count <= 2) {
            return 2;
         }

         {
            int retval = 0;
            const QLocale l = locale();
            const QLocale::FormatType format = count == 4 ? QLocale::LongFormat : QLocale::ShortFormat;

            for (int i = 1; i <= mcount; ++i) {
               const QString str = (s == MonthSection ? l.monthName(i, format) : l.dayName(i, format));
               retval = qMax(str.size(), retval);
            }

            return retval;
         }

      case MSecSection:
         return 3;

      case YearSection:
         return 4;

      case YearSection2Digits:
         return 2;

      case CalendarPopupSection:
      case Internal:
      case TimeSectionMask:
      case DateSectionMask:
      case HourSectionMask:
      case YearSectionMask:
      case DayOfWeekSectionMask:
      case DaySectionMask:
         qWarning("QDateTimeParser::sectionMaxSize() Invalid section %s", csPrintable(SectionNode::name(s)));

      case NoSectionIndex:
      case FirstSectionIndex:
      case LastSectionIndex:
      case CalendarPopupIndex:
         // these cases can not happen
         break;
   }

   return -1;
}

int QDateTimeParser::sectionMaxSize(int index) const
{
   const SectionNode &sn = sectionNode(index);
   return sectionMaxSize(sn.type, sn.count);
}

QString QDateTimeParser::sectionText(const QString &text, int sectionIndex, int index) const
{
   const SectionNode &sn = sectionNode(sectionIndex);

   switch (sn.type) {
      case NoSectionIndex:
      case FirstSectionIndex:
      case LastSectionIndex:
         return QString();

      default:
         break;
   }

   return text.mid(index, sectionSize(sectionIndex));
}

QString QDateTimeParser::sectionText(int sectionIndex) const
{
   const SectionNode &sn = sectionNode(sectionIndex);

   return sectionText(displayText(), sectionIndex, sn.pos);
}

int QDateTimeParser::parseSection(const QDateTime &currentValue, int sectionIndex,
      QString &text, int &cursorPosition, int index, State &state, int *usedptr) const
{
   state   = Invalid;
   int num = 0;

   const SectionNode &sn = sectionNode(sectionIndex);

   if ((sn.type & Internal) == Internal) {
      qWarning("QDateTimeParser::parseSection() Internal error (%s %d)", csPrintable(sn.name()), sectionIndex);
      return -1;
   }

   const int sectionmaxsize = sectionMaxSize(sectionIndex);
   QString sectiontext = text.mid(index, sectionmaxsize);
   int sectiontextSize = sectiontext.size();

   int used = 0;

   switch (sn.type) {
      case AmPmSection: {
         const int ampm = findAmPm(sectiontext, sectionIndex, &used);

         switch (ampm) {
            case AM: // sectiontext == AM
            case PM: // sectiontext == PM
               num = ampm;
               state = Acceptable;
               break;

            case PossibleAM: // sectiontext => AM
            case PossiblePM: // sectiontext => PM
               num = ampm - 2;
               state = Intermediate;
               break;

            case PossibleBoth: // sectiontext => AM|PM
               num = 0;
               state = Intermediate;
               break;

            case Neither:
               state = Invalid;

#if defined(CS_SHOW_DEBUG_CORE)
               qDebug() << "QDateTimeParser::parseSection() Invalid findAmPm(" << sectiontext << ") returned -1";
#endif

               break;

            default:
               break;
         }

         if (state != Invalid) {
            text.replace(index, used, sectiontext.left(used));
         }

         break;
      }

      case MonthSection:
      case DayOfWeekSectionShort:
      case DayOfWeekSectionLong:
         if (sn.count >= 3) {
            if (sn.type == MonthSection) {
               int min = 1;
               const QDate minDate = getMinimum().date();

               if (currentValue.date().year() == minDate.year()) {
                  min = minDate.month();
               }

               num = findMonth(sectiontext.toLower(), min, sectionIndex, &sectiontext, &used);

            } else {
               num = findDay(sectiontext.toLower(), 1, sectionIndex, &sectiontext, &used);
            }

            if (num != -1) {
               state = (used == sectiontext.size() ? Acceptable : Intermediate);
               text.replace(index, used, sectiontext.left(used));
            } else {
               state = Intermediate;
            }

            break;
         }

         [[fallthrough]];

      case DaySection:
      case YearSection:
      case YearSection2Digits:
      case Hour12Section:
      case Hour24Section:
      case MinuteSection:
      case SecondSection:
      case MSecSection: {
         if (sectiontextSize == 0) {
            num   = 0;
            used  = 0;
            state = Intermediate;

         } else {
            const int absMax = absoluteMax(sectionIndex);
            QLocale loc;

            bool ok = true;
            int lastNode = -1;

            used = -1;

            QString digitsStr(sectiontext);

            for (int i = 0; i < sectiontextSize; ++i) {
               if (digitsStr.at(i).isSpace()) {
                  sectiontextSize = i;
                  break;
               }
            }

            const int max = qMin(sectionmaxsize, sectiontextSize);

            for (int digits = max; digits >= 1; --digits) {
               digitsStr.truncate(digits);

               int tmp = (int)loc.toUInt(digitsStr, &ok, 10);

               if (ok && sn.type == Hour12Section) {
                  if (tmp > 12) {
                     tmp = -1;
                     ok  = false;

                  } else if (tmp == 12) {
                     tmp = 0;
                  }
               }

               if (ok && tmp <= absMax) {
                  lastNode = tmp;
                  used = digits;
                  break;
               }
            }

            if (lastNode == -1) {
               QChar firstCh(sectiontext.at(0));

               if (separators.at(sectionIndex + 1).startsWith(firstCh)) {
                  used  = 0;
                  state = Intermediate;

               } else {
                  state = Invalid;
               }

            } else {
               num += lastNode;
               const FieldInfo fi = fieldInfo(sectionIndex);
               const bool done = (used == sectionmaxsize);

               if (! done && fi & Fraction) {
                  // typing 2 in a zzz field should be .200, not .002

                  for (int i = used; i < sectionmaxsize; ++i) {
                     num *= 10;
                  }
               }

               const int absMin = absoluteMin(sectionIndex);

               if (num < absMin) {
                  state = done ? Invalid : Intermediate;

               } else if (num > absMax) {
                  state = Intermediate;

               } else if (!done && (fi & (FixedWidth | Numeric)) == (FixedWidth | Numeric)) {
                  if (skipToNextSection(sectionIndex, currentValue, digitsStr)) {
                     state = Acceptable;

                     const int missingZeroes = sectionmaxsize - digitsStr.size();
                     text.insert(index, QString().fill(QChar('0'), missingZeroes));
                     used = sectionmaxsize;

                     cursorPosition += missingZeroes;
                     ++(const_cast<QDateTimeParser *>(this)->sectionNodes[sectionIndex].zeroesAdded);

                  } else {
                     state = Intermediate;
                  }

               } else {
                  state = Acceptable;
               }
            }
         }

         break;
      }

      default:
         qWarning("QDateTimeParser::parseSection() Internal error (%s %d)", csPrintable(sn.name()), sectionIndex);
         return -1;
   }

   if (usedptr) {
      *usedptr = used;
   }

   return (state != Invalid ? num : -1);
}

// internal
QDateTimeParser::StateNode QDateTimeParser::parse(QString &input, int &cursorPosition,
      const QDateTime &currentValue, bool fixup) const
{
   const QDateTime minimum = getMinimum();
   const QDateTime maximum = getMaximum();

   State state = Acceptable;

   QDateTime newCurrentValue;
   bool conflicts = false;
   const int sectionNodesCount = sectionNodes.size();

   {
      int pos = 0;
      int year, month, day;
      currentValue.date().getDate(&year, &month, &day);
      int year2digits = year % 100;
      int hour = currentValue.time().hour();
      int hour12 = -1;
      int minute = currentValue.time().minute();
      int second = currentValue.time().second();
      int msec = currentValue.time().msec();
      int dayofweek = currentValue.date().dayOfWeek();

      int ampm = -1;
      Sections isSet = NoSection;
      int num;
      State tmpstate;

      for (int index = 0; state != Invalid && index < sectionNodesCount; ++index) {

         if (input.midView(pos, separators.at(index).size()) != separators.at(index)) {
            state = Invalid;
            goto end;
         }

         pos += separators.at(index).size();
         sectionNodes[index].pos = pos;

         int *current = nullptr;
         int used;
         const SectionNode sn = sectionNodes.at(index);

         num = parseSection(currentValue, index, input, cursorPosition, pos, tmpstate, &used);

         if (fixup && tmpstate == Intermediate && used < sn.count) {
            const FieldInfo fi = fieldInfo(index);

            if ((fi & (Numeric | FixedWidth)) == (Numeric | FixedWidth)) {
               const QString newText = QString::fromLatin1("%1").formatArg(num, sn.count, 10, QChar('0'));
               input.replace(pos, used, newText);
               used = sn.count;
            }
         }

         pos += qMax(0, used);

         state = qMin<State>(state, tmpstate);

         if (state == Intermediate && context == FromString) {
            state = Invalid;
            break;
         }


         if (state != Invalid) {
            switch (sn.type) {
               case Hour24Section:
                  current = &hour;
                  break;

               case Hour12Section:
                  current = &hour12;
                  break;

               case MinuteSection:
                  current = &minute;
                  break;

               case SecondSection:
                  current = &second;
                  break;

               case MSecSection:
                  current = &msec;
                  break;

               case YearSection:
                  current = &year;
                  break;

               case YearSection2Digits:
                  current = &year2digits;
                  break;

               case MonthSection:
                  current = &month;
                  break;

               case DayOfWeekSectionShort:
               case DayOfWeekSectionLong:
                  current = &dayofweek;
                  break;

               case DaySection:
                  current = &day;
                  num = qMax<int>(1, num);
                  break;

               case AmPmSection:
                  current = &ampm;
                  break;

               default:
                  qWarning("QDateTimeParser::parse() Internal error (%s)", csPrintable(sn.name()));
                  break;
            }

            if (! current) {
               qWarning("QDateTimeParser::parse() Internal error 2");
               return StateNode();
            }

            if (isSet & sn.type && *current != num) {
#if defined(CS_SHOW_DEBUG_CORE)
               qDebug() << "QDateTimeParser::parse() Conflict " << sn.name() << *current << num;
#endif
               conflicts = true;

               if (index != currentSectionIndex || num == -1) {
                  continue;
               }
            }

            if (num != -1) {
               *current = num;
            }

            isSet |= sn.type;
         }
      }

      if (state != Invalid && input.midView(pos) != separators.last()) {

#if defined(CS_SHOW_DEBUG_CORE)
         qDebug() << "QDateTimeParser::parse() Invalid " << input.mid(pos) << "!=" << separators.last() << pos;
#endif

         state = Invalid;
      }

      if (state != Invalid) {
         if (parserType != QVariant::Time) {
            if (year % 100 != year2digits && (isSet & YearSection2Digits)) {
               if (!(isSet & YearSection)) {
                  year = (year / 100) * 100;
                  year += year2digits;

               } else {
                  conflicts = true;
                  const SectionNode &sn = sectionNode(currentSectionIndex);

                  if (sn.type == YearSection2Digits) {
                     year = (year / 100) * 100;
                     year += year2digits;
                  }
               }
            }

            const QDate date(year, month, day);
            const int diff = dayofweek - date.dayOfWeek();

            if (diff != 0 && state == Acceptable && isSet & DayOfWeekSectionMask) {
               if (isSet & DaySection) {
                  conflicts = true;
               }

               const SectionNode &sn = sectionNode(currentSectionIndex);

               if (sn.type & DayOfWeekSectionMask || currentSectionIndex == -1) {
                  // dayofweek should be preferred
                  day += diff;

                  if (day <= 0) {
                     day += 7;
                  } else if (day > date.daysInMonth()) {
                     day -= 7;
                  }
               }
            }

            bool needfixday = false;

            if (sectionType(currentSectionIndex) & DaySectionMask) {
               cachedDay = day;

            } else if (cachedDay > day) {
               day = cachedDay;
               needfixday = true;
            }

            if (! QDate::isValid(year, month, day)) {
               if (day < 32) {
                  cachedDay = day;
               }

               if (day > 28 && QDate::isValid(year, month, 1)) {
                  needfixday = true;
               }
            }

            if (needfixday) {
               if (context == FromString) {
                  state = Invalid;
                  goto end;
               }

               if (state == Acceptable && fixday) {
                  day = qMin<int>(day, QDate(year, month, 1).daysInMonth());

                  const QLocale loc = locale();

                  for (int i = 0; i < sectionNodesCount; ++i) {
                     const SectionNode sn = sectionNode(i);

                     if (sn.type & DaySection) {
                        input.replace(sectionPos(sn), sectionSize(i), loc.toString(day));

                     } else if (sn.type & DayOfWeekSectionMask) {
                        const int dayOfWeek = QDate(year, month, day).dayOfWeek();

                        const QLocale::FormatType dayFormat = (sn.type == DayOfWeekSectionShort
                              ? QLocale::ShortFormat : QLocale::LongFormat);

                        const QString dayName(loc.dayName(dayOfWeek, dayFormat));
                        input.replace(sectionPos(sn), sectionSize(i), dayName);
                     }
                  }

               } else {
                  state = qMin(Intermediate, state);
               }
            }
         }

         if (parserType != QVariant::Date) {
            if (isSet & Hour12Section) {
               const bool hasHour = isSet & Hour24Section;

               if (ampm == -1) {
                  if (hasHour) {
                     ampm = (hour < 12 ? 0 : 1);
                  } else {
                     ampm = 0; // no way to tell if this is am or pm so I assume am
                  }
               }

               hour12 = (ampm == 0 ? hour12 % 12 : (hour12 % 12) + 12);

               if (!hasHour) {
                  hour = hour12;
               } else if (hour != hour12) {
                  conflicts = true;
               }

            } else if (ampm != -1) {
               if (!(isSet & (Hour24Section))) {
                  hour = (12 * ampm); // special case. Only ap section
               } else if ((ampm == 0) != (hour < 12)) {
                  conflicts = true;
               }
            }
         }

         newCurrentValue = QDateTime(QDate(year, month, day), QTime(hour, minute, second, msec), m_timeZone);
      }
   }

end:

   if (newCurrentValue.isValid()) {
      if (context != FromString && state != Invalid && newCurrentValue < minimum) {
         const QChar space(' ');

         if (newCurrentValue >= minimum) {
            qWarning("QDateTimeParser::parse() Internal error 3 (%s %s)",
                  csPrintable(newCurrentValue.toString()), csPrintable(minimum.toString()));
         }

         bool done = false;
         state = Invalid;

         for (int i = 0; i < sectionNodesCount && !done; ++i) {
            const SectionNode &sn = sectionNodes.at(i);
            QString t = sectionText(input, i, sn.pos).toLower();

            if ((t.size() < sectionMaxSize(i) && (((int)fieldInfo(i) &
                  (FixedWidth | Numeric)) != Numeric)) || t.contains(space)) {

               switch (sn.type) {
                  case AmPmSection:
                     switch (findAmPm(t, i)) {
                        case AM:
                        case PM:
                           state = Acceptable;
                           done = true;
                           break;

                        case Neither:
                           state = Invalid;
                           done = true;
                           break;

                        case PossibleAM:
                        case PossiblePM:
                        case PossibleBoth: {
                           const QDateTime copy(newCurrentValue.addSecs(12 * 60 * 60));

                           if (copy >= minimum && copy <= maximum) {
                              state = Intermediate;
                              done = true;
                           }

                           break;
                        }
                     }

                     [[fallthrough]];

                  case MonthSection:
                     if (sn.count >= 3) {
                        int tmp = newCurrentValue.date().month();

                        // the first possible month makes the date too early
                        while ((tmp = findMonth(t, tmp + 1, i)) != -1) {
                           const QDateTime copy(newCurrentValue.addMonths(tmp - newCurrentValue.date().month()));

                           if (copy >= minimum && copy <= maximum) {
                              break;   // break out of while
                           }
                        }

                        if (tmp == -1) {
                           break;
                        }

                        state = Intermediate;
                        done = true;
                        break;
                     }

                     [[fallthrough]];

                  default: {
                     int toMin;
                     int toMax;

                     if (sn.type & TimeSectionMask) {
                        if (newCurrentValue.daysTo(minimum) != 0) {
                           break;
                        }

                        toMin = newCurrentValue.time().msecsTo(minimum.time());

                        if (newCurrentValue.daysTo(maximum) > 0) {
                           toMax = -1; // can't get to max
                        } else {
                           toMax = newCurrentValue.time().msecsTo(maximum.time());
                        }

                     } else {
                        toMin = newCurrentValue.daysTo(minimum);
                        toMax = newCurrentValue.daysTo(maximum);
                     }

                     const int maxChange = sn.maxChange();

                     if (toMin > maxChange) {
                        state = Invalid;
                        done = true;
                        break;

                     } else if (toMax > maxChange) {
                        toMax = -1; // can't get to max
                     }

                     const int min = getDigit(minimum, i);

                     if (min == -1) {
                        qWarning("QDateTimeParser::parse() Internal error 4 (%s)", csPrintable(sn.name()));
                        state = Invalid;
                        done  = true;
                        break;
                     }

                     int max = toMax != -1 ? getDigit(maximum, i) : absoluteMax(i, newCurrentValue);
                     int pos = cursorPosition - sn.pos;

                     if (pos < 0 || pos >= t.size()) {
                        pos = -1;
                     }

                     if (! potentialValue(t.simplified(), min, max, i, newCurrentValue, pos)) {
                        state = Invalid;
                        done = true;
                        break;
                     }

                     state = Intermediate;
                     done = true;
                     break;
                  }
               }
            }
         }

      } else {
         if (context == FromString) {
            // optimization
            Q_ASSERT(getMaximum().date().toJulianDay() == 5373484);

            if (newCurrentValue.date().toJulianDay() > 5373484) {
               state = Invalid;
            }

         } else {
            if (newCurrentValue > getMaximum()) {
               state = Invalid;
            }
         }
      }
   }

   StateNode node;
   node.input     = input;
   node.state     = state;
   node.conflicts = conflicts;
   node.value     = newCurrentValue.toTimeZone(m_timeZone);
   m_text         = input;

   return node;
}

int QDateTimeParser::findMonth(const QString &str1, int startMonth, int sectionIndex,
      QString *usedMonth, int *used) const
{
   int bestMatch = -1;
   int bestCount = 0;

   if (! str1.isEmpty()) {
      const SectionNode &sn = sectionNode(sectionIndex);

      if (sn.type != MonthSection) {
         qWarning("QDateTimeParser::findMonth() Internal error");
         return -1;
      }

      QLocale::FormatType type = sn.count == 3 ? QLocale::ShortFormat : QLocale::LongFormat;
      QLocale l = locale();

      for (int month = startMonth; month <= 12; ++month) {
         QString str2 = l.monthName(month, type).toLower();

         if (str1.startsWith(str2)) {

            if (used) {
               *used = str2.size();
            }

            if (usedMonth) {
               *usedMonth = l.monthName(month, type);
            }

            return month;
         }

         if (context == FromString) {
            continue;
         }

         const int limit = qMin(str1.size(), str2.size());

         bool equal = true;

         for (int i = 0; i < limit; ++i) {
            if (str1.at(i) != str2.at(i)) {
               equal = false;

               if (i > bestCount) {
                  bestCount = i;
                  bestMatch = month;
               }

               break;
            }
         }

         if (equal) {
            if (used) {
               *used = limit;
            }

            if (usedMonth) {
               *usedMonth = l.monthName(month, type);
            }

            return month;
         }
      }

      if (usedMonth && bestMatch != -1) {
         *usedMonth = l.monthName(bestMatch, type);
      }
   }

   if (used) {
      *used = bestCount;
   }

   return bestMatch;
}

int QDateTimeParser::findDay(const QString &str1, int startDay, int sectionIndex, QString *usedDay, int *used) const
{
   int bestMatch = -1;
   int bestCount = 0;

   if (! str1.isEmpty()) {
      const SectionNode &sn = sectionNode(sectionIndex);

      if (! (sn.type & DaySectionMask)) {
         qWarning("QDateTimeParser::findDay() Internal error");
         return -1;
      }

      const QLocale l = locale();

      for (int day = startDay; day <= 7; ++day) {
         const QString dayName = l.dayName(day, sn.count == 4 ? QLocale::LongFormat : QLocale::ShortFormat);
         const QString str2 = dayName.toLower();

         const int limit = qMin(str1.size(), str2.size());
         int i = 0;

         while (i < limit && str1.at(i) == str2.at(i)) {
            ++i;
         }

         if (i > bestCount) {
            bestCount = i;
            bestMatch = day;
         }
      }

      if (usedDay && bestMatch != -1) {
         *usedDay = l.dayName(bestMatch, sn.count == 4 ? QLocale::LongFormat : QLocale::ShortFormat);
      }
   }

   if (used) {
      *used = bestCount;
   }

   return bestMatch;
}

QDateTimeParser::AmPmFinder QDateTimeParser::findAmPm(QString &str, int sectionIndex, int *used) const
{
   static constexpr const int am_index = 0;
   static constexpr const int pm_index = 1;

   const SectionNode &s = sectionNode(sectionIndex);

   if (s.type != AmPmSection) {
      qWarning("QDateTimeParser::findAmPm() Internal error");
      return Neither;
   }

   if (used) {
      *used = str.size();
   }

   if (str.trimmed().isEmpty()) {
      return PossibleBoth;
   }

   const QChar space(' ');
   int size = sectionMaxSize(sectionIndex);

   QString ampm[2];
   ampm[am_index] = getAmPmText(AmText, s.count == 1 ? UpperCase : LowerCase);
   ampm[pm_index] = getAmPmText(PmText, s.count == 1 ? UpperCase : LowerCase);

   for (int i = 0; i < 2; ++i) {
      ampm[i].truncate(size);
   }

   if (str.indexOf(ampm[am_index], 0, Qt::CaseInsensitive) == 0) {
      str = ampm[am_index];
      return AM;

   } else if (str.indexOf(ampm[pm_index], 0, Qt::CaseInsensitive) == 0) {
      str = ampm[pm_index];
      return PM;

   } else if (context == FromString || (str.count(space) == 0 && str.size() >= size)) {
      return Neither;
   }

   size = qMin(size, str.size());

   bool broken[2] = {false, false};

   for (int i = 0; i < size; ++i) {
      if (str.at(i) != space) {
         for (int j = 0; j < 2; ++j) {

            if (! broken[j]) {
               int index = ampm[j].indexOf(str.at(i));

               if (index == -1) {
                  if (str.at(i).category() == QChar::Letter_Uppercase) {
                     index = ampm[j].indexOf(str.at(i).toLower());

                  } else if (str.at(i).category() == QChar::Letter_Lowercase) {
                     index = ampm[j].indexOf(str.at(i).toUpper());

                  }

                  if (index == -1) {
                     broken[j] = true;

                     if (broken[am_index] && broken[pm_index]) {
                        return Neither;
                     }

                     continue;

                  } else {
                     str.replace(i, 1, ampm[j].at(index));
                  }
               }

               ampm[j].remove(index, 1);
            }
         }
      }
   }

   if (! broken[am_index] && ! broken[pm_index]) {
      return PossibleBoth;
   }

   return (! broken[am_index] ? PossibleAM : PossiblePM);
}

int QDateTimeParser::SectionNode::maxChange() const
{
   switch (type) {
      // Time unit is msec
      case MSecSection:
         return 999;

      case SecondSection:
         return 59 * 1000;

      case MinuteSection:
         return 59 * 60 * 1000;

      case Hour24Section:
      case Hour12Section:
         return 59 * 60 * 60 * 1000;

      // Date unit is day
      case DayOfWeekSectionShort:
      case DayOfWeekSectionLong:
         return 7;

      case DaySection:
         return 30;

      case MonthSection:
         return 365 - 31;

      case YearSection:
         return 9999 * 365;

      case YearSection2Digits:
         return 100 * 365;

      default:
         qWarning("QDateTimeParser::maxChange() Internal error (%s)", csPrintable(name()));
   }

   return -1;
}

QDateTimeParser::FieldInfo QDateTimeParser::fieldInfo(int index) const
{
   FieldInfo ret = Qt::EmptyFlag;

   const SectionNode &sn = sectionNode(index);

   switch (sn.type) {
      case MSecSection:
         ret |= Fraction;
         [[fallthrough]];

      case SecondSection:
      case MinuteSection:
      case Hour24Section:
      case Hour12Section:
      case YearSection:
      case YearSection2Digits:
         ret |= Numeric;

         if (sn.type != YearSection) {
            ret |= AllowPartial;
         }

         if (sn.count != 1) {
            ret |= FixedWidth;
         }

         break;

      case MonthSection:
      case DaySection:
         switch (sn.count) {
            case 2:
               ret |= FixedWidth;
               [[fallthrough]];

            case 1:
               ret |= (Numeric | AllowPartial);
               break;
         }

         break;

      case DayOfWeekSectionShort:
      case DayOfWeekSectionLong:
         if (sn.count == 3) {
            ret |= FixedWidth;
         }

         break;

      case AmPmSection:
         ret |= FixedWidth;
         break;

      default:
         qWarning("QDateTimeParser::fieldInfo() Internal error 2 (%d %s %d)",
               index, csPrintable(sn.name()), sn.count);

         break;
   }

   return ret;
}

QString QDateTimeParser::SectionNode::format() const
{
   QChar fillChar;

   switch (type) {
      case AmPmSection:
         return count == 1 ? QString("AP") : QString("ap");

      case MSecSection:
         fillChar = QChar('z');
         break;

      case SecondSection:
         fillChar = QChar('s');
         break;

      case MinuteSection:
         fillChar = QChar('m');
         break;

      case Hour24Section:
         fillChar = QChar('H');
         break;

      case Hour12Section:
         fillChar = QChar('h');
         break;

      case DayOfWeekSectionShort:
      case DayOfWeekSectionLong:
      case DaySection:
         fillChar = QChar('d');
         break;

      case MonthSection:
         fillChar = QChar('M');
         break;

      case YearSection2Digits:
      case YearSection:
         fillChar = QChar('y');
         break;

      default:
         qWarning("QDateTimeParser::sectionFormat() Internal error (%s)", csPrintable(name(type)));
         return QString();
   }

   if (fillChar.isNull()) {
      qWarning("QDateTimeParser::sectionFormat() Internal error 2");
      return QString();
   }

   QString str;
   str.fill(fillChar, count);
   return str;
}

bool QDateTimeParser::potentialValue(const QString &str, int min, int max, int index,
      const QDateTime &currentValue, int insert) const
{
   if (str.isEmpty()) {
      return true;
   }

   const int size = sectionMaxSize(index);
   int val = (int)locale().toUInt(str, nullptr, 10);
   const SectionNode &sn = sectionNode(index);

   if (sn.type == YearSection2Digits) {
      val += currentValue.date().year() - (currentValue.date().year() % 100);
   }

   if (val >= min && val <= max && str.size() == size) {
      return true;

   } else if (val > max) {
      return false;

   } else if (str.size() == size && val < min) {
      return false;
   }

   const int len = size - str.size();

   for (int i = 0; i < len; ++i) {
      for (int j = 0; j < 10; ++j) {
         if (potentialValue(str + QChar('0' + j), min, max, index, currentValue, insert)) {
            return true;

         } else if (insert >= 0) {
            QString tmp = str;
            tmp.insert(insert, QChar('0' + j));

            if (potentialValue(tmp, min, max, index, currentValue, insert)) {
               return true;
            }
         }
      }
   }

   return false;
}

bool QDateTimeParser::skipToNextSection(int index, const QDateTime &current, const QString &text) const
{
   Q_ASSERT(current >= getMinimum() && current <= getMaximum());

   const SectionNode &node = sectionNode(index);
   Q_ASSERT(text.size() < sectionMaxSize(index));

   const QDateTime maximum = getMaximum();
   const QDateTime minimum = getMinimum();

   QDateTime tmp = current;
   int min = absoluteMin(index);
   setDigit(tmp, index, min);

   if (tmp < minimum) {
      min = getDigit(minimum, index);
   }

   int max = absoluteMax(index, current);
   setDigit(tmp, index, max);

   if (tmp > maximum) {
      max = getDigit(maximum, index);
   }

   int pos = cursorPosition() - node.pos;

   if (pos < 0 || pos >= text.size()) {
      pos = -1;
   }

   const bool potential = potentialValue(text, min, max, index, current, pos);
   return !potential;

   // If the value potentially can become another valid entry we do not want to skip
   // to the next. For example, in a M field (month without leading 0 if you type 1
   // we do not want to autoskip, but if you type 3 we do skip

}

QString QDateTimeParser::SectionNode::name(QDateTimeParser::Section s)
{
   switch (s) {
      case QDateTimeParser::AmPmSection:
         return QString("AmPmSection");

      case QDateTimeParser::DaySection:
         return QString("DaySection");

      case QDateTimeParser::DayOfWeekSectionShort:
         return QString("DayOfWeekSectionShort");

      case QDateTimeParser::DayOfWeekSectionLong:
         return QString("DayOfWeekSectionLong");

      case QDateTimeParser::Hour24Section:
         return QString("Hour24Section");

      case QDateTimeParser::Hour12Section:
         return QString("Hour12Section");

      case QDateTimeParser::MSecSection:
         return QString("MSecSection");

      case QDateTimeParser::MinuteSection:
         return QString("MinuteSection");

      case QDateTimeParser::MonthSection:
         return QString("MonthSection");

      case QDateTimeParser::SecondSection:
         return QString("SecondSection");

      case QDateTimeParser::YearSection:
         return QString("YearSection");

      case QDateTimeParser::YearSection2Digits:
         return QString("YearSection2Digits");

      case QDateTimeParser::NoSection:
         return QString("NoSection");

      case QDateTimeParser::FirstSection:
         return QString("FirstSection");

      case QDateTimeParser::LastSection:
         return QString("LastSection");

      default:
         return QString("Unknown section ") + QString::number(int(s));
   }
}

QString QDateTimeParser::stateName(State s) const
{
   switch (s) {
      case Invalid:
         return QString("Invalid");

      case Intermediate:
         return QString("Intermediate");

      case Acceptable:
         return QString("Acceptable");

      default:
         return QString("Unknown state ") + QString::number(s);
   }
}

bool QDateTimeParser::fromString(const QString &t, QDate *date, QTime *time) const
{
   QDateTime dt(QDate(1900, 1, 1), QDATETIME_TIME_MIN);
   QString text = t;

   int copy = -1;
   const StateNode tmp = parse(text, copy, dt, false);

   if (tmp.state != Acceptable || tmp.conflicts) {
      return false;
   }

   if (time) {
      const QTime newTime = tmp.value.time();

      if (! newTime.isValid()) {
         return false;
      }

      *time = newTime;
   }

   if (date) {
      const QDate newDate = tmp.value.date();

      if (! newDate.isValid()) {
         return false;
      }

      *date = newDate;
   }

   return true;
}

QDateTime QDateTimeParser::getMinimum() const
{
   return QDateTime(QDATETIME_DATE_MIN, QDATETIME_TIME_MIN, m_timeZone);
}

QDateTime QDateTimeParser::getMaximum() const
{
   return QDateTime(QDATETIME_DATE_MAX, QDATETIME_TIME_MAX, m_timeZone);
}

QString QDateTimeParser::getAmPmText(AmPm ap, Case cs) const
{
   const QLocale loc = locale();
   QString raw = ap == AmText ? loc.amText() : loc.pmText();

   return cs == UpperCase ? raw.toUpper() : raw.toLower();
}

bool operator==(const QDateTimeParser::SectionNode &s1, const QDateTimeParser::SectionNode &s2)
{
   return (s1.type == s2.type) && (s1.pos == s2.pos) && (s1.count == s2.count);
}
