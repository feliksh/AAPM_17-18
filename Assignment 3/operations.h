#ifndef OPERATIONS_H
#define OPERATIONS_H

#include<type_traits>
#include<list>
#include<thread>
#include<future>
#include<mutex>
#include <iostream>

#include"matrix.h"
#include"matrix_wrap.h"
#include"exceptions.h"

#define block_size 100

std::mutex m;
std::mutex m2;

template<typename T, typename U>
struct op_traits {
	typedef decltype(T() + U()) sum_type;
	typedef decltype(T() * U()) prod_type;
};


template<typename T, unsigned h, unsigned w>
class matrix_product;

template<typename T, unsigned h, unsigned w>
class matrix_addition{
public:

    static constexpr unsigned H=h;
    static constexpr unsigned W=w;

    operator matrix<T>() {
        resolve();
        matrix_wrap<T> lhs=matrices.front(), rhs=matrices.back();
        assert(lhs.get_width()==rhs.get_width() || lhs.get_height()==rhs.get_height());
        const unsigned height = lhs.get_height();
        const unsigned width = lhs.get_width();
        matrix<T> result(height, width);
        for (unsigned i=0; i!=height; ++i)
            for (unsigned j=0; j!=width; j++)
                result(i,j) = lhs(i,j) + rhs(i,j);
        std::cerr << "addition conversion\n";
        return result;
    }

    template<unsigned h2, unsigned w2>
    operator matrix<T,h2,w2>(){
        static_assert((h==0 || h==h2) && (w==0 || w==w2), "sized addition conversion to wrong sized matrix");
        assert(h2==get_height() && w2==get_width());
        resolve();
        matrix_wrap<T> lhs=matrices.front(), rhs=matrices.back();
        matrix<T,h2,w2> result;
        for(unsigned i=0; i!=H; ++i)
            for(unsigned j=0; j!=W; ++j)
                result(i,j) = lhs(i,j) + rhs(i,j);
        std::cerr << "sized addition conversion\n";
        return result;
    };

    unsigned get_height() const { return matrices.front().get_height(); }
    unsigned get_width() const { return matrices.back().get_width(); }

    template<typename Z, class LType, typename U, class RType>
    friend std::enable_if_t<std::is_same<Z,U>::value,
            matrix_addition<Z, matrix_ref<Z,LType>::H, matrix_ref<Z,LType>::W>>
    operator + (const matrix_ref<Z,LType>& lhs, const matrix_ref<U,RType>& rhs);

    template<typename Z, unsigned h2, unsigned w2, typename U, class RType>
    friend std::enable_if_t<std::is_same<Z, U>::value, matrix_addition<Z,h2,w2>>
    operator + (matrix_addition<Z,h2,w2>&& lhs, const matrix_ref<U,RType>& rhs);

    template<typename Z, unsigned h3, unsigned w3, typename U, unsigned h2, unsigned w2>
    friend std::enable_if_t<std::is_same<Z,U>::value, matrix_addition<Z,h3,w2>>
    operator + (matrix_product<Z,h3,w3>&& lhs, matrix_product<U,h2,w2>&& rhs);

    template<typename Z, unsigned h2, unsigned w2, typename U, class RType>
    friend std::enable_if_t<std::is_same<Z,U>::value, matrix_addition<Z,h2,w2>>
    operator + (matrix_product<Z,h2,w2>&& lhs, const matrix_ref<U,RType>& rhs);

    template<typename Z, unsigned h2, unsigned w2, typename U, class RType>
    friend std::enable_if_t<std::is_same<Z,U>::value, matrix_addition<Z,h2,w2>>
    operator + (const matrix_ref<U,RType>& lhs, matrix_product<Z,h2,w2>&& rhs);

    template<typename Z, unsigned h3, unsigned w3, typename U, unsigned h2, unsigned w2>
    friend std::enable_if_t<std::is_same<Z,U>::value, matrix_addition<Z,h3,w3>>
    operator + (matrix_addition<Z,h3,w3>&& lhs, matrix_addition<U,h2,w2>&& rhs);

    template<typename U, unsigned h2, unsigned w2>
    friend void force_addition(matrix_addition<U,h2,w2>&& sum, std::promise<matrix<U>> promise);


    matrix_addition(matrix_addition<T,h,w>&& X) = default;

private:
    matrix_addition()=default;

    template<unsigned w2>
    matrix_addition(matrix_addition<T,h,w2>&& X) : matrices(std::move(X.matrices)) {}

    template<class matrix_type>
    void add(matrix_ref<T,matrix_type> mat) {
        matrices.emplace_back(mat);
    }

    void resolve(){
        std::unique_lock<std::mutex> lock(m2, std::defer_lock);
        std::vector<std::thread> threads;

        if(matrices.size() > 2){
            lock.lock();
            // this is safe because I know that I never start a new thread using already working matrices
            // and they are not going to change the list 'matrices' until I release the lock.
            // + new threads are started once all old threads have finished their execution.
            for(int k=0; k < matrices.size(); k=k+2)
                if(k+1 < matrices.size()){
                    typename std::list<matrix_wrap<T>>::iterator lhs = std::next(matrices.begin(), k);
                    typename std::list<matrix_wrap<T>>::iterator rhs = lhs;
                    ++rhs;
                    try {
                        threads.emplace_back(std::thread([this, lhs, rhs] {
                            const unsigned height = lhs->get_height();
                            const unsigned width = lhs->get_width();
                            assert(width == rhs->get_width() && height == rhs->get_height());
                            std::unique_lock<std::mutex> locke(m2, std::defer_lock);
                            matrix<T> result(height,width);
                            for (unsigned i = 0; i != height; ++i)
                                for (unsigned j = 0; j != width; ++j)
                                    result(i,j) = (*lhs)(i,j) + (*rhs)(i,j);

                            locke.lock();
                            matrices.erase(rhs);
                            matrices.emplace(lhs, result);
                            matrices.erase(lhs);
                            locke.unlock();
                        }));
                    }catch(...) { handle_exception(); }
                }
            lock.unlock();
            for(auto & t : threads) {t.join();}
            threads.clear();
        }
    }

    std::list<matrix_wrap<T>> matrices;
};

