#include <iostream>
#include <stdexcept>
#include <sstream>
#include <string>
#include <vector>

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/ioctl.h>

#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/split.hpp>

#include "InputReader.hpp"
#include "Transactions.hpp"
#include "Logger.hpp"


std::vector<std::string> split(const std::string& words, const std::string& delim = ", " )
{   
    std::vector<std::string> tokens;
    boost::split(tokens, words, boost::is_any_of(delim), boost::token_compress_on);
    return tokens;
}



InputReader::InputReader( MessageBuffer<Transaction*>& transMsgBuffer, int port ) : 
    m_transMsgBuffer( transMsgBuffer ),
    m_port( port ),
    m_running( false ),
    m_bufferSize( 0 ),
    m_buffer(NULL)
{
    // TODO handle server domain names as well as raw IP address
    if( ( m_socket = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) 
    {
        std::string msg = "InputReader failed to create socket: ";
        msg += strerror( errno );
        throw std::runtime_error( msg.c_str() );
    }

    struct sockaddr_in inaddr;
    memset( &inaddr, 0, sizeof(inaddr));
    inaddr.sin_family = AF_INET; // IPv4
    inaddr.sin_addr.s_addr = INADDR_ANY;
    inaddr.sin_port = htons(m_port);

    // Bind the socket with the server address
    if( bind( m_socket, reinterpret_cast<const sockaddr*>( &inaddr ), sizeof(inaddr)) < 0 )
    {
        std::string msg = "InputReader failed to bind socket: ";
        msg += strerror( errno );
        throw std::runtime_error( msg.c_str() );
    }

    m_readerThread = std::thread( &InputReader::run, this );
    m_running = true;
}


InputReader::~InputReader()
{
    m_running = false;
    m_readerThread.join();

    if( m_buffer )
    {
        delete [] m_buffer;
    }
}


std::stringstream InputReader::readTransactions()
{
    struct sockaddr_in peeraddr;
    socklen_t len = sizeof(peeraddr); 
   
    std::stringstream str;

    // TODO non-blocking 

    // allocate large enough buffer
    int bytesAvailable = 0;
    if(ioctl(m_socket, FIONREAD, &bytesAvailable) != -1)
    {
        if( bytesAvailable > m_bufferSize )
        {
            if( m_bufferSize != 0 )
            {
                delete [] m_buffer;
            }

            m_bufferSize = 2 * bytesAvailable;
            m_buffer = new char[m_bufferSize];

            LOG << "set buffersize: " << m_bufferSize << std::endl;
        }
    }

    if( bytesAvailable > 0 )
    {
        int n = 0;

        if( (n = recvfrom(m_socket, m_buffer, m_bufferSize, MSG_WAITALL,//MSG_DONTWAIT,
                    reinterpret_cast<sockaddr *>(&peeraddr), &len)) > 0 )
        {
            // null terminate
            m_buffer[n] = '\0';

            // TODO remove blank lines and comments from input
            str << m_buffer;
        }

        LOG << "Received : " << str.str() << std::endl;

        /*
        while( (n = recvfrom(m_socket, m_buffer, m_bufferSize, MSG_DONTWAIT,
                        reinterpret_cast<sockaddr *>(&peeraddr), &len) == -1 ) )
        {
            if( (errno == EAGAIN) || (errno == EWOULDBLOCK) )
            {
                LOG << "non blocking wait: " << strerror(errno) << std::endl;
                std::this_thread::sleep_for(std::chrono::milliseconds(1000));
            }
            else
            {
                LOG << "socket read error: " << strerror(errno) << std::endl;
            }
        } 
        */

        if( n > 0 )
        {
            // null terminate
            m_buffer[n] = '\0';

            // TODO remove blank lines and commentsfrom input
            str << m_buffer;
        }

        LOG << "Received : " << str.str() << std::endl;
    }


    return str;
}


NewOrder* InputReader::createNewOrder( const std::string& trans ) const
{
    // Format:  N, user(int), symbol(string), price(int), qty(int), side(char B or S), userOrderId(int)
    auto tokens = split( trans, ", " );
    int user = atoi( tokens[1].c_str() );
    std::string symbol = tokens[2];
    int price = atoi( tokens[3].c_str() );
    int qty = atoi( tokens[4].c_str() );
    char side = tokens[5].front();
    int userOrderId = atoi( tokens[6].c_str() );

    NewOrder* newOrder = new NewOrder( user, symbol, price, qty, side, userOrderId );
    LOG << "Created : " << newOrder->getDumpForLog() << std::endl;

    return newOrder;
}


CancelOrder* InputReader::createCancelOrder( const std::string& trans ) const
{
    // Format:  C, user(int), userOrderId(int)
    auto tokens = split( trans, ", " );
    int user = atoi( tokens[1].c_str() );
    int userOrderId = atoi( tokens[2].c_str() );

    CancelOrder* cancelOrder = new CancelOrder( user, userOrderId );
    LOG << "Created : " << cancelOrder->getDumpForLog() << std::endl;

    return cancelOrder;
}


void InputReader::run()
{
    try
    {
        while( !m_running )
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }

        LOG << "InputReader thread running" << std::endl;

        while( m_running )
        {
            auto && transStream = readTransactions();

            // tokenise
            std::string trans;
            while ( getline( transStream, trans, '\n')) 
            {
                switch( (int)trans.front() )
                {
                case NewOrder::type:
                    m_transMsgBuffer.addToQueue( createNewOrder( trans ) );
                    break;

                case CancelOrder::type:
                    m_transMsgBuffer.addToQueue( createCancelOrder( trans ) );
                    break;

                case FlushOrderBook::type:
                    m_transMsgBuffer.addToQueue( new FlushOrderBook() );
                    break;

                default:
                    std::cerr << "Unexpected transaction : " << trans;
                    break;
                }
            }
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


    LOG << "InputReader thread exit" << std::endl;

}


