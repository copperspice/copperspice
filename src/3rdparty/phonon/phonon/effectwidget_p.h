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

#ifndef PHONON_EFFECTWIDGET_P_H
#define PHONON_EFFECTWIDGET_P_H

#include "effectwidget.h"
#include "effectparameter.h"
#include <QtCore/QHash>

QT_BEGIN_NAMESPACE

#ifndef QT_NO_PHONON_EFFECTWIDGET

namespace Phonon
{
    class EffectWidgetPrivate
    {
        Q_DECLARE_PUBLIC(EffectWidget)
        protected:
            EffectWidgetPrivate(Effect *effect);

            EffectWidget *q_ptr;

        private:
            Effect *effect;
            QHash<QObject *, EffectParameter> parameterForObject;

            void _k_setToggleParameter(bool checked);
            void _k_setIntParameter(int value);
            void _k_setDoubleParameter(double value);
            void _k_setStringParameter(const QString &);
            void _k_setSliderParameter(int);

            void autogenerateUi();
    };
} // namespace Phonon

#endif //QT_NO_PHONON_EFFECTWIDGET

QT_END_NAMESPACE

#endif // PHONON_UI_EFFECTWIDGET_P_H
