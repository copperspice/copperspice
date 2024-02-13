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

#ifndef QPODLIST_P_H
#define QPODLIST_P_H

#include <qvarlengtharray.h>

template <typename T, int Prealloc>
class QPodList : public QVarLengthArray<T, Prealloc>
{
   using QVarLengthArray<T, Prealloc>::s;
   using QVarLengthArray<T, Prealloc>::a;
   using QVarLengthArray<T, Prealloc>::ptr;
   using QVarLengthArray<T, Prealloc>::realloc;

 public:
   explicit QPodList(int size = 0)
      : QVarLengthArray<T, Prealloc>(size)
   {
   }

   void insert(int idx, const T &t) {
      const int sz = s++;

      if (s == a) {
         realloc(s, s << 1);
      }

      ::memmove(ptr + idx + 1, ptr + idx, (sz - idx) * sizeof(T));
      ptr[idx] = t;
   }

   void removeAll(const T &t) {
      int i = 0;

      for (int j = 0; j < s; ++j) {
         if (ptr[j] != t) {
            ptr[i++] = ptr[j];
         }
      }

      s = i;
   }

   void removeAt(int idx) {
      Q_ASSERT(idx >= 0 && idx < s);
      ::memmove(ptr + idx, ptr + idx + 1, (s - idx - 1) * sizeof(T));
      --s;
   }

   T takeFirst() {
      Q_ASSERT(s > 0);
      T tmp = ptr[0];
      removeAt(0);
      return tmp;
   }
};

#endif // QPODLIST_P_H
