#ifndef TRIENODE_HPP
#define TRIENODE_HPP

#include <string>
#include <vector>
using namespace std;

class TrieNode
{
public:
    vector<TrieNode *> children;
    string value;

    TrieNode();
    TrieNode(const string &val);

    TrieNode *addChild(const string &childValue);
    TrieNode *findChild(const string &childValue);
    void removeChild(const string &childValue);
};

#endif

