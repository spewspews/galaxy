#include "flags.h"
#include "galaxy.h"

static constexpr double M_PI2 = 6.28318530718e0;

class RandGen {
	std::default_random_engine eng;
	unsigned long getSeed() {
		auto now = std::chrono::system_clock::now();
		auto time = now.time_since_epoch();
		auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(time);
		return ms.count();
	}

  public:
	RandGen() : eng{getSeed()} {}
	double operator()(double b, double t) {
		std::uniform_real_distribution<> dist{b, t};
		return dist(eng);
	}
};

Galaxy glxy;
RandGen randGen;
Vector gv;
double d = 100, drand, sz = 25, szrand, v, vrand, av, avrand, gs = 2000;
bool c;

void mkbodies() {
	for(auto x = -gs / 2; x < gs / 2; x += d)
		for(auto y = -gs / 2; y < gs / 2; y += d) {
			Vector p{x + randGen(-drand, drand), y + randGen(-drand, drand)};
			if(c && std::hypot(p.x, p.y) > gs / 2)
				continue;
			auto& b = glxy.newBody();
			b.p = p;
			b.v = Vector::polar(randGen(0, M_PI2), v + randGen(-vrand, vrand));
			b.v.x += gv.x - p.y * (av + randGen(-avrand, avrand)) / 1000;
			b.v.y += gv.y + p.x * (av + randGen(-avrand, avrand)) / 1000;
			b.size = sz + randGen(-szrand, szrand);
		}
}

int main(int argc, char** argv) {
	const flags::args args{argc, argv};
	mkbodies();
	for(auto& b : glxy.bodies)
		std::cout << b;
}
