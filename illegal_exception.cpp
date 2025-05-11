#include "illegal_exception.hpp"

illegal_exception::illegal_exception() : message("illegal argument")
{
}

bool illegal_exception::validateNoIllegalClassification(vector<string> &classificationVector)
{
    for (string str : classificationVector)
    {
        if (!validateNoIllegalInputString(str))
        {
            return false;
        }
    }
    return true;
}

bool illegal_exception::validateNoIllegalInputString(string str)
{
    for (char c : str)
    {
        if (isupper(c))
        {
            return false;
        }
    }
    return true;
}
