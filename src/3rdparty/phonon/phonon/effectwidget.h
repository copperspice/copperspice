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

#ifndef PHONON_EFFECTWIDGET_H
#define PHONON_EFFECTWIDGET_H

#include "phonon_export.h"
#include "phonondefs.h"
#include <QtGui/QWidget>

QT_BEGIN_NAMESPACE

#ifndef QT_NO_PHONON_EFFECTWIDGET

namespace Phonon
{
class Effect;

    class EffectWidgetPrivate;

    class PHONON_EXPORT EffectWidget : public QWidget
    {
        PHN_CS_OBJECT(EffectWidget)
        K_DECLARE_PRIVATE(EffectWidget)

        public:
            explicit EffectWidget(Effect *effect, QWidget *parent = nullptr);
            ~EffectWidget();

        protected:
            //EffectWidget(EffectWidgetPrivate &dd, QWidget *parent);
            EffectWidgetPrivate *const k_ptr;

        private:
            PHN_CS_SLOT_1(Private, void _k_setToggleParameter(bool checked))
            PHN_CS_SLOT_2(_k_setToggleParameter)

            PHN_CS_SLOT_1(Private, void _k_setIntParameter(int value))
            PHN_CS_SLOT_2(_k_setIntParameter)

            PHN_CS_SLOT_1(Private, void _k_setDoubleParameter(double value))
            PHN_CS_SLOT_2(_k_setDoubleParameter)

            PHN_CS_SLOT_1(Private, void _k_setStringParameter(const QString & un_named_arg1))
            PHN_CS_SLOT_2(_k_setStringParameter)

            PHN_CS_SLOT_1(Private, void _k_setSliderParameter(int un_named_arg1))
            PHN_CS_SLOT_2(_k_setSliderParameter)
    };

} // namespace Phonon

#endif //QT_NO_PHONON_EFFECTWIDGET

QT_END_NAMESPACE

#endif