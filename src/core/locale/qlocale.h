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

#ifndef QLOCALE_H
#define QLOCALE_H

#include <qshareddatapointer.h>
#include <qvariant.h>

class QLocale;
class QLocalePrivate;
class QTextStream;
class QTextStreamPrivate;

Q_CORE_EXPORT uint qHash(const QLocale &key, uint seed = 0);

class Q_CORE_EXPORT QLocale
{
   CORE_CS_GADGET(QLocale)

 public:
   enum Language {
      AnyLanguage = 0,
      C = 1,
      Abaza = 2,
      Abkhazian = 3,
      Afar = 4,
      Afrikaans = 5,
      Aghem = 6,
      Akan = 7,
      Akkadian = 8,
      Akoose = 9,
      Albanian = 10,
      AmericanSignLanguage = 11,
      Amharic = 12,
      AncientNorthArabian = 13,
      AncientEgyptian = 14,
      AncientGreek = 15,
      Arabic = 16,
      Aragonese = 17,
      Aramaic = 18,
      Armenian = 19,
      Assamese = 20,
      Asturian = 21,
      Asu = 22,
      Atsam = 23,
      Avaric = 24,
      Avestan = 25,
      Aymara = 26,
      Azerbaijani = 27,
      Bafia = 28,
      Balinese = 29,
      Baluchi = 30,
      Bambara = 31,
      Bamun = 32,
      Bangla = 33,
      Basaa = 34,
      Bashkir = 35,
      Basque = 36,
      BatakToba = 37,
      Belarusian = 38,
      Bemba = 39,
      Bena = 40,
      Bhojpuri = 41,
      Bislama = 42,
      Blin = 43,
      Bodo = 44,
      Bosnian = 45,
      Breton = 46,
      Buginese = 47,
      Buhid = 48,
      Bulgarian = 49,
      Burmese = 50,
      Cantonese = 51,
      Carian = 52,
      Catalan = 53,
      Cebuano = 54,
      CentralAtlasTamazight = 55,
      CentralKurdish = 56,
      Chakma = 57,
      Chamorro = 58,
      Chechen = 59,
      Cherokee = 60,
      Chickasaw = 61,
      Chiga = 62,
      Chinese = 63,
      Church = 64,
      Chuvash = 65,
      ClassicalMandaic = 66,
      Colognian = 67,
      Coptic = 68,
      Cornish = 69,
      Corsican = 70,
      Cree = 71,
      Croatian = 72,
      Czech = 73,
      Danish = 74,
      Divehi = 75,
      Dogri = 76,
      Duala = 77,
      Dutch = 78,
      Dzongkha = 79,
      EasternCham = 80,
      EasternKayah = 81,
      Embu = 82,
      English = 83,
      Erzya = 84,
      Esperanto = 85,
      Estonian = 86,
      Etruscan = 87,
      Ewe = 88,
      Ewondo = 89,
      Faroese = 90,
      Fijian = 91,
      Filipino = 92,
      Finnish = 93,
      French = 94,
      Friulian = 95,
      Fulah = 96,
      Ga = 97,
      Gaelic = 98,
      Galician = 99,
      Ganda = 100,
      Geez = 101,
      Georgian = 102,
      German = 103,
      Gothic = 104,
      Greek = 105,
      Guarani = 106,
      Gujarati = 107,
      Gusii = 108,
      Haitian = 109,
      Hanunoo = 110,
      Haryanvi = 111,
      Hausa = 112,
      Hawaiian = 113,
      Hebrew = 114,
      Herero = 115,
      Hindi = 116,
      HiriMotu = 117,
      HmongNjua = 118,
      Ho = 119,
      Hungarian = 120,
      Icelandic = 121,
      Ido = 122,
      Igbo = 123,
      InariSami = 124,
      Indonesian = 125,
      Ingush = 126,
      Interlingua = 127,
      Interlingue = 128,
      Inuktitut = 129,
      Inupiaq = 130,
      Irish = 131,
      Italian = 132,
      Japanese = 133,
      Javanese = 134,
      Jju = 135,
      JolaFonyi = 136,
      Kabuverdianu = 137,
      Kabyle = 138,
      Kaingang = 139,
      Kako = 140,
      Kalaallisut = 141,
      Kalenjin = 142,
      Kamba = 143,
      Kannada = 144,
      Kanuri = 145,
      Kashmiri = 146,
      Kazakh = 147,
      Kenyang = 148,
      Khmer = 149,
      Kiche = 150,
      Kikuyu = 151,
      Kinyarwanda = 152,
      Komi = 153,
      Kongo = 154,
      Konkani = 155,
      Korean = 156,
      Koro = 157,
      KoyraChiini = 158,
      KoyraboroSenni = 159,
      Kpelle = 160,
      Kuanyama = 161,
      Kurdish = 162,
      Kwasio = 163,
      Kyrgyz = 164,
      Lakota = 165,
      Langi = 166,
      Lao = 167,
      LargeFloweryMiao = 168,
      Latin = 169,
      Latvian = 170,
      Lepcha = 171,
      Lezghian = 172,
      Ligurian = 173,
      Limbu = 174,
      Limburgish = 175,
      LinearA = 176,
      Lingala = 177,
      Lisu = 178,
      LiteraryChinese = 179,
      Lithuanian = 180,
      Lojban = 181,
      LowGerman = 182,
      LowerSorbian = 183,
      Lu = 184,
      LubaKatanga = 185,
      LuleSami = 186,
      Luo = 187,
      Luxembourgish = 188,
      Luyia = 189,
      Lycian = 190,
      Lydian = 191,
      Macedonian = 192,
      Machame = 193,
      Maithili = 194,
      MakhuwaMeetto = 195,
      Makonde = 196,
      Malagasy = 197,
      Malay = 198,
      Malayalam = 199,
      Maltese = 200,
      Mandingo = 201,
      ManichaeanMiddlePersian = 202,
      Manipuri = 203,
      Manx = 204,
      Maori = 205,
      Mapuche = 206,
      Marathi = 207,
      Marshallese = 208,
      Masai = 209,
      Mazanderani = 210,
      Mende = 211,
      Meroitic = 212,
      Meru = 213,
      Meta = 214,
      Mohawk = 215,
      Moksha = 216,
      Mongolian = 217,
      Morisyen = 218,
      Mundang = 219,
      Muscogee = 220,
      Nama = 221,
      NauruLanguage = 222,
      Navajo = 223,
      Ndonga = 224,
      Nepali = 225,
      Newari = 226,
      Ngiemboon = 227,
      Ngomba = 228,
      Nheengatu = 229,
      NigerianPidgin = 230,
      Nko = 231,
      NorthNdebele = 232,
      NorthernFrisian = 233,
      NorthernLuri = 234,
      NorthernSami = 235,
      NorthernSotho = 236,
      NorthernThai = 237,
      NorwegianBokmal = 238,
      NorwegianNynorsk = 239,
      Nuer = 240,
      Nyanja = 241,
      Nyankole = 242,
      Obolo = 243,
      Occitan = 244,
      Odia = 245,
      Ojibwa = 246,
      OldIrish = 247,
      OldNorse = 248,
      OldPersian = 249,
      OldTurkish = 250,
      Oromo = 251,
      Osage = 252,
      Ossetic = 253,
      Pahlavi = 254,
      Palauan = 255,
      Pali = 256,
      Papiamento = 257,
      Parthian = 258,
      Pashto = 259,
      Persian = 260,
      Phoenician = 261,
      Pijin = 262,
      Polish = 263,
      Portuguese = 264,
      Prussian = 265,
      Punjabi = 266,
      Quechua = 267,
      Rajasthani = 268,
      Rejang = 269,
      Rohingya = 270,
      Romanian = 271,
      Romansh = 272,
      Rombo = 273,
      Rundi = 274,
      Russian = 275,
      Rwa = 276,
      Sabaean = 277,
      Saho = 278,
      Sakha = 279,
      Samaritan = 280,
      Samburu = 281,
      Samoan = 282,
      Sango = 283,
      Sangu = 284,
      Sanskrit = 285,
      Santali = 286,
      Sardinian = 287,
      Saurashtra = 288,
      Sena = 289,
      Serbian = 290,
      Shambala = 291,
      Shona = 292,
      SichuanYi = 293,
      Sicilian = 294,
      Sidamo = 295,
      Silesian = 296,
      Sindhi = 297,
      Sinhala = 298,
      SkoltSami = 299,
      Slovak = 300,
      Slovenian = 301,
      Soga = 302,
      Somali = 303,
      Sora = 304,
      SouthNdebele = 305,
      SouthernKurdish = 306,
      SouthernSami = 307,
      SouthernSotho = 308,
      Spanish = 309,
      StandardMoroccanTamazight = 310,
      Sundanese = 311,
      Swahili = 312,
      Swati = 313,
      Swedish = 314,
      SwissGerman = 315,
      Sylheti = 316,
      Syriac = 317,
      Tachelhit = 318,
      Tagbanwa = 319,
      Tahitian = 320,
      TaiDam = 321,
      TaiNua = 322,
      Taita = 323,
      Tajik = 324,
      Tamil = 325,
      Taroko = 326,
      Tasawaq = 327,
      Tatar = 328,
      TedimChin = 329,
      Telugu = 330,
      Teso = 331,
      Thai = 332,
      Tibetan = 333,
      Tigre = 334,
      Tigrinya = 335,
      TokPisin = 336,
      TokelauLanguage = 337,
      TokiPona = 338,
      Tongan = 339,
      Torwali = 340,
      Tsonga = 341,
      Tswana = 342,
      Turkish = 343,
      Turkmen = 344,
      TuvaluLanguage = 345,
      Tyap = 346,
      Ugaritic = 347,
      Ukrainian = 348,
      UpperSorbian = 349,
      Urdu = 350,
      Uyghur = 351,
      Uzbek = 352,
      Vai = 353,
      Venda = 354,
      Vietnamese = 355,
      Volapuk = 356,
      Vunjo = 357,
      Walloon = 358,
      Walser = 359,
      Warlpiri = 360,
      Welsh = 361,
      WesternBalochi = 362,
      WesternFrisian = 363,
      Wolaytta = 364,
      Wolof = 365,
      Xhosa = 366,
      Yangben = 367,
      Yiddish = 368,
      Yoruba = 369,
      Zarma = 370,
      Zhuang = 371,
      Zulu = 372,

