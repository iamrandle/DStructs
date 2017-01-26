
#ifndef LINKED_QUEUE_HPP_
#define LINKED_QUEUE_HPP_

#include <string>
#include <iostream>
#include <sstream>
#include <initializer_list>
#include "ics_exceptions.hpp"


namespace ics {


    template<class T>
    class LinkedQueue {
    public:
        //Destructor/Constructors
        ~LinkedQueue();

        LinkedQueue();

        LinkedQueue(const LinkedQueue<T> &to_copy);

        explicit LinkedQueue(const std::initializer_list<T> &il);

        //Iterable class must support "for-each" loop: .begin()/.end() and prefix ++ on returned result
        template<class Iterable>
        explicit LinkedQueue(const Iterable &i);


        //Queries
        bool empty() const;

        int size() const;

        T &peek() const;

        std::string str() const; //supplies useful debugging information; contrast to operator <<


        //Commands
        int enqueue(const T &element);

        T dequeue();

        void clear();

        //Iterable class must support "for-each" loop: .begin()/.end() and prefix ++ on returned result
        template<class Iterable>
        int enqueue_all(const Iterable &i);


        //Operators
        LinkedQueue<T> &operator=(const LinkedQueue<T> &rhs);

        bool operator==(const LinkedQueue<T> &rhs) const;

        bool operator!=(const LinkedQueue<T> &rhs) const;

        template<class T2>
        friend std::ostream &operator<<(std::ostream &outs, const LinkedQueue<T2> &q);


    private:
        class LN;

    public:
        class Iterator {
        public:
            //Private constructor called in begin/end, which are friends of LinkedQueue<T>
            ~Iterator();

            T erase();

            std::string str() const;

            LinkedQueue<T>::Iterator &operator++();

            LinkedQueue<T>::Iterator operator++(int);

            bool operator==(const LinkedQueue<T>::Iterator &rhs) const;

            bool operator!=(const LinkedQueue<T>::Iterator &rhs) const;

            T &operator*() const;

            T *operator->() const;

            friend std::ostream &operator<<(std::ostream &outs, const LinkedQueue<T>::Iterator &i) {
                outs << i.str(); //Use the same meaning as the debugging .str() method
                return outs;
            }

            friend Iterator LinkedQueue<T>::begin() const;

            friend Iterator LinkedQueue<T>::end() const;

        private:
            //If can_erase is false, current indexes the "next" value (must ++ to reach it)
            LN *prev = nullptr;  //if nullptr, current at front of list
            LN *current;         //current == prev->next (if prev != nullptr)
            LinkedQueue<T> *ref_queue;
            int expected_mod_count;
            bool can_erase = true;

            //Called in friends begin/end
            Iterator(LinkedQueue<T> *iterate_over, LN *initial);
        };


        Iterator begin() const;

        Iterator end() const;


    private:
        class LN {
        public:
            LN() {}

            LN(const LN &ln) : value(ln.value), next(ln.next) {}

            LN(T v, LN *n = nullptr) : value(v), next(n) {}

            T value;
            LN *next = nullptr;
        };


        LN *front = nullptr;
        LN *rear = nullptr;
        int used = 0;            //Cache the number of values in linked list
        int mod_count = 0;            //For sensing all concurrent modifications

        //Helper methods
        void delete_list(LN *&front);  //Deallocate all LNs, and set front's argument to nullptr;
    };





////////////////////////////////////////////////////////////////////////////////
//
//LinkedQueue class and related definitions

//Destructor/Constructors

    template<class T>
    LinkedQueue<T>::~LinkedQueue() {
        this->delete_list(this->front);
    }


    template<class T>
    LinkedQueue<T>::LinkedQueue() {
    }


    template<class T>
    LinkedQueue<T>::LinkedQueue(const LinkedQueue<T> &to_copy) {
        for(auto val : to_copy)
        {
            this->enqueue(val);
        }
    }


    template<class T>
    LinkedQueue<T>::LinkedQueue(const std::initializer_list<T> &il) {
        for (auto elem : il)
            this->enqueue(elem);
    }


    template<class T>
    template<class Iterable>
    LinkedQueue<T>::LinkedQueue(const Iterable &i) {
        this->enqueue_all(i);
    }


////////////////////////////////////////////////////////////////////////////////
//
//Queries

    template<class T>
    bool LinkedQueue<T>::empty() const {
        return this->used < 1;
    }


    template<class T>
    int LinkedQueue<T>::size() const {
        return this->used;
    }


    template<class T>
    T &LinkedQueue<T>::peek() const {
        if (this->empty())
            throw EmptyError("LinkedQueue::peek");
        return this->front->value;
    }


