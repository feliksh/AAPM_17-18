#ifndef _MATRIX_FWD_H_
#define _MATRIX_FWD_H_


struct Plain;
template<unsigned height, unsigned width> struct Sized;
template<class decorated> struct Transpose;
template<class decorated> struct Window;
template<class decorated> struct Diagonal;
template<class decorated> struct Diagonal_matrix;

template<typename T, class matrix_type=Plain> class matrix_ref;


struct window_spec { unsigned row_start, row_end, col_start, col_end; };

#endif //_MATRIX_FWD_H_
