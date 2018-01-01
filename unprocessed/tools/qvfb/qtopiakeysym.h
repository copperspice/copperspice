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

#ifndef QTKEYSYM_H
#define QTKEYSYM_H

/* Special keys used by Qtopia, mapped into the X11 private keypad range */
#define QTOPIAXK_Select		0x11000601
#define QTOPIAXK_Yes		0x11000602
#define QTOPIAXK_No		0x11000603

#define QTOPIAXK_Cancel		0x11000604
#define QTOPIAXK_Printer	0x11000605
#define QTOPIAXK_Execute	0x11000606
#define QTOPIAXK_Sleep		0x11000607
#define QTOPIAXK_Play		0x11000608
#define QTOPIAXK_Zoom		0x11000609

#define QTOPIAXK_Context1	0x1100060A
#define QTOPIAXK_Context2	0x1100060B
#define QTOPIAXK_Context3	0x1100060C
#define QTOPIAXK_Context4	0x1100060D
#define QTOPIAXK_Call		0x1100060E
#define QTOPIAXK_Hangup		0x1100060F
#define QTOPIAXK_Flip		0x11000610

#define	QTOPIAXK_Min		QTOPIAXK_Select
#define	QTOPIAXK_Max		QTOPIAXK_Flip

#endif