    template<class T>
    std::string LinkedQueue<T>::str() const {
        std::ostringstream answer;
        answer << "LinkedQueue[";

        if (this->used != 0) {
            int j = 0;
            for (auto i = this->begin(); i != this->end(); ++i, j++) {
                answer << *i;
                answer << (j == this->used - 1 ? "]" : "->");
            }
        }

        answer << "(length=" << this->used << ",front=" << this->front << ",rear=" << this->rear << ",mod_count="
               << this->mod_count << ")";
        return answer.str();
    }


////////////////////////////////////////////////////////////////////////////////
//
//Commands

    template<class T>
    int LinkedQueue<T>::enqueue(const T &element) {
        if (this->front == nullptr)
            this->front = this->rear = new LN(element);
        else
            this->rear = this->rear->next = new LN(element);
        this->used++;
        this->mod_count++;
        return 1;
    }


    template<class T>
    T LinkedQueue<T>::dequeue() {
        if (this->empty())
            throw EmptyError("LinkedQueue::dequeue");
        T retVal = this->front->value;
        LN *toDel = this->front;
        this->front = this->front->next;
        if(!this->front)
            this->rear = this->front;
        delete toDel;
        this->used--;
        this->mod_count++;
        return retVal;
    }


    template<class T>
    void LinkedQueue<T>::clear() {
        this->delete_list(this->front);
        this->mod_count++;
        this->front = this->rear = nullptr;
    }


    template<class T>
    template<class Iterable>
    int LinkedQueue<T>::enqueue_all(const Iterable &i) {
        int count = 0;

        for (const T &v : i)
            count += enqueue(v);

        return count;
    }


////////////////////////////////////////////////////////////////////////////////
//
//Operators

