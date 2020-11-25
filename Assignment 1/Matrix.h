//
// Created by Fabio on 09/11/2017.
//

#include "Implementation.h"

using namespace std;

#ifndef THEAMATRICIANA_MATRIXREF_H
#define THEAMATRICIANA_MATRIXREF_H

template<class T>
class Matrix {
private:
    Imple<T>* pimpl_;   //Implementazione specifica per la matrice (Base, transpose, submatrix,...)
    
    explicit Matrix(Imple<T>* pimpl) {
        this->pimpl_ = pimpl;
        pimpl->pointedBy++;         //Aumento il contatore dei puntatori a pimpl_
    }
public:
    Matrix(unsigned int rows, unsigned int cols) {
        this->pimpl_ = new Base<T>(rows, cols); //Costruisco una nuova matrice da zero
        this->pimpl_->pointedBy++;        //Aumento il contatore dei puntatori a pimpl_
    }


    ~Matrix(){
        this->pimpl_->pointedBy--;    //riduco il numero di puntatori per l'impl
        if(this->pimpl_->pointedBy < 1) {   //Se pimpl_ non è più puntato da nessuno lo distruggo
            delete pimpl_;
        }
        this->pimpl_ = nullptr;   //previene la cancellazione del contenuto
    }


    /**
     * @param s_row riga di inizio (compresa)
     * @param s_col colonna di inizio (compresa)
     * @param e_row riga di fine (compresa)
     * @param e_col colonna di fine (compresa)
     * @return una nuova sottomatrice che condivide i dati con quella attuale
     */
    Matrix subMatrix(unsigned int s_row, unsigned int s_col, unsigned int e_row, unsigned int e_col) {
        try {
            return Matrix(new SubMatrix<T>(this->pimpl_, s_row, s_col, e_row, e_col));
        }catch(ImplementationException& e){
            cout << e.what();
            cout << "#Returned an empty Matrix#\n";
            return Matrix(new Base<T>(0, 0)); //TODO Se va in exception allora restituisco una matrice vuota
        }
    }

    /**
     * @return una nuova matrice trasposta che condivide i dati con quella attuale
     */
    Matrix transpose() {
        try {
            return Matrix(new Transpose<T>(this->pimpl_));
        }catch(ImplementationException& e){
            cout << e.what();
            cout << "#Returned an empty Matrix#\n";
            return Matrix(new Base<T>(0, 0));
        }
    }

    /**
     * @return un vettore contenente la diagonale della matrice attuale, condividendo i dati
     */
    Matrix diagonal() {
        try {
            return Matrix(new Diagonal<T>(this->pimpl_));
        }catch(ImplementationException& e){
            cout << e.what();
            cout << "#Returned an empty Matrix#\n";
            return Matrix(new Base<T>(0, 0));
        }
    }

    /**
     * pre-condizione: deve essere chiamata su un vettore, altrimenti lancia un exception
     * @return una nuova matrice completamente vuota tranne la diagonale che corrisponderà al vettore attuale
     */
    Matrix diagonalMatrix() {
        try {
            return Matrix(new DiagonalMatrix<T>(pimpl_));
        }catch(ImplementationException& e){
            cout << e.what();
            cout << "#Returned an empty Matrix#\n";
            return Matrix(new Base<T>(0, 0));
        }
    }

    /**
     * @param row riga dell'elemento da modificare
     * @param col colonna dell'elemento da modificare
     * @param value nuovo valore
     */
    void set_value(unsigned int row, unsigned int col, T value) {
        try{
            this->pimpl_->set_value(row, col, value);
        }catch(ImplementationException& e){
            cout << e.what();
        }
    }

    /**
     * Utilizzo:
     *      Matrix<T> a(4,4);
     *      T returnValue = a(2,2)
     * @param row riga del valore cercato
     * @param col colonna del valore cercato
     * @return valore nella matrice
     */
    T operator()(int row, int col) {
        try {
            return this->pimpl_->get(row, col);
        }catch(ImplementationException& e){
            cout << e.what();
        }
    }

    friend void swap(Matrix& first, Matrix& second){
        using std::swap;
        // by swapping the members of two objects,
        // the two objects are effectively swapped
        swap(first.pimpl_, second.pimpl_);
    }

