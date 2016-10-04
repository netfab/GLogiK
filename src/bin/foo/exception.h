
#ifndef GLOGIK_EXCEPTION_H
#define GLOGIK_EXCEPTION_H

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

}

#endif

