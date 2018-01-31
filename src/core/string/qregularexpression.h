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

#ifndef QREGULAR_EXPRESSION_H
#define QREGULAR_EXPRESSION_H

#include "regex/regex.h"

#include <locale>

#include <qglobal.h>
#include <qchar32.h>
#include <qlist.h>

template <typename S>
class QStringView;

template <typename S>
class QRegularExpression;

template <typename S>
class QRegularExpressionMatch;

using QRegularExpression8  = QRegularExpression<QString8>;
using QRegularExpression16 = QRegularExpression<QString16>;

using QRegularExpressionMatch8  = QRegularExpressionMatch<QString8>;
using QRegularExpressionMatch16 = QRegularExpressionMatch<QString16>;

template <typename S>
uint qHash(const QRegularExpression<S> &key, uint seed = 0);

enum class QPatternOption {
   NoPatternOption                 = 0x0000,
   CaseInsensitiveOption           = 0x0001,
   DotMatchesEverythingOption      = 0x0002,
   MultilineOption                 = 0x0004,
   ExtendedPatternSyntaxOption     = 0x0008,
   InvertedGreedinessOption        = 0x0010,
   DontCaptureOption               = 0x0020,
   UseUnicodePropertiesOption      = 0x0040
};
Q_DECLARE_FLAGS(QPatternOptionFlags, QPatternOption)

enum class QMatchType {
   NormalMatch = 0,
   PartialPreferCompleteMatch,
   PartialPreferFirstMatch,
   NoMatch
};

enum class QMatchOption {
   NoMatchOption                     = 0x0000,
   AnchoredMatchOption               = 0x0001,
   DontCheckSubjectStringMatchOption = 0x0002
};
Q_DECLARE_FLAGS(QMatchOptionFlags, QMatchOption)

Q_DECLARE_OPERATORS_FOR_FLAGS(QPatternOptionFlags)
Q_DECLARE_OPERATORS_FOR_FLAGS(QMatchOptionFlags)


namespace cs_regex_ns {
namespace cs_regex_detail_ns {

template <>
inline bool is_combining<QChar32>(QChar32 c)
{
   if (c.combiningClass() == 0) {
      return false;
   } else {
      return true;
   }
}

}   // namespace
}   // namespace


// new type traits, internal only
template<typename S>
class QRegexTraits
{
   public:
      enum Char_Category {
         categoryNone     = 0x0000,
         categoryAlpha    = 0x0001,
         categoryWord     = 0x0002,
         categoryLower    = 0x0004,
         categoryUpper    = 0x0008,
         categoryDigit    = 0x0010,
         categoryHexDigit = 0x0020,
         categoryBlank    = 0x0040,
         categorySpace    = 0x0080,
         categoryPrint    = 0x0100,
         categoryControl  = 0x0200,
         categoryPunct    = 0x0400,
         categoryAlnum    = categoryAlpha | categoryDigit,
         categoryGraph    = categoryAlnum | categoryPunct
      };

      using char_type       = QChar32;
      using char_class_type = Char_Category;
      using string_type     = S;
      using locale_type     = std::locale;

      QRegexTraits() = default;

      static size_t length(const QChar32 *p) {
         size_t retval = 0;

         while (p->unicode() != 0) {
            ++p;
            ++retval;
         }

         return retval;
      }

      char32_t toInt(QChar32 c) const {
         return c.unicode();
      }

      QChar32 translate(QChar32 c) const {
         return c;
      }

      QChar32 translate_nocase(QChar32 c) const {
         // broom - add soon, modify for case insensitive
         return c;
      }

      template<typename Iter>
      string_type transform(Iter first, Iter last)  const {
         return S(first, last);
      }

      template<typename Iter>
      string_type transform_primary(Iter first, Iter last)  const {
         return S(first, last).toCaseFolded();
      }

      template<typename Iter>
      string_type lookup_collatename(Iter first, Iter last)  const {
         // broom - add soon, modify for case insensitive
         return S();
      }

