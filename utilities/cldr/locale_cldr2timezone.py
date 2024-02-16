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

# Script to parse the CLDR supplemental/windowsZones.xml for use in QTimeZone

# XML structure is as follows:
#
# <supplementalData>
#     <version number="$Revision: 7825 $"/>
#     <generation date="$Date: 2012-10-10 14:45:31 -0700 (Wed, 10 Oct 2012) $"/>
#     <windowsZones>
#         <mapTimezones otherVersion="7dc0101" typeVersion="2012f">
#             <!-- (UTC-08:00) Pacific Time (US & Canada) -->
#             <mapZone other="Pacific Standard Time" territory="001" type="America/Los_Angeles"/>
#             <mapZone other="Pacific Standard Time" territory="CA"  type="America/Vancouver America/Dawson America/Whitehorse"/>
#             <mapZone other="Pacific Standard Time" territory="MX"  type="America/Tijuana"/>
#             <mapZone other="Pacific Standard Time" territory="US"  type="America/Los_Angeles"/>
#             <mapZone other="Pacific Standard Time" territory="ZZ"  type="PST8PDT"/>
#       </mapTimezones>
#     </windowsZones>
# </supplementalData>

import os
import sys
import datetime
import tempfile
import re

import cs_enumdata
import cs_findpath
# from cs_findpath import DraftResolution

findAlias       = cs_findpath.findAlias
findEntry       = cs_findpath.findEntry
findEntryInFile = cs_findpath._findEntryInFile
findTagsInFile  = cs_findpath.findTagsInFile

# next two are identical to those in locale_xml2locale.py
def unicode2hex(s):
    lst = []
    for x in s:
        v = ord(x)
        if v > 0xFFFF:
            # make a surrogate pair, copied from qchar.h
            high = (v >> 10) + 0xd7c0
            low = (v % 0x400 + 0xdc00)
            lst.append(hex(high))
            lst.append(hex(low))
        else:
            lst.append(hex(v))
    return lst

def wrap_list(lst):
    def split(lst, size):
        for i in range(int(len(lst)/size+1)):
            yield lst[i*size:(i+1)*size]
    return ",\n".join(map(lambda x: ", ".join(x), split(lst, 20)))

class ByteArrayData:
    def __init__(self):
        self.data = []
        self.hash = {}
    def append(self, s):
        s = s + '\0'
        if s in self.hash:
            return self.hash[s]

        lst   = unicode2hex(s)
        index = len(self.data)
        if index > 65535:
            print("\n\n\n#error Data index is too large")
            sys.stderr.write ("\n\n\nERROR: index exceeds the uint16 range! index = %d\n" % index)
            sys.exit(1)
        self.hash[s] = index
        self.data += lst
        return index

