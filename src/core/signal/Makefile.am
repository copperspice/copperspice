
CORE_DIRS += src/core/signal

CORE_PUBLIC_INCLUDES +=

CORE_SIGNAL_INCLUDES = \
	cs_signal.h \
	cs_slot.h \
	cs_internal.h  \
	cs_macro.h  \
	rcu_guarded.hpp \
	rcu_list.hpp

CORE_SIGNAL_PRIVATE_INCLUDES =

CORE_INCLUDES += $(CORE_SIGNAL_INCLUDES)
CORE_PRIVATE_INCLUDES += $(CORE_SIGNAL_PRIVATE_INCLUDES)


noinst_HEADERS+=$(patsubst %,src/core/signal/%,$(CORE_SIGNAL_INCLUDES)) \
		$(patsubst %,src/core/signal/%,$(CORE_SIGNAL_PRIVATE_INCLUDES))

lib_libCsCore@BUILD_ABI@_la_SOURCES += \
	src/core/signal/cs_signal.cpp \
	src/core/signal/cs_slot.cpp
