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
**  Copyright (C) 2005-2006 Matthias Kretz <kretz@kde.org
********************************************************/

#ifndef PHONON_EFFECT_H
#define PHONON_EFFECT_H

#include "phonondefs.h"
#include <QtCore/QObject>
#include "objectdescription.h"
#include "medianode.h"

QT_BEGIN_NAMESPACE

#ifndef QT_NO_PHONON_EFFECT

class QString;
template<class T> class QList;

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

QT_END_NAMESPACE

#endif // PHONON_EFFECT_H

