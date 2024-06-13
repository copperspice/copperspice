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

#include <similartext.h>

#include <translator.h>

#include <qbytearray.h>
#include <qlist.h>
#include <qstring.h>

/*
  How similar are two texts?  The approach used here relies on co-occurrence
  matrices and is very efficient.

  Let's see with an example: how similar are "here" and "hither"?  The
  co-occurrence matrix M for "here" is M[h,e] = 1, M[e,r] = 1, M[r,e] = 1, and 0
  elsewhere; the matrix N for "hither" is N[h,i] = 1, N[i,t] = 1, ...,
  N[h,e] = 1, N[e,r] = 1, and 0 elsewhere.  The union U of both matrices is the
  matrix U[i,j] = max { M[i,j], N[i,j] }, and the intersection V is
  V[i,j] = min { M[i,j], N[i,j] }.  The score for a pair of texts is

      score = (sum of V[i,j] over all i, j) / (sum of U[i,j] over all i, j),

  a formula suggested by Arnt Gulbrandsen.  Here we have

      score = 2 / 6,

  or one third.

  The implementation differs from this in a few details.  Most importantly,
  repetitions are ignored; for input "xxx", M[x,x] equals 1, not 2.
*/

/*
  Every character is assigned to one of 20 buckets so that the co-occurrence
  matrix requires only 20 * 20 = 400 bits, not 256 * 256 = 65536 bits or even
  more if we want the whole Unicode.  Which character falls in which bucket is
  arbitrary.

  The second half of the table is a replica of the first half, because of
  laziness.
*/
static const int indexOf[256] = {
   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
   //      !   "   #   $   %   &   '   (   )   *   +   ,   -   .   /
   0,  2,  6,  7,  10, 12, 15, 19, 2,  6,  7,  10, 12, 15, 19, 0,
   //  0   1   2   3   4   5   6   7   8   9   :   ;   <   =   >   ?
   1,  3,  4,  5,  8,  9,  11, 13, 14, 16, 2,  6,  7,  10, 12, 15,
   //  @   A   B   C   D   E   F   G   H   I   J   K   L   M   N   O
   0,  1,  2,  3,  4,  5,  6,  7,  8,  9,  6,  10, 11, 12, 13, 14,
   //  P   Q   R   S   T   U   V   W   X   Y   Z   [   \   ]   ^   _
   15, 12, 16, 17, 18, 19, 2,  10, 15, 7,  19, 2,  6,  7,  10, 0,
   //  `   a   b   c   d   e   f   g   h   i   j   k   l   m   n   o
   0,  1,  2,  3,  4,  5,  6,  7,  8,  9,  6,  10, 11, 12, 13, 14,
   //  p   q   r   s   t   u   v   w   x   y   z   {   |   }   ~
   15, 12, 16, 17, 18, 19, 2,  10, 15, 7,  19, 2,  6,  7,  10, 0,

   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
   0,  2,  6,  7,  10, 12, 15, 19, 2,  6,  7,  10, 12, 15, 19, 0,
   1,  3,  4,  5,  8,  9,  11, 13, 14, 16, 2,  6,  7,  10, 12, 15,
   0,  1,  2,  3,  4,  5,  6,  7,  8,  9,  6,  10, 11, 12, 13, 14,
   15, 12, 16, 17, 18, 19, 2,  10, 15, 7,  19, 2,  6,  7,  10, 0,
   0,  1,  2,  3,  4,  5,  6,  7,  8,  9,  6,  10, 11, 12, 13, 14,
   15, 12, 16, 17, 18, 19, 2,  10, 15, 7,  19, 2,  6,  7,  10, 0
};

