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

#include "qglcustomshaderstage_p.h"
#include "qglengineshadermanager_p.h"
#include "qpaintengineex_opengl2_p.h"
#include <qpainter_p.h>

QT_BEGIN_NAMESPACE

class QGLCustomShaderStagePrivate
{
 public:
   QGLCustomShaderStagePrivate() :
      m_manager(0) {}

   QPointer<QGLEngineShaderManager> m_manager;
   QByteArray              m_source;
};




QGLCustomShaderStage::QGLCustomShaderStage()
   : d_ptr(new QGLCustomShaderStagePrivate)
{
}

QGLCustomShaderStage::~QGLCustomShaderStage()
{
   Q_D(QGLCustomShaderStage);
   if (d->m_manager) {
      d->m_manager->removeCustomStage();
      d->m_manager->sharedShaders->cleanupCustomStage(this);
   }
}

void QGLCustomShaderStage::setUniformsDirty()
{
   Q_D(QGLCustomShaderStage);
   if (d->m_manager) {
      d->m_manager->setDirty();   // ### Probably a bit overkill!
   }
}

bool QGLCustomShaderStage::setOnPainter(QPainter *p)
{
   Q_D(QGLCustomShaderStage);
   if (p->paintEngine()->type() != QPaintEngine::OpenGL2) {
      qWarning("QGLCustomShaderStage::setOnPainter() - paint engine not OpenGL2");
      return false;
   }
   if (d->m_manager) {
      qWarning("Custom shader is already set on a painter");
   }

   QGL2PaintEngineEx *engine = static_cast<QGL2PaintEngineEx *>(p->paintEngine());
   d->m_manager = QGL2PaintEngineExPrivate::shaderManagerForEngine(engine);
   Q_ASSERT(d->m_manager);

   d->m_manager->setCustomStage(this);
   return true;
}

void QGLCustomShaderStage::removeFromPainter(QPainter *p)
{
   Q_D(QGLCustomShaderStage);
   if (p->paintEngine()->type() != QPaintEngine::OpenGL2) {
      return;
   }

   QGL2PaintEngineEx *engine = static_cast<QGL2PaintEngineEx *>(p->paintEngine());
   d->m_manager = QGL2PaintEngineExPrivate::shaderManagerForEngine(engine);
   Q_ASSERT(d->m_manager);

   // Just set the stage to null, don't call removeCustomStage().
   // This should leave the program in a compiled/linked state
   // if the next custom shader stage is this one again.
   d->m_manager->setCustomStage(0);
   d->m_manager = 0;
}

QByteArray QGLCustomShaderStage::source() const
{
   Q_D(const QGLCustomShaderStage);
   return d->m_source;
}

// Called by the shader manager if another custom shader is attached or
// the manager is deleted
void QGLCustomShaderStage::setInactive()
{
   Q_D(QGLCustomShaderStage);
   d->m_manager = 0;
}

void QGLCustomShaderStage::setSource(const QByteArray &s)
{
   Q_D(QGLCustomShaderStage);
   d->m_source = s;
}

QT_END_NAMESPACE
