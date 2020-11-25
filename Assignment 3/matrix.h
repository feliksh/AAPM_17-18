#ifndef _MATRIX_H_
#define _MATRIX_H_

#include<vector>
#include<memory>
#include<cassert>

#include"matrix_fwd.h"
#include"iterators.h"




template<typename T> 
class matrix_ref<T, Plain> {
	public:
	
	//type members
	typedef T type;
	typedef Plain matrix_type;
	typedef typename std::vector<T>::iterator iterator;
	typedef typename std::vector<T>::const_iterator const_iterator;
	typedef typename std::vector<T>::iterator row_iterator;
	typedef typename std::vector<T>::const_iterator const_row_iterator;
	
	typedef index_col_iterator<T,Plain> col_iterator;
	typedef const_index_col_iterator<T,Plain> const_col_iterator;
	
	
	static constexpr unsigned H=0;
	static constexpr unsigned W=0;
	
	//matrix_ref<T, Plain>& operator =(const matrix_ref<T, Plain>&) = delete;
	//matrix_ref<T, Plain>& operator =(matrix_ref<T, Plain>&&) = delete;
	
	
	T& operator ()( unsigned row, unsigned column ) { 
		return data->operator[](row*width + column);
	}
	const T& operator ()( unsigned row, unsigned column ) const { 
		return data->operator[](row*width + column);
	}
	std::vector<T> get_sub(unsigned from_r, unsigned to_r, unsigned from_c, unsigned to_c){
        assert(from_r<to_r && from_c<to_c);
        std::vector<T> subdata((to_r-from_r)*(to_c-from_c));
        int k=0;
        for(int i=from_r; i!=to_r; ++i)
            for(int j=from_c; j!=to_c; ++j){
                subdata[k] = data->operator[](i*width + j);
                ++k;
            }
        return subdata;
    }
	
	template<unsigned i, unsigned j>
	T& get() { return operator()(i,j); }
	template<unsigned i, unsigned j>
	const T& get() const { return operator()(i,j); }
	
	
	iterator begin() { return data->begin(); }
	iterator end() { return data->end(); }
	const_iterator begin() const { return data->begin(); }
	const_iterator end() const { return data->end(); }
	
	row_iterator row_begin(unsigned i) { return data->begin() + i*width; }
	row_iterator row_end(unsigned i) { return data->begin() + (i+1)*width; }
	const_row_iterator row_begin(unsigned i) const { return data->begin() + i*width; }
	const_row_iterator row_end(unsigned i) const { return data->begin() + (i+1)*width; }
	
	col_iterator col_begin(unsigned i) { return col_iterator(*this,0,i); }
	col_iterator col_end(unsigned i) { return col_iterator(*this,0,i+1); }
	const_col_iterator col_begin(unsigned i) const { return const_col_iterator(*this,0,i); }
	const_col_iterator col_end(unsigned i) const { return const_col_iterator(*this,0,i+1); }
	
	
	matrix_ref<T, Transpose<Plain>> transpose() const { 
		return matrix_ref<T, Transpose<Plain>>(*this);
	}
	
	matrix_ref<T, Window<Plain>> window(window_spec spec) const {
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
	
	
	protected:
	matrix_ref(){}
		
	std::shared_ptr<std::vector<T>> data;
	unsigned height, width;

};




template<typename T, unsigned h, unsigned w> 
class matrix_ref<T, Sized<h,w>> {
	public:
	
	//type members
	typedef T type;
	typedef Sized<h,w> matrix_type;
	typedef typename std::vector<T>::iterator iterator;
	typedef typename std::vector<T>::const_iterator const_iterator;
	typedef typename std::vector<T>::iterator row_iterator;
	typedef typename std::vector<T>::const_iterator const_row_iterator;
	
	typedef index_col_iterator<T,Sized<h,w>> col_iterator;
	typedef const_index_col_iterator<T,Sized<h,w>> const_col_iterator;
	
	
	static constexpr unsigned H=h;
	static constexpr unsigned W=w;
	
	//matrix_ref<T, Plain>& operator =(const matrix_ref<T, Plain>&) = delete;
	//matrix_ref<T, Plain>& operator =(matrix_ref<T, Plain>&&) = delete;
	
	
	T& operator ()( unsigned row, unsigned column ) { 
		return data->operator[](row*width + column);
	}
	const T& operator ()( unsigned row, unsigned column ) const { 
		return data->operator[](row*width + column);
	}
    std::vector<T> get_sub(unsigned from_r, unsigned to_r, unsigned from_c, unsigned to_c){
        assert(from_r<to_r && from_c<to_c);
        std::vector<T> subdata((to_r-from_r)*(to_c-from_c));
        unsigned k=0;
        for(unsigned i=from_r; i!=to_r; ++i)
            for(unsigned j=from_c; j!=to_c; ++j){
                subdata[k] = data->operator[](i*width + j);
                ++k;
            }
        return subdata;
    }
	
