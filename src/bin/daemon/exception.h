
#ifndef __GLOGIKD_EXCEPTION_H__
#define __GLOGIKD_EXCEPTION_H__

#include <exception>
#include <string>

namespace GLogiKd
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

class DeviceFound : public std::exception
{
        public :
                DeviceFound( const std::string& msg = "" );
                virtual ~DeviceFound( void ) throw();
                virtual const char* what( void ) const throw();

        protected :
            std::string message;

};

class DisplayHelp : public std::exception
{
        public :
                DisplayHelp( const std::string& msg = "" );
                virtual ~DisplayHelp( void ) throw();
                virtual const char* what( void ) const throw();

        protected :
            std::string message;

};

} // namespace GLogiKd

#endif

