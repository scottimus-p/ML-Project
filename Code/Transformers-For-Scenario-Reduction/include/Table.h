#pragma once

#include <vector>

#include "Range.h"

struct table_out_of_bounds {};

struct x_axis_t {};
struct y_axis_t {};

constexpr x_axis_t x_axis{};
constexpr y_axis_t y_axis{};

namespace actlib
{

template<typename T = double>
class table
{
    int _low_bound_x = 0;
    int _high_bound_x = 0;
    int _low_bound_y = 0;
    int _high_bound_y = 0;

    int _size_x = 0;
    int _size_y = 0;

    std::vector<T> _data;

    _NODISCARD int get_total_size(X_Range x_range, Y_Range y_range) const
    {
        return range_size(x_range) * range_size(y_range);
    }

    _NODISCARD int get_internal_idx(int idx, x_axis_t) const
    {
        if (idx < _low_bound_x || idx > _high_bound_x)
        {
            throw table_out_of_bounds{};
        }

        return idx - _low_bound_x;
    }

    _NODISCARD int get_internal_idx(int idx, y_axis_t) const
    {
        if (idx < _low_bound_y || idx > _high_bound_y)
        {
            throw table_out_of_bounds{};
        }

        return idx - _low_bound_y;
    }

    _NODISCARD int get_strided_idx(int x, int y) const
    {
        return get_internal_idx(y, y_axis) + get_internal_idx(x, x_axis) * _size_y;
    }

public:

    table() {}

    table(int size_x, int size_y) :
        _low_bound_x(0),
        _high_bound_x(size_x - 1),
        _low_bound_y(0),
        _high_bound_y(size_y - 1),
        _size_x(size_x),
        _size_y(size_y),
        _data(size_x* size_y)
    {

    }

    table(X_Range x_range, Y_Range y_range) :
        _low_bound_x(x_range.lo),
        _high_bound_x(x_range.hi),
        _low_bound_y(y_range.lo),
        _high_bound_y(y_range.hi),
        _size_x(range_size(x_range)),
        _size_y(range_size(y_range)),
        _data(get_total_size(x_range, y_range))
    {

    }

    table(X_Range x_range, Y_Range y_range, const T& val) :
        _low_bound_x(x_range.lo),
        _high_bound_x(x_range.hi),
        _low_bound_y(y_range.lo),
        _high_bound_y(y_range.hi),
        _size_x(range_size(x_range)),
        _size_y(range_size(y_range)),
        _data(get_total_size(x_range, y_range), val)
    {

    }

    ~table() = default;

    _NODISCARD T operator()(int col, int row) const
    {
        return _data[get_strided_idx(col, row)];
    }

    _NODISCARD T& operator()(int col, int row)
    {
        return _data[get_strided_idx(col, row)];
    }

    int lower_bound(int whichDim) const
    {
        if (whichDim == 1)
            return _low_bound_x;
        else if (whichDim == 2)
            return _low_bound_y;
        else
            throw;
    }

    int upper_bound(int whichDim) const
    {
        if (whichDim == 1)
            return _high_bound_x;
        else if (whichDim == 2)
            return _high_bound_y;
        else
            throw;
    }

    int size() const
    {
        return _size_x * _size_y;
    }

    int size(int whichDim) const
    {
        if (whichDim == 1)
            return _size_x;
        else if (whichDim == 2)
            return _size_y;
        else
            throw;
    }

    struct forward_sentinel_t {};
    struct reverse_sentinel_t {};

    constexpr static forward_sentinel_t forward_sentinel{};
    constexpr static reverse_sentinel_t reverse_sentinel{};

    class element_iterator
    {
    public:

        enum class IterationScope
        {
            ROW,      // iterating over a single row
            COLUMN,   // iterating over a single column
            table    // iterating over the whole table
        };

    private:

        friend table;
        friend table::row_iterator;
        friend table::column_iterator;

        table* _data;
        int _rowIdx = 0;
        int _colIdx = 0;
        IterationScope _scope;

