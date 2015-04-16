#ifndef KMAP_H_INCLUDED
#define KMAP_H_INCLUDED

#include "kvector.h"
#include "hash_methods.h"
#include <math.h>
#include <iostream>
#include <stdint.h>
#include <map>
#include <vector>
#include <stdexcept>
#include <utility>

//need to build an interator for kvector as well.
//than transfer next code into kmap_iterator
//make begin set up the internal iterator object in kmap
//and return the internal object
//change kmap methods to work with internal object
//yay lots of work
//need to defriend kmap_iterator and kmap yay

template<class K, class V>
class kmap;//forward declaration

template<class K, class V>
class kmap_iterator
{
private:
	kvector_iterator<std::map<K,V> > index;
	typename std::map<K,V>::iterator it;
	kvector_iterator<std::map<K,V> > end;
    friend class kmap<K,V>;
    std::map<K,V>& map();
public:
    kmap_iterator() {};
    kmap_iterator(kvector_iterator<std::map<K,V> >);
    kmap_iterator(kvector_iterator<std::map<K,V> >, typename std::map<K,V>::iterator, kvector_iterator<std::map<K,V> >);
	bool operator!=(const kmap_iterator&);
	std::pair<const K, V> operator*();
	typename std::map<K,V>::iterator& operator->();
    kmap_iterator& operator++();
    kmap_iterator operator++(int);
    bool in_map();
};

template<class K, class V>
kmap_iterator<K,V>::kmap_iterator(kvector_iterator<std::map<K,V> > index)
{
    this->index = index;
}

template<class K, class V>
kmap_iterator<K,V>::kmap_iterator(kvector_iterator<std::map<K,V> > index, typename std::map<K,V>::iterator it, kvector_iterator<std::map<K,V> > end)
{
    this->index = index;
    this->it = it;
    this->end = end;
}

template<class K, class V>
bool kmap_iterator<K,V>::operator!=(const kmap_iterator<K,V>& test)
{
	return this->index != test.index;
}

template<class K, class V>
std::map<K,V>& kmap_iterator<K,V>::map()
{
    return *index;
}

template<class K, class V>
std::pair<const K, V> kmap_iterator<K,V>::operator*()
{
    return *it;
}

template<class K, class V>
typename std::map<K,V>::iterator& kmap_iterator<K,V>::operator->()
{
    return it;
}

template<class K, class V>
kmap_iterator<K,V>& kmap_iterator<K,V>::operator++()
{
    ++it;
    if (it != map().end()) {
        return *this;
    }
    else {
		//finding the correct index for key_position
        ++index;
        if (index == end)
            return *this;
        while(map().empty()) {
			++index;
            if (index == end)
				return *this;
        }
		//found the correct index not at end of kvector
		//updating key_position.it
        it = map().begin();
        return *this;
    }
}

template<class K, class V>
kmap_iterator<K,V> kmap_iterator<K,V>::operator++(int)
{
    kmap_iterator<K,V> result(*this);
    ++*this;
    return result;
}

template<class K, class V>
bool kmap_iterator<K,V>::in_map()
{
    return it != map().end();
}

template<class K, class V>
class kmap //!this class is not designed to be used as a constant reference none of the methods will work correctly
{
public:
    kmap();
    kmap(uint64_t);
    kmap(const kmap&);
    kmap<K,V>& operator=(const kmap&);
    kmap(kmap&&);
    kmap<K,V>& operator=(kmap&&);

    //!beginning of methods used to manage kmap [inserting keys, clearing/cleaning, resizing, and swapping]
    void clear();
    void clean();
    void insert(const K&, const V&);
    V& operator[](const K&); //!can be used for inserting, changing and finding the value;
    void resize(uint64_t); //!triggers a rehashing
    //!note unlike the vector version this will do absolutely nothing
    //!if the inputed size is less than the current capacity of the vector
    void swap (kmap &);
    //!end of methods used to manage kmap

    //!begining of methods used for finding, getting and setting keys/values
    //!note you cant set the key because that would possibly destroy the map
    kmap_iterator<K,V> find(const K&);
    void remove(const K&);
    //!end of methods used for finding, getting and setting keys/values

    //!beginning of methods used for iteration through the map
    kmap_iterator<K,V> begin();
    kmap_iterator<K,V> end();
    uint64_t hash_size();
    std::map<K,V>& batch(uint64_t);
    //!end of methods used for iteration through the map

    bool empty();
    uint64_t entry_number();
    //!parameter which controls maximum number of entries
    static const uint64_t map_size;//estimated maximum in each std::map
    //!note:this is public in case the user needs to resize the map based on the size of the required vector
    //!take the size of the vector (which can be gotten from using the hash_size method) then
    //!multiply by kmap<T>::map_size
    //!if sizing the map to the number of entries use entry number instead
private:
	uint64_t entries;
    kvector<std::map<K,V> > values;
    //!parameter which controls maximum number of entries
    uint64_t kmap_size;//absolute maximum in kmap before rehash
    //!parameters which work with the hashing function
    static const double a;
    uint64_t m; //capacity of the vector
    void init_hash_props();
    void rehash(uint64_t);
};

