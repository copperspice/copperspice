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

#ifndef QREGULAR_EXPRESSION_H
#define QREGULAR_EXPRESSION_H

#include "regex/regex.h"

#include <locale>

#include <qglobal.h>
#include <qchar32.h>
#include <qlist.h>
#include <qstringfwd.h>

template <typename S>
uint qHash(const Cs::QRegularExpression<S> &key, uint seed = 0);

enum class QPatternOption {
   NoPatternOption               = 0x0000,
   CaseInsensitiveOption         = 0x0001,
   DotMatchesEverythingOption    = 0x0002,
   MultilineOption               = 0x0004,
   ExtendedPatternSyntaxOption   = 0x0008,
   ExactMatchOption              = 0x0010,
   DontCaptureOption             = 0x0020,
   WildcardOption                = 0x0040,
   WildcardUnixOption            = 0x0080,
   FixedStringOption             = 0x0100,
};
Q_DECLARE_FLAGS(QPatternOptionFlags, QPatternOption)

enum class QMatchType {
   NormalMatch = 0,
   PartialPreferCompleteMatch,
   NoMatch
};

enum class QMatchOption {
   NoMatchOption                 = 0x0000,
   AnchoredMatchOption           = 0x0001
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


// type traits, internal only
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

      template<typename C = QChar32>
      QChar32 translate_nocase(QChar32 c) const {
         // broom - modify for case insensitive when multiple chars are returned
         return C(c).toCaseFolded()[0];
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
         string_type retval(first, last);
         return retval;
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

      template<typename C = QChar32>
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
            QChar tmp = C(c).toLower()[0];

            if (tmp.isDigit() || (tmp >= 'a' && tmp <= 'f')) {
               retval = true;
            }
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
         }

         return -1;
      }

      locale_type imbue(locale_type locale) {
         return locale;
      }

      locale_type getloc() const {
         return std::locale();
      }
};

#if ! defined (CS_DOXYPRESS)
namespace Cs {
#endif

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

      QList<QRegularExpressionMatch<S>> globalMatch(QStringView<S> str) const {
         return globalMatch(str, str.begin());
      }

      QList<QRegularExpressionMatch<S>> globalMatch(QStringView<S> str, typename S::const_iterator offset,
                  QMatchType matchType = QMatchType::NormalMatch, QMatchOptionFlags matchOptions = QMatchOption::NoMatchOption) const;

      bool isValid() const {
         return m_valid;
      }

      QRegularExpressionMatch<S> match(const S &str) const {
         return match(str, str.begin());
      }

      QRegularExpressionMatch<S> match(const S &str, typename S::const_iterator offset, QMatchType matchType = QMatchType::NormalMatch,
                  QMatchOptionFlags matchOptions = QMatchOption::NoMatchOption) const;

      QRegularExpressionMatch<S> match(QStringView<S> str) const {
         return match(str, str.begin());
      }

      QRegularExpressionMatch<S> match(QStringView<S> str, typename S::const_iterator offset,
                  QMatchType matchType = QMatchType::NormalMatch, QMatchOptionFlags matchOptions = QMatchOption::NoMatchOption) const;

      QList<S> namedCaptureGroups() const;

      int patternErrorOffset() const;

      S pattern() const {
         return m_pattern;
      }

      QPatternOptionFlags patternOptions() const {
         return m_patternOptions;
      }

      void setPattern(const S &pattern);

      void setPatternOptions(QPatternOptionFlags options)  {
         m_patternOptions = options;
         setPattern(m_pattern);
      }

      void swap(QRegularExpression &other)  {
         swap(m_regex, other.m_regex);
      }

      // operators
      QRegularExpression &operator=(const QRegularExpression &other) = default;
      QRegularExpression &operator=(QRegularExpression &&other) = default;


    // unsure if this methods is required
//    bool operator==(const QRegularExpression &regExp) const {
//    }

