#ifndef _MATRIX_H_
#define _MATRIX_H_

#include<vector>
#include<memory>
#include<cassert>

#include"matrix_fwd.h"
#include"iterators.h"
#include "lazy_product.h"
#include "operations.h"

/*
 * This is the class for matrix_ref dynamic.
 */
template<typename T>
class matrix_ref<T, Plain> {
	public:

	typedef T type;
	typedef Plain matrix_type;
	typedef typename std::vector<T>::iterator iterator;
	typedef typename std::vector<T>::const_iterator const_iterator;
	typedef typename std::vector<T>::iterator row_iterator;
	typedef typename std::vector<T>::const_iterator const_row_iterator;

	typedef index_col_iterator<T,Plain> col_iterator;
	typedef const_index_col_iterator<T,Plain> const_col_iterator;


	T& operator ()( unsigned row, unsigned column ) {
		checkIndices(row, column);
		return data->operator[](row*width + column);
	}
	const T& operator ()( unsigned row, unsigned column ) const {
        checkIndices(row, column);
		return data->operator[](row*width + column);
	}

    /*
     * checkIndices checks if indices are valid. In case not it throw a standard exception.
     */
    void checkIndices(unsigned row, unsigned column) const {
        if (row >= height || column >= width)
            throw std::runtime_error(index_err);
    }

	iterator begin() { return data->begin(); }
	iterator end() { return data->end(); }
	const_iterator begin() const { return data->begin(); }
	const_iterator end() const { return data->end(); }

	row_iterator row_begin(unsigned i) {
        if (i >= height)
            throw std::runtime_error(index_err);
        return data->begin() + i*width;
    }

	row_iterator row_end(unsigned i) {
        if (i >= height)
            throw std::runtime_error(index_err);
        return data->begin() + (i+1)*width;
    }

	const_row_iterator row_begin(unsigned i) const {
        if (i >= height)
            throw std::runtime_error(index_err);
        return data->begin() + i*width;
    }

	const_row_iterator row_end(unsigned i) const {
        if (i >= height)
            throw std::runtime_error(index_err);
        return data->begin() + (i+1)*width;
    }

	col_iterator col_begin(unsigned i) {
        if (i >= width)
            throw std::runtime_error(index_err);
        return col_iterator(*this,0,i);
    }

	col_iterator col_end(unsigned i) {
        if (i >= width)
            throw std::runtime_error(index_err);
        return col_iterator(*this,0,i+1);
    }

	const_col_iterator col_begin(unsigned i) const {
        if (i >= width)
            throw std::runtime_error(index_err);
        return const_col_iterator(*this,0,i);
    }

	const_col_iterator col_end(unsigned i) const {
        if (i >= width)
            throw std::runtime_error(index_err);
        return const_col_iterator(*this,0,i+1);
    }

	matrix_ref<T, Transpose<Plain>> transpose() const {
		return matrix_ref<T, Transpose<Plain>>(*this);
	}

	matrix_ref<T, Window<Plain>> window(window_spec spec) const {
        if (spec.row_start >= spec.row_end || spec.col_start >= spec.col_end)
            throw std::runtime_error(empty_err);
        if (spec.row_start < 0 || spec.col_start < 0 || spec.row_end > get_height() || spec.col_end > get_width())
            throw std::runtime_error(index_err);
		return matrix_ref<T, Window<Plain>>(*this, spec);
	}

	matrix_ref<T, Diagonal<Plain>> diagonal() const {
		return matrix_ref<T, Diagonal<Plain>>(*this);
	}

	const matrix_ref<T, Diagonal_matrix<Plain>> diagonal_matrix() const {
		return matrix_ref<T, Diagonal_matrix<Plain>>(*this);
	}

	unsigned get_height() const { return height; }
    unsigned get_width() const { return width; }
    /*
     * Dynamic matrices must contains static methods to access dimensions too.
     * This is due to design constraints.
     */
    static constexpr unsigned get_height_static() { return 0; }
    static constexpr unsigned get_width_static() { return 0; }

    /*
     * This field let the compiler know if the matrix is static or not.
     */
    constexpr static bool staticMatrix = false;

	protected:
	matrix_ref(){}
	std::shared_ptr<std::vector<T>> data;
	unsigned height, width;

};

/*
 * This is the class for a transpose matrix_ref.
 * It can have static or dynamic knowledge of dimensions, depending from the type of decorated.
 */
