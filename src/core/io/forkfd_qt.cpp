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

/****************************************************************************
**
** Copyright (C) 2014 Intel Corporation
**
****************************************************************************/

#include <atomic>
#include <qprocess_p.h>

#define FORKFD_NO_SPAWNFD

using ffd_atomic_int = std::atomic<int>;

#define ffd_atomic_pointer(type)    std::atomic<type *>
#define FFD_ATOMIC_INIT(val)        ATOMIC_VAR_INIT(val)

#define FFD_ATOMIC_RELAXED          std::memory_order_relaxed
#define FFD_ATOMIC_ACQUIRE          std::memory_order_acquire
#define FFD_ATOMIC_RELEASE          std::memory_order_release
#define loadRelaxed                 load
#define storeRelaxed                store

#define FFD_CONCAT(x, y)            x ## y

#define ffd_atomic_load(ptr,order)          (ptr)->load(order)
#define ffd_atomic_store(ptr,val,order)     (ptr)->store(val, order)
#define ffd_atomic_exchange(ptr,val,order)  (ptr)->exchange(val, order)
#define ffd_atomic_add_fetch(ptr,val,order) ((ptr)->fetch_add(val, order) + val)

#define ffd_atomic_compare_exchange(ptr,expected,desired,order1,order2) \
                                            (ptr)->compare_exchange_strong(*expected, desired, order1, order2)

extern "C" {
#include "../../3rdparty/forkfd/forkfd.c"
}