/*
  The entry bitCount[i] (for i between 0 and 255) is the number of bits used to
  represent i in binary.
*/
static const int bitCount[256] = {
   0,  1,  1,  2,  1,  2,  2,  3,  1,  2,  2,  3,  2,  3,  3,  4,
   1,  2,  2,  3,  2,  3,  3,  4,  2,  3,  3,  4,  3,  4,  4,  5,
   1,  2,  2,  3,  2,  3,  3,  4,  2,  3,  3,  4,  3,  4,  4,  5,
   2,  3,  3,  4,  3,  4,  4,  5,  3,  4,  4,  5,  4,  5,  5,  6,
   1,  2,  2,  3,  2,  3,  3,  4,  2,  3,  3,  4,  3,  4,  4,  5,
   2,  3,  3,  4,  3,  4,  4,  5,  3,  4,  4,  5,  4,  5,  5,  6,
   2,  3,  3,  4,  3,  4,  4,  5,  3,  4,  4,  5,  4,  5,  5,  6,
   3,  4,  4,  5,  4,  5,  5,  6,  4,  5,  5,  6,  5,  6,  6,  7,
   1,  2,  2,  3,  2,  3,  3,  4,  2,  3,  3,  4,  3,  4,  4,  5,
   2,  3,  3,  4,  3,  4,  4,  5,  3,  4,  4,  5,  4,  5,  5,  6,
   2,  3,  3,  4,  3,  4,  4,  5,  3,  4,  4,  5,  4,  5,  5,  6,
   3,  4,  4,  5,  4,  5,  5,  6,  4,  5,  5,  6,  5,  6,  6,  7,
   2,  3,  3,  4,  3,  4,  4,  5,  3,  4,  4,  5,  4,  5,  5,  6,
   3,  4,  4,  5,  4,  5,  5,  6,  4,  5,  5,  6,  5,  6,  6,  7,
   3,  4,  4,  5,  4,  5,  5,  6,  4,  5,  5,  6,  5,  6,  6,  7,
   4,  5,  5,  6,  5,  6,  6,  7,  5,  6,  6,  7,  6,  7,  7,  8
};

static inline void setCoOccurence(CoMatrix &m, char c, char d)
{
   int k = indexOf[(uchar) c] + 20 * indexOf[(uchar) d];
   m.b[k >> 3] |= (1 << (k & 0x7));
}

CoMatrix::CoMatrix(const QString &str)
{
   QByteArray ba = str.toUtf8();
   const char *text = ba.constData();
   char c = '\0', d;
   memset( b, 0, 52 );

   /*
     The Knuth books are not in the office only for show; they help make
     loops 30% faster and 20% as readable.
   */

   while ( (d = *text) != '\0' ) {
      setCoOccurence(*this, c, d);

      if ( (c = *++text) != '\0' ) {
         setCoOccurence(*this, d, c);
         text++;
      }
   }
}


static inline int worth(const CoMatrix &m)
{
   int w = 0;

   for ( int i = 0; i < 50; i++ ) {
      w += bitCount[m.b[i]];
   }

   return w;
}

static inline CoMatrix reunion(const CoMatrix &m, const CoMatrix &n)
{
   CoMatrix p;

   for (int i = 0; i < 13; ++i) {
      p.w[i] = m.w[i] | n.w[i];
   }

   return p;
}

static inline CoMatrix intersection(const CoMatrix &m, const CoMatrix &n)
{
   CoMatrix p;

   for (int i = 0; i < 13; ++i) {
      p.w[i] = m.w[i] & n.w[i];
   }

   return p;
}

StringSimilarityMatcher::StringSimilarityMatcher(const QString &stringToMatch)
   : m_cm(stringToMatch)
{
   m_length = stringToMatch.length();
}

int StringSimilarityMatcher::getSimilarityScore(const QString &strCandidate)
{
   CoMatrix cmTarget(strCandidate);
   int delta = qAbs(m_length - strCandidate.size());

   int score = ( (worth(intersection(m_cm, cmTarget)) + 1) << 10 ) /
               ( worth(reunion(m_cm, cmTarget)) + (delta << 1) + 1 );

   return score;
}

QList<Candidate> similarTextHeuristicCandidates(const Translator *tor, const QString &text, int maxCandidates)
{
   QList<int> scores;

   QList<Candidate> candidates;
   StringSimilarityMatcher matcher(text);

   for (const TranslatorMessage &mtm : tor->messages()) {
      if (mtm.type() == TranslatorMessage::Type::Unfinished || mtm.translation().isEmpty()) {
         continue;
      }

      QString s = mtm.sourceText();
      int score = matcher.getSimilarityScore(s);

      if (candidates.size() == maxCandidates && score > scores[maxCandidates - 1] ) {
         candidates.removeLast();
      }

      if (candidates.size() < maxCandidates && score >= textSimilarityThreshold) {
         Candidate cand(s, mtm.translation() );

         int i;
         for (i = 0; i < candidates.size(); i++) {
            if (score >= scores.at(i)) {
               if (score == scores.at(i)) {
                  if (candidates.at(i) == cand) {
                     goto continue_outer_loop;
                  }

               } else {
                  break;
               }
            }
         }
         scores.insert(i, score);
         candidates.insert(i, cand);
      }

   continue_outer_loop:
      ;
   }

   return candidates;
}