template<typename T, class decorated>
class matrix_ref<T, Transpose<decorated>> : private matrix_ref<T, decorated> {
	public:

	//type members
	typedef T type;
	typedef Transpose<decorated> matrix_type;
	typedef matrix_ref<T, decorated> base;
	friend class matrix_ref<T, decorated>;
	using typename base::iterator;
	using typename base::const_iterator;
	typedef typename base::row_iterator col_iterator;
	typedef typename base::const_row_iterator const_col_iterator;
	typedef typename base::col_iterator row_iterator;
	typedef typename base::const_col_iterator const_row_iterator;

	T& operator ()( unsigned row, unsigned column ) { return base::operator()(column, row); }
	const T& operator ()( unsigned row, unsigned column ) const { return base::operator()(column, row); }

	template <unsigned row, unsigned column>
	T& get() { return base::template get<column, row>(); };

	template <unsigned row, unsigned column>
    const T& get() const { return base::template get<column, row>(); };

	using base::begin;
	using base::end;

	col_iterator col_begin(unsigned i) { return base::row_begin(i); }
	const_col_iterator col_begin(unsigned i) const { return base::row_begin(i); }
	row_iterator row_begin(unsigned i) { return base::col_begin(i); }
	const_row_iterator row_begin(unsigned i) const { return base::col_begin(i); }

	base transpose() const { return *this; }

	matrix_ref<T, Window<Transpose<decorated>>> window(window_spec spec) const {
        if (spec.row_start >= spec.row_end || spec.col_start >= spec.col_end)
            throw std::runtime_error(empty_err);
        if (spec.row_start < 0 || spec.col_start < 0 || spec.row_end > get_height() || spec.col_end > get_width())
            throw std::runtime_error(index_err);
		return matrix_ref<T, Window<Transpose<decorated>>>(*this, spec);
	}

    template <unsigned r_start, unsigned r_end, unsigned c_start, unsigned c_end>
    typename std::enable_if<matrix_ref<T, Transpose<decorated>>::staticMatrix, matrix_ref<T, Window<Transpose<decorated>, r_start, r_end, c_start, c_end>>>::type
    window() const {
        static_assert(r_start < r_end && c_start < c_end, "You can not construct a matrix without elements");
        static_assert(r_start >= 0 && c_start >= 0 && r_end <= get_height_static() && c_end <= get_width_static(), "You tried to access to invalid location");
        matrix_ref<T, Window<Transpose<decorated>, r_start, r_end, c_start, c_end>> temp(base::data) ;
        return temp;
    }

	matrix_ref<T, Diagonal<Transpose<decorated>>> diagonal() const {
		return matrix_ref<T, Diagonal<Transpose<decorated>>>(*this);
	}

	const matrix_ref<T, Diagonal_matrix<Transpose<decorated>>> diagonal_matrix() const {
		return matrix_ref<T, Diagonal_matrix<Transpose<decorated>>>(*this);
	}

    using base::staticMatrix;

	unsigned get_height() const { return base::get_width(); }
	unsigned get_width() const { return base::get_height(); }
    constexpr static unsigned get_height_static() { return base::get_width_static(); }
    constexpr static unsigned get_width_static() { return base::get_height_static(); }

	private:
	matrix_ref(const base&X) : base(X) {}
};

/*
 * This is the class for a window matrix_ref.
 * Since it does not have static real dimension knowledge it is a dynamic matrix.
 */
template<typename T, class decorated>
class matrix_ref<T, Window<decorated>> : private matrix_ref<T, decorated> {
	public:

	//type members
	typedef T type;
	typedef Window<decorated> matrix_type;
	typedef matrix_ref<T, decorated> base;
	friend class matrix_ref<T, decorated>;

	typedef index_row_iterator<T,Window<decorated>> iterator;
	typedef const_index_row_iterator<T,Window<decorated>> const_iterator;
	typedef index_row_iterator<T,Window<decorated>> row_iterator;
	typedef const_index_row_iterator<T,Window<decorated>> const_row_iterator;
	typedef index_col_iterator<T,Window<decorated>> col_iterator;
	typedef const_index_col_iterator<T,Window<decorated>> const_col_iterator;


	T& operator ()( unsigned row, unsigned column ) {
        checkIndices(row, column);
	    return base::operator()(row+spec.row_start, column+spec.col_start);
	}
	const T& operator ()( unsigned row, unsigned column ) const {
        checkIndices(row, column);
	    return base::operator()(row+spec.row_start, column+spec.col_start);
	}

