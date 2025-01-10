/***********************************************************************
*
* Copyright (c) 2023-2025 Barbara Geller
* Copyright (c) 2023-2025 Ansel Sermersheim
*
* This file is part of CsPointer.
*
* CsPointer is free software which is released under the BSD 2-Clause license.
* For license details refer to the LICENSE provided with this project.
*
* CsPointer is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
*
* https://opensource.org/licenses/BSD-2-Clause
*
***********************************************************************/

#ifndef LIB_CS_POINTER_TRAITS_H
#define LIB_CS_POINTER_TRAITS_H

namespace CsPointer {

template <typename T>
struct cs_add_missing_extent {
   using type = T[];
};

template <typename T>
struct cs_add_missing_extent<T[]> {
   using type = T[];
};

template <typename T>
using cs_add_missing_extent_t = typename cs_add_missing_extent<T>::type;

}   // end namespace

#endif
