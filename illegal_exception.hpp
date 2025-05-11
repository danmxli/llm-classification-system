#ifndef ILLEGAL_EXCEPTION
#define ILLEGAL_EXCEPTION

#include <exception>
#include <string>
#include <vector>
using namespace std;

class illegal_exception : public exception
{
public:
    string message;
    illegal_exception();
    static bool validateNoIllegalClassification(vector<string> &classificationVector);
    static bool validateNoIllegalInputString(string str);
};

#endif
