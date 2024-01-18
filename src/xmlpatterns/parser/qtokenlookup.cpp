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

/* C++ code produced by gperf version 3.0.3 */
/* Command-line: gperf TokenLookup.gperf  */
/* Computed positions: -k'1,3,$' */

#if !((' ' == 32) && ('!' == 33) && ('"' == 34) && ('#' == 35) \
      && ('%' == 37) && ('&' == 38) && ('\'' == 39) && ('(' == 40) \
      && (')' == 41) && ('*' == 42) && ('+' == 43) && (',' == 44) \
      && ('-' == 45) && ('.' == 46) && ('/' == 47) && ('0' == 48) \
      && ('1' == 49) && ('2' == 50) && ('3' == 51) && ('4' == 52) \
      && ('5' == 53) && ('6' == 54) && ('7' == 55) && ('8' == 56) \
      && ('9' == 57) && (':' == 58) && (';' == 59) && ('<' == 60) \
      && ('=' == 61) && ('>' == 62) && ('?' == 63) && ('A' == 65) \
      && ('B' == 66) && ('C' == 67) && ('D' == 68) && ('E' == 69) \
      && ('F' == 70) && ('G' == 71) && ('H' == 72) && ('I' == 73) \
      && ('J' == 74) && ('K' == 75) && ('L' == 76) && ('M' == 77) \
      && ('N' == 78) && ('O' == 79) && ('P' == 80) && ('Q' == 81) \
      && ('R' == 82) && ('S' == 83) && ('T' == 84) && ('U' == 85) \
      && ('V' == 86) && ('W' == 87) && ('X' == 88) && ('Y' == 89) \
      && ('Z' == 90) && ('[' == 91) && ('\\' == 92) && (']' == 93) \
      && ('^' == 94) && ('_' == 95) && ('a' == 97) && ('b' == 98) \
      && ('c' == 99) && ('d' == 100) && ('e' == 101) && ('f' == 102) \
      && ('g' == 103) && ('h' == 104) && ('i' == 105) && ('j' == 106) \
      && ('k' == 107) && ('l' == 108) && ('m' == 109) && ('n' == 110) \
      && ('o' == 111) && ('p' == 112) && ('q' == 113) && ('r' == 114) \
      && ('s' == 115) && ('t' == 116) && ('u' == 117) && ('v' == 118) \
      && ('w' == 119) && ('x' == 120) && ('y' == 121) && ('z' == 122) \
      && ('{' == 123) && ('|' == 124) && ('}' == 125) && ('~' == 126))
/* The character set is not based on ISO-646.  */
#error "gperf generated tables do not work with this execution character set. Report to <bug-gnu-gperf@gnu.org>."
#endif

#line 107 "TokenLookup.gperf"

namespace QPatternist {

#line 101 "TokenLookup.gperf"
struct TokenMap {
   const char *name;
   const Tokenizer::TokenType token;
}


/* The strings below are in UTF-16 encoding. Subsequently, each ASCII
 * character is stored as the ASCII character, followed by a null byte.
 * Sorted alphabetically. */;
/* maximum key range = 228, duplicates = 0 */

class TokenLookup
{
 public:
   static const struct TokenMap *value (const char *str, unsigned int len);