      template<typename Iter>
      char_class_type lookup_classname(Iter first, Iter last)  const {
         S cName{first, last};

         if (cName == "alpha")  {
            return Char_Category::categoryAlpha;
         }

         if (cName == "w" || cName == "word")  {
            return Char_Category::categoryWord;
         }

         if (cName == "l" || cName == "lower")  {
            return Char_Category::categoryLower;
         }

         if (cName == "u" || cName == "upper")  {
            return Char_Category::categoryUpper;
         }

         if (cName == "d" || cName == "digit")  {
            return Char_Category::categoryDigit;
         }

         if (cName == "xdigit")  {
            return Char_Category::categoryHexDigit;
         }

         if (cName == "h" || cName == "blank")  {
            return Char_Category::categoryBlank;
         }

         if (cName == "s" || cName == "space")  {
            return Char_Category::categorySpace;
         }

         if (cName == "print")  {
            return Char_Category::categoryPrint;
         }

         if (cName == "cntrl")  {
            return Char_Category::categoryControl;
         }

         if (cName == "punct")  {
            return Char_Category::categoryPunct;
         }

         if (cName == "alnum")  {
            return Char_Category::categoryAlnum;
         }

         if (cName == "graph")  {
            return Char_Category::categoryGraph;
         }

         return Char_Category::categoryNone;
      }

      bool isctype(QChar32 c, char_class_type cType) const {
         // does the character c belong to the class_type t

         bool retval = false;

         if (cType & Char_Category::categoryAlpha) {
            retval = retval || c.isLetter();
         }

         if (cType & Char_Category::categoryWord) {
            retval = retval || (c.isLetterOrNumber() || c.isMark());
         }

         if (cType & Char_Category::categoryLower) {
            retval = retval || c.isLower();
         }

         if (cType & Char_Category::categoryUpper) {
           retval = retval || c.isUpper();
         }

         if (cType & Char_Category::categoryDigit) {
            retval = retval && c.isDigit();
         }

         if (cType & Char_Category::categoryHexDigit) {
            // broom - add soon
//          retval = retval || c.isX();
         }

         if (cType & Char_Category::categoryBlank) {
            retval = retval || c.category() == QChar32::Separator_Space;
         }

         if (cType & Char_Category::categorySpace) {
            retval = retval || c.isSpace();
         }

         if (cType & Char_Category::categoryPrint) {
            retval = retval || c.isPrint();
         }

         if (cType & Char_Category::categoryControl) {
            retval = retval || c.category() == QChar32::Other_Control;
         }

         if (cType & Char_Category::categoryPunct) {
            retval = retval || c.isPunct();
         }

         return retval;
      }

      int value(QChar32 c, int base) const {

         if (base == 8) {
            int tmp = c.digitValue();

            if (tmp > 7) {
               return -1;
            } else {
               return tmp;
            }

         } else if (base == 10) {
            return c.digitValue();

         } else if (base == 16) {
            int tmp = c.digitValue();

            if (tmp == -1) {

               if (c == 'A' || c == 'a') {
                  return 10;

               } else if (c == 'B' || c == 'b') {
                  return 11;

               } else if (c == 'C' || c == 'c') {
                  return 12;

               } else if (c == 'D' || c == 'd') {
                  return 13;

               } else if (c == 'E' || c == 'e') {
                  return 14;

               } else if (c == 'F' || c == 'F') {
                  return 15;

               }

            } else {
               return tmp;
            }

         } else {
            return -1;

         }
      }

      locale_type imbue(locale_type locale) {
         return locale;
      }

      locale_type getloc() const {
         return std::locale();
      }
};

template <typename S>
class Q_CORE_EXPORT QRegularExpression
{
   public:
      QRegularExpression() = default;
      explicit QRegularExpression(const S &pattern, QPatternOptionFlags options = QPatternOption::NoPatternOption);

      QRegularExpression(const QRegularExpression &other) = default;
      QRegularExpression(QRegularExpression &&other) = default;

      // methods
      int captureCount() const;

      S errorString() const {
         return m_errorString;
      }

      static S escape(const S &str);

      QList<QRegularExpressionMatch<S>> globalMatch(const S &str) const {

         if (m_valid) {
            return globalMatch(str, str.begin());
         } else {
            return QList<QRegularExpressionMatch<S>>();
         }
      }

      QList<QRegularExpressionMatch<S>> globalMatch(const S &str, typename S::const_iterator offset,
                  QMatchType matchType = QMatchType::NormalMatch, QMatchOptionFlags matchOptions = QMatchOption::NoMatchOption) const;

