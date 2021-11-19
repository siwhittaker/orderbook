

#include <sstream>
#include <memory>

#include "TransactionProcessor.hpp"
#include "Transactions.hpp"
#include "Logger.hpp"

#include "BookOrder.hpp"


TransactionProcessor::TransactionProcessor( MessageBuffer<Transaction*>& transMsgBuffer,
                                            MessageBuffer<std::string>& outputMsgBuffer )
:
m_transMsgBuffer( transMsgBuffer ),
m_outputMsgBuffer( outputMsgBuffer ),
m_running(true)
{
}


TransactionProcessor::~TransactionProcessor()
{
}



void TransactionProcessor::process() 
{

    while(m_running)
    {
        std::unique_ptr<Transaction> trans( m_transMsgBuffer.getFromQueue() );

        LOG << "Processing " << trans->getDumpForLog() << std::endl;
        trans->process( *this );
    }
}


void TransactionProcessor::newOrder( NewOrder* newOrder )
{
    outputAck( newOrder->getAckMsg() );

    // Create a book order from the new order
    // TODO it would be better if NewOrder and BookOrder has combined functionality 
    // TODO but polymorphic Transaction MessageBuffer is complicating this
    std::shared_ptr<BookOrder> bookOrder( new BookOrder( *newOrder ) );

    auto symbolOrders = m_symbolOrderMap.find( bookOrder->getSymbol() );
    if( symbolOrders != m_symbolOrderMap.end() )
    {
        auto & timedOrderList = (*symbolOrders).second;
        if( timedOrderList.empty() )
        {
            // add new limit order 
            if( !bookOrder->isMarketOrder() )
            {
                timedOrderList.push_back( bookOrder );
                // add cancel lookup
                m_cancelLookupMap.insert( std::make_pair( OrderRef( bookOrder->getUser(), bookOrder->getUserOrderId() ), &timedOrderList ) );

                outputTopOfBook( timedOrderList, bookOrder->getSide(), bookOrder->getSymbol() );
            }
        }
        else
        {
            bool matched = false;
            auto ordersIter = timedOrderList.begin();
            for( ; ordersIter !=timedOrderList.end() ; ++ordersIter )
            {
                auto oldOrder = (*ordersIter);
                
                // match opposide side only
                if( bookOrder->isMarketOrder() 
                    && oldOrder->getUser() != bookOrder->getUser() 
                    && oldOrder->getSide() != bookOrder->getSide() )
                {   
                    matched = true;
                }
                else if( oldOrder->getUser() != bookOrder->getUser() 
                        && oldOrder->getSide() != bookOrder->getSide() 
                        && oldOrder->getPrice() == bookOrder->getPrice() )  
                {
                    // LimitOrder
                    // match opposide side and price
                    matched = true;
                }

                if( matched )
                {
                    // limit or market - take old order price
                    // fit or kill - take lowest quantity
                    outputTrade( ( oldOrder->getSide() == BookOrder::BUY ? *oldOrder : *bookOrder ),
                                ( oldOrder->getSide() == BookOrder::SELL ? *oldOrder : *bookOrder ),
                                oldOrder->getPrice(),
                                ( oldOrder->getQty() < bookOrder->getQty() ? oldOrder->getQty() : bookOrder->getQty() ) );

                    // remove exisiting order
                    timedOrderList.erase( ordersIter );

                    outputTopOfBook( timedOrderList, oldOrder->getSide(), bookOrder->getSymbol() );

                    // TODO remove the cancel lookup entry for match order
                    break;
                }
            }

            if( matched == false && !bookOrder->isMarketOrder() )
            {
                // add new limit order
                timedOrderList.push_back( bookOrder );
                // add cancel lookup
                m_cancelLookupMap.insert( std::make_pair( OrderRef( bookOrder->getUser(), bookOrder->getUserOrderId() ), &timedOrderList ) );

                outputTopOfBook( timedOrderList, bookOrder->getSide(), bookOrder->getSymbol() );
            }
        }
    }
    else if( !bookOrder->isMarketOrder() )
    {
        // if limit order then create new TimerOrder list with new Order for Symbol
        TimedOrderListType timedOrderList;
        timedOrderList.push_back( bookOrder );

        auto result = m_symbolOrderMap.insert( std::make_pair( bookOrder->getSymbol(), timedOrderList ) );

        if( result.second )
        {
            TimedOrderListType& pTimedOrderList = (result.first)->second;

            // add cancel lookup
            m_cancelLookupMap.insert( std::make_pair( OrderRef( bookOrder->getUser(), bookOrder->getUserOrderId() ), &pTimedOrderList ) );
        }

        outputTopOfBook( timedOrderList, bookOrder->getSide(), bookOrder->getSymbol() );
    }
}