    /**
     * Copy constructor (Matrix a = b)
     * @param a Matrix da copiare
     */
    Matrix(const Matrix &a){ // Copy-Constructor -> deep copy
        if(this != &a) {
            Imple<T> *pimpl_to_copy = a.pimpl_;
            Base<T> *new_matrix = new Base<T>(a.pimpl_->rows_, a.pimpl_->cols_);
            for(unsigned int i=0; i<a.pimpl_->rows_; i++)
                for(unsigned int j=0; j<a.pimpl_->cols_; j++)
                    new_matrix->set_value(i, j, pimpl_to_copy->get(i, j));
            this->pimpl_ = new_matrix;
        }
    }

    /**
     * E stato usato l'idioma copy-and-swap
     * @param a oggetto temporaneo da copiare
     * @return
     */
    Matrix& operator=(Matrix a){ // Copy-Assignment -> semplice riferimento
        // a-> temporaneo, quindi dopo lo swap non ci importa il suo contenuto.
        swap(*this, a);
        return *this;
    }

    T* get_ptr(unsigned int row, unsigned int col){
        try {
            return this->pimpl_->get_ptr(row, col);
        }catch(ImplementationException& e){
            cout << e.what();
        }
    }

    /**
     * @return righe della matrice
     */
    int getRows() {
        return pimpl_->rows_;
    }

    /**
     * @return colonne della matrice
     */
    int getCols() {
        return pimpl_->cols_;
    }

    /**
     * Utilizza un iteratore per stampare la matrice
     */
    void print_matrix(){
        int x = 0;
        for(Matrix<T>::IterRow iter = get_row_iterator(); iter != iter.end(); ++iter){
            if(x%getCols() == 0)    cout<<"\n";
            cout << *iter << "\t";
            x++;
        }
        cout << "\n";
    }


    class IterRow {
    private:
        Matrix<T> *ref_;
        T *element_;
        int position_;
    public:
        explicit IterRow(Matrix<T> *ref, T* element) {
            ref_ = ref;
            position_ = 0;
            element_ = element;
        }

        T &operator*() { return *element_; }

        bool operator==(const IterRow &other) {
            return (element_ == other.element_ && ref_ == other.ref_);
        }

        bool operator!=(const IterRow &other) {
            return (element_ != other.element_ || ref_ != other.ref_);
        }

        IterRow &operator++() {
            position_++;    //Aumenta la posizione attuale
            if (position_ > (ref_->getRows() * ref_->getCols()-1) ) { //Se la posizione è oltre la capienza della matrice
                element_ = (T*) nullptr;    //Ho finito la matrice, torno nullptr
            }
            else {
                int row = 0, col = 0;   //se posizion = 0 allora la posizione sarà (0,0)
                if (position_ != 0) {   //Per tutte le altre posizioni, calcolo in base al numero di colonne
                    row = position_ / ref_->getCols();
                    col = position_ % ref_->getCols();
                }
                try {
                    element_ = ref_->get_ptr(row, col); //salvo l'elemento attuale
                } catch (const std::out_of_range &oor) {
                    cerr << "OUT OF RANGE (dereferencing)";
                    element_ = (T*) nullptr;
                }
            }
        }

        IterRow end(){
            return IterRow(ref_, (T*) nullptr);
        }
    };

    class IterReverseRow {
    private:
        Matrix<T> * ref_;
        T *element_;
        int position;
    public:
        explicit IterReverseRow(Matrix<T> * ref, T* element){
            ref_ = ref;
            position = (ref_->getRows() * ref_->getCols())-1;    //Ultimo elemento
            element_ = element;
        }

        T &operator*() { return *element_; }

        bool operator==(const IterReverseRow &other) {
            return (element_ == other.element_ && ref_ == other.ref_);
        }

        bool operator!=(const IterReverseRow &other) {
            return (element_ != other.element_ || ref_ != other.ref_);
        }

        IterReverseRow &operator++() {
            //cout << "Sto scorrendo l'iteratore, posizione: " << position << "\n";
            position--;
            if (position < 0)  //Se la posizione è prima del primo elemento
                element_ = (T*) nullptr;
            else {
                int row = 0, col = 0;
                if( position != 0) {
                    row = position / ref_->getCols();
                    col = position % ref_->getCols();
                }
                try {
                    *element_ = ref_->get_ptr(row, col);
                } catch (const std::out_of_range &oor) {
                    element_ = (T*) nullptr;
                }
            }
        }