template<typename T, unsigned h, unsigned w>
void force_addition(matrix_addition<T,h,w>&& sum, std::promise<matrix<T>> promise){
    sum.resolve();
    matrix_wrap<T> lhs=sum.matrices.front(), rhs=sum.matrices.back();
    assert(lhs.get_width()==rhs.get_width() || lhs.get_height()==rhs.get_height());
    const unsigned height = lhs.get_height();
    const unsigned width = lhs.get_width();
    matrix<T> result(height, width);
    for (unsigned i=0; i!=height; ++i)
        for (unsigned j=0; j!=width; j++)
            result(i,j) = lhs(i,j) + rhs(i,j);
    promise.set_value(result);
};

// ***** Addition operators ******* //
// ******************************** //

// mat + mat  [same type]
template<typename T, class LType, typename U, class RType>
std::enable_if_t<std::is_same<T, U>::value,
        matrix_addition<T, matrix_ref<T,LType>::H, matrix_ref<T,LType>::W>>
operator + (const matrix_ref<T,LType>& lhs, const matrix_ref<U,RType>& rhs){
    if (lhs.get_height()!=rhs.get_height() || lhs.get_width()!=rhs.get_width())
        throw std::domain_error("dimension mismatch in Matrix addition");
    matrix_addition<T, matrix_ref<T,LType>::H, matrix_ref<T,LType>::W> result;
    result.add(lhs);
    result.add(rhs);
    return result;
};

// dyno + dyno [not same type]
template<typename T, class LType, typename U, class RType>
std::enable_if_t<matrix_ref<T,LType>::H*matrix_ref<U,RType>::H==0
                 && !std::is_same<T, U>::value,
        matrix<typename op_traits<T,U>::sum_type>>
operator + (const matrix_ref<T,LType>& left, const matrix_ref<U,RType>& right) {
    if (left.get_height()!=right.get_height() || left.get_width()!=right.get_width())
        throw std::domain_error("dimension mismatch in Matrix addition");
    const unsigned height=left.get_height();
    const unsigned width=left.get_width();
    matrix<typename op_traits<T,U>::sum_type> result(height, width);
    for (unsigned i=0; i!=height; ++i)
        for (unsigned j=0; j!=width; j++)
            result(i,j) = left(i,j) + right(i,j);
    return result;
}

// stat + stat [not same type]
template<typename T, class LType, typename U, class RType>
std::enable_if_t<matrix_ref<T,LType>::H*matrix_ref<U,RType>::H!=0
                 && !std::is_same<T, U>::value,
        matrix<typename op_traits<T,U>::sum_type,matrix_ref<T,LType>::H,matrix_ref<T,LType>::W>>
operator + (const matrix_ref<T,LType>& lhs, const matrix_ref<U,RType>& rhs){
    static_assert(matrix_ref<T,LType>::H==matrix_ref<U,RType>::H
                  && matrix_ref<T,LType>::W==matrix_ref<U,RType>::W, "dimension mismatch in Matrix adidtion");
    const unsigned height=lhs.get_height();
    const unsigned width=lhs.get_width();
    matrix<typename op_traits<T,U>::sum_type, matrix_ref<T,LType>::H, matrix_ref<T,LType>::W> result;
    for (unsigned i=0; i!=height; ++i)
        for (unsigned j=0; j!=width; j++)
            result(i,j) = lhs(i,j) + rhs(i,j);
    return result;
};

// mat_add + mat [same type]
template<typename T, unsigned h, unsigned w, typename U, class RType>
std::enable_if_t<std::is_same<T, U>::value,
        matrix_addition<T, h, w>>
operator + (matrix_addition<T,h,w>&& lhs, const matrix_ref<U,RType>& rhs){
    static_assert(w*matrix_ref<U,RType>::H==0 || (w==matrix_ref<U,RType>::W && h==matrix_ref<U,RType>::H),
                  "dimension mismatch in Matrix addition");
    if (lhs.get_width()!=rhs.get_width() || lhs.get_height()!=rhs.get_height())
        throw std::domain_error("dimension mismatch in Matrix addition");
    matrix_addition<T,h,w> result(std::move(lhs));
    result.add(rhs);
    return result;
};

// mat_add + dyno [not same type]
template<typename T, unsigned h, unsigned w, typename U, class RType>
std::enable_if_t<h*matrix_ref<U,RType>::H==0 && !std::is_same<T, U>::value,
        matrix<typename op_traits<T,U>::sum_type>>
operator + (matrix_addition<T,h,w>&& lhs, const matrix_ref<U,RType>& rhs){
    if(lhs.get_width()!=rhs.get_width() || lhs.get_height()!=rhs.get_height())
        throw std::domain_error("dimension mismatch in Matrix addition");
    const unsigned height = rhs.get_height();
    const unsigned width = rhs.get_width();
    matrix<T> left = lhs;
    matrix<typename op_traits<T,U>::sum_type> result(height, width);
    for(unsigned i=0; i!=height; ++i)
        for(unsigned j=0; j!=width; ++j)
            result(i,j) = left(i,j) + rhs(i,j);
    return result;
};

// mat_add + stat [not same type]
template<typename T, unsigned h, unsigned w, typename U, class RType>
std::enable_if_t<h*matrix_ref<U,RType>::H!=0 && !std::is_same<T, U>::value,
        matrix<typename op_traits<T,U>::sum_type,h,w>>
operator + (matrix_addition<T,h,w>&& lhs, const matrix_ref<U,RType>& rhs){
    static_assert(h==matrix_ref<U,RType>::H && w==matrix_ref<U,RType>::W,
                  "dimension mismatch in Matrix addition");
    const unsigned height = rhs.get_height();
    const unsigned width = rhs.get_width();
    matrix<T,h,w> left = lhs;
    matrix<typename op_traits<T,U>::sum_type,h,w> result;
    for(unsigned i=0; i!=height; ++i)
        for(unsigned j=0; j!=width; ++j)
            result(i,j) = left(i,j) + rhs(i,j);
    return result;
};

