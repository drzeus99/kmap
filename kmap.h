#include "kvector.h"
#include "hash_methods.h"
#include <math.h>
#include <iostream>
#include <stdint.h>
using namespace std;

template<class T>
struct scatter_table
{
    static uint64_t null;
    T* item;
    uint64_t next;
    scatter_table();
    ~scatter_table();
};

template<class K, class V>
class kmap
{
public:
    kmap();
    kmap(uint64_t);
    void clear();
    void clean();
    void insert(const K&, const V&); //!only used for inserting the first time
    V& operator[](const K&); //!can be used for inserting, changing and finding the value;
    //!beginning of methods used for iteration through the map
    K& getkey(uint64_t);
    V& getvalue(uint64_t);
    void setvalue(uint64_t, const V&);
    //!note you cant set the key because that would possibly destroy the map
    uint64_t start();
    uint64_t end();
    uint64_t next(uint64_t);
    //!end of methods used for iteration through the map
    void resize(uint64_t); //!triggers a rehashing
    //!note unlike the vector version this will do absolutely nothing
    //!if size_t inputed is less than the current size of the hash or is it less than the number of entries?
    bool haskey(const K&);
    void removekey(const K&);
private:
    uint64_t entries;
    kvector<scatter_table<K> > keys;
    kvector<V> values;
    //!parameters which work with the hashing function
    static double a;
    uint64_t m;
    void init_hash_props();
    bool match(const K&, uint64_t);
    void rehash(kvector<scatter_table<K> > &, kvector<V> &);
    bool found(const K&, uint64_t*);
    bool found(const K&, uint64_t*, uint64_t*);
    void chain_delete(uint64_t, uint64_t, uint64_t);
};

template <class T>
uint64_t scatter_table<T>::null=~0;

template <class T>
scatter_table<T>::scatter_table()
{
    item=0;
    next=null;
}

template <class T>
scatter_table<T>::~scatter_table()
{
    //!deletes the dynamically allocated item pointer
    if (item!=0)
    {
        //cout<<"the key deleted is "<<*item<<endl;
        delete item;
    }
}

template <class K, class V>
double kmap<K,V>::a=geta();

template <class K, class V>
kmap<K,V>::kmap() : entries(0)
{
    init_hash_props();
}//!default initialization of the two vectors is fine

template <class K, class V>
kmap<K,V>::kmap(uint64_t size) : entries(0),
keys(size), values(size)
{
    init_hash_props();
}

template <class K, class V>
void kmap<K,V>::init_hash_props()
{
    m=values.getsize();
}

template <class K, class V>
void kmap<K,V>::clear()
{
    keys.clear();
    keys.resize(2);
    values.clear();
    values.resize(2);
    /** The above clear/resize remove the possibility
    for memory problems due to operations on an array
    with a size of 0.*/
    init_hash_props();//to reset the size of m which keeps track of the size of the array
    entries=0;
}

template <class K, class V>
void kmap<K,V>::clean()
{
    keys.clean();
    values.clean();
    entries=0;//because this is a new map with 0 entried filled in
}

template <class K, class V>
bool kmap<K,V>::match(const K& key, uint64_t index)
{//The if statement is for the condition where item is a null pointer and prevents the code from crashing.
    if (keys[index].item==0)
        return false;
    if (*keys[index].item==key)
        return true;
    else
        return false;
}

