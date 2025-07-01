#ifndef DEQUE_DMASLOFF_DEQUE_H
#define DEQUE_DMASLOFF_DEQUE_H

#include <cstddef>
#include <iostream>
#include <iterator>
#include <vector>

template <typename T>
class Deque;

namespace deque_namespace {
const static size_t base_size = 16;
}

template <typename T, bool is_const>
class deque_iterator {
  public:
    using const_type = typename std::add_const<T>::type;

    using iterator_category = std::random_access_iterator_tag;
    using difference_type = std::ptrdiff_t;
    using value_type = T;
    using pointer =
        typename std::conditional<is_const, const_type*, value_type*>::type;
    using reference =
        typename std::conditional<is_const, const_type&, value_type&>::type;
    using const_reference = const_type&;

    using const_type_iterator = deque_iterator<T, true>;
    using iterator_const_reference = const const_type_iterator&;
    using container_ptr_type = std::vector<value_type*>*;
    using const_container_ptr_type = const std::vector<value_type*>*;

    deque_iterator(const_container_ptr_type ptr)
        : _storage_ptr(ptr),
          _current_bucket((*ptr)[0]),
          _storage_index(0),
          _base_index(0) {}

    deque_iterator(const_container_ptr_type ptr, int storage_index,
                   int base_index)
        : _storage_ptr(ptr),
          _current_bucket((*ptr)[storage_index]),
          _storage_index(storage_index),
          _base_index(base_index) {}

    deque_iterator& operator++() {
        if (_base_index + 1 < deque_namespace::base_size) {
            ++_base_index;
        } else {
            ++_storage_index;
            _base_index = 0;
            _current_bucket = (*_storage_ptr)[_storage_index];
        }
        return *this;
    }

    deque_iterator operator++(int) {
        deque_iterator tmp = *this;
        ++(*this);
        return tmp;
    }

    deque_iterator& operator--() {
        if (_base_index > 0) {
            --_base_index;
        } else {
            --_storage_index;
            _base_index = deque_namespace::base_size - 1;
            _current_bucket = (*_storage_ptr)[_storage_index];
        }
        return *this;
    }

    deque_iterator operator--(int) {
        deque_iterator tmp = *this;
        --(*this);
        return tmp;
    }

    deque_iterator& operator+=(difference_type value) {
        if (value > 0) {
            _storage_index +=
                (_base_index + value) / deque_namespace::base_size;
        } else {
            _storage_index -=
                (deque_namespace::base_size - 1 - value - _base_index) /
                deque_namespace::base_size;
        }
        _base_index = ((_base_index + value) % deque_namespace::base_size +
                       deque_namespace::base_size) %
                      deque_namespace::base_size;
        _current_bucket = (*_storage_ptr)[_storage_index];
        return *this;
    }

    deque_iterator& operator-=(difference_type value) {
        (*this) += -value;
        return *this;
    }

    deque_iterator operator+(difference_type value) const {
        deque_iterator tmp(*this);
        tmp += value;
        return tmp;
    }

    deque_iterator operator-(difference_type value) const {
        deque_iterator tmp(*this);
        tmp -= value;
        return tmp;
    }

    pointer operator->() const {
        return _current_bucket + _base_index;
    }

    reference operator*() const {
        return _current_bucket[_base_index];
    }

    difference_type operator-(iterator_const_reference another) const {
        return static_cast<difference_type>(*this) -
               static_cast<difference_type>(another);
    }

    explicit operator difference_type() const {
        return _storage_index * deque_namespace::base_size + _base_index;
    }

    operator deque_iterator<T, true>() const {
        return deque_iterator<T, true>(_storage_ptr, _storage_index,
                                       _base_index);
    }

    template <typename K, bool is_const1, bool is_const2>
    friend bool operator==(const deque_iterator<K, is_const1>& first,
                           const deque_iterator<K, is_const2>& second);

    friend Deque<T>;
    friend class deque_iterator<T, true>;
    friend class deque_iterator<T, false>;

    template <bool is_const1>
    bool operator<(const deque_iterator<T, is_const1>& another) {
        return static_cast<difference_type>(*this) <
               static_cast<difference_type>(another);
    }

    ~deque_iterator(){};

  private:
    const std::vector<value_type*>* _storage_ptr;
    value_type* _current_bucket;
    size_t _storage_index;
    size_t _base_index;