    /*
     * checkIndices checks if indices are valid. In case not it throw a standard exception.
     */
    void checkIndices(unsigned row, unsigned column) const {
        if (row+spec.row_start >= spec.row_end || column+spec.col_start >= spec.col_end)
            throw std::runtime_error(index_err);
    }

    using base::staticMatrix;

	iterator begin() { return iterator(*this,0,0); }
	iterator end() { return iterator(*this,get_height(),0); }
	const_iterator begin() const { return const_iterator(*this,0,0); }
	const_iterator end() const { return const_iterator(*this,get_height(),0); }

	row_iterator row_begin(unsigned i) { return row_iterator(*this,i,0); }
	row_iterator row_end(unsigned i) { return row_iterator(*this,i+1,0); }
	const_row_iterator row_begin(unsigned i) const { return const_row_iterator(*this,i,0); }
	const_row_iterator row_end(unsigned i) const { return const_row_iterator(*this,i+1,0); }

	col_iterator col_begin(unsigned i) { return col_iterator(*this,0,i); }
	col_iterator col_end(unsigned i) { return col_iterator(*this,0,i+1); }
	const_col_iterator col_begin(unsigned i) const { return const_col_iterator(*this,0,i); }
	const_col_iterator col_end(unsigned i) const { return const_col_iterator(*this,0,i+1); }


	matrix_ref<T, Transpose<Window<decorated>>> transpose() const {
		return matrix_ref<T, Transpose<Window<decorated>>>(*this);
	}

	matrix_ref<T, Window<decorated>> window(window_spec win) const {
        if (spec.row_start >= spec.row_end || spec.col_start >= spec.col_end)
            throw std::runtime_error(empty_err);
        if (spec.row_start < 0 || spec.col_start < 0 || spec.row_end > get_height() || spec.col_end > get_width())
            throw std::runtime_error(index_err);
		return matrix_ref<T, Window<decorated>>(*this, {
			spec.row_start+win.row_start,
			spec.row_start+win.row_end,
			spec.col_start+win.col_start,
			spec.col_start+win.col_end});
	}

	matrix_ref<T, Diagonal<Window<decorated>>> diagonal() const {
		return matrix_ref<T, Diagonal<Window<decorated>>>(*this);
	}

	const matrix_ref<T, Diagonal_matrix<Window<decorated>>> diagonal_matrix() const {
		return matrix_ref<T, Diagonal_matrix<Window<decorated>>>(*this);
	}

	unsigned get_height() const { return spec.row_end-spec.row_start; }
	unsigned get_width() const { return spec.col_end-spec.col_start; }
    static constexpr unsigned get_height_static() { return 0; }
    static constexpr unsigned get_width_static() { return 0; }

	matrix_ref(const base&X, window_spec win) : base(X), spec(win) {
			assert(spec.row_end<=base::get_height());
			assert(spec.col_end<=base::get_width());
	}
private:
	window_spec spec;
};

/*
 * This is the class for a diagonal matrix_ref.
 * It can have static or dynamic knowledge of dimensions, depending from the type of decorated.
 */
template<typename T, class decorated>
class matrix_ref<T, Diagonal<decorated>> : private matrix_ref<T, decorated> {
	public:

	//type members
	typedef T type;
	typedef Diagonal<decorated> matrix_type;
	typedef matrix_ref<T, decorated> base;
	friend class matrix_ref<T, decorated>;

	typedef index_col_iterator<T,Diagonal<decorated>> iterator;
	typedef const_index_col_iterator<T,Diagonal<decorated>> const_iterator;
	typedef index_row_iterator<T,Diagonal<decorated>> row_iterator;
	typedef const_index_row_iterator<T,Diagonal<decorated>> const_row_iterator;
	typedef index_col_iterator<T,Diagonal<decorated>> col_iterator;
	typedef const_index_col_iterator<T,Diagonal<decorated>> const_col_iterator;


	T& operator ()( unsigned row, unsigned column=0 ) {
		assert(column==0);
		return base::operator()(row,row);
	}
	const T& operator ()( unsigned row, unsigned column=0 ) const {
        assert(column == 0);
        return base::operator()(row, row);
    }

    using base::staticMatrix;

