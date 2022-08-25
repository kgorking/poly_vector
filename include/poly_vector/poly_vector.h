#ifndef POLY_VECTOR_H
#define POLY_VECTOR_H

#include <type_traits>
#include <memory>
#include <cstddef>
#include <vector>

#define PV_BASE(BASE) private: friend class kg::poly_vector<BASE>; virtual void pv_uninitialized_construct(BASE* /*dst*/) = 0
#define PV_IMPL(BASE) private: friend class kg::poly_vector<BASE>; void pv_uninitialized_construct(BASE* dst) override { kg::detail::pv_uninitialized_construct(dst, this); }

namespace kg {

namespace detail {
    template<typename To, typename From>
    constexpr void pv_uninitialized_construct(To* dst, From *src) {
        if constexpr(std::move_constructible<From>) {
            std::construct_at<From, From&&>(static_cast<From*>(dst), std::move(*src));
        } else /*if constexpr(std::is_copy_constructible_v<To>)*/ {
            static_assert(std::constructible_from<From, From const&>, "type is not copy- or move-constructible");
            std::construct_at<From, From const&>(static_cast<From*>(dst), *src);
        }
    }
}

template <typename Base>
class poly_vector {
    static_assert(std::is_polymorphic_v<Base>, "The base class must be polymorphic");
    static_assert(std::has_virtual_destructor_v<Base>, "The base class must have a virtual destructor");
    static_assert(requires { &Base::pv_uninitialized_construct; }, "Base does not have the required 'pv_uninitialized_construct' function. Add PV_BASE(class_name) to the end of it.");

public:
    ~poly_vector() {
        destroy_all();
    }

    void clear() {
        total_size = 0;
        destroy_all();
        bytes.clear();
        offsets.clear();
    }

    /*constexpr*/ Base& at(size_t const index) {
        auto const offset = offsets.at(index);
        return *std::launder(reinterpret_cast<Base*>(bytes.data() + offset));
    }

    template<typename T>
    /*constexpr*/ void add(T&& t) noexcept {
        size_t const newsize = total_size + sizeof(T);
        
        if (newsize > bytes.capacity()) {
            size_t const vecsize = calc_growth(newsize);
            std::vector<std::byte> newbytes(vecsize);

            for(size_t const offset : offsets) {
                auto src = std::launder(reinterpret_cast<Base*>(bytes.data() + offset));
                auto dst = std::launder(reinterpret_cast<Base*>(newbytes.data() + offset));

                src->pv_uninitialized_construct(dst);
                std::destroy_at(src);
            }

            bytes = std::move(newbytes);
        }
        
        Base* dest_t = std::launder(reinterpret_cast<Base*>(bytes.data() + total_size));
        detail::pv_uninitialized_construct(dest_t, &t);
        offsets.push_back(total_size);
        total_size += sizeof(T);
    }

    constexpr size_t size() const noexcept {
        return offsets.size();
    }

    constexpr size_t size_at(size_t const index) const {
        auto const offset = offsets.at(index);
        if (index == offsets.size())
            return total_size - offset;
        else
            return offsets.at(1 + index) - offset;
    }

private:
    void destroy_all() {
        for(size_t const offset : offsets) {
            auto src = std::launder(reinterpret_cast<Base*>(bytes.data() + offset));
            std::destroy_at(src);
        }
    }

    // lifted from msvc stl. 1.5x geometric growth
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

#endif //!POLY_VECTOR_H