//defining static variables
template <class K, class V>
const double kmap<K,V>::a=geta();

template <class K, class V>
const uint64_t kmap<K,V>::map_size=64;

template <class K, class V>
kmap<K,V>::kmap() : entries(0) //default constructor
{
    init_hash_props();
}//!default initialization of values and it

template <class K, class V>
kmap<K,V>::kmap(uint64_t size) : entries(0),
    values(ceil(double(size)/map_size)) //constructor which presizes kmap
{
    //need to make sure the size of value and it is greater than or equal to 2
    //a size below that value makes hashing useless
    //std::cout<<"size is"<<size<<std::endl;
    if (values.getcapacity() < 2)
    {
        values.resize(2);
    }
    init_hash_props();
}

template <class K, class V>
kmap<K,V>::kmap(const kmap<K,V>& input) : entries(input.entries), values(input.values),
     kmap_size(input.kmap_size), m(input.m)

{//The copy constructor for the vector only works correctly when size and capacity are the same.
    //If they are not the same, the objects in the vector do not get copied over.
    //Therefore, they need to be manually copied when capacity and size are not the same.
    //!additional note: because of how kmap works the steps below will never ever repeat
    //!the steps in the vector copy constructor.
    if (values.getsize() != values.getcapacity())
    {
        for (int i=0; i<values.getcapacity(); i++)
        {
            values[i]=input.values[i];
        }
    }
}

template <class K, class V>
kmap<K,V>& kmap<K,V>::operator=(const kmap<K,V>& input)
{
    this->entries = input.entries;
    this->values = input.values; //assignment operator for the vector
    this->kmap_size = input.kmap_size;
    this->m = input.m;
    //The assignment operator for the vector only works correctly when size and capacity are the same.
    //If they are not the same, the objects in the vector do not get copied over.
    //Therefore, they need to be manually copied when capacity and size are not the same.
    //!additional note: because of how kmap works the steps below will never ever repeat
    //!the steps in the vector assignment operator.
    if (values.getsize() != values.getcapacity())
    {
        for (int i=0; i<values.getcapacity(); i++)
        {
            values[i]=input.values[i];
        }
    }
    return *this;
}

template <class K, class V>
kmap<K,V>::kmap(kmap<K,V>&& input): entries(std::move(input.entries)), values(std::move(input.values)),
     kmap_size(std::move(input.kmap_size)), m(std::move(input.m))
{

}

template <class K, class V>
kmap<K,V>& kmap<K,V>::operator=(kmap<K,V>&& input)
{
    entries = std::move(input.entries);
    values = std::move(input.values);
    kmap_size = std::move(input.kmap_size);
    m = std::move(input.m);
    return *this;
}

template <class K, class V>
void kmap<K,V>::init_hash_props()
{
    m=values.getcapacity();//number of slots that hashing function can place values in
    kmap_size=m*map_size;//number of total slots that kmap can assign before growing and rehashing
}

//!removes all of the data from kmap and returns the data structures to their default size of 2
template <class K, class V>
void kmap<K,V>::clear()
{
    values.clear();
    values.resize(2);
    /** The above clear/resize remove the possibility
    for memory problems due to operations on an array
    with a size of 0.*/
    init_hash_props();//to reset the size of m which keeps track of the size of the array
    entries=0;
}

//!removes all of the data from kmap but keeps the data structures the same size
template <class K, class V>
void kmap<K,V>::clean()
{
    values.clean();
    entries=0;//because this is a new map with 0 entries filled in
}

//!inserts the key and value into the map
template <class K, class V> //!bug fixed
void kmap<K,V>::insert(const K& key, const V& val)
{
    if (entries==kmap_size)
    {
        values.resize(2*m);

        uint64_t old_m=m;
        init_hash_props();//find new m and new kmap_size
        rehash(old_m);
    }
    //! do hashing after the rehash check, otherwise the key-value combination would get inserted into a location that will never be searched
    unsigned long long converted=hashvalue(key);
    uint64_t index=hashing(converted, m, a);

    //!insert key-value combination in std::map located at index and update entries by 1
    values[index][key]=val;
    //std::cout<<"the key is "<<key<<" and the value is "<<values[index][key]<<std::endl; //testing if insert actually works
    entries+=1;
}

//!rehashes values using new value for m. moves key-value pairs that hash to a different section of the hash table and than deletes them from their
//!old position
template <class K, class V>
void kmap<K,V>::rehash(uint64_t old_m)
{
    typename std::map<K,V>::iterator hash_it;
    std::vector<K> deleted_keys; //this stores the keys that have been moved to a different map so they can all be deleted at the same time
    unsigned long long converted;
    uint64_t index;

    for (uint64_t i=0; i<old_m; i++) //!O(N) to go through all data in the maps, O(Log(N)) for each delete and O(log(N)) for each insert
    {
        //! check to make sure std::map at index i is not empty if so go to next index
        if (values[i].empty())
            continue;
        //!otherwise iterate through map
        for (hash_it=values[i].begin(); hash_it!=values[i].end(); hash_it++)
        {
            converted=hashvalue(hash_it->first);
            index=hashing(converted, m, a);
            if(index != i) {
                //move key and value to map with that index and append key to deleted_keys
                values[index][hash_it->first]=hash_it->second;
                deleted_keys.push_back(hash_it->first);
            }
        }
        //!test if any keys need deleting from the map at index i than delete the keys and clear the vector deleted_keys
        if(!deleted_keys.empty()) {
            for (size_t j=0; j < deleted_keys.size(); j++) {
                values[i].erase(deleted_keys[j]);
            }
            deleted_keys.clear();
        }
    }
}

