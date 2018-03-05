////////////////////////////////////////////////////////////////////////////////
/// \file
/// \brief     Unit tests for some xi::RBTree private implementation
/// \author    Sergey Shershakov
/// \version   0.1.0
/// \date      01.05.2017
///            This is a part of the course "Algorithms and Data Structures" 
///            provided by  the School of Software Engineering of the Faculty 
///            of Computer Science at the Higher School of Economics.
///
/// Gtest-based unit test.
/// The naming conventions imply the name of a unit-test module is the same as 
/// the name of the corresponding tested module with _test suffix
///
/// Тестирование закрытых методов, что считается плохим подходом, т.к. 
/// unit-тестирование должно работать с тестируемым материалом, как с черным
/// ящиком. Однако в специальных случаях тестирование закрытой реализации
/// допустимо, как, например, в случае со сложными методами вращения поддеревьев,
/// которые (методы) через интерфейсные методы вызываются многократно и 
/// отследить корректность их выполнения по посткондиции довольно затруднительно.
/// https://github.com/google/googletest/blob/master/googletest/docs/AdvancedGuide.md#testing-private-code
///
////////////////////////////////////////////////////////////////////////////////

#include <gtest/gtest.h>

#include "rbtree.h"
#include "def_dumper.h"
#include "individual.h"


namespace xi {

/** \brief Тестовый класс для тестирования закрытой части классов черно-красного дерева.
 *
 *  Данный класс, с одной стороны, представляется полноценный g-тестером, с другой —
 *  ему классы КЧД представляют дружественные полномочия, поэтому он может смело
 *  покопаться во внутренностях этих классов.
 */
template <typename Element, typename Compar = std::less<Element> >
class RBTreeTest : public ::testing::Test {
public:
    // Объявление типов дерева и узла для упрощения доступа
    typedef RBTree<Element, Compar> TTree;
    typedef typename RBTree<Element, Compar>::Node TTreeNode;
    typedef typename RBTree<Element, Compar>::Color TTreeColor;

public:
    static const int STRUCT2_SEQ[];
    static const int STRUCT2_SEQ_NUM;

public:
    RBTreeTest()
        //: _dumper(DUMP_EVENTLOG_PRV_FN, DUMP_IMGS_PRV_PATH)
    {        
    }
protected:

    /** \brief Возвращает ссылку на неконстантный указатель на корень дерева \c tree.  
     *
     *  Обратите внимание на это чудесное сочетание звездочек и амперсандиков!
     *  Оно не случайно...
     */
    typename TTreeNode* & getRootNode(TTree* tree)
    {
        return tree->_root;
    }


    /** \brief Создает узел без привязки к дереву */
    typename TTreeNode* createNode(
        const Element& key = Element(),
        typename TTreeNode* left = nullptr,
        typename TTreeNode* right = nullptr,
        typename TTreeNode* parent = nullptr,
        typename TTreeColor col = TTree::BLACK)
    {
        TTreeNode* newNode = new TTreeNode(key, left, right, parent, col);

        return newNode;
    }

    /** \brief Вращает узел \c node влево. */
    void rotNodeLeft(TTree* tree, TTreeNode* node)
    {
        tree->rotLeft(node);
    }


    /** \brief Вращает узел \c node вправо. */
    void rotNodeRight(TTree* tree, TTreeNode* node)
    {
        tree->rotRight(node);
    }

    /** \brief Для данного узла возвращает его левого потомка. */
    typename TTreeNode* getLeftChild(TTreeNode* node)
    {
        return node->_left;
    }

    /** \brief Для данного узла возвращает его правого потомка. */
    typename TTreeNode* getRightChild(TTreeNode* node)
    {
        return node->_right;
    }

    /** \brief Для данного узла возвращает его предка. */
    typename TTreeNode* getParentChild(TTreeNode* node)
    {
        return node->_parent;
    }
    
