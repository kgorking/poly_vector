#define CATCH_CONFIG_MAIN
#include "catch.hpp"
#include <poly_vector/poly_vector.h>

// A type to count constructions and destructions
struct ctr_counter {
	inline static size_t def_ctr_count = 0;
	inline static size_t ctr_count = 0;
	inline static size_t copy_count = 0;
	inline static size_t move_count = 0;
	inline static size_t dtr_count = 0;

	ctr_counter() noexcept {
		def_ctr_count++;
		ctr_count++;
	}
	ctr_counter(ctr_counter const& /*other*/) {
		copy_count++;
		ctr_count++;
	}
	ctr_counter(ctr_counter&& /*other*/) noexcept {
		move_count++;
		ctr_count++;
	}
	~ctr_counter() {
		dtr_count++;
	}

	ctr_counter& operator=(ctr_counter&&) = default;
	ctr_counter& operator=(ctr_counter const&) = default;
};

class test_base {
public:
	virtual ~test_base() = default;

	virtual int run() {
		return 0;
	};

	PV_BASE(test_base);
};

class impl1 : public test_base {
	int stuff = 1;

public:
	explicit impl1(int s) : stuff(s) {}

	int run() override {
		return stuff;
	};

	PV_IMPL(test_base);
};

template <size_t N>
class impl2 : public test_base {
	char some_more_stuff[N] = {};
	int stuff = 99;
    std::vector<char> so_much_stuff;

public:
	explicit impl2(int s) : stuff(s), so_much_stuff(s) {}

	int run() override {
		return stuff;
	};

	PV_IMPL(test_base);
};

class impl3 : public test_base {
	ctr_counter counter{};
	PV_IMPL(test_base);
};


TEST_CASE("poly_vector specification") {
	SECTION("A new poly_vector is empty") {
        kg::poly_vector<test_base> vec;
        CHECK(0 == vec.size());
	}

	SECTION("An empty poly_vector") {
		SECTION("grows when data is added to it") {
            kg::poly_vector<test_base> vec;

            impl1 t{2};
            vec.add(t);
            CHECK(1 == vec.size());
            CHECK(2 == vec.at(0).run());

            impl1 u{3};
            vec.add(u);
            CHECK(2 == vec.size());
            CHECK(3 == vec.at(1).run());
		}
	}

	SECTION("Adding data to non-empty pv") {
		SECTION("works with different types") {
            kg::poly_vector<test_base> vec;

            constexpr int num_values = 56;

            for (int i = 0; i < num_values; ++i) {
                if (i % 7)
                    vec.add(impl2<3*7>{i});
                else if (i % 5)
                    vec.add(impl2<3*5>{i});
                else if (i % 3)
                    vec.add(impl2<3*3>{i});
                else
                    vec.add(impl1{i});
            }

            CHECK(num_values == vec.size());

            for (int i = 0; i < num_values; ++i) {
                CHECK(i == vec.at(i).run());
            }
		}
	}

	SECTION("data is destructed properly") {
        {
            kg::poly_vector<test_base> pv;
            pv.add(impl3{});
        }
        CHECK(ctr_counter::ctr_count == 2); // construction + move
        CHECK(ctr_counter::def_ctr_count == 1); // default construction
        CHECK(ctr_counter::copy_count == 0);
        CHECK(ctr_counter::move_count == 1); // internal move on resize
        CHECK(ctr_counter::dtr_count == 2);

        {
            kg::poly_vector<test_base> pv;
            pv.add(impl3{});
        }
        CHECK(ctr_counter::ctr_count == 4); // construction + move
        CHECK(ctr_counter::def_ctr_count == 2); // default construction
        CHECK(ctr_counter::copy_count == 0);
        CHECK(ctr_counter::move_count == 2); // internal move on resize
        CHECK(ctr_counter::dtr_count == 4);
    }
}
