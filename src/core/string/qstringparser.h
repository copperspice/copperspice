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

#ifndef QSTRING_PARSER_H
#define QSTRING_PARSER_H

#include <qchar32.h>
#include <qglobal.h>
#include <qlist.h>
#include <qlocale.h>
#include <qlog.h>
#include <qmap.h>
#include <qnamespace.h>
#include <qregularexpression.h>

#include <cctype>
#include <ios>
#include <iomanip>
#include <limits>
#include <locale>
#include <sstream>

// returns an integer data type for use with an output stream
template <typename V>
struct CS_SpecialInt
{
   using type = V;
};

template <>
struct CS_SpecialInt<char>
{
   using type = typename std::conditional<std::numeric_limits<char>::is_signed, int, uint>::type;
};

template <>
struct CS_SpecialInt<uchar>
{
   using type = uint;
};

template <>
struct CS_SpecialInt<signed char>
{
   using type = int;
};

class Q_CORE_EXPORT QStringParser
{
   public:
      enum SectionFlag {
         SectionDefault             = 0x00,
         SectionSkipEmpty           = 0x01,
         SectionIncludeLeadingSep   = 0x02,
         SectionIncludeTrailingSep  = 0x04,
         SectionCaseInsensitiveSeps = 0x08
      };
      using SectionFlags = QFlags<SectionFlag>;

      enum SplitBehavior {
         KeepEmptyParts,
         SkipEmptyParts
      };

      // V is data type quint64, long, short, etc
      template <typename T, typename V, typename = typename std::enable_if<std::is_integral<V>::value>::type>
      [[nodiscard]] static T formatArg(const T &str, V value, int fieldwidth = 0, int base = 10, QChar32 fillChar = QChar32(' '))
      {
         ArgEscapeData d = findArgEscapes(str);

         if (d.occurrences == 0) {
            // T must have a toUtf8() method, can not use csPrintable()

            qWarning("QStringParser::formatArg() Missing a place marker \n"
                  "Format string: %s, Argument value: %lld\n", str.toLatin1().constData(), static_cast<long long>(value) );

            return str;
         }

         std::basic_ostringstream<char> stream;
         stream << std::setbase(base);

         T arg;
         T locale_arg;

         if (d.occurrences > d.locale_occurrences) {
            stream << value;

            std::string s1 = stream.str();
            const char *s2 = s1.c_str();

            arg = T::fromUtf8(s2);

         }

         if (d.locale_occurrences > 0) {
            stream << value;

            std::string s1 = stream.str();
            const char *s2 = s1.c_str();

            locale_arg = T::fromUtf8(s2);

            QLocale locale;

            // add thousand marker
            bool thousands_group = !( locale.numberOptions() & QLocale::OmitGroupSeparator);
            T separator = locale.groupSeparator();

            if (thousands_group && base == 10) {
               int strLen = locale_arg.size();

               for (int i = strLen - 3; i > 0; i -= 3) {
                  locale_arg.insert(i, separator);
               }
            }
         }

         return replaceArgEscapes(str, d, fieldwidth, arg, locale_arg, fillChar);
      }

