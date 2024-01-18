#!/usr/bin/env python3

# **********************************************************************
#
# Copyright (c) 2012-2024 Barbara Geller
# Copyright (c) 2012-2024 Ansel Sermersheim
#
# Copyright (c) 2015 The Qt Company Ltd.
# Copyright (c) 2012-2016 Digia Plc and/or its subsidiary(-ies).
# Copyright (c) 2008-2012 Nokia Corporation and/or its subsidiary(-ies).
#
# This file is part of CopperSpice.
#
# CopperSpice is free software. You can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public License
# version 2.1 as published by the Free Software Foundation.
#
# CopperSpice is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
#
# https://www.gnu.org/licenses/
#
# **********************************************************************

# Use this script to generate CLDR xml after downloading the supplemental files from the Unicode CLDR zip

import os
import sys
import re
from xml.sax.saxutils import escape, unescape

import cs_enumdata
import cs_findpath

from cs_dateconverter import convert_date
from cs_findpath import DraftResolution

findAlias       = cs_findpath.findAlias
findEntry       = cs_findpath.findEntry
findEntryInFile = cs_findpath._findEntryInFile
findTagsInFile  = cs_findpath.findTagsInFile

def parse_number_format(patterns, data):
    # this is a very limited parsing of the number format for currency only.
    def skip_repeating_pattern(x):
        p = x.replace('0', '#').replace(',', '').replace('.', '')
        seen = False
        result = ''
        for c in p:
            if c == '#':
                if seen:
                    continue
                seen = True
            else:
                seen = False
            result = result + c
        return result
    patterns = patterns.split(';')
    result = []
    for pattern in patterns:
        pattern = skip_repeating_pattern(pattern)
        pattern = pattern.replace('#', "%1")
        # according to https://www.unicode.org/reports/tr35/#Number_Format_Patterns
        # there can be doubled or trippled currency sign, however none of the
        # locales use that.
        pattern = pattern.replace(u'\xa4', "%2")
        pattern = pattern.replace("''", "###").replace("'", '').replace("###", "'")
        pattern = pattern.replace('-', data['minus'])
        pattern = pattern.replace('+', data['plus'])
        result.append(pattern)
    return result

def parse_list_pattern_part_format(pattern):
    # this is a very limited parsing of the format for list pattern part only.
    result = ""
    result = pattern.replace("{0}", "%1")
    result = result.replace("{1}", "%2")
    result = result.replace("{2}", "%3")
    return result

def ordStr(c):
    if len(c) == 1:
        return str(ord(c))
    raise cs_findpath.Error("Unable to handle value \"%s\"" % addEscapes(c))
    return "##########"

# the following functions are supposed to fix the problem with QLocale
# returning a character instead of strings for QLocale::exponential()
# and others. So we fallback to default values in these cases.
def fixOrdStrMinus(c):
    if len(c) == 1:
        return str(ord(c))
    return str(ord('-'))

def fixOrdStrPlus(c):
    if len(c) == 1:
        return str(ord(c))
    return str(ord('+'))

def fixOrdStrExp(c):
    if len(c) == 1:
        return str(ord(c))
    return str(ord('e'))

def fixOrdStrPercent(c):
    if len(c) == 1:
        return str(ord(c))
    return str(ord('%'))

def fixOrdStrList(c):
    if len(c) == 1:
        return str(ord(c))
    return str(ord(';'))

def generateLocaleInfo(path):
    if not path.endswith(".xml"):
        return {}

    # skip legacy/compatibility ones
    alias = findAlias(path)
    if alias:
        raise cs_findpath.Error("alias to \"%s\"" % alias)

    language_code = findEntryInFile(path, "identity/language",  attribute="type")[0]
    country_code  = findEntryInFile(path, "identity/territory", attribute="type")[0]
    script_code   = findEntryInFile(path, "identity/script",    attribute="type")[0]
    variant_code  = findEntryInFile(path, "identity/variant",   attribute="type")[0]

    return _generateLocaleInfo(path, language_code, script_code, country_code, variant_code)

