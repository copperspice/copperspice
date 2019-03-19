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

#ifndef PHONON_EFFECTPARAMETER_H
#define PHONON_EFFECTPARAMETER_H

#include "phonon_export.h"

#include <QtCore/QExplicitlySharedDataPointer>
#include <QtCore/QVariant>

QT_BEGIN_NAMESPACE

#ifndef QT_NO_PHONON_EFFECT

namespace Phonon
{

class Effect;
class EffectParameterPrivate;

class PHONON_EXPORT EffectParameter
{
    friend class BrightnessControl;
    public:
        /*
         * \internal
         *
         * Creates an invalid effect parameter.
         */
        EffectParameter();

        /*
         * The name of the parameter. Can be used as the label.
         *
         * \return A label for the parameter.
         */
        const QString &name() const;

        /*
         * The parameter may come with a description (LADSPA doesn't have a
         * field for this, so don't expect many effects to provide a
         * description).
         *
         * The description can be used for a tooltip or WhatsThis help.
         *
         * \return A text describing the parameter.
         */
        const QString &description() const;

        /*
         * Returns the parameter type.
         *
         * Common types are QVariant::Int, QVariant::Double, QVariant::Bool and QVariant::String. When
         * QVariant::String is returned you get the possible values from possibleValues.
         */
        QVariant::Type type() const;

        /*
         * Returns whether the parameter should be
         * displayed using a logarithmic scale. This is particularly useful for
         * frequencies and gains.
         */
        bool isLogarithmicControl() const;

        /*
         * The minimum value to be used for the control to edit the parameter.
         *
         * If the returned QVariant is invalid the value is not bounded from
         * below.
         */
        QVariant minimumValue() const;

        /*
         * The maximum value to be used for the control to edit the parameter.
         *
         * If the returned QVariant is invalid the value is not bounded from
         * above.
         */
        QVariant maximumValue() const;

        /*
         * The default value.
         */
        QVariant defaultValue() const;

        /*
         * The possible values to be used for the control to edit the parameter.
         *
         * if the value of this parameter is to be picked from predefined values
         * this returns the list (otherwise it returns an empty QVariantList).
         */
        QVariantList possibleValues() const;

        /*
         * \internal
         * compares the ids of the parameters
         */
        bool operator<(const EffectParameter &rhs) const;

        /*
         * \internal
         * compares the ids of the parameters
         */
        bool operator>(const EffectParameter &rhs) const;

        /*
         * \internal
         * compares the ids of the parameters
         */
        bool operator==(const EffectParameter &rhs) const;

        /* dtor, cctor and operator= for forward decl of EffectParameterPrivate */
        ~EffectParameter();
        EffectParameter(const EffectParameter &rhs);
        EffectParameter &operator=(const EffectParameter &rhs);

        /*
         * Only for backend developers:
         *
         * Flags to set the return values of isToggleControl(),
         * isLogarithmicControl(), isIntegerControl(), isBoundedBelow() and
         * isBoundedAbove(). The values of the flags correspond to the values
         * used for LADSPA effects.
         */
        enum Hint {
            /*
             * If this hint is set it means that
             * the the control has only two states: zero and non-zero.
             *
             * \see isToggleControl()
             */
            ToggledHint      = 0x04,

            /* LADSPA's SAMPLE_RATE hint needs to be translated by the backend
             * to normal bounds, as the backend knows the sample rate - and the
             * frontend doesn't */

            /*
             * \see isLogarithmicControl()
             */
            LogarithmicHint  = 0x10,
            /*
             * \see isIntegerControl
             */
            IntegerHint      = 0x20
        };
        using Hints = QFlags<Hint>;

        /*
         * Only to be used by backend implementations:
         *
         * Creates a new effect parameter.
         *
         * \param parameterId This is a number to uniquely identify the
         * parameter. The id is used for value() and setValue().
         *
         * \param name The name/label for this parameter.
         *
         * \param hints Sets the hints for the type of parameter.
         *
         * \param defaultValue The value that should be used as a default.
         *
         * \param min The minimum value allowed for this parameter. You only
         * need to set this if the BoundedBelowHint is set.
         *
         * \param max The maximum value allowed for this parameter. You only
         * need to set this if the BoundedAboveHint is set.
         *
         * \param description A descriptive text for the parameter
         * (explaining what it controls) to be used as a tooltip or
         * WhatsThis help.
         */
        EffectParameter(int parameterId, const QString &name, Hints hints,
                const QVariant &defaultValue, const QVariant &min = QVariant(),
                const QVariant &max = QVariant(), const QVariantList &values = QVariantList(),
                const QString &description = QString());

        /*
         * \internal
         *
         * Returns the parameter's id.
         */
        int id() const;

    protected:
        /*
         * The data is implicitly shared.
         */
        QExplicitlySharedDataPointer<EffectParameterPrivate> d;
};

uint PHONON_EXPORT qHash(const Phonon::EffectParameter &param);

} // namespace Phonon


Q_DECLARE_OPERATORS_FOR_FLAGS(Phonon::EffectParameter::Hints)

#endif //QT_NO_PHONON_EFFECT

QT_END_NAMESPACE

#endif // PHONON_EFFECTPARAMETER_H
