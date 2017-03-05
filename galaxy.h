#include <cmath>
#include <cstdint>
#include <iostream>
#include <mutex>
#include <random>
#include <vector>

class RandCol {
	static const std::vector<uint32_t> cols;
	std::default_random_engine eng;
	std::uniform_int_distribution<int> dist;

  public:
	RandCol() : eng{0}, dist{0, static_cast<int>(cols.size())} {}
	uint32_t operator()() { return cols[dist(eng)]; }
};

class Vector {
  public:
	double x, y;

	Vector() : x{0}, y{0} {};
	Vector(double a, double b) : x{a}, y{b} {};

	static Vector polar(double ang, double mag) {
		return {std::cos(ang) * mag, std::sin(ang) * mag};
	}

	Vector& operator=(const Vector& v);

	Vector operator+(const Vector& v) const { return {x + v.x, y + v.y}; };
	Vector operator-(const Vector& v) const { return {x - v.x, y - v.y}; }

	Vector& operator+=(const Vector& v);
	Vector& operator-=(const Vector& v);

	Vector operator*(double m) const { return {x * m, y * m}; }
	Vector operator/(double m) const { return {x / m, y / m}; }

	Vector& operator/=(double);

	friend std::ostream& operator<<(std::ostream&, const Vector&);
};

class Body {
	static uint32_t getRandomColor();

  public:
	Vector p, v, a, newa;
	double size, mass;
	uint8_t r, g, b;

	Body(double s) : p{}, v{}, a{}, newa{}, size{s}, mass{s * s * s} {
		auto color = getRandomColor();
		r = static_cast<uint8_t>(color >> 3 & 0xff);
		g = static_cast<uint8_t>(color >> 2 & 0xff);
		b = static_cast<uint8_t>(color >> 1 & 0xff);
	}
	friend std::ostream& operator<<(std::ostream&, const Body&);
};

class Galaxy {
	static constexpr double defaultSize_ = 2.0;

  public:
	double limit;
	std::vector<Body> bodies;

	Galaxy() : limit{10} {};

	Body& newBody(double scale) {
		bodies.push_back({defaultSize_ * scale});
		return bodies.back();
	}
	Body& newBody() { return newBody(0); }
	void checkLimit(const Vector&);
	Vector center();
};
