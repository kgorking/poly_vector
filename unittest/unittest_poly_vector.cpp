#define CATCH_CONFIG_MAIN
#include "catch.hpp"
#include <poly_vector/poly_vector.h>

struct base_poly {
	PV_BASE(base_poly);

	virtual ~base_poly() = default;

	virtual int run() {
		return 0;
	};
};

struct impl1_poly : base_poly {
	int stuff = 1;

	impl1_poly(int s) : stuff(s) {}

	PV_IMPL(base_poly);

	int run() override {
		return stuff;
	};
};

template <size_t N>
struct impl2_poly : base_poly {
	char some_more_stuff[N] = {};
	int stuff = 99;
    std::vector<char> so_much_stuff;

    impl2_poly(impl2_poly &&) = delete;
    impl2_poly(impl2_poly const&) = default;
	impl2_poly(int s) : stuff(s), so_much_stuff(s) {}

	PV_IMPL(base_poly);

	int run() override {
		return stuff;
	};
};


TEST_CASE("poly_vector specification") {
	SECTION("A new poly_vector is empty") {
        kg::poly_vector<base_poly> vec;
        CHECK(0 == vec.size());
	}

	SECTION("An empty poly_vector") {
		SECTION("grows when data is added to it") {
            kg::poly_vector<base_poly> vec;
            vec.add(impl1_poly{2});

            CHECK(1 == vec.size());
            CHECK(2 == vec.at(0).run());
		}
	}

	SECTION("Adding data to non-empty pv") {
		SECTION("does not explode") {
            kg::poly_vector<base_poly> vec;
            vec.add(impl1_poly{2});
            vec.add(impl1_poly{3});

            CHECK(2 == vec.size());
            CHECK(3 == vec.at(1).run());
		}

		SECTION("works with different types") {
            kg::poly_vector<base_poly> vec;

            constexpr int num_values = 30;

            for (int i = 0; i < num_values; ++i) {
                if (i % 3)
                    vec.add(impl2_poly<3*3>{i});
                else if (i % 5)
                    vec.add(impl2_poly<3*5>{i});
                else if (i % 7)
                    vec.add(impl2_poly<3*7>{i});
                else
                    vec.add(impl1_poly{i});
            }

            CHECK(num_values == vec.size());

            for (int i = 0; i < num_values; ++i) {
                CHECK(i == vec.at(i).run());
            }
		}
	}
}
