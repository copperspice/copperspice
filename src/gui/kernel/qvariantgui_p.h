/***********************************************************************
*
* Copyright (c) 2012-2024 Barbara Geller
* Copyright (c) 2012-2024 Ansel Sermersheim
*
* Copyright (c) 2015 The Qt Company Ltd.
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

#ifndef QVARIANTGUI_P_H
#define QVARIANTGUI_P_H

#include <qvariant.h>

class Q_GUI_EXPORT QVariantGui : public QVariantBase
{
 public:
   bool cs_internal_convert(uint current_userType, uint new_userType, QVariant &self) const override;
   bool cs_internal_create(uint newUserType, const void *other, QVariant &self) const override;
   bool cs_internal_load(QDataStream &stream, uint type, QVariant &self) const override;
   bool cs_internal_save(QDataStream &stream, uint type, const QVariant &self) const override;
};

#endif