// mat_mult + mat_mult [same type]
template<typename T, unsigned h, unsigned w, typename U, unsigned h2, unsigned w2>
std::enable_if_t<std::is_same<T,U>::value, matrix_addition<T,h,w>>
operator + (matrix_addition<T,h,w>&& lhs, matrix_addition<U,h2,w2>&& rhs){
    static_assert(h*w*h2*w2==0 || (h==h2 && w==w2), "dimension mismatch in Matrix addition");
    if(lhs.get_width()!=rhs.get_width() || lhs.get_height()!=rhs.get_height())
        throw std::domain_error("dimension mismatch in Matrix addition");
    std::promise<matrix<T>> lhs_promise;
    std::promise<matrix<U>> rhs_promise;
    auto lhs_future = lhs_promise.get_future();
    auto rhs_future = rhs_promise.get_future();
    matrix_addition<T, h, w2> result;
    try {
        std::thread th1(force_addition<T, h, w>, std::move(lhs), std::move(lhs_promise));
        std::thread th2(force_addition<U, h2, w2>, std::move(rhs), std::move(rhs_promise));
        result.add(lhs_future.get());
        result.add(rhs_future.get());
        th1.join();
        th2.join();
    }catch(...){ handle_exception(); }

    return result;
};


template<typename T, typename U>
void sub_multiply(matrix_wrap<typename op_traits<T,U>::prod_type> result,
                  matrix_wrap<T> lhs, matrix_wrap<U> rhs, unsigned i, unsigned j){
    const unsigned lh = lhs.get_height();
    const unsigned lw = lhs.get_width(); // lw==rh
    const unsigned rh = rhs.get_height();// rh==lw
    const unsigned rw = rhs.get_width();
    const unsigned to_i = std::min(i+block_size, lh);
    const unsigned to_j = std::min(j+block_size, rw);
    unsigned b1_width = 0, b2_width = 0, span = 0;
    std::vector<T> block1;
    std::vector<U> block2;
    for(unsigned ii=i; ii<to_i; ++ii)
        for (unsigned jj=j; jj<to_j; ++jj)
            result(ii, jj) = 0;

    for(unsigned k=0; k<lw; k=k+block_size) {
        block1 = lhs.get_sub(i, to_i, k, std::min(k+block_size, lw));
        b1_width = std::min(k+block_size, lw) - k;
        block2 = rhs.get_sub(k, std::min(k+block_size, rh), j, to_j);
        b2_width = to_j - j;
        span = b1_width;
        for(unsigned ii=i; ii<to_i; ++ii) {
            for (unsigned jj=j; jj<to_j; ++jj) {
                for(unsigned b=0; b!=span; ++b)
                    result(ii, jj) += block1[(b1_width*(ii-i))+b] * block2[(b2_width*b)+(jj-j)];
            }
        }
        block1.clear();
        block2.clear();
    }
}

template<typename T, typename U>
void do_multiply(matrix_wrap<typename op_traits<T,U>::prod_type> result,
                 const matrix_wrap<T> lhs, const matrix_wrap<U> rhs) {
    const unsigned height = result.get_height();
    const unsigned width = result.get_width();
    const unsigned span = lhs.get_width();
    // dimension check not needed since it is made from function which called this
    // assert(span==rhs.get_height());
    for (unsigned i=0; i!=height; ++i)
        for (unsigned j=0; j!=width; ++j) {
            result(i,j)=0;
            for (unsigned k=0; k!=span; ++k)
                result(i,j) += lhs(i,k)*rhs(k,j);
        }
}

template<typename T, typename U>
void do_parallel_multiply(matrix_wrap<typename op_traits<T,U>::prod_type> result, const matrix_wrap<T> lhs, const matrix_wrap<U> rhs) {
    std::list<std::thread> threads;
    const unsigned height = result.get_height();
    const unsigned width = result.get_width();
    // dimension check not needed since it is made from function which called this
    // assert(lhs.get_width()==rhs.get_height());
    if(height < block_size || width < block_size) return do_multiply<T,U>(result, lhs, rhs);
    for(unsigned i=0; i<height; i=i+block_size) {
        for (unsigned j = 0; j < width; j = j + block_size) {
            // call thread on block C_{i, i+block_size, j, j+block_size}
            threads.emplace_back(sub_multiply<T, U>, result, lhs, rhs, i, j);
        }
    }
    for(auto& t: threads) { t.join(); }
    threads.clear();
}

template<typename T,unsigned h, unsigned w>
class matrix_product {
	public:
	
	static constexpr unsigned H=h;
	static constexpr unsigned W=w;

	operator matrix<T>() {
		resolve();
		matrix_wrap<T> lhs=matrices.front(), rhs=matrices.back();
		const unsigned height = lhs.get_height();
		const unsigned width = rhs.get_width();
		const unsigned span = lhs.get_width();
		assert(span==rhs.get_height());
		matrix<T> result(height, width);
        try{ do_parallel_multiply<T,T>(result, lhs, rhs); }
        catch(...) { handle_exception(); }
		std::cerr << "product conversion\n";
		return result;
	}
	
	template<unsigned h2, unsigned w2>
	operator matrix<T,h2,w2>() {
		static_assert((h==0 || h==h2) && (w==0 || w==w2), "sized product conversion to wrong sized matrix");
		assert(h2==get_height() && w2==get_width());
		resolve();
		matrix_wrap<T> lhs=matrices.front(), rhs=matrices.back();
		const unsigned span = lhs.get_width();
		assert(span==rhs.get_height());
		matrix<T,h2,w2> result;
		try{ do_parallel_multiply<T,T>(result, lhs, rhs); }
        catch(...) { handle_exception(); }
		std::cerr << "sized product conversion\n";
		return result;				
	}
	
	unsigned get_height() const { return matrices.front().get_height(); }
	unsigned get_width() const { return matrices.back().get_width(); }
    std::list<matrix_wrap<T>> get_mats() const { return matrices; }
    std::list<unsigned> get_sizes() const { return sizes; }
    std::list<bool> get_busy() const { return busy; }


    template<typename Z, typename U, class LType, class RType>
    friend std::enable_if_t<std::is_same<Z,U>::value,
            matrix_product<Z, matrix_ref<Z,LType>::H, matrix_ref<U,RType>::W>>
    operator * (const matrix_ref<Z,LType>& lhs, const matrix_ref<U,RType>& rhs);

