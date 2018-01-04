
lib_LTLIBRARIES=libGLogiKDBus.la

libGLogiKDBus_la_CXXFLAGS = \
						@AM_CXXFLAGS@ \
						$(DBUS_CFLAGS)

libGLogiKDBus_la_LDFLAGS = \
						@AM_LDFLAGS@

libGLogiKDBus_la_LIBADD = \
				$(DBUS_LIBS) \
				../utils/libGLogiKUtils.la

#	events/GKDBusEventStringToVoid.cpp events/GKDBusEventStringToVoid.h

libGLogiKDBus_la_SOURCES = \
	GKDBusConnection.h \
	GKDBus.cpp GKDBus.h \
	arguments/GKDBusArgument.cpp arguments/GKDBusArgument.h \
	arguments/GKDBusArgString.cpp arguments/GKDBusArgString.h \
	arguments/GKDBusArgBoolean.cpp arguments/GKDBusArgBoolean.h \
	arguments/GKDBusArgByte.cpp arguments/GKDBusArgByte.h \
	arguments/GKDBusArgUInt16.cpp arguments/GKDBusArgUInt16.h \
	messages/GKDBusMessage.cpp messages/GKDBusMessage.h \
	messages/GKDBusReply.cpp messages/GKDBusReply.h \
	messages/GKDBusErrorReply.cpp messages/GKDBusErrorReply.h \
	messages/GKDBusRemoteMethodCall.cpp messages/GKDBusRemoteMethodCall.h \
	messages/GKDBusBroadcastSignal.cpp messages/GKDBusBroadcastSignal.h \
	messages/GKDBusTargetsSignal.cpp messages/GKDBusTargetsSignal.h \
	events/GKDBusEvent.cpp events/GKDBusEvent.h \
	events/GKDBusEventVoidToVoid.cpp events/GKDBusEventVoidToVoid.h \
	events/GKDBusEventStringToBool.cpp events/GKDBusEventStringToBool.h \
	events/GKDBusEventTwoStringsToBool.cpp events/GKDBusEventTwoStringsToBool.h \
	events/GKDBusEventStringToString.cpp events/GKDBusEventStringToString.h \
	events/GKDBusEventTwoStringsToString.cpp events/GKDBusEventTwoStringsToString.h \
	events/GKDBusEventStringToStringsArray.cpp events/GKDBusEventStringToStringsArray.h \
	events/GKDBusEventTwoStringsToStringsArray.cpp events/GKDBusEventTwoStringsToStringsArray.h \
	events/GKDBusEventTwoStringsOneByteToBool.cpp events/GKDBusEventTwoStringsOneByteToBool.h \
	events/GKDBusEventTwoStringsThreeBytesToBool.cpp events/GKDBusEventTwoStringsThreeBytesToBool.h \
	events/GKDBusEventThreeStringsOneByteToMacro.cpp events/GKDBusEventThreeStringsOneByteToMacro.h \
	events/GKDBusEventStringsArrayToVoid.cpp events/GKDBusEventStringsArrayToVoid.h \
	GKDBusEvents.cpp GKDBusEvents.h
