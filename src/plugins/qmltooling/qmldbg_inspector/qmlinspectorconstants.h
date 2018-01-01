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

#ifndef QMLINSPECTORCONSTANTS_H
#define QMLINSPECTORCONSTANTS_H

#include <qdeclarativeglobal_p.h>

namespace QmlJSDebugger {
namespace Constants {

enum DesignTool {
    NoTool = 0,
    SelectionToolMode = 1,
    MarqueeSelectionToolMode = 2,
    MoveToolMode = 3,
    ResizeToolMode = 4,
    ColorPickerMode = 5,
    ZoomMode = 6
};

static const int DragStartTime = 50;

static const int DragStartDistance = 20;

static const double ZoomSnapDelta = 0.04;

static const int EditorItemDataKey = 1000;

enum GraphicsItemTypes {
    EditorItemType = 0xEAAA,
    ResizeHandleItemType = 0xEAEA
};


} // namespace Constants
} // namespace QmlJSDebugger

#endif // QMLINSPECTORCONSTANTS_H
