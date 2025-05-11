#include "trienode.hpp"

TrieNode::TrieNode() : value("")
{
}

TrieNode::TrieNode(const string &val) : value(val)
{
}

TrieNode *TrieNode::addChild(const string &childValue)
{
    TrieNode *child = new TrieNode(childValue);
    children.push_back(child);
    return child;
}

TrieNode *TrieNode::findChild(const string &childValue)
{
    for (TrieNode *child : children)
    {
        if (child->value == childValue)
        {
            return child;
        }
    }
    return nullptr;
}

void TrieNode::removeChild(const string &childValue)
{
    for (auto it = children.begin(); it != children.end(); ++it)
    {
        if ((*it)->value == childValue)
        {
            delete *it;
            children.erase(it);
            return;
        }
    }
}
