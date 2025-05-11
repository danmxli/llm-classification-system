#ifndef TRIE_HPP
#define TRIE_HPP

#include "trienode.hpp"
#include "ece250_socket.h"
#include "illegal_exception.hpp"
#include <string>
#include <iostream>
#include <algorithm>

class Trie
{
private:
    TrieNode *root;
    void deleteNode(TrieNode *node);
    void pathStringBuilder(TrieNode *node, string path);
    string classificationLabelBuilder(const vector<TrieNode *> childClassifications);
    int classificationCount;

public:
    enum ReturnCodes
    {
        SUCCESS = 0,
        FAILURE = -1,
        ILLEGAL_ARGUMENTS = -2,
    };

    Trie();
    ~Trie();

    TrieNode *getRoot();

    ReturnCodes insert(vector<string> classificationVector);
    void classify(string inputPhrase);
    void erase(vector<string> classificationVector);
    void print(TrieNode *node);
    void printEmpty();
    void clear();
    void printSize();
};

#endif
