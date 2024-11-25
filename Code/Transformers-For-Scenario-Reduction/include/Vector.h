#pragma once

#include <vector>

#include "Range.h"

struct vector_out_of_bounds {};


namespace actlib
{

struct forward_sentinel_t {};
struct reverse_sentinel_t {};



template <typename T>
class vector;



template <typename T>
class const_iterator
{
    friend actlib::vector<T>;

    const actlib::vector<T>& _data;
    int _idx = 0;

protected:

    const_iterator(const vector<T>& data, int idx) :
        _data(data),
        _idx(idx)
    {

    }

public:
    using difference_type   = int;
    using value_type        = T;
    using pointer           = const T*;
    using reference         = const T&;
    using iterator_category = std::random_access_iterator_tag;

    const_iterator(const const_iterator & itr) = default;

    const_iterator& operator=(const const_iterator&) = default;

    friend _NODISCARD _CONSTEXPR17 bool operator==(const const_iterator& lhs, const const_iterator& rhs)
    {
        return lhs._idx == rhs._idx && lhs._data == rhs._data;
    }

    friend bool operator!=(const const_iterator& lhs, const const_iterator& rhs)
    {
        return !(lhs == rhs);
    }

    friend bool operator==(const const_iterator & lhs, const forward_sentinel_t & rhs)
    {
        return lhs._idx > lhs._data.upper_bound();
    }

    friend bool operator!=(const const_iterator & lhs, const forward_sentinel_t & rhs)
    {
        return lhs._idx <= lhs._data.upper_bound();
    }

    friend bool operator==(const const_iterator& lhs, const reverse_sentinel_t& rhs)
    {
        return lhs._idx < 0;
    }

    friend bool operator!=(const const_iterator& lhs, const reverse_sentinel_t& rhs)
    {
        return lhs._idx >= 0;
    }

    const_iterator& operator+=(const difference_type & offset)
    {
        _idx += offset;
        return *this;
    }

    const_iterator& operator-=(const difference_type & offset)
    {
        _idx -= offset;
        return *this;
    }

    const_iterator& operator++()
    {
        ++_idx;
        return *this;
    }

    const_iterator& operator--()
    {
        --_idx;
        return *this;
    }

    const_iterator operator++(int)
    {
        auto tmp = *this;
        ++_idx;
        return tmp;
    }

    const_iterator operator--(int)
    {
        auto tmp = *this;
        --_idx;
        return tmp;
    }

    const_iterator operator+(const difference_type & offset)
    {
        auto tmpItr = *this;

        tmpItr._idx += offset;

        return tmpItr;
    }

    const_iterator operator-(const difference_type & offset)
    {
        auto tmpItr = *this;

        tmpItr._idx -= offset;

        return tmpItr;
    }

    difference_type operator-(const const_iterator & subtrahend)
    {
        return subtrahend._idx - this->_idx;
    }

    reference operator*()
    {
        return _data(_idx);
    }

    const reference operator*() const
    {
        return _data(_idx);
    }
};


template <typename T>
class iterator : public const_iterator<T>
{
    friend actlib::vector<T>;

    using _MyBase           = const_iterator<T>;

    iterator(vector<T>& data, int idx) :
        const_iterator<T>(data, idx)
    {

    }

public:
    using difference_type   = int;
    using value_type        = T;
    using pointer           = T*;
    using reference         = T&;
    using iterator_category = std::random_access_iterator_tag;

    iterator(const iterator& itr) = default;

    iterator& operator=(const iterator&) = default;

    iterator& operator+=(const difference_type& offset)
    {
        _MyBase::operator+=(offset);
        return *this;
    }

    iterator& operator-=(const difference_type& offset)
    {
        _MyBase::operator-=(offset);
        return *this;
    }

    iterator& operator++()
    {
        _MyBase::operator++();
        return *this;
    }

    iterator& operator--()
    {
        _MyBase::operator--();
        return *this;
    }

    iterator operator++(int)
    {
        auto tmp = *this;
        _MyBase::operator++();
        return tmp;
    }

    iterator operator--(int)
    {
        auto tmp = *this;
        _MyBase::operator--();
        return tmp;
    }

    iterator operator+(const difference_type& offset)
    {
        auto tmpItr = *this;

        tmpItr._idx += offset;

        return tmpItr;
    }

    iterator operator-(const difference_type& offset)
    {
        auto tmpItr = *this;

        tmpItr._idx -= offset;

        return tmpItr;
    }

    difference_type operator-(const iterator& subtrahend)
    {
        return subtrahend._idx - this->_idx;
    }

    reference operator*()
    {
        return const_cast<reference>(_MyBase::operator*());;
    }
};



template <typename Iter>
class reverse_iterator
{
    Iter _it{};

public:

    using iterator_type   = Iter;
    using value_type      = std::iter_value_t<Iter>;
    using difference_type = std::iter_difference_t<Iter>;
    using pointer         = typename std::iterator_traits<Iter>::pointer;
    using reference       = std::iter_reference_t<Iter>;