        element_iterator(table& data, int colIdx, int rowIdx, IterationScope scope) :
            _data(&data),
            _rowIdx(rowIdx),
            _colIdx(colIdx),
            _scope(scope)
        {

        }

        element_iterator() {}

    public:
        using difference_type = int;
        using value_type = T;
        using pointer = T*;
        using reference = T&;
        using iterator_category = std::random_access_iterator_tag;

        element_iterator(const element_iterator& itr) = default;

        element_iterator& operator=(const element_iterator&) = default;

        friend bool operator==(const element_iterator& lhs, const element_iterator& rhs) noexcept
        {
            return lhs._idx == rhs._idx && lhs._data == rhs._data && lhs._scope == rhs._scope;
        }

        friend bool operator!=(element_iterator& lhs, const element_iterator& rhs) noexcept
        {
            return !(lhs == rhs);
        }

        friend bool operator==(const element_iterator& lhs, const forward_sentinel_t) noexcept
        {
            switch (lhs._scope)
            {
            case IterationScope::ROW:     return lhs._rowIdx > _data.size(1);
            case IterationScope::COLUMN:  return lhs._colIdx > _data.size(2);
            case IterationScope::table:  return lhs._rowIdx > _data.size(1) || lhs.colIdx > _data.size(2);
            }

            return false;
        }

        friend bool operator!=(const element_iterator& lhs, const forward_sentinel_t) noexcept
        {
            switch (lhs._scope)
            {
            case IterationScope::ROW:     return lhs._colIdx <= lhs._data->upper_bound(1);
            case IterationScope::COLUMN:  return lhs._rowIdx <= lhs._data->upper_bound(2);
            case IterationScope::table:  return lhs._rowIdx <= lhs._data->upper_bound(2) && lhs._colIdx <= lhs._data->upper_bound(1);
            }

            return false;
        }

        element_iterator& operator+=(const difference_type& offset)
        {
            switch (_scope)
            {
            case IterationScope::ROW:     _colIdx += offset; break;
            case IterationScope::COLUMN:  _rowIdx += offset; break;
            case IterationScope::table:
                _colIdx += offset / _data->size(2);
                _rowIdx += offset % _data->size(2);

                if (_rowIdx > _data->upper_bound(2))
                {
                    _rowIdx = _data->lower_bound(2);
                    ++_colIdx;
                }

                break;
            }

            return *this;
        }

        element_iterator& operator-=(const difference_type& offset)
        {
            switch (_scope)
            {
            case IterationScope::ROW:     _colIdx -= offset; break;
            case IterationScope::COLUMN:  _rowIdx -= offset; break;
            case IterationScope::table:
                _colIdx -= offset / _data->size(2);
                _rowIdx -= offset % _data->size(2);

                if (_rowIdx < _data->lower_bound(2))
                {
                    _rowIdx = _data->upper_bound(2);
                    --_colIdx;
                }
            }

            return *this;
        }

        element_iterator& operator++()
        {
            switch (_scope)
            {
            case IterationScope::ROW:     ++_colIdx; break;
            case IterationScope::COLUMN:  ++_rowIdx; break;
            case IterationScope::table:
                if (++_rowIdx > _data->upper_bound(2))
                {
                    _rowIdx = _data->lower_bound(2);
                    ++_colIdx;
                }
                break;
            }

            return *this;
        }

        element_iterator& operator--()
        {
            switch (_scope)
            {
            case IterationScope::ROW:     --_colIdx; break;
            case IterationScope::COLUMN:  --_rowIdx; break;
            case IterationScope::table:
                if (--_rowIdx < _data->lower_bound(2))
                {
                    _rowIdx = _data->upper_bound(2);
                    --_colIdx;
                }
                break;
            }

            return *this;
        }

        element_iterator operator++(int)
        {
            auto tmp(*this);

            switch (_scope)
            {
            case IterationScope::ROW:     ++_colIdx; break;
            case IterationScope::COLUMN:  ++_rowIdx; break;
            case IterationScope::table:
                if (++_rowIdx > _data->upper_bound(2))
                {
                    _rowIdx = _data->lower_bound(2);
                    ++_colIdx;
                }
                break;
            }

            return tmp;
        }