        IterReverseRow end(){
            return IterReverseRow(ref_, (T*) nullptr);
        }
    };

    class IterCol {
    private:
        Matrix<T> *ref_;
        T* element_;
        int position;
    public:
        explicit IterCol(Matrix<T> *ref, T* element) {
            ref_ = ref;
            position = 0;
            element_ = element;
        }

        T &operator*() { return *element_; }

        bool operator==(const IterCol &other) {
            return (element_ == other.element_ && ref_ == other.ref_);
        }

        bool operator!=(const IterCol &other) {
            return (element_ != other.element_ || ref_ != other.ref_);
        }

        IterCol &operator++() {
            position++;
            if (position > (ref_->getRows() * ref_->getCols())-1 )  //Se la posizione è oltre la capienza della matrice
                element_ = (T*) nullptr;
            else {
                int row = 0, col = 0;
                if(position != 0){
                    row = position / ref_->getRows();
                    col = position % ref_->getRows();
                }
                try {
                    element_ = ref_->get_ptr(col, row);
                } catch (const std::out_of_range &oor) {
                    cerr << "OUT OF RANGE (dereferencing";
                    element_ = (T*) nullptr;
                }
            }
        }

        IterCol end(){
            return IterCol(ref_, (T*) nullptr);
        }
    };

    class IterReverseCol {
        private:
            Matrix<T> *ref_;
            T* element_;
            int position;
        public:
            explicit IterReverseCol(Matrix<T> *ref, T* element) {
                ref_ = ref;
                position = (ref_->getRows() * ref_->getCols()-1 );    //Ultimo elemento
                element_ = element;
            }

            T &operator*() { return *element_; }

            bool operator==(const IterReverseCol &other) {
                return (element_ == other.element_ && ref_ == other.ref_);
            }

            bool operator!=(const IterReverseCol &other) {
                return (element_ != other.element_ || ref_ != other.ref_);
            }

            IterReverseCol &operator++() {
                position--;
                if (position < 0)  //Se la posizione è prima del primo elemento
                    element_ = (T*) nullptr;
                else {
                    int row = 0, col = 0;
                    if(position != 0){
                        row = position / ref_->getRows();
                        col = position % ref_->getRows();
                    }
                    try {
                        element_ = ref_->get_ptr(col, row);
                    } catch (const std::out_of_range &oor) {
                        element_ = (T*) nullptr;
                    }
                }
            }

            IterReverseCol end(){
                return IterReverseCol(ref_, (T*) nullptr);
            }
        };

    IterRow get_row_iterator(){
        if(pimpl_->rows_ == 0 && pimpl_->cols_ == 0)    //Matrice vuota, torno subito end()
            return IterRow(this, (T*) nullptr);
        return IterRow(this, pimpl_->get_ptr(0,0)); //primo elemento
    }

    IterReverseRow get_reverse_row_iterator(){
        if(pimpl_->rows_ == 0 && pimpl_->cols_ == 0)    //Matrice vuota, torno subito end()
            return IterReverseRow(this, (T*) nullptr);
        unsigned int r = pimpl_->rows_ -1;
        unsigned int c = pimpl_-> cols_ -1;
        return IterReverseRow(this, pimpl_->get_ptr(r, c));
    }

    IterCol get_col_iterator(){
        if(pimpl_->rows_ == 0 && pimpl_->cols_ == 0)    //Matrice vuota, torno subito end()
            return IterCol(this, (T*) nullptr);
        return IterCol(this, pimpl_->get_ptr(0,0));
    }

    IterReverseCol get_reverse_col_iterator(){
        if(pimpl_->rows_ == 0 && pimpl_->cols_ == 0)    //Matrice vuota, torno subito end()
            return IterReverseCol(this, (T*) nullptr);
        unsigned int r = pimpl_->rows_ -1;
        unsigned int c = pimpl_-> cols_ -1;
        return IterReverseCol(this, pimpl_->get_ptr(r, c));
    }

};

#endif //THEAMATRICIANA_MATRIXREF_H