	template <unsigned row, unsigned column>
	T& get() { return base::template get<row, row>(); };

	template <unsigned row, unsigned column>
	const T& get() const { return base::template get<row, row>(); };

	iterator begin() { return iterator(*this,0,0); }
	iterator end() { return iterator(*this,0,1); }
	const_iterator begin() const { return const_iterator(*this,0,0); }
	const_iterator end() const { return const_iterator(*this,0,1); }

	row_iterator row_begin(unsigned i) { return row_iterator(*this,i,0); }
	row_iterator row_end(unsigned i) { return row_iterator(*this,i+1,0); }
	const_row_iterator row_begin(unsigned i) const { return const_row_iterator(*this,i,0); }
	const_row_iterator row_end(unsigned i) const { return const_row_iterator(*this,i+1,0); }

	col_iterator col_begin(unsigned i) { return col_iterator(*this,0,i); }
	col_iterator col_end(unsigned i) { return col_iterator(*this,0,i+1); }
	const_col_iterator col_begin(unsigned i) const { return const_col_iterator(*this,0,i); }
	const_col_iterator col_end(unsigned i) const { return const_col_iterator(*this,0,i+1); }


	matrix_ref<T, Transpose<Diagonal<decorated>>> transpose() const {
		return matrix_ref<T, Transpose<Diagonal<decorated>>>(*this);
	}

	matrix_ref<T, Window<Diagonal<decorated>>> window(window_spec spec) const {
        if (spec.row_start >= spec.row_end || spec.col_start >= spec.col_end)
            throw std::runtime_error(empty_err);
        if (spec.row_start < 0 || spec.col_start < 0 || spec.row_end > get_height() || spec.col_end > get_width())
            throw std::runtime_error(index_err);
		return matrix_ref<T, Window<Diagonal<decorated>>>(*this, spec);
	}

    ///da controllare fioi
    template <unsigned r_start, unsigned r_end, unsigned c_start, unsigned c_end>
    typename std::enable_if<matrix_ref<T, Diagonal<decorated>>::staticMatrix, matrix_ref<T, Window<Diagonal<decorated>, r_start, r_end, c_start, c_end>>>::type
    window() const {
        static_assert(r_start < r_end && c_start < c_end, "You can not construct a matrix without elements");
        static_assert(r_start >= 0 && c_start >= 0 && r_end <= get_height_static() && c_end <= get_width_static(), "You tried to access to invalid location");
        matrix_ref<T, Window<Diagonal<decorated>, r_start, r_end, c_start, c_end>> temp(base::data) ;
        return temp;
    }

	matrix_ref<T, Diagonal<Diagonal<decorated>>> diagonal() const {
		return matrix_ref<T, Diagonal<Diagonal<decorated>>>(*this);
	}

	const matrix_ref<T, Diagonal_matrix<Diagonal<decorated>>> diagonal_matrix() const {
		return matrix_ref<T, Diagonal_matrix<Diagonal<decorated>>>(*this);
	}

	unsigned get_height() const { return std::min(base::get_height(), base::get_width()); }
	unsigned get_width() const { return 1; }
    constexpr static unsigned get_height_static() { return std::min(base::get_height_static(), base::get_width_static()); }
    constexpr static unsigned get_width_static() { return 1; }

	private:
	matrix_ref(const base&X) : base(X) {}
};

/*
 * This is the class for a diagonal_matrix matrix_ref.
 * It can have static or dynamic knowledge of dimensions, depending from the type of decorated.
 */
//aggiungere window statica e controlli accessi
template<typename T, class decorated>
class matrix_ref<T, Diagonal_matrix<decorated>> : private matrix_ref<T, decorated> {
	public:

	//type members
	typedef T type;
	typedef Diagonal_matrix<decorated> matrix_type;
	typedef matrix_ref<T, decorated> base;
	friend class matrix_ref<T, decorated>;

	//typedef index_row_iterator<T,Diagonal_matrix<decorated>> iterator;
	typedef const_index_row_iterator<T,Diagonal_matrix<decorated>> const_iterator;
	//typedef index_row_iterator<T,Diagonal_matrix<decorated>> row_iterator;
	typedef const_index_row_iterator<T,Diagonal_matrix<decorated>> const_row_iterator;
	//typedef index_col_iterator<T,Diagonal_matrix<decorated>> col_iterator;
	typedef const_index_col_iterator<T,Diagonal_matrix<decorated>> const_col_iterator;

