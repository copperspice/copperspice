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

/********************************************************
**  This file is part of the KDE project.
********************************************************/

#include "videoeffect.h"
#include "backendheader.h"
#include "objc_help.h"
#include <phonon/effect.h>
#include <phonon/effectparameter.h>

QT_BEGIN_NAMESPACE

namespace Phonon
{
namespace QT7
{

VideoEffect::VideoEffect(int effectId, QObject *parent) : MediaNode(VideoSink | VideoSource, 0, parent), effectId(effectId)
{
    ciFilter = objc_createCiFilter(effectId);
    if (ciFilter)
        filterName = objc_getCiFilterInfo()->filterDisplayNames[effectId];
}

VideoEffect::~VideoEffect()
{
    if (ciFilter)
        objc_releaseCiFilter(ciFilter);
}

QList<EffectParameter> VideoEffect::parameters() const
{
    IMPLEMENTED;
    return objc_getCiFilterParameterInfo(ciFilter).parameters;
}

QVariant VideoEffect::parameterValue(const EffectParameter &parameter) const
{
    IMPLEMENTED;
    return objc_getCiFilterParameter(ciFilter, parameter.id());
}

void VideoEffect::setParameterValue(const EffectParameter &parameter, const QVariant &newValue)
{
    IMPLEMENTED;
    objc_setCiFilterParameter(ciFilter, parameter.id(), newValue);
}

void VideoEffect::mediaNodeEvent(const MediaNodeEvent */*event*/)
{
}

void VideoEffect::updateVideo(VideoFrame &frame){
    frame.applyCoreImageFilter(ciFilter);
    MediaNode::updateVideo(frame);
}

}}

QT_END_NAMESPACE



