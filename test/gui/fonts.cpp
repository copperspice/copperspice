#include "cs_catch2.h"

#ifdef _MSC_VER
#pragma warning( push )
#pragma warning(disable : 4251 4244 4250 4275 4127)
#endif

#include <QString>
#include <QMainWindow>
#include <QApplication>
#include <QFontDatabase>

#ifdef _MSC_VER
#pragma warning( pop )
#endif

#include "stopwatch.h"

#include <thread>
#include <chrono>
#include <iostream>
#include <unordered_map>
#include <map>

using namespace std::chrono_literals;

TEST_CASE("create", "QFontDatabase")
{
   QApplication app(__argc, __argv);

   Stopwatch dbtimer("QFontDatabase");
   QFontDatabase db;
   auto families = db.families();
   dbtimer.stop();

   auto count = families.count();
   CHECK(count > 240);
   std::cerr << "  db.families().count(): " << count << "\n";

   int totalStyles = 0;
   int totalSizes = 0;
   for (auto& familyName: families)
   {
      auto styles = db.styles(familyName); // 2, 4 or 6 styles per family (mostly 4)
      totalStyles += styles.count();
      //std::cerr << "  styles.count(): " << styles.count() << "\n";
      for (auto& styleName: styles)
      {
         auto sizes = db.pointSizes(familyName, styleName);
         totalSizes += sizes.count(); // always 18 different point-sizes: 6, 8, 9, 10, 11, 12, 14, 16, 18, 20, 22, 24, 26, 28, 36, 48, 72
         // std::cerr << "  sizes.count(): " << sizes.count() << "\n";
         // for (auto p: sizes)
         // {
         //    std::cerr << "  p: " << p << "\n";
         // }
      }
   }

   CHECK(totalStyles > 940);
   std::cerr << "  totalStyles: " << totalStyles << "\n";
   CHECK(totalSizes > 17000);
   std::cerr << "  totalSizes: " << totalSizes << "\n";

   std::set<QString> uniques;
   for (const auto& f : families)
   {
      uniques.insert(f);
   }
   CHECK(uniques.size() == count); // checks that all the font family names are unique
}

TEST_CASE("defaultFamily", "QFont")
{
   QApplication app(__argc, __argv);

   Stopwatch defaultfontTimer("font.defaultFamily()");
   QFont font;
   auto defaultFamily = font.defaultFamily();
   std::cerr << "  defaultFamily: " << defaultFamily.toLatin1().constData() << "\n";
   defaultfontTimer.stop();
}

/*
 f: Tw Cen MT
  f: Tw Cen MT Condensed
  f: Tw Cen MT Condensed Extra Bold
  f: Verdana
  f: Viner Hand ITC
  f: Vivaldi
  f: Vladimir Script
  f: Webdings
  f: Wide Latin
  f: Wingdings
  f: Wingdings 2
  f: Wingdings 3
  f: Yu Gothic
  f: Yu Gothic Light
  f: Yu Gothic Medium
  f: Yu Gothic UI
  f: Yu Gothic UI Light
  f: Yu Gothic UI Semibold
  f: Yu Gothic UI Semilight
  */