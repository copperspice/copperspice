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

#include <qjsonparser_p.h>

#include <qcoreapplication.h>
#include <qjson.h>
#include <qstringparser.h>

static constexpr const int NESTING_LIMIT = 1024;

// error strings for the JSON parser
#define JSONERR_OK          cs_mark_tr("QJsonParseError", "no error occurred")
#define JSONERR_UNTERM_OBJ  cs_mark_tr("QJsonParseError", "unterminated object")
#define JSONERR_MISS_NSEP   cs_mark_tr("QJsonParseError", "missing name separator")
#define JSONERR_UNTERM_AR   cs_mark_tr("QJsonParseError", "unterminated array")
#define JSONERR_MISS_VSEP   cs_mark_tr("QJsonParseError", "missing value separator")
#define JSONERR_ILLEGAL_VAL cs_mark_tr("QJsonParseError", "illegal value")
#define JSONERR_END_OF_NUM  cs_mark_tr("QJsonParseError", "invalid termination by number")
#define JSONERR_ILLEGAL_NUM cs_mark_tr("QJsonParseError", "illegal number")
#define JSONERR_STR_ESC_SEQ cs_mark_tr("QJsonParseError", "invalid escape sequence")
#define JSONERR_STR_UTF8    cs_mark_tr("QJsonParseError", "invalid UTF8 string")
#define JSONERR_UTERM_STR   cs_mark_tr("QJsonParseError", "unterminated string")
#define JSONERR_MISS_OBJ    cs_mark_tr("QJsonParseError", "object is missing after a comma")
#define JSONERR_DEEP_NEST   cs_mark_tr("QJsonParseError", "too deeply nested document")
#define JSONERR_DOC_LARGE   cs_mark_tr("QJsonParseError", "too large document")

QString QJsonParseError::errorString() const
{
   QString sz;

   switch (error) {
      case NoError:
         sz = JSONERR_OK;
         break;

      case UnterminatedObject:
         sz = JSONERR_UNTERM_OBJ;
         break;

      case MissingNameSeparator:
         sz = JSONERR_MISS_NSEP;
         break;

      case UnterminatedArray:
         sz = JSONERR_UNTERM_AR;
         break;

      case MissingValueSeparator:
         sz = JSONERR_MISS_VSEP;
         break;

      case IllegalValue:
         sz = JSONERR_ILLEGAL_VAL;
         break;

      case TerminationByNumber:
         sz = JSONERR_END_OF_NUM;
         break;

      case IllegalNumber:
         sz = JSONERR_ILLEGAL_NUM;
         break;

      case IllegalEscapeSequence:
         sz = JSONERR_STR_ESC_SEQ;
         break;

      case IllegalUTF8String:
         sz = JSONERR_STR_UTF8;
         break;

      case UnterminatedString:
         sz = JSONERR_UTERM_STR;
         break;

      case MissingObject:
         sz = JSONERR_MISS_OBJ;
         break;

      case DeepNesting:
         sz = JSONERR_DEEP_NEST;
         break;

      case DocumentTooLarge:
         sz = JSONERR_DOC_LARGE;
         break;
   }

   return sz;
}

QJsonParser::QJsonParser(QStringView data)
   : m_data(data), nestingLevel(0), lastError(QJsonParseError::NoError)
{
}

void QJsonParser::eatBOM()
{
   // eat byte order mark
   if (m_data.startsWith(char32_t(0xFEFF))  )  {
      ++m_position;
   }
}

bool QJsonParser::eatWhiteSpace()
{
   while (m_position != m_data.end()) {
      QChar uc = *m_position;

      if (uc > QChar::Space) {
         break;

      } else if (uc != QChar::Space && uc != QChar::Tabulation && uc != QChar::LineFeed && uc != QChar::CarriageReturn) {
         break;

      }

      ++m_position;
   }

   return (m_position != m_data.end());
}

