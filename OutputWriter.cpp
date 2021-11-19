#include <iostream>

#include "OutputWriter.hpp"
#include "Logger.hpp"


OutputWriter::OutputWriter( MessageBuffer<std::string>& outputMsgBuffer ) : 
    m_outputMsgBuffer( outputMsgBuffer ),
    m_running( false )
{
    m_outputThread = std::thread( &OutputWriter::run, this );
    m_running = true;
}


OutputWriter::~OutputWriter()
{
    m_running = false;
    m_outputThread.join();
}


void OutputWriter::run()
{
    try
    {
        while( !m_running )
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }

        LOG << "OutputWriter thread running" << std::endl;

        while( m_running )
        {
            auto msg = m_outputMsgBuffer.getFromQueue();

            std::cout << msg << std::endl;
        }
    }
    catch( std::exception& ex )
    {
        std::cerr << __FUNCTION__ << " : Caught exception: " << ex.what() << std::endl;
    }
    catch( ... )
    {
        std::cerr << __FUNCTION__ << " : Caught unhandled exception";
    }


    LOG << "OutputWriter thread exit" << std::endl;

}


