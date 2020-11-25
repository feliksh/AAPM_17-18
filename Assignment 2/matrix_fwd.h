#ifndef _MATRIX_FWD_H_
#define _MATRIX_FWD_H_

struct Plain;
template<unsigned height = 0, unsigned width = 0> struct SizedPlain;
template<class decorated> struct Transpose;
//template<class decorated> struct Window;
template<class decorated, unsigned ... dimensions> struct Window;
template<class decorated> struct Diagonal;
template<class decorated> struct Diagonal_matrix;

template<typename T, class matrix_type=Plain> class matrix_ref;

struct window_spec { unsigned row_start, row_end, col_start, col_end; };

template<typename T, unsigned ... dimensions> class matrix;

template <typename T> class lazy_product;
template <typename T, unsigned height, unsigned width> class lazy_product_static;

/*
 * Error message definitions.
 */

const std::string dim_err = "Matrices must have same dimensions";
const std::string mult_err = "Left matrix's column number must be equal to right matrix's row number";
const std::string index_err = "You tried to access to an invalid location";
const std::string empty_err = "You can not construct a matrix without elements";

#endif //_MATRIX_FWD_H_