 private:
   static inline unsigned int hash (const char *str, unsigned int len);
};

inline unsigned int
TokenLookup::hash (const char *str, unsigned int len)
{
   static const unsigned char asso_values[] = {
      230, 230, 230, 230, 230, 230, 230, 230, 230, 230,
      230, 230, 230, 230, 230, 230, 230, 230, 230, 230,
      230, 230, 230, 230, 230, 230, 230, 230, 230, 230,
      230, 230, 230, 230, 230, 230, 230, 230, 230, 230,
      230, 230, 230, 230, 230,  25, 230, 230, 230, 230,
      230, 230, 230, 230, 230, 230, 230, 230, 230, 230,
      230, 230, 230, 230, 230, 230, 230, 230, 230, 230,
      230, 230, 230, 230, 230, 230, 230, 230, 230, 230,
      230, 230, 230, 230, 230, 230, 230, 230, 230, 230,
      230, 230, 230, 230, 230, 230, 230,   0,   2,   5,
      25,   0,  20,  20,  35,  85, 230, 230,  40, 110,
      25,  65,  80,   0,  60,   5,  10,   0,  55,   5,
      20,   0, 230, 230, 230, 230, 230, 230, 230, 230,
      230, 230, 230, 230, 230, 230, 230, 230, 230, 230,
      230, 230, 230, 230, 230, 230, 230, 230, 230, 230,
      230, 230, 230, 230, 230, 230, 230, 230, 230, 230,
      230, 230, 230, 230, 230, 230, 230, 230, 230, 230,
      230, 230, 230, 230, 230, 230, 230, 230, 230, 230,
      230, 230, 230, 230, 230, 230, 230, 230, 230, 230,
      230, 230, 230, 230, 230, 230, 230, 230, 230, 230,
      230, 230, 230, 230, 230, 230, 230, 230, 230, 230,
      230, 230, 230, 230, 230, 230, 230, 230, 230, 230,
      230, 230, 230, 230, 230, 230, 230, 230, 230, 230,
      230, 230, 230, 230, 230, 230, 230, 230, 230, 230,
      230, 230, 230, 230, 230, 230, 230, 230, 230, 230,
      230, 230, 230, 230, 230, 230
   };
   int hval = len;

   switch (hval) {
      default:
         hval += asso_values[(unsigned char)str[2]];
         [[fallthrough]];

      case 2:
      case 1:
         hval += asso_values[(unsigned char)str[0]];
         break;
   }

   return hval + asso_values[(unsigned char)str[len - 1]];
}

const struct TokenMap * TokenLookup::value (const char *str, unsigned int len)
{
   enum {
      TOTAL_KEYWORDS = 99,
      MIN_WORD_LENGTH = 2,
      MAX_WORD_LENGTH = 22,
      MIN_HASH_VALUE = 2,
      MAX_HASH_VALUE = 229
   };

   static const struct TokenMap wordlist[] = {
      {"", ERROR}, {"", ERROR},
#line 152 "TokenLookup.gperf"
      {"eq",                       EQ},
      {"", ERROR},
#line 130 "TokenLookup.gperf"
      {"by",                       BY},
#line 153 "TokenLookup.gperf"
      {"every",                    EVERY},
      {"", ERROR},
#line 123 "TokenLookup.gperf"
      {"as",                       AS},
      {"", ERROR},
#line 148 "TokenLookup.gperf"
      {"else",                     ELSE},
#line 217 "TokenLookup.gperf"
      {"where",                    WHERE},
#line 204 "TokenLookup.gperf"
      {"stable",                   STABLE},
#line 126 "TokenLookup.gperf"
      {"at",                       AT},
      {"", ERROR},
#line 131 "TokenLookup.gperf"
      {"case",                     CASE},
      {"", ERROR},
#line 129 "TokenLookup.gperf"
      {"boundary-space",           BOUNDARY_SPACE},
#line 147 "TokenLookup.gperf"
      {"element",                  ELEMENT},
#line 132 "TokenLookup.gperf"
      {"castable",                 CASTABLE},
#line 127 "TokenLookup.gperf"
      {"attribute",                ATTRIBUTE},
      {"", ERROR},
#line 154 "TokenLookup.gperf"
      {"except",                   EXCEPT},
#line 161 "TokenLookup.gperf"
      {"ge",                       GE},
      {"", ERROR},
#line 133 "TokenLookup.gperf"
      {"cast",                     CAST},
#line 210 "TokenLookup.gperf"
      {"treat",                    TREAT},
#line 218 "TokenLookup.gperf"
      {"xquery",                   XQUERY},
#line 181 "TokenLookup.gperf"
      {"ne",                       NE},
      {"", ERROR},
#line 198 "TokenLookup.gperf"
      {"satisfies",                SATISFIES},
      {"", ERROR}, {"", ERROR},
#line 163 "TokenLookup.gperf"
      {"gt",                       GT},
#line 151 "TokenLookup.gperf"
      {"encoding",                 ENCODING},
#line 124 "TokenLookup.gperf"
      {"ascending",                ASCENDING},
      {"", ERROR},
#line 125 "TokenLookup.gperf"
      {"assign",                   ASSIGN},
#line 139 "TokenLookup.gperf"
      {"declare",                  DECLARE},
#line 162 "TokenLookup.gperf"
      {"greatest",                 GREATEST},
#line 208 "TokenLookup.gperf"
      {"then",                     THEN},
      {"", ERROR},
#line 121 "TokenLookup.gperf"
      {"ancestor-or-self",         ANCESTOR_OR_SELF},
#line 175 "TokenLookup.gperf"
      {"le",                       LE},
#line 146 "TokenLookup.gperf"
      {"document-node",            DOCUMENT_NODE},
#line 207 "TokenLookup.gperf"
      {"text",                     TEXT},
      {"", ERROR},
#line 201 "TokenLookup.gperf"
      {"schema",                   SCHEMA},
      {"", ERROR},
#line 145 "TokenLookup.gperf"
      {"document",                 DOCUMENT},
      {"", ERROR},
#line 141 "TokenLookup.gperf"
      {"descendant",               DESCENDANT},
      {"", ERROR},
#line 177 "TokenLookup.gperf"
      {"lt",                       LT},
#line 122 "TokenLookup.gperf"
      {"and",                      AND},
#line 182 "TokenLookup.gperf"
      {"node",                     NODE},
#line 174 "TokenLookup.gperf"
      {"least",                    LEAST},
#line 199 "TokenLookup.gperf"
      {"schema-attribute",         SCHEMA_ATTRIBUTE},
      {"", ERROR},
#line 155 "TokenLookup.gperf"
      {"external",                 EXTERNAL},
      {"", ERROR},
#line 143 "TokenLookup.gperf"
      {"descending",               DESCENDING},
#line 184 "TokenLookup.gperf"
      {"no-preserve",              NO_PRESERVE},
#line 140 "TokenLookup.gperf"
      {"default",                  DEFAULT},
#line 176 "TokenLookup.gperf"
      {"let",                      LET},
#line 200 "TokenLookup.gperf"
      {"schema-element",           SCHEMA_ELEMENT},
      {"", ERROR}, {"", ERROR},
#line 137 "TokenLookup.gperf"
      {"construction",             CONSTRUCTION},
#line 142 "TokenLookup.gperf"
      {"descendant-or-self",       DESCENDANT_OR_SELF},
#line 202 "TokenLookup.gperf"
      {"self",                     SELF},
#line 183 "TokenLookup.gperf"
      {"no-inherit",               NO_INHERIT},
      {"", ERROR},
#line 158 "TokenLookup.gperf"
      {"follows",                  FOLLOWS},
#line 120 "TokenLookup.gperf"
      {"ancestor",                 ANCESTOR},
      {"", ERROR}, {"", ERROR}, {"", ERROR},
#line 209 "TokenLookup.gperf"
      {"to",                       TO},
#line 160 "TokenLookup.gperf"
      {"function",                 FUNCTION},
#line 135 "TokenLookup.gperf"
      {"collation",                COLLATION},
      {"", ERROR},
#line 205 "TokenLookup.gperf"
      {"strict",                   STRICT},
      {"", ERROR},
#line 173 "TokenLookup.gperf"
      {"lax",                      LAX},
      {"", ERROR},
#line 149 "TokenLookup.gperf"
      {"empty",                    EMPTY},
      {"", ERROR},
#line 185 "TokenLookup.gperf"
      {"of",                       OF},
#line 195 "TokenLookup.gperf"
      {"preserve",                 PRESERVE},
#line 156 "TokenLookup.gperf"
      {"following",                FOLLOWING},
      {"", ERROR}, {"", ERROR},
#line 171 "TokenLookup.gperf"
      {"is",                       IS},
#line 192 "TokenLookup.gperf"
      {"precedes",                 PRECEDES},
#line 150 "TokenLookup.gperf"
      {"empty-sequence",           EMPTY_SEQUENCE},
      {"", ERROR}, {"", ERROR},
#line 157 "TokenLookup.gperf"
      {"following-sibling",        FOLLOWING_SIBLING},
#line 169 "TokenLookup.gperf"
      {"instance",                 INSTANCE},
#line 213 "TokenLookup.gperf"
      {"unordered",                UNORDERED},
#line 128 "TokenLookup.gperf"
      {"base-uri",                 BASEURI},
#line 197 "TokenLookup.gperf"
      {"return",                   RETURN},
      {"", ERROR},
#line 214 "TokenLookup.gperf"
      {"validate",                 VALIDATE},
      {"", ERROR},
#line 138 "TokenLookup.gperf"
      {"copy-namespaces",          COPY_NAMESPACES},
#line 186 "TokenLookup.gperf"
      {"option",                   OPTION},
#line 165 "TokenLookup.gperf"
      {"if",                       IF},
      {"", ERROR},
#line 193 "TokenLookup.gperf"
      {"preceding",                PRECEDING},
      {"", ERROR}, {"", ERROR},
#line 168 "TokenLookup.gperf"
      {"in",                       IN},
      {"", ERROR},
#line 170 "TokenLookup.gperf"
      {"intersect",                INTERSECT},
#line 212 "TokenLookup.gperf"
      {"union",                    UNION},
      {"", ERROR},
#line 194 "TokenLookup.gperf"
      {"preceding-sibling",        PRECEDING_SIBLING},
#line 188 "TokenLookup.gperf"
      {"ordering",                 ORDERING},
#line 203 "TokenLookup.gperf"
      {"some",                     SOME},
#line 134 "TokenLookup.gperf"
      {"child",                    CHILD},
      {"", ERROR},
#line 187 "TokenLookup.gperf"
      {"ordered",                  ORDERED},
#line 215 "TokenLookup.gperf"
      {"variable",                 VARIABLE},
      {"", ERROR}, {"", ERROR}, {"", ERROR},
#line 190 "TokenLookup.gperf"
      {"or",                       OR},
      {"", ERROR}, {"", ERROR}, {"", ERROR}, {"", ERROR},
#line 136 "TokenLookup.gperf"
      {"comment",                  COMMENT},
      {"", ERROR}, {"", ERROR},
#line 211 "TokenLookup.gperf"
      {"typeswitch",               TYPESWITCH},
      {"", ERROR},
#line 167 "TokenLookup.gperf"
      {"inherit",                  INHERIT},
#line 144 "TokenLookup.gperf"
      {"div",                      DIV},
      {"", ERROR}, {"", ERROR},
#line 179 "TokenLookup.gperf"
      {"module",                   MODULE},
      {"", ERROR},
#line 159 "TokenLookup.gperf"
      {"for",                      FOR},
#line 180 "TokenLookup.gperf"
      {"namespace",                NAMESPACE},
      {"", ERROR}, {"", ERROR},
#line 216 "TokenLookup.gperf"
      {"version",                  VERSION},
      {"", ERROR}, {"", ERROR},
#line 206 "TokenLookup.gperf"
      {"strip",                    STRIP},
      {"", ERROR}, {"", ERROR}, {"", ERROR}, {"", ERROR},
#line 189 "TokenLookup.gperf"
      {"order",                    ORDER},
#line 191 "TokenLookup.gperf"
      {"parent",                   PARENT},
      {"", ERROR}, {"", ERROR}, {"", ERROR}, {"", ERROR},
      {"", ERROR}, {"", ERROR},
#line 178 "TokenLookup.gperf"
      {"mod",                      MOD},
      {"", ERROR}, {"", ERROR}, {"", ERROR}, {"", ERROR},
      {"", ERROR}, {"", ERROR}, {"", ERROR}, {"", ERROR},
      {"", ERROR}, {"", ERROR}, {"", ERROR}, {"", ERROR},
      {"", ERROR}, {"", ERROR}, {"", ERROR}, {"", ERROR},
      {"", ERROR},
#line 166 "TokenLookup.gperf"
      {"import",                   IMPORT},
      {"", ERROR}, {"", ERROR}, {"", ERROR}, {"", ERROR},
      {"", ERROR}, {"", ERROR}, {"", ERROR}, {"", ERROR},
      {"", ERROR}, {"", ERROR},
#line 196 "TokenLookup.gperf"
      {"processing-instruction",   PROCESSING_INSTRUCTION},
      {"", ERROR}, {"", ERROR}, {"", ERROR}, {"", ERROR},
      {"", ERROR}, {"", ERROR},
#line 172 "TokenLookup.gperf"
      {"item",                     ITEM},
      {"", ERROR}, {"", ERROR}, {"", ERROR}, {"", ERROR},
      {"", ERROR}, {"", ERROR}, {"", ERROR}, {"", ERROR},
      {"", ERROR}, {"", ERROR}, {"", ERROR}, {"", ERROR},
      {"", ERROR}, {"", ERROR}, {"", ERROR}, {"", ERROR},
      {"", ERROR}, {"", ERROR}, {"", ERROR}, {"", ERROR},
      {"", ERROR}, {"", ERROR}, {"", ERROR}, {"", ERROR},
      {"", ERROR}, {"", ERROR}, {"", ERROR}, {"", ERROR},
      {"", ERROR},
#line 164 "TokenLookup.gperf"
      {"idiv",                     IDIV}
   };

   if (len <= MAX_WORD_LENGTH && len >= MIN_WORD_LENGTH) {
      int key = hash (str, len);

      if (key <= MAX_HASH_VALUE && key >= 0) {
         const char *s = wordlist[key].name;

         if (*str == *s && !strcmp (str + 1, s + 1)) {
            return &wordlist[key];
         }
      }
   }

   return nullptr;
}
#line 219 "TokenLookup.gperf"


} /* Close the QPatternist namespace. */

