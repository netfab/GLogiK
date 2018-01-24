
lib_LTLIBRARIES += libGLogiKDBus.la

libGLogiKDBus_la_CXXFLAGS = \
						@AM_CXXFLAGS@ \
						$(DBUS_CFLAGS)

libGLogiKDBus_la_LDFLAGS = \
						@AM_LDFLAGS@

libGLogiKDBus_la_LIBADD = \
				$(DBUS_LIBS) \
				libGLogiKUtils.la

#	events/GKDBusEventStringToVoid.cpp events/GKDBusEventStringToVoid.h

libGLogiKDBus_la_SOURCES = \
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
	%D%/messages/GKDBusMessage.h \
	%D%/messages/GKDBusReply.cpp \
	%D%/messages/GKDBusReply.h \
	%D%/messages/GKDBusErrorReply.cpp \
	%D%/messages/GKDBusErrorReply.h \
	%D%/messages/GKDBusRemoteMethodCall.cpp \
	%D%/messages/GKDBusRemoteMethodCall.h \
	%D%/messages/GKDBusBroadcastSignal.cpp \
	%D%/messages/GKDBusBroadcastSignal.h \
	%D%/messages/GKDBusTargetsSignal.cpp \
	%D%/messages/GKDBusTargetsSignal.h \
	%D%/events/GKDBusEvent.cpp \
	%D%/events/GKDBusEvent.h \
	%D%/events/GKDBusEventVoidToVoid.cpp \
	%D%/events/GKDBusEventVoidToVoid.h \
	%D%/events/GKDBusEventStringToBool.cpp \
	%D%/events/GKDBusEventStringToBool.h \
	%D%/events/GKDBusEventTwoStringsToBool.cpp \
	%D%/events/GKDBusEventTwoStringsToBool.h \
	%D%/events/GKDBusEventStringToString.cpp \
	%D%/events/GKDBusEventStringToString.h \
	%D%/events/GKDBusEventTwoStringsToString.cpp \
	%D%/events/GKDBusEventTwoStringsToString.h \
	%D%/events/GKDBusEventStringToStringsArray.cpp \
	%D%/events/GKDBusEventStringToStringsArray.h \
	%D%/events/GKDBusEventTwoStringsToStringsArray.cpp \
	%D%/events/GKDBusEventTwoStringsToStringsArray.h \
	%D%/events/GKDBusEventTwoStringsOneByteToBool.cpp \
	%D%/events/GKDBusEventTwoStringsOneByteToBool.h \
	%D%/events/GKDBusEventTwoStringsThreeBytesToBool.cpp \
	%D%/events/GKDBusEventTwoStringsThreeBytesToBool.h \
	%D%/events/GKDBusEventTwoStringsOneByteOneMacrosBankToBool.cpp \
	%D%/events/GKDBusEventTwoStringsOneByteOneMacrosBankToBool.h \
	%D%/events/GKDBusEventThreeStringsOneByteToMacro.cpp \
	%D%/events/GKDBusEventThreeStringsOneByteToMacro.h \
	%D%/events/GKDBusEventStringsArrayToVoid.cpp \
	%D%/events/GKDBusEventStringsArrayToVoid.h \
	%D%/GKDBusConnection.h \
	%D%/GKDBus.cpp GKDBus.h \
	%D%/GKDBusEvents.cpp \
	%D%/GKDBusEvents.h