QJsonParser::TokenType QJsonParser::nextToken()
{
   if (! eatWhiteSpace()) {
      return TokenType::Null;
   }

   TokenType token;

   QChar ch = *m_position;
   m_position++;

   switch (ch.unicode()) {

      case '[':
         eatWhiteSpace();

         token = TokenType::BeginArray;
         break;

      case '{':
         eatWhiteSpace();

         token = TokenType::BeginObject;
         break;

      case ':':
         eatWhiteSpace();

         token = TokenType::NameSeparator;
         break;

      case ',':
         eatWhiteSpace();

         token = TokenType::ValueSeparator;
         break;

      case ']':
         eatWhiteSpace();

         token = TokenType::EndArray;
         break;

      case '}':
         eatWhiteSpace();

         token = TokenType::EndObject;
         break;

      case '"':
         token = TokenType::Quote;
         break;

      default:
         token = TokenType::Null;
         break;
   }

   return token;
}

QJsonDocument QJsonParser::parse(QJsonParseError *error)
{
   if (error != nullptr) {
      error->offset = 0;
      error->error  = QJsonParseError::NoError;
   }

   m_position = m_data.begin();
   eatBOM();

   TokenType token = nextToken();

   if (token == TokenType::BeginArray) {
      QJsonArray array;

      if (parseArray(array)) {
         return QJsonDocument(array);
      }

   } else if (token == TokenType::BeginObject) {
      QJsonObject object;

      if (parseObject(object)) {
         return QJsonDocument(object);
      }

   } else {
      lastError = QJsonParseError::IllegalValue;
   }

   if (error != nullptr) {
      error->offset = m_position - m_data.begin();
      error->error  = lastError;
   }

   return QJsonDocument();
}

bool QJsonParser::parseArray(QJsonArray &array)
{
   if (++nestingLevel > NESTING_LIMIT) {
      lastError = QJsonParseError::DeepNesting;
      return false;
   }

   eatWhiteSpace();

   if (m_position == m_data.end()) {
      lastError = QJsonParseError::UnterminatedArray;
      return false;
   }

   if (*m_position == ']') {
      // valid empty array
      nextToken();

   } else {

      while (true) {
         // parse a value
         QJsonValue value;

         if (! parseValue(value)) {
            return false;
         }

         array.append(value);

         //
         TokenType token = nextToken();

         if (token == TokenType::EndArray) {
            // close square bracket
            break;

         } else if (token != TokenType::ValueSeparator) {
            // not a comma

            if (! eatWhiteSpace()) {
               lastError = QJsonParseError::UnterminatedArray;

            } else {
               lastError = QJsonParseError::MissingValueSeparator;
            }

            return false;
         }
      }
   }

   --nestingLevel;

   return true;
}

bool QJsonParser::parseObject(QJsonObject &object)
{
   if (++nestingLevel > NESTING_LIMIT) {
      lastError = QJsonParseError::DeepNesting;
      return false;
   }

   TokenType token = nextToken();

   while (token == TokenType::Quote) {

      if (! parseMember(object)) {
         return false;
      }

      //
      token = nextToken();

      if (token != TokenType::ValueSeparator) {
         // not a comma so we are done
         break;
      }

      //
      token = nextToken();

      if (token == TokenType::EndObject) {
         // found closing curly which is invalid
         lastError = QJsonParseError::MissingObject;
         return false;
      }
   }

   if (token != TokenType::EndObject) {
      lastError = QJsonParseError::UnterminatedObject;
      return false;
   }

   --nestingLevel;

   return true;
}

bool QJsonParser::parseMember(QJsonObject &object)
{
   QString key;

   if (! parseString(key) ) {
      return false;
   }

   //
   TokenType token = nextToken();

   if (token != TokenType::NameSeparator) {
      lastError = QJsonParseError::MissingNameSeparator;
      return false;
   }

   QJsonValue value;

   if (! parseValue(value)) {
      return false;
   }

   // save key and value
   object.insert(key, value);

   return true;
}

