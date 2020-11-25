#ifndef _MATRIX_ITERATORS_H_
#define _MATRIX_ITERATORS_H_

#include"matrix_fwd.h"


template<typename T, class matrix_type>
class index_col_iterator {
	matrix_ref<T, matrix_type>& ref;
	unsigned cur_row, cur_col;
	
	public:
	
	index_col_iterator& operator ++() {
		++cur_row;
		if (cur_row==ref.get_height()) {
			cur_row=0;
			++cur_col;
		}
		return *this;
	}
	
	T& operator *() {
		return ref(cur_row,cur_col);
	}
	
	bool operator == (const index_col_iterator& X) const {
		return cur_row==X.cur_row && cur_col==X.cur_col;
	}
	bool operator != (const index_col_iterator& X) const {
		return ! operator==(X);
	}
	
	index_col_iterator(matrix_ref<T, matrix_type>&X, unsigned row, unsigned col) : 
			ref(X), cur_row(row), cur_col(col) {}
};

template<typename T, class matrix_type>
class const_index_col_iterator {
	const matrix_ref<T, matrix_type>& ref;
	unsigned cur_row, cur_col;
	
	public:
	
	const_index_col_iterator& operator ++() {
		++cur_row;
		if (cur_row==ref.get_height()) {
			cur_row=0;
			++cur_col;
		}
		return *this;
	}
	
	const T& operator *() {
		return ref(cur_row,cur_col);
	}
	
	bool operator == (const const_index_col_iterator& X) const {
		return cur_row==X.cur_row && cur_col==X.cur_col;
	}
	bool operator != (const const_index_col_iterator& X) const {
		return ! operator==(X);
	}
	
	const_index_col_iterator(const matrix_ref<T, matrix_type>&X, unsigned row, unsigned col) : 
			ref(X), cur_row(row), cur_col(col) {}
};



template<typename T, class matrix_type>
class index_row_iterator {
	matrix_ref<T, matrix_type>& ref;
	unsigned cur_row, cur_col;
	
	public:
	
	index_row_iterator& operator ++() {
		++cur_col;
		if (cur_col==ref.get_width()) {
			cur_col=0;
			++cur_row;
		}
		return *this;
	}
	
	T& operator *() {
		return ref(cur_row,cur_col);
	}
	
	bool operator == (const index_row_iterator& X) const {
		return cur_row==X.cur_row && cur_col==X.cur_col;
	}
	bool operator != (const index_row_iterator& X) const {
		return ! operator==(X);
	}
	
	index_row_iterator(matrix_ref<T, matrix_type>&X, unsigned row, unsigned col) : 
			ref(X), cur_row(row), cur_col(col) {}
};

template<typename T, class matrix_type>
class const_index_row_iterator {
	const matrix_ref<T, matrix_type>& ref;
	unsigned cur_row, cur_col;
	
	public:
	
	const_index_row_iterator& operator ++() {
		++cur_col;
		if (cur_col==ref.get_width()) {
			cur_col=0;
			++cur_row;
		}
		return *this;
	}
	
	const T& operator *() {
		return ref(cur_row,cur_col);
	}
	
	bool operator == (const const_index_row_iterator& X) const{
		return cur_row==X.cur_row && cur_col==X.cur_col;
	}
	bool operator != (const const_index_row_iterator& X) const {
		return ! operator==(X);
	}
	
	const_index_row_iterator(const matrix_ref<T, matrix_type>&X, unsigned row, unsigned col) : 
			ref(X), cur_row(row), cur_col(col) {}
};

#endif //_MATRIX_ITERATORS_H_