    template<typename Z, typename U, class RType, unsigned h2, unsigned w2>
    friend std::enable_if_t<std::is_same<Z,U>::value, matrix_product<Z,h2,matrix_ref<U,RType>::W>>
    operator * (matrix_product<Z,h2,w2>&& lhs, const matrix_ref<U,RType>& rhs);

    template<typename Z, unsigned h3, unsigned w3, typename U, unsigned h2, unsigned w2>
    friend std::enable_if_t<std::is_same<Z,U>::value, matrix_product<Z,h3,w2>>
    operator * (matrix_addition<Z,h3,w3>&& lhs, matrix_addition<U,h2,w2>&& rhs);

    template<typename Z, unsigned h2, unsigned w2, typename U, class RType>
    friend std::enable_if_t<std::is_same<Z,U>::value, matrix_product<Z,h2,matrix_ref<U,RType>::W>>
    operator * (matrix_addition<Z,h2,w2>&& lhs, const matrix_ref<U,RType>& rhs);

    template<typename Z, unsigned h2, unsigned w2, typename U, class RType>
    friend std::enable_if_t<std::is_same<Z,U>::value, matrix_product<Z,matrix_ref<U,RType>::H,w2>>
    operator * (const matrix_ref<U,RType>& lhs, matrix_addition<Z,h2,w2>&& rhs);

    template<typename Z, unsigned h2, unsigned w2>
    friend void force_multiplication(matrix_product<Z,h2,w2>&& prod, std::promise<matrix<Z>> promise);

    template<typename Z, unsigned h2, unsigned w2>
    friend void force_mult_stat(matrix_product<Z,h2,w2>&& prod, std::promise<matrix<Z,h2,w2>> promise);

    matrix_product(matrix_product<T,h,w>&& X) = default;

	private:

	matrix_product()=default;

    template<unsigned w2>
    matrix_product(matrix_product<T,h,w2>&& X) : matrices(std::move(X.get_mats())),
                                                 sizes(std::move(X.get_sizes())),
                                                 busy(std::move(X.get_busy()))  {}

	template<class matrix_type>
	void add(matrix_ref<T,matrix_type> mat) {
		matrices.emplace_back(mat);
		sizes.push_back(mat.get_width());
        busy.push_back(false);
	}

    unsigned non_busy(){
        unsigned nr = 0;
        for(std::list<bool>::iterator val=busy.begin(); val!=busy.end(); ++val)
            if(*val == false) nr++;
        return nr;
    }

    void resolve() {
        bool found = true;
        int pos_lhs;
        std::unique_lock<std::mutex> lock(m, std::defer_lock);
        std::vector<std::thread> threads;
        while(matrices.size()>2){
            lock.lock();
            while(found && non_busy()>1){
                found = false;
                pos_lhs = -1;
                typename std::list<matrix_wrap<T>>::iterator lhs = find_max(&pos_lhs);
                if(pos_lhs > -1) {
                    found = true;
                    typename std::list<matrix_wrap<T>>::iterator rhs = lhs;
                    ++rhs;
                    std::list<unsigned>::iterator one_size = std::next(sizes.begin(), pos_lhs);
                    std::list<unsigned>::iterator two_size = one_size; ++two_size;
                    std::list<bool>::iterator one_busy = std::next(busy.begin(), pos_lhs);
                    std::list<bool>::iterator two_busy = one_busy; ++two_busy;
                    *one_busy = true;
                    *two_busy = true;

                    try{
                    threads.emplace_back(std::thread([this, lhs, rhs, one_size, two_size, one_busy, two_busy]{
                        // lock ON
                        std::unique_lock<std::mutex> locke(m, std::defer_lock);
                        locke.lock();
                        typename std::list<matrix_wrap<T>>::iterator result=matrices.emplace(lhs, matrix<T>(lhs->get_height(), rhs->get_width()));
                        std::list<bool>::iterator newb = busy.emplace(one_busy, true);
                        std::list<unsigned>::iterator news =  sizes.emplace(one_size, 0);
                        locke.unlock();
                        // lock OFF

                        do_parallel_multiply<T, T>(*result, *lhs, *rhs);

                        // lock ON
                        locke.lock();
                        sizes.erase(one_size);
                        sizes.erase(two_size);
                        *news = result->get_width();
                        busy.erase(one_busy);
                        busy.erase(two_busy);
                        *newb = false;
                        matrices.erase(lhs);
                        matrices.erase(rhs);
                        locke.unlock();
                        // lock OFF
                    }));
                    }catch(...){ handle_exception(); }
                }
            }
            lock.unlock();
            for(auto& t : threads) {t.join();}
            threads.clear();
            found = true;
        }
    }

	typename std::list<matrix_wrap<T>>::iterator find_max(int *pos) {
		typename std::list<matrix_wrap<T>>::iterator mat_iter=matrices.begin();
		typename std::list<matrix_wrap<T>>::iterator mat_max=mat_iter;
		std::list<unsigned>::iterator size_iter=sizes.begin();
		std::list<unsigned>::iterator last=--(sizes.end());
        std::list<bool>::iterator busy_iter = busy.begin();
		unsigned size_max=*size_iter;
        unsigned act_pos = 0;
        if(!(*busy_iter) && !(*std::next(busy_iter))) {
            *pos = 0;
        }else{
            size_max = 0;
        }

		while (size_iter!=last) {
			if(*size_iter > size_max && !(*busy_iter) && !(*std::next(busy_iter))){
				size_max = *size_iter;
				mat_max = mat_iter;
                *pos = act_pos;
			}
			++mat_iter;
			++size_iter;
            ++busy_iter;
            ++act_pos;
		}

		return mat_max;
	}

	std::list<matrix_wrap<T>> matrices;
	std::list<unsigned> sizes;
    std::list<bool> busy;
};


template<typename T, unsigned h, unsigned w>
void force_multiplication(matrix_product<T,h,w>&& prod, std::promise<matrix<T>> promise){
    prod.resolve();
    matrix_wrap<T> lhs = prod.matrices.front(), rhs = prod.matrices.back();
    const unsigned height = lhs.get_height();
    const unsigned width = rhs.get_width();
    const unsigned span = lhs.get_width();
    assert(span == rhs.get_height());
    matrix<T> result(height, width);
    try{ do_parallel_multiply<T,T>(result, lhs, rhs); }
    catch(...){ handle_exception(); }
    promise.set_value(result);
}

