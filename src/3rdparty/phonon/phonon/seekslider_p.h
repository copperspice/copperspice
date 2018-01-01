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

#ifndef PHONON_SEEKSLIDER_P_H
#define PHONON_SEEKSLIDER_P_H

#include "seekslider.h"
#include "swiftslider_p.h"
#include <QtGui/QBoxLayout>
#include <QtGui/QLabel>
#include <QtGui/QPixmap>
#include <QtGui/QIcon>
#include <QtGui/QStyle>
#include "factory_p.h"
#include <QtCore/QPointer>
#include "platform_p.h"

QT_BEGIN_NAMESPACE

#ifndef QT_NO_PHONON_SEEKSLIDER

namespace Phonon
{
class MediaObject;
class SeekSliderPrivate
{
    Q_DECLARE_PUBLIC(SeekSlider)
    protected:
        SeekSliderPrivate(SeekSlider *parent)
            : layout(QBoxLayout::LeftToRight, parent),
            slider(Qt::Horizontal, parent),
            iconLabel(parent),
            ticking(false)
#ifndef QT_NO_PHONON_PLATFORMPLUGIN
            ,icon(Platform::icon(QLatin1String("player-time"), parent->style()))
#endif //QT_NO_PHONON_PLATFORMPLUGIN
        {
            const int e = parent->style()->pixelMetric(QStyle::PM_ButtonIconSize);
            iconSize = QSize(e, e);

            slider.setPageStep(5000); // 5 sec
            slider.setSingleStep(500); // 0.5 sec

            layout.setMargin(0);
            layout.setSpacing(2);
            layout.addWidget(&iconLabel, 0, Qt::AlignVCenter);
            layout.addWidget(&slider, 0, Qt::AlignVCenter);

            setEnabled(false);

            if (icon.isNull()) {
                iconLabel.setVisible(false);
            }
        }

        SeekSlider *q_ptr;

    private:
        void setEnabled(bool);
        void _k_stateChanged(Phonon::State);
        void _k_seek(int);
        void _k_tick(qint64);
        void _k_length(qint64);
        void _k_seekableChanged(bool);
        void _k_currentSourceChanged();

        QBoxLayout layout;
        SwiftSlider slider;
        QLabel iconLabel;
        QPointer<MediaObject> media;
        bool ticking;
        QIcon icon;
        QSize iconSize;
};
} // namespace Phonon

#endif //QT_NO_PHONON_SEEKSLIDER

QT_END_NAMESPACE

#endif // SEEKSLIDER_P_H