	template<unsigned i, unsigned j>
	T& get() { 
		static_assert(i<h && j<w, "dimension mismatch");
		return operator()(i,j); 
	}
	template<unsigned i, unsigned j>
	const T& get() const { 
		static_assert(i<h && j<w, "dimension mismatch");
		return operator()(i,j); 
	}
	
	
	iterator begin() { return data->begin(); }
	iterator end() { return data->end(); }
	const_iterator begin() const { return data->begin(); }
	const_iterator end() const { return data->end(); }
	
	row_iterator row_begin(unsigned i) { return data->begin() + i*width; }
	row_iterator row_end(unsigned i) { return data->begin() + (i+1)*width; }
	const_row_iterator row_begin(unsigned i) const { return data->begin() + i*width; }
	const_row_iterator row_end(unsigned i) const { return data->begin() + (i+1)*width; }
	
	col_iterator col_begin(unsigned i) { return col_iterator(*this,0,i); }
	col_iterator col_end(unsigned i) { return col_iterator(*this,0,i+1); }
	const_col_iterator col_begin(unsigned i) const { return const_col_iterator(*this,0,i); }
	const_col_iterator col_end(unsigned i) const { return const_col_iterator(*this,0,i+1); }
	
	
	matrix_ref<T, Transpose<Sized<h,w>>> transpose() const { 
		return matrix_ref<T, Transpose<Sized<h,w>>>(*this);
	}
	
	matrix_ref<T, Window<Sized<h,w>>> window(window_spec spec) const {
		return matrix_ref<T, Window<Sized<h,w>>>(*this, spec);
	}
	
	matrix_ref<T, Diagonal<Sized<h,w>>> diagonal() const {
		return matrix_ref<T, Diagonal<Sized<h,w>>>(*this);
	}
	
	const matrix_ref<T, Diagonal_matrix<Sized<h,w>>> diagonal_matrix() const {
		return matrix_ref<T, Diagonal_matrix<Sized<h,w>>>(*this);
	}
	
	unsigned get_height() const { return height; }
	unsigned get_width() const { return width; }
	
	
	protected:
	matrix_ref(){}
		
	std::shared_ptr<std::vector<T>> data;
	unsigned height, width;

};









template<typename T, class decorated> 
class matrix_ref<T, Transpose<decorated>> : private matrix_ref<T, decorated> {
	public:
	
	//type members
	typedef T type;
	typedef Transpose<decorated> matrix_type;
	typedef matrix_ref<T, decorated> base;
	friend class matrix_ref<T, decorated>;
	using base::iterator;
	using base::const_iterator;
	typedef typename base::row_iterator col_iterator;
	typedef typename base::const_row_iterator const_col_iterator;
	typedef typename base::col_iterator row_iterator;
	typedef typename base::const_col_iterator const_row_iterator;
	
	static constexpr unsigned H=base::W;
	static constexpr unsigned W=base::H;
	
	
	T& operator ()( unsigned row, unsigned column ) 
	{ return base::operator()(column, row); }
	const T& operator ()( unsigned row, unsigned column ) const
	{ return base::operator()(column, row); }

    std::vector<T> get_sub(unsigned from_r, unsigned to_r, unsigned from_c, unsigned to_c) {
        assert(from_r<to_r && from_c<to_c);
        std::vector<T> subdata((to_r-from_r)*(to_c-from_c));
        std::vector<T> basedata = base::get_sub(from_c, to_c, from_r, to_r);
        const unsigned width = to_r-from_r;
        int k=0;
        for(int j=0; j!=to_r-from_r; ++j)
            for(int i=0; i!=to_c-from_c; ++i) {
                subdata[k] = basedata[i*width + j];
                ++k;
            }
        return subdata;
    }

	template<unsigned i, unsigned j>
	T& get() { 
		static_assert(H!=0 && i<H && j<W, "dimension mismatch");
		return base::template get<j,i>(); 
		}
	template<unsigned i, unsigned j>
	const T& get() const { 
		static_assert(H!=0 && i<H && j<W, "dimension mismatch");
		return base::template get<j,i>(); 
		}

	
	using base::begin;
	using base::end;
	col_iterator col_begin(unsigned i) { return base::row_begin(i); }
	const_col_iterator col_begin(unsigned i) const { return base::row_begin(i); }
	row_iterator row_begin(unsigned i) { return base::col_begin(i); }
	const_row_iterator row_begin(unsigned i) const { return base::col_begin(i); }
	