template<typename T, unsigned h, unsigned w>
void force_mult_stat(matrix_product<T,h,w>&& prod, std::promise<matrix<T,h,w>> promise){
    prod.resolve();
    matrix_wrap<T> lhs = prod.matrices.front(), rhs = prod.matrices.back();
    const unsigned height = lhs.get_height();
    const unsigned width = rhs.get_width();
    const unsigned span = lhs.get_width();
    assert(span == rhs.get_height());
    matrix<T,h,w> result;
    for (unsigned i=0; i!=height; ++i)
        for (unsigned j=0; j!=width; ++j) {
            result(i,j) = 0;
            for (unsigned k=0; k!=span; ++k)
                result(i,j) += lhs(i,k)*rhs(k,j);
        }
    promise.set_value(result);
};




// ***** Multiplication operators ******* //
// ************************************** //

// mat * mat [same type]
template<typename T, typename U, class LType, class RType>
std::enable_if_t<std::is_same<T,U>::value,
        matrix_product<T, matrix_ref<T,LType>::H, matrix_ref<U,RType>::W>>
operator * (const matrix_ref<T,LType>& lhs, const matrix_ref<U,RType>& rhs) {
    static_assert( (matrix_ref<T,LType>::H==0||matrix_ref<U,RType>::W==0) || matrix_ref<T,LType>::W==matrix_ref<U,RType>::H,
                  "dimension mismatch in Matrix multiplication");
    if (lhs.get_width()!=rhs.get_height())
        throw std::domain_error("dimension mismatch in Matrix multiplication");
    matrix_product<T, matrix_ref<T,LType>::H, matrix_ref<U,RType>::W> result;
    result.add(lhs);
    result.add(rhs);
    return result;
}


// dyno * dyno [not same type]
template<typename T, class LType, typename U, class RType>
std::enable_if_t<!std::is_same<T,U>::value && matrix_ref<T,LType>::W*matrix_ref<U,RType>::H==0,
        matrix<typename op_traits<T,U>::prod_type>>
operator * (const matrix_ref<T,LType>& lhs, const matrix_ref<U,RType>& rhs){
    if(lhs.get_width()!=rhs.get_height())
        throw std::domain_error("dimension mismatch in Matrix multiplication");
    const unsigned height = lhs.get_height();
    const unsigned width = rhs.get_width();
    matrix<typename op_traits<T,U>::prod_type> result(height,width);
    try{ do_parallel_multiply<T,U>(result, lhs, rhs); }
    catch(...) { handle_exception(); }
    return result;
};


// stat * stat [not same type]
template<typename T, class LType, typename U, class RType>
std::enable_if_t<!std::is_same<T,U>::value && matrix_ref<T,LType>::W*matrix_ref<U,RType>::H!=0,
        matrix<typename op_traits<T,U>::prod_type, matrix_ref<T,LType>::H, matrix_ref<U,RType>::W>>
operator * (const matrix_ref<T,LType>& lhs, const matrix_ref<U,RType>& rhs){
    static_assert(matrix_ref<T,LType>::W==matrix_ref<U,RType>::H,
                  "dimension mismatch in Matrix multiplication");
    const unsigned height = lhs.get_height();
    const unsigned width = rhs.get_width();
    matrix<typename op_traits<T,U>::prod_type, matrix_ref<T,LType>::H, matrix_ref<U,RType>::W> result;
    try{ do_parallel_multiply<T,U>(result, lhs, rhs); }
    catch(...) { handle_exception(); }
    return result;
};

// mat_mult * mat [same type]
template<typename T, typename U, class RType, unsigned h, unsigned w>
std::enable_if_t<std::is_same<T,U>::value, matrix_product<T,h,matrix_ref<U,RType>::W>>
operator * (matrix_product<T,h,w>&& lhs, const matrix_ref<U,RType>& rhs){
    static_assert(h*w*matrix_ref<U,RType>::H==0 || w==matrix_ref<U,RType>::H,
                  "dimension mismatch in Matrix multiplicaiton");
    if(lhs.get_width()!=rhs.get_height())
        throw std::domain_error("dimension mismatch in Matrix multiplicaiton");
    matrix_product<T,h,matrix_ref<U,RType>::W> result(std::move(lhs));
    result.add(rhs);
    return result;
};

// mat_mult * dyno [not same type]
template<typename T, unsigned h, unsigned w, typename U, class RType>
std::enable_if_t<!std::is_same<T,U>::value && w*h*matrix_ref<U,RType>::H==0,
        matrix<typename op_traits<T,U>::prod_type>>
operator * (matrix_product<T,h,w>&& lhs, const matrix_ref<U,RType>& rhs){
    if(lhs.get_width()!=rhs.get_height())
        throw std::domain_error("dimension mismatch in Matrix multiplicaiton");
    const unsigned height = lhs.get_height();
    const unsigned width = rhs.get_width();
    matrix<T> left = lhs;
    matrix<typename op_traits<T,U>::prod_type> result(height,width);
    try{ do_parallel_multiply<T,U>(result, left, rhs); }
    catch(...) { handle_exception(); }
    return result;
};

// mat_mult * stat [not same type]
template<typename T, unsigned h, unsigned w, typename U, class RType>
std::enable_if_t<!std::is_same<T,U>::value && w*h*matrix_ref<U,RType>::H!=0,
        matrix<typename op_traits<T,U>::prod_type,h,matrix_ref<U,RType>::W>>
operator * (matrix_product<T,h,w>&& lhs, const matrix_ref<U,RType>& rhs) {
    static_assert(w==matrix_ref<U,RType>::H,
                  "dimension mismatch in Matrix multiplication");
    matrix<T,h,w> left = lhs;
    matrix<typename op_traits<T,U>::prod_type,h,matrix_ref<U,RType>::W> result;
    try{ do_parallel_multiply<T,U>(result, left, rhs); }
    catch(...){ handle_exception(); }
    return result;
}

