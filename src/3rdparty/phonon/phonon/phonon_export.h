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

/********************************************************
**  Copyright (C) 2005-2006 Matthias Kretz <kretz@kde.org
********************************************************/

#ifndef PHONON_PHONON_EXPORT_H
#define PHONON_PHONON_EXPORT_H

#include <QtCore/QtGlobal>

#ifndef PHONON_EXPORT

# if defined Q_OS_WIN

#  ifdef MAKE_PHONON_LIB 
#    define PHONON_EXPORT    Q_DECL_EXPORT
#  else 
#    define PHONON_EXPORT    Q_DECL_IMPORT
#  endif

# else 

#  ifdef MAKE_PHONON_LIB 
#    define PHONON_EXPORT    Q_DECL_EXPORT
#  else 
#    define PHONON_EXPORT    Q_DECL_IMPORT
#  endif

# endif

#endif

#ifndef QT_BEGIN_NAMESPACE
#  define QT_BEGIN_NAMESPACE
#endif

#ifndef QT_END_NAMESPACE
#  define QT_END_NAMESPACE
#endif

#endif
