
Kraken C++ Developer Test by Simon Whittaker


BUILD
=====

To build, use  make  (-f Makefile)

built on Ubuntu on windows  5.10.16.3-microsoft-standard-WSL2
gcc version 9.3.0 (Ubuntu 9.3.0-17ubuntu1~20.04)
boost version  1.71.0.0ubuntu2

Tried to avoid using boost (simply because another dependency, not because of anything bad) but utilitised boost::split to tokenise input. Installation of boost in environment did not require additional paths to be added to the Makefile. 


EXECUTE
=======

Run executable in a separate session/window or in the background: Command: orderbook <port>

orderbook 1234

And netcat input as suggest from documentation: 

cat inputFile.csv | sed '/#.*/d;/^\s*$/d' | netcat -u 127.0.0.1 1234


COMMENTS
========

InputReader

1. Handle server domain names as well as raw IP address
2. Use non-blocking (with buffering)
3. Remove blank lines and comments - input validation


Logger

4. Better logger as output to stdout same as program  (enabled with compliation -DLOG_ENABLED )


TransactionProcessor/Transactions/BookOrder 

5. Tt would be better if NewOrder and BookOrder has combined functionality but polymorphic Transaction MessageBuffer is complicating this


TransactionProcessor

6. Need to remove the cancel lookup entry for match order in TransactionProcessor::newOrder()
7. Orders should be kept in depth order by price and timed order in each price bucket
8. OrderBook structure should be partition on key Symbol and Side
9. OrderBook structure should cache the previous Top Of Order Book
10. Not thread safe so locking will be required for handling multiple async input events.
11. Improvement to the interface to allow auto unit testing, e.g. hooks and strutures for input and output data


main

12. Validate args
13. Graceful shutdown


It would have been nice to have a "professional" environment to work in rather than something put together at haste. 
Written from first principle but too limited on time. 


Performance code analysis using kcachegrind 


Notes: Sample input file had missing transaction type on rows 41 and 43. Output for scenario 2 in the example outputFile.csv is incomplete (others are missing?).
Is output Ack for cancels = "C" ? Specs suggests it should be "A"




