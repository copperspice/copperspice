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

#ifndef QToCodepointsIterator_P_H
#define QToCodepointsIterator_P_H

#include <qitem_p.h>

namespace QPatternist {

class ToCodepointsIterator : public Item::Iterator
{
 public:
   /**
    * Constructs a ToCodepointsIterator.
    *
    * @param string the string to retrieve Unicode codepoints from. Can not be
    * empty.
    */
   ToCodepointsIterator(const QString &string);
   Item next() override;
   Item current() const override;
   xsInteger position() const override;
   xsInteger count() override;
   Item::Iterator::Ptr copy() const override;

 private:
   const QString m_string;
   const int m_len;
   Item m_current;
   xsInteger m_position;
};

}

#endif
