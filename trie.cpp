#include "trie.hpp"
#include "illegal_exception.hpp"

void Trie::deleteNode(TrieNode *node)
{
    if (!node)
    {
        return;
    }
    for (TrieNode *child : node->children)
    {
        deleteNode(child);
    }
    delete node;
}

void Trie::pathStringBuilder(TrieNode *node, string path)
{
    if (!node->value.empty()) // build the comma separated classification
    {
        if (!path.empty())
        {
            path += ",";
        }
        path += node->value;
    }

    if (node->children.empty()) // reached a leaf node
    {
        cout << path << "_";
        return;
    }

    for (TrieNode *child : node->children)
    {
        pathStringBuilder(child, path);
    }
}

string Trie::classificationLabelBuilder(const vector<TrieNode *> childClassifications)
{

    string label = childClassifications[0]->value;
    for (int i = 1; i < childClassifications.size(); i++)
    {
        label += "," + childClassifications[i]->value;
    }
    return label;
}

Trie::Trie()
{
    root = new TrieNode();
    classificationCount = 0;
}

Trie::~Trie()
{
    deleteNode(root);
}

TrieNode *Trie::getRoot()
{
    return root;
}

Trie::ReturnCodes Trie::insert(vector<string> classificationVector)
{
    try
    {
        if (!illegal_exception::validateNoIllegalClassification(classificationVector))
        {
            throw illegal_exception();
        }

        TrieNode *current = root;
        bool classificationExists = true;

        for (string substring : classificationVector)
        {
            TrieNode *child = current->findChild(substring);
            if (child == nullptr)
            {
                classificationExists = false;
                child = current->addChild(substring);
            }
            current = child;
        }

        if (classificationExists)
        {
            return FAILURE;
        }
        classificationCount++;
        return SUCCESS;
    }
    catch (illegal_exception &e)
    {
        cout << e.message << endl;
        return ILLEGAL_ARGUMENTS;
    }
}

void Trie::classify(string inputPhrase)
{
    try
    {
        if (!illegal_exception::validateNoIllegalInputString(inputPhrase))
        {
            throw illegal_exception();
        }

        TrieNode *current = root;
        vector<string> classChain;

        while (!current->children.empty())
        {
            string candidateLabels = classificationLabelBuilder(current->children);
            string chosenLabel = labelText(inputPhrase, candidateLabels);

            TrieNode *child = current->findChild(chosenLabel);
            current = child;
            classChain.push_back(chosenLabel);
        }

        // print the classification chain
        if (classChain.empty())
        {
            return;
        }
        cout << classChain[0];
        for (int i = 1; i < classChain.size(); i++)
        {
            cout << "," << classChain[i];
        }
        cout << endl;
    }
    catch (illegal_exception &e)
    {
        cout << e.message << endl;
    }
}

void Trie::erase(vector<string> classificationVector)
{
    try
    {
        if (!illegal_exception::validateNoIllegalClassification(classificationVector))
        {
            throw illegal_exception();
        }

        if (classificationCount <= 0)
        {
            cout << "failure" << endl;
            return;
        }

        TrieNode *parent = nullptr;
        TrieNode *current = root;

        for (string substring : classificationVector)
        {
            TrieNode *child = current->findChild(substring);
            if (child == nullptr)
            {
                cout << "failure" << endl;
                return;
            }

            parent = current;
            current = child;
        }

        if (current->children.empty())
        {
            parent->removeChild(current->value);
        }

        cout << "success" << endl;
        classificationCount--;
    }
    catch (illegal_exception &e)
    {
        cout << e.message << endl;
    }
}

void Trie::print(TrieNode *node)
{
    string path;

    if (node->children.empty())
    {
        cout << "trie is empty" << endl;
        return;
    }

    pathStringBuilder(node, path);
    cout << endl;
}

void Trie::printEmpty()
{
    cout << "empty " << (classificationCount > 0 ? "0" : "1") << endl;
}

void Trie::clear()
{
    deleteNode(root);
    root = new TrieNode();
    classificationCount = 0;
    cout << "success" << endl;
}

void Trie::printSize()
{
    cout << "number of classifications is " << classificationCount << endl;
}
