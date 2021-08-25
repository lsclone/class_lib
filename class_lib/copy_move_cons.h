#pragma once

#include <stdio.h>
#include <iostream>
#include <string>

namespace tcm {

    template<typename Ty>
    class Item {
    public:
        Item() = default;
        ~Item() = default;

        explicit Item(const Ty& da) : data(da) {}

        Item(const Item& another) {
            data = another.data;
        }
        Item& operator=(const Item& another) {
            data = another.data;
            return *this;
        }

        Item(Item&& another) noexcept {
            std::swap(data, another.data);
        }
        Item& operator=(Item&& another) noexcept {
            std::swap(data, another.data);
            return *this;
        }

    private:
        std::remove_reference_t<Ty> data;
    };

    class Abstract {
    public:
        Abstract() = default;
        ~Abstract() = default;

        //Abstract(const Abstract&) = default;
        Abstract(const Abstract& another) {
            data = another.data;
        }
        Abstract& operator=(const Abstract&) = default;

        //Abstract(Abstract&&) = default;
        Abstract(Abstract&& another) {
            std::swap(data, another.data);
        }
        Abstract& operator=(Abstract&&) = default;

    private:
        int data = 0;
    };

    class Base : public Abstract {
    public:
        Base() = default;
        ~Base() = default;

        explicit Base(int da, Item<double>&& im_double);

        // Base(const Base&) = default;
        Base(const Base& another) 
            : Abstract(another),
            data(another.data),
            im_d(another.im_d) {
        }
        Base& operator=(const Base&) = default;

        //Base(Base&&) = default;
        Base(Base&& another)
            : Abstract(std::move(another)),
            data(another.data),
            im_d(std::move(another.im_d)) {
        }
        Base& operator=(Base&&) = default;

    private:
        int data;
        Item<double> im_d;
    };

    Base::Base(int da, Item<double>&& im_double)
        : data(da), im_d(std::move(im_double)) {
    }

    class Extend : public Base {
    public:
        Extend() = default;
        ~Extend() = default;

        explicit Extend(int data, Item<double>&& im_double, 
            std::string&& s, Item<int>&& im_int);

        Extend(const Extend&) = default;
        Extend& operator=(const Extend&) = default;

        Extend(Extend&&) = default;
        Extend& operator=(Extend&&) = default;

    private:
        std::string ss;
        Item<int> im_i;
    };

    Extend::Extend(int data, Item<double>&& im_double, 
        std::string&& s, Item<int>&& im_int) 
        : Base(data, std::move(im_double)), 
          ss(std::move(s)), im_i(std::move(im_int)) {
    }

} // namespace tcm


/*
usage:
void main() {
    ///////////////////////////////////////////
    {
        tcm::Extend ex(10, tcm::Item<double>(2.0),
            "hello234", tcm::Item<int>(20));

        tcm::Extend& ex_ref = ex;

        tcm::Base& ba_ref = ex_ref;

        tcm::Base ba = ba_ref;
    }

    ///////////////////////////////////////////
    {
        tcm::Extend ex1(10, tcm::Item<double>(2.0),
            "hello234", tcm::Item<int>(20));

        tcm::Extend ex2 = ex1;
        tcm::Extend ex3 = std::move(ex1);
    }
}
*/
