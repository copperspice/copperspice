/***********************************************************************
*
* Copyright (c) 2012-2014 Barbara Geller
* Copyright (c) 2012-2014 Ansel Sermersheim
* Copyright (c) 2012-2014 Digia Plc and/or its subsidiary(-ies).
* Copyright (c) 2008-2012 Nokia Corporation and/or its subsidiary(-ies).
* All rights reserved.
*
* This file is part of CopperSpice.
*
* CopperSpice is free software: you can redistribute it and/or 
* modify it under the terms of the GNU Lesser General Public License
* version 2.1 as published by the Free Software Foundation.
*
* CopperSpice is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
* Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public
* License along with CopperSpice.  If not, see 
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
#  ifdef MAKE_PHONON_LIB /* We are building this library */
#   define PHONON_EXPORT Q_DECL_EXPORT
#  else /* We are using this library */
#   define PHONON_EXPORT Q_DECL_IMPORT
#  endif
# else /* UNIX */
#  ifdef MAKE_PHONON_LIB /* We are building this library */
#   define PHONON_EXPORT Q_DECL_EXPORT
#  else /* We are using this library */
#   define PHONON_EXPORT Q_DECL_IMPORT
#  endif
# endif
#endif

#ifndef PHONON_EXPORT_DEPRECATED
# define PHONON_EXPORT_DEPRECATED Q_DECL_DEPRECATED PHONON_EXPORT
#endif

// QT_(BEGIN|END)_NAMESPACE appeared in 4.4
#ifndef QT_BEGIN_NAMESPACE
#  define QT_BEGIN_NAMESPACE
#endif
#ifndef QT_END_NAMESPACE
#  define QT_END_NAMESPACE
#endif

#endif
