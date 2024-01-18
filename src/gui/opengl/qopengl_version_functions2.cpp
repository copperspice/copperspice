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
#include "qopenglfunctions_3_1.h"
#include "qopenglfunctions_3_0.h"
#include "qopenglfunctions_2_1.h"
#include "qopenglfunctions_2_0.h"
#include "qopenglfunctions_1_5.h"
#include "qopenglfunctions_1_4.h"
#include "qopenglfunctions_1_3.h"
#include "qopenglfunctions_1_2.h"
#include "qopenglfunctions_1_1.h"
#include "qopenglfunctions_1_0.h"
#endif

QAbstractOpenGLFunctions *QOpenGLVersionFunctionsFactory::createNoProfile(const int major, const int minor)
{
   if (major == 3 && minor == 1)
      return new QOpenGLFunctions_3_1;

   else if (major == 3 && minor == 0)
      return new QOpenGLFunctions_3_0;

   else if (major == 2 && minor == 1)
      return new QOpenGLFunctions_2_1;

   else if (major == 2 && minor == 0)
      return new QOpenGLFunctions_2_0;

   else if (major == 1 && minor == 5)
      return new QOpenGLFunctions_1_5;

   else if (major == 1 && minor == 4)
      return new QOpenGLFunctions_1_4;

   else if (major == 1 && minor == 3)
      return new QOpenGLFunctions_1_3;

   else if (major == 1 && minor == 2)
      return new QOpenGLFunctions_1_2;

   else if (major == 1 && minor == 1)
      return new QOpenGLFunctions_1_1;

   else if (major == 1 && minor == 0)
      return new QOpenGLFunctions_1_0;

   return nullptr;
}