      // V data type is double, float, long double
      template <typename T, typename V, typename = typename std::enable_if<std::is_floating_point<V>::value>::type>
      [[nodiscard]] static T formatArg(const T &str, V value, int fieldwidth = 0, char format = 'g', int precision = 6,
                  QChar32 fillChar = QChar32(' ') )
      {
         ArgEscapeData d = findArgEscapes(str);

         if (d.occurrences == 0) {
            // T must have a toUtf8() method

            qWarning("QStringParser::formatArg() Missing place marker '%%n'\n"
                  "Format string: %s, Argument value: %f\n", str.toLatin1().constData(), value);

            return str;
         }

         std::basic_ostringstream<char> stream;

         switch (format) {
            case 'f':
               stream << std::nouppercase << std::fixed;
               break;

            case 'e':
               stream << std::nouppercase << std::scientific;
               break;

            case 'E':
               stream << std::uppercase << std::scientific;
               break;

            case 'g':
               stream << std::nouppercase;
               stream.unsetf(std::ios_base::floatfield);
               break;

            case 'G':
               stream << std::uppercase;
               stream.unsetf(std::ios_base::floatfield);
               break;

            default:
               if (format <= 32) {
                  qWarning("QStringParser::formatArg() Invalid format '%d'", format);

               } else {
                  qWarning("QStringParser::formatArg() Invalid format '%c'", format);
               }

               break;
         }

         stream << std::setprecision(precision);
         stream.imbue(std::locale::classic());

         T arg;
         T locale_arg;

         if (d.occurrences > d.locale_occurrences) {
            stream << value;

            std::string s1 = stream.str();
            const char *s2 = s1.c_str();

            arg = T::fromUtf8(s2);
         }

         if (d.locale_occurrences > 0) {
            stream << value;

            std::string s1 = stream.str();
            const char *s2 = s1.c_str();

            locale_arg = T::fromUtf8(s2);

            QLocale locale;

            // replace decimal with correct char
            int decimal_pos = locale_arg.indexOf('.');
            T decimal = locale.decimalPoint();

            if (decimal_pos != -1) {
               locale_arg.replace(decimal_pos, 1, decimal);

            } else {
                decimal_pos = locale_arg.size();
            }

            // add thousand marker
            bool thousands_group = !( locale.numberOptions() & QLocale::OmitGroupSeparator);
            T separator = locale.groupSeparator();

            if (thousands_group) {
               for (int i = decimal_pos - 3; i > 0; i -= 3) {
                  locale_arg.insert(i, separator);
               }
            }
         }

         return replaceArgEscapes(str, d, fieldwidth, arg, locale_arg, fillChar);
      }

      // V data type is char, string
      template <typename T, typename V,
                  typename = typename std::enable_if<! std::is_arithmetic<typename std::remove_reference<V>::type>::value>::type>

      [[nodiscard]] static T formatArg(const T &str, V &&value, int fieldwidth = 0, QChar32 fillChar = QChar32(' '))
      {
         const T tmp(std::forward<V>(value));
         ArgEscapeData d = findArgEscapes(str);

         if (d.occurrences == 0) {
            // T must have a toUtf8() method

            qWarning("QStringParser::formatArg() Missing place marker '%%n'\n"
                  "Format string: %s, Argument value: %s\n", str.toLatin1().constData(), tmp.toLatin1().constData());

            return str;
         }

         return replaceArgEscapes(str, d, fieldwidth, tmp, tmp, fillChar);
      }

      // a4
      template <typename T, typename ...Ts>
      [[nodiscard]] static T formatArgs(const T &str, Ts... args)
      {
         const QVector<T> argList = { T("%1").formatArg(args)... };
         return multiArg(str, argList);
      }

      template <typename T, typename V>
      static T join(const QList<T> &list, const V &separator);


      // b1  value - quint64, long, short, etc
      template <typename T = QString8, typename V>
      [[nodiscard]] static T number(V value, int base  = 10)
      {
         if (base < 2 || base > 36) {
            qWarning("QStringParser::number() Invalid numeric base (%d)", base);
            base = 10;
         }

         std::basic_ostringstream<char> stream;
         stream << std::setbase(base);
         stream << typename CS_SpecialInt<V>::type(value);

         std::string s1 = stream.str();
         const char *s2 = s1.c_str();

         T retval = T::fromUtf8(s2);

         return retval;
      }

      // b2  value
      template <typename T = QString8>
      [[nodiscard]] static T number(double value, char format = 'g', int precision = 6)
      {
         std::basic_ostringstream<char> stream;

         switch (format) {
            case 'f':
               stream << std::nouppercase << std::fixed;
               break;

            case 'e':
               stream << std::nouppercase << std::scientific;
               break;

            case 'E':
               stream << std::uppercase << std::scientific;
               break;

            case 'g':
               stream << std::nouppercase;
               stream.unsetf(std::ios_base::floatfield);
               break;

            case 'G':
               stream << std::uppercase;
               stream.unsetf(std::ios_base::floatfield);
               break;

            default:
               qWarning("QStringParser::number() Invalid format '%c'", format);
               break;
         }

         stream << std::setprecision(precision);
         stream.imbue(std::locale::classic());
         stream << value;

         std::string s1 = stream.str();
         const char *s2 = s1.c_str();

         T retval= T::fromUtf8(s2);

         return retval;
      }