      QList<QRegularExpressionMatch<S>> globalMatch(const QStringView<S> &str) const {
         return globalMatch(str, str.begin());
      }

      QList<QRegularExpressionMatch<S>> globalMatch(const QStringView<S> &str, typename S::const_iterator offset,
                  QMatchType matchType = QMatchType::NormalMatch, QMatchOptionFlags matchOptions = QMatchOption::NoMatchOption) const;

      bool isValid() const {
         return m_valid;
      }

      QRegularExpressionMatch<S> match(const S &str) const {
         return match(str, str.begin());
      }

      QRegularExpressionMatch<S> match(const S &str, typename S::const_iterator offset, QMatchType matchType = QMatchType::NormalMatch,
                  QMatchOptionFlags matchOptions = QMatchOption::NoMatchOption) const;

      QRegularExpressionMatch<S> match(const QStringView<S> &str) const {
         return match(str, str.begin());
      }

      QRegularExpressionMatch<S> match(const QStringView<S> &str, typename S::const_iterator offset,
                  QMatchType matchType = QMatchType::NormalMatch, QMatchOptionFlags matchOptions = QMatchOption::NoMatchOption) const;


      QRegularExpressionMatch<S> rmatch(const S &str) const {
         return rmatch(str, str.end());
      }

      QRegularExpressionMatch<S> rmatch(const S &str, typename S::const_iterator offset, QMatchType matchType = QMatchType::NormalMatch,
                  QMatchOptionFlags matchOptions = QMatchOption::NoMatchOption) const;

      QRegularExpressionMatch<S> rmatch(const QStringView<S> &str) const {
         return rmatch(str, str.end());
      }

      QRegularExpressionMatch<S> rmatch(const QStringView<S> &str, typename S::const_iterator offset,
                  QMatchType matchType = QMatchType::NormalMatch,
                  QMatchOptionFlags matchOptions = QMatchOption::NoMatchOption) const;

      QList<S> namedCaptureGroups() const;

      int patternErrorOffset() const;

      S pattern() const {
         return m_pattern;
      }

      QPatternOptionFlags patternOptions() const {
         return m_options;
      }

      void setPattern(const S &pattern);

      void setPatternOptions(QPatternOptionFlags options)  {
         m_options = options;
         setPattern(m_pattern);
      }

      void swap(QRegularExpression &other)  {
         swap(m_regex, other.m_regex);
      }

      // operators
      QRegularExpression &operator=(const QRegularExpression &other) = default;
      QRegularExpression &operator=(QRegularExpression &&other) = default;


/*    broom - hold for now
      bool operator==(const QRegularExpression &regExp) const {
      }

      bool operator!=(const QRegularExpression &regExp) const {
         return !operator==(regExp);
      }
*/

   private:
      S m_pattern;
      QPatternOptionFlags m_options;
      cs_regex_ns::basic_regex<QChar32, QRegexTraits<S>> m_regex;

      bool m_valid = false;
      S m_errorString;

      template <typename S2>
      friend Q_CORE_EXPORT uint qHash(const QRegularExpression<S2> &key, uint seed);
};

#ifndef QT_NO_DATASTREAM
   template <typename S>
   Q_CORE_EXPORT QDataStream &operator<<(QDataStream &out, const QRegularExpression<S> &regExp);

   template <typename S>
   Q_CORE_EXPORT QDataStream &operator>>(QDataStream &in, QRegularExpression<S> &regExp);
#endif

template <typename S>
Q_CORE_EXPORT QDebug operator<<(QDebug debug, const QRegularExpression<S> &regExp);

template <typename S>
Q_CORE_EXPORT QDebug operator<<(QDebug debug, QPatternOptionFlags patternOptions);


template <typename S>
class Q_CORE_EXPORT QRegularExpressionMatch
{
   public:
      QRegularExpressionMatch() = default;
      QRegularExpressionMatch(const QRegularExpressionMatch &other) = default;
      QRegularExpressionMatch(QRegularExpressionMatch &&other) = default;

      // internal only
      QRegularExpressionMatch(cs_regex_ns::match_results<typename S::const_iterator> match)
         : m_results(std::move(match)), m_valid(true)
      {
      }

      // operators
      QRegularExpressionMatch &operator=(const QRegularExpressionMatch &other) = default;
      QRegularExpressionMatch &operator=(QRegularExpressionMatch &&other) = default;

