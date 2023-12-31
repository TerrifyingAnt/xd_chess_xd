//
// Created by Viktor Hundahl Strate on 19/02/2019.
//


#pragma once

#include "logger.h"
#include <vector>

template <typename T>
class LinkedList
{
public:
    T value;
    LinkedList<T>* next;

    LinkedList();

    explicit LinkedList(T item);

    ~LinkedList();

    void push(const T& item);

    void swap(int a, int b);

    LinkedList<T>* get(int i);

    void printList();

    bool contains(const T& item);

    bool end() const;

    int size();

    std::vector<T> toList();

private:
    bool initialized;
};

template<typename T>
LinkedList<T>::LinkedList() : next(nullptr), initialized(false) {}

template<typename T>
LinkedList<T>::LinkedList(T item) : value(item), next(nullptr), initialized(true)
{}

template<typename T>
LinkedList<T>::~LinkedList()
{
    if (this->next != nullptr)
        delete this->next;
}

template<typename T>
void LinkedList<T>::push(const T& item)
{
    if (!initialized)
    {
        this->value = item;
        initialized = true;
        return;
    }

    auto* link = new LinkedList(item);

    LinkedList<T>* tail = this;

    while(tail->next != nullptr) {
        tail = tail->next;
    }

    tail->next = link;
}

template<typename T>
LinkedList<T>* LinkedList<T>::get(int i)
{
    LinkedList<T>* current = this;
    while (i > 0 && current != nullptr) {
        current = current->next;
        i--;
    }

    return i == 0 ? current : nullptr;
}


template<typename T>
void LinkedList<T>::swap(int a, int b)
{
    auto* listA = get(a);
    auto* listB = get(b);

    T temp = listA->value;
    listA->value = listB->value;
    listB->value = temp;
}

template<typename T>
bool LinkedList<T>::contains(const T& item)
{
    LinkedList<T>* head = this;

    while (head != nullptr) {
        if (head->value == item) return true;
        head = head->next;
    }

    return false;
}

template<typename T>
void LinkedList<T>::printList()
{
    if (!initialized) {
        Println(";");
        return;
    }

    Print(value);

    if (next == nullptr) {
        Println("");
        return;
    }

    Print(", ");

    next->printList();
}

template<typename T>
bool LinkedList<T>::end() const
{
    return !initialized;
}

template<typename T>
int LinkedList<T>::size() 
{
    int i = 0;
    const LinkedList<T>* current = this;  // Use 'const' for safety
    while (current != nullptr) {
        i++;
        current = current->next;
    }
    return i;
}

template<typename T>
std::vector<T> LinkedList<T>::toList() {
    std::vector<T> list;
    LinkedList<T>* current = this;
    while(current != nullptr && current->initialized) {
        list.push_back(current->value);
        current = current->next;
    }
    return list;
}

