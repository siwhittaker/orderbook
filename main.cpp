

#include <iostream>
#include <thread>

#include "OutputWriter.hpp"
#include "TransactionProcessor.hpp"
#include "InputReader.hpp"
#include "MessageBuffer.hpp"



int main(int argc, char* argv[])
{

    // TODO validate args
    if( argc != 2 )
    {
        std::cerr << "Usage " << argv[0] << " udp-port" << std::endl;
        return 1;
    }
    
    try
    {
        int port = atoi(argv[1] );

        MessageBuffer<Transaction*> transMsgBuffer;
        MessageBuffer<std::string> outputMsgBuffer;

        OutputWriter outputWriter( outputMsgBuffer );

        TransactionProcessor transactionProcessor( transMsgBuffer, outputMsgBuffer );
    
        InputReader inputReader( transMsgBuffer, port );

        transactionProcessor.process();

        // TODO graceful shutdown
    }
    catch( std::exception& ex )
    {
        std::cerr << __FUNCTION__ << " : Caught exception: " << ex.what() << std::endl;
    }
    catch( ... )
    {
        std::cerr << __FUNCTION__ << " : Caught unhandled exception";
    }


    return 0;
}

