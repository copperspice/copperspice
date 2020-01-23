/***********************************************************************
*
* Copyright (c) 2012-2020 Barbara Geller
* Copyright (c) 2012-2020 Ansel Sermersheim
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

#include "directshowevrvideowindowcontrol.h"

#include "directshowglobal.h"

DirectShowEvrVideoWindowControl::DirectShowEvrVideoWindowControl(QObject *parent)
   : EvrVideoWindowControl(parent)
   , m_evrFilter(NULL)
{
}

DirectShowEvrVideoWindowControl::~DirectShowEvrVideoWindowControl()
{
   if (m_evrFilter) {
      m_evrFilter->Release();
   }
}

IBaseFilter *DirectShowEvrVideoWindowControl::filter()
{
   if (!m_evrFilter) {
      m_evrFilter = com_new<IBaseFilter>(clsid_EnhancedVideoRenderer);
      if (!setEvr(m_evrFilter)) {
         m_evrFilter->Release();
         m_evrFilter = NULL;
      }
   }

   return m_evrFilter;
}
