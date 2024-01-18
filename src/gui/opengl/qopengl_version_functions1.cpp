/***********************************************************************
*
* Copyright (c) 2012-2024 Barbara Geller
* Copyright (c) 2012-2024 Ansel Sermersheim
*
* Copyright (c) 2013 Klarälvdalens Datakonsult AB, a KDAB Group company
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

#include <qopengl_version_functions_p.h>

#if ! defined(QT_OPENGL_ES_2)
#include "qopenglfunctions_4_5_core.h"
#include "qopenglfunctions_4_5_compatibility.h"
#include "qopenglfunctions_4_4_core.h"
#include "qopenglfunctions_4_4_compatibility.h"
#include "qopenglfunctions_4_3_core.h"
#include "qopenglfunctions_4_3_compatibility.h"
#include "qopenglfunctions_4_2_core.h"
#include "qopenglfunctions_4_2_compatibility.h"
#include "qopenglfunctions_4_1_core.h"
#include "qopenglfunctions_4_1_compatibility.h"
#include "qopenglfunctions_4_0_core.h"
#include "qopenglfunctions_4_0_compatibility.h"
#include "qopenglfunctions_3_3_core.h"
#include "qopenglfunctions_3_3_compatibility.h"
#include "qopenglfunctions_3_2_core.h"
#include "qopenglfunctions_3_2_compatibility.h"

#else
#include "qopenglfunctions_es2.h"

#endif

QAbstractOpenGLFunctions *QOpenGLVersionFunctionsFactory::create(const QOpenGLVersionProfile &versionProfile)
{
#if !defined(QT_OPENGL_ES_2)
   const int major = versionProfile.version().first;
   const int minor = versionProfile.version().second;

   if (versionProfile.hasProfiles()) {

      switch (versionProfile.profile()) {

        case QSurfaceFormat::CoreProfile:
            if (major == 4 && minor == 5)
                return new QOpenGLFunctions_4_5_Core;
            else if (major == 4 && minor == 4)
                return new QOpenGLFunctions_4_4_Core;
            else if (major == 4 && minor == 3)
                return new QOpenGLFunctions_4_3_Core;
            else if (major == 4 && minor == 2)
                return new QOpenGLFunctions_4_2_Core;
            else if (major == 4 && minor == 1)
                return new QOpenGLFunctions_4_1_Core;
            else if (major == 4 && minor == 0)
                return new QOpenGLFunctions_4_0_Core;
            else if (major == 3 && minor == 3)
                return new QOpenGLFunctions_3_3_Core;
            else if (major == 3 && minor == 2)
                return new QOpenGLFunctions_3_2_Core;
            break;

        case QSurfaceFormat::CompatibilityProfile:
            if (major == 4 && minor == 5)
                return new QOpenGLFunctions_4_5_Compatibility;
            else if (major == 4 && minor == 4)
                return new QOpenGLFunctions_4_4_Compatibility;
            else if (major == 4 && minor == 3)
                return new QOpenGLFunctions_4_3_Compatibility;
            else if (major == 4 && minor == 2)
                return new QOpenGLFunctions_4_2_Compatibility;
            else if (major == 4 && minor == 1)
                return new QOpenGLFunctions_4_1_Compatibility;
            else if (major == 4 && minor == 0)
                return new QOpenGLFunctions_4_0_Compatibility;
            else if (major == 3 && minor == 3)
                return new QOpenGLFunctions_3_3_Compatibility;
            else if (major == 3 && minor == 2)
                return new QOpenGLFunctions_3_2_Compatibility;
            break;

        case QSurfaceFormat::NoProfile:

        default:
            break;
        };

   } else {
      return createNoProfile(major, minor);

   }

   return nullptr;

#else
    return new QOpenGLFunctions_ES2;

#endif
}

