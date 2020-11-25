#include<iostream>

#include"matrix.h"
#include"operations.h"


int main() {
    const int size = 200;
    int k = 1;

    matrix<int> A(size, size);
    for (int i = 0; i != size; ++i)
        for (int j = 0; j != size; ++j)
            A(i, j) = (k++) % 10;

    auto B = A.transpose();

    k=1;
    matrix<double> A_double(size, size);
    for (int i = 0; i != size; ++i)
        for (int j = 0; j != size; ++j)
            A_double(i,j) = (k++) % 10;
    auto B_double = A_double.transpose();




    matrix<int> R = A*B;
    std::cout << "first value:" << R(0,0)
              << "\nlast value:" <<R(R.get_height()-1, R.get_width()-1) << std::endl;

    matrix<int> E = A*B*B*A;
    std::cout << "first value:" << E(0, 0)
              << "\nlast value:" << E(E.get_height() - 1, E.get_width() - 1) << std::endl;

    matrix<int> S = A+B*B+A;
    std::cout << "first value:" << S(0,0)
              << "\nlast value:" << S(S.get_height()-1, S.get_width()-1) << std::endl;

    matrix<int> U = A + B*A;
    std::cout << "first value:" << U(0,0)
              << "\nlast value:" << U(U.get_height()-1, U.get_width()-1) << std::endl;

    matrix<double> L = A_double + B*A;
    std::cout << "first value:" << L(0,0)
              << "\nlast value:" << L(L.get_height()-1, L.get_width()-1) << std::endl;

    matrix<double> T = (A_double+B_double) * (B+A);
    std::cout << "first value:" << T(0,0)
              << "\nlast value:" << T(T.get_height()-1, T.get_width()-1) << std::endl;

    return 0;
}
























