////////////////////////////////////////////////////////////////////////////////
/// \file
/// \brief     Реализация классов красно-черного дерева
/// \author    Sergey Shershakov
/// \version   0.1.0
/// \date      01.05.2017
///            This is a part of the course "Algorithms and Data Structures"
///            provided by  the School of Software Engineering of the Faculty
///            of Computer Science at the Higher School of Economics.
///
/// "Реализация" (шаблонов) методов, описанных в файле rbtree.h
///
////////////////////////////////////////////////////////////////////////////////

#include <stdexcept>        // std::invalid_argument


namespace xi {


//==============================================================================
// class RBTree::node
//==============================================================================

    template <typename Element, typename Compar >
    RBTree<Element, Compar>::Node::~Node()
    {
        if (_left)
            delete _left;
        if (_right)
            delete _right;
    }



    template <typename Element, typename Compar>
    typename RBTree<Element, Compar>::Node* RBTree<Element, Compar>::Node::setLeft(Node* lf)
    {
        // предупреждаем повторное присвоение
        if (_left == lf)
            return nullptr;

        // если новый левый — действительный элемент
        if (lf)
        {
            // если у него был родитель
            if (lf->_parent)
            {
                // ищем у родителя, кем был этот элемент, и вместо него ставим бублик
                if (lf->_parent->_left == lf)
                    lf->_parent->_left = nullptr;
                else                                    // доп. не проверяем, что он был правым, иначе нарушение целостности
                    lf->_parent->_right = nullptr;
            }

            // задаем нового родителя
            lf->_parent = this;
        }

        // если у текущего уже был один левый — отменяем его родительскую связь и вернем его
        Node* prevLeft = _left;
        _left = lf;

        if (prevLeft)
            prevLeft->_parent = nullptr;

        return prevLeft;
    }


    template <typename Element, typename Compar>
    typename RBTree<Element, Compar>::Node* RBTree<Element, Compar>::Node::setRight(Node* rg)
    {
        // предупреждаем повторное присвоение
        if (_right == rg)
            return nullptr;

        // если новый правый — действительный элемент
        if (rg)
        {
            // если у него был родитель
            if (rg->_parent)
            {
                // ищем у родителя, кем был этот элемент, и вместо него ставим бублик
                if (rg->_parent->_left == rg)
                    rg->_parent->_left = nullptr;
                else                                    // доп. не проверяем, что он был правым, иначе нарушение целостности
                    rg->_parent->_right = nullptr;
            }

            // задаем нового родителя
            rg->_parent = this;
        }

        // если у текущего уже был один левый — отменяем его родительскую связь и вернем его
        Node* prevRight = _right;
        _right = rg;

        if (prevRight)
            prevRight->_parent = nullptr;

        return prevRight;
    }


//==============================================================================
// class RBTree
//==============================================================================

    template <typename Element, typename Compar >
    RBTree<Element, Compar>::RBTree()
    {
        _root = nullptr;
        _dumper = nullptr;
    }

    template <typename Element, typename Compar >
    RBTree<Element, Compar>::~RBTree()
    {
        // грохаем пока что всех через корень
        if (_root)
            delete _root;
    }


    template <typename Element, typename Compar >
    void RBTree<Element, Compar>::deleteNode(Node* nd)
    {
        // если переданный узел не существует, просто ничего не делаем, т.к. в вызывающем проверок нет
        if (nd == nullptr)
            return;

        // потомков убьет в деструкторе
        delete nd;
    }


    template <typename Element, typename Compar >
    void RBTree<Element, Compar>::insert(const Element& key)
    {
        // этот метод можно оставить студентам целиком
        Node* newNode = insertNewBstEl(key);

        // отладочное событие
        if (_dumper)
            _dumper->rbTreeEvent(IRBTreeDumper<Element, Compar>::DE_AFTER_BST_INS, this, newNode);

        rebalance(newNode);

        // отладочное событие
        if (_dumper)
            _dumper->rbTreeEvent(IRBTreeDumper<Element, Compar>::DE_AFTER_INSERT, this, newNode);

    }

