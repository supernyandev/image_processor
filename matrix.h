
#ifndef CPP_HSE_MATRIX_H
#define CPP_HSE_MATRIX_H
#include <cstddef>
#include <stdexcept>
#include <vector>
template <typename T>
class TMatrix {
public:
    // ожидаемые (стандартные) для типов данных производнных от T

    using ValueType = T;
    using Pointer = T*;
    using ConstPointer = const T*;
    using Reference = ValueType&;

    // Представляет абстракцию над строчкой матрицы,
    // самодостаточную и компактную для копирования по значению
    class Row {
        friend class TMatrix<T>;

    public:
        T& operator[](size_t colnum) const {
            return matrix_->operator()(rownum_, colnum);
        }

        size_t Size() const {
            //            return matrix_->cols_num_;
            return matrix_->GetCollsNum();
        }

    private:
        // Разрешаем доступ к конструктору только матрице
        Row(TMatrix<T>* m, size_t rownum) : rownum_(rownum), matrix_(m) {
        }

    private:
        size_t rownum_;
        TMatrix<T>* matrix_;
    };

    class ConstRow {
        friend class TMatrix<T>;

    public:
        /*const int&*/ T operator[](size_t colnum) const {
            return matrix_->operator()(rownum_, colnum);
        }

        size_t Size() const {
            return matrix_->cols_num_;
        }

    private:
        // Разрешаем доступ к конструктору только матрице
        ConstRow(const TMatrix<T>* m, size_t rownum) : rownum_(rownum), matrix_(m) {
        }

    private:
        size_t rownum_;
        const TMatrix<T>* matrix_;
    };

    class Col {
        friend class Matrix;

    public:
        T& operator[](size_t rownum) const {
            return matrix_->operator()(rownum, colnum_);
        }

        size_t Size() const {
            return matrix_->rows_num_;
        }

    private:
        // Разрешаем доступ к конструктору только матрице
        Col(TMatrix<T>* m, size_t colnum) : colnum_(colnum), matrix_(m) {
        }

    private:
        size_t colnum_;
        TMatrix<T>* matrix_;
    };

    class ConstCol {
        friend class TMatrix<T>;

    public:
        /*const int&*/ T operator[](size_t rownum) const {
            return matrix_->operator()(rownum, colnum_);
        }

        size_t Size() const {
            return matrix_->rows_num_;
        }

    private:
        // Разрешаем доступ к конструктору только матрице
        ConstCol(const TMatrix<T>* m, size_t colnum) : colnum_(colnum), matrix_(m) {
        }

    private:
        size_t colnum_;
        const TMatrix<T>* matrix_;
    };

public:
    // TODO: исправить)
    TMatrix(std::initializer_list<std::initializer_list<T>> lists) {
        uint32_t cnt_rows = 0;
        for (const auto& list : lists) {
            cnt_rows++;
            uint32_t cnt_colls = 0;
            for (const T& item : list) {
                cnt_colls++;
                // иначе не пропускает потому что item не использован
                T temp = item;
                T temp1 = temp;
                temp = temp1;
            }
            this->cols_num_ = cnt_colls;
        }
        this->rows_num_ = cnt_rows;
        table_ = this->AllocateMatrix(rows_num_, cols_num_);
        uint32_t i = 0;
        for (const auto& list : lists) {
            uint32_t j = 0;
            for (const T& item : list) {
                (*this)(i, j) = item;
                ++j;
            }
            ++i;
        }
    }

    TMatrix(const TMatrix<T>& other) {
        table_ = AllocateMatrix(other.rows_num_, other.cols_num_);
        rows_num_ = other.rows_num_;
        cols_num_ = other.cols_num_;
        for (size_t i = 0; i < other.rows_num_; ++i) {
            for (size_t j = 0; j < other.cols_num_; ++j) {
                (*this)(i, j) = other(i, j);
            }
        }
    };
    static void Swap(TMatrix<T>& lhv, TMatrix<T>& rhv) {
        std::swap(lhv.cols_num_, rhv.cols_num_);
        std::swap(lhv.rows_num_, rhv.rows_num_);
        std::swap(lhv.table_, rhv.table_);
    }
    TMatrix<T>& operator=(const TMatrix<T>& other) {
        TMatrix<T> temp(other);
        Swap(*this, temp);
        return *this;
    }

