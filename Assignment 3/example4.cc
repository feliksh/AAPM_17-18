#include<iostream>
#include <chrono>
#include<fstream>
#include <zconf.h>

#include"matrix.h"
#include"matrix_wrap.h"
#include"operations.h"



int main() {


matrix<int> A(4,4);
for (int i=0; i!=4; ++i)
	for(int j=0; j!=4; ++j)
	  A(i,j) = (i+1)*10+j+1;
	  
	  
for (int i=0; i!=4; ++i) {
	for(int j=0; j!=4; ++j)
		std::cout << A(i,j) << ' ';
	std::cout << '\n';
}
std::cout << std::endl;


auto B=A.transpose();
for (int i=0; i!=4; ++i) {
	for(int j=0; j!=4; ++j)
		std::cout << B(i,j) << ' ';
	std::cout << '\n';
}
std::cout << std::endl;

matrix<int> C = A+B+A;
for (int i=0; i!=4; ++i) {
	for(int j=0; j!=4; ++j)
		std::cout << C(i,j) << ' ';
	std::cout << '\n';
}
std::cout << std::endl;

matrix<double> D(4,4);
for (int i=0; i!=4; ++i)
	for(int j=0; j!=4; ++j)
	  D(i,j) = 1;

std::cout << "=======\n";
A+B+D;
std::cout << "=======\n";
D+A+B;
std::cout << "=======\n";

matrix<int,4,4> A2;
for (int i=0; i!=4; ++i)
	for(int j=0; j!=4; ++j)
	  A2(i,j) = (i+1)*10+j+1;
	  
	  
for (int i=0; i!=4; ++i) {
	for(int j=0; j!=4; ++j)
		std::cout << A2(i,j) << ' ';
	std::cout << '\n';
}
std::cout << std::endl;


auto B2=A2.transpose();
for (int i=0; i!=4; ++i) {
	for(int j=0; j!=4; ++j)
		std::cout << B2(i,j) << ' ';
	std::cout << '\n';
}
std::cout << std::endl;

matrix<int,4,4> C2 = A2+B2+A2;
for (int i=0; i!=4; ++i) {
	for(int j=0; j!=4; ++j)
		std::cout << C2(i,j) << ' ';
	std::cout << '\n';
}
std::cout << std::endl;


matrix<double,4,4> D2;
for (int i=0; i!=4; ++i)
	for(int j=0; j!=4; ++j)
	  D(i,j) = 1;

std::cout << "=======\n";
A2+B2+D2;
std::cout << "=======\n";
D2+A2+B2;
std::cout << "=======\n";


A+B2;
A2+B;

std::cout << "=======\n";

matrix<int> M = A*B*C;

for (int i=0; i!=4; ++i) {
	for(int j=0; j!=4; ++j)
		std::cout << M(i,j) << ' ';
	std::cout << '\n';
}
std::cout << std::endl;



std::cout << "=======\n";

matrix<int,4,4> M2 = A2*B2*C2;

for (int i=0; i!=4; ++i) {
	for(int j=0; j!=4; ++j)
		std::cout << M2(i,j) << ' ';
	std::cout << '\n';
}
std::cout << std::endl;


matrix<int,4,4> Z=A;
//matrix<int,4,5> Z2=A2;

std::cout << "=======\n";

matrix<int,4,4> M3 = A*B*C;

for (int i=0; i!=4; ++i) {
	for(int j=0; j!=4; ++j)
		std::cout << M3(i,j) << ' ';
	std::cout << '\n';
}
std::cout << std::endl;

return 0;
}
