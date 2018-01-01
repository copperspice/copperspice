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

#ifndef QSourceLocationReflection_P_H
#define QSourceLocationReflection_P_H

QT_BEGIN_NAMESPACE

class QString;

namespace QPatternist {
class SourceLocationReflection
{
 public:
   inline SourceLocationReflection() {
   }

   virtual ~SourceLocationReflection() {
   }

   virtual const SourceLocationReflection *actualReflection() const = 0;

   /**
    * A description of what represents the source code location, for
    * human consumption. Must be translated, as appropriate.
    */
   virtual QString description() const {
      return QString();
   }

   virtual QSourceLocation sourceLocation() const;

 private:
   Q_DISABLE_COPY(SourceLocationReflection)
};

class DelegatingSourceLocationReflection : public SourceLocationReflection
{
 public:
   inline DelegatingSourceLocationReflection(const SourceLocationReflection *const r) : m_r(r) {
      Q_ASSERT(r);
   }

   const SourceLocationReflection *actualReflection() const override;
   QString description() const override;

 private:
   const SourceLocationReflection *const m_r;
};

}

QT_END_NAMESPACE

#endif
