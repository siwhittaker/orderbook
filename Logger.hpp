#ifndef LOGGER_HPP
#define LOGGER_HPP

//TODO proper logger

#include <iostream>

#ifdef LOG_ENABLED
#define LOG     if( true ) std::cout
#else
#define LOG     if( false ) std::cout
#endif



#endif //LOGGER_CPP
