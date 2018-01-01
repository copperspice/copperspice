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

#ifndef SIMTEXTH_H
#define SIMTEXTH_H

const int textSimilarityThreshold = 190;

#include <QString>
#include <QList>

QT_BEGIN_NAMESPACE

class Translator;

struct Candidate {
   Candidate() {}
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

typedef QList<Candidate> CandidateList;

struct CoMatrix;
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
   ~StringSimilarityMatcher();
   int getSimilarityScore(const QString &strCandidate);

 private:
   CoMatrix *m_cm;
   int m_length;
};

int getSimilarityScore(const QString &str1, const QString &str2);

CandidateList similarTextHeuristicCandidates( const Translator *tor,
      const QString &text,
      int maxCandidates );

QT_END_NAMESPACE

#endif
