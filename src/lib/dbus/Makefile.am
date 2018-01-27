
lib_LTLIBRARIES += libGKDBus.la

libGKDBus_la_CXXFLAGS = \
						@AM_CXXFLAGS@ \
						$(DBUS_CFLAGS)

libGKDBus_la_LDFLAGS = \
						@AM_LDFLAGS@

libGKDBus_la_LIBADD = \
				$(DBUS_LIBS) \
				libGKUtils.la

#	events/GKDBusEventStringToVoid.cpp events/GKDBusEventStringToVoid.h

libGKDBus_la_SOURCES = \
	%D%/arguments/GKDBusArgument.cpp \
	%D%/arguments/GKDBusArgument.h \
	%D%/arguments/GKDBusArgString.cpp \
	%D%/arguments/GKDBusArgString.h \
	%D%/arguments/GKDBusArgBoolean.cpp \
	%D%/arguments/GKDBusArgBoolean.h \
	%D%/arguments/GKDBusArgByte.cpp \
	%D%/arguments/GKDBusArgByte.h \
	%D%/arguments/GKDBusArgUInt16.cpp \
	%D%/arguments/GKDBusArgUInt16.h \
	%D%/arguments/GKDBusArgMacro.cpp \
	%D%/arguments/GKDBusArgMacro.h \
	%D%/arguments/GKDBusArgMacrosBank.cpp \
	%D%/arguments/GKDBusArgMacrosBank.h \
	%D%/messages/GKDBusMessage.cpp \
	%D%/messages/GKDBusMessage.h \
	%D%/messages/GKDBusReply.cpp \
	%D%/messages/GKDBusReply.h \
	%D%/messages/GKDBusErrorReply.cpp \
	%D%/messages/GKDBusErrorReply.h \
	%D%/messages/GKDBusRemoteMethodCall.cpp \
	%D%/messages/GKDBusRemoteMethodCall.h \
	%D%/messages/GKDBusBroadcastSignal.cpp \
	%D%/messages/GKDBusBroadcastSignal.h \
	%D%/messages/GKDBusTargetsSignal.cpp \
	%D%/messages/GKDBusTargetsSignal.h \
	%D%/events/GKDBusEvent.cpp \
	%D%/events/GKDBusEvent.h \
	%D%/events/GKDBusEventTemplates.cpp \
	%D%/events/GKDBusEventTemplates.h \
	%D%/GKDBusConnection.h \
	%D%/GKDBus.cpp \
	%D%/GKDBus.h \
	%D%/GKDBusEvents.cpp \
	%D%/GKDBusEvents.h