// ***** Mixed operations operators ******* //
// **************************************** //

// mat_add * mat_add [same type]
template<typename T, unsigned h, unsigned w, typename U, unsigned h2, unsigned w2>
std::enable_if_t<std::is_same<T,U>::value, matrix_product<T,h,w2>>
operator * (matrix_addition<T,h,w>&& lhs, matrix_addition<U,h2,w2>&& rhs){
    static_assert(h*w*h2*w2==0 || w==h2,
                  "dimension mismatch in Matrix multiplication");
    if (lhs.get_width()!=rhs.get_height())
        throw std::domain_error("dimension mismatch in Matrix multiplication");
    std::promise<matrix<T>> lhs_promise;
    std::promise<matrix<U>> rhs_promise;
    auto lhs_future = lhs_promise.get_future();
    auto rhs_future = rhs_promise.get_future();
    matrix_product<T, h, w2> result;
    try {
        std::thread th1(force_addition<T, h, w>, std::move(lhs), std::move(lhs_promise));
        std::thread th2(force_addition<U, h2, w2>, std::move(rhs), std::move(rhs_promise));
        result.add(lhs_future.get());
        result.add(rhs_future.get());
        th1.join();
        th2.join();
    }catch(...) { handle_exception(); }
    return result;
};

// mat_add_stat * mat_add_stat [not same type]
template<typename T, unsigned h, unsigned w, typename U, unsigned h2, unsigned w2>
std::enable_if_t<!std::is_same<T,U>::value && h*w*h2*w2!=0, matrix<typename op_traits<T,U>::prod_type,h,w2>>
operator * (matrix_addition<T,h,w>&& lhs, matrix_addition<U,h2,w2>&& rhs){
    static_assert(w==h2, "dimension mismatch in Matrix multiplication");
    std::promise<matrix<T>> lhs_promise;
    std::promise<matrix<U>> rhs_promise;
    auto lhs_future = lhs_promise.get_future();
    auto rhs_future = rhs_promise.get_future();
    matrix<typename op_traits<T,U>::prod_type,h,w2> result;
    try {
        std::thread th1(force_addition<T, h, w>, std::move(lhs), std::move(lhs_promise));
        std::thread th2(force_addition<U, h2, w2>, std::move(rhs), std::move(rhs_promise));
        do_parallel_multiply<T, U>(result, std::move(lhs_future.get()), std::move(rhs_future.get()));
        th1.join();
        th2.join();
    }catch(...) { handle_exception(); }
    return result;
};

// mat_add_dyno * mat_add_dyno [not same type]
template<typename T, unsigned h, unsigned w, typename U, unsigned h2, unsigned w2>
std::enable_if_t<!std::is_same<T,U>::value && h*w*h2*w2==0, matrix<typename op_traits<T,U>::prod_type>>
operator * (matrix_addition<T,h,w>&& lhs, matrix_addition<U,h2,w2>&& rhs) {
    if(lhs.get_width()!=rhs.get_height())
        throw std::domain_error("dimension mismatch in Matrix multiplication");
    const unsigned height = lhs.get_height();
    const unsigned width = rhs.get_width();
    std::promise<matrix<T>> lhs_promise;
    std::promise<matrix<U>> rhs_promise;
    auto lhs_future = lhs_promise.get_future();
    auto rhs_future = rhs_promise.get_future();
    matrix<typename op_traits<T,U>::prod_type> result(height,width);
    try {
        std::thread th1(force_addition<T, h, w>, std::move(lhs), std::move(lhs_promise));
        std::thread th2(force_addition<U, h2, w2>, std::move(rhs), std::move(rhs_promise));
        do_parallel_multiply<T, U>(result, std::move(lhs_future.get()), std::move(rhs_future.get()));
        th1.join();
        th2.join();
    }catch(...) { handle_exception(); }
    return result;
}

// mat_mult + mat_mult [same type]
template<typename T, unsigned h, unsigned w, typename U, unsigned h2, unsigned w2>
std::enable_if_t<std::is_same<T,U>::value, matrix_addition<T,h,w2>>
operator + (matrix_product<T,h,w>&& lhs, matrix_product<U,h2,w2>&& rhs){
    std::promise<matrix<T>> lhs_promise;
    std::promise<matrix<T>> rhs_promise;
    auto lhs_future = lhs_promise.get_future();
    auto rhs_future = rhs_promise.get_future();
    matrix_addition<T,h,w2> result;
    try {
        std::thread th1(force_multiplication<T, h, w>, std::move(lhs), std::move(lhs_promise));
        std::thread th2(force_multiplication<T, h2, w2>, std::move(rhs), std::move(rhs_promise));
        result.add(lhs_future.get());
        result.add(rhs_future.get());
        th1.join();
        th2.join();
    }catch(...) { handle_exception(); }
    return result;
};

// mat_mult_dyno + mat_mult_dyno [not same type]
template<typename T, unsigned h, unsigned w, typename U, unsigned h2, unsigned w2>
std::enable_if_t<!std::is_same<T,U>::value && h*w*h2*w2==0, matrix<typename op_traits<T,U>::sum_type>>
operator + (matrix_product<T,h,w>&& lhs, matrix_product<U,h2,w2>&& rhs){
    if(lhs.get_width()!=rhs.get_width() || lhs.get_height()!=rhs.get_height())
        throw std::domain_error("dimension mismatch in Matrix addition");
    const unsigned height = lhs.get_height();
    const unsigned width = rhs.get_width();
    std::promise<matrix<T>> lhs_promise;
    std::promise<matrix<U>> rhs_promise;
    auto lhs_future = lhs_promise.get_future();
    auto rhs_future = rhs_promise.get_future();
    matrix<T> left;
    matrix<U> right;
    try {
        std::thread th1(force_multiplication<T, h, w>, std::move(lhs), std::move(lhs_promise));
        std::thread th2(force_multiplication<U, h2, w2>, std::move(rhs), std::move(rhs_promise));
        left = lhs_future.get();
        right = rhs_future.get();
        th1.join(); th2.join();
    }catch(...) { handle_exception(); }
    matrix<typename op_traits<T,U>::prod_type> result(height,width);
    for(unsigned i=0; i!=height; ++i)
        for(unsigned j=0; j!=width; ++j)
            result(i,j) = left(i,j) + right(i,j);

    return result;
};