      Afan = Oromo,
      Bengali = Bangla,
      Bhutani = Dzongkha,
      Byelorussian = Belarusian,
      Cambodian = Khmer,
      Chewa = Nyanja,
      Frisian = WesternFrisian,
      Greenlandic = Kalaallisut,
      Kurundi = Rundi,
      Inupiak = Inupiaq,
      Navaho = Navajo,
      RhaetoRomance = Romansh,
      Uighur = Uyghur,
      Uigur = Uyghur,

      LastLanguage = Zulu
   };

   enum Script {
      AnyScript = 0,
      AdlamScript = 1,
      AhomScript = 2,
      AnatolianHieroglyphsScript = 3,
      ArabicScript = 4,
      ArmenianScript = 5,
      AvestanScript = 6,
      BalineseScript = 7,
      BamumScript = 8,
      BanglaScript = 9,
      BassaVahScript = 10,
      BatakScript = 11,
      BhaiksukiScript = 12,
      BopomofoScript = 13,
      BrahmiScript = 14,
      BrailleScript = 15,
      BugineseScript = 16,
      BuhidScript = 17,
      CanadianAboriginalScript = 18,
      CarianScript = 19,
      CaucasianAlbanianScript = 20,
      ChakmaScript = 21,
      ChamScript = 22,
      CherokeeScript = 23,
      ChorasmianScript = 24,
      CopticScript = 25,
      CuneiformScript = 26,
      CypriotScript = 27,
      Cypro_MinoanScript = 28,
      CyrillicScript = 29,
      DeseretScript = 30,
      DevanagariScript = 31,
      Dives_AkuruScript = 32,
      DograScript = 33,
      DuployanScript = 34,
      EgyptianHieroglyphsScript = 35,
      ElbasanScript = 36,
      ElymaicScript = 37,
      EthiopicScript = 38,
      FraserScript = 39,
      GeorgianScript = 40,
      GlagoliticScript = 41,
      GothicScript = 42,
      GranthaScript = 43,
      GreekScript = 44,
      GujaratiScript = 45,
      GunjalaGondiScript = 46,
      GurmukhiScript = 47,
      HanScript = 48,
      HangulScript = 49,
      HanifiRohingyaScript = 50,
      HanunooScript = 51,
      HatranScript = 52,
      HebrewScript = 53,
      HiraganaScript = 54,
      ImperialAramaicScript = 55,
      InscriptionalPahlaviScript = 56,
      InscriptionalParthianScript = 57,
      JamoScript = 58,
      JapaneseScript = 59,
      JavaneseScript = 60,
      KaithiScript = 61,
      KannadaScript = 62,
      KatakanaScript = 63,
      KawiScript = 64,
      KayahLiScript = 65,
      KharoshthiScript = 66,
      KhitanSmallScript = 67,
      KhmerScript = 68,
      KhojkiScript = 69,
      KhudawadiScript = 70,
      KoreanScript = 71,
      LannaScript = 72,
      LaoScript = 73,
      LatinScript = 74,
      LepchaScript = 75,
      LimbuScript = 76,
      LinearAScript = 77,
      LinearBScript = 78,
      LycianScript = 79,
      LydianScript = 80,
      MahajaniScript = 81,
      MakasarScript = 82,
      MalayalamScript = 83,
      MandaeanScript = 84,
      ManichaeanScript = 85,
      MarchenScript = 86,
      MasaramGondiScript = 87,
      MedefaidrinScript = 88,
      MeiteiMayekScript = 89,
      MendeScript = 90,
      MeroiticCursiveScript = 91,
      MeroiticScript = 92,
      ModiScript = 93,
      MongolianScript = 94,
      MroScript = 95,
      MultaniScript = 96,
      MyanmarScript = 97,
      NabataeanScript = 98,
      NagMundariScript = 99,
      NandinagariScript = 100,
      NewTaiLueScript = 101,
      NewaScript = 102,
      NkoScript = 103,
      NushuScript = 104,
      NyiakengPuachueHmongScript = 105,
      OdiaScript = 106,
      OghamScript = 107,
      OlChikiScript = 108,
      OldHungarianScript = 109,
      OldItalicScript = 110,
      OldNorthArabianScript = 111,
      OldPermicScript = 112,
      OldPersianScript = 113,
      OldSogdianScript = 114,
      OldSouthArabianScript = 115,
      OldUyghurScript = 116,
      OrkhonScript = 117,
      OsageScript = 118,
      OsmanyaScript = 119,
      PahawhHmongScript = 120,
      PalmyreneScript = 121,
      PauCinHauScript = 122,
      PhagsPaScript = 123,
      PhoenicianScript = 124,
      PollardPhoneticScript = 125,
      PsalterPahlaviScript = 126,
      RejangScript = 127,
      RunicScript = 128,
      SamaritanScript = 129,
      SaurashtraScript = 130,
      SharadaScript = 131,
      ShavianScript = 132,
      SiddhamScript = 133,
      SignWritingScript = 134,
      SimplifiedHanScript = 135,
      SinhalaScript = 136,
      SogdianScript = 137,
      SoraSompengScript = 138,
      SoyomboScript = 139,
      SundaneseScript = 140,
      SylotiNagriScript = 141,
      SyriacScript = 142,
      TagalogScript = 143,
      TagbanwaScript = 144,
      TangsaScript = 145,
      TaiLeScript = 146,
      TaiVietScript = 147,
      TakriScript = 148,
      TamilScript = 149,
      TangutScript = 150,
      TeluguScript = 151,
      ThaanaScript = 152,
      ThaiScript = 153,
      TibetanScript = 154,
      TifinaghScript = 155,
      TirhutaScript = 156,
      TotoScript = 157,
      TraditionalHanScript = 158,
      UgariticScript = 159,
      VaiScript = 160,
      VarangKshitiScript = 161,
      VithkuqiScript = 162,
      WanchoScript = 163,
      YezidiScript = 164,
      YiScript = 165,
      Zanabazar_SquareScript = 166,

