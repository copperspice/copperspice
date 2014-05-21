
CORE_DIRS += src/corelib/animation

CORE_PUBLIC_INCLUDES += \
	QAbstractAnimation \
	QAnimationDriver \
	QAnimationGroup \
	QParallelAnimationGroup \
	QPauseAnimation \
	QPropertyAnimation \
	QSequentialAnimationGroup \
	QVariantAnimation

CORE_ANIMATION_INCLUDES = \
	qabstractanimation.h \
	qanimationdriver.h \
	qanimationgroup.h \
	qparallelanimationgroup.h \
	qpauseanimation.h \
	qpropertyanimation.h \
	qsequentialanimationgroup.h \
	qvariantanimation.h

CORE_ANIMATION_PRIVATE_INCLUDES = \
	qabstractanimation_p.h \
	qanimationgroup_p.h \
	qparallelanimationgroup_p.h \
	qpropertyanimation_p.h \
	qsequentialanimationgroup_p.h \
	qvariantanimation_p.h

CORE_INCLUDES += $(CORE_ANIMATION_INCLUDES)
CORE_PRIVATE_INCLUDES += $(CORE_ANIMATION_PRIVATE_INCLUDES)


noinst_HEADERS+=$(patsubst %,src/corelib/animation/%,$(CORE_ANIMATION_INCLUDES)) \
		$(patsubst %,src/corelib/animation/%,$(CORE_ANIMATION_PRIVATE_INCLUDES))

lib_libCsCore1_la_SOURCES += \
	src/corelib/animation/qabstractanimation.cpp \
	src/corelib/animation/qvariantanimation.cpp \
	src/corelib/animation/qpropertyanimation.cpp \
	src/corelib/animation/qanimationgroup.cpp \
	src/corelib/animation/qsequentialanimationgroup.cpp \
	src/corelib/animation/qparallelanimationgroup.cpp \
	src/corelib/animation/qpauseanimation.cpp