      template <typename T>
      static T section(const T &str, QChar32 separator, int firstSection, int lastSection = -1,
               SectionFlags flags = SectionDefault) {
         return section(str, T(separator), firstSection, lastSection, flags);
      }

      template <typename T, int N>
      static T section(const T &str, const char (&separator)[N], int firstSection, int lastSection = -1,
               SectionFlags flags = SectionDefault) {
         return section(str, T(separator), firstSection, lastSection, flags);
      }

      template <typename T>
      static T section(const T &str, const T &separator, int firstSection, int lastSection = -1,
               SectionFlags flags = SectionDefault);

#if defined (CS_DOXYPRESS)
      template <typename T>
      static T section(const T &str, const QRegularExpression &separator, int firstSection, int lastSection = -1,
               SectionFlags flags = SectionDefault);
#else
      template <typename T>
      static T section(const T &str, const Cs::QRegularExpression<T> &separator, int firstSection, int lastSection = -1,
               SectionFlags flags = SectionDefault);
#endif

      template <typename T>
      static QList<T> split(const T &str, QChar32 separator, SplitBehavior behavior = KeepEmptyParts,
               Qt::CaseSensitivity cs = Qt::CaseSensitive);

      template <typename T>
      static QList<T> split(const T &str, const T &separator, SplitBehavior behavior = KeepEmptyParts,
               Qt::CaseSensitivity cs = Qt::CaseSensitive);

#if defined (CS_DOXYPRESS)
      template <typename T>
      static QList<T> split(const T &str, const QRegularExpression &regExp, SplitBehavior behavior = KeepEmptyParts);
#else
      template <typename T>
      static QList<T> split(const T &str, const Cs::QRegularExpression<T> &regExp, SplitBehavior behavior = KeepEmptyParts);

#endif

      //
      template <typename R, typename T = QString8>
      static R toInteger(const T &str, bool *ok = nullptr, int base = 10)
      {
         if (base != 0 && (base < 2 || base > 36)) {
            qWarning("QStringParser::toInteger() Invalid numeric base (%d)", base);
            base = 10;
         }

         R retval;

         std::istringstream stream(str.toLatin1().constData());
         stream >> std::setbase(base);
         stream >> retval;

         if (ok != nullptr) {
            *ok = ! stream.fail();
         }

         if (stream.fail()) {
            return 0;
         }

         return retval;
      }

      template <typename T = QString8>
      static double toDouble(const T &str, bool *ok = nullptr)
      {
         double retval;

         std::istringstream stream(str.toLatin1().constData());
         stream >> retval;

         if (ok != nullptr) {
            *ok = ! stream.fail();
         }

         if (stream.fail()) {
            return 0;
         }

         return retval;
      }

      template <typename T = QString8>
      static float toFloat(const T &str, bool *ok = nullptr)
      {
         float retval;

         std::istringstream stream(str.toLatin1().constData());
         stream >> retval;

         if (ok != nullptr) {
            *ok = ! stream.fail();
         }

         if (stream.fail()) {
            return 0;
         }

         return retval;
      }

   private:
      struct ArgEscapeData {
         int min_escape;            // lowest escape sequence number
         int occurrences;           // number of occurrences of the lowest escape sequence number
         int locale_occurrences;    // number of occurrences of the lowest escape sequence number that contain 'L'
         int escape_len;            // total length of escape sequences which will be replaced
      };

