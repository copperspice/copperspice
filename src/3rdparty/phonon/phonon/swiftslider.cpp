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

#include "swiftslider_p.h"

#include <QtGui/QMouseEvent>
#include <QtGui/QStyle>
#include <QtGui/QStyleOption>

QT_BEGIN_NAMESPACE

#if !defined(QT_NO_PHONON_SEEKSLIDER) && !defined(QT_NO_PHONON_VOLUMESLIDER)

namespace Phonon
{

SwiftSlider::SwiftSlider(Qt::Orientation orientation, QWidget * parent)
	: QSlider(orientation, parent)
{
}

SwiftSlider::~SwiftSlider()
{
}

// Function copied from qslider.cpp
inline int SwiftSlider::pick(const QPoint &pt) const
{
    return orientation() == Qt::Horizontal ? pt.x() : pt.y();
}

// Function copied from qslider.cpp and modified to make it compile
int SwiftSlider::pixelPosToRangeValue(int pos) const
{
    QStyleOptionSlider opt;
    initStyleOption(&opt);
    QRect gr = style()->subControlRect(QStyle::CC_Slider, &opt, QStyle::SC_SliderGroove, this);
    QRect sr = style()->subControlRect(QStyle::CC_Slider, &opt, QStyle::SC_SliderHandle, this);
    int sliderMin, sliderMax, sliderLength;

    if (orientation() == Qt::Horizontal) {
        sliderLength = sr.width();
        sliderMin = gr.x();
        sliderMax = gr.right() - sliderLength + 1;
    } else {
        sliderLength = sr.height();
        sliderMin = gr.y();
        sliderMax = gr.bottom() - sliderLength + 1;
    }
    return QStyle::sliderValueFromPosition(minimum(), maximum(), pos - sliderMin,
                                           sliderMax - sliderMin, opt.upsideDown);
}

// Based on code from qslider.cpp
void SwiftSlider::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        QStyleOptionSlider opt;
        initStyleOption(&opt);
        const QRect sliderRect = style()->subControlRect(QStyle::CC_Slider, &opt, QStyle::SC_SliderHandle, this);
        const QPoint center = sliderRect.center() - sliderRect.topLeft();
        // to take half of the slider off for the setSliderPosition call we use the center - topLeft

        if (!sliderRect.contains(event->pos())) {
            event->accept();

            setSliderPosition(pixelPosToRangeValue(pick(event->pos() - center)));
            triggerAction(SliderMove);
            setRepeatAction(SliderNoAction);
        } else {
            QSlider::mousePressEvent(event);
        }
    } else {
        QSlider::mousePressEvent(event);
    }
}

} // namespace Phonon

#endif //QT_NO_PHONON_VOLUMESLIDER && QT_NO_PHONON_VOLUMESLIDER

QT_END_NAMESPACE
