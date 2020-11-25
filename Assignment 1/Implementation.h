//
// Created by felix on 08/11/17.
//

#ifndef THEAMATRICIANA_IMPLEMENTATION_H
#define THEAMATRICIANA_IMPLEMENTATION_H

#include <iostream>
#include <memory>
#include "stdexcept"

using namespace std;

template <class T>
class Imple{
public:
    virtual void set_value(unsigned int row, unsigned int col, T value) = 0;
    virtual T get(unsigned int row, unsigned int col) = 0;
    virtual T* get_ptr(unsigned int row, unsigned int col) = 0;
    virtual ~Imple(){};
    unsigned int rows_, cols_;
    int pointedBy = 0;  //svolge una funzione simile agli shared_ptr
};

class ImplementationException : public runtime_error{
public:
    ImplementationException(string msg) : runtime_error("#Exception: "+msg+"#"){};
};

template <class T>
class Base : public Imple<T>{
private:
    T **data_;
    void init(){
        data_ = new T *[this->rows_];
        for(int i=0; i<this->rows_; i++) {
            data_[i] = new T [this->cols_];
            for (int j = 0; j < this->cols_; j++)
                data_[i][j] = T {};
        }
    }

    /**
     * @param row riga da controllare
     * @param col colonna da controllare
     * @return true se row e col sono all'interno della matrice, false altrimenti
     */
    bool check_index(int row, int col){
        return (row >= 0 && row < this->rows_) && (col >= 0 && col < this->cols_);
    }
public:
    explicit Base(unsigned int rows, unsigned int cols){
        this->rows_ = rows; //Di base le righe sono tutte, eventuali modifiche cambieranno questo valore
        this->cols_ = cols; //e le colonne sono tutte, eventuali modifiche cambieranno questo valore
        init();
    }

    ~Base(){
        //Se nessuno punta più la base, cancello tutti i dati
        if(this->pointedBy == 0) {
            for (int w = 0; w < this->rows_; w++)
                delete[] data_[w];
            delete[] data_;
        }
    }

    /**
     * @param row riga dell'elemento da modificare
     * @param col colonna dell'elemento da modificare
     * @param value Nuovo valore da assegnare all'elemento in posizione (row, col)
     */
    void set_value(unsigned int row, unsigned int col, T value){
        if(!check_index(row, col))
            throw ImplementationException("[Base] Wrong index on setValue");
        data_[row][col] = value;
    }

    /**
     * @param row riga dell'elemento cercato
     * @param col colonna dell'elemento cercato
     * @return elemento cercato
     */
    T get(unsigned int row, unsigned int col){
        if (!check_index(row, col))
            throw ImplementationException("[Base] Wrong index");
        else
            return data_[row][col];
    }

    T* get_ptr(unsigned int row, unsigned int col){
        if (!check_index(row, col))
            throw ImplementationException("[Base] Wrong Index");
        else
            return &data_[row][col];
    }
};

template <class T>
class Decorator : public Imple<T>{
public:
    Imple<T> *father;

    explicit Decorator(Imple<T> *i){
        this->father = i;
        this->rows_ = father->rows_;
        this->cols_ = father->cols_;
        this->father->pointedBy++;
    }

    virtual ~Decorator(){}

    void set_value(unsigned int row, unsigned int col, T value){
        this->father->set_value(row, col, value);
    }
};

template <class T>
class SubMatrix : public Decorator<T>{
private:
    unsigned int s_row_, s_col_, e_row_, e_col_;
    bool check_coords(int row, int col){
        return ((row+s_row_ <= e_row_) && (col+s_col_ <= e_col_));
    }
public:
    SubMatrix(Imple<T> *i, unsigned int s_row, unsigned int s_col,
              unsigned int e_row, unsigned int e_col) : Decorator<T>(i){
        this->father = i;
        if(!(s_row <= e_row &&
             s_col <= e_col &&
             e_row <= this->father->rows_ &&
             e_col <= this->father->cols_))
            throw ImplementationException("Wrong index provided to build SubMatrix");

        s_row_ = s_row;
        s_col_ = s_col;
        e_row_ = (e_row < this->rows_) ? e_row : this->rows_-1;
        e_col_ = (e_col < this->cols_) ? e_col : this->cols_-1;

        this->rows_ = e_row_ - s_row_ +1;   //Calcolo le righe effettive
        this->cols_ = e_col_ - s_col_ +1;   //Calcolo le colonne effettive
    }

    ~SubMatrix(){
        if(this->pointedBy < 1) {
            this->father->pointedBy--;
            if(this->father->pointedBy == 0)
                delete this->father;
            this->father = nullptr; //impedisce la distruzione del padre
        }
    }

    void set_value(unsigned int row, unsigned int col, T value){
        if(!check_coords(row, col))
            throw ImplementationException("[SubMatrix] Wrong index");
        this->father->set_value(row+s_row_, col+s_col_, value);
    }

