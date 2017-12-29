
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
	events/GKDBusEvent.cpp events/GKDBusEvent.h \
	events/GKDBusArgument.cpp events/GKDBusArgument.h \
	events/GKDBusArgString.cpp events/GKDBusArgString.h \
	message/GKDBusMessage.cpp message/GKDBusMessage.h \
	message/GKDBusReply.cpp message/GKDBusReply.h \
	message/GKDBusErrorReply.cpp message/GKDBusErrorReply.h \
	events/GKDBusEventStringToBool.cpp events/GKDBusEventStringToBool.h \
	events/GKDBusEventStringToString.cpp events/GKDBusEventStringToString.h \
	GKDBusEvents.cpp GKDBusEvents.h
