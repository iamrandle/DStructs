
#ifndef LINKED_SET_HPP_
#define LINKED_SET_HPP_

#include <string>
#include <iostream>
#include <sstream>
#include <initializer_list>
#include "ics_exceptions.hpp"


namespace ics {


template<class T> class LinkedSet {
  public:
    //Destructor/Constructors
    ~LinkedSet();

    LinkedSet          ();
    explicit LinkedSet (int initialLength);
    LinkedSet          (const LinkedSet<T>& to_copy);
    explicit LinkedSet (const std::initializer_list<T>& il);

    //Iterable class must support "for-each" loop: .begin()/.end() and prefix ++ on returned result
    template <class Iterable>
    explicit LinkedSet (const Iterable& i);


    //Queries
    bool empty      () const;
    int  size       () const;
    bool contains   (const T& element) const;
    std::string str () const; //supplies useful debugging information; contrast to operator <<

    //Iterable class must support "for-each" loop: .begin()/.end() and prefix ++ on returned result
    template <class Iterable>
    bool contains_all (const Iterable& i) const;


    //Commands
    int  insert (const T& element);
    int  erase  (const T& element);
    void clear  ();

    //Iterable class must support "for" loop: .begin()/.end() and prefix ++ on returned result

    template <class Iterable>
    int insert_all(const Iterable& i);

    template <class Iterable>
    int erase_all(const Iterable& i);

    template<class Iterable>
    int retain_all(const Iterable& i);


    //Operators
    LinkedSet<T>& operator = (const LinkedSet<T>& rhs);
    bool operator == (const LinkedSet<T>& rhs) const;
    bool operator != (const LinkedSet<T>& rhs) const;
    bool operator <= (const LinkedSet<T>& rhs) const;
    bool operator <  (const LinkedSet<T>& rhs) const;
    bool operator >= (const LinkedSet<T>& rhs) const;
    bool operator >  (const LinkedSet<T>& rhs) const;

    template<class T2>
    friend std::ostream& operator << (std::ostream& outs, const LinkedSet<T2>& s);



  private:
    class LN;

  public:
    class Iterator {
      public:
        //Private constructor called in begin/end, which are friends of LinkedSet<T>
        ~Iterator();
        T           erase();
        std::string str  () const;
        LinkedSet<T>::Iterator& operator ++ ();
        LinkedSet<T>::Iterator  operator ++ (int);
        bool operator == (const LinkedSet<T>::Iterator& rhs) const;
        bool operator != (const LinkedSet<T>::Iterator& rhs) const;
        T& operator *  () const;
        T* operator -> () const;
        friend std::ostream& operator << (std::ostream& outs, const LinkedSet<T>::Iterator& i) {
          outs << i.str(); //Use the same meaning as the debugging .str() method
          return outs;
        }
        friend Iterator LinkedSet<T>::begin () const;
        friend Iterator LinkedSet<T>::end   () const;

      private:
        //If can_erase is false, current indexes the "next" value (must ++ to reach it)
        LN*           current;  //if can_erase is false, this value is unusable
        LinkedSet<T>* ref_set;
        int           expected_mod_count;
        bool          can_erase = true;

        //Called in friends begin/end
        Iterator(LinkedSet<T>* iterate_over, LN* initial);
    };


    Iterator begin () const;
    Iterator end   () const;


  private:
    class LN {
      public:
        LN ()                      {}
        LN (const LN& ln)          : value(ln.value), next(ln.next){}
        LN (T v,  LN* n = nullptr) : value(v), next(n){}

        T   value;
        LN* next   = nullptr;
    };


    LN* front     = new LN();
    LN* trailer   = front;         //Always point to special trailer LN
    int used      =  0;            //Cache the number of values in linked list
    int mod_count = 0;             //For sensing concurrent modification

    //Helper methods
    int  erase_at   (LN* p);
    void delete_list(LN*& front);  //Deallocate all LNs (but trailer), and set front's argument to trailer;
};





////////////////////////////////////////////////////////////////////////////////
//
//LinkedSet class and related definitions

//Destructor/Constructors

template<class T>
LinkedSet<T>::~LinkedSet() {
    this->delete_list(this->front);
}


template<class T>
LinkedSet<T>::LinkedSet() {
}


template<class T>
LinkedSet<T>::LinkedSet(const LinkedSet<T>& to_copy) : used(to_copy.used) {
    for(auto val : to_copy)
    {
        this->trailer->value = val;
        this->trailer = this->trailer->next = new LN();
    }
    this->mod_count++;
}


template<class T>
LinkedSet<T>::LinkedSet(const std::initializer_list<T>& il) {
    for (auto elem : il)
        this->insert(elem);
}


template<class T>
template<class Iterable>
LinkedSet<T>::LinkedSet(const Iterable& i) {
    this->insert_all(i);
}


////////////////////////////////////////////////////////////////////////////////
//
//Queries

template<class T>
bool LinkedSet<T>::empty() const {
    return this->used < 1;
}


template<class T>
int LinkedSet<T>::size() const {
    return this->used;
}



template<class T>
bool LinkedSet<T>::contains (const T& element) const {
    for(auto i = this->begin(); i != this->end(); i++)
    {
        if(element == *i)
            return true;
    }
    return false;
}


template<class T>
std::string LinkedSet<T>::str() const {
    std::ostringstream answer;
    answer << "LinkedSet[";

    if (this->used != 0) {
        //auto j = this->begin();
        int j = 0;
        for (auto i = this->begin(); i != this->end(); ++i, j++) {
            answer << *i; //<< ":" << *j;
            answer << (j == this->used - 1 ? "TRAILER]" : "->");
        }
    }

    answer << "(length=" << this->used << ",front=" << this->front << ",rear=" << this->trailer << ",mod_count="
           << this->mod_count << ")";
    return answer.str();
}


template<class T>
template<class Iterable>
bool LinkedSet<T>::contains_all (const Iterable& i) const {
    for (const T& v : i)
        if (!contains(v))
            return false;

    return true;
}


////////////////////////////////////////////////////////////////////////////////
//
//Commands


template<class T>
int LinkedSet<T>::insert(const T& element) {
    if (!this->contains(element))
    {
        this->trailer->value = element;
        this->trailer = this->trailer->next = new LN();
        this->used++;
        this->mod_count++;
        return 1;
    }
    else
        return 0;
}


template<class T>
int LinkedSet<T>::erase(const T& element) {
    if(!this->contains(element))
        return 0;
    else
    {
        LN* p = this->front;
        while(p->value != element)
        {
            p=p->next;
        }
        return this->erase_at(p);
    }
}


template<class T>
void LinkedSet<T>::clear() {
    this->delete_list(this->front);
    this->mod_count++;
    this->front = this->trailer = new LN();
    this->used++;
}


template<class T>
template<class Iterable>
int LinkedSet<T>::insert_all(const Iterable& i) {
    int count = 0;

    for (const T &v : i)
        count += insert(v);

    return count;
}


template<class T>
template<class Iterable>
int LinkedSet<T>::erase_all(const Iterable& i) {
    int count = 0;
    for(auto val : i)
    {
        this->erase(val);
        count++;
    }
    return count;
}


template<class T>
template<class Iterable>
int LinkedSet<T>::retain_all(const Iterable& i) {
    LinkedSet s(i);
    int count = 0;
    for(auto j = this->begin(); j != this->end(); j++)
        if(!s.contains(*j))
        {
            j.erase();
            count++;
        }
    return count;

}


////////////////////////////////////////////////////////////////////////////////
//
//Operators

template<class T>
LinkedSet<T>& LinkedSet<T>::operator = (const LinkedSet<T>& rhs) {
    if (!this->front)
    {
        for (const T &v : rhs)
        {
            this->trailer->value = v;
            this->trailer = this->trailer->next = new LN();
        }
    }
    else if (this->used < rhs.used)
    {

        auto j = rhs.begin();
        for(auto i = this->begin(); i!=this->end(); i++, j++)
        {
            *i = *j;
        }
        for(j; j!= rhs.end(); j++)
        {
            this->trailer->value = *j;
            this->trailer = this->trailer->next = new LN();
            this->used++;
        }
    }
    else
    {
        auto i = this->begin();
        for(auto j = rhs.begin(); j != rhs.end(); j++, i++)
        {
            *i = *j;
        }
        for(i;i!=this->end();i++)
        {
            i.erase();
        }
    }
    this->mod_count++;
    return *this;
}


template<class T>
bool LinkedSet<T>::operator == (const LinkedSet<T>& rhs) const {
    if (this == &rhs)
        return true;

    if (this->used != rhs.size())
        return false;

    //LinkedSet<T>::Iterator rhs_i = rhs.begin();
    for (auto val : *this) { // Uses ! and ==, so != on T need not be defined
        if (!rhs.contains(val))
            return false;
    }
    return true;

}


template<class T>
bool LinkedSet<T>::operator != (const LinkedSet<T>& rhs) const {
    return !(*this == rhs);
}


template<class T>
bool LinkedSet<T>::operator <= (const LinkedSet<T>& rhs) const {
    return (*this < rhs || *this == rhs);
}


template<class T>
bool LinkedSet<T>::operator < (const LinkedSet<T>& rhs) const {
    if (this == &rhs)
        return false;

    if (this->used >= rhs.size())
        return false;

    for (auto val : *this)
        if (!rhs.contains(val))
            return false;

    return true;
}


template<class T>
bool LinkedSet<T>::operator >= (const LinkedSet<T>& rhs) const {
    return (*this > rhs || *this == rhs);
}


template<class T>
bool LinkedSet<T>::operator > (const LinkedSet<T>& rhs) const {
    return rhs < *this;
}


template<class T>
std::ostream& operator << (std::ostream& outs, const LinkedSet<T>& s) {
    outs << "set[";

    auto p = s.front;
    for (int i = 0; i < s.used; i++) {
        if (i == 0)
            outs << p->value;
        else
            outs << "," << p->value;
        if (p->next)
            p = p->next;
    }

    outs << "]";
    return outs;
}


////////////////////////////////////////////////////////////////////////////////
//
//Iterator constructors

template<class T>
auto LinkedSet<T>::begin () const -> LinkedSet<T>::Iterator{
    return Iterator(const_cast<LinkedSet<T> *>(this), this->front);
}


template<class T>
auto LinkedSet<T>::end () const -> LinkedSet<T>::Iterator {
    return Iterator(const_cast<LinkedSet<T> *>(this), this->trailer);
}


////////////////////////////////////////////////////////////////////////////////
//
//Private helper methods

template<class T>
int LinkedSet<T>::erase_at(LN* p) {
    if(p->next != this->trailer)
    {
        LN* del = p->next;
        p->value = del->value;
        p->next = del->next;
        delete del;
        this->mod_count++;
        this->used--;

        return 1;
    }
    else if(p->next == this->trailer)
    {
        LN* del = this->trailer;
        this->trailer = p;
        this->trailer->next = nullptr;
        delete del;
        this->mod_count++;
        this->used--;

        return 1;
    }
    else
        return 0;
}


template<class T>
void LinkedSet<T>::delete_list(LN*& front) {
    if (front == nullptr)
        return;
    else {
        this->delete_list(front->next);
        delete front;
        this->used--;
        return;
    }
}





////////////////////////////////////////////////////////////////////////////////
//
//Iterator class definitions

template<class T>
LinkedSet<T>::Iterator::Iterator(LinkedSet<T>* iterate_over, LN* initial)
:current(initial), ref_set(iterate_over), expected_mod_count(ref_set->mod_count)
{

}


template<class T>
LinkedSet<T>::Iterator::~Iterator()
{
    current = nullptr;
    delete current;
    ref_set = nullptr;
    delete ref_set;

}


template<class T>
T LinkedSet<T>::Iterator::erase() {
    if (expected_mod_count != ref_set->mod_count)
        throw ConcurrentModificationError("LinkedSet::Iterator::erase");
    if (!can_erase)
    {
        throw CannotEraseError("LinkedSet::Iterator::erase Iterator cursor already erased");
    }
    if (current == ref_set->trailer)
    {
        throw CannotEraseError("LinkedSet::Iterator::erase Iterator cursor beyond data structure");
    }
    can_erase = false;
    T toret = current->value;
    ref_set->erase(current->value);
    expected_mod_count = ref_set->mod_count;
    return toret;

}


template<class T>
std::string LinkedSet<T>::Iterator::str() const {
    std::ostringstream answer;
    answer << ref_set->str() << "(current=" << current << ",expected_mod_count=" << expected_mod_count
           << ",can_erase=" << can_erase << ")";
    return answer.str();
}


template<class T>
auto LinkedSet<T>::Iterator::operator ++ () -> LinkedSet<T>::Iterator& {

    if (expected_mod_count != ref_set->mod_count)
        throw ConcurrentModificationError("LinkedSet::Iterator::operator ++");
    if (!current || current == ref_set->trailer)
        return *this;
    if (can_erase) {
        current = current->next;
    } else
        can_erase = true;
    return *this;

}


template<class T>
auto LinkedSet<T>::Iterator::operator ++ (int) -> LinkedSet<T>::Iterator {
    if (expected_mod_count != ref_set->mod_count)
        throw ConcurrentModificationError("LinkedSet::Iterator::operator ++(int)");

    if (!current || current == ref_set->trailer)
        return *this;

    Iterator to_return(*this);
    if (can_erase) {
        current = current->next;
    } else
        can_erase = true;  //current already indexes "one beyond" deleted value

    return to_return;
}


template<class T>
bool LinkedSet<T>::Iterator::operator == (const LinkedSet<T>::Iterator& rhs) const {
    const Iterator *rhsASI = dynamic_cast<const Iterator *>(&rhs);
    if (rhsASI == 0)
        throw IteratorTypeError("LinkedSet::Iterator::operator !=");
    if (expected_mod_count != ref_set->mod_count)
        throw ConcurrentModificationError("LinkedSet::Iterator::operator !=");
    if (ref_set != rhsASI->ref_set)
        throw ComparingDifferentIteratorsError("LinkedSet::Iterator::operator !=");

    return current == rhsASI->current;
}


template<class T>
bool LinkedSet<T>::Iterator::operator != (const LinkedSet<T>::Iterator& rhs) const {
    const Iterator *rhsASI = dynamic_cast<const Iterator *>(&rhs);
    if (rhsASI == 0)
        throw IteratorTypeError("LinkedSet::Iterator::operator !=");
    if (expected_mod_count != ref_set->mod_count)
        throw ConcurrentModificationError("LinkedSet::Iterator::operator !=");
    if (ref_set != rhsASI->ref_set)
        throw ComparingDifferentIteratorsError("LinkedSet::Iterator::operator !=");

    return current != rhsASI->current;
}


template<class T>
T& LinkedSet<T>::Iterator::operator *() const {
    if (expected_mod_count != ref_set->mod_count)
        throw ConcurrentModificationError("LinkedSet::Iterator::operator ->");
    if (!can_erase || !current) {
        std::ostringstream where;
        where << current
              << " when front = " << ref_set->front
              << " and rear = " << ref_set->trailer;
        throw IteratorPositionIllegal("LinkedSet::Iterator::operator -> Iterator illegal: "+where.str());
    }

    return current->value;
}


template<class T>
T* LinkedSet<T>::Iterator::operator ->() const {
    if (expected_mod_count != ref_set->mod_count)
        throw ConcurrentModificationError("LinkedSet::Iterator::operator ->");
    if (!can_erase || !current) {
        std::ostringstream where;
        where << current
              << " when front = " << ref_set->front
              << " and rear = " << ref_set->trailer;
        throw IteratorPositionIllegal("LinkedSet::Iterator::operator -> Iterator illegal: "+where.str());
    }

    return &current->value;
}


}

#endif /* LINKED_SET_HPP_ */
