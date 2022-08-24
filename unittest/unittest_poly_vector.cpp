#define CATCH_CONFIG_MAIN
#include "catch.hpp"
#include <poly_vector/poly_vector.h>

#if __cpp_lib_constexpr_vector && __cpp_constexpr_dynamic_alloc
#define CE_UT(t) static_assert((t))
#else
#define CE_UT(t) ((void)0)
#endif

struct base_poly {
	PV_BASE(base_poly);

	virtual ~base_poly() = default;

	virtual int run() {
		return 0;
	};

	virtual void const* address() const {
		return static_cast<void const*>(this);
	}
};

struct impl1_poly : base_poly {
	int stuff = 1;

	impl1_poly(int s) : stuff(s) {}

	PV_IMPL(impl1_poly, base_poly);

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

	PV_IMPL(impl2_poly, base_poly);

	int run() override {
		return stuff;
	};
};

TEST_CASE("poly_vector specification") {
	SECTION("A new poly_vector is empty") {
		auto const test = [] {
			kg::poly_vector<base_poly> vec;
			return 0 == vec.size();
		};
		CE_UT(test());
		CHECK(test());
	}

	SECTION("An empty poly_vector") {
		SECTION("grows when data is added to it") {
			auto const test = [] {
				kg::poly_vector<base_poly> vec;
				vec.add(impl1_poly{2});

				bool const size_matches = 1 == vec.size();
				bool const data_matches = 2 == vec.at(0).run();
				return size_matches && data_matches;
			};
			// CE_UT(test());
			CHECK(test());
		}
	}

	SECTION("Adding data to non-empty pv") {
		SECTION("does not explode") {
			auto const test = [] {
				kg::poly_vector<base_poly> vec;
				vec.add(impl1_poly{2});
				vec.add(impl1_poly{3});

				bool const size_matches = 2 == vec.size();
				bool const data_matches = 3 == vec.at(1).run();
				return size_matches && data_matches;
			};
			// CE_UT(test());
			CHECK(test());
		}

		SECTION("works with different types") {
			auto const test = [] {
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

				bool const size_matches = num_values == vec.size();
				if (!size_matches) {
					return false;
				}

				bool data_matches = true;
				for (int i = 0; i < num_values; ++i) {
					data_matches = (i == vec.at(i).run()) && data_matches;
				}

				return data_matches;
			};
			// CE_UT(test());
			CHECK(test());
		}
	}
}
