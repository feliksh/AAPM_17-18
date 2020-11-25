#ifndef _MATRIX_WRAP_H_
#define _MATRIX_WRAP_H_

#include"matrix.h"

template<typename T>
struct iterator_impl {
	virtual void increment() = 0;
	virtual T& dereference() = 0;
	virtual bool is_equal(const iterator_impl<T>*) const = 0;
	
	virtual std::unique_ptr<iterator_impl<T>> clone() const = 0;
	virtual ~iterator_impl() {}
};

template<typename T>
struct const_iterator_impl {
	virtual void increment() = 0;
	virtual const T& dereference() = 0;
	virtual bool is_equal(const const_iterator_impl<T>*) const = 0;
	
	virtual std::unique_ptr<const_iterator_impl<T>> clone() const = 0;
	virtual ~const_iterator_impl() {}
};


template<typename T, typename iterator_type>
class concrete_iterator_impl : public iterator_impl<T> {
	public:
	concrete_iterator_impl(iterator_type iter) : iterator(iter) {}
	
	void increment() override { ++iterator; }
	T& dereference() override { return *iterator; }
	bool is_equal(const iterator_impl<T>* X) const override {
		const concrete_iterator_impl<T,iterator_type>* Xp = 
			dynamic_cast<const concrete_iterator_impl<T,iterator_type>*>(X);
		if (!Xp) return false;
		else return iterator == Xp->iterator;
	}
	
	std::unique_ptr<iterator_impl<T>> clone() const override {
		return std::make_unique<concrete_iterator_impl<T,iterator_type>>(iterator);
	}
	
	private:
	iterator_type iterator;
};


template<typename T, typename iterator_type>
class concrete_nonconst_iterator_impl : public iterator_impl<T> {
	public:
	concrete_nonconst_iterator_impl(iterator_type iter) : iterator(iter) {}
	
	void increment() override { ++iterator; }
	T& dereference() override { 
		static T result;
		return (result = *iterator); 
	}
	bool is_equal(const iterator_impl<T>* X) const override {
		const concrete_nonconst_iterator_impl<T,iterator_type>* Xp = 
			dynamic_cast<const concrete_nonconst_iterator_impl<T,iterator_type>*>(X);
		if (!Xp) return false;
		else return iterator == Xp->iterator;
	}
	
	std::unique_ptr<iterator_impl<T>> clone() const override {
		return std::make_unique<concrete_nonconst_iterator_impl<T,iterator_type>>(iterator);
	}
	
	private:
	iterator_type iterator;
};



	
template<typename T, typename const_iterator_type>
class concrete_const_iterator_impl : public const_iterator_impl<T> {
	public:
	concrete_const_iterator_impl(const_iterator_type iter) : iterator(iter) {}
	
	void increment() override { ++iterator; }
	virtual const T& dereference() override { return *iterator; }
	virtual bool is_equal(const const_iterator_impl<T>* X) const override {
		const concrete_const_iterator_impl<T,const_iterator_type>* Xp = 
			dynamic_cast<const concrete_const_iterator_impl<T,const_iterator_type>*>(X);
		if (!Xp) return false;
		else return iterator == Xp->iterator;
	}
	
	std::unique_ptr<const_iterator_impl<T>> clone() const override {
		return std::make_unique<concrete_const_iterator_impl<T,const_iterator_type>>(iterator);
	}
	
	private:
	const_iterator_type iterator;
};


template<typename T>
class iterator_wrap {
	public:
	iterator_wrap& operator ++() {
		pimpl->increment();
		return *this;
	}
	T& operator *() { return pimpl->dereference(); }
	
	bool operator == (const iterator_wrap<T>& X) {
		return pimpl->is_equal(X.pimpl.get());
	}
	bool operator != (const iterator_wrap<T>& X) {
		return !pimpl->is_equal(X.pimpl.get());
	}
	
	iterator_wrap(const iterator_wrap& X) : pimpl(X.pimpl->clone()) {}
	
	iterator_wrap(std::unique_ptr<iterator_impl<T>> impl) : pimpl(impl) {}
	
	// probably never necessary to construct a wrap from actual iterators
	//template<typename iterator_type>
	//iterator_wrap(const iterator_type& iter) : 
	//	pimpl(std::make_unique<concrete_iterator_impl<T,iterator_type>>(iter){}
		
	private:
	std::unique_ptr<iterator_impl<T>> pimpl;
};


template<typename T>
class const_iterator_wrap {
	public:
	const_iterator_wrap& operator ++() {
		pimpl->increment();
		return *this;
	}
	T& operator *() { return pimpl->dereference(); }
	
	bool operator == (const const_iterator_wrap<T>& X) {
		return pimpl->is_equal(X.pimpl.get());
	}
	bool operator != (const const_iterator_wrap<T>& X) {
		return !pimpl->is_equal(X.pimpl.get());
	}
	
