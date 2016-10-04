
#include <string>
#include "exception.h"

namespace GLogiK
{

GLogiKExcept::GLogiKExcept( const std::string& msg ) : message(msg) {}
GLogiKExcept::~GLogiKExcept( void ) throw() {}


const char* GLogiKExcept::what( void ) const throw()
{
    return message.c_str();
}

}

