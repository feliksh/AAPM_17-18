#include<iostream>

#include"matrix.h"

template <typename T, class matrix_type>
void print(matrix_ref<T, matrix_type>& m)  {
    for (int i = 0; i < m.get_height(); i++) {
        for (int j = 0; j < m.get_width(); j++)
            std::cout << m(i, j) << "\t";
        std::cout << std::endl;
    }
    std::cout << std::endl;
}

template <typename T, class matrix_type>
void print(matrix_ref<T, matrix_type>& m, bool)  {
    for (int i = 0; i < m.get_height(); i++) {
        for (int j = 0; j < m.get_width(); j++) {
            std::cout << m(i, j) << "\t";
            if (m(i, j) / 1000 < 1)
                std::cout << "\t";
        }
        std::cout << std::endl;
    }
    std::cout << std::endl;
}

int main() {
    matrix<int> A(3, 4);
    matrix<int, 3, 4> B;

    std::cout << "Creation of dynamic and static matrices:" << std::endl;
    std::cout << std::endl;

    for (int i = 0; i < A.get_height(); i++)
        for (int j = 0; j < A.get_width(); j++)
            A(i, j) = (i + 1) * 10 + j + 1;

    for (int i = 0; i < B.get_height(); i++)
        for (int j = 0; j < B.get_width(); j++)
            B(i, j) = (i + 1) * 10 + j + 1;

    std::cout << "Matrix<int> A(3, 4):" << std::endl;
    print(A);

    std::cout << "Matrix<int, 3, 4> B:" << std::endl;
    print(B);

    std::cout << "Get statico (0, 0):" << std::endl;
    std::cout << B.get<0, 0>() << std::endl;
    std::cout << std::endl;

    std::cout << "=======================================" << std::endl << std::endl;

    std::cout << "Creation of transformated matrices:" << std::endl;
    std::cout << std::endl;

    auto A_transpose = A.transpose();
    auto A_window = A.window({0, 3, 0, 3});
    auto A_diagonal = A.diagonal();
    auto A_diagonalMatrix = A_diagonal.diagonal_matrix();

    std::cout << "Dynamic transpose:" << std::endl;
    print(A_transpose);
    std::cout << "Dynamic window (1, 3, 1, 3):" << std::endl;
    print(A_window);
    std::cout << "Dynamic diagonal:" << std::endl;
    print(A_diagonal);
    std::cout << "Dynamic diagonal_matrix:" << std::endl;
    print(A_diagonalMatrix);

    auto B_transpose = B.transpose();
    auto B_window = B.window<0, 3, 0, 3>();
    auto B_diagonal = B.diagonal();
    auto B_diagonalMatrix = B_diagonal.diagonal_matrix();

    std::cout << "Static transpose:" << std::endl;
    print(B_transpose);
    std::cout << "Static window (1, 3, 1, 3):" << std::endl;
    print(B_window);
    std::cout << "Static diagonal:" << std::endl;
    print(B_diagonal);
    std::cout << "Static diagonal_matrix:" << std::endl;
    print(B_diagonalMatrix);

    std::cout << "=======================================" << std::endl << std::endl;

    std::cout << "Operations:" << std::endl;
    std::cout << std::endl;

    std::cout << "Dynamic sum:" << std::endl;
    std::cout << std::endl;

    matrix<int> DynamicSum = A + A + A;
    std::cout << "A + A + A:" << std::endl;
    print(DynamicSum);

    matrix<int> DynamicSumTranspose = A_transpose + A_transpose + A_transpose;
    std::cout << "A_transpose + A_transpose + A_transpose:" << std::endl;
    print(DynamicSumTranspose);

    matrix<int> DynamicSumWindow = A_window + A_window + A_window;
    std::cout << "A_window + A_window + A_window:" << std::endl;
    print(DynamicSumWindow);

    matrix<int> DynamicSumDiagonal = A_diagonal + A_diagonal + A_diagonal;
    std::cout << "A_diagonal + A_diagonal + A_diagonal:" << std::endl;
    print(DynamicSumDiagonal);

    matrix<int> DynamicSumDiagonalMatrix = A_diagonalMatrix + A_diagonalMatrix + A_diagonalMatrix;
    std::cout << "A_diagonalMatrix + A_diagonalMatrix + A_diagonalMatrix:" << std::endl;
    print(DynamicSumDiagonalMatrix);

    std::cout << "Dynamic product:" << std::endl;
    std::cout << std::endl;

    matrix<int> DynamicProduct = A * A_transpose * A;
    std::cout << "A * A_transpose * A:" << std::endl;
    print(DynamicProduct, true);


    matrix<int> DynamicProductTranspose = A_transpose * A * A_transpose;
    std::cout << "A_tranpose * A * A_transpose:" << std::endl;
    print(DynamicProductTranspose, true);


    matrix<int> DynamicProductWindow = A_window * A_window * A_window;
    std::cout << "A_window * A_window * A_window:" << std::endl;
    print(DynamicProductWindow, true);

    matrix<int> DynamicProductDiagonal = A_transpose * A_diagonal;
    std::cout << "A_transpose * A_diagonal:" << std::endl;
    print(DynamicProductDiagonal, true);

    std::cout << "=======================================" << std::endl << std::endl;

    std::cout << "Static sum:" << std::endl;
    std::cout << std::endl;

    matrix<int, 3, 4> StaticSum = B + B + B;
    std::cout << "B + B + B:" << std::endl;
    print(StaticSum);

    matrix<int, 4, 3> StaticSumTranspose = B_transpose + B_transpose + B_transpose;
    std::cout << "B_transpose + B_transpose + B_transpose:" << std::endl;
    print(StaticSumTranspose);


    matrix<int, 3, 3> StaticSumWindow = B_window + B_window + B_window;
    std::cout << "B_window + B_window + B_window:" << std::endl;
    print(StaticSumWindow);

    matrix<int, 3, 1> StaticSumDiagonal = B_diagonal + B_diagonal + B_diagonal;
    std::cout << "B_diagonal + B_diagonal + B_diagonal:" << std::endl;
    print(StaticSumDiagonal);


    matrix<int> StaticSumDiagonalMatrix = B_diagonalMatrix + B_diagonalMatrix + B_diagonalMatrix;
    std::cout << "B_diagonalMatrix + B_diagonalMatrix + B_diagonalMatrix:" << std::endl;
    print(DynamicSumDiagonalMatrix);


    std::cout << "Static product:" << std::endl;
    std::cout << std::endl;

    matrix<int, 3, 4> StaticProduct = B * B_transpose * B;
    std::cout << "B * B_transpose * B:" << std::endl;
    print(StaticProduct, true);


    matrix<int, 4, 3> StaticProductTranspose = B_transpose * B * B_transpose;
    std::cout << "B_tranpose * B * B_transpose:" << std::endl;
    print(StaticProductTranspose, true);

    /*
    matrix<int> StaticProductWindow = B_window *  B_window * B_window;
    std::cout << "B_window * B_window * B_window:" << std::endl;
    print(StaticProductWindow, true);
    */

    matrix<int, 4, 1> StaticProductDiagonal = B_transpose * B_diagonal;
    std::cout << "B_transpose * B_diagonal:" << std::endl;
    print(StaticProductDiagonal, true);

    std::cout << "=======================================" << std::endl << std::endl;

    std::cout << "Mix of static and dynamic sum and product:" << std::endl;
    std::cout << std::endl;

    matrix<int> mix1 = B + A + A * B_transpose * A;
    std::cout << "B + A + A * B_transpose + B_window" << std::endl;
    print(mix1, true);

    matrix<int> mix2 = B_diagonalMatrix + A_window + A_window.transpose() * A_window;
    std::cout << "B_diagonalMatrix + A_window + A_window.transpose() * A_window" << std::endl;
    print(mix2, true);

    std::cout << "=======================================" << std::endl << std::endl;

    std::cout << "Exceptions and compile-time errors list:" << std::endl;
    std::cout << std::endl;

    std::cout << "Exception: int a = A(10, 10);" << std::endl;
    std::cout << std::endl;
    //int a = A(10, 10);

    std::cout << "Compile-time error: int b = B.get<10, 10>();" << std::endl;
    std::cout << std::endl;
    //int b = B.get<10, 10>();

    std::cout << "Exception: auto C = A + A_transpose;" << std::endl;
    std::cout << std::endl;
    //auto C = A + A_transpose;

    std::cout << "Compile-time error: auto C = B + B_transpose;" << std::endl;
    std::cout << std::endl;
    //auto C = B + B_transpose;

    std::cout << "Exception: auto C = A * A;" << std::endl;
    std::cout << std::endl;
    //auto C = A * A;

    std::cout << "Compile-time error: auto C = B * B;" << std::endl;
    std::cout << std::endl;
    //auto C = B * B;

    return 0;
}