def _generateLocaleInfo(path, language_code, script_code, country_code, variant_code=""):
    if not path.endswith(".xml"):
        return {}

    if language_code == 'root':
        # just skip it
        return {}

    # do not support variants
    # ### actually there is only one locale with variant: en_US_POSIX
    if variant_code:
        raise cs_findpath.Error("Varients are not supported (\"%s\")" % variant_code)

    language_id = cs_enumdata.languageCodeToId(language_code)
    if language_id <= 0:
        raise cs_findpath.Error("Unknown language code \"%s\"" % language_code)
    language = cs_enumdata.language_list[language_id][0]

    script_id = cs_enumdata.scriptCodeToId(script_code)
    if script_id == -1:
        raise cs_findpath.Error("Unknown script code \"%s\"" % script_code)
    script = cs_enumdata.script_list[script_id][0]

    # handle fully qualified names with the territory
    if not country_code:
        return {}
    country_id = cs_enumdata.countryCodeToId(country_code)
    if country_id <= 0:
        raise cs_findpath.Error("Unknown country code \"%s\"" % country_code)
    country = cs_enumdata.country_list[country_id][0]

    draft = DraftResolution.contributed

    result = {}
    result['language']      = language
    result['script']        = script
    result['country']       = country
    result['language_code'] = language_code
    result['country_code']  = country_code
    result['script_code']   = script_code
    result['variant_code']  = variant_code
    result['language_id']   = language_id
    result['script_id']     = script_id
    result['country_id']    = country_id

    supplementalPath = CLDR_INPUT + "supplemental/supplementalData.xml"
    currencies = findTagsInFile(supplementalPath, "currencyData/region[iso3166=%s]"%country_code);
    result['currencyIsoCode']  = ''
    result['currencyDigits']   = 2
    result['currencyRounding'] = 1

    if currencies:
        for e in currencies:
            if e[0] == 'currency':
                tender = True
                t = list(filter(lambda x: x[0] == 'tender', e[1]))
                if t and t[0][1] == 'false':
                    tender = False;
                if tender and not list(filter(lambda x: x[0] == 'to', e[1])):
                    result['currencyIsoCode'] = list(filter(lambda x: x[0] == 'iso4217', e[1]))[0][1]
                    break

        if result['currencyIsoCode']:
            t = findTagsInFile(supplementalPath, "currencyData/fractions/info[iso4217=%s]"%result['currencyIsoCode']);
            if t and t[0][0] == 'info':
                result['currencyDigits']   = int(list(filter(lambda x: x[0] == 'digits', t[0][1]))[0][1])
                result['currencyRounding'] = int(list(filter(lambda x: x[0] == 'rounding', t[0][1]))[0][1])

    numbering_system = None
    try:
        numbering_system = findEntry(CLDR_INPUT, path, "numbers/defaultNumberingSystem")
    except cs_findpath.Error:
        pass

    def findEntryDef(path, xpath, value=''):
        try:
            return findEntry(CLDR_MAIN, path, xpath)
        except cs_findpath.Error:
            return value

    def get_number_in_system(path, xpath, numbering_system):
        if numbering_system:
            try:
                return findEntry(CLDR_INPUT, path, xpath + "[numberSystem=" + numbering_system + "]")

            except cs_findpath.Error:
                try:
                    return findEntry(CLDR_INPUT, path, xpath.replace("/symbols/", "/symbols[numberSystem=" + numbering_system + "]/"))

                except cs_findpath.Error:
                    # fallback to default
                    pass
        return findEntry(CLDR_INPUT, path, xpath)

    result['decimal'] = get_number_in_system(path, "numbers/symbols/decimal",     numbering_system)
    result['group']   = get_number_in_system(path, "numbers/symbols/group",       numbering_system)
    result['list']    = get_number_in_system(path, "numbers/symbols/list",        numbering_system)
    result['percent'] = get_number_in_system(path, "numbers/symbols/percentSign", numbering_system)

    try:
        numbering_systems = {}
        for ns in findTagsInFile(CLDR_INPUT + "supplemental/numberingSystems.xml", "numberingSystems"):
            tmp = {}
            id = ""
            for data in ns[1:][0]: # ns looks like this: [u'numberingSystem', [(u'digits', u'0123456789'), (u'type', u'numeric'), (u'id', u'latn')]]
                tmp[data[0]] = data[1]
                if data[0] == u"id":
                    id = data[1]
            numbering_systems[id] = tmp
        result['zero'] = numbering_systems[numbering_system][u"digits"][0]
    except e:
        sys.stderr.write("Native zero detection problem:\n" + str(e) + "\n")
        result['zero'] = get_number_in_system(path, "numbers/symbols/nativeZeroDigit", numbering_system)

    result['minus'] = get_number_in_system(path, "numbers/symbols/minusSign", numbering_system)
    result['plus']  = get_number_in_system(path, "numbers/symbols/plusSign", numbering_system)
    result['exp']   = get_number_in_system(path, "numbers/symbols/exponential", numbering_system).lower()

    result['quotationStart']          = findEntry(CLDR_MAIN, path, "delimiters/quotationStart")
    result['quotationEnd']            = findEntry(CLDR_MAIN, path, "delimiters/quotationEnd")
    result['alternateQuotationStart'] = findEntry(CLDR_MAIN, path, "delimiters/alternateQuotationStart")
    result['alternateQuotationEnd']   = findEntry(CLDR_MAIN, path, "delimiters/alternateQuotationEnd")

    result['listPatternPartStart']    = parse_list_pattern_part_format(findEntry(CLDR_MAIN, path, "listPatterns/listPattern/listPatternPart[start]"))
    result['listPatternPartMiddle']   = parse_list_pattern_part_format(findEntry(CLDR_MAIN, path, "listPatterns/listPattern/listPatternPart[middle]"))
    result['listPatternPartEnd']      = parse_list_pattern_part_format(findEntry(CLDR_MAIN, path, "listPatterns/listPattern/listPatternPart[end]"))
    result['listPatternPartTwo']      = parse_list_pattern_part_format(findEntry(CLDR_MAIN, path, "listPatterns/listPattern/listPatternPart[2]"))

    result['am'] = findEntry(CLDR_MAIN, path, "dates/calendars/calendar[gregorian]/dayPeriods/dayPeriodContext[format]/dayPeriodWidth[wide]/dayPeriod[am]", draft)
    result['pm'] = findEntry(CLDR_MAIN, path, "dates/calendars/calendar[gregorian]/dayPeriods/dayPeriodContext[format]/dayPeriodWidth[wide]/dayPeriod[pm]", draft)

    result['longDateFormat']  = convert_date(findEntry(CLDR_MAIN, path, "dates/calendars/calendar[gregorian]/dateFormats/dateFormatLength[full]/dateFormat/pattern"))
    result['shortDateFormat'] = convert_date(findEntry(CLDR_MAIN, path, "dates/calendars/calendar[gregorian]/dateFormats/dateFormatLength[short]/dateFormat/pattern"))
    result['longTimeFormat']  = convert_date(findEntry(CLDR_MAIN, path, "dates/calendars/calendar[gregorian]/timeFormats/timeFormatLength[full]/timeFormat/pattern"))
    result['shortTimeFormat'] = convert_date(findEntry(CLDR_MAIN, path, "dates/calendars/calendar[gregorian]/timeFormats/timeFormatLength[short]/timeFormat/pattern"))

    endonym = None
    if country_code and script_code:
        endonym = findEntryDef(path, "localeDisplayNames/languages/language[type=%s_%s_%s]" % (language_code, script_code, country_code))
    if not endonym and script_code:
        endonym = findEntryDef(path, "localeDisplayNames/languages/language[type=%s_%s]" % (language_code, script_code))
    if not endonym and country_code:
        endonym = findEntryDef(path, "localeDisplayNames/languages/language[type=%s_%s]" % (language_code, country_code))
    if not endonym:
        endonym = findEntryDef(path, "localeDisplayNames/languages/language[type=%s]" % (language_code))
    result['language_endonym'] = endonym
    result['country_endonym'] = findEntryDef(path, "localeDisplayNames/territories/territory[type=%s]" % (country_code))

    currency_format = get_number_in_system(path, "numbers/currencyFormats/currencyFormatLength/currencyFormat/pattern", numbering_system)
    currency_format = parse_number_format(currency_format, result)
    result['currencyFormat'] = currency_format[0]
    result['currencyNegativeFormat'] = ''
    if len(currency_format) > 1:
        result['currencyNegativeFormat'] = currency_format[1]

    result['currencySymbol']      = ''
    result['currencyDisplayName'] = ''
    if result['currencyIsoCode']:
        result['currencySymbol'] = findEntryDef(path, "numbers/currencies/currency[%s]/symbol" % result['currencyIsoCode'])
        display_name_path = "numbers/currencies/currency[%s]/displayName" % result['currencyIsoCode']
        result['currencyDisplayName'] \
            = findEntryDef(path, display_name_path) + ";" \
            + findEntryDef(path, display_name_path + "[count=zero]")  + ";" \
            + findEntryDef(path, display_name_path + "[count=one]")   + ";" \
            + findEntryDef(path, display_name_path + "[count=two]")   + ";" \
            + findEntryDef(path, display_name_path + "[count=few]")   + ";" \
            + findEntryDef(path, display_name_path + "[count=many]")  + ";" \
            + findEntryDef(path, display_name_path + "[count=other]") + ";"

    standalone_long_month_path = "dates/calendars/calendar[gregorian]/months/monthContext[stand-alone]/monthWidth[wide]/month"
    result['standaloneLongMonths'] \
        = findEntry(CLDR_MAIN, path, standalone_long_month_path + "[1]") + ";" \
        + findEntry(CLDR_MAIN, path, standalone_long_month_path + "[2]") + ";" \
        + findEntry(CLDR_MAIN, path, standalone_long_month_path + "[3]") + ";" \
        + findEntry(CLDR_MAIN, path, standalone_long_month_path + "[4]") + ";" \
        + findEntry(CLDR_MAIN, path, standalone_long_month_path + "[5]") + ";" \
        + findEntry(CLDR_MAIN, path, standalone_long_month_path + "[6]") + ";" \
        + findEntry(CLDR_MAIN, path, standalone_long_month_path + "[7]") + ";" \
        + findEntry(CLDR_MAIN, path, standalone_long_month_path + "[8]") + ";" \
        + findEntry(CLDR_MAIN, path, standalone_long_month_path + "[9]") + ";" \
        + findEntry(CLDR_MAIN, path, standalone_long_month_path + "[10]") + ";" \
        + findEntry(CLDR_MAIN, path, standalone_long_month_path + "[11]") + ";" \
        + findEntry(CLDR_MAIN, path, standalone_long_month_path + "[12]") + ";"

    standalone_short_month_path = "dates/calendars/calendar[gregorian]/months/monthContext[stand-alone]/monthWidth[abbreviated]/month"
    result['standaloneShortMonths'] \
        = findEntry(CLDR_MAIN, path, standalone_short_month_path + "[1]") + ";" \
        + findEntry(CLDR_MAIN, path, standalone_short_month_path + "[2]") + ";" \
        + findEntry(CLDR_MAIN, path, standalone_short_month_path + "[3]") + ";" \
        + findEntry(CLDR_MAIN, path, standalone_short_month_path + "[4]") + ";" \
        + findEntry(CLDR_MAIN, path, standalone_short_month_path + "[5]") + ";" \
        + findEntry(CLDR_MAIN, path, standalone_short_month_path + "[6]") + ";" \
        + findEntry(CLDR_MAIN, path, standalone_short_month_path + "[7]") + ";" \
        + findEntry(CLDR_MAIN, path, standalone_short_month_path + "[8]") + ";" \
        + findEntry(CLDR_MAIN, path, standalone_short_month_path + "[9]") + ";" \
        + findEntry(CLDR_MAIN, path, standalone_short_month_path + "[10]") + ";" \
        + findEntry(CLDR_MAIN, path, standalone_short_month_path + "[11]") + ";" \
        + findEntry(CLDR_MAIN, path, standalone_short_month_path + "[12]") + ";"

    standalone_narrow_month_path = "dates/calendars/calendar[gregorian]/months/monthContext[stand-alone]/monthWidth[narrow]/month"
    result['standaloneNarrowMonths'] \
        = findEntry(CLDR_MAIN, path, standalone_narrow_month_path + "[1]") + ";" \
        + findEntry(CLDR_MAIN, path, standalone_narrow_month_path + "[2]") + ";" \
        + findEntry(CLDR_MAIN, path, standalone_narrow_month_path + "[3]") + ";" \
        + findEntry(CLDR_MAIN, path, standalone_narrow_month_path + "[4]") + ";" \
        + findEntry(CLDR_MAIN, path, standalone_narrow_month_path + "[5]") + ";" \
        + findEntry(CLDR_MAIN, path, standalone_narrow_month_path + "[6]") + ";" \
        + findEntry(CLDR_MAIN, path, standalone_narrow_month_path + "[7]") + ";" \
        + findEntry(CLDR_MAIN, path, standalone_narrow_month_path + "[8]") + ";" \
        + findEntry(CLDR_MAIN, path, standalone_narrow_month_path + "[9]") + ";" \
        + findEntry(CLDR_MAIN, path, standalone_narrow_month_path + "[10]") + ";" \
        + findEntry(CLDR_MAIN, path, standalone_narrow_month_path + "[11]") + ";" \
        + findEntry(CLDR_MAIN, path, standalone_narrow_month_path + "[12]") + ";"

    long_month_path = "dates/calendars/calendar[gregorian]/months/monthContext[format]/monthWidth[wide]/month"
    result['longMonths'] \
        = findEntry(CLDR_MAIN, path, long_month_path + "[1]") + ";" \
        + findEntry(CLDR_MAIN, path, long_month_path + "[2]") + ";" \
        + findEntry(CLDR_MAIN, path, long_month_path + "[3]") + ";" \
        + findEntry(CLDR_MAIN, path, long_month_path + "[4]") + ";" \
        + findEntry(CLDR_MAIN, path, long_month_path + "[5]") + ";" \
        + findEntry(CLDR_MAIN, path, long_month_path + "[6]") + ";" \
        + findEntry(CLDR_MAIN, path, long_month_path + "[7]") + ";" \
        + findEntry(CLDR_MAIN, path, long_month_path + "[8]") + ";" \
        + findEntry(CLDR_MAIN, path, long_month_path + "[9]") + ";" \
        + findEntry(CLDR_MAIN, path, long_month_path + "[10]") + ";" \
        + findEntry(CLDR_MAIN, path, long_month_path + "[11]") + ";" \
        + findEntry(CLDR_MAIN, path, long_month_path + "[12]") + ";"

    short_month_path = "dates/calendars/calendar[gregorian]/months/monthContext[format]/monthWidth[abbreviated]/month"
    result['shortMonths'] \
        = findEntry(CLDR_MAIN, path, short_month_path + "[1]") + ";" \
        + findEntry(CLDR_MAIN, path, short_month_path + "[2]") + ";" \
        + findEntry(CLDR_MAIN, path, short_month_path + "[3]") + ";" \
        + findEntry(CLDR_MAIN, path, short_month_path + "[4]") + ";" \
        + findEntry(CLDR_MAIN, path, short_month_path + "[5]") + ";" \
        + findEntry(CLDR_MAIN, path, short_month_path + "[6]") + ";" \
        + findEntry(CLDR_MAIN, path, short_month_path + "[7]") + ";" \
        + findEntry(CLDR_MAIN, path, short_month_path + "[8]") + ";" \
        + findEntry(CLDR_MAIN, path, short_month_path + "[9]") + ";" \
        + findEntry(CLDR_MAIN, path, short_month_path + "[10]") + ";" \
        + findEntry(CLDR_MAIN, path, short_month_path + "[11]") + ";" \
        + findEntry(CLDR_MAIN, path, short_month_path + "[12]") + ";"

    narrow_month_path = "dates/calendars/calendar[gregorian]/months/monthContext[format]/monthWidth[narrow]/month"
    result['narrowMonths'] \
        = findEntry(CLDR_MAIN, path, narrow_month_path + "[1]") + ";" \
        + findEntry(CLDR_MAIN, path, narrow_month_path + "[2]") + ";" \
        + findEntry(CLDR_MAIN, path, narrow_month_path + "[3]") + ";" \
        + findEntry(CLDR_MAIN, path, narrow_month_path + "[4]") + ";" \
        + findEntry(CLDR_MAIN, path, narrow_month_path + "[5]") + ";" \
        + findEntry(CLDR_MAIN, path, narrow_month_path + "[6]") + ";" \
        + findEntry(CLDR_MAIN, path, narrow_month_path + "[7]") + ";" \
        + findEntry(CLDR_MAIN, path, narrow_month_path + "[8]") + ";" \
        + findEntry(CLDR_MAIN, path, narrow_month_path + "[9]") + ";" \
        + findEntry(CLDR_MAIN, path, narrow_month_path + "[10]") + ";" \
        + findEntry(CLDR_MAIN, path, narrow_month_path + "[11]") + ";" \
        + findEntry(CLDR_MAIN, path, narrow_month_path + "[12]") + ";"

    long_day_path = "dates/calendars/calendar[gregorian]/days/dayContext[format]/dayWidth[wide]/day"
    result['longDays'] \
        = findEntry(CLDR_MAIN, path, long_day_path + "[sun]") + ";" \
        + findEntry(CLDR_MAIN, path, long_day_path + "[mon]") + ";" \
        + findEntry(CLDR_MAIN, path, long_day_path + "[tue]") + ";" \
        + findEntry(CLDR_MAIN, path, long_day_path + "[wed]") + ";" \
        + findEntry(CLDR_MAIN, path, long_day_path + "[thu]") + ";" \
        + findEntry(CLDR_MAIN, path, long_day_path + "[fri]") + ";" \
        + findEntry(CLDR_MAIN, path, long_day_path + "[sat]") + ";"

    short_day_path = "dates/calendars/calendar[gregorian]/days/dayContext[format]/dayWidth[abbreviated]/day"
    result['shortDays'] \
        = findEntry(CLDR_MAIN, path, short_day_path + "[sun]") + ";" \
        + findEntry(CLDR_MAIN, path, short_day_path + "[mon]") + ";" \
        + findEntry(CLDR_MAIN, path, short_day_path + "[tue]") + ";" \
        + findEntry(CLDR_MAIN, path, short_day_path + "[wed]") + ";" \
        + findEntry(CLDR_MAIN, path, short_day_path + "[thu]") + ";" \
        + findEntry(CLDR_MAIN, path, short_day_path + "[fri]") + ";" \
        + findEntry(CLDR_MAIN, path, short_day_path + "[sat]") + ";"

    narrow_day_path = "dates/calendars/calendar[gregorian]/days/dayContext[format]/dayWidth[narrow]/day"
    result['narrowDays'] \
        = findEntry(CLDR_MAIN, path, narrow_day_path + "[sun]") + ";" \
        + findEntry(CLDR_MAIN, path, narrow_day_path + "[mon]") + ";" \
        + findEntry(CLDR_MAIN, path, narrow_day_path + "[tue]") + ";" \
        + findEntry(CLDR_MAIN, path, narrow_day_path + "[wed]") + ";" \
        + findEntry(CLDR_MAIN, path, narrow_day_path + "[thu]") + ";" \
        + findEntry(CLDR_MAIN, path, narrow_day_path + "[fri]") + ";" \
        + findEntry(CLDR_MAIN, path, narrow_day_path + "[sat]") + ";"

    standalone_long_day_path = "dates/calendars/calendar[gregorian]/days/dayContext[stand-alone]/dayWidth[wide]/day"
    result['standaloneLongDays'] \
        = findEntry(CLDR_MAIN, path, standalone_long_day_path + "[sun]") + ";" \
        + findEntry(CLDR_MAIN, path, standalone_long_day_path + "[mon]") + ";" \
        + findEntry(CLDR_MAIN, path, standalone_long_day_path + "[tue]") + ";" \
        + findEntry(CLDR_MAIN, path, standalone_long_day_path + "[wed]") + ";" \
        + findEntry(CLDR_MAIN, path, standalone_long_day_path + "[thu]") + ";" \
        + findEntry(CLDR_MAIN, path, standalone_long_day_path + "[fri]") + ";" \
        + findEntry(CLDR_MAIN, path, standalone_long_day_path + "[sat]") + ";"

    standalone_short_day_path = "dates/calendars/calendar[gregorian]/days/dayContext[stand-alone]/dayWidth[abbreviated]/day"
    result['standaloneShortDays'] \
        = findEntry(CLDR_MAIN, path, standalone_short_day_path + "[sun]") + ";" \
        + findEntry(CLDR_MAIN, path, standalone_short_day_path + "[mon]") + ";" \
        + findEntry(CLDR_MAIN, path, standalone_short_day_path + "[tue]") + ";" \
        + findEntry(CLDR_MAIN, path, standalone_short_day_path + "[wed]") + ";" \
        + findEntry(CLDR_MAIN, path, standalone_short_day_path + "[thu]") + ";" \
        + findEntry(CLDR_MAIN, path, standalone_short_day_path + "[fri]") + ";" \
        + findEntry(CLDR_MAIN, path, standalone_short_day_path + "[sat]") + ";"

    standalone_narrow_day_path = "dates/calendars/calendar[gregorian]/days/dayContext[stand-alone]/dayWidth[narrow]/day"
    result['standaloneNarrowDays'] \
        = findEntry(CLDR_MAIN, path, standalone_narrow_day_path + "[sun]") + ";" \
        + findEntry(CLDR_MAIN, path, standalone_narrow_day_path + "[mon]") + ";" \
        + findEntry(CLDR_MAIN, path, standalone_narrow_day_path + "[tue]") + ";" \
        + findEntry(CLDR_MAIN, path, standalone_narrow_day_path + "[wed]") + ";" \
        + findEntry(CLDR_MAIN, path, standalone_narrow_day_path + "[thu]") + ";" \
        + findEntry(CLDR_MAIN, path, standalone_narrow_day_path + "[fri]") + ";" \
        + findEntry(CLDR_MAIN, path, standalone_narrow_day_path + "[sat]") + ";"

    return result

