#include <iostream>
#include "Matrix.h"

using namespace std;

int main() {

    const unsigned int a = 3;

    Matrix<int> matrix(4, 4);
    Matrix<int> matrix2 = matrix;

    matrix.set_value(0, 2, 23);

    matrix.print_matrix();
    matrix2.print_matrix();


    matrix2.print_matrix();

    return 0;
}