/***********************************************************************
*
* Copyright (c) 2012-2017 Barbara Geller
* Copyright (c) 2012-2017 Ansel Sermersheim
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

#ifndef QSTRINGVIEW8_H
#define QSTRINGVIEW8_H

#include <cs_string_view.h>

class Q_CORE_EXPORT QStringView8 : public CsString::CsStringView
{
   public:
      QStringView8() = default;

      QStringView8(const_iterator begin, const_iterator end)
         : CsString::CsStringView(begin, end)
      { }

      // methods


      // iterators
      iterator begin() {
         return CsString::CsStringView::begin();
      }

      const_iterator begin() const {
         return CsString::CsStringView::cbegin();
      }

      const_iterator cbegin() const {
         return CsString::CsStringView::cbegin();
      }

      const_iterator constBegin() const {
         return CsString::CsStringView::cbegin();
      }

      iterator end() {
         return CsString::CsStringView::end();
      }

      const_iterator end() const {
         return CsString::CsStringView::cend();
      }

      const_iterator cend() const {
         return CsString::CsStringView::cend();
      }

      const_iterator constEnd() const {
         return CsString::CsStringView::cend();
      }
};

#endif