#ifndef MATRIXLIB_OPERATIONS_H
#define MATRIXLIB_OPERATIONS_H

#include "matrix_fwd.h"

/*
 * Sum is redefined twice for matrices: the first is enabled if the two matrices are both static, the second otherwise.
 */
template <typename T, class leftType, class rightType>
typename std::enable_if<matrix_ref<T, rightType>::staticMatrix && matrix_ref<T, leftType>::staticMatrix, matrix<T, matrix_ref<T, rightType>::get_height_static(), matrix_ref<T, rightType>::get_width_static()>>::type
operator + (const matrix_ref<T, leftType>& left_m, const matrix_ref<T, rightType>& right_m) {
    static_assert(matrix_ref<T, leftType>::get_height_static() == matrix_ref<T, rightType>::get_height_static() && matrix_ref<T, leftType>::get_width_static() == matrix_ref<T, rightType>::get_width_static(), "Matrices must have same dimensions");
    matrix<T, matrix_ref<T, leftType>::get_height_static(), matrix_ref<T, leftType>::get_width_static()> result;
    for (int i = 0; i < left_m.get_height(); i++)
        for (int j = 0; j < left_m.get_width(); j++)
            result(i, j) =  left_m(i, j) + right_m(i, j);
    return result;
}

template <typename T, class leftType, class rightType>
typename std::enable_if<!matrix_ref<T, rightType>::staticMatrix || !matrix_ref<T, leftType>::staticMatrix, matrix<T>>::type
operator + (const matrix_ref<T, leftType>& left_m, const matrix_ref<T, rightType>& right_m) {
    if (right_m.get_height() != left_m.get_height() || right_m.get_width() != left_m.get_width())
        throw std::out_of_range("Matrices must have same dimensions");
    matrix<T> result(left_m.get_height(), left_m.get_width());
    for (int i = 0; i < left_m.get_height(); i++)
        for (int j = 0; j < left_m.get_width(); j++)
            result(i, j) = left_m(i, j) +right_m(i, j);
    return result;
}

/*
 * Sum is defined between a matrix and a lazy_product too.
 */

template <typename T, class type>
matrix<T> operator + (const matrix_ref<T, type>& m, const lazy_product<T> &p) {
    matrix<T> aux = solve(p.get_list());
    return aux + m;
}

/*
 * Operator * is redefined.
 * The return type will be a a dynamic lazy_product class in case at least one matrix is dynamic,
 * lazy_product_static if both are static.
 */
template <typename T, class leftType, class rightType>
typename std::enable_if<!matrix_ref<T, leftType>::staticMatrix || !matrix_ref<T, rightType>::staticMatrix, lazy_product<T>>::type
operator * (const matrix_ref<T, leftType>& left_m, const matrix_ref<T, rightType>& right_m) {
    if (left_m.get_width() != right_m.get_height())
        throw std::runtime_error("Left matrix's column number must be equal to right matrix's row number");
    return lazy_product<T>(left_m,right_m);
}

template <typename T, class leftType, class rightType>
typename std::enable_if<matrix_ref<T, leftType>::staticMatrix && matrix_ref<T, rightType>::staticMatrix, lazy_product_static<T, matrix_ref<T, rightType>::get_height_static(), matrix_ref<T, rightType>::get_width_static()>>::type
operator * (const matrix_ref<T, leftType>& left_m, const matrix_ref<T, rightType>& right_m) {
    static_assert(matrix_ref<T, leftType>::get_width_static() == matrix_ref<T, rightType>::get_height_static(), "Left matrix's column number must be equal to right matrix's row number");
    return lazy_product_static<T, matrix_ref<T, rightType>::get_height_static(), matrix_ref<T, rightType>::get_width_static()>(left_m,right_m);
}

#endif //MATRIXLIB_OPERATIONS_H