      SimplifiedChineseScript = SimplifiedHanScript,
      TraditionalChineseScript = TraditionalHanScript,
      BengaliScript = BanglaScript,
      OriyaScript = OdiaScript,
      MendeKikakuiScript = MendeScript,

      LastScript = Zanabazar_SquareScript
   };

   enum Country {
      AnyCountry = 0,
      Afghanistan = 1,
      AlandIslands = 2,
      Albania = 3,
      Algeria = 4,
      AmericanSamoa = 5,
      Andorra = 6,
      Angola = 7,
      Anguilla = 8,
      Antarctica = 9,
      AntiguaAndBarbuda = 10,
      Argentina = 11,
      Armenia = 12,
      Aruba = 13,
      AscensionIsland = 14,
      Australia = 15,
      Austria = 16,
      Azerbaijan = 17,
      Bahamas = 18,
      Bahrain = 19,
      Bangladesh = 20,
      Barbados = 21,
      Belarus = 22,
      Belgium = 23,
      Belize = 24,
      Benin = 25,
      Bermuda = 26,
      Bhutan = 27,
      Bolivia = 28,
      BosniaAndHerzegowina = 29,
      Botswana = 30,
      BouvetIsland = 31,
      Brazil = 32,
      BritishIndianOceanTerritory = 33,
      BritishVirginIslands = 34,
      Brunei = 35,
      Bulgaria = 36,
      BurkinaFaso = 37,
      Burundi = 38,
      Cambodia = 39,
      Cameroon = 40,
      Canada = 41,
      CanaryIslands = 42,
      CapeVerde = 43,
      CaribbeanNetherlands = 44,
      CaymanIslands = 45,
      CentralAfricanRepublic = 46,
      CeutaAndMelilla = 47,
      Chad = 48,
      Chile = 49,
      China = 50,
      ChristmasIsland = 51,
      ClippertonIsland = 52,
      CocosIslands = 53,
      Colombia = 54,
      Comoros = 55,
      CongoBrazzaville = 56,
      CongoKinshasa = 57,
      CookIslands = 58,
      CostaRica = 59,
      Croatia = 60,
      Cuba = 61,
      Curacao = 62,
      Cyprus = 63,
      Czechia = 64,
      Denmark = 65,
      DiegoGarcia = 66,
      Djibouti = 67,
      Dominica = 68,
      DominicanRepublic = 69,
      Ecuador = 70,
      Egypt = 71,
      ElSalvador = 72,
      EquatorialGuinea = 73,
      Eritrea = 74,
      Estonia = 75,
      Eswatini = 76,
      Ethiopia = 77,
      FalklandIslands = 78,
      FaroeIslands = 79,
      Fiji = 80,
      Finland = 81,
      France = 82,
      FrenchGuiana = 83,
      FrenchPolynesia = 84,
      FrenchSouthernTerritories = 85,
      Gabon = 86,
      Gambia = 87,
      Georgia = 88,
      Germany = 89,
      Ghana = 90,
      Gibraltar = 91,
      Greece = 92,
      Greenland = 93,
      Grenada = 94,
      Guadeloupe = 95,
      Guam = 96,
      Guatemala = 97,
      Guernsey = 98,
      Guinea = 99,
      GuineaBissau = 100,
      Guyana = 101,
      Haiti = 102,
      HeardAndMcDonaldIslands = 103,
      Honduras = 104,
      HongKong = 105,
      Hungary = 106,
      Iceland = 107,
      India = 108,
      Indonesia = 109,
      Iran = 110,
      Iraq = 111,
      Ireland = 112,
      IsleOfMan = 113,
      Israel = 114,
      Italy = 115,
      IvoryCoast = 116,
      Jamaica = 117,
      Japan = 118,
      Jersey = 119,
      Jordan = 120,
      Kazakhstan = 121,
      Kenya = 122,
      Kiribati = 123,
      Kosovo = 124,
      Kuwait = 125,
      Kyrgyzstan = 126,
      Laos = 127,
      Latvia = 128,
      Lebanon = 129,
      Lesotho = 130,
      Liberia = 131,
      Libya = 132,
      Liechtenstein = 133,
      Lithuania = 134,
      Luxembourg = 135,
      Macao = 136,
      Madagascar = 137,
      Malawi = 138,
      Malaysia = 139,
      Maldives = 140,
      Mali = 141,
      Malta = 142,
      MarshallIslands = 143,
      Martinique = 144,
      Mauritania = 145,
      Mauritius = 146,
      Mayotte = 147,
      Mexico = 148,
      Micronesia = 149,
      Moldova = 150,
      Monaco = 151,
      Mongolia = 152,
      Montenegro = 153,
      Montserrat = 154,
      Morocco = 155,
      Mozambique = 156,
      Myanmar = 157,
      Namibia = 158,
      NauruCountry = 159,
      Nepal = 160,
      Netherlands = 161,
      NewCaledonia = 162,
      NewZealand = 163,
      Nicaragua = 164,
      Niger = 165,
      Nigeria = 166,
      Niue = 167,
      NorfolkIsland = 168,
      NorthKorea = 169,
      NorthMacedonia = 170,
      NorthernMarianaIslands = 171,
      Norway = 172,
      Oman = 173,
      Pakistan = 174,
      Palau = 175,
      PalestinianTerritories = 176,
      Panama = 177,
      PapuaNewGuinea = 178,
      Paraguay = 179,
      Peru = 180,
      Philippines = 181,
      Pitcairn = 182,
      Poland = 183,
      Portugal = 184,
      PuertoRico = 185,
      Qatar = 186,
      Reunion = 187,
      Romania = 188,
      Russia = 189,
      Rwanda = 190,
      Samoa = 191,
      SanMarino = 192,
      SaoTomeAndPrincipe = 193,
      SaudiArabia = 194,
      Senegal = 195,
      Serbia = 196,
      Seychelles = 197,
      SierraLeone = 198,
      Singapore = 199,
      SintMaarten = 200,
      Slovakia = 201,
      Slovenia = 202,
      SolomonIslands = 203,
      Somalia = 204,
      SouthAfrica = 205,
      SouthGeorgiaAndSouthSandwichIslands = 206,
      SouthKorea = 207,
      SouthSudan = 208,
      Spain = 209,
      SriLanka = 210,
      SaintBarthelemy = 211,
      SaintHelena = 212,
      SaintKittsAndNevis = 213,
      SaintLucia = 214,
      SaintMartin = 215,
      SaintPierreAndMiquelon = 216,
      SaintVincentAndGrenadines = 217,
      Sudan = 218,
      Suriname = 219,
      SvalbardAndJanMayen = 220,
      Sweden = 221,
      Switzerland = 222,
      Syria = 223,
      Taiwan = 224,
      Tajikistan = 225,
      Tanzania = 226,
      Thailand = 227,
      TimorLeste = 228,
      Togo = 229,
      TokelauCountry = 230,
      Tonga = 231,
      TrinidadAndTobago = 232,
      TristanDaCunha = 233,
      Tunisia = 234,
      Turkey = 235,
      Turkmenistan = 236,
      TurksAndCaicosIslands = 237,
      TuvaluCountry = 238,
      Uganda = 239,
      Ukraine = 240,
      UnitedArabEmirates = 241,
      UnitedKingdom = 242,
      UnitedStates = 243,
      UnitedStatesOutlyingIslands = 244,
      UnitedStatesVirginIslands = 245,
      Uruguay = 246,
      Uzbekistan = 247,
      Vanuatu = 248,
      VaticanCity = 249,
      Venezuela = 250,
      Vietnam = 251,
      WallisAndFutuna = 252,
      WesternSahara = 253,
      World = 254,
      Yemen = 255,
      Zambia = 256,
      Zimbabwe = 257,