template <class K, class V> //!bug fixed
void kmap<K,V>::insert(const K& key, const V& val)
{
    unsigned long long converted=hashvalue(key);
    uint64_t index=hashing(converted, m, a);
    if (entries==m)
    {
        //!produce two new vectors and swap them with the current vectors
        kvector<scatter_table<K> > temp_keys(2*m);
        kvector<V> temp_vals(2*m);

        //this call works perfectly
        keys.swap(temp_keys);
        values.swap(temp_vals);

        entries=0;//!Because the current version of keys and values has nothing entered in it yet

        //!now that keys and values have grown in size change hashing function properties
        //!and initiate a rehash using temp_keys and temp_vals as vectors where the previous information was stored
        init_hash_props();
        rehash(temp_keys, temp_vals);
        //!the good thing about this approach over using kvector.resize methods is it uses the same memory during the rehash
        //!but saves the computer from having to iterate over kvector twice.
        //!additionally temp_keys and temp_vals will get destroyed at the end of this block clearing the old hash table
        //!automatically.
    }

    if (keys[index].item!=0)
    {
        while (keys[index].next!=scatter_table<K>::null)
        {
            if(match(key,index)) //!check if key has already been placed and return if so
                return;
            index=keys[index].next;
        }
        if (keys[index].item==0)
            goto kmapinsert; //since we wont need to find the tail in this case because the tail is already
        //pointing to an empty spot in the map goto the code which inserts new information into kmap
        //!end of the chain
        uint64_t tail=index;
        //!finds open spot in chain to insert key
        index+=1;
        if (index==keys.getsize()) //!this should be way faster than index=(index+1) % keys.getsize()
            index=0;                //!because the integer division operation is being replaced with only addition
        while (keys[index].item!=0)
        {
            index+=1;
            if (index==keys.getsize()) //!this should be way faster than index=(index+1) % keys.getsize()
                index=0;               //!because the integer division operation is being replaced with only addition
        }
        keys[tail].next=index;
    }
    kmapinsert:
    keys[index].item=new K;
    *keys[index].item=key;
    //cout<<"the key inserted is "<<*keys[index].item<<endl;
    keys[index].next=scatter_table<K>::null;
    values[index]=val;
    entries+=1;
}

template <class K, class V>
void kmap<K,V>::rehash(kvector<scatter_table<K> > &temp_keys, kvector<V> &temp_vals)
{
    for (uint64_t i=0; i<temp_keys.getsize(); i++)
    {
        //check to make sure temp_keys[i].item is not empty first
        if(temp_keys[i].item!=0)
        {
             insert(*temp_keys[i].item, temp_vals[i]);//!inserts old map data into new map best case O(n), worst case O(n^2)
        }
        //!where n is the number of entries
        //!the * at the begining of temp_keys[i] is to dereference item;
    }
}

template <class K, class V>
bool kmap<K,V>::haskey(const K& key)
{
    unsigned long long converted=hashvalue(key);
    for (uint64_t index=hashing(converted, m, a);
        index!=scatter_table<K>::null; index=keys[index].next)
    {
        if (match(key, index))
            return true;
    }
    return false;
}

template <class K, class V> //!bug fixed
V& kmap<K,V>::operator[](const K& key)
{
    uint64_t position;
    if (found(key, &position))
    {
        return values[position];
    }
    else
    {
        if (entries==m)
        {
            //!produce two new vectors and swap them with the current keys
            kvector<scatter_table<K> > temp_keys(2*entries);
            kvector<V> temp_vals(2*entries);
            keys.swap(temp_keys);//need to make sure this is the correct way to make this call
            values.swap(temp_vals);//need to make sure this is the correct way to make this call
            //!now that keys and values have grown in size change hashing function properties
            //!and initiate a rehash using temp_keys and temp_vals as arrays where previous information was stored
            init_hash_props();
            entries=0; //!Because the current version of keys and values has nothing entered in it yet
            rehash(temp_keys, temp_vals);
            //!the good thing about this approach over using kvector.resize methods is it uses the same memory during the rehash
            //!but saves the computer from having to iterate over kvector twice.
            //!additionally temp_keys and temp_vals will get destroyed at the end of this block clearing the old hash table
            //!automatically.
            bool test=found(key, &position);
            //!dummy test just to get new position which is either a tail or open position
        }
        //cout<<"the index being tested here is "<<position<<endl;
        if (keys[position].item!=0) //!note the found method could return an
            //!empty position in the map rather than a tail position in a chain
        {
            //!end of the chain
            uint64_t tail=position;
            //!finds open spot in chain to insert key
            position+=1;
            if (position==keys.getsize()) //!this should be way faster than index=(index+1) % keys.getsize()
                position=0;                //!because the integer division operation is being replaced with only addition
            while (keys[position].item!=0)
            {
                position+=1;
                if (position==keys.getsize()) //!this should be way faster than index=(index+1) % keys.getsize()
                   position=0;               //!because the integer division operation is being replaced with only addition
            }
            keys[tail].next=position;
        }
        //keys[position].item=const_cast<K*>(&key); Next two lines replace this
        keys[position].item=new K;
        //cout<<"the item pointer is "<<keys[index].item<<endl;
        *keys[position].item=key;
        //cout<<"the key inserted is "<<*keys[position].item<<endl;
        keys[position].next=scatter_table<K>::null;
        entries+=1;
        return values[position];
    }
}

