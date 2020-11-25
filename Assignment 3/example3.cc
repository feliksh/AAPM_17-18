#include<iostream>

#include"matrix.h"
#include"matrix_wrap.h"


int main() {

matrix<int,4,5> A;
for (int i=0; i!=4; ++i)
	for(int j=0; j!=5; ++j)
	  A(i,j) = (i+1)*10+j+1;
	  
	  
for (int i=0; i!=4; ++i) {
	for(int j=0; j!=5; ++j)
		std::cout << A(i,j) << ' ';
	std::cout << '\n';
}
std::cout << A.get<1,2>() << ' ' << decltype(A)::H <<',' << decltype(A)::W << '\n';
std::cout << std::endl;


auto B=A.transpose();
for (int i=0; i!=5; ++i) {
	for(int j=0; j!=4; ++j)
		std::cout << B(i,j) << ' ';
	std::cout << '\n';
}
std::cout << B.get<1,2>() << ' ' << decltype(B)::H <<',' << decltype(B)::W  << '\n';
std::cout << std::endl;


auto C=B.transpose();
for (int i=0; i!=4; ++i) {
	for(int j=0; j!=5; ++j)
		std::cout << C(i,j) << ' ';
	std::cout << '\n';
}
std::cout << C.get<1,2>() << ' ' << decltype(C)::H <<',' << decltype(C)::W  << '\n';
std::cout << std::endl;

auto D=A.window({1,4,1,4});
for (int i=0; i!=3; ++i) {
	for(int j=0; j!=3; ++j)
		std::cout << D(i,j) << ' ';
	std::cout << '\n';
}
std::cout /*<< D.get<1,2>() << ' '*/ << decltype(D)::H <<',' << decltype(D)::W  << '\n';
std::cout << std::endl;

matrix<int,4,1> V;
for (int i=0; i!=4; ++i) V(i,0) = i+1;

auto W=V.diagonal_matrix();
for (int i=0; i!=4; ++i) {
	for(int j=0; j!=4; ++j)
		std::cout << W(i,j) << ' ';
	std::cout << '\n';
}
std::cout << W.get<2,2>() << ' ' << decltype(W)::H <<',' << decltype(W)::W << ' ' << decltype(V)::H <<',' << decltype(V)::W << '\n';
std::cout << std::endl;




return 0;
}