      Bonaire = CaribbeanNetherlands,
      CuraSao = Curacao,
      CzechRepublic = Czechia,
      DemocraticRepublicOfCongo = CongoKinshasa,
      DemocraticRepublicOfKorea = NorthKorea,
      EastTimor = TimorLeste,
      Macau = Macao,
      Macedonia = NorthMacedonia,
      PeoplesRepublicOfCongo = CongoBrazzaville,
      RepublicOfKorea = SouthKorea,
      RussianFederation = Russia,
      SyrianArabRepublic = Syria,
      Swaziland = Eswatini,
      UnitedStatesMinorOutlyingIslands = UnitedStatesOutlyingIslands,
      VaticanCityState = VaticanCity,
      WallisAndFutunaIslands = WallisAndFutuna,

      LastCountry = Zimbabwe
   };

   enum MeasurementSystem {
      MetricSystem,
      ImperialUSSystem,
      ImperialUKSystem,
      ImperialSystem = ImperialUSSystem
   };

   enum FormatType {
      LongFormat,
      ShortFormat,
      NarrowFormat
   };

   enum NumberOption {
      OmitGroupSeparator   = 0x01,
      RejectGroupSeparator = 0x02
   };
   using NumberOptions = QFlags<NumberOption>;

   enum CurrencySymbolFormat {
      CurrencyIsoCode,
      CurrencySymbol,
      CurrencyDisplayName
   };

