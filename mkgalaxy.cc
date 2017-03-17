#include "body.h"
#include "flags.h"

static constexpr double M_PI2 = 6.28318530718e0;

class RandGen {
	std::default_random_engine eng;
	unsigned int getSeed() {
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

struct Range {
	double v, rand;
	Range() : v{0}, rand{0} {}
	Range(double f) : v{f}, rand{0} {}
	Range(double f, double frand) : v{f}, rand{frand} {}
};

Galaxy old, glxy;
RandGen randGen;
Vector o, gv;
Range d{100}, sz{25}, v, av;
double gs{2000};
bool c{true};

void mkbodies() {
	for(auto x = -gs / 2; x < gs / 2; x += d.v) {
		for(auto y = -gs / 2; y < gs / 2; y += d.v) {
			Vector p{x + randGen(-d.rand, d.rand),
			         y + randGen(-d.rand, d.rand)};
			if(c && std::hypot(p.x, p.y) > gs / 2)
				continue;
			Body b{sz.v + randGen(-sz.rand, sz.rand)};
			b.p = p;
			b.v = Vector::polar(randGen(0, M_PI2),
			                    v.v + randGen(-v.rand, v.rand));
			b.v.x += gv.x - p.y * (av.v + randGen(-av.rand, av.rand)) / 1000;
			b.v.y += gv.y + p.x * (av.v + randGen(-av.rand, av.rand)) / 1000;
			glxy.bodies.push_back(b);
		}
	}
}

std::istream& operator>>(std::istream& is, Range& r) {
	is >> r.v;
	if(!is.good())
		return is;

	char c{'Z'};
	is >> c;
	if(c == '+') {
		is >> r.rand;
		return is;
	}

	return is;
}

void load(Galaxy& g, std::istream& is) {
	for(;;) {
		ReadCmd rc;
		is >> rc;
		Body b;
		switch(rc) {
		default:
			break;
		case ReadCmd::body:
			if(is >> b)
				g.bodies.push_back(b);
			break;
		case ReadCmd::nocmd:
			if(is.eof()) {
				is.clear();
				return;
			}
			break;
		}
	}
}

void doArgs(flags::args& args) {
	if(args.get("i"))
		load(old, std::cin);

	args.get<Range>(d, "d", {100, 0});
	args.get<Range>(sz, "sz", {25, 0});
	args.get<Range>(v, "v");
	args.get<Range>(av, "av");

	args.get<Vector>(o, "o");
	args.get<Vector>(gv, "gv");

	c = !args.get("sq");
}

int main(int argc, char** argv) {
	flags::args args{argc, argv};
	doArgs(args);
	auto& arg = args.positional();
	if(arg.size() == 1) {
		std::istringstream ss(arg[0]);
		ss >> gs;
	}

	mkbodies();

	for(auto& b : old.bodies)
		std::cout << "BODY " << b << "\n";

	for(auto& b : glxy.bodies) {
		b.p += o;
		std::cout << "BODY " << b << "\n";
	}
}
