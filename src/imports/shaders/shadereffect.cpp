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

#include "shadereffect.h"
#include "shadereffectbuffer.h"
#include "shadereffectsource.h"

#include <QDeclarativeItem>
#include <QPainter>
#include <QtOpenGL>

static QTransform savedWorldTransform;

ShaderEffect::ShaderEffect(QObject *parent)
    : QGraphicsEffect(parent)
    , m_changed(true)
{
}

ShaderEffect::~ShaderEffect()
{
}

void ShaderEffect::prepareBufferedDraw(QPainter *painter)
{
    // This workaround needed because QGraphicsEffect seems to always utilize default painters worldtransform
    // instead of the active painters worldtransform.
    const ShaderEffectBuffer *effectBuffer = dynamic_cast<ShaderEffectBuffer*> (painter->device());
    if (effectBuffer) {
        savedWorldTransform = painter->worldTransform() * savedWorldTransform;
        painter->setWorldTransform(savedWorldTransform);
    } else {
        savedWorldTransform = painter->worldTransform();
    }
}

void ShaderEffect::draw (QPainter *painter)
{
    const QGLContext *context = QGLContext::currentContext();

    prepareBufferedDraw(painter);

    if (context) {
        updateRenderTargets();
    }

    if (!context || m_renderTargets.count() == 0 || !hideOriginal())
        drawSource(painter);
}

void ShaderEffect::updateRenderTargets()
{
    if (!m_changed)
        return;

    m_changed = false;

    int count = m_renderTargets.count();
    for (int i = 0; i < count; i++) {
        if (m_renderTargets[i]->isLive() || m_renderTargets[i]->isDirtyTexture()) {
            m_renderTargets[i]->updateBackbuffer();
            ShaderEffectBuffer* target = m_renderTargets[i]->fbo();
            if (target && target->isValid() && target->width() > 0 && target->height() > 0) {
                QPainter p(target);
                p.setCompositionMode(QPainter::CompositionMode_Clear);
                p.fillRect(QRect(QPoint(0, 0), target->size()), Qt::transparent);
                p.setCompositionMode(QPainter::CompositionMode_SourceOver);

                QRectF sourceRect = m_renderTargets[i]->sourceRect();
                QSize textureSize = m_renderTargets[i]->textureSize();

                qreal yflip = m_renderTargets[i]->isMirrored() ? -1.0 : 1.0; // flip y to match scenegraph, it also flips texturecoordinates
                qreal xscale = 1.0;
                qreal yscale = 1.0 * yflip;

                qreal leftMargin = 0.0;
                qreal rightMargin = 0.0;
                qreal topMargin = 0.0;
                qreal bottomMargin = 0.0;

                qreal width = m_renderTargets[i]->sourceItem()->width();
                qreal height = m_renderTargets[i]->sourceItem()->height();

                if (!sourceRect.isEmpty()) {
                    leftMargin = -sourceRect.left();
                    rightMargin = sourceRect.right() - width;
                    topMargin = -sourceRect.top();
                    bottomMargin = sourceRect.bottom() - height;
                }

                if ((width + leftMargin + rightMargin) > 0 && (height + topMargin + bottomMargin) > 0) {
                    if (!textureSize.isEmpty()) {
                        qreal textureWidth = textureSize.width();
                        qreal textureHeight = textureSize.height();

                        xscale = width / (width + leftMargin + rightMargin);
                        yscale = height / (height + topMargin + bottomMargin);

                        p.translate(textureWidth / 2, textureHeight / 2);
                        p.scale(xscale, yscale * yflip);
                        p.translate(-textureWidth / 2, -textureHeight / 2);
                        p.scale(textureWidth / width, textureHeight / height);
                    } else {
                        xscale = width / (width + leftMargin + rightMargin);
                        yscale = height / (height + topMargin + bottomMargin);

                        p.translate(width / 2, height / 2);
                        p.scale(xscale, yscale * yflip);
                        p.translate(-width / 2, -height / 2);
                    }
                }

                drawSource(&p);
                p.end();
                m_renderTargets[i]->markSceneGraphDirty();
            }
        }
    }
}

void ShaderEffect::sourceChanged (ChangeFlags flags)
{
    Q_UNUSED(flags);
    m_changed = true;
}

void ShaderEffect::addRenderTarget(ShaderEffectSource *target)
{
    if (!m_renderTargets.contains(target))
        m_renderTargets.append(target);
}

void ShaderEffect::removeRenderTarget(ShaderEffectSource *target)
{
    int index = m_renderTargets.indexOf(target);
    if (index >= 0)
        m_renderTargets.remove(index);
    else
        qWarning() << "ShaderEffect::removeRenderTarget - did not find target.";
}

bool ShaderEffect::hideOriginal() const
{
    if (m_renderTargets.count() == 0)
        return false;

    // Just like scenegraph version, if there is even one source that says "hide original" we hide it.
    int count = m_renderTargets.count();
    for (int i = 0; i < count; i++) {
        if (m_renderTargets[i]->hideSource())
            return true;
    }
    return false;
}
