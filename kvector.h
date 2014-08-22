#ifndef KVECTOR_H_INCLUDED
#define KVECTOR_H_INCLUDED

#include <stdint.h>
#include <algorithm>
//!a semi complete substitute for the stl class vector
//!which allows easy clearing of the assigned memory of
//!a vector

template <class T>
class kvector
{
public: //check everything later {work on this first}
    kvector();
    kvector(uint64_t);
    kvector(uint64_t, T);
    kvector(const kvector &);
    ~kvector();
    void resize(uint64_t);
    void clear();
    T & operator [] (uint64_t);
    const T & operator [] (uint64_t) const;
    void push_back(T);
    void pop_back();
    uint64_t getsize() const;
    uint64_t getcapacity() const;
    bool empty() const;
    void clean();
    void swap(kvector<T> &);
    kvector<T> & operator= (const kvector<T> &);
    T & back();
    const T & back() const;
    T & front();
    const T & front() const;

private:
    T* data;
    static const uint64_t init_length;
    uint64_t length;//!capacity
    uint64_t current;//!size
};

template <class T>
const uint64_t kvector<T>::init_length=2;

template <class T>
kvector<T>::kvector()
{
    data = new T[init_length];
    length = init_length;
    current = 0;
}

//!this initializes the kvector to have a certain length
//!but does not assign certain values
template <class T>
kvector<T>::kvector(uint64_t _length)
{
    data = new T[_length];
    length = _length;
    current = 0;
}

//!this initializes the kvector to have a certain length
//!but does assign certain values
template <class T>
kvector<T>::kvector(uint64_t _length, T val)
{
    data = new T[_length];
    length = _length;
    for (uint64_t i=0; i<length; i++)
        data[i] = T(val);
    current = length;
}

template <class T>
kvector<T>::kvector(const kvector &input):length(input.length), current(input.current)
{
    data = new T[length];
    for (uint64_t i = 0; i<current; i++)
        data[i] = input.data[i];
}

template <class T>
T & kvector<T>::operator[](uint64_t position)
{
    //!does not check the position
    return data[position];
    //!does not automatically resize the array either.
}

template <class T>
const T & kvector<T>::operator[](uint64_t position) const
{
    //!does not check the position
    return data[position];
    //!does not automatically resize the array either.
}

template <class T>
void kvector<T>::resize(uint64_t size)
{
    T* newdata= new T[size];
    uint64_t minimum=length<size ? length : size;
    for (uint64_t i=0; i<minimum; i++ )
        newdata[i] = data[i];
    delete[] data;
    data = newdata;
    length = size;
    current = size; //note::this behavior matches exactly the behavior of std::vector
}

//!cleans the vector of all values
template <class T>
void kvector<T>::clean()
{
    T* newdata = new T[length];
    delete[] data;
    data = newdata;
    current = 0;
}

template <class T>
void kvector<T>::clear()
{
    this->resize(0);
}

template <class T>
kvector<T>::~kvector()
{
    delete [] data;
}

template <class T>
void kvector<T>::push_back(T val)
{
   if (current==length)
   {
        if(length==0)
            length = 2;
        else
            length *= 2;
        T* new_data = new T[length];//produce a new array
        for (int i=0; i<current; i++)//copy values from old array into new array
            new_data[i] = data[i];
        delete[] data; //delete old array
        data = new_data; //copy new array to old array;
   }
   data[current] = val;
   current += 1;
}

template <class T>
void kvector<T>::pop_back()
{
    //rather than deleting the actual value at data[current]
    //reduce the value of current by 1
    //so when running through the values stored in the vector
    //the user will never access the deleted value even though its still there
    if (current != 0)
        current -= 1;
    //if statement prevents current from going below 0 which would cause a crash because current is unsigned
}

template <class T>
uint64_t kvector<T>::getsize() const
{
    return current;
}

template <class T>
uint64_t kvector<T>::getcapacity() const
{
    return length;
}

template <class T>
bool kvector<T>::empty() const
{
    if (current != 0)
        return false;
    else
        return true;
}

template <class T>
void kvector<T>::swap(kvector<T> &in)
{
    std::swap(this->current, in.current);
    std::swap(this->length, in.length);
    std::swap(this->data, in.data);
}

template <class T>
kvector<T> & kvector<T>::operator= (const kvector<T> &input)
{//note cannot use copy and swap here due to the fact both speed and memory are important.
    current = input.current;
    length = input.length;
    delete[] data;
    data = new T[length];
    for (int i=0; i<current; i++)
        data[i] = input.data[i];
    return *this;
}

template <class T>
T & kvector<T>::back()
{
    if (current!=0)
        return data[current-1];
    else
        return data[0];
}

template <class T>
const T & kvector<T>::back() const
{
    if (current!=0)
        return data[current-1];
    else
        return data[0];
}

template <class T>
T & kvector<T>::front()
{
    return data[0];
}

template <class T>
const T & kvector<T>::front() const
{
    return data[0];
}
#endif // KVECTOR_H_INCLUDED
