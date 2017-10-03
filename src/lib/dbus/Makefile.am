
lib_LTLIBRARIES=libGLogiKDBus.la

libGLogiKDBus_la_CXXFLAGS = $(DBUS_CFLAGS)
libGLogiKDBus_la_LIBADD = $(DBUS_LIBS) \
				../utils/libGLogiKUtils.la

libGLogiKDBus_la_SOURCES = \
	GKDBus.cpp GKDBus.h \
	GKDBusEvents.cpp GKDBusEvents.h \
	GKDBusMessage.cpp GKDBusMessage.h \
	GKDBusMsgReply.cpp GKDBusMsgReply.h \
	GKDBusRemoteMethodCall.cpp GKDBusRemoteMethodCall.h \
	GKDBusBroadcastSignal.cpp GKDBusBroadcastSignal.h \
	GKDBusMessageErrorReply.cpp GKDBusMessageErrorReply.h
