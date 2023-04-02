/***********************************************************************
*
* Copyright (c) 2012-2023 Barbara Geller
* Copyright (c) 2012-2023 Ansel Sermersheim
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
        Bambara = 30,
        Bamun = 31,
        Bangla = 32,
        Basaa = 33,
        Bashkir = 34,
        Basque = 35,
        BatakToba = 36,
        Belarusian = 37,
        Bemba = 38,
        Bena = 39,
        Bhojpuri = 40,
        Bislama = 41,
        Blin = 42,
        Bodo = 43,
        Bosnian = 44,
        Breton = 45,
        Buginese = 46,
        Buhid = 47,
        Bulgarian = 48,
        Burmese = 49,
        Cantonese = 50,
        Carian = 51,
        Catalan = 52,
        Cebuano = 53,
        CentralAtlasTamazight = 54,
        CentralKurdish = 55,
        Chakma = 56,
        Chamorro = 57,
        Chechen = 58,
        Cherokee = 59,
        Chickasaw = 60,
        Chiga = 61,
        Chinese = 62,
        Church = 63,
        Chuvash = 64,
        ClassicalMandaic = 65,
        Colognian = 66,
        Coptic = 67,
        Cornish = 68,
        Corsican = 69,
        Cree = 70,
        Croatian = 71,
        Czech = 72,
        Danish = 73,
        Divehi = 74,
        Dogri = 75,
        Duala = 76,
        Dutch = 77,
        Dzongkha = 78,
        EasternCham = 79,
        EasternKayah = 80,
        Embu = 81,
        English = 82,
        Erzya = 83,
        Esperanto = 84,
        Estonian = 85,
        Etruscan = 86,
        Ewe = 87,
        Ewondo = 88,
        Faroese = 89,
        Fijian = 90,
        Filipino = 91,
        Finnish = 92,
        French = 93,
        Friulian = 94,
        Fulah = 95,
        Ga = 96,
        Gaelic = 97,
        Galician = 98,
        Ganda = 99,
        Geez = 100,
        Georgian = 101,
        German = 102,
        Gothic = 103,
        Greek = 104,
        Guarani = 105,
        Gujarati = 106,
        Gusii = 107,
        Haitian = 108,
        Hanunoo = 109,
        Hausa = 110,
        Hawaiian = 111,
        Hebrew = 112,
        Herero = 113,
        Hindi = 114,
        HiriMotu = 115,
        HmongNjua = 116,
        Ho = 117,
        Hungarian = 118,
        Icelandic = 119,
        Igbo = 120,
        InariSami = 121,
        Indonesian = 122,
        Ingush = 123,
        Interlingua = 124,
        Interlingue = 125,
        Inuktitut = 126,
        Inupiaq = 127,
        Irish = 128,
        Italian = 129,
        Japanese = 130,
        Javanese = 131,
        Jju = 132,
        JolaFonyi = 133,
        Kabuverdianu = 134,
        Kabyle = 135,
        Kako = 136,
        Kalaallisut = 137,
        Kalenjin = 138,
        Kamba = 139,
        Kannada = 140,
        Kanuri = 141,
        Kashmiri = 142,
        Kazakh = 143,
        Kenyang = 144,
        Khmer = 145,
        Kiche = 146,
        Kikuyu = 147,
        Kinyarwanda = 148,
        Komi = 149,
        Kongo = 150,
        Konkani = 151,
        Korean = 152,
        Koro = 153,
        KoyraChiini = 154,
        KoyraboroSenni = 155,
        Kpelle = 156,
        Kuanyama = 157,
        Kurdish = 158,
        Kwasio = 159,
        Kyrgyz = 160,
        Lakota = 161,
        Langi = 162,
        Lao = 163,
        LargeFloweryMiao = 164,
        Latin = 165,
        Latvian = 166,
        Lepcha = 167,
        Lezghian = 168,
        Limbu = 169,
        Limburgish = 170,
        LinearA = 171,
        Lingala = 172,
        Lisu = 173,
        LiteraryChinese = 174,
        Lithuanian = 175,
        LowGerman = 176,
        LowerSorbian = 177,
        Lu = 178,
        LubaKatanga = 179,
        LuleSami = 180,
        Luo = 181,
        Luxembourgish = 182,
        Luyia = 183,
        Lycian = 184,
        Lydian = 185,
        Macedonian = 186,
        Machame = 187,
        Maithili = 188,
        MakhuwaMeetto = 189,
        Makonde = 190,
        Malagasy = 191,
        Malay = 192,
        Malayalam = 193,
        Maltese = 194,
        Mandingo = 195,
        ManichaeanMiddlePersian = 196,
        Manipuri = 197,
        Manx = 198,
        Maori = 199,
        Mapuche = 200,
        Marathi = 201,
        Marshallese = 202,
        Masai = 203,
        Mazanderani = 204,
        Mende = 205,
        Meroitic = 206,
        Meru = 207,
        Meta = 208,
        Mohawk = 209,
        Mongolian = 210,
        Morisyen = 211,
        Mundang = 212,
        Muscogee = 213,
        Nama = 214,
        NauruLanguage = 215,
        Navajo = 216,
        Ndonga = 217,
        Nepali = 218,
        Newari = 219,
        Ngiemboon = 220,
        Ngomba = 221,
        NigerianPidgin = 222,
        Nko = 223,
        NorthNdebele = 224,
        NorthernLuri = 225,
        NorthernSami = 226,
        NorthernSotho = 227,
        NorthernThai = 228,
        NorwegianBokmal = 229,
        NorwegianNynorsk = 230,
        Nuer = 231,
        Nyanja = 232,
        Nyankole = 233,
        Occitan = 234,
        Odia = 235,
        Ojibwa = 236,
        OldIrish = 237,
        OldNorse = 238,
        OldPersian = 239,
        OldTurkish = 240,
        Oromo = 241,
        Osage = 242,
        Ossetic = 243,
        Pahlavi = 244,
        Palauan = 245,
        Pali = 246,
        Papiamento = 247,
        Parthian = 248,
        Pashto = 249,
        Persian = 250,
        Phoenician = 251,
        Polish = 252,
        Portuguese = 253,
        Prussian = 254,
        Punjabi = 255,
        Quechua = 256,
        Rejang = 257,
        Romanian = 258,
        Romansh = 259,
        Rombo = 260,
        Rundi = 261,
        Russian = 262,
        Rwa = 263,
        Sabaean = 264,
        Saho = 265,
        Sakha = 266,
        Samaritan = 267,
        Samburu = 268,
        Samoan = 269,
        Sango = 270,
        Sangu = 271,
        Sanskrit = 272,
        Santali = 273,
        Sardinian = 274,
        Saurashtra = 275,
        Sena = 276,
        Serbian = 277,
        Shambala = 278,
        Shona = 279,
        SichuanYi = 280,
        Sicilian = 281,
        Sidamo = 282,
        Silesian = 283,
        Sindhi = 284,
        Sinhala = 285,
        SkoltSami = 286,
        Slovak = 287,
        Slovenian = 288,
        Soga = 289,
        Somali = 290,
        Sora = 291,
        SouthNdebele = 292,
        SouthernKurdish = 293,
        SouthernSami = 294,
        SouthernSotho = 295,
        Spanish = 296,
        StandardMoroccanTamazight = 297,
        Sundanese = 298,
        Swahili = 299,
        Swati = 300,
        Swedish = 301,
        SwissGerman = 302,
        Sylheti = 303,
        Syriac = 304,
        Tachelhit = 305,
        Tagbanwa = 306,
        Tahitian = 307,
        TaiDam = 308,
        TaiNua = 309,
        Taita = 310,
        Tajik = 311,
        Tamil = 312,
        Taroko = 313,
        Tasawaq = 314,
        Tatar = 315,
        TedimChin = 316,
        Telugu = 317,
        Teso = 318,
        Thai = 319,
        Tibetan = 320,
        Tigre = 321,
        Tigrinya = 322,
        TokPisin = 323,
        TokelauLanguage = 324,
        Tongan = 325,
        Tsonga = 326,
        Tswana = 327,
        Turkish = 328,
        Turkmen = 329,
        TuvaluLanguage = 330,
        Tyap = 331,
        Ugaritic = 332,
        Ukrainian = 333,
        UpperSorbian = 334,
        Urdu = 335,
        Uyghur = 336,
        Uzbek = 337,
        Vai = 338,
        Venda = 339,
        Vietnamese = 340,
        Volapuk = 341,
        Vunjo = 342,
        Walloon = 343,
        Walser = 344,
        Warlpiri = 345,
        Welsh = 346,
        WesternBalochi = 347,
        WesternFrisian = 348,
        Wolaytta = 349,
        Wolof = 350,
        Xhosa = 351,
        Yangben = 352,
        Yiddish = 353,
        Yoruba = 354,
        Zarma = 355,
        Zhuang = 356,
        Zulu = 357,

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
        ArabicScript = 2,
        ArmenianScript = 3,
        AvestanScript = 4,
        BalineseScript = 5,
        BamumScript = 6,
        BanglaScript = 7,
        BassaVahScript = 8,
        BatakScript = 9,
        BhaiksukiScript = 10,
        BopomofoScript = 11,
        BrahmiScript = 12,
        BrailleScript = 13,
        BugineseScript = 14,
        BuhidScript = 15,
        CanadianAboriginalScript = 16,
        CarianScript = 17,
        CaucasianAlbanianScript = 18,
        ChakmaScript = 19,
        ChamScript = 20,
        CherokeeScript = 21,
        CopticScript = 22,
        CuneiformScript = 23,
        CypriotScript = 24,
        CyrillicScript = 25,
        DeseretScript = 26,
        DevanagariScript = 27,
        DuployanScript = 28,
        EgyptianHieroglyphsScript = 29,
        ElbasanScript = 30,
        EthiopicScript = 31,
        FraserScript = 32,
        GeorgianScript = 33,
        GlagoliticScript = 34,
        GothicScript = 35,
        GranthaScript = 36,
        GreekScript = 37,
        GujaratiScript = 38,
        GurmukhiScript = 39,
        HanScript = 40,
        HangulScript = 41,
        HanunooScript = 42,
        HebrewScript = 43,
        HiraganaScript = 44,
        ImperialAramaicScript = 45,
        InscriptionalPahlaviScript = 46,
        InscriptionalParthianScript = 47,
        JapaneseScript = 48,
        JavaneseScript = 49,
        KaithiScript = 50,
        KannadaScript = 51,
        KatakanaScript = 52,
        KayahLiScript = 53,
        KharoshthiScript = 54,
        KhmerScript = 55,
        KhojkiScript = 56,
        KhudawadiScript = 57,
        KoreanScript = 58,
        LannaScript = 59,
        LaoScript = 60,
        LatinScript = 61,
        LepchaScript = 62,
        LimbuScript = 63,
        LinearAScript = 64,
        LinearBScript = 65,
        LycianScript = 66,
        LydianScript = 67,
        MahajaniScript = 68,
        MalayalamScript = 69,
        MandaeanScript = 70,
        ManichaeanScript = 71,
        MeiteiMayekScript = 72,
        MendeScript = 73,
        MeroiticCursiveScript = 74,
        MeroiticScript = 75,
        ModiScript = 76,
        MongolianScript = 77,
        MroScript = 78,
        MyanmarScript = 79,
        NabataeanScript = 80,
        NewTaiLueScript = 81,
        NkoScript = 82,
        OdiaScript = 83,
        OghamScript = 84,
        OlChikiScript = 85,
        OldItalicScript = 86,
        OldNorthArabianScript = 87,
        OldPermicScript = 88,
        OldPersianScript = 89,
        OldSouthArabianScript = 90,
        OrkhonScript = 91,
        OsageScript = 92,
        OsmanyaScript = 93,
        PahawhHmongScript = 94,
        PalmyreneScript = 95,
        PauCinHauScript = 96,
        PhagsPaScript = 97,
        PhoenicianScript = 98,
        PollardPhoneticScript = 99,
        PsalterPahlaviScript = 100,
        RejangScript = 101,
        RunicScript = 102,
        SamaritanScript = 103,
        SaurashtraScript = 104,
        SharadaScript = 105,
        ShavianScript = 106,
        SiddhamScript = 107,
        SimplifiedHanScript = 108,
        SinhalaScript = 109,
        SoraSompengScript = 110,
        SundaneseScript = 111,
        SylotiNagriScript = 112,
        SyriacScript = 113,
        TagalogScript = 114,
        TagbanwaScript = 115,
        TaiLeScript = 116,
        TaiVietScript = 117,
        TakriScript = 118,
        TamilScript = 119,
        TeluguScript = 120,
        ThaanaScript = 121,
        ThaiScript = 122,
        TibetanScript = 123,
        TifinaghScript = 124,
        TirhutaScript = 125,
        TraditionalHanScript = 126,
        UgariticScript = 127,
        VaiScript = 128,
        VarangKshitiScript = 129,
        YiScript = 130,

        SimplifiedChineseScript = SimplifiedHanScript,
        TraditionalChineseScript = TraditionalHanScript,
        BengaliScript = BanglaScript,
        OriyaScript = OdiaScript,
        MendeKikakuiScript = MendeScript,

        LastScript = YiScript
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
        Yemen = 254,
        Zambia = 255,
        Zimbabwe = 256,

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

   // ### emerald - should return QString since unicode data contains several characters for these fields.
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

   enum QuotationStyle { StandardQuotation, AlternateQuotation };
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
