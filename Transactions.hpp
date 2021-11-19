#ifndef TRANSACTIONS_HPP
#define TRANSACTIONS_HPP

#include <string>

class TransactionProcessor;


class Transaction
{
public: 

    Transaction()
    {
    }

    virtual ~Transaction()
    {
    }


    virtual void process( TransactionProcessor& transactionProcessor ) = 0;
    virtual std::string getDumpForLog() const = 0;

private:

};




class NewOrder : public Transaction
{
public: 

    static constexpr char type = 'N';

    NewOrder( int user, const std::string& symbol, int price, int qty, char side, int userOrderId );

    NewOrder( const NewOrder& that ) = default;


    virtual void process( TransactionProcessor& transactionProcessor );
    virtual std::string getDumpForLog() const;

    std::string getAckMsg() const;

/*
    int getUser() const { return m_user; }
    std::string getSymbol() const { return m_symbol; }
    int getPrice() const { return m_price; }
    int getQty() const { return m_qty; }
    char getSide() const { return m_side; }
    int getUserOrderId() const { return m_userOrderId; }
*/

protected:
    int         m_user;
    std::string m_symbol;
    int         m_price;
    int         m_qty;
    char        m_side;
    int         m_userOrderId;
};




class CancelOrder : public Transaction
{
public: 

    static constexpr char type = 'C';

    CancelOrder( int user, int userOrderId );

    virtual void process( TransactionProcessor& transactionProcessor );
    virtual std::string getDumpForLog() const;

    int getUser() const { return m_user; }
    int getUserOrderId() const { return m_userOrderId; }

    std::string getAckMsg() const;


protected:
    int         m_user;
    int         m_userOrderId;
};




class FlushOrderBook : public Transaction
{
public:

    static constexpr char type = 'F';

    FlushOrderBook();

    virtual void process( TransactionProcessor& transactionProcessor );
    virtual std::string getDumpForLog() const;

};



#endif //TRANSACTIONS_HPP