    template <bool is_const1>
    bool equal_to(const deque_iterator<T, is_const1>& another) const {
        return _storage_ptr == another._storage_ptr &&
               static_cast<difference_type>(*this) ==
                   static_cast<difference_type>(another);
    }
};

template <typename K, bool is_const1, bool is_const2>
bool operator==(const deque_iterator<K, is_const1>& first,
                const deque_iterator<K, is_const2>& second) {
    return first.equal_to(second);
}

template <typename T>
class Deque {
  public:
    using const_type = typename std::add_const<T>::type;

    using value_type = T;
    using pointer = T*;
    using reference = T&;
    using const_reference = const_type&;

    using iterator = deque_iterator<value_type, false>;
    using const_iterator = deque_iterator<value_type, true>;
    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;

    friend deque_iterator<value_type, false>;
    friend deque_iterator<value_type, true>;

    Deque()
        : _storage(std::vector<T*>(_min_length, nullptr)),
          _storage_size(_min_length),
          _element_number(0),
          _first_base(_min_length / 4),
          _first_index(0){};

    Deque(int size)
        : _storage(std::vector<T*>(4 * (size / 16 + 1), nullptr)),
          _storage_size(4 * (size / 16 + 1)),
          _element_number(size),
          _first_base(_storage_size / 4),
          _first_index(0) {
        size_t bucket_number = _first_base;
        size_t inbucket_index = 0;
        try {
            for (size_t i = 0; i < _element_number; ++i) {
                if (_storage[bucket_number] == nullptr) {
                    _storage[bucket_number] = reinterpret_cast<T*>(
                        new char[sizeof(T) * deque_namespace::base_size]);
                }
                new (_storage[bucket_number] + inbucket_index) T();
                ++inbucket_index;
                if (inbucket_index == deque_namespace::base_size) {
                    inbucket_index = 0;
                    ++bucket_number;
                }
            }
        } catch (...) {
            clear(bucket_number, 0, inbucket_index);
            delete[] reinterpret_cast<char*>(_storage[bucket_number]);
            for (size_t i = _first_base; i < bucket_number; ++i) {
                clear(i, 0, deque_namespace::base_size);
                delete[] reinterpret_cast<char*>(_storage[i]);
            }
            throw;
        }
    };

    Deque(int size, const T& value)
        : _storage(std::vector<T*>(4 * (size / 16 + 1), nullptr)),
          _storage_size(4 * (size / 16 + 1)),
          _element_number(size),
          _first_base(_storage_size / 4),
          _first_index(0) {
        size_t bucket_number = _first_base;
        size_t inbucket_index = 0;
        try {
            for (size_t i = 0; i < _element_number; ++i) {
                if (_storage[bucket_number] == nullptr) {
                    _storage[bucket_number] = reinterpret_cast<T*>(
                        new char[sizeof(T) * deque_namespace::base_size]);
                }
                new (_storage[bucket_number] + inbucket_index) T(value);
                ++inbucket_index;
                if (inbucket_index == deque_namespace::base_size) {
                    inbucket_index = 0;
                    ++bucket_number;
                }
            }
        } catch (...) {
            clear(bucket_number, 0, inbucket_index);
            delete[] reinterpret_cast<char*>(_storage[bucket_number]);
            for (size_t i = _first_base; i < bucket_number; ++i) {
                clear(i, 0, deque_namespace::base_size);
                delete[] reinterpret_cast<char*>(_storage[i]);
            }
            throw;
        }
    };

    Deque(const Deque& another)
        : _storage(std::vector<T*>(another._storage_size, nullptr)),
          _storage_size(another._storage_size),
          _element_number(another._element_number),
          _first_base(another._first_base),
          _first_index(another._first_index) {
        for (size_t i = _first_base; i < _storage_size; ++i) {
            if (another._storage[i] != nullptr) {
                std::pair<size_t, size_t> border = another._baseIndex(i);
                _storage[i] = reinterpret_cast<T*>(
                    new char[sizeof(T) * deque_namespace::base_size]);
                for (size_t j = border.first; j != border.second; ++j) {
                    new (_storage[i] + j) T(another._storage[i][j]);
                }
            }
        }
    }

    Deque& operator=(const Deque<T>& another) {
        Deque copy(another);
        _swap(copy);
        return *this;
    }

    reference operator[](size_t index) {
        std::pair<size_t, size_t> pair = _addIndex(index);
        return _storage[pair.first][pair.second];
    }