// mat_mult_stat + mat_mult_stat [not same type]
template<typename T, unsigned h, unsigned w, typename U, unsigned h2, unsigned w2>
std::enable_if_t<!std::is_same<T,U>::value && h*w*h2*w2!=0, matrix<typename op_traits<T,U>::sum_type,h,w2>>
operator + (matrix_product<T,h,w>&& lhs, matrix_product<U,h2,w2>&& rhs) {
    static_assert(h==h2 && w==w2, "dimension mismatch in Matrix addition");
    const unsigned height = lhs.get_height();
    const unsigned width = rhs.get_width();
    std::promise<matrix<T,h,w>> lhs_promise;
    std::promise<matrix<U,h2,w2>> rhs_promise;
    auto lhs_future = lhs_promise.get_future();
    auto rhs_future = rhs_promise.get_future();
    matrix<T,h,w> left;
    matrix<U,h2,w2> right;
    try {
        std::thread th1(force_mult_stat<T, h, w>, std::move(lhs), std::move(lhs_promise));
        std::thread th2(force_mult_stat<U, h2, w2>, std::move(rhs), std::move(rhs_promise));
        left = lhs_future.get();
        right = rhs_future.get();
    }catch(...) { handle_exception(); }
    matrix<typename op_traits<T,U>::prod_type,h,w2> result;
    for(unsigned i=0; i!=height; ++i)
        for(unsigned j=0; j!=width; ++j)
            result(i,j) = left(i,j) + right(i,j);

    return result;
}

// mat_add * mat [same type]
template<typename T, unsigned h, unsigned w, typename U, class RType>
std::enable_if_t<std::is_same<T,U>::value, matrix_product<T,h,matrix_ref<U,RType>::W>>
operator * (matrix_addition<T,h,w>&& lhs, const matrix_ref<U,RType>& rhs){
    static_assert(h*w*matrix_ref<U,RType>::H == 0 || w==matrix_ref<U,RType>::H,
                  "dimension mismatch in Matrix multiplication");
    if(lhs.get_width()!=rhs.get_height())
        throw std::domain_error("dimension mismatch in Matrix multiplication");
    std::promise<matrix<T>> lhs_promise;
    auto lhs_future = lhs_promise.get_future();
    matrix_product<T,h,matrix_ref<U,RType>::W> result;
    try {
        std::thread th(force_addition<T, h, w>, std::move(lhs), std::move(lhs_promise));
        result.add(lhs_future.get());
        result.add(rhs);
        th.join();
    }catch(...) { handle_exception(); }
    return result;
};
//mirror: mat * mat_add [same type] ok
template<typename T, unsigned h, unsigned w, typename U, class RType>
std::enable_if_t<std::is_same<T,U>::value, matrix_product<T,matrix_ref<U,RType>::H,w>>
operator * (const matrix_ref<U,RType>& lhs, matrix_addition<T,h,w>&& rhs){
    static_assert(h*w*matrix_ref<U,RType>::H == 0 || matrix_ref<U,RType>::W==h,
                  "dimension mismatch in Matrix multiplication");
    if(lhs.get_width()!=rhs.get_height())
        throw std::domain_error("dimension mismatch in Matrix multiplication");
    std::promise<matrix<T>> rhs_promise;
    auto rhs_future = rhs_promise.get_future();
    matrix_product<T,matrix_ref<U,RType>::H,w> result;
    try {
        std::thread th(force_addition<T, h, w>, std::move(rhs), std::move(rhs_promise));
        result.add(lhs);
        result.add(rhs_future.get());
        th.join();
    }catch(...) { handle_exception(); }
    return result;
};

// mat_add * stat [not same type]
template<typename T, unsigned h, unsigned w, typename U, class RType>
std::enable_if_t<!std::is_same<T,U>::value && h*w*matrix_ref<U,RType>::H!=0,
        matrix<typename op_traits<T,U>::prod_type,h,matrix_ref<U,RType>::W>>
operator * (matrix_addition<T,h,w>&& lhs, const matrix_ref<U,RType>& rhs){
    static_assert(w==matrix_ref<U,RType>::H, "dimension mismatch in Matrix multiplication");
    matrix<T,h,w> left = lhs;
    matrix<typename op_traits<T,U>::prod_type,h,matrix_ref<U,RType>::W> result;
    try{ do_parallel_multiply<T,U>(result, left, rhs); }
    catch(...){ handle_exception(); }
    return result;
};
// mirror: stat * mat_add [not same type] ok
template<typename T, unsigned h, unsigned w, typename U, class RType>
std::enable_if_t<!std::is_same<T,U>::value && h*w*matrix_ref<U,RType>::H!=0,
        matrix<typename op_traits<U,T>::prod_type,matrix_ref<U,RType>::H,w>>
operator * (const matrix_ref<U,RType>& lhs, matrix_addition<T,h,w>&& rhs){
    static_assert(matrix_ref<U,RType>::W==h, "dimension mismatch in Matrix multiplication");
    matrix<T,h,w> right = rhs;
    matrix<typename op_traits<U,T>::prod_type,matrix_ref<U,RType>::H,w> result;
    try{ do_parallel_multiply<U,T>(result, lhs, right); }
    catch(...){ handle_exception(); }
    return result;
};

// mat_add * dyno [not same type]
template<typename T, unsigned h, unsigned w, typename U, class RType>
std::enable_if_t<!std::is_same<T,U>::value && h*w*matrix_ref<U,RType>::H==0,
        matrix<typename op_traits<T,U>::prod_type>>
operator * (matrix_addition<T,h,w>&& lhs, const matrix_ref<U,RType>& rhs){
    const unsigned height = lhs.get_height();
    const unsigned width = rhs.get_width();
    if(lhs.get_width()!=rhs.get_height())
        throw std::domain_error("dimension mismatch in Matrix multiplication");
    matrix<T> left = lhs;
    matrix<typename op_traits<T,U>::prod_type> result(height, width);
    try{ do_parallel_multiply<T,U>(result, left, rhs); }
    catch(...){ handle_exception(); }
    return result;
};
// mirror: dyno * mat_add [not same type] ok
template<typename T, unsigned h, unsigned w, typename U, class RType>
std::enable_if_t<!std::is_same<T,U>::value && h*w*matrix_ref<U,RType>::H==0,
        matrix<typename op_traits<U,T>::prod_type>>
