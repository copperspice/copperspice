GUI_DIRS += src/gui/math3d

GUI_PUBLIC_INCLUDES += \
	QGenericMatrix \
	QMatrix2x2 \
	QMatrix2x3 \
	QMatrix2x4 \
	QMatrix3x2 \
	QMatrix3x3 \
	QMatrix3x4 \
	QMatrix4x2 \
	QMatrix4x3 \
	QMatrix4x4 \
	QQuaternion \
	QVector2D \
	QVector3D \
	QVector4D

GUI_MATH3D_INCLUDES = \
	qgenericmatrix.h \
	qmatrix2x2.h \
	qmatrix2x3.h \
	qmatrix2x4.h \
	qmatrix3x2.h \
	qmatrix3x3.h \
	qmatrix3x4.h \
	qmatrix4x2.h \
	qmatrix4x3.h \
	qmatrix4x4.h \
	qquaternion.h \
	qvector2d.h \
	qvector3d.h \
	qvector4d.h

#GUI_MATH3D_PRIVATE_INCLUDES =

GUI_INCLUDES += $(GUI_MATH3D_INCLUDES)
#GUI_PRIVATE_INCLUDES += $(GUI_MATH3D_PRIVATE_INCLUDES)

noinst_HEADERS+=$(patsubst %,src/gui/math3d/%,$(GUI_MATH3D_INCLUDES)) \
		$(patsubst %,src/gui/math3d/%,$(GUI_MATH3D_PRIVATE_INCLUDES))

lib_libCsGui@BUILD_ABI@_la_SOURCES += \
	src/gui/math3d/qgenericmatrix.cpp \
	src/gui/math3d/qmatrix4x4.cpp \
	src/gui/math3d/qquaternion.cpp \
	src/gui/math3d/qvector2d.cpp \
	src/gui/math3d/qvector3d.cpp \
	src/gui/math3d/qvector4d.cpp

