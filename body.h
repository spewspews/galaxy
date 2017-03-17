#include <cmath>
#include <cstdint>
#include <iostream>
#include <mutex>
#include <random>
#include <vector>

struct RandCol {
	static const std::vector<uint32_t> cols;
	std::default_random_engine eng;
	std::uniform_int_distribution<int> dist;

	RandCol() : eng{0}, dist{0, static_cast<int>(cols.size())} {}
	uint32_t operator()() { return cols[dist(eng)]; }
};

enum struct ReadCmd { body, orig, dt, scale, grav, nocmd };

std::istream& operator>>(std::istream&, ReadCmd&);

struct Vector {
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
	friend std::istream& operator>>(std::istream&, Vector&);
};

struct Body {
	Vector p, v, a, newa;
	double size, mass;
	uint8_t r, g, b;

	Body(double s) : p{}, v{}, a{}, newa{}, size{s}, mass{s * s * s} {
		auto color = getRandomColor();
		r = static_cast<uint8_t>(color >> 3 & 0xff);
		g = static_cast<uint8_t>(color >> 2 & 0xff);
		b = static_cast<uint8_t>(color >> 1 & 0xff);
	}
	Body() : Body{0} {}

	friend std::ostream& operator<<(std::ostream&, const Body&);
	friend std::istream& operator>>(std::istream&, Body&);

  private:
	static uint32_t getRandomColor();
};

struct Galaxy {
	double limit{10};
	std::vector<Body> bodies;

	void checkLimit(const Vector& v) {
		auto f = fabs(v.x);
		if(f > limit / 2)
			limit = f * 2;
		f = fabs(v.y);
		if(f > limit / 2)
			limit = f * 2;
	}
	Vector center();
};
