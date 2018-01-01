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

#ifndef QT7_VIDEOEFFECT_H
#define QT7_VIDEOEFFECT_H

#include <QtCore/QList>
#include <QtCore/QString>
#include <phonon/effectinterface.h>
#include "medianode.h"
#include "videoframe.h"

QT_BEGIN_NAMESPACE

namespace Phonon
{
    class EffectParameter;

namespace QT7
{
    class MediaNodeEvent;

    class VideoEffect : public MediaNode, public Phonon::EffectInterface
    {
        QT7_CS_OBJECT(VideoEffect)
        CS_INTERFACES(Phonon::EffectInterface)

        public:
            VideoEffect(int effectId, QObject *parent);
            virtual ~VideoEffect();

            QList<EffectParameter> parameters() const override;
            QVariant parameterValue(const EffectParameter &) const override;
            void setParameterValue(const EffectParameter &, const QVariant &newValue) override;

            void updateVideo(VideoFrame &frame) override;

        protected:
            void mediaNodeEvent(const MediaNodeEvent *event) override;

        private:
            int effectId;
            void *ciFilter;
            QString filterName;
    };
}} // namespace Phonon::QT7

QT_END_NAMESPACE
#endif // Phonon_QT7_VIDEOEFFECT_H