        element_iterator operator--(int)
        {
            auto tmp(*this);

            switch (_scope)
            {
            case IterationScope::ROW:     --_colIdx; break;
            case IterationScope::COLUMN:  --_rowIdx; break;
            case IterationScope::table:
                if (--_rowIdx < _data->lower_bound(2))
                {
                    _rowIdx = _data->upper_bound(2);
                    --_colIdx;
                }
                break;
            }

            return tmp;
        }

        element_iterator operator+(const difference_type& offset)
        {
            auto tmp(*this);

            switch (_scope)
            {
            case IterationScope::ROW:     tmp._colIdx += offset; break;
            case IterationScope::COLUMN:  tmp._rowIdx += offset; break;
            case IterationScope::table:
                tmp._colIdx += offset / tmp._data->size(2);
                tmp._rowIdx += offset % tmp._data->size(2);

                if (tmp._rowIdx > tmp._data->upper_bound(2))
                {
                    tmp._rowIdx = tmp._data->lower_bound(2);
                    ++(tmp._colIdx);
                }
                break;
            }

            return tmp;
        }

        element_iterator operator-(const difference_type& offset)
        {
            auto tmp(*this);

            switch (_scope)
            {
            case IterationScope::ROW:     tmp._colIdx -= offset; break;
            case IterationScope::COLUMN:  tmp._rowIdx -= offset; break;
            case IterationScope::table:
                tmp._colIdx -= offset / tmp._data->size(2);
                tmp._rowIdx -= offset % tmp._data->size(2);

                if (tmp._rowIdx < tmp._data->lower_bound(2))
                {
                    tmp._rowIdx = tmp._data->upper_bound(2);
                    --(tmp._colIdx);
                }
                break;
            }

            return tmp;
        }

        difference_type operator-(const element_iterator& subtrahend)
        {
            return subtrahend._idx - this->_idx;
        }

        reference operator*()
        {
            return (*_data)(_colIdx, _rowIdx);
        }

        const reference operator*() const
        {
            return (*_data)(_colIdx, _rowIdx);
        }
    };

    class const_element_iterator
    {
    public:

        enum class IterationScope
        {
            ROW,      // iterating over a single row
            COLUMN,   // iterating over a single column
            table    // iterating over the whole table
        };

    private:

        friend table;
        friend table::row_iterator;
        friend table::column_iterator;

        table* _data;
        int _rowIdx = 0;
        int _colIdx = 0;
        IterationScope _scope;

        const_element_iterator(table& data, int colIdx, int rowIdx, IterationScope scope) :
            _data(&data),
            _rowIdx(rowIdx),
            _colIdx(colIdx),
            _scope(scope)
        {

        }

        const_element_iterator() {}

    public:
        using difference_type = int;
        using value_type = T;
        using pointer = T*;
        using reference = T&;
        using iterator_category = std::random_access_iterator_tag;

        const_element_iterator(const const_element_iterator& itr) = default;

        const_element_iterator& operator=(const const_element_iterator&) = default;

        friend bool operator==(const const_element_iterator& lhs, const const_element_iterator& rhs) noexcept
        {
            return lhs._idx == rhs._idx && lhs._data == rhs._data && lhs._scope == rhs._scope;
        }

        friend bool operator!=(const const_element_iterator& lhs, const const_element_iterator& rhs) noexcept
        {
            return !(lhs == rhs);
        }

        friend bool operator==(const const_element_iterator& lhs, const forward_sentinel_t) noexcept
        {
            switch (lhs._scope)
            {
            case IterationScope::ROW:     return lhs._rowIdx > _data.size(1);
            case IterationScope::COLUMN:  return lhs._colIdx > _data.size(2);
            case IterationScope::table:  return lhs._rowIdx > _data.size(1) || lhs.colIdx > _data.size(2);
            }

            return false;
        }

