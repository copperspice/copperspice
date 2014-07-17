
CORE_DIRS += src/core/arch

CORE_PUBLIC_INCLUDES += \
	QAtomicInt \
	QAtomicPointer

CORE_ARCH_INCLUDES = \
	qatomic_arch.h \
	qatomic_generic.h \
	qatomic_i386.h \
	qatomic_macosx.h \
	qatomic_windows.h \
	qatomic_x86_64.h \
	qatomicint.h \
	qatomicpointer.h

CORE_INCLUDES += $(CORE_ARCH_INCLUDES)

noinst_HEADERS+=$(patsubst %,src/core/arch/%,$(CORE_ARCH_INCLUDES))
