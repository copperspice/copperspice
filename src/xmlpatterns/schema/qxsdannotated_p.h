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

#ifndef QXsdAnnotated_P_H
#define QXsdAnnotated_P_H

#include <qxsdannotation_p.h>

namespace QPatternist {

class XsdAnnotated
{
 public:
   /**
    * Adds a new @p annotation to the component.
    */
   void addAnnotation(const XsdAnnotation::Ptr &annotation);

   /**
    * Adds a list of new @p annotations to the component.
    */
   void addAnnotations(const XsdAnnotation::List &annotations);

   /**
    * Returns the list of all annotations of the component.
    */
   XsdAnnotation::List annotations() const;

 private:
   XsdAnnotation::List m_annotations;
};

}

#endif
