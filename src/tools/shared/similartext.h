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

#ifndef SIMILARTEXT_H
#define SIMILARTEXT_H

#include <qlist.h>
#include <qstring.h>

class Translator;

constexpr const int textSimilarityThreshold = 190;

struct Candidate {
   Candidate() {
   }

   Candidate(const QString &source0, const QString &target0)
      : source(source0), target(target0) {
   }

   QString source;
   QString target;
};

inline bool operator==( const Candidate &c, const Candidate &d )
{
   return c.target == d.target && c.source == d.source;
}

inline bool operator!=( const Candidate &c, const Candidate &d )
{
   return !operator==( c, d );
}

struct CoMatrix {
   CoMatrix(const QString &str);
   CoMatrix() {}

   /*
     The matrix has 20 * 20 = 400 entries.  This requires 50 bytes, or 13
     words.  Some operations are performed on words for more efficiency.
   */
   union {
      quint8 b[52];
      quint32 w[13];
   };
};
/**
 * This class is more efficient for searching through a large array of candidate strings, since we only
 * have to construct the CoMatrix for the \a stringToMatch once,
 * after that we just call getSimilarityScore(strCandidate).
 * \sa getSimilarityScore
 */
class StringSimilarityMatcher
{
 public:
   StringSimilarityMatcher(const QString &stringToMatch);

   int getSimilarityScore(const QString &strCandidate);

 private:
   CoMatrix m_cm;
   int m_length;
};

static inline int getSimilarityScore(const QString &str1, const QString &str2)
{
   return StringSimilarityMatcher(str1).getSimilarityScore(str2);
}

QList<Candidate> similarTextHeuristicCandidates(const Translator *tor, const QString &text, int maxCandidates);

#endif
