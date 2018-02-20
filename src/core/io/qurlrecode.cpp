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

#include <qurl.h>
#include <qutfcodec_p.h>
#include <qtools_p.h>

#include <qstring8.h>

// ### move to qurl_p.h
enum EncodingAction {
   DecodeCharacter = 0,
   LeaveCharacter  = 1,
   EncodeCharacter = 2
};

// From RFC 3896, Appendix A Collected ABNF for URI
//    unreserved    = ALPHA / DIGIT / "-" / "." / "_" / "~"
//    reserved      = gen-delims / sub-delims
//    gen-delims    = ":" / "/" / "?" / "#" / "[" / "]" / "@"
//    sub-delims    = "!" / "$" / "&" / "'" / "(" / ")"
//                  / "*" / "+" / "," / ";" / "="
static const uchar defaultActionTable[96] = {
   2, // space
   1, // '!' (sub-delim)
   2, // '"'
   1, // '#' (gen-delim)
   1, // '$' (gen-delim)
   2, // '%' (percent)
   1, // '&' (gen-delim)
   1, // "'" (sub-delim)
   1, // '(' (sub-delim)
   1, // ')' (sub-delim)
   1, // '*' (sub-delim)
   1, // '+' (sub-delim)
   1, // ',' (sub-delim)
   0, // '-' (unreserved)
   0, // '.' (unreserved)
   1, // '/' (gen-delim)

   0, 0, 0, 0, 0,  // '0' to '4' (unreserved)
   0, 0, 0, 0, 0,  // '5' to '9' (unreserved)
   1, // ':' (gen-delim)
   1, // ';' (sub-delim)
   2, // '<'
   1, // '=' (sub-delim)
   2, // '>'
   1, // '?' (gen-delim)

   1, // '@' (gen-delim)
   0, 0, 0, 0, 0,  // 'A' to 'E' (unreserved)
   0, 0, 0, 0, 0,  // 'F' to 'J' (unreserved)
   0, 0, 0, 0, 0,  // 'K' to 'O' (unreserved)
   0, 0, 0, 0, 0,  // 'P' to 'T' (unreserved)
   0, 0, 0, 0, 0, 0,  // 'U' to 'Z' (unreserved)
   1, // '[' (gen-delim)
   2, // '\'
   1, // ']' (gen-delim)
   2, // '^'
   0, // '_' (unreserved)

   2, // '`'
   0, 0, 0, 0, 0,  // 'a' to 'e' (unreserved)
   0, 0, 0, 0, 0,  // 'f' to 'j' (unreserved)
   0, 0, 0, 0, 0,  // 'k' to 'o' (unreserved)
   0, 0, 0, 0, 0,  // 'p' to 't' (unreserved)
   0, 0, 0, 0, 0, 0,  // 'u' to 'z' (unreserved)
   2, // '{'
   2, // '|'
   2, // '}'
   0, // '~' (unreserved)

   2  // BSKP
};

// mask tables, in negative polarity
// 0x00 if it belongs to this category
// 0xff if it doesn't

static const uchar reservedMask[96] = {
   0xff, // space
   0xff, // '!' (sub-delim)
   0x00, // '"'
   0xff, // '#' (gen-delim)
   0xff, // '$' (gen-delim)
   0xff, // '%' (percent)
   0xff, // '&' (gen-delim)
   0xff, // "'" (sub-delim)
   0xff, // '(' (sub-delim)
   0xff, // ')' (sub-delim)
   0xff, // '*' (sub-delim)
   0xff, // '+' (sub-delim)
   0xff, // ',' (sub-delim)
   0xff, // '-' (unreserved)
   0xff, // '.' (unreserved)
   0xff, // '/' (gen-delim)

   0xff, 0xff, 0xff, 0xff, 0xff,  // '0' to '4' (unreserved)
   0xff, 0xff, 0xff, 0xff, 0xff,  // '5' to '9' (unreserved)
   0xff, // ':' (gen-delim)
   0xff, // ';' (sub-delim)
   0x00, // '<'
   0xff, // '=' (sub-delim)
   0x00, // '>'
   0xff, // '?' (gen-delim)

   0xff, // '@' (gen-delim)
   0xff, 0xff, 0xff, 0xff, 0xff,  // 'A' to 'E' (unreserved)
   0xff, 0xff, 0xff, 0xff, 0xff,  // 'F' to 'J' (unreserved)
   0xff, 0xff, 0xff, 0xff, 0xff,  // 'K' to 'O' (unreserved)
   0xff, 0xff, 0xff, 0xff, 0xff,  // 'P' to 'T' (unreserved)
   0xff, 0xff, 0xff, 0xff, 0xff, 0xff,  // 'U' to 'Z' (unreserved)
   0xff, // '[' (gen-delim)
   0x00, // '\'
   0xff, // ']' (gen-delim)
   0x00, // '^'
   0xff, // '_' (unreserved)

   0x00, // '`'
   0xff, 0xff, 0xff, 0xff, 0xff,  // 'a' to 'e' (unreserved)
   0xff, 0xff, 0xff, 0xff, 0xff,  // 'f' to 'j' (unreserved)
   0xff, 0xff, 0xff, 0xff, 0xff,  // 'k' to 'o' (unreserved)
   0xff, 0xff, 0xff, 0xff, 0xff,  // 'p' to 't' (unreserved)
   0xff, 0xff, 0xff, 0xff, 0xff, 0xff,  // 'u' to 'z' (unreserved)
   0x00, // '{'
   0x00, // '|'
   0x00, // '}'
   0xff, // '~' (unreserved)

   0xff  // BSKP
};