operator * (const matrix_ref<U,RType>& lhs, matrix_addition<T,h,w>&& rhs){
    const unsigned height = lhs.get_height();
    const unsigned width = rhs.get_width();
    if(lhs.get_width()!=rhs.get_height())
        throw std::domain_error("dimension mismatch in Matrix multiplication");
    matrix<T> right = rhs;
    matrix<typename op_traits<U,T>::prod_type> result(height, width);
    try{ do_parallel_multiply<U,T>(result, lhs, right); }
    catch(...){ handle_exception(); }
    return result;
};


// mat_mult + mat [same type]
template<typename T, unsigned h, unsigned w, typename U, class RType>
std::enable_if_t<std::is_same<T,U>::value, matrix_addition<T,h,w>>
operator + (matrix_product<T,h,w>&& lhs, const matrix_ref<U,RType>& rhs){
    static_assert(h*w*matrix_ref<U,RType>::H == 0 || (w==matrix_ref<U,RType>::W && h==matrix_ref<U,RType>::H),
                  "dimension mismatch in Matrix addition");
    if(lhs.get_width()!=rhs.get_height())
        throw std::domain_error("dimension mismatch in Matrix addition");
    matrix<T> left = lhs;
    matrix_addition<T,h,w> result;
    result.add(left);
    result.add(rhs);
    return result;
};
// mirror: mat + mat_mult [same type]
template<typename T, unsigned h, unsigned w, typename U, class RType>
std::enable_if_t<std::is_same<T,U>::value, matrix_addition<T,h,w>>
operator + (const matrix_ref<U,RType>& lhs, matrix_product<T,h,w>&& rhs){
    static_assert(h*w*matrix_ref<U,RType>::H == 0 || (w==matrix_ref<U,RType>::W && h==matrix_ref<U,RType>::H),
                  "dimension mismatch in Matrix addition");
    if(lhs.get_width()!=rhs.get_height())
        throw std::domain_error("dimension mismatch in Matrix addition");
    matrix<T> right = rhs;
    matrix_addition<T,h,w> result;
    result.add(lhs);
    result.add(right);
    return result;
};

// mat_mult + stat [not same type]
template<typename T, unsigned h, unsigned w, typename U, class RType>
std::enable_if_t<!std::is_same<T,U>::value && h*w*matrix_ref<U,RType>::H!=0,
        matrix<typename op_traits<T,U>::sum_type,h,w>>
operator + (matrix_product<T,h,w>&& lhs, const matrix_ref<U,RType>& rhs){
    static_assert(h==matrix_ref<U,RType>::H && w==matrix_ref<U,RType>::W,
                  "dimension mismatch in Matrix addition");
    const unsigned height = lhs.get_height();
    const unsigned width = rhs.get_width();
    matrix<T,h,w> left = lhs;
    matrix<typename op_traits<T,U>::sum_type,h,w> result;
    for(unsigned i=0; i!=height; ++i)
        for(unsigned j=0; j!=width; ++j)
            result(i,j) = left(i,j) + rhs(i,j);
    return result;
};
// mirror: stat + mat_mult [not same type]
template<typename T, unsigned h, unsigned w, typename U, class RType>
std::enable_if_t<!std::is_same<T,U>::value && h*w*matrix_ref<U,RType>::H!=0,
        matrix<typename op_traits<T,U>::sum_type,h,w>>
operator + (const matrix_ref<U,RType>& lhs, matrix_product<T,h,w>&& rhs){
    static_assert(h==matrix_ref<U,RType>::H && w==matrix_ref<U,RType>::W,
                  "dimension mismatch in Matrix addition");
    const unsigned height = lhs.get_height();
    const unsigned width = rhs.get_width();
    matrix<T,h,w> right = rhs;
    matrix<typename op_traits<T,U>::sum_type,h,w> result;
    for(unsigned i=0; i!=height; ++i)
        for(unsigned j=0; j!=width; ++j)
            result(i,j) = lhs(i,j) + right(i,j);
    return result;
};

// mat_mult + dyno [not same type]
template<typename T, unsigned h, unsigned w, typename U, class RType>
std::enable_if_t<!std::is_same<T,U>::value && h*w*matrix_ref<U,RType>::H==0,
        matrix<typename op_traits<T,U>::sum_type>>
operator + (matrix_product<T,h,w>&& lhs, const matrix_ref<U,RType>& rhs){
    if(lhs.get_width()!=rhs.get_width() || lhs.get_height()!=rhs.get_height())
        throw std::domain_error("dimension mismatch in Matrix addition");
    const unsigned height = lhs.get_height();
    const unsigned width = rhs.get_width();
    matrix<T> left = lhs;
    matrix<typename op_traits<T,U>::sum_type> result(height,width);
    for(unsigned i=0; i!=height; ++i)
        for(unsigned j=0; j!=width; ++j)
            result(i,j) = left(i,j) + rhs(i,j);
    return result;
};
// mirror: dyno + mat_mult [not same type]
template<typename T, unsigned h, unsigned w, typename U, class RType>
std::enable_if_t<!std::is_same<T,U>::value && h*w*matrix_ref<U,RType>::H==0,
        matrix<typename op_traits<T,U>::sum_type>>
operator + (const matrix_ref<U,RType>& lhs, matrix_product<T,h,w>&& rhs){
    if(lhs.get_width()!=rhs.get_width() || lhs.get_height()!=rhs.get_height())
        throw std::domain_error("dimension mismatch in Matrix addition");
    const unsigned height = lhs.get_height();
    const unsigned width = rhs.get_width();
    matrix<T> right = rhs;
    matrix<typename op_traits<T,U>::sum_type> result(height,width);
    for(unsigned i=0; i!=height; ++i)
        for(unsigned j=0; j!=width; ++j)
            result(i,j) = lhs(i,j) + right(i,j);
    return result;
};


#endif // OPERATIONS_H 