//!Returns the index the key would be hashed at. If the key exists in this position sets the iterator at this position to point to the key
//!Otherwise the iterator will point to the end
//!This function is useful if you want to use getkey, getvalue or setvalue methods without iterating through the entire kmap
template <class K, class V>
kmap_iterator<K,V> kmap<K,V>::find(const K& key)
{
    unsigned long long converted=hashvalue(key);
    uint64_t index = hashing(converted, m, a);
    kvector_iterator<std::map<K, V> > vector_index(&values[index]);
    typename std::map<K,V>::iterator it = values[index].find(key);
    kmap_iterator<K,V> key_position(vector_index, it, values.end());
    return key_position;
}

//!this is different than insert
//!if the key exists in kmap then the value assigned to that key will be returned.
//!otherwise the key will be inserted before returning a value
//!note this algorithm will be slightly slower for inserting key value pairs than insert method
template <class K, class V>
V& kmap<K,V>::operator[](const K& key)
{
    kmap_iterator<K,V> key_position = find(key);
    //The method for checking if key exists below is faster than using the getvalue method and a try catch block.
    //since the try catch block will cause the code to slow down every time the key does not exist in kmap and the key needs inserting.
    if (key_position.in_map())
    {
        return key_position->second;
    }
    else
    {
        if (entries==kmap_size)
        {
            values.resize(2*m);

            uint64_t old_m = m;
            init_hash_props();//find new m
            rehash(old_m);

            //!Because the map was rehashed the index needs to be calculated again.
            unsigned long long converted = hashvalue(key);
            uint64_t vector_index = hashing(converted, m, a);
            key_position.index = &values[vector_index];
        }

        //!update entries by 1 and insert key in std::map located at index and return the reference to its mapped value
        entries+=1;
        return key_position.map()[key];
    }
}

template <class K, class V>
void kmap<K,V>::remove(const K &key)
{
    if (entries!=0) { //note this is the more common condition to occur so it should go first to decrease code branching.
        unsigned long long converted=hashvalue(key);
        uint64_t position=hashing(converted, m, a);
        if (values[position].erase(key) != 0)//this returns the number of keys erased. If the key doesn't exist no changes will occur to the map at values[position]
        //and also no errors will be thrown.
            entries-=1;
    }
    //if there are no entries this method does absolutely nothing.
}

//!finds the starting point for iteration through kmap
template <class K, class V>
kmap_iterator<K,V> kmap<K,V>::begin()
{//need to change algorithm to work with kmap_iterator
    kmap_iterator<K,V> key_position;
    if (entries!=0) { //note this is the more common condition to occur so it should go first to decrease code branching.
        uint64_t index=0;//!start at the begining of values
        //!find the first index of values in which the map contains key-value pairs
        while(values[index].empty()) {
            index+=1;
            if (index == m) {
                return end();
            }
        }
        key_position.it = values[index].begin();
        key_position.index = &values[index];
    }
    else
        key_position.index = values.end(); //!the position right after the end of values

    key_position.end = values.end();
    return key_position;
}

//!returns the ending point for iteration through kmap
template <class K, class V>
kmap_iterator<K,V> kmap<K,V>::end()
{
    return kmap_iterator<K,V>(values.end());//!the position right after the end of values
}

template <class K, class V>
uint64_t kmap<K,V>::hash_size()
{
    return m;//the size of the hash table
}

//returns the map which is inside the vector or hash table at the position index
template <class K, class V>
std::map<K,V>& kmap<K,V>::batch(uint64_t index)
{
    return values[index];
}

//!resizes kmap but only if the inputed size is greater than the current kmap_size
template <class K, class V>
void kmap<K,V>::resize(uint64_t size)
{
    if(size < kmap_size)
        return;//!do nothing
    else
    {
        values.resize(ceil(double(size) / map_size));

        uint64_t old_m=m;
        init_hash_props();//find new m and new kmap_size
        rehash(old_m);
    }
}

template <class K, class V>
void kmap<K,V>::swap(kmap<K,V> &other)
{
    std::swap(this->entries, other.entries);
    std::swap(this->kmap_size, other.kmap_size);
    std::swap(this->m, other.m);
    values.swap(other.values);
}

template <class K, class V>
bool kmap<K,V>::empty()
{
    if (entries != 0) //note: this is the more common condition so it's first
        return false;
    else
        return true;
}

template <class K, class V>
uint64_t kmap<K,V>::entry_number()
{
    return entries;
}
#endif //KMAP_H_INCLUDED
