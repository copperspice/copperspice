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

#include <qdebug.h>
#include <qdir.h>
#include <qstringview.h>
#include <qtemporaryfile.h>

#include <cs_catch2.h>

FILE *handle = nullptr;

TEST_CASE("QDebug traits", "[qdebug]")
{
   REQUIRE(std::is_copy_constructible_v<QDebug> == true);
   REQUIRE(std::is_move_constructible_v<QDebug> == true);

   REQUIRE(std::is_copy_assignable_v<QDebug> == true);
   REQUIRE(std::is_move_assignable_v<QDebug> == true);

   REQUIRE(std::has_virtual_destructor_v<QDebug> == false);
}

void myMessageTest(QtMsgType type, QStringView msg) {
   switch (type) {
      case QtDebugMsg:
         fputs("Debug: ", handle);

         REQUIRE(msg.size_storage() == 23);
         break;

      case QtWarningMsg:
         fputs("Warning: ", handle);

         REQUIRE(msg.size_storage() == 27);
         break;

      case QtCriticalMsg:
         fputs("Critical: ", handle);

         REQUIRE(msg.size_storage() == 15);
         break;

      case QtFatalMsg:
         fputs("Fatal: ", handle);
         break;
   }

   fwrite(msg.charData(), msg.size_storage(), 1, handle);
   fputc('\n', handle);
   fflush(handle);
}

void messageOperator(QtMsgType, QStringView msg) {
   fwrite(msg.charData(), msg.size_storage(), 1, handle);
   fputc('\n', handle);
   fflush(handle);
}

TEST_CASE("QDebug basic", "[qdebug]")
{
   QString str(QDir::tempPath() + "/DebugTest.txt");

   QTemporaryFile tmpFile(str);
   tmpFile.open();

   // append mode
   handle = std::fopen(tmpFile.fileName().constData(), "a");

   csInstallMsgHandler(myMessageTest);

   qDebug("This is a debug message");
   qCritical("Serious message");
   qWarning("Show a message for the user");

   csInstallMsgHandler(nullptr);
   std::fclose(handle);

   {
      tmpFile.seek(0);

      QByteArray data = tmpFile.readLine();
      REQUIRE(data.trimmed() == "Debug: This is a debug message");

      data = tmpFile.readLine();
      REQUIRE(data.trimmed() == "Critical: Serious message");

      data = tmpFile.readLine();
      REQUIRE(data.trimmed() == "Warning: Show a message for the user");
   }
}

TEST_CASE("QDebug operators", "[qdebug]")
{
   QString str(QDir::tempPath() + "/DebugTest.txt");

   QTemporaryFile tmpFile(str);
   tmpFile.open();

   // append mode
   handle = std::fopen(tmpFile.fileName().constData(), "a");

   csInstallMsgHandler(messageOperator);

   qDebug() << 'X';
   qDebug() << 1009;
   qDebug() << 45U;
   qDebug() << -17L;
   qDebug() << 500U;
   qDebug() << QString("I am a string");
   qDebug() << nullptr;
   qDebug() << "End of test";

   csInstallMsgHandler(nullptr);
   std::fclose(handle);

   {
      tmpFile.seek(0);

      QByteArray data = tmpFile.readLine();
      REQUIRE(data.trimmed() == "X");

      data = tmpFile.readLine();
      REQUIRE(data.trimmed() == "1009");

      data = tmpFile.readLine();
      REQUIRE(data.trimmed() == "45");

      data = tmpFile.readLine();
      REQUIRE(data.trimmed() == "-17");

      data = tmpFile.readLine();
      REQUIRE(data.trimmed() == "500");

      data = tmpFile.readLine();
      REQUIRE(data.trimmed() == "I am a string");

      data = tmpFile.readLine();
      REQUIRE(data.trimmed() == "(nullptr)");

      data = tmpFile.readLine();
      REQUIRE(data.trimmed() == "End of test");
   }
}