      // unsure if this methods is required
//    bool operator!=(const QRegularExpression &regExp) const {
//       return !operator==(regExp);
//    }

   private:
      S m_pattern;
      QPatternOptionFlags m_patternOptions = QPatternOption::NoPatternOption;
      cs_regex_ns::basic_regex<QChar32, QRegexTraits<S>> m_regex;

      bool m_valid = false;
      S m_errorString;

      static S convert_wildcard(const S &str, const bool enableEscaping);

      template <typename S2>
      friend Q_CORE_EXPORT uint qHash(const QRegularExpression<S2> &key, uint seed);
};

template <typename S>
class Q_CORE_EXPORT QRegularExpressionMatch
{
   public:
      QRegularExpressionMatch() = default;
      QRegularExpressionMatch(const QRegularExpressionMatch &other) = default;
      QRegularExpressionMatch(QRegularExpressionMatch &&other) = default;

      // internal only
      QRegularExpressionMatch(cs_regex_ns::match_results<QRegexTraits<S>> match, QMatchType matchType, QMatchOptionFlags matchOptions)
         : m_results(std::move(match)), m_matchType(matchType), m_matchOptions(matchOptions), m_valid(true)
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
         if (m_results.empty()) {
            return false;
         }

         return m_results[0].matched;
      }

      bool hasPartialMatch() const {
         if (m_results.empty()) {
            return false;
         }

         // may need additional changes

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
      cs_regex_ns::match_results<QRegexTraits<S>> m_results;

      QMatchType m_matchType = QMatchType::NoMatch;
      QMatchOptionFlags m_matchOptions = QMatchOption::NoMatchOption;

      bool m_valid = false;
};

// implementations

