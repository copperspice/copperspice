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

#ifndef GSTREAMER_EFFECT_H
#define GSTREAMER_EFFECT_H

#include "common.h"
#include "medianode.h"
#include <phonon/effectparameter.h>
#include <phonon/effectinterface.h>
#include <QtCore/QObject>
#include <gst/gst.h>

#ifndef QT_NO_PHONON_EFFECT
QT_BEGIN_NAMESPACE
namespace Phonon
{
namespace Gstreamer
{
    class EffectInfo;

    class Effect : public QObject, public Phonon::EffectInterface, public MediaNode
    {
        CS_OBJECT_MULTIPLE(Effect, QObject)

        CS_INTERFACES(Phonon::EffectInterface, Phonon::Gstreamer::MediaNode)
      
        public:
            Effect (Backend *backend, QObject *parent, NodeDescription description);
            virtual ~Effect ();

            QList<EffectParameter> parameters() const override;
            QVariant parameterValue(const EffectParameter &) const override;
            void setParameterValue(const EffectParameter &, const QVariant &) override;

            virtual GstElement* createEffectBin() = 0;
            virtual void init();
            virtual void setupEffectParams();

        protected:
            GstElement *m_effectBin;
            GstElement *m_effectElement;
            QList<Phonon::EffectParameter> m_parameterList;
    };
}} //namespace Phonon::Gstreamer

QT_END_NAMESPACE
#endif //QT_NO_PHONON_EFFECT

#endif // Phonon_GSTREAMER_EFFECT_H
