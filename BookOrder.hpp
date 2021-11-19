#ifndef BOOK_ORDER_HPP
#define BOOK_ORDER_HPP


#include "Transactions.hpp"


class BookOrder : public NewOrder
{
public:

    static constexpr char BUY = 'B';
    static constexpr char SELL = 'S';

    BookOrder( const NewOrder& newOrder ) : NewOrder( newOrder )
    {
    }

    ~BookOrder()
    {
    }


    int getUser() const { return m_user; }
    std::string getSymbol() const { return m_symbol; }
    int getPrice() const { return m_price; }
    int getQty() const { return m_qty; }
    char getSide() const { return m_side; }
    int getUserOrderId() const { return m_userOrderId; }


    bool isMarketOrder() { return m_price == 0; }

};


#endif //BOOK_ORDER_HPP