static inline bool isHex(ushort c)
{
   return (c >= 'a' && c <= 'f') ||
          (c >= 'A' && c <= 'F') ||
          (c >= '0' && c <= '9');
}

static inline bool isHex(QChar c)
{
   return (c >= 'a' && c <= 'f') ||
          (c >= 'A' && c <= 'F') ||
          (c >= '0' && c <= '9');
}

static inline bool isUpperHex(ushort c)
{
   // undefined behaviour if c is not a hex char
   return c < 0x60;
}

static inline bool isUpperHex(QChar c)
{
   return c < 0x60;
}

static inline ushort toUpperHex(ushort c)
{
   return isUpperHex(c) ? c : c - 0x20;
}

static inline QChar toUpperHex(QChar c)
{
   return isUpperHex(c) ? c : c.unicode() - 0x20;
}

static inline ushort decodeNibble(ushort c)
{
   return c >= 'a' ? c - 'a' + 0xA :
          c >= 'A' ? c - 'A' + 0xA : c - '0';
}

static inline int decodeNibble(QChar c)
{
   return c >= 'a' ? c.unicode() - 'a' + 0xA :
          c >= 'A' ? c.unicode() - 'A' + 0xA : c.unicode() - '0';
}

// if the sequence at input is 2*HEXDIG then return its decoding, otherwise returns -1
static inline int decodePercentEncoding(QString::const_iterator begin, QString::const_iterator end)
{
   ++begin;

   if (begin == end) {
      return -1;
   }
   QChar c1 = *begin;

   ++begin;

   if (begin == end) {
      return -1;
   }
   QChar c2 = *begin;

   if (! isHex(c1) || ! isHex(c2)) {
      return -1;
   }

   return decodeNibble(c1) << 4 | decodeNibble(c2);
}

static inline ushort encodeNibble(ushort c)
{
   return ushort(QtMiscUtils::toHexUpper(c));
}

// returns true if we performed a UTF-8 decoding
static bool encoded_utf8_to_utf16(int c, QString::const_iterator &iter, QString &retval,
                                  QString::const_iterator end)
{
   QByteArray tmpStr;
   tmpStr.append(c);

   QString::const_iterator tmpIter = iter + 3;

   while (tmpIter != end) {

      if (*tmpIter != '%') {
         // end of percent encoded data
         break;
      }

      int tmp = decodePercentEncoding(tmpIter, end);

      if (tmp  == -1) {
         // invalid char after %
         return false;
      }

      tmpStr.append(tmp);
      tmpIter = tmpIter + 3;
   }

   retval.append(QString::fromUtf8(tmpStr));
   iter = tmpIter - 1;

   return true;
}

static void utf16_to_encoded_utf8(QChar c, QString::const_iterator &iter, QString &retval,
                                  QString::const_iterator end)
{
   // check  QChar is a surrogate
   QString8 tmpStr;

   if (! c.isHighSurrogate() &&  ! c.isLowSurrogate()) {
      // have entire code point
      tmpStr.append( static_cast<char32_t>(c.unicode()) );

   } else if (c.isHighSurrogate() && iter != end && iter->isLowSurrogate()) {
      // low surrogate after current high surrogate

      QChar c2 = *iter;
      ++iter;

      tmpStr.append(  static_cast<char32_t>(QChar::surrogateToUcs4(c, c2)));

   } else {
      // bad surrogate pair sequence, encode bad UTF-16 to WTF-8

      // first of three bytes
      uchar tmp = 0xe0 | uchar(c.unicode() >> 12);

      retval.append("%E");
      retval.append( encodeNibble(tmp & 0xf) );

      // second byte
      tmp = 0x80 | (uchar(c.unicode() >> 6) & 0x3f);

      retval.append('%');
      retval.append( encodeNibble(tmp >> 4)  );
      retval.append( encodeNibble(tmp & 0xf) );

      // third byte
      tmp = 0x80 | (c.unicode() & 0x3f);

      retval.append('%');
      retval.append( encodeNibble(tmp >> 4)  );
      retval.append( encodeNibble(tmp & 0xf) );

      return;
   }

   // add the percent
   for (const char *data = tmpStr.constData(); *data != 0; ++data)  {
      uchar tmp = *data;

      retval.append('%');
      retval.append( encodeNibble(tmp >> 4)  );
      retval.append( encodeNibble(tmp & 0xf) );
   }
}