   enum QuotationStyle {
      StandardQuotation,
      AlternateQuotation
   };

   CORE_CS_ENUM(Language)
   CORE_CS_ENUM(Country)
   CORE_CS_ENUM(Script)
   CORE_CS_ENUM(MeasurementSystem)

   QLocale();
   QLocale(const QString &name);
   QLocale(Language language, Country country);
   QLocale(Language language, Script script = Script::AnyScript, Country country = Country::AnyCountry);

   QLocale(const QLocale &other);
   QLocale(QLocale &&other);

   ~QLocale();

   QLocale &operator=(const QLocale &other);
   QLocale &operator=(QLocale &&other);

   Language language() const;
   Script script() const;
   Country country() const;
   QString name() const;

   QString bcp47Name() const;
   QString nativeLanguageName() const;
   QString nativeCountryName() const;

   // emerald, use qstringview instead of qstring
   short toShort(const QString &s, bool *ok = nullptr, int base = 0) const;
   ushort toUShort(const QString &s, bool *ok = nullptr, int base = 0) const;
   int toInt(const QString &s, bool *ok = nullptr, int base = 0) const;
   uint toUInt(const QString &s, bool *ok = nullptr, int base = 0) const;
   qint64 toLongLong(const QString &s, bool *ok = nullptr, int base = 0) const;
   quint64 toULongLong(const QString &s, bool *ok = nullptr, int base = 0) const;
   float toFloat(const QString &s, bool *ok = nullptr) const;
   double toDouble(const QString &s, bool *ok = nullptr) const;