      int captureIndexForName(const S &name) const;
      int captureIndexForName(QStringView<S> name) const;

      S captured(int index = 0) const {

         if (index < m_results.size()) {
            return S(m_results[index].first, m_results[index].second);

         } else {
            return S();
         }
      }

      S captured(const S &name) const;
      S captured(QStringView<S> name) const;

      int capturedLength(int index = 0) const;
      int capturedLength(const S &name) const;
      int capturedLength(QStringView<S> name) const;

      typename S::const_iterator capturedEnd(int index = 0) const;
      typename S::const_iterator capturedEnd(const S &name) const;
      typename S::const_iterator capturedEnd(QStringView<S> name) const;

      typename S::const_iterator capturedStart(int index = 0) const;
      typename S::const_iterator capturedStart(const S &name) const;
      typename S::const_iterator capturedStart(QStringView<S> name) const;

      QList<S> capturedTexts() const;

      QStringView<S> capturedView(int index = 0) const {

         if (index < m_results.size()) {
            return QStringView<S>(m_results[index].first, m_results[index].second);

         } else {
            return QStringView<S>();
         }
      }

      QStringView<S> capturedView(const S &name) const;
      QStringView<S> capturedView(QStringView<S> name) const;

      bool hasMatch() const {
         return ! m_results.empty();
      }

      bool hasPartialMatch() const {
         if (! m_results[0].matched) {
            return true;
         }

         return false;
      }

      bool isValid() const  {
         return m_valid;
      }

      int lastCapturedIndex() const {
         return m_results.size();
      }

      QMatchOptionFlags matchOptions() const;
      QMatchType matchType() const;

      void swap(QRegularExpressionMatch &other) {
         swap(m_results, other.m_results);
      }

   private:
      cs_regex_ns::match_results<typename S::const_iterator> m_results;
      bool m_valid = false;
};

template <typename S>
Q_CORE_EXPORT QDebug operator<<(QDebug debug, const QRegularExpressionMatch<S> &match);


// implementations

template <typename S>
QRegularExpression<S>::QRegularExpression(const S &pattern, QPatternOptionFlags options)
{
   m_options = options;
   setPattern(pattern);
}

template <typename S>
int QRegularExpression<S>::captureCount() const
{
   if (m_valid) {
      return m_regex.mark_count();
   } else {
      return -1;
   }
}

template <typename S>
S QRegularExpression<S>::escape(const S &str)
{
   S result;

   // everything but [a-zA-Z0-9_] is escaped
   for (auto ch : str) {

      if (ch == '\0') {
         // literal NUL must be escaped with "\\0" and not "\\\0"

         result.append(QLatin1Char('\\'));
         result.append(QLatin1Char('0'));

      } else if ( (ch < 'a' || ch > 'z') && (ch < 'A' || ch > 'Z') &&
                  (ch < '0' || ch > '9') && (ch != '_') )  {

         result.append('\\');
         result.append(ch);

      } else {
         result.append(ch);
      }
   }

   return result;
}

template <typename S>
QList<QRegularExpressionMatch<S>> QRegularExpression<S>::globalMatch(const S &str, typename S::const_iterator offset,
                  QMatchType matchType, QMatchOptionFlags matchOptions) const
{
   if (m_valid) {
      QList<QRegularExpressionMatch<S>> retval;

      typename S::const_iterator iter = offset;

      while (iter != str.end()) {
         QRegularExpressionMatch<S> tmp = match(str, iter, matchType, matchOptions);

         if (! tmp.hasMatch()) {
            break;
         }

         // move on
         iter = tmp.capturedEnd(0);

         retval.append(std::move(tmp));
      }

      return retval;

   } else {
      return QList<QRegularExpressionMatch<S>>();
   }
}

template <typename S>
QList<QRegularExpressionMatch<S>> QRegularExpression<S>::globalMatch(const QStringView<S> &str, typename S::const_iterator offset,
                  QMatchType matchType, QMatchOptionFlags matchOptions) const
{
  if (m_valid) {
      QList<QRegularExpressionMatch<S>> retval;

      typename S::const_iterator iter = offset;

      while (iter != str.end()) {
         QRegularExpressionMatch<S> tmp = match(str, iter, matchType, matchOptions);

         if (! tmp.hasMatch()) {
            break;
         }

         // move on
         iter = tmp.capturedEnd(0);

         retval.append(std::move(tmp));
      }

      return retval;

   } else {
      return QList<QRegularExpressionMatch<S>>();
   }
}

