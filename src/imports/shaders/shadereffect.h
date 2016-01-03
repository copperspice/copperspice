/***********************************************************************
*
* Copyright (c) 2012-2016 Barbara Geller
* Copyright (c) 2012-2016 Ansel Sermersheim
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

#ifndef SHADEREFFECT_H
#define SHADEREFFECT_H

#include <QGraphicsEffect>

QT_BEGIN_NAMESPACE

class ShaderEffectSource;

class ShaderEffect : public QGraphicsEffect
{
    Q_OBJECT

public:
    ShaderEffect(QObject *parent = 0);
    ~ShaderEffect();
    void addRenderTarget(ShaderEffectSource *target);
    void removeRenderTarget(ShaderEffectSource *target);

    QVector<ShaderEffectSource*> m_renderTargets;
    bool m_changed : 1;

protected:
    virtual void draw (QPainter *painter);
    virtual void sourceChanged (ChangeFlags flags);

private:
    void prepareBufferedDraw(QPainter *painter);
    void updateRenderTargets();
    bool hideOriginal() const;
   
};

QT_END_NAMESPACE

#endif // SHADEREFFECT_H