	const_iterator_wrap(const const_iterator_wrap& X) : pimpl(X.pimpl->clone()) {}
	
	const_iterator_wrap(std::unique_ptr<const_iterator_impl<T>> impl) : pimpl(impl) {}
	
	// probably never necessary to construct a wrap from actual iterators
	//template<typename const_iterator_type>
	//const_iterator_wrap(const const_iterator_type& iter) : 
	//	pimpl(std::make_unique<concrete_const_iterator_impl<T,const_iterator_type>>(iter){}
		
	private:
	std::unique_ptr<iterator_impl<T>> pimpl;
};


	
	

template<typename T>
struct matrix_wrap_impl {
	virtual T& get(unsigned i, unsigned j) = 0;
	virtual const T& get(unsigned i, unsigned j) const = 0;
    virtual std::vector<T> get_sub(unsigned from_r, unsigned to_r, unsigned from_c, unsigned to_c) = 0;
	
	virtual std::unique_ptr<matrix_wrap_impl<T>> clone() const = 0;
	virtual ~matrix_wrap_impl() {}
	
	virtual std::unique_ptr<matrix_wrap_impl<T>> transpose() const = 0;
	
	//will not work: cyclic type expansion!
	//virtual std::unique_ptr<matrix_wrap_impl<T>> window(window_spec) const = 0;
	//virtual std::unique_ptr<matrix_wrap_impl<T>> diagonal() const = 0;
	//virtual std::unique_ptr<matrix_wrap_impl<T>> diagonal_matrix() const = 0;
	
	virtual std::unique_ptr<iterator_impl<T>> begin() = 0; 
	virtual std::unique_ptr<iterator_impl<T>> end() = 0;
	virtual std::unique_ptr<const_iterator_impl<T>> begin() const = 0; 
	virtual std::unique_ptr<const_iterator_impl<T>> end() const = 0;
	
	virtual unsigned get_height() const = 0;
	virtual unsigned get_width() const = 0;
};




template<typename T, class matrix_type>
class concrete_matrix_wrap_impl : public matrix_wrap_impl<T> {
	public:
	T& get(unsigned i, unsigned j) override { return mat(i,j); }
	const T& get(unsigned i, unsigned j) const override { return mat(i,j); }
    std::vector<T> get_sub(unsigned from_r, unsigned to_r, unsigned from_c, unsigned to_c)
    override { return mat.get_sub(from_r, to_r, from_c, to_c); }
	
	std::unique_ptr<matrix_wrap_impl<T>> clone() const override {
		return std::make_unique<concrete_matrix_wrap_impl<T,matrix_type>>(mat);
	}
	
	std::unique_ptr<matrix_wrap_impl<T>> transpose() const override {
		return std::make_unique<
			concrete_matrix_wrap_impl<T,typename decltype(mat.transpose())::matrix_type>
			>(mat.transpose());
	}

	//will not work: cyclic type expansion!
	/*
	std::unique_ptr<matrix_wrap_impl<T>> window(window_spec spec) const  override{
		return std::make_unique<
			concrete_matrix_wrap_impl<T,typename decltype(mat.window(spec))::matrix_type>
			>(mat.window(spec));
	}

	std::unique_ptr<matrix_wrap_impl<T>> diagonal() const override {
		return std::make_unique<
			concrete_matrix_wrap_impl<T,Diagonal<matrix_type>>
			>(mat.diagonal());
		}
	
	std::unique_ptr<matrix_wrap_impl<T>> diagonal_matrix() const override {
		return std::make_unique<
			concrete_matrix_wrap_impl<T,typename decltype(mat.diagonal_matrix())::matrix_type>
			>(mat.diagonal_matrix());
		}
	*/
	
	
	std::unique_ptr<iterator_impl<T>> begin() override {
		return std::make_unique< 
			concrete_iterator_impl<T,
				typename matrix_ref<T,matrix_type>::iterator> 
			> (mat.begin());
	}
	
	std::unique_ptr<iterator_impl<T>> end() override {
		return std::make_unique< 
			concrete_iterator_impl<T,
				typename matrix_ref<T,matrix_type>::iterator> 
			> (mat.end());
	}
	std::unique_ptr<const_iterator_impl<T>> begin() const override {
		return std::make_unique< 
			concrete_const_iterator_impl<T,
				typename matrix_ref<T,matrix_type>::const_iterator> 
			> (mat.begin());
	}
	std::unique_ptr<const_iterator_impl<T>> end() const override {
		return std::make_unique< 
			concrete_const_iterator_impl<T,
				typename matrix_ref<T,matrix_type>::const_iterator> 
			> (mat.end());
	}
	
	unsigned get_height() const override { return mat.get_height(); }
	unsigned get_width() const override { return mat.get_width(); }
	
	concrete_matrix_wrap_impl(const matrix_ref<T,matrix_type>& M) : mat(M) {}
	
