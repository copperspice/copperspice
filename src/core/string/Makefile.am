
CORE_DIRS += src/core/string

CORE_PUBLIC_INCLUDES += \
	QChar32   \
	QString8  \
	QString16 \
	QStringParser \
	QStringView \
	QRegularExpression

CORE_STRING_INCLUDES = \
	cs_string.h \
	cs_string_iterator.h \
	cs_encoding.h  \
	cs_char.h   \
	cs_string_view.h \
	qchar32.h   \
	qstring8.h  \
	qstring16.h  \
	qstringparser.h \
	qstringview.h \
	qregularexpression.h

CORE_REGEX_INCLUDES = \
	regex/basic_regex.h \
	regex/basic_regex_creator.h \
	regex/basic_regex_parser.h \
	regex/iterator_category.h \
	regex/match_flags.h \
	regex/match_results.h \
	regex/perl_matcher.h \
	regex/perl_matcher_common.h \
	regex/perl_matcher_non_recursive.h \
	regex/regex.h \
	regex/regex_constants.h \
	regex/regex_fwd.h \
	regex/regex_iterator.h \
	regex/regex_match.h \
	regex/regex_merge.h \
	regex/regex_raw_buffer.h \
	regex/regex_search.h \
	regex/regex_split.h \
	regex/regex_sub_match.h \
	regex/regex_token_iterator.h \
	regex/regex_traits.h \
	regex/regex_traits_defaults.h \
	regex/r_config.h \
	regex/r_error_type.h \
	regex/r_object_cache.h \
	regex/r_pattern_except.h \
	regex/r_protected_call.h \
	regex/r_states.h


CORE_STRING_PRIVATE_INCLUDES =

CORE_INCLUDES += $(CORE_STRING_INCLUDES) $(CORE_REGEX_INCLUDES)
CORE_PRIVATE_INCLUDES += $(CORE_STRING_PRIVATE_INCLUDES)


noinst_HEADERS+=$(patsubst %,src/core/string/%,$(CORE_STRING_INCLUDES)) \
		$(patsubst %,src/core/string/%,$(CORE_STRING_PRIVATE_INCLUDES)) \
		$(patsubst %,src/core/string/%,$(CORE_REGEX_INCLUDES))


lib_libCsCore@BUILD_ABI@_la_SOURCES += \
	src/core/string/qchar32.cpp  \
	src/core/string/qstring8.cpp \
	src/core/string/qstring16.cpp \
	src/core/string/qstringparser.cpp \
	src/core/string/qstringview.cpp \
	src/core/string/regex/regex_raw_buffer.cpp
