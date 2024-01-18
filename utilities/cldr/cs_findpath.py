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

import sys
import os
import xml.dom.minidom
from functools import reduce

class DraftResolution:
    # See http://www.unicode.org/cldr/process.html for description
    unconfirmed = 'unconfirmed'
    provisional = 'provisional'
    contributed = 'contributed'
    approved    = 'approved'
    _values     = { unconfirmed : 1, provisional : 2, contributed : 3, approved : 4 }
    def __init__(self, resolution):
        self.resolution = resolution
    def toInt(self):
        return DraftResolution._values[self.resolution]

class Error(BaseException):
    def __init__(self, msg):
        self.msg = msg
    def __str__(self):
        return self.msg

doc_cache = {}
def parseDoc(file):
    if not file in doc_cache:
        doc_cache[file] = xml.dom.minidom.parse(file)
    return doc_cache[file]

def findChild(parent, tag_name, arg_name=None, arg_value=None, draft=None):
    for node in parent.childNodes:
        if node.nodeType != node.ELEMENT_NODE:
            continue
        if node.nodeName != tag_name:
            continue
        if arg_value:
            if not arg_name in node.attributes:
                continue
            if node.attributes[arg_name].nodeValue != arg_value:
                continue
        if draft:
            if not 'draft' in node.attributes:
                # if draft is not specified then it's approved
                return node
            value = node.attributes['draft'].nodeValue
            value = DraftResolution(value).toInt()
            exemplar = DraftResolution(draft).toInt()
            if exemplar > value:
                continue
        return node
    return False

def findTagsInFile(file, path):
    doc = parseDoc(file)

    elt = doc.documentElement
    tag_spec_list = path.split("/")
    last_entry = None
    for i in range(len(tag_spec_list)):
        tag_spec = tag_spec_list[i]
        tag_name = tag_spec
        arg_name = 'type'
        arg_value = ''
        left_bracket = tag_spec.find('[')
        if left_bracket != -1:
            tag_name = tag_spec[:left_bracket]
            arg_value = tag_spec[left_bracket+1:-1].split("=")
            if len(arg_value) == 2:
                arg_name = arg_value[0]
                arg_value = arg_value[1]
            else:
                arg_value = arg_value[0]
        elt = findChild(elt, tag_name, arg_name, arg_value)
        if not elt:
            return None
    ret = []
    if elt.childNodes:
        for node in elt.childNodes:
            if node.attributes:
                element = [node.nodeName, None]
                element[1] = node.attributes.items()
                ret.append(element)
    else:
        if elt.attributes:
            element = [elt.nodeName, None]
            element[1] = elt.attributes.items()
            ret.append(element)
    return ret

def _findEntryInFile(file, path, draft=None, attribute=None):
    doc = parseDoc(file)

    elt = doc.documentElement
    tag_spec_list = path.split("/")
    last_entry = None
    for i in range(len(tag_spec_list)):
        tag_spec = tag_spec_list[i]
        tag_name = tag_spec
        arg_name = 'type'
        arg_value = ''
        left_bracket = tag_spec.find('[')
        if left_bracket != -1:
            tag_name = tag_spec[:left_bracket]
            arg_value = tag_spec[left_bracket+1:-1].split("=")
            if len(arg_value) == 2:
                arg_name = arg_value[0].replace("@", "").replace("'", "")
                arg_value = arg_value[1]
            else:
                arg_value = arg_value[0]
        alias = findChild(elt, 'alias')
        if alias and alias.attributes['source'].nodeValue == 'locale':
            path = alias.attributes['path'].nodeValue
            aliaspath = tag_spec_list[:i] + path.split("/")
            def resolve(x, y):
                if y == '..':
                    return x[:-1]
                return x + [y]
            # resolve all dot-dot parts of the path
            aliaspath = reduce(resolve, aliaspath, [])
            # remove attribute specification that our xpathlite doesnt support
            aliaspath = list(map(lambda x: x.replace("@type=", "").replace("'", ""), aliaspath))
            # append the remaining path
            aliaspath = aliaspath + tag_spec_list[i:]
            aliaspath = "/".join(aliaspath)
            # "locale" aliases are special - we need to start lookup from scratch
            return (None, aliaspath)
        elt = findChild(elt, tag_name, arg_name, arg_value, draft)
        if not elt:
            return ("", None)
    if attribute is not None:
        if attribute in elt.attributes:
            return (elt.attributes[attribute].nodeValue, None)
        return (None, None)
    try:
        return (elt.firstChild.nodeValue, None)
    except:
        pass
    return (None, None)

