
#include <string>
#include "exception.h"

namespace GLogiKd
{

GLogiKExcept::GLogiKExcept( const std::string& msg ) : message(msg) {}
GLogiKExcept::~GLogiKExcept( void ) throw() {}

const char* GLogiKExcept::what( void ) const throw()
{
    return message.c_str();
}

DeviceFound::DeviceFound( const std::string& msg ) : message(msg) {}
DeviceFound::~DeviceFound( void ) throw() {}

const char* DeviceFound::what( void ) const throw()
{
    return message.c_str();
}

DisplayHelp::DisplayHelp( const std::string& msg ) : message(msg) {}
DisplayHelp::~DisplayHelp( void ) throw() {}

const char* DisplayHelp::what( void ) const throw()
{
    return message.c_str();
}


} // namespace GLogiKd

