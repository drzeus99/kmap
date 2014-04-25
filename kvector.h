#ifndef KVECTOR_H_INCLUDED
#define KVECTOR_H_INCLUDED
//!not planning on using this for inheritance
#include <stdint.h>

template <class T>
class kvector
{
public:
    kvector();
    kvector(uint64_t);
    kvector(uint64_t, T);
    ~kvector();
    void resize(uint64_t);
    void clear();
    T & operator [] (uint64_t);
    void push_back(T);
    uint64_t getsize();
    void clean();
    void swap(kvector<T> &);
    //!friend class kvector is not neccesary because its already friends with itself.
private:
    T* data;
    static uint64_t init_length;
    uint64_t length;
    uint64_t current;
};

template <class T>
uint64_t kvector<T>::init_length=2;

template <class T>
kvector<T>::kvector()
{
    data=new T[init_length];
    length=init_length;
    current=0;
}

//!this initializes the kvector to have a certain length
//!but does not assign certain values
template <class T>
kvector<T>::kvector(uint64_t _length)
{
    data=new T[_length];
    length=_length;
    current=0;
}

//!this initializes the kvector to have a certain length
//!but does assign certain values
template <class T>
kvector<T>::kvector(uint64_t _length, T val)
{
    data=new T[_length];
    length=_length;
    for (size_t i=0; i<length; i++)
        data[i]=T(val);
    current=length;
}

template <class T>
T & kvector<T>::operator[](uint64_t position)
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
        newdata[i]=data[i];
    delete [] data;
    data=newdata;
    length=size;
}

template <class T>
void kvector<T>::clear()
{
    this->resize(0);
    current=0;
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
            this->resize(2);
        else
            this->resize(length*2);
   }
   data[current]=val;
   current+=1;
}

template <class T>
uint64_t kvector<T>::getsize()
{
    return length;
}

//!cleans the vector of all values
template <class T>
void kvector<T>::clean()
{
    T* newdata= new T[length];
    delete [] data;
    data=newdata;
}

template <class T>
void kvector<T>::swap(kvector<T> &in)
{
    kvector<T> temp(in);//!should work as a copy constructor; note:the pointer will also be copied as well
    //!use of *this should be correct in the next two lines
    in=*this;
    *this=temp;
    temp.data=0; //!sets the pointer to 0 so when temp is deleted at the end of this method
    //!the returned vectors are not accessing unassigned memory.
}
#endif // KVECTOR_H_INCLUDED