	const T& operator ()( unsigned row, unsigned column) const {
		if (row!=column) return zero;
		else return base::operator()(row,0);
	}

	template <unsigned row, unsigned column>
	const T& get() const {
		return base::template get<row, 0>();
	};

    using base::staticMatrix;

	//iterator begin() { return iterator(*this,0,0); }
	//iterator end() { return iterator(*this,get_height(),0); }
	const_iterator begin() const { return const_iterator(*this,0,0); }
	const_iterator end() const { return const_iterator(*this,get_height(),0); }

	//row_iterator row_begin(unsigned i) { return row_iterator(*this,i,0); }
	//row_iterator row_end(unsigned i) { return row_iterator(*this,i+1,0); }
	const_row_iterator row_begin(unsigned i) const { return const_row_iterator(*this,i,0); }
	const_row_iterator row_end(unsigned i) const { return const_row_iterator(*this,i+1,0); }

	//col_iterator col_begin(unsigned i) { return col_iterator(*this,0,i); }
	//col_iterator col_end(unsigned i) { return col_iterator(*this,0,i+1); }
	const_col_iterator col_begin(unsigned i) const { return const_col_iterator(*this,0,i); }
	const_col_iterator col_end(unsigned i) const { return const_col_iterator(*this,0,i+1); }


	matrix_ref<T, Transpose<Diagonal_matrix<decorated>>> transpose() const {
		return matrix_ref<T, Transpose<Diagonal_matrix<decorated>>>(*this);
	}

	matrix_ref<T, Window<Diagonal_matrix<decorated>>> window(window_spec spec) const {
        if (spec.row_start >= spec.row_end || spec.col_start >= spec.col_end)
            throw std::runtime_error(empty_err);
        if (spec.row_start < 0 || spec.col_start < 0 || spec.row_end > get_height() || spec.col_end > get_width())
            throw std::runtime_error(index_err);
		return matrix_ref<T, Window<Diagonal_matrix<decorated>>>(*this, spec);
	}

    template <unsigned r_start, unsigned r_end, unsigned c_start, unsigned c_end>
    typename std::enable_if<matrix_ref<T, Diagonal_matrix<decorated>>::staticMatrix, matrix_ref<T, Window<Diagonal_matrix<decorated>, r_start, r_end, c_start, c_end>>>::type
    window() const {
        static_assert(r_start < r_end && c_start < c_end, "You can not construct a matrix without elements");
        static_assert(r_start >= 0 && c_start >= 0 && r_end <= get_height_static() && c_end <= get_width_static(), "You tried to access to invalid location");
        matrix_ref<T, Window<Diagonal_matrix<decorated>, r_start, r_end, c_start, c_end>> temp(base::data) ;
        return temp;
    }

	matrix_ref<T, decorated> diagonal() const {
		return matrix_ref<T, decorated>(*this);
	}

	const matrix_ref<T, Diagonal_matrix<decorated>> diagonal_matrix() const {
		assert(false);
		return *this;
	}

	unsigned get_height() const { return base::get_height(); }
	unsigned get_width() const { return base::get_height(); }
    constexpr static unsigned get_height_static() { return base::get_height_static(); }
    constexpr static unsigned get_width_static() { return base::get_height_static(); }

	private:
	matrix_ref(const base&X) : base(X), zero(0) { assert(base::get_width()==1); }
	const T zero;
};

/*
 * This is the class of a dynamic matrix.
 */
template<typename T> 
class matrix<T> : public matrix_ref<T,Plain> {
	public:

	matrix(unsigned height, unsigned width ) {
		assert(height > 0 && width > 0);
		this->height = height;
		this->width = width;
		data = std::make_shared<std::vector<T>>(width*height);
	}
	
	matrix(const matrix<T> &X) {
		height = X.height;
		width = X.width;
		data = std::make_shared<std::vector<T>>(width*height);
		*data = *(X.data);
	}

    matrix(const matrix_wrap<T> &X) {
        this->height = X.get_height();
        this->width = X.get_width();
        data = std::make_shared<std::vector<T>>(width*height);
        for (int i = 0; i < height; i++)
            for (int j = 0; j < width; j++)
                data->operator[](i*width+j) = X(i, j);
    }