void TransactionProcessor::cancelOrder( CancelOrder* cancelOrder )
{
    outputAck( cancelOrder->getAckMsg() );

    auto cancelIter = m_cancelLookupMap.find( OrderRef( cancelOrder->getUser(), cancelOrder->getUserOrderId() ) );
    if( cancelIter != m_cancelLookupMap.end() )
    {
        auto timedOrderList = (*cancelIter).second;

        auto ordersIter = timedOrderList->begin();
        for( ; ordersIter !=timedOrderList->end() ; ++ordersIter )
        {
            auto oldOrder = (*ordersIter);

            if( oldOrder->getUser() == cancelOrder->getUser() 
                && oldOrder->getUserOrderId() == cancelOrder->getUserOrderId() )
            {
                // remove cancelleed order
                timedOrderList->erase( ordersIter );
            
                // remove lookup
                m_cancelLookupMap.erase( cancelIter );
                break;
            }
        }
    }
}


void TransactionProcessor::flushTransactionProcessor()
{
    m_cancelLookupMap.clear();
    m_symbolOrderMap.clear();
    m_TopOfOrderBookBuy.clear();
    m_TopOfOrderBookSell.clear();
}


void TransactionProcessor::outputAck( const std::string& ackMsg )
{
    m_outputMsgBuffer.addToQueue( ackMsg );
}


void TransactionProcessor::outputTrade( const BookOrder& buyOrder, const BookOrder& sellOrder, int price, int quantity )
{
    std::ostringstream ostr; 
    ostr << "T, " << buyOrder.getUser() << ", " << buyOrder.getUserOrderId() << ", " 
            << sellOrder.getUser() << ", " << sellOrder.getUserOrderId()
            << ", " << price << ", " << quantity << std::ends;

     m_outputMsgBuffer.addToQueue( ostr.str() );
}


void TransactionProcessor::outputTopOfBook( const TimedOrderListType& timedOrderList, char side, const std::string& symbol )
{
    std::ostringstream ostr; 

    TopOfOrderBookType& topOfOrderBook = ( side == BookOrder::BUY ? m_TopOfOrderBookBuy : m_TopOfOrderBookSell );
    TopOfBook prevTopOfBook;

    // get existing top of order book (if exists)
    auto iter = topOfOrderBook.find( symbol );
    if( iter != topOfOrderBook.end() )
    {
        prevTopOfBook = (*iter).second;
    }
    
        
    TopOfBook topOrder;
    for( auto bookOrder : timedOrderList )
    {
        if( bookOrder->getSide() == side )
        {
            if( topOrder.price == 0 && topOrder.qty == 0 )
            {
                topOrder.price = bookOrder->getPrice();
                topOrder.qty = bookOrder->getQty();
            }
            else
            { 
                // highest bid
                // lowest ask
                if( ( bookOrder->getSide() == BookOrder::BUY && bookOrder->getPrice() > topOrder.price )
                    || ( bookOrder->getSide() == BookOrder::SELL && bookOrder->getPrice() < topOrder.price ) )
                {
                    topOrder.price = bookOrder->getPrice();
                    topOrder.qty = bookOrder->getQty();
                }
                else if( bookOrder->getPrice() == topOrder.price )
                {
                    // aggregate qantity at same price
                    topOrder.qty += bookOrder->getQty();
                }
            }
        }
    }

    if( topOrder.price == prevTopOfBook.price
        && topOrder.qty == prevTopOfBook.qty )
    {
        // no change
        return;
    }


    if( iter != topOfOrderBook.end() )
    {
        // changed so remove old one
        topOfOrderBook.erase( symbol );
    }

    if( topOrder.price == 0 && topOrder.qty == 0 )
    {
        ostr << "B, " << side << ", -, - " << std::ends;
    }
    else
    {
        ostr << "B, " << side << ", " << topOrder.price << ", " << topOrder.qty << std::ends;

        // update top of book
        topOfOrderBook.insert( std::make_pair( symbol, topOrder ) );
    }

    m_outputMsgBuffer.addToQueue( ostr.str() );
}