   QString toString(qint64 value) const;
   QString toString(quint64 value) const;
   inline QString toString(short value) const;
   inline QString toString(ushort value) const;
   inline QString toString(int value) const;
   inline QString toString(uint value) const;
   QString toString(double value, char f = 'g', int prec = 6) const;
   inline QString toString(float value, char f = 'g', int prec = 6) const;

   QString toString(const QDate &date, const QString &format) const;
   QString toString(const QDate &date, FormatType formatType = LongFormat) const;
   QString toString(const QTime &time, const QString &format) const;
   QString toString(const QTime &time, FormatType formatType = LongFormat) const;
   QString toString(const QDateTime &dateTime, FormatType formatType = LongFormat) const;
   QString toString(const QDateTime &dateTime, const QString &format) const;

   QString dateFormat(FormatType formatType = LongFormat) const;
   QString timeFormat(FormatType formatType = LongFormat) const;
   QString dateTimeFormat(FormatType formatType = LongFormat) const;

   QDate toDate(const QString &string, FormatType formatType = LongFormat) const;
   QTime toTime(const QString &string, FormatType formatType = LongFormat) const;
   QDateTime toDateTime(const QString &string, FormatType formatType = LongFormat) const;
   QDate toDate(const QString &string, const QString &format) const;
   QTime toTime(const QString &string, const QString &format) const;
   QDateTime toDateTime(const QString &string, const QString &format) const;