template <typename S>
QRegularExpressionMatch<S> QRegularExpression<S>::match(const S &str, typename S::const_iterator offset, QMatchType matchType,
                  QMatchOptionFlags matchOptions) const
{
   // broom - need to handle matchType, matchOptions

   if (m_valid) {
      cs_regex_ns::match_results<typename S::const_iterator> matchResult;
      cs_regex_ns::regex_search(offset, str.cend(), matchResult, m_regex);

      QRegularExpressionMatch<S> retval{matchResult};
      return retval;

   } else {
      return QRegularExpressionMatch<S>();

   }
}

template <typename S>
QRegularExpressionMatch<S> QRegularExpression<S>::match(const QStringView<S> &str, typename S::const_iterator offset,
                  QMatchType matchType, QMatchOptionFlags matchOptions) const
{
   // broom - need to handle matchType, matchOptions

   if (m_valid) {
      cs_regex_ns::match_results<typename S::const_iterator> matchResult;
      cs_regex_ns::regex_search(offset, str.cend(), matchResult, m_regex);

      QRegularExpressionMatch<S> retval{matchResult};
      return retval;

   } else {
      return QRegularExpressionMatch<S>();

   }
}

template <typename S>
QRegularExpressionMatch<S> QRegularExpression<S>::rmatch(const S &str, typename S::const_iterator offset, QMatchType matchType,
                  QMatchOptionFlags matchOptions) const
{
   if (m_valid) {
      // broom -implement

   } else {
      return QRegularExpressionMatch<S>();
   }
}

template <typename S>
QRegularExpressionMatch<S> QRegularExpression<S>::rmatch(const QStringView<S> &str, typename S::const_iterator offset,
                  QMatchType matchType, QMatchOptionFlags matchOptions) const
{
   if (m_valid) {
      // broom -implement

   } else {
      return QRegularExpressionMatch<S>();
   }
}

template <typename S>
QList<S> QRegularExpression<S>::namedCaptureGroups() const
{
   if (m_valid) {
      QList<S> retval;

      // broom -implement

      return retval;

   } else {
      return QList<S>();
   }
}

template <typename S>
void QRegularExpression<S>::setPattern(const S &pattern)
{
   m_pattern = pattern;

   // declare flags
   typename cs_regex_ns::basic_regex<QChar32, QRegexTraits<S>>::flag_type flags;

   if (m_options & QPatternOption::CaseInsensitiveOption) {
// BROOM      flags |= std::regex_constants::icase;
   }

   if (m_options & QPatternOption::DotMatchesEverythingOption) {
// BROOM      flags |= std::regex_constants::icase;
   }

   if (m_options & QPatternOption::MultilineOption) {
// BROOM    flags |= std::regex_constants::multiline;
   }

   if (m_options & QPatternOption::ExtendedPatternSyntaxOption) {
// BROOM       flags |= std::regex_constants::extended;
   }

   if (m_options & QPatternOption::InvertedGreedinessOption) {
// BROOM       flags |= std::regex_constants::icase;
   }

   if (m_options & QPatternOption::DontCaptureOption ) {
      flags |= cs_regex_ns::regex_constants::nosubs;
   }

   if (m_options & QPatternOption::UseUnicodePropertiesOption) {
// BROOM     flags |= std::regex_constants::icase;
   }

   try  {
      m_regex.assign(m_pattern.begin(), m_pattern.end(), flags);
      m_valid = true;

   } catch (std::exception &err) {
      m_errorString = err.what();
      m_valid = false;

   } catch (...) {
      m_valid = false;

   }
}


// ** Match Implementations

template <typename S>
int QRegularExpressionMatch<S>::captureIndexForName(const S &name) const
{
   if (name.isEmpty()) {
      return -1;
   }

   int retval =  m_results.named_subexpression_index(name.begin(), name.end());

   if (retval > 0) {
      return retval;
   }

   return -1;
}

template <typename S>
int QRegularExpressionMatch<S>::captureIndexForName(QStringView<S> name) const
{
   if (name.isEmpty()) {
      return -1;
   }

   int retval =  m_results.named_subexpression_index(name.begin(), name.end());

   if (retval > 0) {
      return retval;
   }

   return -1;
}