# Currently known IDs, if this script fails due to a missing ID add it here
# This is not public so it can be safely changed.
# Windows Key : [ Windows Id, Offset Seconds ]
windowsIdList = {
    1: [ u'Afghanistan Standard Time',        16200  ],
    2: [ u'Alaskan Standard Time',           -32400  ],
    3: [ u'Aleutian Standard Time',          -36000  ],
    4: [ u'Altai Standard Time',              25200  ],
    5: [ u'Arab Standard Time',               10800  ],
    6: [ u'Arabian Standard Time',            14400  ],
    7: [ u'Arabic Standard Time',             10800  ],
    8: [ u'Argentina Standard Time',         -10800  ],
    9: [ u'Astrakhan Standard Time',          14400  ],
   10: [ u'Atlantic Standard Time',          -14400  ],
   11: [u'AUS Central Standard Time',         34200  ],
   12: [u'Aus Central W. Standard Time',      31500  ],
   13: [u'AUS Eastern Standard Time',         36000  ],
   14: [u'Azerbaijan Standard Time',          14400  ],
   15: [u'Azores Standard Time',              -3600  ],
   16: [u'Bahia Standard Time',              -10800  ],
   17: [u'Bangladesh Standard Time',          21600  ],
   18: [u'Belarus Standard Time',             10800  ],
   19: [u'Bougainville Standard Time',        39600  ],
   20: [u'Canada Central Standard Time',     -21600  ],
   21: [u'Cape Verde Standard Time',          -3600  ],
   22: [u'Caucasus Standard Time',            14400  ],
   23: [u'Cen. Australia Standard Time',      34200  ],
   24: [u'Central America Standard Time',    -21600  ],
   25: [u'Central Asia Standard Time',        21600  ],
   26: [u'Central Brazilian Standard Time',  -14400  ],
   27: [u'Central Europe Standard Time',       3600  ],
   28: [u'Central European Standard Time',     3600  ],
   29: [u'Central Pacific Standard Time',     39600  ],
   30: [u'Central Standard Time (Mexico)',   -21600  ],
   31: [u'Central Standard Time',            -21600  ],
   32: [u'China Standard Time',               28800  ],
   33: [u'Chatham Islands Standard Time',     45900  ],
   34: [u'Cuba Standard Time',               -18000  ],
   35: [u'Dateline Standard Time',           -43200  ],
   36: [u'E. Africa Standard Time',           10800  ],
   37: [u'E. Australia Standard Time',        36000  ],
   38: [u'E. Europe Standard Time',            7200  ],
   39: [u'E. South America Standard Time',   -10800  ],
   40: [u'Easter Island Standard Time',      -21600  ],
   41: [u'Eastern Standard Time',            -18000  ],
   42: [u'Eastern Standard Time (Mexico)',   -18000  ],
   43: [u'Egypt Standard Time',                7200  ],
   44: [u'Ekaterinburg Standard Time',        18000  ],
   45: [u'Fiji Standard Time',                43200  ],
   46: [u'FLE Standard Time',                  7200  ],
   47: [u'Georgian Standard Time',            14400  ],
   48: [u'GMT Standard Time',                     0  ],
   49: [u'Greenland Standard Time',          -10800  ],
   50: [u'Greenwich Standard Time',               0  ],
   51: [u'GTB Standard Time',                  7200  ],
   52: [u'Haiti Standard Time',              -18000  ],
   53: [u'Hawaiian Standard Time',           -36000  ],
   54: [u'India Standard Time',               19800  ],
   55: [u'Iran Standard Time',                12600  ],
   56: [u'Israel Standard Time',               7200  ],
   57: [u'Jordan Standard Time',               7200  ],
   58: [u'Kaliningrad Standard Time',          7200  ],
   59: [u'Korea Standard Time',               32400  ],
   60: [u'Libya Standard Time',                7200  ],
   61: [u'Line Islands Standard Time',        50400  ],
   62: [u'Lord Howe Standard Time',           37800  ],
   63: [u'Magadan Standard Time',             36000  ],
   64: [u'Magallanes Standard Time',         -10800  ],
   65: [u'Marquesas Standard Time',          -34200  ],
   67: [u'Mauritius Standard Time',           14400  ],
   68: [u'Middle East Standard Time',          7200  ],
   69: [u'Montevideo Standard Time',         -10800  ],
   70: [u'Morocco Standard Time',                 0  ],
   71: [u'Mountain Standard Time (Mexico)',  -25200  ],
   72: [u'Mountain Standard Time',           -25200  ],
   73: [u'Myanmar Standard Time',             23400  ],
   74: [u'N. Central Asia Standard Time',     21600  ],
   75: [u'Namibia Standard Time',              3600  ],
   76: [u'Nepal Standard Time',               20700  ],
   77: [u'New Zealand Standard Time',         43200  ],
   78: [u'Newfoundland Standard Time',       -12600  ],
   79: [u'Norfolk Standard Time',             39600  ],
   80: [u'North Asia East Standard Time',     28800  ],
   81: [u'North Asia Standard Time',          25200  ],
   82: [u'North Korea Standard Time',         30600  ],
   83: [u'Omsk Standard Time',                21600  ],
   84: [u'Pacific SA Standard Time',         -10800  ],
   85: [u'Pacific Standard Time',            -28800  ],
   86: [u'Pacific Standard Time (Mexico)',   -28800  ],
   87: [u'Pakistan Standard Time',            18000  ],
   88: [u'Paraguay Standard Time',           -14400  ],
   89: [u'Qyzylorda Standard Time',           18000  ],
   90: [u'Romance Standard Time',              3600  ],
   91: [u'Russia Time Zone 3',                14400  ],
   92: [u'Russia Time Zone 10',               39600  ],
   93: [u'Russia Time Zone 11',               43200  ],
   94: [u'Russian Standard Time',             10800  ],
   95: [u'SA Eastern Standard Time',         -10800  ],
   96: [u'SA Pacific Standard Time',         -18000  ],
   97: [u'SA Western Standard Time',         -14400  ],
   98: [u'Saint Pierre Standard Time',       -10800  ],
   99: [u'Sakhalin Standard Time',            39600  ],
  100: [u'Samoa Standard Time',               46800  ],
  101: [u'Sao Tome Standard Time',                0  ],
  102: [u'Saratov Standard Time',             14400  ],
  103: [u'SE Asia Standard Time',             25200  ],
  104: [u'Singapore Standard Time',           28800  ],
  105: [u'South Africa Standard Time',         7200  ],
  106: [u'South Sudan Standard Time',          7200  ],
  107: [u'Sri Lanka Standard Time',           19800  ],
  108: [u'Sudan Standard Time',                7200  ],
  109: [u'Syria Standard Time',                7200  ],
  110: [u'Taipei Standard Time',              28800  ],
  111: [u'Tasmania Standard Time',            36000  ],
  112: [u'Tocantins Standard Time',          -10800  ],
  113: [u'Tokyo Standard Time',               32400  ],
  114: [u'Tomsk Standard Time',               25200  ],
  115: [u'Tonga Standard Time',               46800  ],
  116: [u'Transbaikal Standard Time',         32400  ],
  117: [u'Turkey Standard Time',               7200  ],
  118: [u'Turks And Caicos Standard Time',   -14400  ],
  119: [u'Ulaanbaatar Standard Time',         28800  ],
  120: [u'US Eastern Standard Time',         -18000  ],
  121: [u'US Mountain Standard Time',        -25200  ],
  122: [u'UTC-11',                           -39600  ],
  123: [u'UTC-09',                           -32400  ],
  124: [u'UTC-08',                           -28800  ],
  125: [u'UTC-02',                            -7200  ],
  126: [u'UTC',                                   0  ],
  127: [u'UTC+12',                            43200  ],
  128: [u'UTC+13',                            46800  ],
  129: [u'Venezuela Standard Time',          -16200  ],
  130: [u'Vladivostok Standard Time',         36000  ],
  131: [u'Volgograd Standard Time',           14400  ],
  132: [u'W. Australia Standard Time',        28800  ],
  133: [u'W. Central Africa Standard Time',    3600  ],
  134: [u'W. Europe Standard Time',            3600  ],
  135: [u'W. Mongolia Standard Time',         25200  ],
  136: [u'West Asia Standard Time',           18000  ],
  137: [u'West Bank Standard Time',            7200  ],
  138: [u'West Pacific Standard Time',        36000  ],
  139: [u'Yakutsk Standard Time',             32400  ],
  140: [u'Yukon Standard Time',              -25200  ]
}