   QChar decimalPoint() const;
   QChar groupSeparator() const;
   QChar percent() const;
   QChar zeroDigit() const;
   QChar negativeSign() const;
   QChar positiveSign() const;
   QChar exponential() const;

   QString monthName(int month, FormatType formatType = LongFormat) const;
   QString standaloneMonthName(int month, FormatType formatType = LongFormat) const;
   QString dayName(int day, FormatType formatType = LongFormat) const;
   QString standaloneDayName(int day, FormatType formatType = LongFormat) const;

   Qt::DayOfWeek firstDayOfWeek() const;
   QList<Qt::DayOfWeek> weekdays() const;

   QString amText() const;
   QString pmText() const;

   MeasurementSystem measurementSystem() const;

   Qt::LayoutDirection textDirection() const;

   QString toUpper(const QString &str) const;
   QString toLower(const QString &str) const;

   QString currencySymbol(CurrencySymbolFormat symbolFormat = CurrencySymbol) const;
   QString toCurrencyString(qint64 value, const QString &symbol = QString()) const;
   QString toCurrencyString(quint64 value, const QString &symbol = QString()) const;
   inline QString toCurrencyString(short value, const QString &symbol = QString()) const;
   inline QString toCurrencyString(ushort value, const QString &symbol = QString()) const;
   inline QString toCurrencyString(int value, const QString &symbol = QString()) const;
   inline QString toCurrencyString(uint value, const QString &symbol = QString()) const;
   QString toCurrencyString(double value, const QString &symbol = QString()) const;
   inline QString toCurrencyString(float value, const QString &symbol = QString()) const;