template <typename S>
S QRegularExpressionMatch<S>::captured(const S &name) const
{
   if (name.isEmpty()) {
      return S();
   }

   int index = captureIndexForName(name);

   if (index == -1) {
      return S();
   }

   return captured(index);
}

template <typename S>
S QRegularExpressionMatch<S>::captured(QStringView<S> name) const
{
   if (name.isEmpty()) {
      return S();
   }

   int index = captureIndexForName(name);

   if (index == -1) {
      return S();
   }

   return captured(index);
}

template <typename S>
int QRegularExpressionMatch<S>::capturedLength(int index) const
{
   return capturedEnd(index) - capturedStart(index);
}

template <typename S>
int QRegularExpressionMatch<S>::capturedLength(const S &name) const
{
   if (name.isEmpty()) {
      return 0;
   }

   int index = captureIndexForName(name);

   if (index == -1) {
      return 0;
   }

   return capturedLength(index);
}

template <typename S>
int QRegularExpressionMatch<S>::capturedLength(QStringView<S> name) const
{
   if (name.isEmpty()) {
      return 0;
   }

   int index = captureIndexForName(name);

   if (index == -1) {
      return 0;
   }

   return capturedLength(index);
}

template <typename S>
typename S::const_iterator QRegularExpressionMatch<S>::capturedEnd(int index) const
{
   if (index < m_results.size()) {
      return m_results[index].second;

   } else {
      return typename S::const_iterator();
   }
}

template <typename S>
typename S::const_iterator QRegularExpressionMatch<S>::capturedEnd(const S &name) const
{
   if (name.isEmpty()) {
      return S();
   }

   int index = captureIndexForName(name);

   if (index == -1) {
      return S();
   }

   return capturedEnd(index);
}


template <typename S>
typename S::const_iterator QRegularExpressionMatch<S>::capturedEnd(QStringView<S> name) const
{
   if (name.isEmpty()) {
      return S();
   }

   int index = captureIndexForName(name);

   if (index == -1) {
      return S();
   }

   return capturedEnd(index);
}

template <typename S>
typename S::const_iterator QRegularExpressionMatch<S>::capturedStart(int index) const
{
   if (index < m_results.size()) {
      return m_results[index].first;

   } else {
      return typename S::const_iterator();
   }
}

template <typename S>
typename S::const_iterator QRegularExpressionMatch<S>::capturedStart(const S &name) const
{
   if (name.isEmpty()) {
      return S();
   }

   int index = captureIndexForName(name);

   if (index == -1) {
      return S();
   }

   return capturedStart(index);
}

template <typename S>
typename S::const_iterator QRegularExpressionMatch<S>::capturedStart(QStringView<S> name) const
{
   if (name.isEmpty()) {
      return S();
   }

   int index = captureIndexForName(name);

   if (index == -1) {
      return S();
   }

   return capturedStart(index);
}

template <typename S>
QStringView<S> QRegularExpressionMatch<S>::capturedView(const S &name) const
{
   if (name.isEmpty()) {
      return S();
   }

   int index = captureIndexForName(name);

   if (index == -1) {
      return S();
   }

   return capturedView(index);
}

template <typename S>
QStringView<S> QRegularExpressionMatch<S>::capturedView(QStringView<S> name) const
{
   if (name.isEmpty()) {
      return S();
   }

   int index = captureIndexForName(name);

   if (index == -1) {
      return S();
   }

   return capturedView(index);
}

template <typename S>
QList<S> QRegularExpressionMatch<S>::capturedTexts() const
{
   QList<S> list;

   for (const auto &item : m_results) {
      list.append( S(item.first, item.second) );
   }

   return list;
}

template <typename S>
QMatchType QRegularExpressionMatch<S>::matchType() const
{
   // broom - implement
}

template <typename S>
QMatchOptionFlags QRegularExpressionMatch<S>::matchOptions() const
{
   // broom - implement
}


// ** hash Implementations

template <typename S>
uint qHash(const QRegularExpression<S> &key, uint seed)
{
   // broom - implement
}

namespace std {
   template<typename S>
   struct hash<QRegularExpression<S>>
   {
      size_t operator()(const QRegularExpression<S> &key) const
      {
         // broom - implement
      }
   };
}

#endif
