#pragma once

#include <zeno/zycl/core.h>
#include <zeno/zycl/instance.h>
#include <zeno/zycl/helper.h>


#ifndef ZENO_WITH_SYCL

ZENO_NAMESPACE_BEGIN
namespace zycl {
inline namespace ns_vector {

template <class T>
struct vector : std::vector<T> {
    using std::vector<T>::vector;

    template <access::mode mode>
    auto get_access(auto &&cgh, auto &&...) {
        return functor_accessor([this] (id<1> idx) -> decltype(auto) {
            return (*this)[idx];
        });
    }

    inline auto &as_vector() {
        return static_cast<std::vector<T> &>(*this);
    }

    inline auto const &to_vector() const {
        return static_cast<std::vector<T> const &>(*this);
    }

    inline auto &&move_vector() {
        return std::move(static_cast<std::vector<T> &>(*this));
    }
};

}
}
ZENO_NAMESPACE_END

#else

ZENO_NAMESPACE_BEGIN
namespace zycl {
inline namespace ns_vector {

inline constexpr size_t _M_nozerosize(size_t size) {
#ifdef __HIPSYCL__
    return size;
#else
    // DPC++ doesn't allow zero-sized buffer...
    return std::max((size_t)1, size);
#endif
}

template <class Vector, class Parent>
struct _M_as_vector : Vector {
    Parent *_M_parent;

    explicit _M_as_vector(Parent *parent) : _M_parent(parent) {}
    _M_as_vector(_M_as_vector const &) = delete;
    _M_as_vector &operator=(_M_as_vector const &) = delete;
    _M_as_vector(_M_as_vector &&) = default;
    _M_as_vector &operator=(_M_as_vector &&) = default;

    ~_M_as_vector() {
        size_t size = Vector::size();
        _M_parent->_M_size = size;
        _M_parent->_M_buf = decltype(_M_parent->_M_buf)(_M_nozerosize(size));
        auto hacc = _M_parent->_M_buf.template get_access<access::mode::discard_write>();
        for (size_t i = 0; i < size; i++) {
            hacc[i] = (*this)[i];
        }
    }
};

template <class T>
inline void _M_transfer(buffer<T, 1> &buf_src, buffer<T, 1> &buf_dst, size_t size) {
    default_queue().submit([&] (handler &cgh) {
        auto src_acc = buf_src.template get_access<access::mode::read>(cgh, range<1>(size));
        auto dst_acc = buf_dst.template get_access<access::mode::discard_write>(cgh, range<1>(size));
        cgh.copy(src_acc, dst_acc);
        /*cgh.parallel_for(range<1>(size), [=] (item<1> it) {
            dst_acc[it[0]] = src_acc[it[0]];
        });*/
    });
}

template <class T>
inline void _M_fillwith(buffer<T, 1> &buf_dst, size_t beg, size_t end, T const &val) {
    default_queue().submit([&] (handler &cgh) {
        auto dst_acc = buf_dst.template get_access<access::mode::discard_write>(
            cgh, range<1>(end - beg), id<1>(beg));
        cgh.fill(dst_acc, val);
        /*cgh.parallel_for(range<1>(end - beg), [=] (item<1> it) {
            dst_acc[it[0]] = val;
        });*/
    });
}

template <class T>
    requires (std::is_trivially_copy_constructible_v<T> && std::is_trivially_destructible_v<T>)
struct vector {
    mutable buffer<T, 1> _M_buf;
    size_t _M_size;

    vector(vector const &) = default;
    vector &operator=(vector const &) = default;
    vector(vector &&) = default;
    vector &operator=(vector &&) = default;

    void resize(size_t size, T const &val = {}) {
        if (_M_size) {
            auto old_buf = std::exchange(_M_buf, buffer<T, 1>(_M_nozerosize(size)));
            auto n_trans = std::min(size, _M_size);
            _M_transfer<T>(old_buf, _M_buf, n_trans);
            if (size > _M_size)
                _M_fillwith(_M_buf, _M_size, size, val);
            _M_size = size;
        } else {
            _M_buf = buffer<T, 1>(_M_nozerosize(size));
            if (size)
                _M_fillwith(_M_buf, 0, size, val);
            _M_size = size;
        }
    }

    bool empty() const {
        return !_M_size;
    }

    size_t size() const {
        return _M_size;
    }

    void clear() {
        _M_buf = buffer<T, 1>(_M_nozerosize(0));
        _M_size = 0;
    }

    vector() : _M_buf(_M_nozerosize(0)), _M_size(0) {
    }

    explicit vector(size_t size, T const &val = {})
        : _M_buf(_M_nozerosize(size)), _M_size(size)
    {
        if (size)
            _M_fillwith(_M_buf, 0, size, val);
    }

    template <access::mode mode>
    auto get_access(auto &&cgh, auto &&...args) const {
        if constexpr (std::is_same_v<std::remove_cvref_t<decltype(cgh)>, host_handler>)
            return _M_buf.template get_access<mode>(std::forward<decltype(args)>(args)...);
        else
            return _M_buf.template get_access<mode>(cgh, std::forward<decltype(args)>(args)...);
    }

    buffer<T, 1> &get_buffer() const {
        return _M_buf;
    }

    template <class Vector>
    void _M_copy_to_vector(Vector &vec) const {
        size_t size = _M_size;
        if (size) {
            vec.reserve(size);
            auto hacc = _M_buf.template get_access<access::mode::read>();
            for (size_t i = 0; i < size; i++) {
                vec.push_back(hacc[i]);
            }
        }
    }

    template <class Vector = std::vector<T>>
    auto as_vector() {
        _M_as_vector<Vector, vector> vec(this);
        _M_copy_to_vector(vec);
        return vec;
    }

    template <class Vector = std::vector<T>>
    auto to_vector() const {
        Vector vec;
        _M_copy_to_vector<Vector>(vec);
        return vec;
    }

    inline auto move_vector() {
        return to_vector();
    }
};

}
}
ZENO_NAMESPACE_END

#endif