void non_trivial ( QChar c, QString::const_iterator &iter, QString &retval, EncodingAction &action,
                   QString::const_iterator begin, QString::const_iterator end,
                   QUrl::FormattingOptions encoding, const uchar *actionTable)
{
   QChar decoded;

   if (c == '%') {
      // check if the input is valid
      int tmp = decodePercentEncoding(iter, end);

      if (tmp  == -1) {
         // invalid char
         retval.append("%25");
         return;
      }

      decoded = tmp;

      if (tmp >= 0x80) {
         // decode the UTF-8 sequence
         if (! (encoding & QUrl::EncodeUnicode) && encoded_utf8_to_utf16(tmp, iter, retval, end)) {
            return;
         }

         // decoding the encoded UTF-8 failed
         action = LeaveCharacter;

      } else if (decoded >= 0x20) {
         action = EncodingAction(actionTable[decoded.unicode() - ' ']);
      }

   } else {

      if (c >= 0x80 && encoding & QUrl::EncodeUnicode) {
         // encode the UTF-8 sequence
         utf16_to_encoded_utf8(c, iter, retval, end);
         return;

      } else if (c >= 0x80) {
         retval.append(c);
         return;
      }

      decoded = c;
   }

   // there are six possibilities:
   //  current \ action  | DecodeCharacter | LeaveCharacter | EncodeCharacter
   //      decoded       |    1:leave      |    2:leave     |    3:encode
   //      encoded       |    4:decode     |    5:leave     |    6:leave
   // cases 1 and 2 were handled before this section

   if (c == '%' && action != DecodeCharacter) {
      // cases 5 and 6: it is encoded and we are leaving it as it is except uppercase the hex

      if (! isUpperHex(iter[1]) || ! isUpperHex(iter[2])) {

         retval.append('%');
         retval.append( toUpperHex(*++iter) );
         retval.append( toUpperHex(*++iter) );
      }

   } else if (c == '%' && action == DecodeCharacter) {
      // case 4: we need to decode

      retval.append(decoded);
      iter += 2;

   } else {
      // must be case 3: we need to encode

      retval.append('%');
      retval.append( encodeNibble(c.unicode() >> 4) );
      retval.append( encodeNibble(c.unicode() & 0xf) );
   }
}

static int decode(QString &appendTo, const ushort *begin, const ushort *end)
{
   const int origSize  = appendTo.size();
   const ushort *input = begin;
   ushort *output = 0;

   while (input != end) {
      if (*input != '%') {

         if (output) {
            *output++ = *input;
         }

         ++input;
         continue;
      }

      if (Q_UNLIKELY(end - input < 3 || !isHex(input[1]) || !isHex(input[2]))) {
         // badly-encoded data
         appendTo.resize(origSize + (end - begin));
         memcpy(appendTo.begin() + origSize, begin, (end - begin) * sizeof(ushort));
         return end - begin;
      }

      if (Q_UNLIKELY(!output)) {
         // detach
         appendTo.resize(origSize + (end - begin));
         output = reinterpret_cast<ushort *>(appendTo.begin()) + origSize;
         memcpy(output, begin, (input - begin) * sizeof(ushort));
         output += input - begin;
      }

      ++input;
      *output++ = decodeNibble(input[0]) << 4 | decodeNibble(input[1]);

      if (output[-1] >= 0x80) {
         output[-1] = QChar::ReplacementCharacter;
      }
      input += 2;
   }

   if (output) {
      int len = output - reinterpret_cast<ushort *>(appendTo.begin());
      appendTo.truncate(len);
      return len - origSize;
   }

   return 0;
}

