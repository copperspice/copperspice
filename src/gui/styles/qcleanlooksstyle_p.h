/***********************************************************************
*
* Copyright (c) 2012-2019 Barbara Geller
* Copyright (c) 2012-2019 Ansel Sermersheim
*
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

#ifndef QCLEANLOOKSSTYLE_P_H
#define QCLEANLOOKSSTYLE_P_H

#include <qwindowsstyle.h>
#include <qwindowsstyle_p.h>

#ifndef QT_NO_STYLE_CLEANLOOKS

QT_BEGIN_NAMESPACE

class QCleanlooksStylePrivate : public QWindowsStylePrivate
{
   Q_DECLARE_PUBLIC(QCleanlooksStyle)

 public:
   QCleanlooksStylePrivate()
      : QWindowsStylePrivate() {
      animationFps = 24;
   }

   ~QCleanlooksStylePrivate() {
   }
};

QT_END_NAMESPACE

#endif // QT_NO_STYLE_CLEANLOOKS

#endif //QCLEANLOOKSSTYLE_P_H
