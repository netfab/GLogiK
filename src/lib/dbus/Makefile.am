
lib_LTLIBRARIES=libGLogiKDBus.la

libGLogiKDBus_la_CXXFLAGS = \
						@AM_CXXFLAGS@ \
						$(DBUS_CFLAGS)

libGLogiKDBus_la_LDFLAGS = \
						@AM_LDFLAGS@

libGLogiKDBus_la_LIBADD = \
				$(DBUS_LIBS) \
				../utils/libGLogiKUtils.la

libGLogiKDBus_la_SOURCES = \
	GKDBus.cpp GKDBus.h \
	arguments/GKDBusArgument.cpp arguments/GKDBusArgument.h \
	arguments/GKDBusArgString.cpp arguments/GKDBusArgString.h \
	messages/GKDBusMessage.cpp messages/GKDBusMessage.h \
	messages/GKDBusReply.cpp messages/GKDBusReply.h \
	messages/GKDBusErrorReply.cpp messages/GKDBusErrorReply.h \
	messages/GKDBusRemoteMethodCall.cpp messages/GKDBusRemoteMethodCall.h \
	events/GKDBusEvent.cpp events/GKDBusEvent.h \
	events/GKDBusEventStringToBool.cpp events/GKDBusEventStringToBool.h \
	events/GKDBusEventStringToString.cpp events/GKDBusEventStringToString.h \
	GKDBusEvents.cpp GKDBusEvents.h
