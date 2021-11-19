#ifndef TRANSACTION_PROCESSOR_HPP
#define TRANSACTION_PROCESSOR_HPP

#include <memory>
#include <list>
#include <unordered_map>

#include "MessageBuffer.hpp"
#include "Transactions.hpp"
#include "BookOrder.hpp"



class OrderRef
{
public:

    OrderRef( int user, int userOrderId )
    :
    m_user( user ), m_userOrderId( userOrderId )
    {
    }

    int         m_user;
    int         m_userOrderId;

     
    bool operator==(const OrderRef& rhs) const
    {
        return m_user == rhs.m_user && m_userOrderId == rhs.m_userOrderId;
    }

    struct Hasher 
    {
        std::size_t operator()(const OrderRef& orderRef) const
        {
            return std::hash<int>()(orderRef.m_user) ^ (std::hash<int>()(orderRef.m_userOrderId) << 1);
        }
    };
};



class TransactionProcessor
{
public:

    TransactionProcessor( MessageBuffer<Transaction*>& transMsgBuffer,
                            MessageBuffer<std::string>& outputMsgBuffer );

    ~TransactionProcessor();


    void process();

    void newOrder( NewOrder* newOrder );
    void cancelOrder( CancelOrder* cancelOrder );
    void flushTransactionProcessor();


private:

    typedef std::list<std::shared_ptr<BookOrder>>        TimedOrderListType;

    void outputAck( const std::string& ackMsg );
    void outputTrade( const BookOrder& buyOrder, const BookOrder& sellOrder, int price, int quantity );
    void outputTopOfBook( const TimedOrderListType& timedOrderList, char side, const std::string& symbol );


private:

    MessageBuffer<Transaction*>&            m_transMsgBuffer;
    MessageBuffer<std::string>&             m_outputMsgBuffer;

    bool                                    m_running;


    // TODO partition by Symbol and Side
    typedef std::unordered_map<std::string,TimedOrderListType>  SymbolOrderMapType;
    
    SymbolOrderMapType                      m_symbolOrderMap;


    // TODO combine thise into the SymbolOrderMap
    struct TopOfBook
    {
        int price = 0;
        int qty = 0;
    };

    typedef std::unordered_map<std::string,TopOfBook>  TopOfOrderBookType;

    TopOfOrderBookType                      m_TopOfOrderBookBuy;
    TopOfOrderBookType                      m_TopOfOrderBookSell;


    typedef std::unordered_map<OrderRef,TimedOrderListType*,OrderRef::Hasher>  CancelLookupMapType;

    CancelLookupMapType                     m_cancelLookupMap;


};



#endif //TRANSACTION_PROCESSOR_HPP