def addEscapes(s):
    result = ''
    for c in s:
        n = ord(c)
        if n < 128:
            result += c
        else:
            result += "\\x"
            result += "%02x" % (n)
    return result

def unicodeStr(s):
    utf8 = s.encode('utf-8')
    return "<size>" + str(len(utf8)) + "</size><data>" + addEscapes(utf8) + "</data>"

def integrateWeekData(filePath):
    if not filePath.endswith(".xml"):
        return {}
    monFirstDayIn = findEntryInFile(filePath, "weekData/firstDay[day=mon]", attribute="territories")[0].split(" ")
    tueFirstDayIn = findEntryInFile(filePath, "weekData/firstDay[day=tue]", attribute="territories")[0].split(" ")
    wedFirstDayIn = findEntryInFile(filePath, "weekData/firstDay[day=wed]", attribute="territories")[0].split(" ")
    thuFirstDayIn = findEntryInFile(filePath, "weekData/firstDay[day=thu]", attribute="territories")[0].split(" ")
    friFirstDayIn = findEntryInFile(filePath, "weekData/firstDay[day=fri]", attribute="territories")[0].split(" ")
    satFirstDayIn = findEntryInFile(filePath, "weekData/firstDay[day=sat]", attribute="territories")[0].split(" ")
    sunFirstDayIn = findEntryInFile(filePath, "weekData/firstDay[day=sun]", attribute="territories")[0].split(" ")

    monWeekendStart = findEntryInFile(filePath, "weekData/weekendStart[day=mon]", attribute="territories")[0].split(" ")
    tueWeekendStart = findEntryInFile(filePath, "weekData/weekendStart[day=tue]", attribute="territories")[0].split(" ")
    wedWeekendStart = findEntryInFile(filePath, "weekData/weekendStart[day=wed]", attribute="territories")[0].split(" ")
    thuWeekendStart = findEntryInFile(filePath, "weekData/weekendStart[day=thu]", attribute="territories")[0].split(" ")
    friWeekendStart = findEntryInFile(filePath, "weekData/weekendStart[day=fri]", attribute="territories")[0].split(" ")
    satWeekendStart = findEntryInFile(filePath, "weekData/weekendStart[day=sat]", attribute="territories")[0].split(" ")
    sunWeekendStart = findEntryInFile(filePath, "weekData/weekendStart[day=sun]", attribute="territories")[0].split(" ")

    monWeekendEnd = findEntryInFile(filePath, "weekData/weekendEnd[day=mon]", attribute="territories")[0].split(" ")
    tueWeekendEnd = findEntryInFile(filePath, "weekData/weekendEnd[day=tue]", attribute="territories")[0].split(" ")
    wedWeekendEnd = findEntryInFile(filePath, "weekData/weekendEnd[day=wed]", attribute="territories")[0].split(" ")
    thuWeekendEnd = findEntryInFile(filePath, "weekData/weekendEnd[day=thu]", attribute="territories")[0].split(" ")
    friWeekendEnd = findEntryInFile(filePath, "weekData/weekendEnd[day=fri]", attribute="territories")[0].split(" ")
    satWeekendEnd = findEntryInFile(filePath, "weekData/weekendEnd[day=sat]", attribute="territories")[0].split(" ")
    sunWeekendEnd = findEntryInFile(filePath, "weekData/weekendEnd[day=sun]", attribute="territories")[0].split(" ")

    firstDayByCountryCode = {}
    for countryCode in monFirstDayIn:
        firstDayByCountryCode[countryCode] = "mon"
    for countryCode in tueFirstDayIn:
        firstDayByCountryCode[countryCode] = "tue"
    for countryCode in wedFirstDayIn:
        firstDayByCountryCode[countryCode] = "wed"
    for countryCode in thuFirstDayIn:
        firstDayByCountryCode[countryCode] = "thu"
    for countryCode in friFirstDayIn:
        firstDayByCountryCode[countryCode] = "fri"
    for countryCode in satFirstDayIn:
        firstDayByCountryCode[countryCode] = "sat"
    for countryCode in sunFirstDayIn:
        firstDayByCountryCode[countryCode] = "sun"

    weekendStartByCountryCode = {}
    for countryCode in monWeekendStart:
        weekendStartByCountryCode[countryCode] = "mon"
    for countryCode in tueWeekendStart:
        weekendStartByCountryCode[countryCode] = "tue"
    for countryCode in wedWeekendStart:
        weekendStartByCountryCode[countryCode] = "wed"
    for countryCode in thuWeekendStart:
        weekendStartByCountryCode[countryCode] = "thu"
    for countryCode in friWeekendStart:
        weekendStartByCountryCode[countryCode] = "fri"
    for countryCode in satWeekendStart:
        weekendStartByCountryCode[countryCode] = "sat"
    for countryCode in sunWeekendStart:
        weekendStartByCountryCode[countryCode] = "sun"

    weekendEndByCountryCode = {}
    for countryCode in monWeekendEnd:
        weekendEndByCountryCode[countryCode] = "mon"
    for countryCode in tueWeekendEnd:
        weekendEndByCountryCode[countryCode] = "tue"
    for countryCode in wedWeekendEnd:
        weekendEndByCountryCode[countryCode] = "wed"
    for countryCode in thuWeekendEnd:
        weekendEndByCountryCode[countryCode] = "thu"
    for countryCode in friWeekendEnd:
        weekendEndByCountryCode[countryCode] = "fri"
    for countryCode in satWeekendEnd:
        weekendEndByCountryCode[countryCode] = "sat"
    for countryCode in sunWeekendEnd:
        weekendEndByCountryCode[countryCode] = "sun"

    for (key,locale) in locale_database.items():
        countryCode = locale['country_code']
        if countryCode in firstDayByCountryCode:
            locale_database[key]['firstDayOfWeek'] = firstDayByCountryCode[countryCode]
        else:
            locale_database[key]['firstDayOfWeek'] = firstDayByCountryCode["001"]

        if countryCode in weekendStartByCountryCode:
            locale_database[key]['weekendStart'] = weekendStartByCountryCode[countryCode]
        else:
            locale_database[key]['weekendStart'] = weekendStartByCountryCode["001"]

        if countryCode in weekendEndByCountryCode:
            locale_database[key]['weekendEnd'] = weekendEndByCountryCode[countryCode]
        else:
            locale_database[key]['weekendEnd'] = weekendEndByCountryCode["001"]

