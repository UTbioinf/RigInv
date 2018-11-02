#ifndef __UTIL_MULTI_ARRAY_H
#define __UTIL_MULTI_ARRAY_H

#include <iostream>
#include <vector>
#include <stdexcept>
#include <cmath>

namespace loon
{

/*! \ingroup Class_util
 * @{
 */

/*! \brief A generic dynamic 2-D Array
 *
 * In this implementation, the assumption is that the number of columns in a
 * 2-D array is fixed, but the number of rows can grow dynamically. So before
 * adding anything in the array, the user should call set_column() to set the 
 * number of columns. And it's the users responsibility to call resize to grow 
 * the rows.
 *
 * Notice:
 *
 * The member function call resize() will not clear the data. So it's the users
 * responsibility to clear all the data when they use it.
 *
 * \todo Add member function assign() so as to clear the data elegantly.
 * \todo Implement high dimensional array
 */
template<class T>
class Array2D: public std::vector<T>
{
private:
    size_t column;
    size_t row;

public:
    /*! \brief Constructor
     *
     * \param [in] n Set the initial size of the container, must be no smaller than `r * c`
     * \param [in] c Set the number of columns in the 2-D array
     * \param [in] r Set the number of rows in the 2-D array
     *
     * Notice:
     * 
     * If `n < c * r`, an `out_of_range` exception will be thrown. 
     */
    Array2D(size_t n = 0, size_t c=0, size_t r = 0);

    /*! \brief Set column size
     * 
     * Usually, the column of the array should be fixed. And the number of lines will grow
     * dynamically. So before adding new elements, one should fix the number of columns.
     *
     * \param [in] c Set the number of columns of the 2-D array
     */
    void set_column(size_t c);

    /*! \brief Resize rows
     * 
     * Once the number of the columns is set, the user should call resize() to get space 
     * for storing elements.
     * 
     * \param [in] row Set the number of rows.
     */
    void resize(size_t row);

    /*! \brief Resize both rows and columns
     *
     * This function will set both the number of rows and columns. It's a short form of
     * calling set_column() and then resize()
     *
     * \param [in] row Set the number of rows
     * \param [in] col Set the number of columns
     */
    void resize(size_t row, size_t col); // this will also change the column of the 2-D array

    /*! \brief Resize column
     * 
     * Calling this member function will lead to a one-row array. The number of columns will
     * be set to `c`. (Weird!!! May be reimplemented later).
     *
     * \param [in] c Number of columns in the new 2-D one-row array
     */
    void resize_column(size_t c);

    /*!\brief Add one row to the 2-D array*/
    void add_row();

    /*!\brief Fetch the element located at `(i, j)`*/
    T& at(size_t i, size_t j);

    /*!\brief Fetch the constant element located at `(i, j)`*/
    const T& at(size_t i, size_t j) const;

    /*!\brief Clear the 2-D array*/
    void clear();

    /*!\brief Get the number of rows in the 2-D array*/
    size_t get_row() const;
};

/*! @}*/

template<class T>
Array2D<T>::Array2D(size_t n /*=0*/, size_t c /*=0*/, size_t r /*=0*/): 
        column(c), row(r), 
        std::vector<T>::vector(n)
{
    if(n < r * c)
        throw std::out_of_range("Array2D_dimension_check");
}

template<class T>
void Array2D<T>::set_column(size_t c)
{
    column = c;
}

template<class T>
void Array2D<T>::resize(size_t row)
{
    this->row = row;
    if(row * column > this->size())
        std::vector<T>::resize( std::max(row * column, this->size() << 1) );
}

template<class T>
void Array2D<T>::resize(size_t row, size_t col)
{
    column = col;
    this->row = row;
    if(row * col > this->size())
        std::vector<T>::resize( std::max(row * col, this->size() << 1) );
}

template<class T>
void Array2D<T>::resize_column(size_t c)
{
    row = 1;
    column = c;
    if(column > this->size())
        std::vector<T>::resize( std::max(column, this->size() << 1) );
}

template<class T>
void Array2D<T>::add_row()
{
    ++row;
    if(row * column > this->size())
        std::vector<T>::resize( std::max(row * column, this->size() << 1) );
}

template<class T>
T& Array2D<T>::at(size_t i, size_t j)
{
    if(row <= i || column <= j)
        throw std::out_of_range("Array2D_range_check");
    return std::vector<T>::at( i * column + j);
}

template<class T>
const T& Array2D<T>::at(size_t i, size_t j) const
{
    if(row <= i || column <= j)
        throw std::out_of_range("Array2D_range_check");
    return std::vector<T>::at( i * column + j);
}

template<class T>
void Array2D<T>::clear()
{
    column = row = 0;
    std::vector<T>::clear();
}

template<class T>
size_t Array2D<T>::get_row() const
{
    return row;
}


/*! \ingroup Class_util
 * @{
 */

/*! \brief A generic dynamic 2-D Square Array
 * 
 * This data structure is useful when implementing a complete graph, or a complete
 * bipartite graph. In particular, if a lot of such graphs will be created, the
 * dynamic property is an advantage over the commonly used 2-D array.
 *
 * This 2-D Square Array will automatically increase its size if one tries to
 * access a cell that is not within the domain of `n x n`, where `n` is the side
 * length of the square.
 *
 */
template<class T>
class Square2D: public std::vector<T>
{
private:
    size_t n;
    T default_value;
public:
    /*!\brief Default constructor.
     *
     * This will create a 2-D square array of size 0x0
     */
    Square2D();