      template <typename T>
      static T multiArg(const T &str, const QVector<T> &argList)
      {
         T retval;

         auto argCount = argList.size();
         QMap<int, int> numbersUsed;

         const auto begin = str.cbegin();
         const auto end   = str.cend();

         // populate the numbersUsed map with the %n's that occur in the string
         for (auto iter = begin; iter != end; ++iter) {

            if (*iter == QChar32('%')) {
               std::pair<int, decltype(iter)> tmp = getEscape(iter, end);

               int id = tmp.first;
               iter   = tmp.second;

               if (id != -1) {
                  numbersUsed.insert(id, -1);
               }

               if (iter == end) {
                  break;
               }
            }
         }

         // assign an argument number to each of the %n's
         int lastNumber = -1;
         int cnt        = 0;

         for (auto iter = numbersUsed.begin(); iter != numbersUsed.end() && cnt < argCount; ++iter) {
            iter.value() = cnt++;
            lastNumber   = iter.key();
         }

         if (argCount > cnt) {
            qWarning("QStringParser::toInteger() Format string has %d arguments and %d place holders\n%s",
                  int(argCount), cnt, csPrintable(str) );

            argCount = cnt;
         }

         for (auto iter = begin; iter != end; ++iter) {

            if (*iter == QChar32('%')) {
               std::pair<int, decltype(iter)> tmp = getEscape(iter, end, lastNumber);
               int id = tmp.first;
               iter   = tmp.second;

               int argIndex = numbersUsed[id];

               if (id != -1 && argIndex != -1) {
                  retval.append(argList[argIndex]);
               }

               if (iter == end) {
                  break;
               }

            } else {
               retval.append(*iter);

            }
         }

         return retval;
      }

      template <typename Iterator>
      static std::pair<int, Iterator> getEscape(Iterator current, Iterator end, int maxNumber = 999)
      {
         Iterator origCurrent = current;
         ++current;

         if (current != end && *current == QChar32('L')) {
            ++current;
         }

         if (current != end) {
            int escape = current->digitValue();

            if (escape == -1) {
               return {-1, origCurrent};
            }

            while (current != end) {
               int digit = current->digitValue();

               if (digit == -1) {
                  break;
               }

               escape = (escape * 10) + digit;
               ++current;
            }

            if (escape <= maxNumber) {
               return {escape, current - 1};
            }
         }

         return {-1, origCurrent};
      }

      template <typename T>
      static ArgEscapeData findArgEscapes(const T &str)
      {
         ArgEscapeData d;

         d.min_escape  = INT_MAX;
         d.occurrences = 0;
         d.escape_len  = 0;
         d.locale_occurrences = 0;

         auto current = str.cbegin();
         auto end     = str.cend();

         while (current != end) {

            while (current != end && current->unicode() != '%') {
               ++current;
            }

            if (current == end) {
               break;
            }

            auto escape_start = current;

            if (++current == end) {
               break;
            }

            bool locale_arg = false;
            if (current->unicode() == 'L') {
               locale_arg = true;

               if (++current == end) {
                  break;
               }
            }

            int escape = current->digitValue();

            if (escape == -1) {
               continue;
            }

            ++current;

            if (current != end) {
               int next_escape = current->digitValue();

               if (next_escape != -1) {
                  escape = (10 * escape) + current->digitValue();
                  ++current;
               }
            }

            if (escape > d.min_escape) {
               continue;
            }

            if (escape < d.min_escape) {
               d.min_escape  = escape;
               d.occurrences = 0;
               d.escape_len  = 0;
               d.locale_occurrences = 0;
            }

            ++d.occurrences;

            if (locale_arg) {
               ++d.locale_occurrences;
            }

            d.escape_len += current - escape_start;
         }

         return d;
      }

