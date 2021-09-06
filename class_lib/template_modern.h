/*
* cplusplus template meta programming
*/

#pragma once

#include <deque>
#include <mutex>

namespace tmp {

    template<typename Ty>
    class List {
    public:
        List() = default;
        ~List() = default;

        List(const List&) = delete;
        List& operator=(const List&) = delete;

        List(List&& right) noexcept;
        List& operator=(List&& right);

        void push(Ty&& item);
        decltype(auto) pop();
        bool empty();

    private:
        template<typename T>
        using item_type = std::remove_reference_t<T>;

        std::deque<item_type<Ty>> list;
        std::mutex mtx; // std::mutex has no copy or move construction.
    };

    template<typename Ty>
    List<Ty>::List(List<Ty>&& right) noexcept {
        std::unique_lock<std::mutex> lck(right.mtx);
        list = std::move(right.list); // invoke move assignment.
    }

    template<typename Ty>
    List<Ty>& List<Ty>::operator=(List<Ty>&& right) {
        if (this != &right) {
            std::unique_lock<std::mutex> lck(right.mtx);
            list = std::move(right.list); // invoke move assignment.
        }
        return *this;
    }

    template<typename Ty>
    void List<Ty>::push(Ty&& item) {
        std::unique_lock<std::mutex> lck(mtx);
        list.push_back(std::forward<Ty>(item));
    }

    template<typename Ty>
    decltype(auto) List<Ty>::pop() {
        std::unique_lock<std::mutex> lck(mtx);
        auto item = std::move(list.front()); // invoke move construction.
        list.pop_front();
        return item;
    }

    template<typename Ty>
    bool List<Ty>::empty() {
        std::unique_lock<std::mutex> lck(mtx);
        return list.empty();
    }

} // namespace tmp
