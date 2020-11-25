#ifndef MATRIXLIB_LAZY_PRODUCT_H
#define MATRIXLIB_LAZY_PRODUCT_H

#include "matrix.h"
#include "matrix_wrap.h"

template<typename T>
matrix<T> product(const matrix_wrap<T> &left, const matrix_wrap<T> &right) {
    matrix<T> result(left.get_height(), right.get_width());
    for (int i = 0; i < left.get_height(); i++)
        for (int j = 0; j < right.get_width(); j++)
            for (int k = 0; k < right.get_height(); k++)
                result(i, j) += left(i, k) * right(k, j);

    return result;
}

template<typename T>
matrix<T> solve(std::vector<matrix_wrap<T>> list) {
    while(list.size()!=1) {
        for (int i = list.size() - 1; 0 < i; i--) {
            if (list[i].get_width() <= list[i - 1].get_width()) {
                matrix<T> aux = product(list[i - 1], list[i]);
                list[i - 1] = aux;
                list.erase(list.begin() + i);
            }
        }

        if(list.size()!=1)
            for (int i = 0; i < list.size() - 1; i++) {
                    if (list[i].get_width() <= list[i + 1].get_width()) {
                        matrix<T> aux = product(list[i], list[i+1]);
                        list[i] = aux;
                        list.erase(list.begin() + i + 1);
                    }
                }
    }
    return list[0];
}

template<typename T>
class lazy_product {
public:
    virtual const std::vector<matrix_wrap<T>>& get_list() const {
        return list;
    }

    template<class type>
    matrix<T> operator+(const matrix_ref<T, type> &right) {
        return solve(list) + right;
    }

    matrix<T> operator+(const lazy_product<T> &r) {
        return solve(list) + solve(r.get_list());
    }

    /*
     * This function is called if right matrix is dynamic, returns a dynamic lazy_product.
     */
    template<class type>
    typename std::enable_if<!matrix_ref<T, type>::staticMatrix, lazy_product<T>>::type
    operator*(matrix_ref<T, type>& right) {
        if (list[list.size()-1].get_width() != right.get_height())
            throw std::runtime_error(mult_err);
        list.push_back(matrix_wrap<T>(right));
        return *this;
    }

    /*
     * This function is called if right matrix is static, returns a lazy_product_static.
     */
    template<class type>
    typename std::enable_if<matrix_ref<T, type>::staticMatrix, lazy_product_static<T, matrix_ref<T, type>::get_height_static(), matrix_ref<T, type>::get_height_static()>>::type
    operator*(matrix_ref<T, type> &right) {
        if (list[list.size()-1].get_width != right.get_height())
            throw std::runtime_error(mult_err);

        list.push_back(matrix_wrap<T>(right));
        lazy_product_static<T, matrix_ref<T, type>::get_height_static(), matrix_ref<T, type>::get_height_static()> result(list);
        return result;
    }

    template <class leftType, class rightType>
    lazy_product(const matrix_ref<T, leftType> &left, const matrix_ref<T, rightType> &right) {
        list.push_back(left);
        list.push_back(right);
    }

    lazy_product(const std::vector<matrix_wrap<T>>& list) {
        this->list = list;
    }

    lazy_product() {}

protected:
    std::vector<matrix_wrap<T>> list;
};

template<typename T, unsigned height, unsigned width>
class lazy_product_static : public lazy_product<T> {
public:
    using lazy_product<T>::list;

    /*
     * This function is called if right matrix is static, returns a lazy_product_static.
     */
    template<class type>
    typename std::enable_if<matrix_ref<T, type>::staticMatrix, lazy_product_static<T, matrix_ref<T, type>::get_height_static(), matrix_ref<T, type>::get_height_static()>>::type
    operator*(matrix_ref<T, type> &right) {
        static_assert(width == matrix_ref<T, type>::get_height_static(), "Left matrix's column number must be equal to right matrix's row number");
        list.push_back(matrix_wrap<T>(right));
        return lazy_product_static<T, matrix_ref<T, type>::get_height_static(), matrix_ref<T, type>::get_height_static()>(list);
    }

    template <class leftType, class rightType>
    lazy_product_static(const matrix_ref<T, leftType> &left, const matrix_ref<T, rightType> &right) {
        list.push_back(left);
        list.push_back(right);
    }

    lazy_product_static(std::vector<matrix_wrap<T>> list) : lazy_product<T>(list) {}

    lazy_product_static() {}
};

#endif //MATRIXLIB_LAZY_PRODUCT_H