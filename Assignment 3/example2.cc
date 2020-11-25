#include<iostream>

#include"matrix.h"
#include"matrix_wrap.h"


int main() {

matrix<int> A(4,5);
for (int i=0; i!=4; ++i)
	for(int j=0; j!=5; ++j)
	  A(i,j) = (i+1)*10+j+1;
	  
	  
for (int i=0; i!=4; ++i) {
	for(int j=0; j!=5; ++j)
		std::cout << A(i,j) << ' ';
	std::cout << '\n';
}
std::cout << std::endl;

matrix_wrap<int> B=A.transpose();
for (int i=0; i!=5; ++i) {
	for(int j=0; j!=4; ++j)
		std::cout << B(i,j) << ' ';
	std::cout << '\n';
}
std::cout << std::endl;

matrix_wrap<int> C=B.transpose();
for (int i=0; i!=4; ++i) {
	for(int j=0; j!=5; ++j)
		std::cout << C(i,j) << ' ';
	std::cout << '\n';
}
std::cout << std::endl;

matrix_wrap<int> D=A.window({1,4,1,4});
for (int i=0; i!=3; ++i) {
	for(int j=0; j!=3; ++j)
		std::cout << D(i,j) << ' ';
	std::cout << '\n';
}
std::cout << std::endl;

matrix<int> V(4,1);
for (int i=0; i!=4; ++i) V(i,0) = i+1;

matrix_wrap<int> W=V.diagonal_matrix();
for (int i=0; i!=4; ++i) {
	for(int j=0; j!=4; ++j)
		std::cout << W(i,j) << ' ';
	std::cout << '\n';
}
std::cout << std::endl;

return 0;
}
