
lib_LTLIBRARIES += libGKDBus.la

libGKDBus_la_CXXFLAGS = \
						@AM_CXXFLAGS@ \
						$(DBUS_CFLAGS)

libGKDBus_la_LDFLAGS = \
						@AM_LDFLAGS@

libGKDBus_la_LIBADD = \
				$(DBUS_LIBS) \
				libGKUtils.la

libGKDBus_la_SOURCES = \
	%D%/arguments/GKDBusArgument.cpp \
	%D%/arguments/GKDBusArgument.hpp \
	%D%/arguments/GKDBusArgString.cpp \
	%D%/arguments/GKDBusArgString.hpp \
	%D%/arguments/GKDBusArgBoolean.cpp \
	%D%/arguments/GKDBusArgBoolean.hpp \
	%D%/arguments/GKDBusArgByte.cpp \
	%D%/arguments/GKDBusArgByte.hpp \
	%D%/arguments/GKDBusArgUInt16.cpp \
	%D%/arguments/GKDBusArgUInt16.hpp \
	%D%/arguments/GKDBusArgUInt64.cpp \
	%D%/arguments/GKDBusArgUInt64.hpp \
	%D%/arguments/GKDBusArgMKeysID.cpp \
	%D%/arguments/GKDBusArgMKeysID.hpp \
	%D%/arguments/GKDBusArgMacro.cpp \
	%D%/arguments/GKDBusArgMacro.hpp \
	%D%/arguments/GKDBusArgMacrosBank.cpp \
	%D%/arguments/GKDBusArgMacrosBank.hpp \
	%D%/arguments/GKDBusArgDevicesMap.cpp \
	%D%/arguments/GKDBusArgDevicesMap.hpp \
	%D%/arguments/GKDBusArgLCDPluginsArray.cpp \
	%D%/arguments/GKDBusArgLCDPluginsArray.hpp \
	%D%/messages/GKDBusMessage.cpp \
	%D%/messages/GKDBusMessage.hpp \
	%D%/messages/GKDBusReply.cpp \
	%D%/messages/GKDBusReply.hpp \
	%D%/messages/GKDBusErrorReply.cpp \
	%D%/messages/GKDBusErrorReply.hpp \
	%D%/messages/GKDBusRemoteMethodCall.cpp \
	%D%/messages/GKDBusRemoteMethodCall.hpp \
	%D%/messages/GKDBusBroadcastSignal.cpp \
	%D%/messages/GKDBusBroadcastSignal.hpp \
	%D%/messages/GKDBusAsyncContainer.cpp \
	%D%/messages/GKDBusAsyncContainer.hpp \
	%D%/events/GKDBusEvent.cpp \
	%D%/events/GKDBusEvent.hpp \
	%D%/events/GKDBusEventCallback.hpp \
	%D%/events/GKDBusEventTemplates.cpp \
	%D%/events/GKDBusEventTemplates.hpp \
	%D%/events/SIGv2v.cpp \
	%D%/events/SIGv2v.hpp \
	%D%/events/SIGs2v.cpp \
	%D%/events/SIGs2v.hpp \
	%D%/events/SIGss2v.cpp \
	%D%/events/SIGss2v.hpp \
	%D%/events/SIGs2b.cpp \
	%D%/events/SIGs2b.hpp \
	%D%/events/SIGss2b.cpp \
	%D%/events/SIGss2b.hpp \
	%D%/GKDBusConnection.hpp \
	%D%/GKDBus.cpp \
	%D%/GKDBus.hpp \
	%D%/GKDBusEvents.cpp \
	%D%/GKDBusEvents.hpp