    template<class T>
    LinkedQueue<T> &LinkedQueue<T>::operator=(const LinkedQueue<T> &rhs) {
        if (!this->front)
            this->enqueue_all(rhs);
//        else {
//            this->clear();
//            this->enqueue_all(rhs);
//        }

    else if (this->used < rhs.used)
    {

        auto j = rhs.begin();
        for(auto i = this->begin(); i!=this->end(); i++, j++)
        {
            *i = *j;
        }
        for(j; j!= rhs.end(); j++)
            this->enqueue(*j);
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
        return *this;
    }


    template<class T>
    bool LinkedQueue<T>::operator==(const LinkedQueue<T> &rhs) const {
        if (this == &rhs)
            return true;

        if (this->used != rhs.size())
            return false;

        LinkedQueue<T>::Iterator rhs_i = rhs.begin();
        for (auto i = this->begin(); i != this->end(); ++i, ++rhs_i) { // Uses ! and ==, so != on T need not be defined
            if (!(*i == *rhs_i))
                return false;
        }
        return true;
    }


    template<class T>
    bool LinkedQueue<T>::operator!=(const LinkedQueue<T> &rhs) const {
        return !(*this == rhs);
    }


    template<class T>
    std::ostream &operator<<(std::ostream &outs, const LinkedQueue<T> &q) {
        outs << "queue[";

        auto p = q.front;
        for (int i = 0; i < q.used; i++) {
            if (i == 0)
                outs << p->value;
            else
                outs << "," << p->value;
            if (p->next)
                p = p->next;
        }

        outs << "]:rear";
        return outs;
    }


////////////////////////////////////////////////////////////////////////////////
//
//Iterator constructors

    template<class T>
    auto LinkedQueue<T>::begin() const -> LinkedQueue<T>::Iterator {
        return Iterator(const_cast<LinkedQueue<T> *>(this), this->front);
    }

    template<class T>
    auto LinkedQueue<T>::end() const -> LinkedQueue<T>::Iterator {
        return Iterator(const_cast<LinkedQueue<T> *>(this), this->used == 0 ? this->rear : this->rear->next);
    }


////////////////////////////////////////////////////////////////////////////////
//
//Private helper methods

    template<class T>
    void LinkedQueue<T>::delete_list(LN *&front) {
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
    LinkedQueue<T>::Iterator::Iterator(LinkedQueue<T> *iterate_over, LN *initial)
            :current(initial), ref_queue(iterate_over), expected_mod_count(ref_queue->mod_count) {
        if (!initial && ref_queue->used > 0) {
            prev = ref_queue->rear;
        }
    }


    template<class T>
    LinkedQueue<T>::Iterator::~Iterator() {
        current = prev = nullptr;
        delete current, prev;
        ref_queue = nullptr;
        delete ref_queue;

    }


    template<class T>
    T LinkedQueue<T>::Iterator::erase() {
        if (expected_mod_count != ref_queue->mod_count)
            throw ConcurrentModificationError("LinkedQueue::Iterator::erase");
        if (!can_erase)
        {

            throw CannotEraseError("LinkedQueue::Iterator::erase Iterator cursor already erased");
        }
        if (!current)
        {
            throw CannotEraseError("LinkedQueue::Iterator::erase Iterator cursor beyond data structure");
        }

        can_erase = false;
        if(ref_queue->front == current)
            ref_queue->front = current->next;
        if (ref_queue->rear == current)
            ref_queue->rear = prev;
        T toret = current->value;
        LN* del = current;
        current=current->next;
        if(prev)
        {
            prev->next = current;
        }
        delete del;
        ref_queue->mod_count++;
        ref_queue->used--;
        expected_mod_count = ref_queue->mod_count;
        return toret;
    }


    template<class T>
    std::string LinkedQueue<T>::Iterator::str() const {
        std::ostringstream answer;
        answer << ref_queue->str() << "(current=" << current << ",expected_mod_count=" << expected_mod_count
               << ",can_erase=" << can_erase << ")";
        return answer.str();
    }


    template<class T>
    auto LinkedQueue<T>::Iterator::operator++() -> LinkedQueue<T>::Iterator & {

        if (expected_mod_count != ref_queue->mod_count)
            throw ConcurrentModificationError("LinkedQueue::Iterator::operator ++");
        if (!current)
            return *this;
        if (can_erase) {
            prev = current;
            current = prev->next;
        } else
            can_erase = true;
        return *this;
    }


    template<class T>
    auto LinkedQueue<T>::Iterator::operator++(int) -> LinkedQueue<T>::Iterator {
        if (expected_mod_count != ref_queue->mod_count)
            throw ConcurrentModificationError("LinkedQueue::Iterator::operator ++(int)");

        if (!current)
            return *this;

        Iterator to_return(*this);
        if (can_erase) {
            prev = current;
            current = prev->next;
        } else
            can_erase = true;  //current already indexes "one beyond" deleted value

        return to_return;
    }


    template<class T>
    bool LinkedQueue<T>::Iterator::operator==(const LinkedQueue<T>::Iterator &rhs) const {
        const Iterator *rhsASI = dynamic_cast<const Iterator *>(&rhs);
        if (rhsASI == 0)
            throw IteratorTypeError("LinkedQueue::Iterator::operator !=");
        if (expected_mod_count != ref_queue->mod_count)
            throw ConcurrentModificationError("LinkedQueue::Iterator::operator !=");
        if (ref_queue != rhsASI->ref_queue)
            throw ComparingDifferentIteratorsError("LinkedQueue::Iterator::operator !=");

        return current == rhsASI->current;
    }


    template<class T>
    bool LinkedQueue<T>::Iterator::operator!=(const LinkedQueue<T>::Iterator &rhs) const {
        const Iterator *rhsASI = dynamic_cast<const Iterator *>(&rhs);
        if (rhsASI == 0)
            throw IteratorTypeError("LinkedQueue::Iterator::operator !=");
        if (expected_mod_count != ref_queue->mod_count)
            throw ConcurrentModificationError("LinkedQueue::Iterator::operator !=");
        if (ref_queue != rhsASI->ref_queue)
            throw ComparingDifferentIteratorsError("LinkedQueue::Iterator::operator !=");

        return current != rhsASI->current;
    }


    template<class T>
    T &LinkedQueue<T>::Iterator::operator*() const {
        if (expected_mod_count != ref_queue->mod_count)
            throw ConcurrentModificationError("LinkedQueue::Iterator::operator ->");
        if (!can_erase || !current) {
            std::ostringstream where;
            where << current
                  << " when front = " << ref_queue->front
                  << " and rear = " << ref_queue->rear;
            throw IteratorPositionIllegal("LinkedQueue::Iterator::operator -> Iterator illegal: "+where.str());
        }

        return current->value;
    }


    template<class T>
    T *LinkedQueue<T>::Iterator::operator->() const {
        if (expected_mod_count != ref_queue->mod_count)
            throw ConcurrentModificationError("LinkedQueue::Iterator::operator ->");
        if (!can_erase || !current) {
            std::ostringstream where;
            where << current
                  << " when front = " << ref_queue->front
                  << " and rear = " << ref_queue->rear;
            throw IteratorPositionIllegal("LinkedQueue::Iterator::operator -> Iterator illegal: "+where.str());
        }

        return &current->value;
    }


}

#endif /* LINKED_QUEUE_HPP_ */
