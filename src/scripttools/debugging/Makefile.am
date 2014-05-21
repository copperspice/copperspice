SCRIPTTOOLS_DIRS += src/scripttools/debugging

SCRIPTTOOLS_PUBLIC_INCLUDES += \
	QScriptEngineDebugger \
	QtScriptTools


SCRIPTTOOLS_DEBUGGING_INCLUDES = \
	qscriptenginedebugger.h \
	qtscripttools.h


SCRIPTTOOLS_DEBUGGING_PRIVATE_INCLUDES = \
	qscriptbreakpointdata_p.h \
	qscriptbreakpointsmodel_p.h \
	qscriptbreakpointswidget_p.h \
	qscriptbreakpointswidgetinterface_p.h \
	qscriptbreakpointswidgetinterface_p_p.h \
	qscriptcompletionproviderinterface_p.h \
	qscriptcompletiontask_p.h \
	qscriptcompletiontaskinterface_p.h \
	qscriptcompletiontaskinterface_p_p.h \
	qscriptdebugger_p.h \
	qscriptdebuggeragent_p.h \
	qscriptdebuggeragent_p_p.h \
	qscriptdebuggerbackend_p.h \
	qscriptdebuggerbackend_p_p.h \
	qscriptdebuggercodefinderwidget_p.h \
	qscriptdebuggercodefinderwidgetinterface_p.h \
	qscriptdebuggercodefinderwidgetinterface_p_p.h \
	qscriptdebuggercodeview_p.h \
	qscriptdebuggercodeviewinterface_p.h \
	qscriptdebuggercodeviewinterface_p_p.h \
	qscriptdebuggercodewidget_p.h \
	qscriptdebuggercodewidgetinterface_p.h \
	qscriptdebuggercodewidgetinterface_p_p.h \
	qscriptdebuggercommand_p.h \
	qscriptdebuggercommandexecutor_p.h \
	qscriptdebuggercommandschedulerfrontend_p.h \
	qscriptdebuggercommandschedulerinterface_p.h \
	qscriptdebuggercommandschedulerjob_p.h \
	qscriptdebuggercommandschedulerjob_p_p.h \
	qscriptdebuggerconsole_p.h \
	qscriptdebuggerconsolecommand_p.h \
	qscriptdebuggerconsolecommand_p_p.h \
	qscriptdebuggerconsolecommandgroupdata_p.h \
	qscriptdebuggerconsolecommandjob_p.h \
	qscriptdebuggerconsolecommandjob_p_p.h \
	qscriptdebuggerconsolecommandmanager_p.h \
	qscriptdebuggerconsoleglobalobject_p.h \
	qscriptdebuggerconsolehistorianinterface_p.h \
	qscriptdebuggerconsolewidget_p.h \
	qscriptdebuggerconsolewidgetinterface_p.h \
	qscriptdebuggerconsolewidgetinterface_p_p.h \
	qscriptdebuggerevent_p.h \
	qscriptdebuggereventhandlerinterface_p.h \
	qscriptdebuggerfrontend_p.h \
	qscriptdebuggerfrontend_p_p.h \
	qscriptdebuggerjob_p.h \
	qscriptdebuggerjob_p_p.h \
	qscriptdebuggerjobschedulerinterface_p.h \
	qscriptdebuggerlocalsmodel_p.h \
	qscriptdebuggerlocalswidget_p.h \
	qscriptdebuggerlocalswidgetinterface_p.h \
	qscriptdebuggerlocalswidgetinterface_p_p.h \
	qscriptdebuggerobjectsnapshotdelta_p.h \
	qscriptdebuggerresponse_p.h \
	qscriptdebuggerresponsehandlerinterface_p.h \
	qscriptdebuggerscriptedconsolecommand_p.h \
	qscriptdebuggerscriptsmodel_p.h \
	qscriptdebuggerscriptswidget_p.h \
	qscriptdebuggerscriptswidgetinterface_p.h \
	qscriptdebuggerscriptswidgetinterface_p_p.h \
	qscriptdebuggerstackmodel_p.h \
	qscriptdebuggerstackwidget_p.h \
	qscriptdebuggerstackwidgetinterface_p.h \
	qscriptdebuggerstackwidgetinterface_p_p.h \
	qscriptdebuggerstandardwidgetfactory_p.h \
	qscriptdebuggervalue_p.h \
	qscriptdebuggervalueproperty_p.h \
	qscriptdebuggerwidgetfactoryinterface_p.h \
	qscriptdebugoutputwidget_p.h \
	qscriptdebugoutputwidgetinterface_p.h \
	qscriptdebugoutputwidgetinterface_p_p.h \
	qscriptedit_p.h \
	qscriptenginedebuggerfrontend_p.h \
	qscripterrorlogwidget_p.h \
	qscripterrorlogwidgetinterface_p.h \
	qscripterrorlogwidgetinterface_p_p.h \
	qscriptmessagehandlerinterface_p.h \
	qscriptobjectsnapshot_p.h \
	qscriptscriptdata_p.h \
	qscriptstdmessagehandler_p.h \
	qscriptsyntaxhighlighter_p.h \
	qscripttooltipproviderinterface_p.h \
	qscriptvalueproperty_p.h \
	qscriptxmlparser_p.h