bool QJsonParser::parseValue(QJsonValue &value)
{
   QChar32 ch = *m_position;
   ++m_position;

   switch (ch.unicode()) {

      case 'n':
         if (m_data.end() - m_position < 4) {
            lastError = QJsonParseError::IllegalValue;
            return false;
         }

         if (QStringView(m_position, m_data.end()).startsWith("ull")) {
            m_position += 3;

            value = QJsonValue(QJsonValue::Null);
            return true;
         }

         lastError = QJsonParseError::IllegalValue;
         return false;

      case 't':
         if (m_data.end() - m_position < 4) {
            lastError = QJsonParseError::IllegalValue;
            return false;
         }

         if (QStringView(m_position, m_data.end()).startsWith("rue")) {
            m_position += 3;

            value = QJsonValue(true);
            return true;
         }

         lastError = QJsonParseError::IllegalValue;
         return false;

      case 'f':
         if (m_data.end() - m_position < 5) {
            lastError = QJsonParseError::IllegalValue;
            return false;
         }

         if (QStringView(m_position, m_data.end()).startsWith("alse")) {
            m_position += 4;

            value = QJsonValue(false);
            return true;
         }

         lastError = QJsonParseError::IllegalValue;
         return false;

      case '"': {
         QString key;

         if (! parseString(key) ) {
            return false;
         }

         value = QJsonValue(key);

         return true;
      }

      case '[': {
         QJsonArray array;

         if (! parseArray(array)) {
            return false;
         }

         value = QJsonValue(array);
         return true;
      }

      case '{':  {
         QJsonObject object;

         if (! parseObject(object)) {
            return false;
         }

         value = QJsonValue(object);
         return true;
      }

      case ']':
         lastError = QJsonParseError::MissingObject;
         return false;

      default:
         --m_position;

         if (! parseNumber(value)) {
            return false;
         }
   }

   return true;
}

bool QJsonParser::parseNumber(QJsonValue &value)
{
   QString::const_iterator start = m_position;

   while (m_position != m_data.end())  {

      QChar ch = *m_position;

      if (ch == '.' || ch == '-' || ch == '+' || ch == 'e' || ch == 'E' ||  (ch >= '0' && ch <= '9') ) {
         ++m_position;

      } else {
         break;

      }
   }

   bool ok;
   double retval = QStringParser::toDouble(QStringView(start, m_position), &ok);

   if (m_position == m_data.end()) {
      lastError = QJsonParseError::TerminationByNumber;
      return false;
   }

   if (! ok) {
      lastError = QJsonParseError::IllegalNumber;
      return false;
   }

   value = QJsonValue(retval);

   return true;
}

static inline bool addHexDigit(QChar digit, char32_t &result)
{
   result <<= 4;

   if (digit >= '0' && digit <= '9') {
      result |= (digit.unicode() - '0');

   } else if (digit >= 'a' && digit <= 'f') {
      result |= (digit.unicode()  - 'a') + 10;

   } else if (digit >= 'A' && digit <= 'F') {
      result |= (digit.unicode()  - 'A') + 10;

   } else {
      return false;
   }

   return true;
}

static inline bool scanEscapeSequence(QString::const_iterator &m_position, QString::const_iterator end, QChar32 &ch)
{
   ++m_position;

   if (m_position == end) {
      return false;
   }

   QChar32 escaped = *m_position;
   ++m_position;

   switch (escaped.unicode()) {
      case '"':
         ch = '"';
         break;

      case '\\':
         ch = '\\';
         break;

      case '/':
         ch = '/';
         break;

      case 'b':
         ch = 0x8;
         break;

      case 'f':
         ch = 0xc;
         break;

      case 'n':
         ch = 0xa;
         break;

      case 'r':
         ch = 0xd;
         break;

      case 't':
         ch = 0x9;
         break;

      case 'u': {
         char32_t tmp = 0;

         for (int i = 0; i < 4; ++i) {

            if (! addHexDigit(*m_position, tmp)) {
               return false;
            }

            ++m_position;

            if (m_position == end && i != 3) {
               return false;
            }
         }

         ch = tmp;
         return true;
      }

      default:
         // not as strict, allows for more Json files to be parsed correctly
         ch = escaped;
         return true;
   }

   return true;
}

bool QJsonParser::parseString(QString &str)
{
   while (m_position != m_data.end()) {
      QChar32 ch = *m_position;

      if (ch == '"') {
         break;

      } else if (ch == '\\') {
         if (! scanEscapeSequence(m_position, m_data.end(), ch)) {
            lastError = QJsonParseError::IllegalEscapeSequence;
            return false;
         }

      } else {
         ++m_position;

      }

      str.append(ch);
   }

   ++m_position;

   if (m_position == m_data.end()) {
      lastError = QJsonParseError::UnterminatedString;
      return false;
   }

   return true;
}
