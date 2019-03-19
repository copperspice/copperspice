/***********************************************************************
*
* Copyright (c) 2012-2019 Barbara Geller
* Copyright (c) 2012-2019 Ansel Sermersheim
*
* Copyright (C) 2005-2006 Matthias Kretz <kretz@kde.org>
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

#ifndef PHONON_EFFECT_H
#define PHONON_EFFECT_H

#include "phonondefs.h"
#include <QObject>
#include "objectdescription.h"
#include "medianode.h"
#include <qcontainerfwd.h>
#include <qstringfwd.h>

#ifndef QT_NO_PHONON_EFFECT

namespace Phonon
{
    class EffectParameter;
    class EffectPrivate;

    class PHONON_EXPORT Effect : public QObject, public MediaNode
    {
        PHN_CS_OBJECT(Effect)
        K_DECLARE_PRIVATE(Effect)

        public:
            ~Effect();

//X             enum Type {
//X                 AudioEffect,
//X                 VideoEffect
//X             };

            /**
             * QObject constructor.
             *
             * \param description An EffectDescription object to determine the
             * type of effect. See BackendCapabilities::availableAudioEffects().
             * \param parent QObject parent
             */
            explicit Effect(const EffectDescription &description, QObject *parent = nullptr);

//X             Type type() const;

            /**
             * Returns the description of this effect. This is the same type as was
             * passed to the constructor.
             */
            EffectDescription description() const;

            /**
             * Returns a list of parameters that this effect provides to control
             * its behaviour.
             *
             * \see EffectParameter
             * \see EffectWidget
             */
            QList<EffectParameter> parameters() const;

            QVariant parameterValue(const EffectParameter&) const;
            void setParameterValue(const EffectParameter&, const QVariant &value);

        protected:
            Effect(EffectPrivate &dd, QObject *parent);
    };
} //namespace Phonon

#endif // QT_NO_EFFECT

#endif // PHONON_EFFECT_H

