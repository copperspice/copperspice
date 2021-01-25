/***********************************************************************
*
* Copyright (c) 2012-2021 Barbara Geller
* Copyright (c) 2012-2021 Ansel Sermersheim
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

#include <qbytearray.h>
#include <qdatetime.h>
#include <qrect.h>
#include <qurl.h>
#include <quuid.h>
#include <qvariant.h>

#include <cs_catch2.h>

TEST_CASE("QVariant empty", "[qvariant]")
{
   QVariant data;

   REQUIRE(! data.isValid());

   REQUIRE(data.toBool() == false);
   REQUIRE(data.toByteArray() == "");
   REQUIRE(data.toInt() == 0);
   REQUIRE(data.toString() == "");
   REQUIRE(data.toString16() == "");
}

TEST_CASE("QVariant constructor_int", "[qvariant]")
{
   QVariant data = 17;

   REQUIRE(data.isValid());
   REQUIRE(data.type() == QVariant::Int);

   REQUIRE(data.toBool() == true);
   REQUIRE(data.toDate() == QDate());
   REQUIRE(data.toChar() == '\x11');
   REQUIRE(data.toString() == "17");
   REQUIRE(data.toString16() == "17");

   REQUIRE(data.toInt() == 17);
   REQUIRE(data.toUInt() == 17);
   REQUIRE(data.toLong() == 17);
   REQUIRE(data.toULong() == 17);
   REQUIRE(data.toLongLong() == 17);
   REQUIRE(data.toULongLong() == 17);
   REQUIRE(data.toDouble() == 17.0);
   REQUIRE(data.toFloat() == 17.0f);
}

TEST_CASE("QVariant constructor_string8", "[qvariant]")
{
   QVariant data = QString("apple");

   REQUIRE(data.isValid());
   REQUIRE(data.type() == QVariant::String);

   REQUIRE(data.toDate() == QDate());
   REQUIRE(data.toString() == "apple");
   REQUIRE(data.toString16() == "apple");

   REQUIRE(data.toInt() == 0);
}

TEST_CASE("QVariant constructor_string16", "[qvariant]")
{
   QVariant data = QString16("apple");

   REQUIRE(data.isValid());
   REQUIRE(data.type() == QVariant::String16);

   REQUIRE(data.toDate() == QDate());
   REQUIRE(data.toString() == "apple");
   REQUIRE(data.toString16() == "apple");

   REQUIRE(data.toInt() == 0);
}

TEST_CASE("QVariant copy_constructor_string8", "[qvariant]")
{
   QVariant data1 = QString("apple");

   QVariant data2 = data1;

   REQUIRE(data1.isValid());
   REQUIRE(data2.isValid());

   data1 = QString("pineapple");

   REQUIRE(data1.toString() == "pineapple");
   REQUIRE(data2.toString() == "apple");
}

TEST_CASE("QVariant copy_constructor_string16", "[qvariant]")
{
   QVariant data1 = QString16("apple");

   QVariant data2 = data1;

   REQUIRE(data1.isValid());
   REQUIRE(data2.isValid());

   data1 = QString16("pineapple");

   REQUIRE(data1.toString16() == "pineapple");
   REQUIRE(data2.toString16() == "apple");
}

TEST_CASE("QVariant can_convert_string8", "[qvariant]")
{
   QVariant data = QString("apple");

   REQUIRE(data.canConvert(QVariant::Bool));
   REQUIRE(data.canConvert(QVariant::ByteArray));
   REQUIRE(data.canConvert(QVariant::Char));
   REQUIRE(data.canConvert(QVariant::Color));
   REQUIRE(data.canConvert(QVariant::Date));
   REQUIRE(data.canConvert(QVariant::DateTime));
   REQUIRE(data.canConvert(QVariant::Font));
   REQUIRE(data.canConvert(QVariant::KeySequence));
   REQUIRE(data.canConvert(QVariant::String));
   REQUIRE(data.canConvert(QVariant::String16));
   REQUIRE(data.canConvert(QVariant::StringList));
   REQUIRE(data.canConvert(QVariant::Time));
   REQUIRE(data.canConvert(QVariant::Url));
   REQUIRE(data.canConvert(QVariant::Uuid));

   REQUIRE(data.canConvert(QVariant::QChar));
   REQUIRE(data.canConvert(QVariant::SChar));
   REQUIRE(data.canConvert(QVariant::UChar));

   REQUIRE(data.canConvert(QVariant::Double));
   REQUIRE(data.canConvert(QVariant::Float));
   REQUIRE(data.canConvert(QVariant::Int));
   REQUIRE(data.canConvert(QVariant::Long));
   REQUIRE(data.canConvert(QVariant::LongLong));
   REQUIRE(data.canConvert(QVariant::Short));
   REQUIRE(data.canConvert(QVariant::UInt));
   REQUIRE(data.canConvert(QVariant::ULong));
   REQUIRE(data.canConvert(QVariant::ULongLong));
   REQUIRE(data.canConvert(QVariant::UShort));

   REQUIRE(! data.canConvert(QVariant::Invalid));
   REQUIRE(! data.canConvert(QVariant::BitArray));
   REQUIRE(! data.canConvert(QVariant::Bitmap));
   REQUIRE(! data.canConvert(QVariant::Brush));
   REQUIRE(! data.canConvert(QVariant::Char8_t));
   REQUIRE(! data.canConvert(QVariant::Char16_t));
   REQUIRE(! data.canConvert(QVariant::Char32_t));
   REQUIRE(! data.canConvert(QVariant::Cursor));
   REQUIRE(! data.canConvert(QVariant::EasingCurve));
   REQUIRE(! data.canConvert(QVariant::JsonArray));
   REQUIRE(! data.canConvert(QVariant::JsonDocument));
   REQUIRE(! data.canConvert(QVariant::JsonObject));
   REQUIRE(! data.canConvert(QVariant::JsonValue));
   REQUIRE(! data.canConvert(QVariant::Hash));
   REQUIRE(! data.canConvert(QVariant::Icon));
   REQUIRE(! data.canConvert(QVariant::Image));
   REQUIRE(! data.canConvert(QVariant::Line));
   REQUIRE(! data.canConvert(QVariant::LineF));
   REQUIRE(! data.canConvert(QVariant::List));
   REQUIRE(! data.canConvert(QVariant::Locale));
   REQUIRE(! data.canConvert(QVariant::Map));
   REQUIRE(! data.canConvert(QVariant::Matrix));
   REQUIRE(! data.canConvert(QVariant::Matrix4x4));
   REQUIRE(! data.canConvert(QVariant::ModelIndex));
   REQUIRE(! data.canConvert(QVariant::MultiHash));
   REQUIRE(! data.canConvert(QVariant::MultiMap));
   REQUIRE(! data.canConvert(QVariant::ObjectStar));
   REQUIRE(! data.canConvert(QVariant::Palette));
   REQUIRE(! data.canConvert(QVariant::Pen));
   REQUIRE(! data.canConvert(QVariant::PersistentModelIndex));
   REQUIRE(! data.canConvert(QVariant::Pixmap));
   REQUIRE(! data.canConvert(QVariant::Point));
   REQUIRE(! data.canConvert(QVariant::PointF));
   REQUIRE(! data.canConvert(QVariant::Polygon));
   REQUIRE(! data.canConvert(QVariant::PolygonF));
   REQUIRE(! data.canConvert(QVariant::Quaternion));
   REQUIRE(! data.canConvert(QVariant::Region));
   REQUIRE(! data.canConvert(QVariant::RegularExpression));
   REQUIRE(! data.canConvert(QVariant::Rect));
   REQUIRE(! data.canConvert(QVariant::RectF));
   REQUIRE(! data.canConvert(QVariant::Size));
   REQUIRE(! data.canConvert(QVariant::SizeF));
   REQUIRE(! data.canConvert(QVariant::SizePolicy));
   REQUIRE(! data.canConvert(QVariant::StringView));
   REQUIRE(! data.canConvert(QVariant::TextFormat));
   REQUIRE(! data.canConvert(QVariant::TextLength));
   REQUIRE(! data.canConvert(QVariant::Transform));
   REQUIRE(! data.canConvert(QVariant::Variant));
   REQUIRE(! data.canConvert(QVariant::Vector2D));
   REQUIRE(! data.canConvert(QVariant::Vector3D));
   REQUIRE(! data.canConvert(QVariant::Vector4D));
   REQUIRE(! data.canConvert(QVariant::Void));
   REQUIRE(! data.canConvert(QVariant::VoidStar));
   REQUIRE(! data.canConvert(QVariant::WidgetStar));
}

TEST_CASE("QVariant can_convert_string16", "[qvariant]")
{
   QVariant data = QString16("apple");

   REQUIRE(data.canConvert(QVariant::Bool));
   REQUIRE(data.canConvert(QVariant::ByteArray));
   REQUIRE(data.canConvert(QVariant::Char));
   REQUIRE(data.canConvert(QVariant::Color));
   REQUIRE(data.canConvert(QVariant::Date));
   REQUIRE(data.canConvert(QVariant::DateTime));
   REQUIRE(data.canConvert(QVariant::Font));
   REQUIRE(data.canConvert(QVariant::KeySequence));
   REQUIRE(data.canConvert(QVariant::String));
   REQUIRE(data.canConvert(QVariant::String16));
   REQUIRE(data.canConvert(QVariant::StringList));
   REQUIRE(data.canConvert(QVariant::Time));
   REQUIRE(data.canConvert(QVariant::Url));
   REQUIRE(data.canConvert(QVariant::Uuid));

   REQUIRE(data.canConvert(QVariant::QChar));
   REQUIRE(data.canConvert(QVariant::SChar));
   REQUIRE(data.canConvert(QVariant::UChar));

   REQUIRE(data.canConvert(QVariant::Double));
   REQUIRE(data.canConvert(QVariant::Float));
   REQUIRE(data.canConvert(QVariant::Int));
   REQUIRE(data.canConvert(QVariant::Long));
   REQUIRE(data.canConvert(QVariant::LongLong));
   REQUIRE(data.canConvert(QVariant::Short));
   REQUIRE(data.canConvert(QVariant::UInt));
   REQUIRE(data.canConvert(QVariant::ULong));
   REQUIRE(data.canConvert(QVariant::ULongLong));
   REQUIRE(data.canConvert(QVariant::UShort));

   REQUIRE(! data.canConvert(QVariant::Invalid));
   REQUIRE(! data.canConvert(QVariant::BitArray));
   REQUIRE(! data.canConvert(QVariant::Bitmap));
   REQUIRE(! data.canConvert(QVariant::Brush));
   REQUIRE(! data.canConvert(QVariant::Char8_t));
   REQUIRE(! data.canConvert(QVariant::Char16_t));
   REQUIRE(! data.canConvert(QVariant::Char32_t));
   REQUIRE(! data.canConvert(QVariant::Cursor));
   REQUIRE(! data.canConvert(QVariant::EasingCurve));
   REQUIRE(! data.canConvert(QVariant::JsonArray));
   REQUIRE(! data.canConvert(QVariant::JsonDocument));
   REQUIRE(! data.canConvert(QVariant::JsonObject));
   REQUIRE(! data.canConvert(QVariant::JsonValue));
   REQUIRE(! data.canConvert(QVariant::Hash));
   REQUIRE(! data.canConvert(QVariant::Icon));
   REQUIRE(! data.canConvert(QVariant::Image));
   REQUIRE(! data.canConvert(QVariant::Line));
   REQUIRE(! data.canConvert(QVariant::LineF));
   REQUIRE(! data.canConvert(QVariant::List));
   REQUIRE(! data.canConvert(QVariant::Locale));
   REQUIRE(! data.canConvert(QVariant::Map));
   REQUIRE(! data.canConvert(QVariant::Matrix));
   REQUIRE(! data.canConvert(QVariant::Matrix4x4));
   REQUIRE(! data.canConvert(QVariant::ModelIndex));
   REQUIRE(! data.canConvert(QVariant::MultiHash));
   REQUIRE(! data.canConvert(QVariant::MultiMap));
   REQUIRE(! data.canConvert(QVariant::ObjectStar));
   REQUIRE(! data.canConvert(QVariant::Palette));
   REQUIRE(! data.canConvert(QVariant::Pen));
   REQUIRE(! data.canConvert(QVariant::PersistentModelIndex));
   REQUIRE(! data.canConvert(QVariant::Pixmap));
   REQUIRE(! data.canConvert(QVariant::Point));
   REQUIRE(! data.canConvert(QVariant::PointF));
   REQUIRE(! data.canConvert(QVariant::Polygon));
   REQUIRE(! data.canConvert(QVariant::PolygonF));
   REQUIRE(! data.canConvert(QVariant::Quaternion));
   REQUIRE(! data.canConvert(QVariant::Region));
   REQUIRE(! data.canConvert(QVariant::RegularExpression));
   REQUIRE(! data.canConvert(QVariant::Rect));
   REQUIRE(! data.canConvert(QVariant::RectF));
   REQUIRE(! data.canConvert(QVariant::Size));
   REQUIRE(! data.canConvert(QVariant::SizeF));
   REQUIRE(! data.canConvert(QVariant::SizePolicy));
   REQUIRE(! data.canConvert(QVariant::StringView));
   REQUIRE(! data.canConvert(QVariant::TextFormat));
   REQUIRE(! data.canConvert(QVariant::TextLength));
   REQUIRE(! data.canConvert(QVariant::Transform));
   REQUIRE(! data.canConvert(QVariant::Variant));
   REQUIRE(! data.canConvert(QVariant::Vector2D));
   REQUIRE(! data.canConvert(QVariant::Vector3D));
   REQUIRE(! data.canConvert(QVariant::Vector4D));
   REQUIRE(! data.canConvert(QVariant::Void));
   REQUIRE(! data.canConvert(QVariant::VoidStar));
   REQUIRE(! data.canConvert(QVariant::WidgetStar));
}

TEST_CASE("QVariant swap_string", "[qvariant]")
{
   QVariant data1 = QString("apple");
   QVariant data2 = QString("pineapple");

   data1.swap(data2);

   REQUIRE(data1.toString() == "pineapple");
   REQUIRE(data2.toString() == "apple");
}

TEST_CASE("QVariant type_name", "[qvariant]")
{
   QVariant data = QByteArray();
   REQUIRE(data.typeName() == "QByteArray");

   data = QUrl();
   REQUIRE(data.typeName() == "QUrl");

   data = QUuid();
   REQUIRE(data.typeName() == "QUuid");
}

TEST_CASE("QVariant type_to_name", "[qvariant]")
{
   REQUIRE(QVariant::typeToName(QVariant::Invalid) == "");
   REQUIRE(QVariant::typeToName(QVariant::Bool) == "bool");

   REQUIRE(QVariant::typeToName(QVariant::Date) == "QDate");
   REQUIRE(QVariant::typeToName(QVariant::JsonArray) == "QJsonArray");
   REQUIRE(QVariant::typeToName(QVariant::JsonDocument) == "QJsonDocument");
   REQUIRE(QVariant::typeToName(QVariant::JsonObject) == "QJsonObject");
   REQUIRE(QVariant::typeToName(QVariant::JsonValue) == "QJsonValue");
   REQUIRE(QVariant::typeToName(QVariant::Hash) == "QVariantHash");
   REQUIRE(QVariant::typeToName(QVariant::List) == "QVariantList");
   REQUIRE(QVariant::typeToName(QVariant::Map) == "QVariantMap");
   REQUIRE(QVariant::typeToName(QVariant::Rect) == "QRect");
   REQUIRE(QVariant::typeToName(QVariant::RectF) == "QRectF");
   REQUIRE(QVariant::typeToName(QVariant::Size) == "QSize");
   REQUIRE(QVariant::typeToName(QVariant::SizeF) == "QSizeF");
   REQUIRE(QVariant::typeToName(QVariant::String) == "QString");
   REQUIRE(QVariant::typeToName(QVariant::String16) == "QString16");
   REQUIRE(QVariant::typeToName(QVariant::Url) == "QUrl");
   REQUIRE(QVariant::typeToName(QVariant::Uuid) == "QUuid");
}

TEST_CASE("QVariant name_to_type", "[qvariant]")
{
   REQUIRE(QVariant::nameToType("SillName") == QVariant::Invalid);
   REQUIRE(QVariant::nameToType("bool") == QVariant::Bool);

   REQUIRE(QVariant::nameToType("QDate") == QVariant::Date);
   REQUIRE(QVariant::nameToType("QJsonArray") == QVariant::JsonArray);
   REQUIRE(QVariant::nameToType("QJsonDocument") ==QVariant::JsonDocument);
   REQUIRE(QVariant::nameToType("QJsonObject") == QVariant::JsonObject);
   REQUIRE(QVariant::nameToType("QJsonValue") == QVariant::JsonValue);
   REQUIRE(QVariant::nameToType("QVariantHash") == QVariant::Hash);
   REQUIRE(QVariant::nameToType("QVariantList") == QVariant::List);
   REQUIRE(QVariant::nameToType("QVariantMap") == QVariant::Map);
   REQUIRE(QVariant::nameToType("QRect") == QVariant::Rect);
   REQUIRE(QVariant::nameToType("QRectF") == QVariant::RectF);
   REQUIRE(QVariant::nameToType("QSize") == QVariant::Size);
   REQUIRE(QVariant::nameToType("QSizeF") == QVariant::SizeF);
   REQUIRE(QVariant::nameToType("QString") == QVariant::String);
   REQUIRE(QVariant::nameToType("QString16") == QVariant::String16);
   REQUIRE(QVariant::nameToType("QUrl") == QVariant::Url);
   REQUIRE(QVariant::nameToType("QUuid") == QVariant::Uuid);
}

TEST_CASE("QVariant user_type", "[qvariant]")
{
   // emerald
}

TEST_CASE("QVariant map", "[qvariant]")
{
   QMap<QString, QVariant> map1 = { {"orange", 17} };

   QVariant data    = map1;
   QVariantMap map2 = data.value<QVariantMap>();

   REQUIRE(map1 == map2);
   REQUIRE(map2.value("orange").toInt() == 17);

   map2 = data.toMap();
   REQUIRE(map1 == map2);
   REQUIRE(map2.value("orange").toInt() == 17);
}

TEST_CASE("QVariant hash", "[qvariant]")
{
   QHash<QString, QVariant> hash1 = { {"orange", 17} };

   QVariant data      = hash1;
   QVariantHash hash2 = data.value<QVariantHash>();

   REQUIRE(hash1 == hash2);
   REQUIRE(hash2.value("orange").toInt() == 17);

   hash2 = data.toHash();
   REQUIRE(hash1 == hash2);
   REQUIRE(hash2.value("orange").toInt() == 17);
}

TEST_CASE("QVariant url", "[qvariant]")
{
   QString str("https://www.copperspice.com");
   QUrl url(str);

   QVariant data1(str);
   QVariant data2(url);

   REQUIRE(data1.toUrl() == url);

   REQUIRE(data1.canConvert<QUrl>());
   REQUIRE(data2.canConvert<QString>());
}

TEST_CASE("QVariant equality_string8", "[qvariant]")
{
   QVariant data1 = 5;
   QVariant data2 = QString("5");

   REQUIRE(data1 == data2);
}

TEST_CASE("QVariant move_operations", "[qvariant]")
{
   // emerald
}

TEST_CASE("QVariant get_data", "[qvariant]")
{
   // emerald
}

TEST_CASE("QVariant get_data_or", "[qvariant]")
{
   // emerald
}

TEST_CASE("QVariant value", "[qvariant]")
{
   // emerald
}

TEST_CASE("QVariant from_value", "[qvariant]")
{
   // emerald
}