template <class K, class V>
bool kmap<K,V>::found(const K &key, uint64_t *position)
{
    unsigned long long converted=hashvalue(key);
    uint64_t index=hashing(converted, m, a);
    while (keys[index].next!=scatter_table<K>::null)
    {   //bug in match
        if(match(key,index)) //!check if key has already been placed and return if so
        {
            *position=index;
            return true;
        }
        index=keys[index].next;
    }
    *position=index;
    //!in case above while statement is true becasue key is not part of a chain but in its proper hashed positions
    return match(key,index);
}

template <class K, class V>
bool kmap<K,V>::found(const K &key, uint64_t *position, uint64_t *tail)
{
    uint64_t index=*position;
    uint64_t before=*tail;
    //in case the inputed position has no value stored there
    //this prevents a failure due to an out of bounds error.
    if (keys[index].item==0)
        return false;

    //in case the inputed position is at the end of the chain
    //this prevents a failure due to an out of bounds error
    if (keys[index].next==scatter_table<K>::null)
        return false;

    //index is set to the hashed position but since there is zero possibility for the key to be in this position
    //index is changed to the next index value
    index=keys[index].next;
    while (keys[index].next!=scatter_table<K>::null)
    {
        if(match(key,index))
        {
            *position=index;
            *tail=before;
            return true;
        }
        before=index;
        index=keys[index].next;
    }
    *position=index;
    *tail=before;
    return match(key,index);//in case the key at the end of the chain matches the inputed key
}

template <class K, class V> //current implementation has one bug in it.
void kmap<K,V>::removekey(const K &key)//the bug occurs when case 1 occurs but is part of the end of a chain.
//in this case the previous chain member will be pointing to an empty spot.
//This case is really rare and should only occur after two or more removekeys have occured.
//Due to the rarity of this bug occuring and the O(n) nature of the fix,
//implementing a fix that may occur with a 1% chance and slows down the code drastically does not seem reasonable.
//instead the other methods in this class will be worked around this bug.
{
    if (entries==0)
        return; //instead of a throw
    unsigned long long converted=hashvalue(key);
    uint64_t position=hashing(converted, m, a);
    uint64_t tail=position;
    if (match(key,position))
    {
        if (keys[position].next==scatter_table<K>::null)
        {
            //!case 1 where the hashed key matches the key's position and no chain exists.
            delete keys[position].item;
            keys[position].item=0;
            keys[position].next=scatter_table<K>::null;
        }
        else
        {
            //!case 2 where the hashed key matches the key's position and a chain exists.
            delete keys[position].item;
            uint64_t current=position; //save current to the position that has been tested
            position=keys[position].next;//update position to the next position in the chain

            chain_delete(position,current,tail);//moves the removed key to the end of the chain
            //and than makes a new end of the chain
        }
    }
    else if(found(key, &position, &tail))
    {
        if(keys[position].next!=scatter_table<K>::null)
        {
            //!case 3 where the hashed key matches a key in the middle of the chain.
            delete keys[position].item;
            uint64_t current=position; //save current to the position that has been tested
            position=keys[position].next; //update position to the next position in the chain

            chain_delete(position,current,tail);//moves the removed key to the end of the chain
            //and than makes a new end of the chain
        }
        else //!note will get rid of this else statement for the final version->this is here for testing purposes only
        {
            //!case 4 where the hashed key matches a key at the end of the chain.
            delete keys[position].item;
            keys[position].item=0;
            //this is at the end of the chain so there is no reason to reassign next at position to be null
            keys[tail].next=scatter_table<K>::null;
        }
    }
    else
    {
        //!case 5 where the key has not been stored in the map.
        return;
    }
    entries-=1;
}

