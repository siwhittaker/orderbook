#ifndef OUTPUT_WRITER_HPP
#define OUTPUT_WRITER_HPP


#include <thread>

#include "MessageBuffer.hpp"


class OutputWriter 
{
public:
    OutputWriter( MessageBuffer<std::string>& outputMsgBuffer );
    ~OutputWriter();


private:

    //thread run
    void run();

   //non copyable
   OutputWriter(const OutputWriter&) = delete;
   OutputWriter& operator=(const OutputWriter&) = delete;


private:
    MessageBuffer<std::string>&     m_outputMsgBuffer;

    bool                            m_running;
    std::thread                     m_outputThread;
};



#endif //OUTPUT_WRITER_HPP

