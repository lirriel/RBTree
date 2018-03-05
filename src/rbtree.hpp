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
//    // TODO: убрать!
//    setLeft(_left);


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
            throw std::logic_error("No such node!");

        if (node->_left && node->_right)
        {
            Node* pred = node->predecessor();
            node->_key = pred->_key;
            node = pred;
        }

        Node* child;
        if (node->_left)
        {
            child = node->_left;
            node->_left = nullptr;
        }
        else
        {
            child = node->_right;
            node->_right = nullptr;
        }

        if (child != nullptr)
        {
            if (node == _root)
                _root = child;
            else
            {
                if (node->isLeftChild())
                    node->_parent->_left = child;
                else
                    node->_parent->_right = child;

                child->_parent = node->_parent;
            }


            if (node->isBlack())
                deleteFixUp(child);
        }
        else if (node == _root)
            _root = nullptr;
        else
        {
            if (node->isBlack())
                deleteFixUp(node);

            if (node->_parent != nullptr)
            {
                if (node->_parent->_left == node)
                    node->_parent->_left = nullptr;
                else if (node->_parent->_right == node)
                    node->_parent->_right = nullptr;
                node->_parent = nullptr;
            }
        }

        deleteNode(node);
    }

    template <typename Element, typename Compar>
    void RBTree<Element, Compar>::deleteFixUp(Node *node)
    {
        while (node != _root && node->isBlack()) {
            if (node == node->_parent->_left) {
                Node* bro = node->brother();
                //if brother is red => left rotation between father and brother
                //making bro black, father - red [tree's hight is saved]
                if (bro->isRed()) {
                    bro->_color = BLACK;
                    node->_parent->_color = RED;
                    rotLeft(node->_parent);
                    bro = node->_parent->_right;
                }
                //if bro's children both black
                //bro becomes red and consider node's parent
                if ((!bro->_left || bro->_left->isBlack())
                    && (!bro->_right || bro->_right->isBlack())){
                    bro->setRed();
                    node = node->_parent;
                } else {
                    //if bro's right child red, left - black
                    //bro and left become black and make right rotation
                    if (bro->_right->isBlack()) {
                        bro->_left->_color = BLACK;
                        bro->setBlack();
                        rotRight(bro);
                        bro = node->_parent->_right;
                    }
                    //make bro the same color with father
                    //bro's child and father -> black
                    bro->_color = node->_parent->_color;
                    node->_parent->_color = BLACK;
                    if (bro->_right)
                        bro->_right->_color = BLACK;
                    rotLeft(node->_parent);
                    node = _root;
                }
            } else {
                //symmetric to the upper code
                Node* bro = node->brother();
                if (bro->isRed()) {
                    bro->setBlack();
                    node->_parent->_color = RED;
                    rotRight(node->_parent);
                    bro = node->_parent->_left;
                }
                if (bro->_left->isBlack() && bro->_right->isBlack()) {
                    bro->setRed();
                    node = node->_parent;
                } else {
                    if (bro->_left->isBlack()) {
                        bro->_right->_color = BLACK;
                        bro->_color = RED;
                        rotLeft(bro);
                        bro = node->_parent->_left;
                    }
                    bro->_color = node->_parent->_color;
                    node->_parent->_color = BLACK;
                    bro->_left->_color = BLACK;
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
        // TODO: метод реализуют студенты
        Node* current = _root;
        //search by a key from a root
        while (current)
        {
            if (current->_key == key)
                return current;

            if(key < current->_key)
                current=current->_left;
            else
                current=current->_right;
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
        Node* current = _root;
        Node* temp = nullptr;

        //there was nothing in a tree
        if (current == nullptr)
        {
            _root = node;
            node->setBlack();
            return node;
        }

        //choose the parent for a new node
        do
        {
            temp = current;
            if (node->_key < current->_key)
                current = current->_left;
            else
                current = current->_right;
        }
        while (current);

        //check for the node to be on the right or on the left
        if (node->_key < temp->_key)
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
        // TODO: этот метод студенты могут оставить и реализовать при декомпозиции балансировки дерева
        // В методе оставлены некоторые важные комментарии/snippet-ы
        if (!nd->getParent())
            return nd;

        bool f;


        // попадание в этот метод уже означает, что папа есть (а вот про дедушку пока не известно)
        //...

        Node * uncle = nd->getUncle(); // для левого случая нужен правый дядя и наоборот.

        // если дядя такой же красный, как сам нод и его папа...
        if (uncle && uncle->isRed())
        {
            // дядю и папу красим в черное
            // а дедушку — в коммунистические цвета
            nd->getDaddy(f)->setBlack();
            uncle->setBlack();
            uncle->getDaddy(f)->setRed();

            // отладочное событие
            if (_dumper)
                _dumper->rbTreeEvent(IRBTreeDumper<Element, Compar>::DE_AFTER_RECOLOR1, this, nd);

            // теперь чередование цветов "узел-папа-дедушка-дядя" — К-Ч-К-Ч, но надо разобраться, что там
            // с дедушкой и его предками, поэтому продолжим с дедушкой
            //..
            rebalanceDUG(uncle->getDaddy(f));
        }

            // дядя черный
            // смотрим, является ли узел "правильно-правым" у папочки
        else if (nd->getParent()->isLeftChild())                                        // для левого случая нужен правый узел, поэтом отрицание
        {                                               // CASE2 в действии

            // ... при вращении будет вызвано отладочное событие
            // ...
            if (nd->isRightChild())
                rotLeft(nd->getDaddy(f));

            nd->getDaddy(f)->setBlack();

            Node* grandParent = nd->getDaddy(f)->getDaddy(f);

            // отладочное событие
            if (_dumper)
                _dumper->rbTreeEvent(IRBTreeDumper<Element, Compar>::DE_AFTER_RECOLOR3D, this, nd);


            // деда в красный

            grandParent->setRed();


            // отладочное событие
            if (_dumper)
                _dumper->rbTreeEvent(IRBTreeDumper<Element, Compar>::DE_AFTER_RECOLOR3G, this, nd);

            rotRight(grandParent);

        }
        else if (nd->getParent()->isRightChild())
        {
            if (nd->isLeftChild())
                rotRight(nd->getDaddy(f));

            nd->getDaddy(f)->setBlack();
            Node* grandParent = nd->getDaddy(f)->getDaddy(f);

            // отладочное событие
            if (_dumper)
                _dumper->rbTreeEvent(IRBTreeDumper<Element, Compar>::DE_AFTER_RECOLOR3D, this, nd);


            // деда в красный
            grandParent->setRed();

            // отладочное событие
            if (_dumper)
                _dumper->rbTreeEvent(IRBTreeDumper<Element, Compar>::DE_AFTER_RECOLOR3G, this, nd);

            rotLeft(grandParent);
        }

//    // ...
//
//    // отладочное событие
//    if (_dumper)
//        _dumper->rbTreeEvent(IRBTreeDumper<Element, Compar>::DE_AFTER_RECOLOR3D, this, nd);
//
//
//    // деда в красный
//
//    // ...
//
//    // отладочное событие
//    if (_dumper)
//        _dumper->rbTreeEvent(IRBTreeDumper<Element, Compar>::DE_AFTER_RECOLOR3G, this, nd);
//
//    // ...


        return nd;
    }


    template <typename Element, typename Compar >
    void RBTree<Element, Compar>::rebalance(Node* nd)
    {

        // TODO: метод реализуют студенты

        // ...
        nd->setRed();

        // пока папа — цвета пионерского галстука, действуем
        while (nd !=_root && nd->getParent()->isRed())
        {
            // локальная перебалансировка семейства "папа, дядя, дедушка" и повторная проверка
            if (nd && nd !=_root)
                nd = rebalanceDUG(nd);
        }


        // ...
        _root->setBlack();

    }



    template <typename Element, typename Compar>
    void RBTree<Element, Compar>::rotLeft(typename RBTree<Element, Compar>::Node* nd)
    {
        // TODO: метод реализуют студенты

        // правый потомок, который станет после левого поворота "выше"
        Node* y = nd->_right;

        if (!y)
            throw std::invalid_argument("Can't rotate left since the right child is nil");

        // ...
        //changing nodeRight's left pointer
        y->_right = nd->_left;
        if (y->_left)
            y->_left->_parent = nd;

        //changing nodeRight's parent pointer
        if (y)
            y->_parent = nd->_parent;

        //changing parent's pointer
        if (nd->_parent)
            if (nd == nd->_parent->_left)
                nd->_parent->_left = y;
            else
                nd->_parent->_right = y;
        else
            _root = y; // in case node was a root

        // changing node's parent
        y->_left = nd;
        if (nd)
            nd->_parent = y;


        // отладочное событие
        if (_dumper)
            _dumper->rbTreeEvent(IRBTreeDumper<Element, Compar>::DE_AFTER_LROT, this, nd);
    }



    template <typename Element, typename Compar>
    void RBTree<Element, Compar>::rotRight(typename RBTree<Element, Compar>::Node* nd)
    {
        // TODO: метод реализуют студенты

        Node* nodeLeft = nd->_left;

        //changing nodeLeft's right pointer
        nd->_left = nodeLeft->_right;
        if (nodeLeft->_right)
            nodeLeft->_right->_parent = nd;

        //changing nodeLeft's parent pointer
        if (nodeLeft)
            nodeLeft->_parent = nd->_parent;

        //changing parent's pointer
        if (nd->_parent)
            if (nd == nd->_parent->_right)
                nd->_parent->_right = nodeLeft;
            else
                nd->_parent->_left = nodeLeft;
        else
            _root = nodeLeft;

        nodeLeft->_right = nd;
        if (nodeLeft)
            nd->_parent = nodeLeft;

        // ...

        // отладочное событие
        if (_dumper)
            _dumper->rbTreeEvent(IRBTreeDumper<Element, Compar>::DE_AFTER_RROT, this, nd);
    }


} // namespace xi