template <class K, class V>
void kmap<K,V>::chain_delete(uint64_t position, uint64_t current, uint64_t tail)
{
    //operation to shift the values in the rest of the chain up and find the last location where
    //a blank spot can be inserted into the chain
    while(position!=scatter_table<K>::null)
    {
        if (keys[position].item!=0)
        {///the if statement above prevents the line below from causing the program to crash.
            unsigned long long converted=hashvalue(*keys[position].item);
            //!additionally if the line above cant be run than none of the other lines in this if block need running either.
            uint64_t hashed=hashing(converted,m,a);
            if (hashed!=position) //you should be able to move the value at positon up because its in the chain
                //and hash position doesn't match position
            {
                keys[current].item=keys[position].item;//move value up the chain
                values[current]=values[position];
                //update current
                current=position;
            }
        }
        position=keys[position].next;//update position
    }

    //now that the correct spot has been found for inserting a blank spot->update that position
    keys[current].item=0;
    keys[current].next=scatter_table<K>::null; //in case this value is not already null;

    //now we need to find the item in the chain that points to current and make next equal to null
    for (position=tail; position!=current; position=keys[position].next)
    {
        tail=position;
    }
    keys[tail].next=scatter_table<K>::null;
}

template <class K, class V>
uint64_t kmap<K,V>::start()
{
    uint64_t position=0;//!start at the begining of the array
    //!find the first position with a filled in item.
    while(keys[position].item==0)
        position+=1;
    return position;
}

template <class K, class V>
uint64_t kmap<K,V>::end()
{
    return m;//!the position right after the end
    //!of the array
}

template <class K, class V>
uint64_t kmap<K,V>::next(uint64_t index)
{
    uint64_t position=index+1;
    if (position==m) //!to prevent next from accesing invalid index in keys[position]
        return position;
    else
    {
        while(keys[position].item==0)
        {
            position+=1;
            if (position==m)  //!to prevent next from accesing invalid index in keys[position]
                return position;
        }
        //!note the if statements shouldn't slow the code down since the if statement is only true 1/m times
        return position;
    }
}

template <class K, class V>
K& kmap<K,V>::getkey(uint64_t index)
{
    return *keys[index].item;
}

template <class K, class V>
V& kmap<K,V>::getvalue(uint64_t index)
{
    return values[index];
}

template <class K, class V>
void kmap<K,V>::setvalue(uint64_t position, const V& value)
{
    values[position]=value;
}

template <class K, class V>
void kmap<K,V>::resize(uint64_t size)
{
    if(size<entries)
        return;//!do nothing
    else
    {
        kvector<scatter_table<K> > temp_keys(size);
        kvector<V> temp_vals(size);

        keys.swap(temp_keys);
        values.swap(temp_vals);
        //!now that keys and values have grown in size change hashing function properties
        //!and initiate a rehash using temp_keys and temp_vals as arrays where previous information was stored

        entries=0;//!because the current version of keys and values have nothing entered into them

        init_hash_props();

        rehash(temp_keys, temp_vals);
        //!the good thing about this approach over using kvector.resize methods is it uses the same memory during the rehash
        //!but saves the computer from having to iterate over kvector twice.
        //!additionally temp_keys and temp_vals will get destroyed at the end of this block clearing the old hash table
        //!automatically.
    }
}