	base transpose() const { return *this; }
	
	matrix_ref<T, Window<Transpose<decorated>>> window(window_spec spec) const {
		return matrix_ref<T, Window<Transpose<decorated>>>(*this, spec);
	}
	
	matrix_ref<T, Diagonal<Transpose<decorated>>> diagonal() const {
		return matrix_ref<T, Diagonal<Transpose<decorated>>>(*this);
	}
	
	const matrix_ref<T, Diagonal_matrix<Transpose<decorated>>> diagonal_matrix() const {
		return matrix_ref<T, Diagonal_matrix<Transpose<decorated>>>(*this);
	}
	
	unsigned get_height() const { return base::get_width(); }
	unsigned get_width() const { return base::get_height(); }
		
	private:
	matrix_ref(const base&X) : base(X) {}
};



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
	
	static constexpr unsigned H=0;
	static constexpr unsigned W=0;
	
	T& operator ()( unsigned row, unsigned column ) 
	{ return base::operator()(row+spec.row_start, column+spec.col_start); }
	const T& operator ()( unsigned row, unsigned column ) const
	{ return base::operator()(row+spec.row_start, column+spec.col_start); }
    std::vector<T> get_sub(unsigned from_r, unsigned to_r, unsigned from_c, unsigned to_c) {
        assert(from_r<to_r && from_c<to_c);
        assert(to_r+spec.row_start <= spec.row_end &&
               to_c+spec.col_start <= spec.col_end);
        return base::get_sub(from_r+spec.row_start, to_r+spec.row_start,
                           from_c+spec.col_start, to_c+spec.col_start);
    }
	
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
	
	
	
		
	private:
	matrix_ref(const base&X, window_spec win) : base(X), spec(win) {
			assert(spec.row_end<=base::get_height());
			assert(spec.col_end<=base::get_width());
	}
	
	window_spec spec;
};


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
	
	
	static constexpr unsigned H=base::H;
	static constexpr unsigned W=(base::W==0)?0:1;
	
	T& operator ()( unsigned row, unsigned column=0 ) {
		assert(column==0);
		return base::operator()(row,row);
	}
	const T& operator ()( unsigned row, unsigned column=0 ) const {
		assert(column==0);
		return base::operator()(row,row);
	}
    std::vector<T> get_sub(unsigned from_r, unsigned to_r, unsigned from_c=0, unsigned to_c=1) {
        assert(from_r<to_r && from_c==0 && to_c==1);
        std::vector<T> subdata((to_r-from_r)*(to_c-from_c));
        unsigned k=0;
        for(unsigned i=from_r; i!=to_r; ++i) {
            subdata[k] = base::operator()(i, i);
            ++k;
        }
        return subdata;
    }
	
	template<unsigned i, unsigned j=0>
	T& get() { 
		static_assert(H!=0 && i<H && j<W, "dimension mismatch");
		return base::template get<i,j>(); 
	}
	template<unsigned i, unsigned j=0>
	const T& get() const { 
		static_assert(H!=0 && i<H && j<W, "dimension mismatch");
		return base::template get<i,j>(); 
		}
	
	
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
	
	matrix_ref<T, Window<Diagonal<decorated>>> window(window_spec win) const {
		return matrix_ref<T, Window<Diagonal<decorated>>>(*this, win);
	}
	
	matrix_ref<T, Diagonal<Diagonal<decorated>>> diagonal() const {
		return matrix_ref<T, Diagonal<Diagonal<decorated>>>(*this);
	}
	
	const matrix_ref<T, Diagonal_matrix<Diagonal<decorated>>> diagonal_matrix() const {
		return matrix_ref<T, Diagonal_matrix<Diagonal<decorated>>>(*this);
	}
	
	unsigned get_height() const { 
		return std::min(base::get_height(), base::get_width()); 
		}
	unsigned get_width() const { return 1; }
		
	private:
	matrix_ref(const base&X) : base(X) {}
};



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
	
	static constexpr unsigned H=base::H;
	static constexpr unsigned W=base::H;
	
	
	//Diagonal_matrix is always const!
	/*
	T& operator ()( unsigned row, unsigned column ) 
	{
		if (row!=column) return zero;
		else return base::operator()(row,row);
	}
	*/
	const T& operator ()( unsigned row, unsigned column) const
	{
		if (row!=column) return zero;
		else return base::operator()(row,0);
	}
    std::vector<T> get_sub(unsigned from_r, unsigned to_r, unsigned from_c, unsigned to_c){
        assert(from_r<to_r && from_c<to_c);
        std::vector<T> subdata((to_r-from_r)*(to_c-from_c));
        int k=0;
        for(int i=from_r; i!=to_r; ++i)
            for(int j=from_c; j!=to_c; ++j){
                if(i!=j) subdata[k] = zero;
                else subdata[k] = base::operator()(i,0);
                ++k;
            }
        return subdata;
    }
	/*
	template<unsigned i, unsigned j=0>
	typename std::enable_if<H!=0,T&>::type get() { 
		static_asert(i<H && j<W, "dimension mismatch");
		return base::template get<i,j>(); 
	}
	*/
	template<unsigned i, unsigned j=0>
	const T& get() const { 
		static_assert(H!=0 && i<H && j<W, "dimension mismatch");
		return (i!=j)?zero:(base::template get<i,0>()); 
		}
	
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
	
	matrix_ref<T, Window<Diagonal_matrix<decorated>>> window(window_spec win) const {
		return matrix_ref<T, Window<Diagonal_matrix<decorated>>>(*this, win);
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
		
	private:
	matrix_ref(const base&X) : base(X), zero(0) { assert(base::get_width()==1); }
	const T zero;
};