      template <typename T, typename V>
      static T replaceArgEscapes(const T &str, const ArgEscapeData &d, int fieldwidth, V &arg, V &larg, QChar32 fillChar)
      {
         T retval;

         auto current  = str.cbegin();
         auto end      = str.cend();

         int abs_field_width = qAbs(fieldwidth);
         int repl_cnt        = 0;

         while (current != end) {
            /* no need to check if we run off the end of the string, as long as d.occurrences > 0 we know
               there are valid escape sequences. */

            auto text_start = current;

            while (current->unicode() != '%') {
               ++current;
            }

            auto escape_start = current;
            ++current;

            bool locale_arg = false;
            if (current->unicode() == 'L') {
               locale_arg = true;
               ++current;
            }

            int escape = current->digitValue();
            if (escape != -1) {
               if (current + 1 != end && (current + 1)->digitValue() != -1) {
                  escape = (10 * escape) + (current + 1)->digitValue();
                  ++current;
               }
            }

            if (escape != d.min_escape) {
               retval.append(text_start, current);

            } else {
               // replace %X with fillChar

               ++current;
               retval.append(text_start, escape_start);

               uint pad_chars;

               if (locale_arg) {
                  pad_chars = qMax(abs_field_width, larg.length()) - larg.length();
               } else {
                  pad_chars = qMax(abs_field_width, arg.length()) - arg.length();
               }

               if (fieldwidth > 0) {
                  // left padded

                  for (uint i = 0; i < pad_chars; ++i) {
                     retval.append(fillChar);
                  }
               }

               if (locale_arg) {
                  retval.append(larg);

               } else {
                  retval.append(arg);

               }

               if (fieldwidth < 0) {
                  // right padded

                  for (uint i = 0; i < pad_chars; ++i) {
                     retval.append(fillChar);
                  }
               }

               if (++repl_cnt == d.occurrences) {
                  retval.append(current, end);
                  break;
               }
            }
         }

         return retval;
      }
};

Q_DECLARE_OPERATORS_FOR_FLAGS(QStringParser::SectionFlags)

template <typename T, typename V>
T QStringParser::join(const QList<T> &list, const V &separator)
{
   T retval;
   bool isFirst = true;

   for (const auto &value : list) {

      if (isFirst) {
         isFirst = false;
      } else {
         retval += separator;
      }

      retval += value;
   }

   return retval;
}

template <typename T>
T QStringParser::section(const T &str, const T &separator, int firstSection, int lastSection, SectionFlags flags)
{
   Qt::CaseSensitivity cs;

   if (flags & SectionCaseInsensitiveSeps) {
      cs = Qt::CaseInsensitive;

   } else {
      cs = Qt::CaseSensitive;

   }

   QList<T> sections = split(str, separator, SplitBehavior::KeepEmptyParts, cs);

   const auto sectionsSize = sections.count();

   if (sectionsSize == 0) {
      return T();
   }

   if (! (flags & SectionFlag::SectionSkipEmpty)) {
      if (firstSection < 0) {
         firstSection += sectionsSize;
      }

      if (lastSection < 0) {
         lastSection += sectionsSize;
      }

   } else {
      int skip = 0;

      for (const auto &item : sections) {
         if (item.isEmpty()) {
            ++skip;
         }
      }

      if (firstSection < 0) {
         firstSection += sectionsSize - skip;
      }

      if (lastSection < 0) {
         lastSection  += sectionsSize - skip;
      }
   }

   if (firstSection >= sectionsSize || lastSection < 0 || firstSection > lastSection) {
      return T();
   }

   T retval;

   int tmp         = 0;
   int first_index = firstSection;
   int last_index  = lastSection;

   for (int k = 0; tmp  <= lastSection && k < sectionsSize; ++k) {
      const auto &item = sections.at(k);
      const bool empty = item.isEmpty();

      if (tmp  >= firstSection) {
         if (tmp  == firstSection) {
            first_index = k;
         }

         if (tmp  == lastSection) {
            last_index = k;
         }

         if (tmp  > firstSection) {
            retval += separator;
         }

         retval += item;
      }

      if (! empty || ! (flags & SectionFlag::SectionSkipEmpty)) {
         ++tmp;
      }
   }

   if ( (flags & SectionFlag::SectionIncludeLeadingSep) && first_index != 0) {
      retval.prepend(separator);
   }

   if ( (flags & SectionFlag::SectionIncludeTrailingSep) && last_index < sectionsSize - 1) {
      retval += separator;
   }

   return retval;
}