    matrix(const lazy_product<T>& node) {
        std::vector<matrix_wrap<T>> list = node.get_list();
        matrix<T> result = solve(list);
        this->height = result.get_height();
        this->width = result.get_width();
        data = std::make_shared<std::vector<T>>(width*height);
        *data = *(result.data);
    }

	template<class matrix_type>
	matrix(const matrix_ref<T, matrix_type> &X) {
		height = X.get_height();
		width = X.get_width();
		data = std::make_shared<std::vector<T>>(width*height);
        auto dest=data->begin();
        for (int i=0;i<X.get_height();i++)
            for (int j=0;j<X.get_width();j++,++dest)
                *dest = X(i,j);
	}

    using matrix_ref<T,Plain>::staticMatrix;
    using matrix_ref<T,Plain>::data;

private:
	using matrix_ref<T,Plain>::height;
	using matrix_ref<T,Plain>::width;


};

/*
 * This is the class of a static matrix_ref.
 */
template<typename T, unsigned height, unsigned width>
class matrix_ref<T, SizedPlain<height, width>> {
public:

    //type members
    typedef T type;
    typedef SizedPlain<height, width> matrix_type;
    typedef typename std::vector<T>::iterator iterator;
    typedef typename std::vector<T>::const_iterator const_iterator;
    typedef typename std::vector<T>::iterator row_iterator;
    typedef typename std::vector<T>::const_iterator const_row_iterator;

    typedef index_col_iterator<T, SizedPlain<height, width>> col_iterator;
    typedef const_index_col_iterator<T, SizedPlain<height, width>> const_col_iterator;


    T& operator ()( unsigned row, unsigned column ) {
        checkIndices(row, column);
        return data->operator[](row*width + column);
    }

    const T& operator ()( unsigned row, unsigned column ) const {
        checkIndices(row, column);
        return data->operator[](row*width + column);
    }

    /*
     * checkIndices checks if indices are valid. In case not it throw a standard exception.
     */
    void checkIndices(unsigned row, unsigned column) const {
        if (row >= height || column >= width)
            throw std::runtime_error(index_err);
    }

    /*
     * get method can be used to do indices compile time check before returning a value.
     * It can be called by every static matrix_ref.
     */
    template<unsigned row, unsigned column>
    T& get() {
        static_assert(row < height && column < width, "You tried to access to an invalid location");
        return data->operator[](row*width + column);
    }

    template<unsigned row, unsigned column>
    const T& get() const {
        static_assert(row < height && column < width, "You tried to access to an invalid location");
        return data->operator[](row*width + column);
    }

    iterator begin() { return data->begin(); }
    iterator end() { return data->end(); }
    const_iterator begin() const { return data->begin(); }
    const_iterator end() const { return data->end(); }

    row_iterator row_begin(unsigned i) {
        if (i >= height)
            throw std::runtime_error(index_err);
        return data->begin() + i*width;
    }
    row_iterator row_end(unsigned i) {
        if (i >= height)
            throw std::runtime_error(index_err);
        return data->begin() + (i+1)*width;
    }
    const_row_iterator row_begin(unsigned i) const {
        if (i >= height) {
                std::cout << height << "h : i -> " << i << "iii \n";
                throw std::runtime_error(index_err);
        }
        return data->begin() + i*width;
    }
    const_row_iterator row_end(unsigned i) const {
        if (i >= height)
            throw std::runtime_error(index_err);
        return data->begin() + (i+1)*width;
    }

    col_iterator col_begin(unsigned i) {
        if (i >= width)
            throw std::runtime_error(index_err);
        return col_iterator(*this,0,i);
    }
    col_iterator col_end(unsigned i) {
        if (i >= width)
            throw std::runtime_error(index_err);
        return col_iterator(*this,0,i+1);
    }
    const_col_iterator col_begin(unsigned i) const {
        if (i >= width)
            throw std::runtime_error(index_err);
        return const_col_iterator(*this,0,i);
    }
    const_col_iterator col_end(unsigned i) const {
        if (i >= width)
            throw std::runtime_error(index_err);
        return const_col_iterator(*this,0,i+1);
        if (i >= width)
            throw std::runtime_error(index_err);
    }

    matrix_ref<T, Transpose<SizedPlain<height, width>>> transpose() const {
        return matrix_ref<T, Transpose<SizedPlain<height, width>>>(*this);
    }

