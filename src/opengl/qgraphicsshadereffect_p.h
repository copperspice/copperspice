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

#ifndef QGRAPHICSSHADEREFFECT_P_H
#define QGRAPHICSSHADEREFFECT_P_H

#include <qgraphicseffect.h>

#ifndef QT_NO_GRAPHICSEFFECT

class QGLShaderProgram;
class QGLCustomShaderEffectStage;
class QGraphicsShaderEffectPrivate;

class Q_OPENGL_EXPORT QGraphicsShaderEffect : public QGraphicsEffect
{
   OPENGL_CS_OBJECT(QGraphicsShaderEffect)

 public:
   QGraphicsShaderEffect(QObject *parent = nullptr);

   QGraphicsShaderEffect(const QGraphicsShaderEffect &) = delete;
   QGraphicsShaderEffect &operator=(const QGraphicsShaderEffect &) = delete;

   virtual ~QGraphicsShaderEffect();

   QByteArray pixelShaderFragment() const;
   void setPixelShaderFragment(const QByteArray &code);

 protected:
   void draw(QPainter *painter) override;
   void setUniformsDirty();
   virtual void setUniforms(QGLShaderProgram *program);

 private:
   Q_DECLARE_PRIVATE(QGraphicsShaderEffect)

   friend class QGLCustomShaderEffectStage;
};

#endif // QT_NO_GRAPHICSEFFECT

#endif