    /*!\brief Constructor
     *
     * Create a 2-D square array of size 0x0
     *
     * \param [in] reserve Reserve the size of the 2-D array
     * \param [in] def_val Set the default value that will fill the empty cell in the square
     **/
    Square2D(size_t reserve, const T& def_val = T());

    /*!\brief Set default value
     * 
     * Set the default value that will fill the empty cell in the square
     *
     * \param [in] def_val The default value that will be used to fill the empty cell in the square
     */
    void set_default(const T& def_val);

    /*!\brief Clear the square
     * 
     * Clear all the contents in the square and set the dimension of the square to 0.
     * 
     * Notice:
     *
     * This implementation relies on the underlying implementation of std::vector. Specifically,
     * even if clear() is called, the max size of the vector is still unchanged. In newer C++,
     * this may not be the case any more. So in the future, a more adaptive implementation of this
     * reset() function is required.
     *
     * \todo A more adaptive implementation of the reset() function.
     */
    void reset();

    /*!\brief Fetch the element located at `(i, j)`
     *
     * This member function will increase the size of the square array if either `i` or `j` is larger
     * than the side length of the square.
     */
    T& at(size_t i, size_t j);

    /*!\brief Fetch the element located at `(i, j)`
     *
     * Fetching an element out of range will lead to an `std::out_of_range` exception.
     **/
    const T& at(size_t i, size_t j) const;

    /*!\brief Set the side length of the square
     *
     * The underlying size of the square array will increase automatically if `nn` is larger than the
     * side length of the square.
     *
     * \param [in] nn The side length of the new square
     */
    void setn(size_t nn);

    /*!\brief Get the side length of the 2-D square array.*/
    size_t getn() const;
};

/*! @}*/

template<class T>
Square2D<T>::Square2D(): n(0), default_value(0)
{
}

template<class T>
Square2D<T>::Square2D(size_t reserve, const T& def_val/* = T() */):
        n(0), default_value(def_val), std::vector<T>::vector(reserve, def_val)
{
}

template<class T>
void Square2D<T>::set_default(const T& def_val)
{
    default_value = def_val;
}

template<class T>
void Square2D<T>::reset()
{
    n = 0;
    clear();
}

template<class T>
T& Square2D<T>::at(size_t i, size_t j)
{
    if(i < j)
    {
        if(n <= j)
        {
            size_t old_cap = n * n;
            n = j + 1;
            size_t new_cap = n * n;
            if(std::vector<T>::size() < new_cap)
                std::vector<T>::resize( std::max(std::vector<T>::size() << 2, new_cap));
            for(size_t ii = old_cap; ii < new_cap; ++ii)
                std::vector<T>::at(ii) = default_value;
        }
        return std::vector<T>::at( j*j + i );
    }
    else
    {
        if(n <= i)
        {
            size_t old_cap = n * n;
            n = i + 1;
            size_t new_cap = n * n;
            if(std::vector<T>::size() < new_cap)
                std::vector<T>::resize( std::max(std::vector<T>::size() << 2, new_cap) );
            for(size_t ii = old_cap; ii < new_cap; ++ii)
                std::vector<T>::at(ii) = default_value;
        }
        return std::vector<T>::at( i*i + i + j );
    }
}

template<class T>
const T& Square2D<T>::at(size_t i, size_t j) const
{
    if(n <= std::max(i, j))
        throw std::out_of_range("Square2D_range_check");
    if(i < j)
        return std::vector<T>::at( j*j + i);
    else
        return std::vector<T>::at( i*i + i + j );
}

template<class T>
void Square2D<T>::setn(size_t nn)
{
    size_t new_cap = nn * nn;
    if(n < nn)
    {
        size_t old_cap = n * n;
        if(std::vector<T>::size() < new_cap)
            std::vector<T>::resize( std::max(std::vector<T>::size() << 2, new_cap) );
    }
    std::vector<T>::assign(new_cap, default_value);
    n = nn;
}

template<class T>
size_t Square2D<T>::getn() const
{
    return n;
}

}
#endif