template <typename S>
QRegularExpression<S>::QRegularExpression(const S &pattern, QPatternOptionFlags options)
{
   S regex_pattern = cs_internal_regexp_toCanonical(pattern, options);

   m_patternOptions = options;
   setPattern(regex_pattern);
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

         result.append('\\');
         result.append('0');

      } else if (ch == '.'  || ch == '[' || ch == '{' || ch == '}' || ch == '(' || ch == ')' ||
                 ch == '\\' || ch == '*' || ch == '+' || ch == '?' || ch == '|' || ch == '^' || ch == '$')  {

         // must escaped exactly these chars
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
QList<QRegularExpressionMatch<S>> QRegularExpression<S>::globalMatch(QStringView<S> str, typename S::const_iterator offset,
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
   if (m_valid) {
      cs_regex_ns::match_results<QRegexTraits<S>> matchResult;

      // option flags
      cs_regex_ns::regex_constants::match_flag_type tmpOptions = cs_regex_ns::regex_constants::match_default;


      if (matchType == QMatchType::PartialPreferCompleteMatch) {
         tmpOptions |= cs_regex_ns::regex_constants::match_partial;
      }

      if (matchOptions & QMatchOption::AnchoredMatchOption) {
         tmpOptions |= cs_regex_ns::regex_constants::match_continuous;
      }

      // pattern flags
      if (! (m_patternOptions & QPatternOption::DotMatchesEverythingOption)) {
         // backwards from flag
         tmpOptions |= cs_regex_ns::regex_constants::match_not_dot_newline;
      }

      if (! (m_patternOptions & QPatternOption::MultilineOption)) {
         // backwards from flag
         tmpOptions |= cs_regex_ns::regex_constants::match_single_line;
      }

      if (cs_regex_ns::regex_search(offset, str.cend(), matchResult, m_regex, tmpOptions)) {
         QRegularExpressionMatch<S> retval{matchResult, matchType, matchOptions};
         return retval;

      } else {
         // valid regex, no match
         QRegularExpressionMatch<S> retval{cs_regex_ns::match_results<QRegexTraits<S>>(), QMatchType::NoMatch, matchOptions};
         return retval;
      }

   } else {
      // invalid
      return QRegularExpressionMatch<S>();

   }
}

template <typename S>
QRegularExpressionMatch<S> QRegularExpression<S>::match(QStringView<S> str, typename S::const_iterator offset,
                  QMatchType matchType, QMatchOptionFlags matchOptions) const
{
   if (m_valid) {
      cs_regex_ns::match_results<QRegexTraits<S>> matchResult;

      // option flags
      cs_regex_ns::regex_constants::match_flag_type tmpOptions = cs_regex_ns::regex_constants::match_default;


      if (matchType == QMatchType::PartialPreferCompleteMatch) {
         tmpOptions |= cs_regex_ns::regex_constants::match_partial;
      }

      if (matchOptions & QMatchOption::AnchoredMatchOption) {
         tmpOptions |= cs_regex_ns::regex_constants::match_continuous;
      }

      // pattern flags
      if (! (m_patternOptions & QPatternOption::DotMatchesEverythingOption)) {
         // backwards from flag
         tmpOptions |= cs_regex_ns::regex_constants::match_not_dot_newline;
      }

      if (! (m_patternOptions & QPatternOption::MultilineOption)) {
         // backwards from flag
         tmpOptions |= cs_regex_ns::regex_constants::match_single_line;
      }

      if (cs_regex_ns::regex_search(offset, str.cend(), matchResult, m_regex, tmpOptions)) {
         QRegularExpressionMatch<S> retval{matchResult, matchType, matchOptions};
         return retval;

      } else {
         // valid regex, no match
         QRegularExpressionMatch<S> retval{cs_regex_ns::match_results<QRegexTraits<S>>(), QMatchType::NoMatch, matchOptions};
         return retval;
      }

   } else {
      // invalid
      return QRegularExpressionMatch<S>();

   }
}

template <typename S>
QList<S> QRegularExpression<S>::namedCaptureGroups() const
{
   if (m_valid) {
      std::vector<S> tmp = m_regex.getNamedCaptureGroups();

      QList<S> retval(tmp.begin(), tmp.end());
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
   flags = 0;

   if (m_patternOptions & QPatternOption::CaseInsensitiveOption) {
      flags |= cs_regex_ns::regex_constants::icase;
   }

   if (m_patternOptions & QPatternOption::ExtendedPatternSyntaxOption) {
      flags |= cs_regex_ns::regex_constants::mod_x;
   }

   if (m_patternOptions & QPatternOption::DontCaptureOption ) {
      flags |= cs_regex_ns::regex_constants::nosubs;
   }

   // exact match
   if (m_patternOptions &QPatternOption::ExactMatchOption) {
      m_pattern = "\\A(?:" + m_pattern + ")\\z";
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
   return m_matchType;
}

template <typename S>
QMatchOptionFlags QRegularExpressionMatch<S>::matchOptions() const
{
   return m_matchOptions;
}

template<typename S>
S QRegularExpression<S>::convert_wildcard(const S &str, const bool enableEscaping)
{
   S retval;

   const int len = str.length();

   // previous character is '\'
   bool isEscaping = false;

   typename S::const_iterator iter = str.begin();
   typename S::const_iterator end  = str.end();

   while (iter != end) {
      const QChar32 c = *iter;
      ++iter;

      switch (c.unicode()) {

         case '\\':
            if (enableEscaping) {

               if (isEscaping) {
                  retval += "\\\\";
               }

               // we insert the \\ later if necessary

               if (iter == end) {
                  // the end
                  retval += "\\\\";
               }

            } else {
               retval += "\\\\";
            }

            isEscaping = true;
            break;

         case '*':
            if (isEscaping) {
               retval += "\\*";
               isEscaping = false;

            } else {
               retval += ".*";
            }

            break;

         case '?':
            if (isEscaping) {
               retval += "\\?";
               isEscaping = false;

            } else {
               retval += '.';
            }

            break;

         case '$':
         case '(':
         case ')':
         case '+':
         case '.':
         case '^':
         case '{':
         case '|':
         case '}':
            if (isEscaping) {
               isEscaping = false;
               retval += "\\\\";
            }

            retval += '\\';
            retval += c;
            break;

         case '[':
            if (isEscaping) {
               isEscaping = false;
               retval += "\\[";

            } else {
               retval += c;

               if (*iter == '^') {
                  retval += *iter;
                  ++iter;
               }

               if (iter != end) {

                  if (*iter == ']') {
                     retval += *iter;
                     ++iter;
                  }

                  while (iter != end && *iter != ']') {
                     if (*iter == '\\') {
                        retval += '\\';
                     }

                     retval += *iter;
                     ++iter;
                  }
               }
            }
            break;

         case ']':
            if (isEscaping) {
               isEscaping = false;
               retval += "\\";
            }

            retval += c;
            break;

         default:
            if (isEscaping) {
               isEscaping = false;
               retval += "\\\\";
            }
            retval += c;
      }
   }

   return retval;
}

#if ! defined (CS_DOXYPRESS)
}  // cs namespace
#endif


// ** hash Implementations

template <typename S>
uint qHash(const Cs::QRegularExpression<S> &key, uint seed)
{
   seed = qHash(key.m_pattern, seed);
   seed = qHash(key.m_patternOptions, seed);
   return seed;
}

// used by QScriptEngine::newRegExp to convert to a pattern that JavaScriptCore can understand
template <typename S>
static S wc2rx(const S &str, bool enableEscaping)
{
   S retval;

   int i           = 0;
   bool isEscaping = false;                // the previous character is '\'

   typename S::const_iterator iter = str.begin();
   typename S::const_iterator end  = str.end();

   while (iter != end) {
      auto c = *iter;
      ++iter;

      switch (c.unicode()) {

         case '\\':
            if (enableEscaping) {
               if (isEscaping) {
                  retval.append("\\\\");
               }

               // we insert the \\ later if necessary
               if (iter == end) {
                  retval.append("\\\\");
               }

            } else {
               retval.append("\\\\");
            }

            isEscaping = true;
            break;

         case '*':
            if (isEscaping) {
               retval.append("\\*");
               isEscaping = false;

            } else {
               retval.append(".*");
            }
            break;

         case '?':
            if (isEscaping) {
               retval.append("\\?");
               isEscaping = false;

            } else {
               retval.append('.');
            }

            break;

         case '$':
         case '(':
         case ')':
         case '+':
         case '.':
         case '^':
         case '{':
         case '|':
         case '}':
            if (isEscaping) {
               isEscaping = false;
               retval.append("\\\\");
            }

            retval.append('\\');
            retval.append(c);
            break;

         case '[':
            if (isEscaping) {
               isEscaping = false;
               retval.append("\\[");

            } else {
               retval.append(c);

               if (*iter == '^') {
                  retval.append(*iter);
                  ++iter;
               }

               if (iter != end) {

                  while (iter != end && *iter != ']') {
                     if (*iter == '\\') {
                        retval.append('\\');
                     }

                     retval.append(*iter);
                     ++iter;
                  }
               }
            }
            break;

         case ']':
            if (isEscaping) {
               isEscaping = false;
               retval.append("\\");
            }

            retval.append(c);
            break;

         default:
            if (isEscaping) {
               isEscaping = false;
               retval.append("\\\\");
            }

            retval.append(c);
      }
   }

   return retval;
}

template <typename S>
S cs_internal_regexp_toCanonical(const S &pattern, QPatternOptionFlags flags)
{
   if (flags & QPatternOption::WildcardOption) {
      return wc2rx(pattern, false);

   } else if (flags & QPatternOption::WildcardUnixOption) {
      return wc2rx(pattern, true);

   } else if (flags & QPatternOption::FixedStringOption) {
      return Cs::QRegularExpression<S>::escape(pattern);

   } else {
      return pattern;
   }
}

#endif