    const_reference operator[](size_t index) const {
        std::pair<size_t, size_t> pair = _addIndex(index);
        return _storage[pair.first][pair.second];
    }

    reference at(size_t index) {
        if (index >= _element_number) {
            throw std::out_of_range("Index out of range!");
        }
        return (*this)[index];
    }

    const_reference at(size_t index) const {
        if (index >= _element_number) {
            throw std::out_of_range("Index out of range!");
        }
        return (*this)[index];
    }

    size_t size() const {
        return _element_number;
    }

    bool empty() const {
        return _element_number == 0;
    }

    reference front() {
        return this->operator[](0);
    }

    const_reference front() const {
        return this->operator[](0);
    }

    reference back() {
        return this->operator[](_element_number - 1);
    }

    const_reference back() const {
        return this->operator[](_element_number - 1);
    }

    void push_back(const_reference value) {
        std::pair<size_t, size_t> pair = _addIndex(_element_number);
        if (pair.first == _storage_size ||
            (pair.first + 1 == _storage_size &&
             pair.second + 1 == deque_namespace::base_size)) {
            _remem();
        }
        pair = _addIndex(_element_number);
        if (_storage[pair.first] == nullptr) {
            _storage[pair.first] = reinterpret_cast<T*>(
                new char[sizeof(T) * deque_namespace::base_size]);
        }
        new (_storage[pair.first] + pair.second) T(value);
        ++_element_number;
    }

    void push_front(const_reference value) {
        if (_first_base == 0 && _first_index == 0) {
            _remem();
        }
        if (_first_index > 0) {
            --_first_index;
        } else {
            --_first_base;
            _first_index = deque_namespace::base_size - 1;
            if (_storage[_first_base] == nullptr) {
                _storage[_first_base] = reinterpret_cast<T*>(
                    new char[sizeof(T) * deque_namespace::base_size]);
            }
        }
        new (_storage[_first_base] + _first_index) T(value);
        ++_element_number;
    }

    void pop_back() {
        erase(--end());
    }

    void pop_front() {
        --_element_number;
        (_storage[_first_base] + _first_index)->~T();
        if (_first_index + 1 < deque_namespace::base_size) {
            ++_first_index;
        } else {
            _first_index = 0;
            ++_first_base;
        }
    }

    void insert(const_iterator it, const_reference value) {
        std::pair<size_t, size_t> pair1, pair2 = _addIndex(_element_number);
        if (pair2.first == _storage_size) {
            _remem();
        }
        if (it == end()) {
            push_back(value);
            return;
        }
        if (it == begin()) {
            push_front(value);
            return;
        }
        ptrdiff_t index = it - begin();
        push_back(back());
        for (ptrdiff_t i = _element_number - 1; i > index; --i) {
            pair1 = _addIndex(i - 1);
            pair2 = _addIndex(i);
            (_storage[pair2.first] + pair2.second)->~T();
            new (_storage[pair2.first] + pair2.second)
                T(_storage[pair1.first][pair1.second]);
        }
        pair2 = _addIndex(index);
        new (_storage[pair2.first] + pair2.second) T(value);
    }

    void erase(const_iterator it) {
        ptrdiff_t index = it - begin();
        std::pair<size_t, size_t> pair1 = _addIndex(index), pair2;
        (_storage[pair1.first] + pair1.second)->~T();
        for (size_t i = index + 1; i < _element_number; ++i) {
            pair1 = _addIndex(i - 1);
            pair2 = _addIndex(i);
            new (_storage[pair1.first] + pair1.second)
                T(_storage[pair2.first][pair2.second]);
            (_storage[pair2.first] + pair2.second)->~T();
        }
        --_element_number;
    }

    iterator begin() {
        return iterator(&_storage, _first_base, _first_index);
    }

    const_iterator begin() const {
        return const_iterator(&_storage, _first_base, _first_index);
    }

    const_iterator cbegin() const {
        return const_iterator(&_storage, _first_base, _first_index);
    }

    iterator end() {
        std::pair<size_t, size_t> pair = _addIndex(_element_number);
        return iterator(&_storage, pair.first, pair.second);
    }

    const_iterator end() const {
        std::pair<size_t, size_t> pair = _addIndex(_element_number);
        return const_iterator(&_storage, pair.first, pair.second);
    }

    const_iterator cend() const {
        std::pair<size_t, size_t> pair = _addIndex(_element_number);
        return const_iterator(&_storage, pair.first, pair.second);
    }

