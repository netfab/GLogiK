
lib_LTLIBRARIES = libGKUtils.la

libGKUtils_la_CXXFLAGS = \
				@AM_CXXFLAGS@ \
				@BOOST_CPPFLAGS@

libGKUtils_la_LDFLAGS = \
				@AM_LDFLAGS@ \
				@BOOST_LDFLAGS@

libGKUtils_la_LIBADD = \
				@BOOST_SYSTEM_LIB@ \
				@BOOST_FILESYSTEM_LIB@ \
				@BOOST_LOG_SETUP_LIB@ \
				@BOOST_LOG_LIB@ \
				@BOOST_PROCESS_LIB@ \
				@BOOST_THREAD_LIB@

libGKUtils_la_SOURCES = \
	%D%/templates.hpp \
	%D%/randomGenerator.cpp \
	%D%/randomGenerator.hpp \
	%D%/FSNotify.cpp \
	%D%/FSNotify.hpp \
	%D%/filesystem.cpp \
	%D%/filesystem.hpp \
	%D%/functions.cpp \
	%D%/functions.hpp \
	%D%/exception.cpp \
	%D%/exception.hpp \
	%D%/XDGUserDirs.cpp \
	%D%/XDGUserDirs.hpp \
	%D%/GKLogging.cpp \
	%D%/GKLogging.hpp \
	%D%/process.cpp \
	%D%/process.hpp \
	%D%/utils.hpp