    /*
     * If we call window dynamically it returns a dynamic matrix.
     */
    matrix_ref<T, Window<Plain>> window(window_spec spec) const {
        if (spec.row_start >= spec.row_end || spec.col_start >= spec.col_end)
            throw std::runtime_error(empty_err);
        if (spec.row_start < 0 || spec.col_start < 0 || spec.row_end > get_height() || spec.col_end > get_width())
            throw std::runtime_error(index_err);
        matrix_ref<T, Plain> temp(height, width, data);
        return matrix_ref<T, Window<Plain>>(temp, spec);
    }
    /*
     * If we call window statically it returns a static matrix.
     */
    constexpr static bool staticMatrix = true;

    template <unsigned r_start, unsigned r_end, unsigned c_start, unsigned c_end>
    typename std::enable_if<matrix_ref<T, SizedPlain<height, width>>::staticMatrix, matrix_ref<T, Window<SizedPlain<height, width>, r_start, r_end, c_start, c_end>>>::type
    window() const {
        static_assert(r_start < r_end && c_start < c_end, "You can not construct a matrix without elements");
        static_assert(r_start >= 0 && c_start >= 0 && r_end <= get_height_static() && c_end <= get_width_static(), "You tried to access to invalid location");
        matrix_ref<T, Window<SizedPlain<height, width>, r_start, r_end, c_start, c_end>> temp(data) ;
        return temp;
    }

    matrix_ref<T, Diagonal<SizedPlain<height, width>>> diagonal() const {
        return matrix_ref<T, Diagonal<SizedPlain<height, width>>>(*this);
    }

    const matrix_ref<T, Diagonal_matrix<SizedPlain<height, width>>> diagonal_matrix() const {
        return matrix_ref<T, Diagonal_matrix<SizedPlain<height, width>>>(*this);
    }

    unsigned get_height() const { return height; }
    unsigned get_width() const { return width; }
    static constexpr unsigned get_height_static() { return height; }
    static constexpr unsigned get_width_static() { return width; }

    matrix_ref(std::shared_ptr<std::vector<T>> data) {
        this->data = data;
    }

protected:
    matrix_ref(){}

    std::shared_ptr<std::vector<T>> data;
};

template<typename T, unsigned height, unsigned width>
class matrix<T, height, width> : public matrix_ref<T,SizedPlain<height, width>> {
public:

    matrix() {
		static_assert(height > 0 && width > 0, "You can not define a matrix with row or column equals to zero");
        data = std::make_shared<std::vector<T>>(width*height);
    }

    matrix(const lazy_product<T>& node) {
        std::vector<matrix_wrap<T>> list = node.get_list();
        matrix<T> result = solve(list);
        if (result.get_height() == height && result.get_width() == width) {
            data = std::make_shared<std::vector<T>>(width * height);
            *data = *(result.data);
        }
        else
            throw std::runtime_error(dim_err);
    }

    template<class matrix_type>
    matrix(const matrix_ref<T, matrix_type> &X) {
        static_assert(!matrix_ref<T, matrix_type>::staticMatrix || (matrix_ref<T, matrix_type>::get_height_static() == height && matrix_ref<T, matrix_type>::get_width_static() == width), "Matrices must have same dimensions");
        if (height!=X.get_height() || width!=X.get_width())
            throw std::runtime_error("Matrices must have same dimensions");
        data = std::make_shared<std::vector<T>>(width*height);
        for (int i = 0; i < height; i++)
            for (int j = 0; j < width; j++)
            data->operator[](i*width+j) = X(i, j);
    }
    using matrix_ref<T,SizedPlain<height, width>>::staticMatrix;
private:
    using matrix_ref<T,SizedPlain<height, width>>::data;
};

template<typename T, unsigned r_start, unsigned r_end, unsigned c_start, unsigned c_end, class decorated>
class matrix_ref<T, Window<decorated, r_start, r_end, c_start, c_end>> : private matrix_ref<T, decorated> {
public:

    typedef T type;
    typedef Window<decorated> matrix_type;
    typedef matrix_ref<T, decorated> base;
    friend class matrix_ref<T, decorated>;

    typedef index_row_iterator<T,Window<decorated>> iterator;
    typedef const_index_row_iterator<T,Window<decorated>> const_iterator;
    typedef index_row_iterator<T,Window<decorated>> row_iterator;
    typedef const_index_row_iterator<T,Window<decorated>> const_row_iterator;
    typedef index_col_iterator<T,Window<decorated>> col_iterator;
    typedef const_index_col_iterator<T,Window<decorated>> const_col_iterator;