static int recode(QString &result, QString::const_iterator begin, QString::const_iterator end,
                  QUrl::FormattingOptions encoding, const uchar *actionTable)
{
   QString retval = result;

   const int origSize = result.size();
   QString::const_iterator iter = begin;
   QChar c;

   EncodingAction action = EncodeCharacter;

   // try a run where no change is necessary
   for ( ; iter != end; ++iter) {
      c = *iter;

      if (c < 0x20U) {
         action = EncodeCharacter;
      }

      if (c < 0x20U || c >= 0x80U) {    // also: (c - 0x20 < 0x60U)
         non_trivial(c, iter, retval, action, begin, end, encoding, actionTable);
         continue;
      }

      action = EncodingAction(actionTable[c.unicode() - ' ']);

      if (action == EncodeCharacter) {
         non_trivial(c, iter, retval, action, begin, end, encoding, actionTable);
         continue;
      }

      retval.append(c);
   }

   if (retval != result) {
      result = std::move(retval);
      return result.size() - origSize;
   }

   return 0;
}

template <size_t N>
static void maskTable(uchar (&table)[N], const uchar (&mask)[N])
{
   for (size_t i = 0; i < N; ++i) {
      table[i] &= mask[i];
   }
}

/*!
    \internal

    Recodes the string from begin to end.
    If any transformations are done append them to appendTo and return the number of characters added.
    If no transformations were required return 0.

    \li QUrl::DecodeReserved: if set reserved characters will be decoded;
                              if unset reserved characters will be encoded

    \li QUrl::EncodeSpaces:   if set spaces will be encoded to "%20"; if unset, they will be " "

    \li QUrl::EncodeUnicode:  if set characters above U+0080 will be encoded to their UTF-8
                              percent-encoded form; if unset, they will be decoded to UTF-16

    \li QUrl::FullyDecoded:   if set this function will decode all percent-encoded sequences,
                              including that of the percent character. The resulting string
                              will not be percent-encoded anymore. Use with caution!
                              In this mode, the behaviour is undefined if the input string
                              contains any percent-encoding sequences above %80.
                              Also, the function will not correct bad % sequences.

    The tableModifications argument can be used to supply extra modifications to the tables,
    to be applied after the flags above are handled. It consists of a sequence of 16-bit values,
    where the low 8 bits indicate the character in question and the high 8 bits are either
    EncodeCharacter, LeaveCharacter or DecodeCharacter.

    Corrects percent encoded errors by interpreting every '%' as meaning "%25"
*/

int qt_urlRecode(QString &appendTo, const QChar *begin, const QChar *end,
                 QUrl::FormattingOptions encoding, const ushort *tableModifications)
{
   uchar actionTable[sizeof defaultActionTable];

   if (encoding == QUrl::FullyDecoded) {
      return decode(appendTo, reinterpret_cast<const ushort *>(begin), reinterpret_cast<const ushort *>(end));
   }

   memcpy(actionTable, defaultActionTable, sizeof actionTable);

   if (encoding & QUrl::DecodeReserved)  {
      maskTable(actionTable, reservedMask);
   }

   if (! (encoding & QUrl::EncodeSpaces)) {
      // decode
      actionTable[0] = DecodeCharacter;
   }

   if (tableModifications) {
      for (const ushort *p = tableModifications; *p; ++p)  {
         actionTable[uchar(*p) - ' '] = *p >> 8;
      }
   }

   // begin and end now treated as iterators
   int retval = recode(appendTo, begin, end, encoding, actionTable);

   return retval;
}

QString qt_urlRecodeByteArray(const QByteArray &ba)
{
   if (ba.isNull()) {
      return QString();
   }

   // scan ba for anything above or equal to 0x80
   // control points below 0x20 are fine in QString
   const char *in = ba.constData();
   const char *const end = ba.constEnd();

   for ( ; in < end; ++in) {
      if (*in & 0x80) {
         break;
      }
   }

   if (in == end) {
      // no non-ASCII found
      return QString::fromLatin1(ba.constData(), ba.size());
   }

   // we found something that we need to encode
   QByteArray intermediate = ba;
   intermediate.resize(ba.size() * 3 - (in - ba.constData()));
   uchar *out = reinterpret_cast<uchar *>(intermediate.data() + (in - ba.constData()));

   for ( ; in < end; ++in) {
      if (*in & 0x80) {
         // encode
         *out++ = '%';
         *out++ = encodeNibble(uchar(*in) >> 4);
         *out++ = encodeNibble(uchar(*in) & 0xf);

      } else {
         // keep
         *out++ = uchar(*in);
      }
   }

   return QString::fromLatin1(intermediate.constData(), out - reinterpret_cast<uchar *>(intermediate.data()));
}

