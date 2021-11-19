
#include <sstream>

#include "Transactions.hpp"
#include "TransactionProcessor.hpp"
#include "Logger.hpp"




NewOrder::NewOrder( int user, const std::string& symbol, int price, int qty, char side, int userOrderId )
:
m_user( user ),
m_symbol( symbol ), 
m_price(price), 
m_qty(qty), 
m_side(side), 
m_userOrderId( userOrderId )
{
}

void NewOrder::process( TransactionProcessor& transactionProcessor )
{
    transactionProcessor.newOrder( this );
}

std::string NewOrder::getDumpForLog() const
{
    std::ostringstream ostr;
    ostr << "NewOrder:: " << m_user << " " << m_symbol << " " << m_price << " " << m_qty << " " << m_side << " " << m_userOrderId;
    return ostr.str();
}


std::string NewOrder::getAckMsg() const
{
    std::ostringstream ostr;
    ostr << "A, " <<  m_user << ", " << m_userOrderId << std::ends;
    return ostr.str();
}


CancelOrder::CancelOrder( int user, int userOrderId )
:
m_user( user ),
m_userOrderId( userOrderId )
{
}

void CancelOrder::process( TransactionProcessor& transactionProcessor )
{
    transactionProcessor.cancelOrder( this );
}

std::string CancelOrder::getDumpForLog() const
{
    std::ostringstream ostr;
    ostr << "CancelOrder:: " << m_user << " " << m_userOrderId;
    return ostr.str();
}


std::string CancelOrder::getAckMsg() const
{
    std::ostringstream ostr;
    ostr << "A, " <<  m_user << ", " << m_userOrderId << std::ends;
    return ostr.str();
}


FlushOrderBook::FlushOrderBook()
{
}

void FlushOrderBook::process( TransactionProcessor& transactionProcessor )
{
    transactionProcessor.flushTransactionProcessor();
}


std::string FlushOrderBook::getDumpForLog() const
{
    return "FlushOrderBook";
}

