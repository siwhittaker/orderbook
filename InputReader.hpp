#ifndef INPUT_READER_HPP
#define INPUT_READER_HPP


#include <thread>
#include <string>

#include "MessageBuffer.hpp"

class Transaction;
class NewOrder;
class CancelOrder;

class InputReader 
{
public:
    InputReader( MessageBuffer<Transaction*>& transMsgBuffer, int port );
    ~InputReader();


private:

    std::stringstream readTransactions();

    NewOrder* createNewOrder( const std::string& trans ) const;
    CancelOrder* createCancelOrder( const std::string& trans ) const;

    //thread run
    void run();

   //non copyable
   InputReader(const InputReader&) = delete;
   InputReader& operator=(const InputReader&) = delete;


private:
    MessageBuffer<Transaction*>&    m_transMsgBuffer;

    int                             m_socket;
    int                             m_port;

    bool                            m_running;
    std::thread                     m_readerThread;

    int                             m_bufferSize;
    char*                           m_buffer;
};



#endif //INPUT_READER_HPP


