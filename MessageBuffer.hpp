#ifndef MESSAGE_BUFFER_HPP
#define MESSAGE_BUFFER_HPP

#include <queue>
#include <mutex>
#include <condition_variable>


template< class T >
class MessageBuffer 
{
public:
    MessageBuffer()
    {
    }

    ~MessageBuffer()
    {
    }

    void addToQueue( T message )
    {
        {
            std::unique_lock<std::mutex> lock( m_mutex );
            m_queue.push( message );
        }

        // notify if waiting
        m_condVar.notify_one();
    }


    // blocking wait if empty
    T getFromQueue()
    {
        std::unique_lock<std::mutex> lock( m_mutex );

        while( m_queue.empty() )
        {
            // temporarily unlocks mutex for new messages to be added
            m_condVar.wait( lock );
        }

        T message = m_queue.front();
        m_queue.pop();

        return message;
    }

private:

    //non copyable
   MessageBuffer(const MessageBuffer&) = delete;
   MessageBuffer& operator=(const MessageBuffer&) = delete;


private:

    std::queue<T>               m_queue;
    std::mutex                  m_mutex;
    std::condition_variable     m_condVar;

};



#endif //MESSAGE_BUFFER_HPP


