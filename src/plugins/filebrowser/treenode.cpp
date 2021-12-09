#include "treenode.h"

Node::Node(Node *parent,
           const QString &text,
           const QString &toolTip,
           Node::Type type)
    : parent(parent)
    , text(text)
    , toolTip(toolTip)
    , type(type)
    , children({})
    , isLoaded(false)
{

}

Node::~Node()
{
    for (auto val : children) {
        delete val;
    }
}