def windowsIdToKey(windowsId):
    for windowsKey in windowsIdList:
        if windowsIdList[windowsKey][0] == windowsId:
            return windowsKey
    return 0

# List of standard UTC IDs
# This is not public so it can be safely changed.
# Key : [ UTC Id, Offset Seconds ]
utcIdList = {
    0 : [ u'UTC',            0  ],  # default should be first
    1 : [ u'UTC-14:00', -50400  ],
    2 : [ u'UTC-13:00', -46800  ],
    3 : [ u'UTC-12:00', -43200  ],
    4 : [ u'UTC-11:00', -39600  ],
    5 : [ u'UTC-10:00', -36000  ],
    6 : [ u'UTC-09:00', -32400  ],
    7 : [ u'UTC-08:00', -28800  ],
    8 : [ u'UTC-07:00', -25200  ],
    9 : [ u'UTC-06:00', -21600  ],
   10 : [ u'UTC-05:00', -18000  ],
   11 : [ u'UTC-04:30', -16200  ],
   12 : [ u'UTC-04:00', -14400  ],
   13 : [ u'UTC-03:30', -12600  ],
   14 : [ u'UTC-03:00', -10800  ],
   15 : [ u'UTC-02:00',  -7200  ],
   16 : [ u'UTC-01:00',  -3600  ],
   17 : [ u'UTC-00:00',      0  ],
   18 : [ u'UTC+00:00',      0  ],
   19 : [ u'UTC+01:00',   3600  ],
   20 : [ u'UTC+02:00',   7200  ],
   21 : [ u'UTC+03:00',  10800  ],
   22 : [ u'UTC+03:30',  12600  ],
   23 : [ u'UTC+04:00',  14400  ],
   24 : [ u'UTC+04:30',  16200  ],
   25 : [ u'UTC+05:00',  18000  ],
   26 : [ u'UTC+05:30',  19800  ],
   27 : [ u'UTC+05:45',  20700  ],
   28 : [ u'UTC+06:00',  21600  ],
   29 : [ u'UTC+06:30',  23400  ],
   30 : [ u'UTC+07:00',  25200  ],
   31 : [ u'UTC+08:00',  28800  ],
   32 : [ u'UTC+09:00',  32400  ],
   33 : [ u'UTC+09:30',  34200  ],
   34 : [ u'UTC+10:00',  36000  ],
   35 : [ u'UTC+11:00',  39600  ],
   36 : [ u'UTC+12:00',  43200  ],
   37 : [ u'UTC+13:00',  46800  ],
   38 : [ u'UTC+14:00',  50400  ]
}