def _parseLocale(l):
    language = "AnyLanguage"
    script   = "AnyScript"
    country  = "AnyCountry"

    if l == "und":
        raise cs_findpath.Error("we are treating unknown locale like C")

    items = l.split("_")
    language_code = items[0]
    if language_code != "und":
        language_id = cs_enumdata.languageCodeToId(language_code)
        if language_id == -1:
            raise cs_findpath.Error("unknown language code \"%s\"" % language_code)
        language = cs_enumdata.language_list[language_id][0]

    if len(items) > 1:
        script_code = items[1]
        country_code = ""
        if len(items) > 2:
            country_code = items[2]
        if len(script_code) == 4:
            script_id = cs_enumdata.scriptCodeToId(script_code)
            if script_id == -1:
                raise cs_findpath.Error("unknown script code \"%s\"" % script_code)
            script = cs_enumdata.script_list[script_id][0]
        else:
            country_code = script_code
        if country_code:
            country_id = cs_enumdata.countryCodeToId(country_code)
            if country_id == -1:
                raise cs_findpath.Error("unknown country code \"%s\"" % country_code)
            country = cs_enumdata.country_list[country_id][0]

    return (language, script, country)


CLDR_INPUT = "data/"
CLDR_MAIN  = "data/main/"

locale_database = {}