    T get(unsigned int row, unsigned int col){
        if(!check_coords(row, col))
            throw ImplementationException("[SubMatrix] Wrong index");
        return this->father->get(row+s_row_, col+s_col_);
    }

    T* get_ptr(unsigned int row, unsigned int col){
        if(!check_coords(row, col))
            throw ImplementationException("[SubMatrix] Wrong index");
        return this->father->get_ptr(row+s_row_, col+s_col_);
    }
};

template <class T>
class Transpose : public Decorator<T>{
private:
    bool check_coords(unsigned int row, unsigned int col){
        return (row < this->rows_ && col < this->cols_);
    }
public:
    explicit Transpose(Imple<T> *i) : Decorator<T>(i){
        this->father = i;
        unsigned int temp = i->cols_;
        this->cols_ = i->rows_;
        this->rows_ = temp;
    }

    ~Transpose(){
        if(this->pointedBy < 1) {
            this->father->pointedBy--;
            if(this->father->pointedBy == 0)
                delete this->father;
            this->father = nullptr; //impedisce distruzione del padre
        }
    }

    void set_value(unsigned int row, unsigned int col, T value){
        if(!check_coords(row, col))
            throw ImplementationException("[Transpose] Wrong index");
        this->father->set_value(col, row, value);
    }

    T get(unsigned int row, unsigned int col){
        if(!check_coords(row, col))
            throw ImplementationException("[Transpose] Wrong index");
        return this->father->get(col, row);
    }

    T* get_ptr(unsigned int row, unsigned int col){
        if(!check_coords(row, col))
            throw ImplementationException("[Transpose] Wrong index");
        return this->father->get_ptr(col, row);
    }
};

template <class T>
class Diagonal : public Decorator<T>{
private:
    bool check_index(unsigned int row, unsigned int col){
        return (row < this->rows_ && col < this->cols_);
    }
public:
    explicit Diagonal(Imple<T> *i) : Decorator<T>(i){
        this->father = i;
        this->cols_ = 1;
        this->rows_ = min(this->father->rows_, this->father->cols_);
    }

    ~Diagonal(){
        if(this->pointedBy < 1) {
            this->father->pointedBy--;
            if(this->father->pointedBy == 0)
                delete this->father;
            this->father = nullptr; //impedisce distruzione del padre
        }
    }

    void set_value(unsigned int row, unsigned int col, T value){
        if(!check_index(row, col))
            throw ImplementationException("[Diagonal] Wrong index");
        unsigned int val = max(row, col);
        this->father->set_value(val, val, value);
    }

    T get(unsigned int row, unsigned int col){
        if(!check_index(row, col)){
            throw ImplementationException("[Diagonal] Wrong index");
        }else {
            unsigned int val = max(row, col);
            return this->father->get(val, val);
        }
    }

    T* get_ptr(unsigned int row, unsigned int col){
        if(!check_index(row, col)){
            throw ImplementationException("[Diagonal] Wrong index");
        }else {
            unsigned int val = max(row, col);
            return this->father->get_ptr(val, val);
        }
    }
};

template <class T>
class DiagonalMatrix : public Decorator<T>{
private:
    bool check_index(unsigned int row, unsigned int col){
        return (row < this->rows_ && col < this->cols_);
    }
public:
    explicit DiagonalMatrix(Imple<T> *i) : Decorator<T>(i){
        if(!(i->rows_ == 1 || i->cols_ == 1))
            throw ImplementationException("Trying to build a DiagonalMatrix through a non-vector object");
        unsigned int val = max(i->cols_, i->rows_);
        Base<T> *fabio = new Base<T>(val, val);
        this->cols_ = val;
        this->rows_ = val;
        if(i->rows_ == 1) {
            for (unsigned int k = 0; k < val; k++)
                fabio->set_value(k, k, i->get(0, k));
        }
        if(i->cols_ == 1) {
            for (unsigned int k = 0; k < val; k++)
                fabio->set_value(k, k, i->get(k, 0));
        }
        this->father = fabio;
    }

    ~DiagonalMatrix(){
        if(this->pointedBy < 1) {
            this->father->pointedBy--;
            if(this->father->pointedBy == 0)
                delete this->father;
            this->father = nullptr; //prevent father destruction
        }
    }

    void set_value(unsigned int row, unsigned int col, T value){
        throw ImplementationException("[DiagonalMatrix] Object is unmodifiable!");
    }

    T get(unsigned int row, unsigned int col){
        if(!check_index(row, col))
            throw ImplementationException("[DiagonalMatrix] Wrong index");
        return this->father->get(row, col);
    }

    T* get_ptr(unsigned int row, unsigned int col){
        if(!check_index(row, col))
            throw ImplementationException("[DiagonalMatrix] Wrong index");
        return this->father->get_ptr(row, col);
    }
};


#endif //THEAMATRICIANA_IMPLEMENTATION_H
