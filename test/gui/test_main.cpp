/***********************************************************************
*
* Copyright (c) 2012-2024 Barbara Geller
* Copyright (c) 2012-2024 Ansel Sermersheim
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

#define CATCH_CONFIG_MAIN

#include <catch2/catch.hpp>

#include <qvariantgui_p.h>

void registerGuiVariant()
{
   static bool isRegistered = false;

   if (! isRegistered) {
      // add gui to the variant system
      static QVariantGui objVariant;
      QVariant::registerClient(&objVariant);

      isRegistered = true;
   }
}