def usage():
    print("Usage: locale_cldr2xml.py > output/cldr_output.xml")
    sys.exit()

def main():
   if len(sys.argv) != 1:
       usage()

   if not os.path.isdir(CLDR_MAIN):
       usage()

   cldr_files = os.listdir(CLDR_MAIN)

   # configure output for utf-8
   sys.stdout.reconfigure(encoding='utf-8')

   # refer to https://www.unicode.org/reports/tr35/tr35-info.html#Default_Content

   defaultContent_locales = {}

   for ns in findTagsInFile(CLDR_INPUT + "supplemental/supplementalMetadata.xml", "metadata/defaultContent"):
       for data in ns[1:][0]:
           if data[0] == u"locales":
               defaultContent_locales = data[1].split()

   for file in defaultContent_locales:
       items = file.split("_")
       if len(items) == 3:
           language_code = items[0]
           script_code   = items[1]
           country_code  = items[2]
       else:
           if len(items) != 2:
               sys.stderr.write("skipping defaultContent locale \"" + file + "\"\n")
               continue
           language_code = items[0]
           script_code   = ""
           country_code  = items[1]
           if len(country_code) == 4:
               sys.stderr.write("skipping defaultContent locale \"" + file + "\"\n")
               continue
       try:
           l = _generateLocaleInfo(CLDR_MAIN + file + ".xml", language_code, script_code, country_code)
           if not l:
               sys.stderr.write("skipping defaultContent locale \"" + file + "\"\n")
               continue
       except cs_findpath.Error as e:
           sys.stderr.write("skipping defaultContent locale \"%s\" (%s)\n" % (file, str(e)))
           continue

       locale_database[(l['language_id'], l['script_id'], l['country_id'], l['variant_code'])] = l

   for file in cldr_files:
       try:
           l = generateLocaleInfo(CLDR_MAIN + file)
           if not l:
               sys.stderr.write("skipping file \"" + file + "\"\n")
               continue
       except cs_findpath.Error as e:
           sys.stderr.write("skipping file \"%s\" (%s)\n" % (file, str(e)))
           continue

       locale_database[(l['language_id'], l['script_id'], l['country_id'], l['variant_code'])] = l

   integrateWeekData(CLDR_INPUT + "supplemental/supplementalData.xml")
   locale_keys = list(locale_database.keys())
   locale_keys.sort()

   cldr_version = 'unknown'
   ldml = open(CLDR_INPUT + "dtd/ldml.dtd", "r")
   for line in ldml:
       if 'version cldrVersion CDATA #FIXED' in line:
           cldr_version = line.split('"')[1]

   print("<localeDatabase>")
   print( "    <version>" + cldr_version + "</version>")
   print( "    <languageList>")

   for id in range(len(cs_enumdata.language_list)):
       l = cs_enumdata.language_list[id]
       print("        <language>")
       print("            <name>" + l[0] + "</name>")
       print("            <id>" + str(id) + "</id>")
       print("            <code>" + l[1] + "</code>")
       print("        </language>")
   print("    </languageList>")

   print("    <scriptList>")
   for id in range(len(cs_enumdata.script_list)):
       l = cs_enumdata.script_list[id]
       print("        <script>")
       print("            <name>" + l[0] + "</name>")
       print("            <id>" + str(id) + "</id>")
       print("            <code>" + l[1] + "</code>")
       print("        </script>")
   print("    </scriptList>")

   print("    <countryList>")
   for id in range(len(cs_enumdata.country_list)):
       l = cs_enumdata.country_list[id]
       print("        <country>")
       print("            <name>" + l[0] + "</name>")
       print("            <id>" + str(id) + "</id>")
       print("            <code>" + l[1] + "</code>")
       print("        </country>")
   print("    </countryList>")

   print("    <likelySubtags>")
   for ns in findTagsInFile(CLDR_INPUT + "supplemental/likelySubtags.xml", "likelySubtags"):
       tmp = {}
       for data in ns[1:][0]: # ns looks like this: [u'likelySubtag', [(u'from', u'aa'), (u'to', u'aa_Latn_ET')]]
           tmp[data[0]] = data[1]

       try:
           (from_language, from_script, from_country) = _parseLocale(tmp[u"from"])
       except cs_findpath.Error as e:
           sys.stderr.write("skipping likelySubtag \"%s\" -> \"%s\" (%s)\n" % (tmp[u"from"], tmp[u"to"], str(e)))
           continue
       try:
           (to_language, to_script, to_country) = _parseLocale(tmp[u"to"])
       except cs_findpath.Error as e:
           sys.stderr.write("skipping likelySubtag \"%s\" -> \"%s\" (%s)\n" % (tmp[u"from"], tmp[u"to"], str(e)))
           continue
       # substitute according to https://www.unicode.org/reports/tr35/#Likely_Subtags
       if to_country == "AnyCountry" and from_country != to_country:
           to_country = from_country
       if to_script == "AnyScript" and from_script != to_script:
           to_script = from_script

       print("        <likelySubtag>")
       print("            <from>")
       print("                <language>" + from_language + "</language>")
       print("                <script>" + from_script + "</script>")
       print("                <country>" + from_country + "</country>")
       print("            </from>")
       print("            <to>")
       print("                <language>" + to_language + "</language>")
       print("                <script>" + to_script + "</script>")
       print("                <country>" + to_country + "</country>")
       print("            </to>")
       print("        </likelySubtag>")
   print("    </likelySubtags>")

   print("    <localeList>")
   print("        <locale>")
   print("\
            <language>C</language>\n\
            <languageEndonym></languageEndonym>\n\
            <script>AnyScript</script>\n\
            <country>AnyCountry</country>\n\
            <countryEndonym></countryEndonym>\n\
            <decimal>46</decimal>\n\
            <group>44</group>\n\
            <list>59</list>\n\
            <percent>37</percent>\n\
            <zero>48</zero>\n\
            <minus>45</minus>\n\
            <plus>43</plus>\n\
            <exp>101</exp>\n\
            <quotationStart>\"</quotationStart>\n\
            <quotationEnd>\"</quotationEnd>\n\
            <alternateQuotationStart>\'</alternateQuotationStart>\n\
            <alternateQuotationEnd>\'</alternateQuotationEnd>\n\
            <listPatternPartStart>%1, %2</listPatternPartStart>\n\
            <listPatternPartMiddle>%1, %2</listPatternPartMiddle>\n\
            <listPatternPartEnd>%1, %2</listPatternPartEnd>\n\
            <listPatternPartTwo>%1, %2</listPatternPartTwo>\n\
            <am>AM</am>\n\
            <pm>PM</pm>\n\
            <firstDayOfWeek>mon</firstDayOfWeek>\n\
            <weekendStart>sat</weekendStart>\n\
            <weekendEnd>sun</weekendEnd>\n\
            <longDateFormat>EEEE, d MMMM yyyy</longDateFormat>\n\
            <shortDateFormat>d MMM yyyy</shortDateFormat>\n\
            <longTimeFormat>HH:mm:ss z</longTimeFormat>\n\
            <shortTimeFormat>HH:mm:ss</shortTimeFormat>\n\
            <standaloneLongMonths>January;February;March;April;May;June;July;August;September;October;November;December;</standaloneLongMonths>\n\
            <standaloneShortMonths>Jan;Feb;Mar;Apr;May;Jun;Jul;Aug;Sep;Oct;Nov;Dec;</standaloneShortMonths>\n\
            <standaloneNarrowMonths>J;F;M;A;M;J;J;A;S;O;N;D;</standaloneNarrowMonths>\n\
            <longMonths>January;February;March;April;May;June;July;August;September;October;November;December;</longMonths>\n\
            <shortMonths>Jan;Feb;Mar;Apr;May;Jun;Jul;Aug;Sep;Oct;Nov;Dec;</shortMonths>\n\
            <narrowMonths>1;2;3;4;5;6;7;8;9;10;11;12;</narrowMonths>\n\
            <longDays>Sunday;Monday;Tuesday;Wednesday;Thursday;Friday;Saturday;</longDays>\n\
            <shortDays>Sun;Mon;Tue;Wed;Thu;Fri;Sat;</shortDays>\n\
            <narrowDays>7;1;2;3;4;5;6;</narrowDays>\n\
            <standaloneLongDays>Sunday;Monday;Tuesday;Wednesday;Thursday;Friday;Saturday;</standaloneLongDays>\n\
            <standaloneShortDays>Sun;Mon;Tue;Wed;Thu;Fri;Sat;</standaloneShortDays>\n\
            <standaloneNarrowDays>S;M;T;W;T;F;S;</standaloneNarrowDays>\n\
            <currencyIsoCode></currencyIsoCode>\n\
            <currencySymbol></currencySymbol>\n\
            <currencyDisplayName>;;;;;;;</currencyDisplayName>\n\
            <currencyDigits>2</currencyDigits>\n\
            <currencyRounding>1</currencyRounding>\n\
            <currencyFormat>%1%2</currencyFormat>\n\
            <currencyNegativeFormat></currencyNegativeFormat>")
   print("        </locale>")

   for key in locale_keys:
       l = locale_database[key]

       print("        <locale>")
       print("            <language>" + l['language']        + "</language>")
       print("            <languageEndonym>" + escape(l['language_endonym']) + "</languageEndonym>")
       print("            <script>" + l['script']        + "</script>")
       print("            <country>"  + l['country']         + "</country>")
       print("            <countryEndonym>"  + escape(l['country_endonym']) + "</countryEndonym>")
       print("            <languagecode>" + l['language_code']        + "</languagecode>")
       print("            <scriptcode>" + l['script_code']        + "</scriptcode>")
       print("            <countrycode>"  + l['country_code']         + "</countrycode>")
       print("            <decimal>"  + ordStr(l['decimal']) + "</decimal>")
       print("            <group>"    + ordStr(l['group'])   + "</group>")
       print("            <list>"     + fixOrdStrList(l['list'])    + "</list>")
       print("            <percent>"  + fixOrdStrPercent(l['percent']) + "</percent>")
       print("            <zero>"     + ordStr(l['zero'])    + "</zero>")
       print("            <minus>"    + fixOrdStrMinus(l['minus'])   + "</minus>")
       print("            <plus>"     + fixOrdStrPlus(l['plus'])   + "</plus>")
       print("            <exp>"      + fixOrdStrExp(l['exp'])     + "</exp>")
       print("            <quotationStart>" + escape(l['quotationStart']) + "</quotationStart>")
       print("            <quotationEnd>" + escape(l['quotationEnd'])   + "</quotationEnd>")
       print("            <alternateQuotationStart>" + escape(l['alternateQuotationStart']) + "</alternateQuotationStart>")
       print("            <alternateQuotationEnd>" + escape(l['alternateQuotationEnd'])  + "</alternateQuotationEnd>")
       print("            <listPatternPartStart>" + escape(l['listPatternPartStart'])   + "</listPatternPartStart>")
       print("            <listPatternPartMiddle>" + escape(l['listPatternPartMiddle'])   + "</listPatternPartMiddle>")
       print("            <listPatternPartEnd>" + escape(l['listPatternPartEnd'])  + "</listPatternPartEnd>")
       print("            <listPatternPartTwo>" + escape(l['listPatternPartTwo'])   + "</listPatternPartTwo>")
       print("            <am>"       + escape(l['am']) + "</am>")
       print("            <pm>"       + escape(l['pm']) + "</pm>")
       print("            <firstDayOfWeek>"  + escape(l['firstDayOfWeek']) + "</firstDayOfWeek>")
       print("            <weekendStart>"  + escape(l['weekendStart']) + "</weekendStart>")
       print("            <weekendEnd>"  + escape(l['weekendEnd']) + "</weekendEnd>")
       print("            <longDateFormat>"  + escape(l['longDateFormat'])  + "</longDateFormat>")
       print("            <shortDateFormat>" + escape(l['shortDateFormat']) + "</shortDateFormat>")
       print("            <longTimeFormat>"  + escape(l['longTimeFormat'])  + "</longTimeFormat>")
       print("            <shortTimeFormat>" + escape(l['shortTimeFormat']) + "</shortTimeFormat>")
       print("            <standaloneLongMonths>" + escape(l['standaloneLongMonths'])      + "</standaloneLongMonths>")
       print("            <standaloneShortMonths>"+ escape(l['standaloneShortMonths'])      + "</standaloneShortMonths>")
       print("            <standaloneNarrowMonths>"+ escape(l['standaloneNarrowMonths'])      + "</standaloneNarrowMonths>")
       print("            <longMonths>"      + escape(l['longMonths'])      + "</longMonths>")
       print("            <shortMonths>"     + escape(l['shortMonths'])     + "</shortMonths>")
       print("            <narrowMonths>"     + escape(l['narrowMonths'])     + "</narrowMonths>")
       print("            <longDays>"        + escape(l['longDays'])        + "</longDays>")
       print("            <shortDays>"       + escape(l['shortDays'])       + "</shortDays>")
       print("            <narrowDays>"       + escape(l['narrowDays'])       + "</narrowDays>")
       print("            <standaloneLongDays>" + escape(l['standaloneLongDays'])        + "</standaloneLongDays>")
       print("            <standaloneShortDays>" + escape(l['standaloneShortDays'])       + "</standaloneShortDays>")
       print("            <standaloneNarrowDays>" + escape(l['standaloneNarrowDays'])       + "</standaloneNarrowDays>")
       print("            <currencyIsoCode>" + escape(l['currencyIsoCode']) + "</currencyIsoCode>")
       print("            <currencySymbol>" + escape(l['currencySymbol']) + "</currencySymbol>")
       print("            <currencyDisplayName>" + escape(l['currencyDisplayName']) + "</currencyDisplayName>")
       print("            <currencyDigits>" + str(l['currencyDigits']) + "</currencyDigits>")
       print("            <currencyRounding>" + str(l['currencyRounding']) + "</currencyRounding>")
       print("            <currencyFormat>" + escape(l['currencyFormat']) + "</currencyFormat>")
       print("            <currencyNegativeFormat>" + escape(l['currencyNegativeFormat']) + "</currencyNegativeFormat>")
       print("        </locale>")
   print("    </localeList>")
   print("</localeDatabase>")

if __name__ == "__main__":
    main()