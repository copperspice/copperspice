
CORE_DIRS += src/core/animation

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


noinst_HEADERS+=$(patsubst %,src/core/animation/%,$(CORE_ANIMATION_INCLUDES)) \
		$(patsubst %,src/core/animation/%,$(CORE_ANIMATION_PRIVATE_INCLUDES))

lib_libCsCore@BUILD_ABI@_la_SOURCES += \
	src/core/animation/qabstractanimation.cpp \
	src/core/animation/qvariantanimation.cpp \
	src/core/animation/qpropertyanimation.cpp \
	src/core/animation/qanimationgroup.cpp \
	src/core/animation/qsequentialanimationgroup.cpp \
	src/core/animation/qparallelanimationgroup.cpp \
	src/core/animation/qpauseanimation.cpp
