
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

} // namespace GLogiKd