    /** \brief Вставляет элемент \c el в BST без учета свойств КЧД. */
    typename TTreeNode* insertNewBstEl(TTree* tree, const Element& el)
    {
        return tree->insertNewBstEl(el);
    }


protected:
    // Некоторые методы инициализации деревьев
    TTree& createStruct1(TTree& tree)
    {
        // создаем структуру с [Рисунка 1]

        TTreeNode* n1 = createNode(1);
        TTreeNode* n4 = createNode(4);
        TTreeNode* n6 = createNode(6);
        TTreeNode* n5 = createNode(5, n4, n6);
        TTreeNode* n3 = createNode(3, n1, n5);

        // устанавливаем 3 как корень
        TTreeNode* & rt = getRootNode(&tree);
        rt = n3;

        return tree;
    }



protected:
    //RBTreeDefDumper<Element, Compar> _dumper;

    /** \brief Выводить в формате GraphViz. */
    RBTreeGvDumper<Element, Compar> _gvDumper;

}; // class RBTreeTest


// Вынесенная инициализация массива
template <typename Element, typename Compar>
const int RBTreeTest<Element, Compar>::STRUCT2_SEQ[] = 
    {  4, 50, 10, 40, 17, 35, 20, 27, 37, 45, 60, 21, 1, 30 };

template <typename Element, typename Compar>
const int RBTreeTest<Element, Compar>::STRUCT2_SEQ_NUM = sizeof(STRUCT2_SEQ) / sizeof(STRUCT2_SEQ[0]);


// Тестируем на целых числах.
typedef RBTree<int> RBTreeInt;

// Конкретизация шаблонного класса для далеетестируемого целочисленного дерева
typedef RBTreeTest<int> RBTreeIntTester;


TEST_F(RBTreeIntTester, Simplest)
{
    RBTreeInt tree;

    TTreeNode* & rt = getRootNode(&tree);
    EXPECT_EQ(nullptr, rt);
}

// тестирует ручное добавление корня
TEST_F(RBTreeIntTester, AddRootManually)
{
    RBTreeInt tree;

    // берем ссылку на корень
    TTreeNode* & rt = getRootNode(&tree);
    rt = createNode();
    EXPECT_NE(nullptr, rt);
    EXPECT_FALSE(tree.isEmpty());           // теперь должен быть непустым!
}


// создание простой структуры дерева
TEST_F(RBTreeIntTester, SimpleStruct1)
{
    // создаем структуру с [Рисунка 1]
    RBTreeInt tree;
    createStruct1(tree);

    // выводим в отладочный файл
    std::string fn(DUMP_IMGS_PRV_PATH);
    fn.append("SimpleStruct1.gv");

    _gvDumper.dump(fn, tree);

}


// левый поворот
TEST_F(RBTreeIntTester, LeftRot1)
{
    // создаем структуру с [Рисунка 1]
    RBTreeInt tree;
    createStruct1(tree);

    // выводим в отладочный файл до
    std::string fn1(DUMP_IMGS_PRV_PATH);
    fn1.append("LeftRot1Before.gv");
    _gvDumper.dump(fn1, tree);

    // вращаем относительно n3 влево
    TTreeNode* & rt = getRootNode(&tree);
    TTreeNode* n3 = rt;                     // сохраняем для последующего изучения
    TTreeNode* n5 = getRightChild(n3);
    rotNodeLeft(&tree, rt);

    // выводим в отладочный файл после
    std::string fn2(DUMP_IMGS_PRV_PATH);
    fn2.append("LeftRot1After.gv");
    _gvDumper.dump(fn2, tree);


    // постусловия
    EXPECT_EQ(5, rt->getKey());
    EXPECT_EQ(nullptr, rt->getParent());
    
    
    EXPECT_EQ(nullptr, n5->getParent());
    EXPECT_EQ(n5, n3->getParent());
    EXPECT_EQ(n3, n5->getLeft());
}


// правый поворот 1
TEST_F(RBTreeIntTester, RightRot1)
{
    // создаем структуру с [Рисунка 1]
    RBTreeInt tree;
    createStruct1(tree);

    // выводим в отладочный файл до
    std::string fn1(DUMP_IMGS_PRV_PATH);
    fn1.append("RightRot1Before.gv");
    _gvDumper.dump(fn1, tree);

    
    TTreeNode* & rt = getRootNode(&tree);
    TTreeNode* n5 = getRightChild(rt);
    
    TTreeNode* n4 = getLeftChild(n5);       // эти два для посл. изучения
    TTreeNode* n6 = getRightChild(n5);

    // вращаем относительно n5 вправо
    rotNodeRight(&tree, n5);

    // выводим в отладочный файл после
    std::string fn2(DUMP_IMGS_PRV_PATH);
    fn2.append("RightRot1After.gv");
    _gvDumper.dump(fn2, tree);


    // постусловия
    EXPECT_EQ(3, rt->getKey());
    EXPECT_EQ(nullptr, rt->getParent());

    EXPECT_EQ(n4, rt->getRight());
    EXPECT_EQ(rt, n4->getParent());

    EXPECT_EQ(n5, n4->getRight());
    EXPECT_EQ(nullptr, n4->getLeft());
    EXPECT_EQ(nullptr, n5->getLeft());

    EXPECT_EQ(n6, n5->getRight());
    EXPECT_EQ(n5, n6->getParent());

}



// правый поворот 2
TEST_F(RBTreeIntTester, RightRot2)
{
    // создаем структуру с [Рисунка 1]
    RBTreeInt tree;
    createStruct1(tree);

    // выводим в отладочный файл до
    std::string fn1(DUMP_IMGS_PRV_PATH);
    fn1.append("RightRot2Before.gv");
    _gvDumper.dump(fn1, tree);

  

    // сохраняем указатели на инд. узлы
    TTreeNode* & rt = getRootNode(&tree);
    TTreeNode* n3 = rt;
    TTreeNode* n5 = getRightChild(rt);
    TTreeNode* n1 = getLeftChild(rt);
    TTreeNode* n4 = getLeftChild(n5);       // эти два для посл. изучения
    TTreeNode* n6 = getRightChild(n5);

    rotNodeRight(&tree, n3);


    // выводим в отладочный файл после
    std::string fn2(DUMP_IMGS_PRV_PATH);
    fn2.append("RightRot2After.gv");
    _gvDumper.dump(fn2, tree);


    // постусловия
    EXPECT_EQ(nullptr, n1->getParent());
    EXPECT_EQ(n1, n3->getParent());

    EXPECT_EQ(nullptr, n1->getLeft());
    EXPECT_EQ(nullptr, n3->getLeft());

    EXPECT_EQ(n5, n3->getRight());
    EXPECT_EQ(n3, n5->getParent());

}


// левый поворот 2
TEST_F(RBTreeIntTester, LeftRot2)
{
    // создаем структуру с [Рисунка 1]
    RBTreeInt tree;
    createStruct1(tree);

    // выводим в отладочный файл до
    std::string fn1(DUMP_IMGS_PRV_PATH);
    fn1.append("LeftRot2Before.gv");
    _gvDumper.dump(fn1, tree);



    // сохраняем указатели на инд. узлы
    TTreeNode* & rt = getRootNode(&tree);
    TTreeNode* n3 = rt;
    TTreeNode* n5 = getRightChild(rt);
    TTreeNode* n1 = getLeftChild(rt);
    TTreeNode* n4 = getLeftChild(n5);       // эти два для посл. изучения
    TTreeNode* n6 = getRightChild(n5);

    rotNodeLeft(&tree, n5);


    // выводим в отладочный файл после
    std::string fn2(DUMP_IMGS_PRV_PATH);
    fn2.append("LeftRot2After.gv");
    _gvDumper.dump(fn2, tree);


    // постусловия
    EXPECT_EQ(nullptr, n3->getParent());
    EXPECT_EQ(n3, n1->getParent());
    EXPECT_EQ(n3, n6->getParent());
    EXPECT_EQ(n1, n3->getLeft());
    EXPECT_EQ(n6, n3->getRight());

    EXPECT_EQ(nullptr, n6->getRight());
    EXPECT_EQ(n5, n6->getLeft());

    EXPECT_EQ(n6, n5->getParent());
    EXPECT_EQ(nullptr, n5->getRight());
    EXPECT_EQ(n4, n5->getLeft());
}



// внутренняя вставка элемента — добавление нода
TEST_F(RBTreeIntTester, insertNewBstEl1)
{
    RBTreeInt tree;

    for (int i = 0; i < STRUCT2_SEQ_NUM; ++i)
        insertNewBstEl(&tree, STRUCT2_SEQ[i]);


    std::string fn1(DUMP_IMGS_PRV_PATH);
    fn1.append("InsertNewBstEl1.gv");
    _gvDumper.dump(fn1, tree);
}





} // namespace xi