CLDR_INPUT = "data/"

def usage():
    print("Usage: locale_cldr2timezone.py <path-to-cs-src>")
    sys.exit()

def main():
   if len(sys.argv) != 2:
       print("Missing CopperSpice path parameter")
       usage()

   cs_source  = sys.argv[1]

   if not os.path.isdir(cs_source):
       print("Issue: Path to CopperSpice source does not exist")
       usage()

   # configure output for utf-8
   sys.stdout.reconfigure(encoding='utf-8')

   windowsZonesPath = CLDR_INPUT + "supplemental/windowsZones.xml"
   dataFilePath     = cs_source  + "/src/core/datetime/qtimezone_data_p.h"

   if not os.path.isfile(windowsZonesPath):
       print("Issue: windowsZones.xml is missing")
       usage()

   if not os.path.isfile(dataFilePath):
       print("Issue: qtimezone_data_p.h is missing")
       usage()

   cldr_version = 'unknown'
   ldml = open(CLDR_INPUT + "dtd/ldml.dtd", "r")
   for line in ldml:
       if 'version cldrVersion CDATA #FIXED' in line:
           cldr_version = line.split('"')[1]

   # [[u'version', [(u'number', u'$Revision: 1234 $')]]]
   versionNumber = findTagsInFile(windowsZonesPath, "version")[0][1][0][1]
   mapTimezones  = findTagsInFile(windowsZonesPath, "windowsZones/mapTimezones")

   defaultDict   = {}
   windowsIdDict = {}

   if mapTimezones:
       for mapZone in mapTimezones:
           # [u'mapZone', [(u'territory', u'MH'), (u'other', u'UTC+12'), (u'type', u'Pacific/Majuro Pacific/Kwajalein')]]
           if mapZone[0] == u'mapZone':
               data = {}
               for attribute in mapZone[1]:
                   if attribute[0] == u'other':
                       data['windowsId'] = attribute[1]
                   if attribute[0] == u'territory':
                       data['countryCode'] = attribute[1]
                   if attribute[0] == u'type':
                       data['ianaList'] = attribute[1]

               data['windowsKey'] = windowsIdToKey(data['windowsId'])
               if data['windowsKey'] <= 0:
                   raise cs_findpath.Error("Unknown Windows ID, please add \"%s\"" % data['windowsId'])

               countryId = 0
               if data['countryCode'] == u'001':
                   defaultDict[data['windowsKey']] = data['ianaList']
               else:
                   data['countryId'] = cs_enumdata.countryCodeToId(data['countryCode'])
                   if data['countryId'] < 0:
                       raise cs_findpath.Error("Unknown Country Code \"%s\"" % data['countryCode'])
                   data['country'] = cs_enumdata.country_list[data['countryId']][0]
                   windowsIdDict[data['windowsKey'], data['countryId']] = data

   print("Input file parsed, writing data...")

   GENERATED_BLOCK_START = "// GENERATED PART STARTS HERE\n"
   GENERATED_BLOCK_END   = "// GENERATED PART ENDS HERE\n"

   # Create a temp file to write the new data into
   (newTempFile, newTempFilePath) = tempfile.mkstemp("_qtimezone_data_p.h", dir="output")
   newTempFile = os.fdopen(newTempFile, "w")

   # Open the old file and copy over the first non-generated section to the new file
   oldDataFile = open(dataFilePath, "r")
   s = oldDataFile.readline()

   while s and s != GENERATED_BLOCK_START:
       newTempFile.write(s)
       s = oldDataFile.readline()

   # Write out generated block start tag and warning
   newTempFile.write(GENERATED_BLOCK_START)
   newTempFile.write("\n\
   /*\n\
       This part of the file was generated on %s from the\n\
       Common Locale Data Repository v%s supplemental/windowsZones.xml file %s\n\
   \n\
       https://www.unicode.org/cldr/\n\
   \n\
       Do not change this data, only generate it using locale_cldr2timezone.py.\n\
   */\n\n" % (str(datetime.date.today()), cldr_version, versionNumber) )

   windowsIdData = ByteArrayData()
   ianaIdData    = ByteArrayData()

   # Write Windows/IANA table
   newTempFile.write("// Windows ID Key, Country Enum, IANA ID Index\n")
   newTempFile.write("static const QZoneData zoneDataTable[] = {\n")
   for index in windowsIdDict:
       data = windowsIdDict[index]
       newTempFile.write("    { %6d,%6d,%6d }, // %s / %s\n" \
                            % (data['windowsKey'],
                               data['countryId'],
                               ianaIdData.append(data['ianaList']),
                               data['windowsId'],
                               data['country']))
   newTempFile.write("    {      0,     0,     0 } // Trailing zeroes\n")
   newTempFile.write("};\n\n")

   print("Parsing Windows Data Table...")

   # Write Windows ID key table
   newTempFile.write("// Windows ID Key, Windows ID Index, IANA ID Index, UTC Offset\n")
   newTempFile.write("static const QWindowsData windowsDataTable[] = {\n")
   for windowsKey in windowsIdList:
       newTempFile.write("    { %6d,%6d,%6d,%6d }, // %s\n" \
                            % (windowsKey,
                               windowsIdData.append(windowsIdList[windowsKey][0]),
                               ianaIdData.append(defaultDict[windowsKey]),
                               windowsIdList[windowsKey][1],
                               windowsIdList[windowsKey][0]))
   newTempFile.write("    {      0,     0,     0,     0 } // Trailing zeroes\n")
   newTempFile.write("};\n\n")

   print("Parsing UTC Data Table...")

   # Write UTC ID key table
   newTempFile.write("// IANA ID Index, UTC Offset\n")
   newTempFile.write("static const QUtcData utcDataTable[] = {\n")
   for index in utcIdList:
       data = utcIdList[index]
       newTempFile.write("    { %6d,%6d }, // %s\n" \
                            % (ianaIdData.append(data[0]),
                               data[1],
                               data[0]))
   newTempFile.write("    {     0,      0 } // Trailing zeroes\n")
   newTempFile.write("};\n\n")

   print("Parsing Id Table...")

   # Write out Windows ID's data
   newTempFile.write("static const char windowsIdData[] = {\n")
   newTempFile.write(wrap_list(windowsIdData.data))
   newTempFile.write("\n};\n\n")

   # Write out IANA ID's data
   newTempFile.write("static const char ianaIdData[] = {\n")
   newTempFile.write(wrap_list(ianaIdData.data))
   newTempFile.write("\n};\n")

   print("Writing output file...")

   # Write out the end of generated block tag
   newTempFile.write(GENERATED_BLOCK_END)
   s = oldDataFile.readline()

   # Skip through the old generated data in the old file
   while s and s != GENERATED_BLOCK_END:
       s = oldDataFile.readline()

   # Now copy the rest of the original file into the new file
   s = oldDataFile.readline()
   while s:
       newTempFile.write(s)
       s = oldDataFile.readline()

if __name__ == "__main__":
    main()