    reverse_iterator(const Iter& it) :
        _it(std::move(it))
    {

    }

    reverse_iterator(const reverse_iterator&) = default;

    friend _NODISCARD _CONSTEXPR17 bool operator==(const reverse_iterator& lhs, const reverse_iterator& rhs)
    {
        return lhs._it == rhs._it;
    }

    friend bool operator!=(const reverse_iterator& lhs, const reverse_iterator& rhs)
    {
        return !(lhs == rhs);
    }

    friend bool operator==(const reverse_iterator& lhs, const reverse_sentinel_t& rhs)
    {
        return lhs._it == rhs;
    }

    friend bool operator!=(const reverse_iterator& lhs, const reverse_sentinel_t& rhs)
    {
        return lhs._it != rhs;
    }

    reverse_iterator& operator+=(const difference_type& offset)
    {
        _it -= offset;
        return *this;
    }

    reverse_iterator& operator-=(const difference_type& offset)
    {
        _it += offset;
        return *this;
    }

    reverse_iterator operator++()
    {
        --_it;
        return *this;   
    }

    reverse_iterator operator++(int)
    {
        auto tmp = *this;
        --_it;
        return tmp;
    }

    reverse_iterator operator--()
    {
        ++_it;
        return *this;
    }

    reverse_iterator operator--(int)
    {
        auto tmp = *this;
        ++_it;
        return tmp;
    }

    reverse_iterator operator+(const difference_type& offset)
    {
        auto tmpItr = *this;

        _it -= offset;

        return tmpItr;
    }

    reverse_iterator operator-(const difference_type& offset)
    {
        auto tmpItr = *this;

        _it += offset;

        return tmpItr;
    }

    difference_type operator-(const reverse_iterator& subtrahend)
    {
        return this->_it - subtrahend._it;
    }

    reference operator*()
    {
        return *_it;
    }
};



template <typename T>
class vector
{
    std::vector<T> _data;

    int _low_bound = 0;
    int _high_bound = 0;

    _NODISCARD int get_internal_idx(int idx) const
    {
        if (idx < _low_bound || idx > _high_bound)
            throw vector_out_of_bounds{};

        return idx - _low_bound;
    }

public:

    using value_type      = T;
    using pointer         = typename std::vector<T>::pointer;
    using const_pointer   = typename std::vector<T>::const_pointer;
    using reference       = T&;
    using size_type       = typename std::vector<T>::size_type;
    using difference_type = typename std::vector<T>::difference_type;

    vector() :
        _low_bound(0),
        _high_bound(0)
    {

    }

    explicit vector(Range range) :
        _low_bound(range.lo),
        _high_bound(range.hi),
        _data(range_size(range))
    {

    }

    vector(Range range, const T& value) :
        _low_bound(range.lo),
        _high_bound(range.hi),
        _data(range_size(range), value)
    {

    }

    explicit vector(size_type count) :
        _low_bound(0),
        _high_bound(count - 1),
        _data(count)
    {
        if (count > INT_MAX) throw;
    }

    vector(size_type count, const T& value) :
        _low_bound(0),
        _high_bound(count - 1),
        _data(count, value)
    {

    }

    vector& operator=(const vector& other) = default;

    vector(const vector&) = default;
    vector(vector&&)      = default;

    //flex_vector(std::initializer_list<T> init);

    int lower_bound() const
    {
        return _low_bound;
    }

    int upper_bound() const
    {
        return _high_bound;
    }

    T& operator()(int idx)
    {
        return _data[get_internal_idx(idx)];
    }

    const T& operator()(int idx) const
    {
        return _data[get_internal_idx(idx)];
    }

    void clear()
    {
        for (auto& i : _data)
        {
            i = T();
        }
    }

    void fill(const T& val)
    {
        for (auto& i : _data)
        {
            i = val;
        }
    }

    int size() const
    {
        Range r {.lo = _low_bound, .hi = _high_bound};

        return range_size(r);
    }   

    iterator<value_type> begin()
    {
        return iterator<value_type>(*this, _low_bound);
    }

    const_iterator<value_type> cbegin() const
    {
        return const_iterator<value_type>(*this, _low_bound);
    }

    reverse_iterator<iterator<value_type>> rbegin()
    {
        return reverse_iterator<iterator<value_type>>(iterator<value_type>(*this, _data.size() - 1));
    }

    reverse_iterator<const_iterator<value_type>> crbegin() const
    {
        return reverse_iterator<const_iterator<value_type>>(const_iterator<value_type>(*this, _data.size() - 1));
    }

    forward_sentinel_t end() const
    {
        return forward_sentinel_t {};
    }

    forward_sentinel_t cend() const
    {
        return forward_sentinel_t {};
    }

    reverse_sentinel_t rend() const
    {
        return reverse_sentinel_t {};
    }

    reverse_sentinel_t crend() const
    {
        return reverse_sentinel_t{};
    }
};


}