    template <typename Element, typename Compar>
    void RBTree<Element, Compar>::remove(const Element &key)
    {
        Node* node = (Node *) find(key);

        if (node == nullptr)
            throw std::invalid_argument("No such node!");

        if (node->_left && node->_right)
        {
            Node* pred = node->predecessor();
            node->_key = pred->_key;
            node = pred;
        }

        Node* child;
        bool f = (bool) node->_left;
        child = node->getChild(f);

        if (child)
        {
            if (node == getRoot())
                _root = child;
            else
            if (node->isLeftChild())
                node->_parent->setLeft(child);
            else
                node->_parent->setRight(child);

            if (_dumper)
                _dumper->rbTreeEvent(IRBTreeDumper<Element, Compar>::DE_AFTER_BST_REMOVE, this, node);

            if (node->isBlack())
                deleteFixUp(child);
        }
        else if (node == _root)
        {
            _root = nullptr;
            if (_dumper)
                _dumper->rbTreeEvent(IRBTreeDumper<Element, Compar>::DE_AFTER_BST_REMOVE, this, node);
        }
        else
        {
            if (_dumper)
                _dumper->rbTreeEvent(IRBTreeDumper<Element, Compar>::DE_AFTER_BST_REMOVE, this, node);

            if (node->isBlack())
                deleteFixUp(node);

            if (node->_parent)
                if (node->isLeftChild())
                    node->_parent->setLeft(nullptr);
                else
                    node->_parent->setRight(nullptr);
        }

        if (_dumper)
            _dumper->rbTreeEvent(IRBTreeDumper<Element, Compar>:: DE_AFTER_REMOVE, this, node);

        deleteNode(node);
    }

    template <typename Element, typename Compar>
    void RBTree<Element, Compar>::deleteFixUp(Node *node)
    {
        while (node != _root && node->isBlack())
        {
            if (node->isLeftChild())
            {
                Node* brother = node->brother();
                if (brother->isRed())
                {
                    brother->setBlack();
                    node->_parent->setRed();
                    rotLeft(node->_parent);
                    brother = node->_parent->_right;
                }
                if ( (!brother->_left || brother->_left->isBlack()) && (!brother->_right || brother->_right->isBlack()) )
                {
                    brother->setRed();
                    node = node->_parent;
                }
                else
                {
                    if (brother->getRight()->isBlack())
                    {
                        brother->_left->setBlack();
                        brother->setBlack();

                        rotRight(brother);

                        brother = node->_parent->_right;
                    }

                    brother->_color = node->_parent->_color;
                    node->_parent->setBlack();

                    if (brother->_right)
                        brother->_right->setBlack();

                    rotLeft(node->_parent);
                    node = _root;
                }
            }
            else
            {
                Node* bro = node->brother();
                if (bro->isRed())
                {
                    bro->setBlack();
                    node->_parent->setRed();
                    rotRight(node->_parent);
                    bro = node->_parent->_left;
                }
                if (bro->_left->isBlack() && bro->_right->isBlack())
                {
                    bro->setRed();
                    node = node->_parent;
                }
                else
                {
                    if (bro->_left->isBlack())
                    {
                        bro->_right->setBlack();
                        bro->setRed();
                        rotLeft(bro);
                        bro = node->_parent->_left;
                    }

                    bro->_color = node->_parent->_color;
                    node->_parent->setBlack();
                    bro->_left->setBlack();
                    rotRight(node->_parent);
                    node = _root;
                }
            }
        }

        node->setBlack();
    }

    template <typename Element, typename Compar>
    const typename RBTree<Element, Compar>::Node* RBTree<Element, Compar>::find(const Element& key)
    {
        Node* current = _root;
        while (current)
        {
            if (current->_key == key)
                return current;

            if(key < current->_key)
                current = current->_left;
            else
                current = current->_right;
        }

        return nullptr;
    }

    template <typename Element, typename Compar >
    typename RBTree<Element, Compar>::Node*
    RBTree<Element, Compar>::insertNewBstEl(const Element& key)
    {
        // TODO: метод реализуют студенты
        if (find(key) != nullptr)
            throw std::logic_error("Tree already has such key!");

        Node* node = new Node(key);
        Node* current = (Node*) getRoot();
        Node* temp = nullptr;

        if (!current)
        {
            _root = node;
            node->setBlack();
            return node;
        }

        do
        {
            temp = current;
            if (node->getKey() < current->getKey())
                current = current->_left;
            else
                current = current->_right;
        }
        while (current);

        if (node->getKey() < temp->getKey())
            temp->_left = node;
        else
            temp->_right = node;

        node->_parent = temp;
        return node;
    }


