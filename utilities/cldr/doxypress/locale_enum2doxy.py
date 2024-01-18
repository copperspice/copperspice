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

# generate doxy file for language, country, and script

#   <tr><td><tt>QLocale::AnyLanguage</tt></td>
#       <td><tt>0</tt></td><td>&nbsp;</td></tr>
#
#   <tr><td><tt>QLocale::C</tt></td><td><tt>1</tt></td>
#       <td>"C" locale is identical in behavior to English/UnitedStates.</td></tr>
#
#   <tr><td><tt>QLocale::Bengali</tt></td>
#       <td><tt>Same as Bangla</tt></td>
#       <td>&nbsp;</td></tr>

import os
import sys
import tempfile
import re

sys.path.append("..")

import cs_enumdata
import cs_extra_enums

remove_space_table = str.maketrans("", "", " -")

def remove_spaces(input):
    return input.translate(remove_space_table)

def print_language(output, name, code, extra_map):
    if name in extra_map:
        id = "Use " + extra_map[name]
    else:
        id = cs_enumdata.languageCodeToId(code)

    if name == "C":
        extratext = "Refer to QLocale::c()"
    else:
        extratext = "&nbsp;"


    output.write("""
   <tr><td><tt>QLocale::%s</tt></td>
       <td><tt>%s</tt></td>
       <td>%s</td></tr>\n""" \
          % (remove_spaces(name), id, extratext))


def print_country(output, name, code, extra_map):
    if name in extra_map:
        id = "Use " + extra_map[name]
    else:
        id = cs_enumdata.countryCodeToId(code)

    output.write("""
   <tr><td><tt>QLocale::%s</tt></td>
       <td>%s</td></tr>\n""" \
          % (remove_spaces(name), id))

def add_script(name):
    if name.endswith("Script"):
        return name
    return name + "Script"

def print_script(output, name, code, extra_map):
    if name in extra_map:
        id = "Use " + extra_map[name]
    else:
        id = cs_enumdata.scriptCodeToId(code)

    output.write("""
   <tr><td><tt>QLocale::%s</tt></td>
       <td>%s</td></tr>\n""" \
          % (add_script(remove_spaces(name)), id))


def sort_key(data):
    if data[0] == "AnyLanguage" or data[0] == "AnyCountry" or data[0] == "AnyScript":
        return " 1"                     # Starts with space so it sorts before any other vaue
    elif data[0] == "C":
        return " 2"                     # Sort after "AnyXXX" but before any real value
    elif data[0].startswith("Saint"):
        return "St" + data[0][5:]       # Treat "SaintXXX" as "StXXX"
    return data[0]


def print_block(block_list, extra_map, print_function, output):
    combined_list = block_list + list(map(lambda x: [x,""], extra_map.keys()))
    combined_list.sort(key=sort_key)

    for item in combined_list:
        print_function(output, item[0], item[1], extra_map)

def parse_string(data):
    return dict(re.findall('(\w+)\s*=\s*(\w+)', data))


def main():

    language_file = open("doxy_language.txt", "w")

    print_block(cs_enumdata.language_list, parse_string(cs_extra_enums.extra_language_string), print_language, language_file)

    country_file = open("doxy_country.txt", "w")
    print_block(cs_enumdata.country_list, parse_string(cs_extra_enums.extra_country_string), print_country, country_file)

    script_file = open("doxy_script.txt", "w")
    print_block(cs_enumdata.script_list, parse_string(cs_extra_enums.extra_script_string), print_script, script_file)


if __name__ == "__main__":
    main()