        friend bool operator!=(const const_element_iterator& lhs, const forward_sentinel_t) noexcept
        {
            switch (lhs._scope)
            {
            case IterationScope::ROW:     return lhs._colIdx <= lhs._data->upper_bound(1);
            case IterationScope::COLUMN:  return lhs._rowIdx <= lhs._data->upper_bound(2);
            case IterationScope::table:  return lhs._rowIdx <= lhs._data->upper_bound(2) && lhs._colIdx <= lhs._data->upper_bound(1);
            }

            return false;
        }

        const_element_iterator& operator+=(const difference_type& offset)
        {
            switch (_scope)
            {
            case IterationScope::ROW:     _colIdx += offset; break;
            case IterationScope::COLUMN:  _rowIdx += offset; break;
            case IterationScope::table:
                _colIdx += offset / _data->size(2);
                _rowIdx += offset % _data->size(2);

                if (_rowIdx > _data->upper_bound(2))
                {
                    _rowIdx = _data->lower_bound(2);
                    ++_colIdx;
                }

                break;
            }

            return *this;
        }

        const_element_iterator& operator-=(const difference_type& offset)
        {
            switch (_scope)
            {
            case IterationScope::ROW:     _colIdx -= offset; break;
            case IterationScope::COLUMN:  _rowIdx -= offset; break;
            case IterationScope::table:
                _colIdx -= offset / _data->size(2);
                _rowIdx -= offset % _data->size(2);

                if (_rowIdx < _data->lower_bound(2))
                {
                    _rowIdx = _data->upper_bound(2);
                    --_colIdx;
                }
            }

            return *this;
        }

        const_element_iterator& operator++()
        {
            switch (_scope)
            {
            case IterationScope::ROW:     ++_colIdx; break;
            case IterationScope::COLUMN:  ++_rowIdx; break;
            case IterationScope::table:
                if (++_rowIdx > _data->upper_bound(2))
                {
                    _rowIdx = _data->lower_bound(2);
                    ++_colIdx;
                }
                break;
            }

            return *this;
        }

        const_element_iterator& operator--()
        {
            switch (_scope)
            {
            case IterationScope::ROW:     --_colIdx; break;
            case IterationScope::COLUMN:  --_rowIdx; break;
            case IterationScope::table:
                if (--_rowIdx < _data->lower_bound(2))
                {
                    _rowIdx = _data->upper_bound(2);
                    --_colIdx;
                }
                break;
            }

            return *this;
        }

        const_element_iterator operator++(int)
        {
            auto tmp(*this);

            switch (_scope)
            {
            case IterationScope::ROW:     ++_colIdx; break;
            case IterationScope::COLUMN:  ++_rowIdx; break;
            case IterationScope::table:
                if (++_rowIdx > _data->upper_bound(2))
                {
                    _rowIdx = _data->lower_bound(2);
                    ++_colIdx;
                }
                break;
            }

            return tmp;
        }

        const_element_iterator operator--(int)
        {
            auto tmp(*this);

            switch (_scope)
            {
            case IterationScope::ROW:     --_colIdx; break;
            case IterationScope::COLUMN:  --_rowIdx; break;
            case IterationScope::table:
                if (--_rowIdx < _data->lower_bound(2))
                {
                    _rowIdx = _data->upper_bound(2);
                    --_colIdx;
                }
                break;
            }

            return tmp;
        }

        const_element_iterator operator+(const difference_type& offset)
        {
            auto tmp(*this);

            switch (_scope)
            {
            case IterationScope::ROW:     tmp._colIdx += offset; break;
            case IterationScope::COLUMN:  tmp._rowIdx += offset; break;
            case IterationScope::table:
                tmp._colIdx += offset / tmp._data->size(2);
                tmp._rowIdx += offset % tmp._data->size(2);

                if (tmp._rowIdx > tmp._data->upper_bound(2))
                {
                    tmp._rowIdx = tmp._data->lower_bound(2);
                    ++(tmp._colIdx);
                }
                break;
            }

            return tmp;
        }

