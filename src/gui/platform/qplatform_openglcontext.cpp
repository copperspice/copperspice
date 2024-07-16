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

#include <qplatform_openglcontext.h>

#include <QOpenGLFunctions>

class QPlatformOpenGLContextPrivate
{
 public:
   QPlatformOpenGLContextPrivate()
      : context(nullptr)
   {
   }

   QOpenGLContext *context;
};

QPlatformOpenGLContext::QPlatformOpenGLContext()
   : d_ptr(new QPlatformOpenGLContextPrivate)
{
}

QPlatformOpenGLContext::~QPlatformOpenGLContext()
{
}

void QPlatformOpenGLContext::initialize()
{
}

GLuint QPlatformOpenGLContext::defaultFramebufferObject(QPlatformSurface *) const
{
   return 0;
}

QOpenGLContext *QPlatformOpenGLContext::context() const
{
   Q_D(const QPlatformOpenGLContext);
   return d->context;
}

void QPlatformOpenGLContext::setContext(QOpenGLContext *context)
{
   Q_D(QPlatformOpenGLContext);
   d->context = context;
}

bool QPlatformOpenGLContext::parseOpenGLVersion(const QByteArray &versionString, int &major, int &minor)
{
   bool majorOk = false;
   bool minorOk = false;

   QList<QByteArray> parts = versionString.split(' ');

   if (versionString.startsWith("OpenGL ES")) {
      if (parts.size() >= 3) {
         QList<QByteArray> versionParts = parts.at(2).split('.');

         if (versionParts.size() >= 2) {
            major = versionParts.at(0).toInt(&majorOk);
            minor = versionParts.at(1).toInt(&minorOk);

            // Nexus 6 has "OpenGL ES 3.0V@95.0 (GIT@I86da836d38)"
            if (! minorOk) {
               if (int idx = versionParts.at(1).indexOf('V')) {
                  minor = versionParts.at(1).left(idx).toInt(&minorOk);
               }
            }

         } else {
            qWarning("QPlatformOpenGLContext::parseOpenGLVersion() Unrecognized OpenGL ES version");
         }

      } else {
         // If < 3 parts to the name, it is an unrecognised OpenGL ES
         qWarning("QPlatformOpenGLContext::parseOpenGLVersion() Unknown OpenGL ES version");
      }

   } else {
      // Not OpenGL ES, but regular OpenGL the version numbers are first in the string
      QList<QByteArray> versionParts = parts.at(0).split('.');

      if (versionParts.size() >= 2) {
         major = versionParts.at(0).toInt(&majorOk);
         minor = versionParts.at(1).toInt(&minorOk);

      } else {
         qWarning("QPlatformOpenGLContext::parseOpenGLVersion() Unrecognized OpenGL version");
      }
   }

   if (! majorOk || ! minorOk) {
      qWarning("QPlatformOpenGLContext::parseOpenGLVersion() Unknown OpenGL version");
   }

   return (majorOk && minorOk);
}