    reverse_iterator rbegin() {
        return reverse_iterator(end());
    }

    const_reverse_iterator rbegin() const {
        return reverse_iterator(end());
    }

    const_reverse_iterator crbegin() const {
        return const_reverse_iterator(end());
    }

    reverse_iterator rend() {
        return reverse_iterator(begin());
    }

    const_reverse_iterator rend() const {
        return reverse_iterator(begin());
    }

    const_reverse_iterator crend() const {
        return const_reverse_iterator(begin());
    }

    ~Deque() {
        for (size_t i = 0; i < _storage_size; ++i) {
            if (_storage[i] != nullptr) {
                std::pair<size_t, size_t> border = _baseIndex(i);
                clear(i, border.first, border.second);
                delete[] reinterpret_cast<char*>(_storage[i]);
            }
        }
    }

  private:
    std::vector<T*> _storage;
    size_t _storage_size;
    size_t _element_number;
    size_t _first_base;
    size_t _first_index;

    const static size_t _min_length = 4;

    std::pair<size_t, size_t> _addIndex(size_t diff) const {
        size_t base =
            _first_base + (_first_index + diff) / deque_namespace::base_size;
        size_t index = (_first_index + diff) % deque_namespace::base_size;
        return {base, index};
    }

    std::pair<size_t, size_t> _baseIndex(size_t index) const {
        std::pair<size_t, size_t> border = _addIndex(_element_number);
        if (index < _first_base || index > border.first) {
            return {deque_namespace::base_size, deque_namespace::base_size};
        }
        size_t first = 0, last = 0;
        first = (index == _first_base ? _first_index : 0);
        last = (index == border.first ? border.second
                                      : deque_namespace::base_size);
        return {first, last};
    }

    void _swap(Deque<T>& another) {
        std::swap(_storage, another._storage);
        std::swap(_storage_size, another._storage_size);
        std::swap(_element_number, another._element_number);
        std::swap(_first_base, another._first_base);
        std::swap(_first_index, another._first_index);
    }

    void _remem() {
        std::vector<T*> tmp(_storage_size * 3, nullptr);
        size_t tmp_index = _storage_size / 3;
        for (size_t i = 0; i < _storage_size; ++i) {
            tmp[tmp_index + i] = _storage[i];
            _storage[i] = nullptr;
        }
        _first_base += _storage_size / 3;
        _storage_size *= 3;
        std::swap(_storage, tmp);
    }

    void clear(size_t bucket_number, size_t first, size_t last) const {
        for (size_t i = first; i < std::min(deque_namespace::base_size, last);
             ++i) {
            (_storage[bucket_number] + i)->~T();
        }
    }
};

template <typename T, bool is_const>
deque_iterator<T, is_const> operator+(
    typename deque_iterator<T, is_const>::difference_type value,
    deque_iterator<T, is_const> iterator) {
    iterator += value;
    return iterator;
}

template <typename T, bool is_const>
deque_iterator<T, is_const> operator-(
    typename deque_iterator<T, is_const>::difference_type value,
    deque_iterator<T, is_const> iterator) {
    iterator -= value;
    return iterator;
}

template <typename T, bool is_const1, bool is_const2>
bool operator<(const deque_iterator<T, is_const1>& first,
               const deque_iterator<T, is_const2>& second) {
    return static_cast<typename deque_iterator<T, is_const1>::difference_type>(
               first) <
           static_cast<typename deque_iterator<T, is_const2>::difference_type>(
               second);
}

template <typename T, bool is_const1, bool is_const2>
bool operator!=(const deque_iterator<T, is_const1>& first,
                const deque_iterator<T, is_const2>& second) {
    return !(first == second);
}

template <typename T, bool is_const1, bool is_const2>
bool operator>(const deque_iterator<T, is_const1>& first,
               const deque_iterator<T, is_const2>& second) {
    return second < first;
}

template <typename T, bool is_const1, bool is_const2>
bool operator<=(const deque_iterator<T, is_const1>& first,
                const deque_iterator<T, is_const2>& second) {
    return !(second < first);
}

template <typename T, bool is_const1, bool is_const2>
bool operator>=(const deque_iterator<T, is_const1>& first,
                const deque_iterator<T, is_const2>& second) {
    return !(first < second);
}

#endif  //DEQUE_DMASLOFF_DEQUE_H