        const_element_iterator operator-(const difference_type& offset)
        {
            auto tmp(*this);

            switch (_scope)
            {
            case IterationScope::ROW:     tmp._colIdx -= offset; break;
            case IterationScope::COLUMN:  tmp._rowIdx -= offset; break;
            case IterationScope::table:
                tmp._colIdx -= offset / tmp._data->size(2);
                tmp._rowIdx -= offset % tmp._data->size(2);

                if (tmp._rowIdx < tmp._data->lower_bound(2))
                {
                    tmp._rowIdx = tmp._data->upper_bound(2);
                    --(tmp._colIdx);
                }
                break;
            }

            return tmp;
        }

        difference_type operator-(const const_element_iterator& subtrahend)
        {
            return subtrahend._idx - this->_idx;
        }

        const reference operator*() const
        {
            return (*_data)(_colIdx, _rowIdx);
        }
    };

    class row_iterator
    {
        friend class table;

        table* _data;
        int _rowIdx;

        row_iterator(table& data, int rowIdx) :
            _data(&data),
            _rowIdx(rowIdx)
        {

        }

        using difference_type = int;

    public:

        row_iterator(const row_iterator&) = default;
        row_iterator& operator=(const row_iterator&) = default;

        row_iterator& operator++()
        {
            ++_rowIdx;
            return *this;
        }

        row_iterator operator++(int)
        {
            row_iterator tmp = *this;

            ++_rowIdx;

            return tmp;
        }

        row_iterator operator+=(difference_type offset)
        {
            _rowIdx += offset;
            return *this;
        }

        row_iterator operator-=(difference_type offset)
        {
            _rowIdx -= offset;
            return *this;
        }

        row_iterator operator+(const difference_type& offset)
        {
            auto tmpItr = *this;

            tmpItr._rowIdx -= offset;

            return tmpItr;
        }

        row_iterator operator-(const difference_type& offset)
        {
            auto tmpItr = *this;

            tmpItr._rowIdx += offset;

            return tmpItr;
        }

        difference_type operator-(const row_iterator& subtrahend)
        {
            return this->_rowIdx - subtrahend._rowIdx;
        }

        table::element_iterator begin()
        {
            return table::element_iterator(*_data, _data->lower_bound(1), _rowIdx, element_iterator::IterationScope::ROW);
        }

        forward_sentinel_t end()
        {
            return forward_sentinel;
        }

        friend bool operator==(const row_iterator& lhs, const row_iterator& rhs)
        {
            return lhs._rowIdx == rhs._rowIdx && lhs._data == rhs._data;
        }

        friend bool operator!=(const row_iterator& lhs, const row_iterator& rhs)
        {
            return !(lhs == rhs);
        }

        friend bool operator!=(const row_iterator& itr, forward_sentinel_t)
        {
            return itr._rowIdx <= itr._data->upper_bound(2);
        }

        friend bool operator==(const row_iterator& itr, forward_sentinel_t)
        {
            return itr._rowIdx > itr._data->upper_bound(2);
        }
    };

    class const_row_iterator
    {
        friend class table;

        const table* _data;
        int _rowIdx;

        const_row_iterator(table& data, int rowIdx) :
            _data(&data),
            _rowIdx(rowIdx)
        {

        }

        using difference_type = int;

    public:

        const_row_iterator(const const_row_iterator&) = default;
        const_row_iterator& operator=(const const_row_iterator&) = default;

        const_row_iterator& operator++()
        {
            ++_rowIdx;
            return *this;
        }

        const_row_iterator operator++(int)
        {
            const_row_iterator tmp = *this;

            ++_rowIdx;

            return tmp;
        }

        const_row_iterator operator+=(difference_type offset)
        {
            _rowIdx += offset;
            return *this;
        }

        const_row_iterator operator-=(difference_type offset)
        {
            _rowIdx -= offset;
            return *this;
        }

        const_row_iterator operator+(const difference_type& offset)
        {
            auto tmpItr = *this;

            tmpItr._rowIdx -= offset;

            return tmpItr;
        }

        const_row_iterator operator-(const difference_type& offset)
        {
            auto tmpItr = *this;

            tmpItr._rowIdx += offset;

            return tmpItr;
        }

        difference_type operator-(const const_row_iterator& subtrahend)
        {
            return this->_rowIdx - subtrahend._rowIdx;
        }

