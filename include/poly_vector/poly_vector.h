#pragma once
#include <type_traits>
#include <memory>
#include <cstddef>
#include <vector>

#define PV_BASE(BASE) virtual void uninitialized_construct(BASE* /*dst*/) = 0
#define PV_IMPL(BASE) void uninitialized_construct(BASE* dst) override { kg::detail::uninitialized_construct(dst, this); }

namespace kg {

namespace detail {
    template<typename From, typename To>
    constexpr void uninitialized_construct(From* dst, To *src) {
        if constexpr(std::move_constructible<To>) {
            std::construct_at<To, To&&>(static_cast<To*>(dst), std::move(*src));
        } else /*if constexpr(std::is_copy_constructible_v<To>)*/ {
            static_assert(std::constructible_from<To, To&>, "type is not copy- or move-constructible");
            std::construct_at<To, To const&>(static_cast<To*>(dst), *src);
        }
    }
}

template <typename Base>
class poly_vector {
    static_assert(std::is_polymorphic_v<Base>, "The base class must be polymorphic");
    static_assert(std::has_virtual_destructor_v<Base>, "The base class must have a virtual destructor");

public:
    /*constexpr*/ Base& at(size_t const index) {
        auto const offset = offsets.at(index);
        return *reinterpret_cast<Base*>(bytes.data() + offset);
    }

    template<typename T>
    /*constexpr*/ void add(T&& t) noexcept {
        size_t const curr_offset = total_size;
        size_t const newsize = curr_offset + sizeof(T);
        
        if (newsize > bytes.capacity()) {
            size_t const vecsize = calc_growth(newsize);
            std::vector<std::byte> newbytes(vecsize);

            for(size_t const offset : offsets) {
                auto src = reinterpret_cast<Base*>(bytes.data() + offset);
                auto dst = reinterpret_cast<Base*>(newbytes.data() + offset);

                src->uninitialized_construct(dst);
                src->~Base();
            }

            bytes = std::move(newbytes);
        }
        
        T* dest_t = reinterpret_cast<T*>(bytes.data() + curr_offset);
        detail::uninitialized_construct<T,T>(dest_t, &t);
        offsets.push_back(curr_offset);
        total_size += sizeof(T);
    }

    constexpr size_t size() const noexcept {
        return offsets.size();
    }

private:
    // lifted from msvc stl
    constexpr size_t calc_growth(size_t const newsize) const {
        if (bytes.capacity() > (bytes.max_size() - bytes.capacity() / 2)) {
            return bytes.max_size();
        }

        size_t const geometric = bytes.capacity() + bytes.capacity() / 2;

        if (newsize >= geometric) {
            return newsize;
        }

        return geometric;
    }
private:
    std::size_t total_size = 0;
    std::vector<std::byte> bytes;
    std::vector<std::size_t> offsets;
};
} // namespace kg