def findAlias(file):
    doc = parseDoc(file)

    alias_elt = findChild(doc.documentElement, "alias")
    if not alias_elt:
        return False
    if not 'source' in alias_elt.attributes:
        return False
    return alias_elt.attributes['source'].nodeValue

lookup_chain_cache = {}
parent_locales = {}
def _fixedLookupChain(cldr_input, filedir, name):
    if name in lookup_chain_cache:
        return lookup_chain_cache[name]

    # see http://www.unicode.org/reports/tr35/#Parent_Locales
    if not parent_locales:
        for ns in findTagsInFile(cldr_input + "supplemental/supplementalData.xml", "parentLocales"):
            tmp = {}
            parent_locale = ""
            for data in ns[1:][0]: # ns looks like this: [u'parentLocale', [(u'parent', u'root'), (u'locales', u'az_Cyrl bs_Cyrl en_Dsrt ..')]]
                tmp[data[0]] = data[1]
                if data[0] == u"parent":
                    parent_locale = data[1]
            parent_locales[parent_locale] = tmp[u"locales"].split(" ")

    items = name.split("_")
    # split locale name into items and iterate through them from back to front
    # example: az_Latn_AZ => [az_Latn_AZ, az_Latn, az]
    items = list(reversed(list(map(lambda x: "_".join(items[:x+1]), range(len(items))))))

    for i in range(len(items)):
        item = items[i]
        for parent_locale in parent_locales.keys():
            for locale in parent_locales[parent_locale]:
                if item == locale:
                    if parent_locale == u"root":
                        items = items[:i+1]
                    else:
                        items = items[:i+1] + _fixedLookupChain(cldr_input, filedir, parent_locale)
                    lookup_chain_cache[name] = items
                    return items

    lookup_chain_cache[name] = items
    return items

def _findEntry(cldr_input, base, path, draft=None, attribute=None):
    file = base
    if base.endswith(".xml"):
        filename = base
        base = base[:-4]
    else:
        file = base + ".xml"

    (filedir, filename) = os.path.split(base)

    items = _fixedLookupChain(cldr_input, filedir, filename)

    for item in items:
        file = filedir + "/" + item + ".xml"

        if os.path.isfile(file):
            alias = findAlias(file)

            if alias:
                # if alias is found we should follow it and stop processing current file
                # see http://www.unicode.org/reports/tr35/#Common_Elements
                aliasfile = os.path.dirname(file) + "/" + alias + ".xml"

                if not os.path.isfile(aliasfile):
                    raise Error("findEntry: fatal error: found an alias '%s' to '%s', but the alias file couldn't be found" % (filename, alias))
                # found an alias, recurse into parsing it
                result = _findEntry(cldr_input, aliasfile, path, draft, attribute)
                return result

            (result, aliaspath) = _findEntryInFile(file, path, draft, attribute)

            if aliaspath:
                # start lookup again because of the alias source="locale"
                return _findEntry(cldr_input, base, aliaspath, draft, attribute)
            if result:
                return result
    return None

def findEntry(cldr_input, base, path, draft=None, attribute=None):
    file = base
    if base.endswith(".xml"):
        file = base
        base = base[:-4]
    else:
        file = base + ".xml"

    (filedir, filename) = os.path.split(base)

    result = None
    while path:
        result = _findEntry(cldr_input, base, path, draft, attribute)
        if result:
            return result

        (result, aliaspath) = _findEntryInFile(filedir + "/root.xml", path, draft, attribute)
        if result:
            return result
        if not aliaspath:
            raise Error("findEntry: fatal error: %s: can not find key %s" % (filename, path))
        path = aliaspath

    return result

