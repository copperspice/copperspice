/***********************************************************************
*
* Copyright (c) 2012-2019 Barbara Geller
* Copyright (c) 2012-2019 Ansel Sermersheim
*
* Copyright (C) 2015 The Qt Company Ltd.
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

#ifndef QGRAPHICSSHADEREFFECT_P_H
#define QGRAPHICSSHADEREFFECT_P_H

#include <QtGui/qgraphicseffect.h>

QT_BEGIN_NAMESPACE

class QGLShaderProgram;
class QGLCustomShaderEffectStage;
class QGraphicsShaderEffectPrivate;

class Q_OPENGL_EXPORT QGraphicsShaderEffect : public QGraphicsEffect
{
   OPENGL_CS_OBJECT(QGraphicsShaderEffect)

 public:
   QGraphicsShaderEffect(QObject *parent = nullptr);
   virtual ~QGraphicsShaderEffect();

   QByteArray pixelShaderFragment() const;
   void setPixelShaderFragment(const QByteArray &code);

 protected:
   void draw(QPainter *painter) override;
   void setUniformsDirty();
   virtual void setUniforms(QGLShaderProgram *program);

 private:
   Q_DECLARE_PRIVATE(QGraphicsShaderEffect)
   Q_DISABLE_COPY(QGraphicsShaderEffect)

   friend class QGLCustomShaderEffectStage;
};

QT_END_NAMESPACE

#endif // QGRAPHICSSHADEREFFECT_P_H