SCRIPTTOOLS_INCLUDES += $(SCRIPTTOOLS_DEBUGGING_INCLUDES)
SCRIPTTOOLS_PRIVATE_INCLUDES += $(SCRIPTTOOLS_DEBUGGING_PRIVATE_INCLUDES)

noinst_HEADERS+=$(patsubst %,src/scripttools/debugging/%,$(SCRIPTTOOLS_DEBUGGING_INCLUDES)) \
		$(patsubst %,src/scripttools/debugging/%,$(SCRIPTTOOLS_DEBUGGING_PRIVATE_INCLUDES))

lib_libCsScriptTools1_la_SOURCES += \
	src/scripttools/debugging/qscriptbreakpointdata.cpp \
	src/scripttools/debugging/qscriptbreakpointsmodel.cpp \
	src/scripttools/debugging/qscriptbreakpointswidget.cpp \
	src/scripttools/debugging/qscriptbreakpointswidgetinterface.cpp \
	src/scripttools/debugging/qscriptcompletiontask.cpp \
	src/scripttools/debugging/qscriptcompletiontaskinterface.cpp \
	src/scripttools/debugging/qscriptdebugger.cpp \
	src/scripttools/debugging/qscriptdebuggeragent.cpp \
	src/scripttools/debugging/qscriptdebuggerbackend.cpp \
	src/scripttools/debugging/qscriptdebuggercodefinderwidget.cpp \
	src/scripttools/debugging/qscriptdebuggercodefinderwidgetinterface.cpp \
	src/scripttools/debugging/qscriptdebuggercodeview.cpp \
	src/scripttools/debugging/qscriptdebuggercodeviewinterface.cpp \
	src/scripttools/debugging/qscriptdebuggercodewidget.cpp \
	src/scripttools/debugging/qscriptdebuggercodewidgetinterface.cpp \
	src/scripttools/debugging/qscriptdebuggercommand.cpp \
	src/scripttools/debugging/qscriptdebuggercommandexecutor.cpp \
	src/scripttools/debugging/qscriptdebuggercommandschedulerfrontend.cpp \
	src/scripttools/debugging/qscriptdebuggercommandschedulerjob.cpp \
	src/scripttools/debugging/qscriptdebuggerconsole.cpp \
	src/scripttools/debugging/qscriptdebuggerconsolecommand.cpp \
	src/scripttools/debugging/qscriptdebuggerconsolecommandgroupdata.cpp \
	src/scripttools/debugging/qscriptdebuggerconsolecommandjob.cpp \
	src/scripttools/debugging/qscriptdebuggerconsolecommandmanager.cpp \
	src/scripttools/debugging/qscriptdebuggerconsoleglobalobject.cpp \
	src/scripttools/debugging/qscriptdebuggerconsolewidget.cpp \
	src/scripttools/debugging/qscriptdebuggerconsolewidgetinterface.cpp \
	src/scripttools/debugging/qscriptdebuggerevent.cpp \
	src/scripttools/debugging/qscriptdebuggerfrontend.cpp \
	src/scripttools/debugging/qscriptdebuggerjob.cpp \
	src/scripttools/debugging/qscriptdebuggerlocalsmodel.cpp \
	src/scripttools/debugging/qscriptdebuggerlocalswidget.cpp \
	src/scripttools/debugging/qscriptdebuggerlocalswidgetinterface.cpp \
	src/scripttools/debugging/qscriptdebuggerresponse.cpp \
	src/scripttools/debugging/qscriptdebuggerscriptedconsolecommand.cpp \
	src/scripttools/debugging/qscriptdebuggerscriptsmodel.cpp \
	src/scripttools/debugging/qscriptdebuggerscriptswidget.cpp \
	src/scripttools/debugging/qscriptdebuggerscriptswidgetinterface.cpp \
	src/scripttools/debugging/qscriptdebuggerstackmodel.cpp \
	src/scripttools/debugging/qscriptdebuggerstackwidget.cpp \
	src/scripttools/debugging/qscriptdebuggerstackwidgetinterface.cpp \
	src/scripttools/debugging/qscriptdebuggerstandardwidgetfactory.cpp \
	src/scripttools/debugging/qscriptdebuggervalue.cpp \
	src/scripttools/debugging/qscriptdebuggervalueproperty.cpp \
	src/scripttools/debugging/qscriptdebugoutputwidget.cpp \
	src/scripttools/debugging/qscriptdebugoutputwidgetinterface.cpp \
	src/scripttools/debugging/qscriptedit.cpp \
	src/scripttools/debugging/qscriptenginedebugger.cpp \
	src/scripttools/debugging/qscriptenginedebuggerfrontend.cpp \
	src/scripttools/debugging/qscripterrorlogwidget.cpp \
	src/scripttools/debugging/qscripterrorlogwidgetinterface.cpp \
	src/scripttools/debugging/qscriptobjectsnapshot.cpp \
	src/scripttools/debugging/qscriptscriptdata.cpp \
	src/scripttools/debugging/qscriptstdmessagehandler.cpp \
	src/scripttools/debugging/qscriptsyntaxhighlighter.cpp \
	src/scripttools/debugging/qscriptvalueproperty.cpp \
	src/scripttools/debugging/qscriptxmlparser.cpp  \
	src/scripttools/debugging/qrc_scripttools_debugging.cpp

