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

#ifndef PHONON_VOLUMESLIDER_P_H
#define PHONON_VOLUMESLIDER_P_H

#include "volumeslider.h"
#include "swiftslider_p.h"
#include <QtGui/QBoxLayout>
#include <QtGui/QLabel>
#include <QtGui/QPixmap>
#include <QtGui/QToolButton>
#include "factory_p.h"
#include "audiooutput.h"
#include <QtGui/QIcon>
#include <QtCore/QPointer>
#include "platform_p.h"

QT_BEGIN_NAMESPACE

#ifndef QT_NO_PHONON_VOLUMESLIDER

namespace Phonon
{

class VolumeSliderPrivate
{
    Q_DECLARE_PUBLIC(VolumeSlider)

    protected:
        VolumeSliderPrivate(VolumeSlider *parent)
            : q_ptr(parent),
            layout(QBoxLayout::LeftToRight, parent),
            slider(Qt::Horizontal, parent),
            muteButton(parent),
            volumeIcon(Platform::icon(QLatin1String("player-volume"), parent->style())),
            mutedIcon(Platform::icon(QLatin1String("player-volume-muted"), parent->style())),
            output(0),
            ignoreVolumeChange(false)
        {
            slider.setRange(0, 100);
            slider.setPageStep(5);
            slider.setSingleStep(1);

            muteButton.setIcon(volumeIcon);
            muteButton.setAutoRaise(true);
            layout.setMargin(0);
            layout.setSpacing(2);
            layout.addWidget(&muteButton, 0, Qt::AlignVCenter);
            layout.addWidget(&slider, 0, Qt::AlignVCenter);

            slider.setEnabled(false);
            muteButton.setEnabled(false);

            if (volumeIcon.isNull()) {
                muteButton.setVisible(false);
            }
        }

        VolumeSlider *q_ptr;

        void _k_sliderChanged(int);
        void _k_volumeChanged(qreal);
        void _k_mutedChanged(bool);
        void _k_buttonClicked();

    private:
        QBoxLayout layout;
        SwiftSlider slider;
        QToolButton muteButton;
        QIcon volumeIcon;
        QIcon mutedIcon;

        QPointer<AudioOutput> output;
        bool ignoreVolumeChange;
};
} // namespace Phonon

#endif //QT_NO_PHONON_VOLUMESLIDER

QT_END_NAMESPACE

#endif