    T& operator ()( unsigned row, unsigned column ) {
        checkIndices(row, column);
        return base::operator()(row+r_start, column+c_start);
    }
    const T& operator ()( unsigned row, unsigned column ) const {
        checkIndices(row, column);
        return base::operator()(row+r_start, column+c_start);
    }

    template <unsigned row, unsigned column>
    T& get() { return base::template get<row+r_start, column+c_start>(); }
    template <unsigned row, unsigned column>
    const T& get() const { return base::template get<row+r_start, column+c_start>(); }

    /*
     * checkIndices checks if indices are valid. In case not it throw a standard exception.
     */
    void checkIndices(unsigned row, unsigned column) const {
        if (row+r_start >= r_end || column+c_start >= c_end)
            throw std::runtime_error(index_err);
    }

    using base::staticMatrix;

    iterator begin() { return iterator(*this,0,0); }
    iterator end() { return iterator(*this,get_height(),0); }
    const_iterator begin() const { return const_iterator(*this,0,0); }
    const_iterator end() const { return const_iterator(*this,get_height(),0); }

    row_iterator row_begin(unsigned i) { return row_iterator(*this,i,0); }
    row_iterator row_end(unsigned i) { return row_iterator(*this,i+1,0); }
    const_row_iterator row_begin(unsigned i) const { return const_row_iterator(*this,i,0); }
    const_row_iterator row_end(unsigned i) const { return const_row_iterator(*this,i+1,0); }

    col_iterator col_begin(unsigned i) { return col_iterator(*this,0,i); }
    col_iterator col_end(unsigned i) { return col_iterator(*this,0,i+1); }
    const_col_iterator col_begin(unsigned i) const { return const_col_iterator(*this,0,i); }
    const_col_iterator col_end(unsigned i) const { return const_col_iterator(*this,0,i+1); }


    matrix_ref<T, Transpose<Window<decorated>>> transpose() const {
        return matrix_ref<T, Transpose<Window<decorated>>>(*this);
    }

    matrix_ref<T, Window<decorated>> window(window_spec win) const {
        if (win.row_start >= win.row_end || win.col_start >= win.col_end)
            throw std::runtime_error(empty_err);
        if (win.row_start < 0 || win.col_start < 0 || win.row_end > get_height() || win.col_end > get_width())
            throw std::runtime_error(index_err);
        return matrix_ref<T, Window<decorated>>(*this, {
                r_start+win.row_start,
                r_start+win.row_end,
                c_start+win.col_start,
                c_start+win.col_end});
    }

    template <unsigned row_start, unsigned row_end, unsigned col_start, unsigned col_end>
    typename std::enable_if<matrix_ref<T, Window<decorated, r_start, r_end, c_start, c_end>>::staticMatrix, matrix_ref<T, Window<matrix_ref<T, Window<decorated, r_start, r_end, c_start, c_end>>, row_start+r_start, r_end+r_start, c_start+c_start, c_end+c_start>>>::type
    window() const {
        static_assert(r_start < r_end && c_start < c_end, "You can not construct a matrix without elements");
        static_assert(r_start >= 0 && c_start >= 0 && r_end <= get_height_static() && c_end <= get_width_static(), "You tried to access to invalid location");
        matrix_ref<T, Window<matrix_ref<T, Window<decorated, r_start, r_end, c_start, c_end>>, row_start+r_start, r_end+r_start, c_start+c_start, c_end+c_start>> temp(base::data) ;
        return temp;
    }

    matrix_ref<T, Diagonal<Window<decorated>>> diagonal() const {
        return matrix_ref<T, Diagonal<Window<decorated>>>(*this);
    }

    const matrix_ref<T, Diagonal_matrix<Window<decorated>>> diagonal_matrix() const {
        return matrix_ref<T, Diagonal_matrix<Window<decorated>>>(*this);
    }

    unsigned get_height() const { return r_end - r_start; }
    unsigned get_width() const { return c_end - c_start; }
    constexpr static unsigned get_height_static() { return r_end - r_start; }
    constexpr static unsigned get_width_static() { return c_end - c_start; }


    matrix_ref<T, Window<decorated, r_start, r_end, c_start, c_end>> () { };

    matrix_ref(const base&X) : base(X) { }
};

#endif //_MATRIX_H_