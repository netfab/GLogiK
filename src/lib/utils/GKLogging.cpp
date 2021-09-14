/*
 *
 *	This file is part of GLogiK project.
 *	GLogiK, daemon to handle special features on gaming keyboards
 *	Copyright (C) 2016-2021  Fabrice Delliaux <netbox253@gmail.com>
 *
 *	This program is free software: you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation, either version 3 of the License, or
 *	(at your option) any later version.
 *
 *	This program is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 *
 *	You should have received a copy of the GNU General Public License
 *	along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include <chrono>
#include <fstream>
#include <iomanip>

#include <config.h>

#include <boost/smart_ptr/shared_ptr.hpp>
#include <boost/smart_ptr/make_shared_object.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/core/null_deleter.hpp>
#include <boost/log/core.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/attributes.hpp>
#include <boost/log/sinks/sync_frontend.hpp>
#include <boost/log/sinks/text_ostream_backend.hpp>
#include <boost/log/attributes/scoped_attribute.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>

#include "GKLogging.hpp"

#define UTILS_COMPILATION 1

#include "exception.hpp"

#undef UTILS_COMPILATION

namespace NSGKUtils
{

namespace chr = std::chrono;

namespace expr = boost::log::expressions;
namespace logging = boost::log;
namespace sinks = boost::log::sinks;
namespace attrs = boost::log::attributes;


BOOST_LOG_ATTRIBUTE_KEYWORD(line_id, "LineID", unsigned int)
BOOST_LOG_ATTRIBUTE_KEYWORD(severity, "Severity", severity_level)
BOOST_LOG_ATTRIBUTE_KEYWORD(scope, "Scope", attrs::named_scope::value_type)
BOOST_LOG_ATTRIBUTE_KEYWORD(timeline, "Timeline", attrs::timer::value_type)

bool GKLogging::initialized = false;
src::severity_logger< severity_level > GKLogging::GKLogger;

// The operator puts a human-friendly representation of the severity level to the stream
std::ostream& operator<< (std::ostream& strm, severity_level level)
{
    static const char* strings[] =
    {
		"trace",
		"info",
		"warning",
		"error",
		"critical"
    };

    if (static_cast< std::size_t >(level) < sizeof(strings) / sizeof(*strings))
        strm << strings[level];
    else
        strm << static_cast< int >(level);

    return strm;
}

void GKLogging::init(void)
{
	// Add attributes
	logging::add_common_attributes();
	logging::core::get()->add_global_attribute("Scope", attrs::named_scope());
	GKLogging::initialized = true;
}

void GKLogging::initConsoleLog(void)
{
	if( ! GKLogging::initialized )
		GKLogging::init();

	typedef sinks::synchronous_sink< sinks::text_ostream_backend > text_sink_type;

	boost::shared_ptr< text_sink_type > consoleSink = boost::make_shared< text_sink_type >();

	consoleSink->locked_backend()->add_stream(
		boost::shared_ptr< std::ostream >(&std::clog, boost::null_deleter()));

	consoleSink->set_formatter
    (
        expr::stream
            << std::hex << std::setw(8) << std::setfill('0') << line_id << std::dec << std::setfill(' ')
            << ": <" << severity << ">\t"
            //<< "(" << scope << ") "
            << expr::if_(expr::has_attr(timeline))
               [
                    expr::stream << "[" << timeline << "] "
               ]
            << expr::smessage
    );

	consoleSink->set_filter(severity >= info);

	logging::core::get()->add_sink(consoleSink);
}

void GKLogging::initDebugFile(const std::string & baseName, const fs::perms prms)
{
	if( ! GKLogging::initialized )
		GKLogging::init();

	typedef sinks::synchronous_sink< sinks::text_ostream_backend > text_sink_type;

	boost::shared_ptr< text_sink_type > fileSink = boost::make_shared< text_sink_type >();

	fs::path debugFile;
	{
		fs::path tmpPath(DEBUG_DIR);
		{
			chr::seconds s = chr::duration_cast< chr::seconds >(
				chr::system_clock::now().time_since_epoch()
			);

			tmpPath /= baseName;
			tmpPath += "-";
			tmpPath += std::to_string(s.count());
		}
		tmpPath += "-%%%%%%.log";
		debugFile = fs::unique_path(tmpPath);
	}

	if( fs::exists(debugFile) ) {
		throw GLogiKExcept("failed to create debug file : file exists");
	}

	fileSink->locked_backend()->add_stream(
		boost::make_shared< std::ofstream >(debugFile.string()));

	fileSink->locked_backend()->auto_flush(true);

	fileSink->set_formatter
    (
        expr::stream
            << std::hex << std::setw(8) << std::setfill('0') << line_id << std::dec << std::setfill(' ')
            << ": <" << severity << ">\t"
            << "(" << scope << ") "
            << expr::if_(expr::has_attr(timeline))
               [
                    expr::stream << "[" << timeline << "] "
               ]
            << expr::smessage
    );

	logging::core::get()->add_sink(fileSink);
}

} // namespace NSGKUtils