template <typename T>
T QStringParser::section(const T &str, const Cs::QRegularExpression<T> &separator,
      int firstSection, int lastSection, SectionFlags flags)
{
   Cs::QRegularExpression<T> regExp = separator;

   if (flags & SectionCaseInsensitiveSeps) {
      regExp.setPatternOptions(regExp.patternOptions() | QPatternOption::CaseInsensitiveOption);
   }

   auto matchList = regExp.globalMatch(str);

   if (flags & SectionFlag::SectionSkipEmpty) {
      auto iter = matchList.begin();

      while (iter != matchList.end()) {
         if (iter->capturedView().isEmpty()) {
            iter = matchList.erase(iter);

         } else {
            ++iter;
         }
      }
   }

   T retval;

   const auto matchSize = matchList.count();

   if (matchSize == 0) {
      return retval;
   }

   // handle negative values
   if (firstSection < 0)  {
      firstSection = matchSize + 1 + firstSection;
   }

   if (lastSection < 0)  {
      lastSection = matchSize + 1 + lastSection;
   }

   auto iter_begin = str.begin();

   if (firstSection > matchSize || firstSection > lastSection) {
      return retval;

   } else if (firstSection > 0) {

      if (flags & SectionFlag::SectionIncludeLeadingSep) {
         iter_begin = matchList[firstSection-1].capturedStart();

      } else {
         iter_begin = matchList[firstSection-1].capturedEnd();
      }
   }

   auto iter_end = str.end();

   if (lastSection < matchSize) {

      if (flags & SectionFlag::SectionIncludeTrailingSep) {
         iter_end = matchList[lastSection].capturedEnd();

      } else {
         iter_end = matchList[lastSection].capturedStart();
      }
   }

   retval = T(iter_begin, iter_end);

   return retval;
}

template <typename T>
QList<T> QStringParser::split(const T &str, QChar32 separator, SplitBehavior behavior, Qt::CaseSensitivity cs)
{
   QList<T> retval;

   auto first_iter = str.cbegin();
   auto last_iter  = str.cend();

   typename T::const_iterator iter;

   while ( (iter = str.indexOfFast(separator, first_iter, cs)) != last_iter) {

      if (first_iter != iter || behavior == SplitBehavior::KeepEmptyParts) {
         retval.append(T(first_iter, iter));
      }

      first_iter = ++iter;
   }

   if (first_iter != last_iter || behavior == SplitBehavior::KeepEmptyParts) {
      retval.append(T(first_iter, last_iter));
   }

   return retval;
}

template <typename T>
QList<T> QStringParser::split(const T &str, const T &separator, SplitBehavior behavior, Qt::CaseSensitivity cs)
{
   QList<T> retval;

   auto first_iter = str.cbegin();
   auto last_iter  = str.cend();

   typename T::const_iterator iter;

   int len   = separator.size();
   int extra = 0;

   while ( (iter = str.indexOfFast(separator, first_iter + extra, cs)) != last_iter) {

      if (first_iter != iter || behavior == SplitBehavior::KeepEmptyParts) {
         retval.append(T(first_iter, iter));
      }

      first_iter = iter + len;

      if (len == 0) {
         extra = 1;
      }
   }

   if (first_iter != last_iter || behavior == SplitBehavior::KeepEmptyParts) {
      retval.append(T(first_iter, last_iter));
   }

   return retval;
}

template <typename T>
QList<T> QStringParser::split(const T &str, const Cs::QRegularExpression<T> &regExp, SplitBehavior behavior)
{
   QList<T> retval;

   if (! regExp.isValid()) {
      qWarning("QStringParser::split() Invalid regular expression");
      return retval;
   }

   typename T::const_iterator start_iter = str.cbegin();
   typename T::const_iterator end_iter;

   Cs::QRegularExpressionMatch<T> match = regExp.match(str);

   while (match.hasMatch())  {
      end_iter = match.capturedStart();

      if (start_iter != end_iter || behavior == QStringParser::KeepEmptyParts) {
         retval.append( T(start_iter, end_iter) );
      }

      start_iter = match.capturedEnd();

      // redo the match
      match = regExp.match(str, start_iter);
   }

   // pick up remaining text
   if (start_iter != str.cend() || behavior == QStringParser::KeepEmptyParts)  {
      retval.append( T(start_iter, str.cend()) );
   }

   return retval;
}


#endif