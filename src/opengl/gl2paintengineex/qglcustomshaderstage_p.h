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

#ifndef QGLCUSTOMSHADERSTAGE_P_H
#define QGLCUSTOMSHADERSTAGE_P_H

#include <QGLShaderProgram>

class QGLCustomShaderStagePrivate;

class Q_OPENGL_EXPORT QGLCustomShaderStage
{
   Q_DECLARE_PRIVATE(QGLCustomShaderStage)

 public:
   QGLCustomShaderStage();
   virtual ~QGLCustomShaderStage();

   virtual void setUniforms(QGLShaderProgram *) {}
   void setUniformsDirty();

   bool setOnPainter(QPainter *);
   void removeFromPainter(QPainter *);
   QString source() const;

   void setInactive();

 protected:
   void setSource(const QString &);

 private:
   QGLCustomShaderStagePrivate *d_ptr;
};


#endif