template<typename T, unsigned...sizes> class matrix;



template<typename T> 
class matrix<T> : public matrix_ref<T,Plain> {
	public:
	
	matrix( unsigned height, unsigned width ) {
		this->height = height;
		this->width = width;
		data = std::make_shared<std::vector<T>>(width*height);
		
		std::cerr << "matrix constructor\n";
	}
	
	matrix(const matrix<T>& X) {
		height = X.height;
		width = X.width;
		data = std::make_shared<std::vector<T>>(width*height);
		*data = *(X.data);
		
		std::cerr << "matrix copy constructor\n";
	}
	
	matrix(matrix<T>&& X) {
		height = X.height;
		width = X.width;
		data = std::move(X.data);
		
		std::cerr << "matrix move constructor\n";
	}
	
	template<class matrix_type>
	matrix(const matrix_ref<T,matrix_type>&X) {
		height = X.get_height();
		width = X.get_width();
		data = std::make_shared<std::vector<T>>(width*height);
		auto source=X.row_begin(0);
		const auto end=X.row_begin(height);
		auto dest=data->begin();
		while (source!=end) {
			*dest = *source;
			++dest;
			++source;
		}
		
		std::cerr << "matrix foreign constructor\n";
	}
	
	using matrix_ref<T,Plain>::H;
	using matrix_ref<T,Plain>::W;

	private:
	using matrix_ref<T,Plain>::height;
	using matrix_ref<T,Plain>::width; 
	using matrix_ref<T,Plain>::data; 

};





template<typename T, unsigned h, unsigned w> 
class matrix<T,h,w> : public matrix_ref<T,Sized<h,w>> {
	public:
	
	static_assert(h!=0 && w!=0, "matrix static sizes cannot be 0");
	
	matrix() {
		this->height = h;
		this->width = w;
		data = std::make_shared<std::vector<T>>(w*h);
		
		std::cerr << "sized matrix constructor\n";
	}
	
	matrix(const matrix<T,h,w>& X) {
		height = X.height;
		width = X.width;
		data = std::make_shared<std::vector<T>>(width*height);
		*data = *(X.data);
		
		std::cerr << "sized matrix copy constructor\n";
	}
	
		matrix(matrix<T,h,w>&& X) {
		height = X.height;
		width = X.width;
		data = std::move(X.data);
		
		std::cerr << "sized matrix move constructor\n";
	}
	

	
	template<class matrix_type>
	matrix(const matrix_ref<T,matrix_type>&X) {
		static_assert((matrix_ref<T,matrix_type>::H==0 || matrix_ref<T,matrix_type>::H==h) && (
				matrix_ref<T,matrix_type>::W==0 || matrix_ref<T,matrix_type>::W==w),
				"uncompatible sizes in sized matrix construction");
		
		height = X.get_height();
		width = X.get_width();
		assert(height==h && width==w);
		data = std::make_shared<std::vector<T>>(width*height);
		auto source=X.row_begin(0);
		const auto end=X.row_begin(height);
		auto dest=data->begin();
		while (source!=end) {
			*dest = *source;
			++dest;
			++source;
		}
		
		std::cerr << "sized matrix foreign constructor\n";
	}	

	using matrix_ref<T,Sized<h,w>>::H;
	using matrix_ref<T,Sized<h,w>>::W;
	
	
	private:
	using matrix_ref<T,Sized<h,w>>::height;
	using matrix_ref<T,Sized<h,w>>::width; 
	using matrix_ref<T,Sized<h,w>>::data; 

};

#endif //_MATRIX_H_
