#include <poly_vector/poly_vector.h>
#include <iostream>
#include <format>

class base_interface {
public:
	virtual ~base_interface() = default;
	virtual void run() = 0;

	void* address() {
		return static_cast<void*>(this);
	}

	PV_BASE(base_interface);
};

class impl1 : public base_interface {
	int a = rand();
	int b = rand();
	int c = rand();
public:
	void run() override {
		std::cout << std::format("1: {} - {}, {}, {}\n", address(), a, b, c);
	}

	PV_IMPL(base_interface);
};

class impl2 : public base_interface {
	char dummy[53];
	char e = '5';
	float f = 6.7f;
	
public:
	void run() override {
		std::cout << std::format("2: {} - {}, {}\n", address(), e, f);
	}

	PV_IMPL(base_interface);
};

int main() {
	constexpr int count = 20;

	kg::poly_vector<base_interface> pv;

	for (int i=0; i<count; i++) {
		if (rand() & 1)
			pv.add(impl1{});
		else
			pv.add(impl2{});
	}

	for(size_t i=0; i<pv.size(); ++i) {
		pv.at(i).run();
	}

	for(size_t i=0; i<pv.size(); ++i) {
		pv.at(i).run();
	}
}
