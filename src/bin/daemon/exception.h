
#ifndef __GLOGIK_EXCEPTION_H__
#define __GLOGIK_EXCEPTION_H__

#include <exception>
#include <string>

namespace GLogiK
{

class GLogiKExcept : public std::exception
{
        public :
                GLogiKExcept( const std::string& msg = "" );
                virtual ~GLogiKExcept( void ) throw();
                virtual const char* what( void ) const throw();

        protected :
            std::string message;

};

} // namespace GLogiK

#endif