    explicit TMatrix(size_t rows_num = 0, size_t cols_num = 0, T def = T{})  // теперь T default constractable
    {
        if (rows_num == 0 && cols_num == 0) {
            rows_num_ = 0;
            cols_num_ = 0;
            table_ = nullptr;
            return;
        }

        if (rows_num == 0 || cols_num == 0) {
            throw std::invalid_argument("Can't make matrix with 0 rows or columns");
        }
        rows_num_ = rows_num;
        cols_num_ = cols_num;
        table_ = AllocateMatrix(rows_num_, cols_num_);

        // Нежизненно:
        for (size_t i = 0; i < rows_num; ++i) {
            for (size_t j = 0; j < cols_num; ++j) {
                this->operator()(i, j) = def;
            }
        }
    }

    ~TMatrix() {
        Clear();
    }

    // Возвращает i,j -й элемент матрицы
    T operator()(size_t i, size_t j) const {
        //        return table_[i * cols_num_ + j];
        return GetIJEl(table_, i, j, cols_num_);
    }

    T& operator()(size_t i, size_t j) {
        //        return table_[i * cols_num_ + j];
        return GetIJEl(table_, i, j, cols_num_);
    }

    // Возвращает i-ю строку, обёрнутую в абстракцию
    Row operator[](size_t i) {
        //        return Row(this, i);
        /*typename*/ TMatrix<T>::Row ri(this, i);
        return ri;
    }

    ConstRow operator[](size_t i) const {
        return ConstRow(this, i);
    };

    T At(size_t i, size_t j) const  // на самом деле долно было подставляться наиболее подходяшее: const & или значение
    {
        if (i < rows_num_ && j < cols_num_) {
            return this->operator()(i, j);
        }
        throw std::out_of_range("IOJ (matrix).");
    }

    // T& At(size_t i, size_t j);
    Reference At(size_t i, size_t j) {
        if (i < rows_num_ && j < cols_num_) {
            return this->operator()(i, j);
        }
        throw std::out_of_range("IOJ (matrix).");
    }

public:
    void Resize(size_t new_col_num, size_t new_row_num, T def = T{}) {
        if (new_col_num == 0 && new_row_num == 0) {
            Clear();
            return;
        }
        if (new_row_num == 0 || new_col_num == 0) {
            throw std::invalid_argument("Can't make matrix with 0 rows or columns");
        }

        T* new_table = AllocateMatrix(new_row_num, new_col_num);
        for (size_t i = 0; i < new_row_num; ++i) {
            for (size_t j = 0; j < new_col_num; ++j) {
                if (i < rows_num_ && j < cols_num_) {
                    GetIJEl(new_table, i, j, new_col_num) = GetIJEl(table_, i, j, cols_num_);
                } else {
                    GetIJEl(new_table, i, j, new_col_num) = def;
                }
            }
        }
        delete[] table_;
        table_ = new_table;
        rows_num_ = new_row_num;
        cols_num_ = new_col_num;
    }

    void Clear() {
        if (!table_) {
            return;
        }
        delete[] table_;
        rows_num_ = 0;
        cols_num_ = 0;
    }

    size_t GetRowsNum() const {
        return rows_num_;
    }

    size_t GetCollsNum() const {
        return cols_num_;
    }

    // Неконстантная перегрузка позволяет изменять подлежащую память
    T* GetPtr() {
        return table_;
    }

    // Константная перегрузка позволяет только читать подлежащую память
    const T* GetPtr() const {
        return table_;
    }

    // help-ерная функция, которая по 4 параметрам позволяет получить правильное место эл-та 5-ого параметра
    // static T& GetIJEl(T* arr, size_t i, size_t j, size_t cols_num){
    static Reference GetIJEl(T* arr, size_t i, size_t j, size_t cols_num) {
        return arr[i * cols_num + j];
    }

private:
    // Метод аллоцирует память в кучу для заданных параметров n и m
    // Методу от this ничего не надо, поэтому он статический
    static T* AllocateMatrix(size_t rows_n, size_t cols_m) {
        T* ans = new T[rows_n * cols_m];
        return ans;
    }

private:
    //    T* table_;
    Pointer table_;  // читается тяжелее чем T*, но так написано в стандарте
    size_t rows_num_;
    size_t cols_num_;
};

#endif  // CPP_HSE_MATRIX_H
