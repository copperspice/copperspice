GUI_DIRS += src/gui/embedded

GUI_PUBLIC_INCLUDES += \
	QAuthDevice \
	QCopChannel

GUI_EMBEDDED_INCLUDES = \
	qauthdevice.h \
	qtransportauth_qws.h \
	qcopchannel.h \
	qcopchannel_qws.h

#GUI_EMBEDDED_PRIVATE_INCLUDES =

GUI_INCLUDES += $(GUI_EMBEDDED_INCLUDES)
#GUI_PRIVATE_INCLUDES += $(GUI_EMBEDDED_PRIVATE_INCLUDES)

noinst_HEADERS+=$(patsubst %,src/gui/embedded/%,$(GUI_EMBEDDED_INCLUDES)) \
		$(patsubst %,src/gui/embedded/%,$(GUI_EMBEDDED_PRIVATE_INCLUDES))



