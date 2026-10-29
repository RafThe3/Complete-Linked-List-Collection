#pragma once
#include <string>

using std::string;

class BaseException
{
public:
    BaseException(string msg = "An unknown error occurred") : msg(msg) {}
	~BaseException(){}


	string getMessage()
    {
        return msg;
    }

	void setMessage(string msg)
    {
        this->msg = msg;
    }

protected:
	string msg;
};

class IndexOutOfBounds : public BaseException
{
public:
    IndexOutOfBounds(string msg = "Attempt to access element out of range") : BaseException(msg)
    {
    }
	~IndexOutOfBounds() {}

private:

};

class EmptyList : public BaseException
{
    public:
    EmptyList(string msg = "Attempt to access element in empty list") : BaseException(msg) {}
    ~EmptyList() {}

    private:
};