        table::element_iterator begin()
        {
            return table::const_element_iterator(*_data, _data->lower_bound(1), _rowIdx, element_iterator::IterationScope::ROW);
        }

        forward_sentinel_t end()
        {
            return forward_sentinel;
        }

        friend bool operator==(const const_row_iterator& lhs, const const_row_iterator& rhs)
        {
            return lhs._rowIdx == rhs._rowIdx && lhs._data == rhs._data;
        }

        friend bool operator!=(const const_row_iterator& lhs, const const_row_iterator& rhs)
        {
            return !(lhs == rhs);
        }

        friend bool operator!=(const const_row_iterator& itr, forward_sentinel_t)
        {
            return itr._rowIdx <= itr._data->upper_bound(2);
        }

        friend bool operator==(const const_row_iterator& itr, forward_sentinel_t)
        {
            return itr._rowIdx > itr._data->upper_bound(2);
        }
    };

    class column_iterator
    {
        friend class table;

        table* _data;
        int _colIdx;

        column_iterator(table& data, int colIdx) :
            _data(&data),
            _colIdx(colIdx)
        {

        }

        using difference_type = int;

    public:

        column_iterator(const column_iterator&) = default;
        column_iterator& operator=(const column_iterator&) = default;

        column_iterator& operator++()
        {
            ++_colIdx;
            return *this;
        }

        column_iterator operator++(int)
        {
            column_iterator tmp = *this;

            ++_colIdx;

            return tmp;
        }

        column_iterator operator+=(difference_type offset)
        {
            _colIdx += offset;
            return *this;
        }

        column_iterator operator-=(difference_type offset)
        {
            _colIdx -= offset;
            return *this;
        }

        column_iterator operator+(const difference_type& offset)
        {
            auto tmpItr = *this;

            tmpItr._colIdx -= offset;

            return tmpItr;
        }

        column_iterator operator-(const difference_type& offset)
        {
            auto tmpItr = *this;

            tmpItr._colIdx += offset;

            return tmpItr;
        }

        difference_type operator-(const column_iterator& subtrahend)
        {
            return this->_colIdx - subtrahend._colIdx;
        }

        table::element_iterator begin()
        {
            return table::element_iterator(*_data, _colIdx, _data->lower_bound(2), element_iterator::IterationScope::COLUMN);
        }

        forward_sentinel_t end()
        {
            return forward_sentinel;
        }

        friend bool operator==(const column_iterator& lhs, const column_iterator& rhs)
        {
            return lhs._colIdx == rhs._colIdx && lhs._data == rhs._data;
        }

        friend bool operator!=(const column_iterator& lhs, const column_iterator& rhs)
        {
            return !(lhs == rhs);
        }

        friend bool operator!=(const column_iterator& itr, forward_sentinel_t)
        {
            return itr._colIdx <= itr._data->upper_bound(1);
        }

        friend bool operator==(const column_iterator& itr, forward_sentinel_t)
        {
            return itr._colIdx > itr._data->upper_bound(1);
        }
    };

    element_iterator begin()
    {
        return element_iterator(*this, _low_bound_x, _low_bound_y, element_iterator::IterationScope::table);
    }

    const_element_iterator cbegin() const
    {
        return const_element_iterator(*this, _low_bound_x, _low_bound_y, const_element_iterator::IterationScope::table);
    }

    row_iterator begin_row()
    {
        return row_iterator(*this, _low_bound_x);
    }

    column_iterator begin_column()
    {
        return column_iterator(*this, _low_bound_y);
    }

    const_row_iterator cbegin_row() const
    {
        return const_row_iterator(*this, _low_bound_x);
    }

    forward_sentinel_t end() const
    {
        return forward_sentinel;
    }

    forward_sentinel_t cend() const
    {
        return forward_sentinel;
    }

    forward_sentinel_t end_row()
    {
        return forward_sentinel;
    }

    forward_sentinel_t cend_row()
    {
        return forward_sentinel;
    }

    forward_sentinel_t end_column()
    {
        return forward_sentinel;
    }
};

}