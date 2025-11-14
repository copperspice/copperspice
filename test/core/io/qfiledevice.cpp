/***********************************************************************
*
* Copyright (c) 2012-2025 Barbara Geller
* Copyright (c) 2012-2025 Ansel Sermersheim
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

#include <qfile.h>
#include <qfiledevice.h>
#include <qtemporaryfile.h>

#include <cs_catch2.h>

TEST_CASE("QFileDevice traits", "[qfiledevice]")
{
   REQUIRE(std::is_copy_constructible_v<QFileDevice> == false);
   REQUIRE(std::is_move_constructible_v<QFileDevice> == false);

   REQUIRE(std::is_copy_assignable_v<QFileDevice> == false);
   REQUIRE(std::is_move_assignable_v<QFileDevice> == false);

   REQUIRE(std::has_virtual_destructor_v<QFileDevice> == true);
}

TEST_CASE("QFileDevice file_error", "[qfiledevice]")
{
   QFile file("nonexistent_file.txt");

   REQUIRE(file.open(QIODevice::ReadOnly) == false);
   REQUIRE(file.error() == QFileDevice::FileError::OpenError);
}

TEST_CASE("QFileDevice permissions", "[qfiledevice]")
{
   QTemporaryFile tmpFile;

   REQUIRE(tmpFile.fileName().isEmpty() == true);
   REQUIRE(tmpFile.isOpen() == false);

   REQUIRE(tmpFile.open() == true);        // opens cs_temp.X

   REQUIRE(tmpFile.fileName().isEmpty() == false);
   REQUIRE(tmpFile.isOpen() == true);

   REQUIRE((tmpFile.permissions() & QFileDevice::ReadOwner) != 0);

   //
   tmpFile.setPermissions(QFileDevice::ReadOwner | QFileDevice::WriteOwner);

   REQUIRE((tmpFile.permissions() & QFileDevice::ReadOwner)  != 0);
   REQUIRE((tmpFile.permissions() & QFileDevice::WriteOwner) != 0);
}

TEST_CASE("QFileDevice read_write", "[qfiledevice]")
{
   QTemporaryFile tmpFile;

   REQUIRE(tmpFile.open() == true);        // opens cs_temp.X

   //
   QByteArray data = "Save this text";

   REQUIRE(tmpFile.write(data) == data.size());
   REQUIRE(tmpFile.flush() == true);

   REQUIRE(tmpFile.size() == 14);

   REQUIRE(tmpFile.seek(0) == true);
   REQUIRE(tmpFile.readAll() == data);

   REQUIRE(tmpFile.seek(6) == true);
   REQUIRE(tmpFile.pos() == 6);
   REQUIRE(tmpFile.read(2) == "hi");
}