EXTRA_DIST+= \
	src/scripttools/debugging/scripttools_debugging.qrc \
	src/scripttools/debugging/scripts/commands/advance.qs \
	src/scripttools/debugging/scripts/commands/backtrace.qs \
	src/scripttools/debugging/scripts/commands/break.qs \
	src/scripttools/debugging/scripts/commands/clear.qs \
	src/scripttools/debugging/scripts/commands/complete.qs \
	src/scripttools/debugging/scripts/commands/condition.qs \
	src/scripttools/debugging/scripts/commands/continue.qs \
	src/scripttools/debugging/scripts/commands/delete.qs \
	src/scripttools/debugging/scripts/commands/disable.qs \
	src/scripttools/debugging/scripts/commands/down.qs \
	src/scripttools/debugging/scripts/commands/enable.qs \
	src/scripttools/debugging/scripts/commands/eval.qs \
	src/scripttools/debugging/scripts/commands/finish.qs \
	src/scripttools/debugging/scripts/commands/frame.qs \
	src/scripttools/debugging/scripts/commands/help.qs \
	src/scripttools/debugging/scripts/commands/ignore.qs \
	src/scripttools/debugging/scripts/commands/info.qs \
	src/scripttools/debugging/scripts/commands/interrupt.qs \
	src/scripttools/debugging/scripts/commands/list.qs \
	src/scripttools/debugging/scripts/commands/next.qs \
	src/scripttools/debugging/scripts/commands/print.qs \
	src/scripttools/debugging/scripts/commands/return.qs \
	src/scripttools/debugging/scripts/commands/step.qs \
	src/scripttools/debugging/scripts/commands/tbreak.qs \
	src/scripttools/debugging/scripts/commands/up.qs \
	src/scripttools/debugging/images/interrupt.png \
	src/scripttools/debugging/images/d_interrupt.png \
	src/scripttools/debugging/images/play.png \
	src/scripttools/debugging/images/d_play.png \
	src/scripttools/debugging/images/stepinto.png \
	src/scripttools/debugging/images/stepover.png \
	src/scripttools/debugging/images/stepout.png \
	src/scripttools/debugging/images/runtocursor.png \
	src/scripttools/debugging/images/runtonewscript.png \
	src/scripttools/debugging/images/return.png \
	src/scripttools/debugging/images/reload.png \
	src/scripttools/debugging/images/location.svg \
	src/scripttools/debugging/images/breakpoint.svg \
	src/scripttools/debugging/images/d_breakpoint.svg \
	src/scripttools/debugging/images/location.png \
	src/scripttools/debugging/images/breakpoint.png \
	src/scripttools/debugging/images/d_breakpoint.png \
	src/scripttools/debugging/images/mac/closetab.png \
	src/scripttools/debugging/images/mac/next.png \
	src/scripttools/debugging/images/mac/previous.png \
	src/scripttools/debugging/images/mac/plus.png \
	src/scripttools/debugging/images/win/closetab.png \
	src/scripttools/debugging/images/win/next.png \
	src/scripttools/debugging/images/win/previous.png \
	src/scripttools/debugging/images/win/plus.png \
	src/scripttools/debugging/images/wrap.png \
	src/scripttools/debugging/images/find.png \
	src/scripttools/debugging/images/new.png \
	src/scripttools/debugging/images/delete.png