    template <typename Element, typename Compar >
    typename RBTree<Element, Compar>::Node*
    RBTree<Element, Compar>::rebalanceDUG(Node* nd)
    {
        Node* grandPa = nd->_parent->_parent;
        if (nd->_parent->isLeftChild())
        {
            Node* uncle = grandPa->_right;

            if (uncle && uncle->isRed())
            {
                nd->_parent->setBlack();
                uncle->setBlack();
                grandPa->setRed();

                if (_dumper)
                    _dumper->rbTreeEvent(IRBTreeDumper<Element, Compar>::DE_AFTER_RECOLOR1, this, nd);

                nd = grandPa;
            }
            else
            {
                if (nd->isRightChild())
                {
                    nd = nd->_parent;
                    rotLeft(nd);
                }

                if (nd->getParent())
                {
                    nd->_parent->setBlack();
                    if (_dumper)
                        _dumper->rbTreeEvent(IRBTreeDumper<Element, Compar>::DE_AFTER_RECOLOR3D, this, nd);


                    if (nd->getParent()->getParent())
                    {
                        grandPa->setRed();
                        if (_dumper)
                            _dumper->rbTreeEvent(IRBTreeDumper<Element, Compar>::DE_AFTER_RECOLOR3G, this, nd);

                        rotRight(grandPa);
                    }
                }
            }
        }
        else
        {
            Node* uncle = grandPa->_left;

            if (uncle && uncle->isRed())
            {
                nd->_parent->setBlack();
                uncle->setBlack();
                grandPa->setRed();

                if (_dumper)
                    _dumper->rbTreeEvent(IRBTreeDumper<Element, Compar>::DE_AFTER_RECOLOR1, this, nd);

                nd = grandPa;
            }
            else
            {
                if (nd->isLeftChild())
                {
                    nd = nd->_parent;
                    rotRight(nd);
                }

                if (nd->getParent())
                {
                    nd->_parent->setBlack();
                    if (_dumper)
                        _dumper->rbTreeEvent(IRBTreeDumper<Element, Compar>::DE_AFTER_RECOLOR3D, this, nd);


                    if (nd->getParent()->getParent())
                    {
                        grandPa->setRed();
                        if (_dumper)
                            _dumper->rbTreeEvent(IRBTreeDumper<Element, Compar>::DE_AFTER_RECOLOR3G, this, nd);

                        rotLeft(grandPa);
                    }
                }
            }
        }

        if (_dumper)
            _dumper->rbTreeEvent(IRBTreeDumper<Element, Compar>::DE_AFTER_RECOLOR1, this, nd);

        return nd;
    }


    template <typename Element, typename Compar >
    void RBTree<Element, Compar>::rebalance(Node* nd)
    {
        nd->setRed();

        while (nd !=_root && nd->getParent()->isRed())
        {
            nd = rebalanceDUG(nd);
        }

        _root->setBlack();
    }



    template <typename Element, typename Compar>
    void RBTree<Element, Compar>::rotLeft(typename RBTree<Element, Compar>::Node* nd)
    {
        Node*y = nd->_right;

        if(!y)
            throw std::invalid_argument("Can't rotate left since the right child is nil");

        nd->_right = y->_left;
        if(y->_left)
            y->_left->_parent=nd;

        if(y)
            y->_parent = nd->_parent;

        if(nd->_parent)
            if(nd == nd->_parent->_left)
                nd->_parent->_left = y;
            else
                nd->_parent->_right = y;
        else
            _root = y;

        y->_left = nd;

        if(nd)
            nd->_parent = y;

        if(_dumper)
            _dumper->rbTreeEvent(IRBTreeDumper<Element,Compar>::DE_AFTER_LROT,this,nd);

    }



    template <typename Element, typename Compar>
    void RBTree<Element, Compar>::rotRight(typename RBTree<Element, Compar>::Node* nd)
    {
        Node* nodeLeft = nd->_left;

        nd->_left = nodeLeft->_right;
        if(nodeLeft->_right)
            nodeLeft->_right->_parent = nd;

        if(nodeLeft)
            nodeLeft->_parent = nd->_parent;

        if(nd->_parent)
            if(nd == nd->_parent->_right)
                nd->_parent->_right = nodeLeft;
            else
                nd->_parent->_left = nodeLeft;
        else
            _root = nodeLeft;

        nodeLeft->_right = nd;
        if(nodeLeft)
            nd->_parent = nodeLeft;

        if(_dumper)
            _dumper->rbTreeEvent(IRBTreeDumper<Element,Compar>::DE_AFTER_RROT,this,nd);
    }


} // namespace xi