   QStringList uiLanguages() const;

   bool operator==(const QLocale &other) const;
   bool operator!=(const QLocale &other) const;

   static QString languageToString(Language language);
   static QString countryToString(Country country);
   static QString scriptToString(Script script);
   static void setDefault(const QLocale &locale);

   static QLocale c() {
      return QLocale(C);
   }

   static QLocale system();
   static QList<QLocale> matchingLocales(QLocale::Language language, QLocale::Script script, QLocale::Country country);

   void setNumberOptions(NumberOptions options);
   NumberOptions numberOptions() const;

   QString quoteString(const QString &str, QuotationStyle style = StandardQuotation) const;
   QString quoteString(QStringView str, QuotationStyle style = StandardQuotation) const;

   QString createSeparatedList(const QStringList &list) const;

 private:
   QLocale(QLocalePrivate &dd);
   QSharedDataPointer<QLocalePrivate> d;

   friend class QLocalePrivate;
   friend Q_CORE_EXPORT uint qHash(const QLocale &key, uint seed);
   friend class QByteArray;
   friend class QIntValidator;
   friend class QDoubleValidatorPrivate;
   friend class QTextStream;
   friend class QTextStreamPrivate;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(QLocale::NumberOptions)

inline QString QLocale::toString(short value) const
{
   return toString(qint64(value));
}

inline QString QLocale::toString(ushort value) const
{
   return toString(quint64(value));
}

inline QString QLocale::toString(int value) const
{
   return toString(qint64(value));
}

inline QString QLocale::toString(uint value) const
{
   return toString(quint64(value));
}

inline QString QLocale::toString(float value, char f, int prec) const
{
   return toString(double(value), f, prec);
}

inline QString QLocale::toCurrencyString(short value, const QString &symbol) const
{
   return toCurrencyString(qint64(value), symbol);
}

inline QString QLocale::toCurrencyString(ushort value, const QString &symbol) const
{
   return toCurrencyString(quint64(value), symbol);
}

inline QString QLocale::toCurrencyString(int value, const QString &symbol) const
{
   return toCurrencyString(qint64(value), symbol);
}

inline QString QLocale::toCurrencyString(uint value, const QString &symbol) const
{
   return toCurrencyString(quint64(value), symbol);
}

inline QString QLocale::toCurrencyString(float value, const QString &symbol) const
{
   return toCurrencyString(double(value), symbol);
}

Q_CORE_EXPORT QDataStream &operator<<(QDataStream &stream, const QLocale &locale);
Q_CORE_EXPORT QDataStream &operator>>(QDataStream &stream, QLocale &locale);

#endif // QLOCALE_H