	private:
	matrix_ref<T,matrix_type> mat;
};














template<typename T, class decorated>
class concrete_matrix_wrap_impl<T,Diagonal_matrix<decorated>> : public matrix_wrap_impl<T> {
	public:
	T& get(unsigned i, unsigned j) override { 
		static T result;
		return result=mat(i,j); 
		}
	const T& get(unsigned i, unsigned j) const override { return mat(i,j); }

    std::vector<T> get_sub(unsigned from_r, unsigned to_r, unsigned from_c, unsigned to_c)
    override { return mat.get_sub(from_r, to_r, from_c, to_c); }


	std::unique_ptr<matrix_wrap_impl<T>> clone() const override {
		return std::make_unique<concrete_matrix_wrap_impl<T,Diagonal_matrix<decorated>>>(mat);
	}
	
	std::unique_ptr<matrix_wrap_impl<T>> transpose() const override {
		return clone();
	}

	//will not work: cyclic type expansion!
	/*
	std::unique_ptr<matrix_wrap_impl<T>> window(window_spec spec) const  override{
		return std::make_unique<
			concrete_matrix_wrap_impl<T,typename decltype(mat.window(spec))::matrix_type>
			>(mat.window(spec));
	}

	std::unique_ptr<matrix_wrap_impl<T>> diagonal() const override {
		return std::make_unique<
			concrete_matrix_wrap_impl<T,Diagonal<matrix_type>>
			>(mat.diagonal());
		}
	
	std::unique_ptr<matrix_wrap_impl<T>> diagonal_matrix() const override {
		return std::make_unique<
			concrete_matrix_wrap_impl<T,typename decltype(mat.diagonal_matrix())::matrix_type>
			>(mat.diagonal_matrix());
		}
	*/
	
	
	std::unique_ptr<iterator_impl<T>> begin() override {
		return std::make_unique< 
			concrete_nonconst_iterator_impl<T,
				typename matrix_ref<T,Diagonal_matrix<decorated>>::const_iterator> 
			> (mat.begin());
	}
	
	std::unique_ptr<iterator_impl<T>> end() override {
		return std::make_unique< 
			concrete_nonconst_iterator_impl<T,
				typename matrix_ref<T,Diagonal_matrix<decorated>>::const_iterator> 
			> (mat.end());
	}
	
	
	std::unique_ptr<const_iterator_impl<T>> begin() const override {
		return std::make_unique< 
			concrete_const_iterator_impl<T,
				typename matrix_ref<T,Diagonal_matrix<decorated>>::const_iterator> 
			> (mat.begin());
	}
	std::unique_ptr<const_iterator_impl<T>> end() const override {
		return std::make_unique< 
			concrete_const_iterator_impl<T,
				typename matrix_ref<T,Diagonal_matrix<decorated>>::const_iterator> 
			> (mat.end());
	}
	
	unsigned get_height() const override { return mat.get_height(); }
	unsigned get_width() const override { return mat.get_width(); }
	
	concrete_matrix_wrap_impl(const matrix_ref<T,Diagonal_matrix<decorated>>& M) : mat(M) {}
	
	private:
	matrix_ref<T,Diagonal_matrix<decorated>> mat;
};














template<typename T>
class matrix_wrap {
	public:
	
	typedef T type;
	typedef iterator_wrap<T> iterator;
	typedef const_iterator_wrap<T> const_iterator;
	
	
	T& operator ()(unsigned i, unsigned j) { return pimpl->get(i,j); }
	const T& operator ()(unsigned i, unsigned j) const { return pimpl->get(i,j); }
	std::vector<T> get_sub(unsigned from_r, unsigned to_r, unsigned from_c, unsigned to_c)
    { return pimpl->get_sub(from_r, to_r, from_c, to_c); }
	
	iterator begin() { return pimpl->begin(); }
	iterator end() { return pimpl->end(); }
	const_iterator begin() const { return pimpl->begin(); }
	const_iterator end() const { return pimpl->end(); }
	
	matrix_wrap(const matrix_wrap<T>& X) : pimpl(X.pimpl->clone()) {}
	matrix_wrap transpose() const { return matrix_wrap(pimpl->transpose()); }
	
	
	template<class matrix_type>
	matrix_wrap(const matrix_ref<T,matrix_type>& M) : 
		pimpl(std::make_unique<concrete_matrix_wrap_impl<T,matrix_type>>(M)) {}
		
	unsigned get_height() const { return pimpl->get_height(); }
	unsigned get_width() const { return pimpl->get_width(); }
	
	private:
	matrix_wrap(std::unique_ptr<matrix_wrap_impl<T>>&& impl) : pimpl(std::move(impl)) {}
	
	std::unique_ptr<matrix_wrap_impl<T>> pimpl;
};





#endif //_MATRIX_WRAP_H_
