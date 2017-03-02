#include <SDL2/SDL.h>
#include <array>
#include <chrono>
#include <iostream>
#include <mutex>
#include <random>
#include <thread>
#include <vector>

#define DBlack 0x000000FF
#define DWhite 0xFFFFFFFF
#define DRed 0xFF0000FF
#define DGreen 0x00FF00FF
#define DBlue 0x0000FFFF
#define DCyan 0x00FFFFFF
#define DMagenta 0xFF00FFFF
#define DYellow 0xFFFF00FF
#define DPaleyellow 0xFFFFAAFF
#define DDarkyellow 0xEEEE9EFF
#define DDarkgreen 0x448844FF
#define DPalegreen 0xAAFFAAFF
#define DMedgreen 0x88CC88FF
#define DDarkblue 0x000055FF
#define DPalebluegreen 0xAAFFFFFF
#define DPaleblue 0x0000BBFF
#define DBluegreen 0x008888FF
#define DGreygreen 0x55AAAAFF
#define DPalegreygreen 0x9EEEEEFF
#define DYellowgreen 0x99994CFF
#define DGreyblue 0x005DBBFF
#define DPalegreyblue 0x4993DDFF

class RandCol {
	static const std::vector<Uint32> cols;
	std::default_random_engine eng;
	std::uniform_int_distribution<int> dist;

  public:
	RandCol() : eng{0}, dist{0, static_cast<int>(cols.size())} {}
	Uint32 operator()() { return cols[dist(eng)]; }
};

class Point;

class Vector {
  public:
	double x, y;

	Vector() : x{0}, y{0} {};
	Vector(double a, double b) : x{a}, y{b} {};

	Vector& operator=(const Vector& v);
	Vector operator-(const Vector& v) const { return {x - v.x, y - v.y}; }
	Vector operator+(const Vector& v) const { return {x + v.x, y + v.y}; };
	Vector operator*(double m) const { return {x * m, y * m}; }
	Vector operator/(double m) const { return {x / m, y / m}; }
	Vector& operator+=(const Vector& v);
	Vector& operator-=(const Vector& v);
	Vector& operator/=(double);
	friend std::ostream& operator<<(std::ostream&, const Vector&);
};

class Point {
  public:
	int x, y;

	Point() : x{0}, y{0} {}
	Point(SDL_Point& p) : x{p.x}, y{p.y} {}
	Point(int a, int b) : x{a}, y{b} {}

	Point& operator=(const Point&);
	Point& operator/=(int);
	Point& operator+=(const Point&);
	Point operator+(const Point& p) const { return {x + p.x, y + p.y}; }
	Point operator-(const Point& p) const { return {x - p.x, y - p.y}; }
	friend std::ostream& operator<<(std::ostream&, const Point&);
};

class Body {
	Uint32 color_;

	static Uint32 getRandomColor();

  public:
	Vector p, v, a, newa;
	double size, mass;
	Uint8 r, g, b;

	Body(double scale)
	    : color_{getRandomColor()}, p{}, v{}, a{}, newa{}, size{2.0 / scale},
	      mass{scale * scale * scale},
	      r{static_cast<Uint8>(color_ >> 3 & 0xff)},
	      g{static_cast<Uint8>(color_ >> 2 & 0xff)},
	      b{static_cast<Uint8>(color_ >> 1 & 0xff)} {}

	void draw() const;
	void drawVel() const;
	void drawAcc() const;
	friend std::ostream& operator<<(std::ostream&, const Body&);
};

class Quad;

class QB {
  public:
	enum { empty, body, quad } t;
	union {
		Quad* q;
		const Body* b;
	};
	QB() : t{empty} {}
};

class Quad {
  public:
	Vector p;
	double mass;
	std::array<QB, 4> c;
	Quad() {}
	void setPosMass(const Body& b) {
		p = b.p;
		mass = b.mass;
	}
	void clearChild() { c.fill({}); }
	friend std::ostream& operator<<(std::ostream&, const Quad&);
};

class Galaxy {
  public:
	double limit;
	Point orig;
	double scale;
	std::vector<Body> bodies;
	std::mutex mutex;

	Galaxy() : limit{10}, orig{0, 0}, scale{10} {};

	Body& newBody() {
		bodies.push_back({scale});
		return bodies.back();
	}
	void checkLimit(const Vector&);
	void draw() const;
	Point center();
	void toVector(Vector&, const Point&) const;
	void toPoint(Point&, const Vector&) const;
	Vector toVector(const Point&) const;
	Point toPoint(const Vector&) const;
};

class BHTree {
	std::vector<Quad> quads_;
	QB root_;
	size_t i_, size_;
	bool insert(const Body&, double);
	Quad* getQuad(const Body&);

  public:
	BHTree() : quads_{5}, size_{quads_.size()} {};
	void insert(Galaxy&);
	void resize() {
		size_ *= 2;
		quads_.resize(size_);
	}
};

class Mouse {
	Uint32 buttons_;

	void body();
	void move();
	void setSize(Body&);
	void setVel(Body&);

  public:
	Point p;
	Vector vp;

	void operator()();
	void update();
	void updatePos(Sint32, Sint32);
};

extern SDL_Window* screen;
extern SDL_Renderer* renderer;
extern Mouse mouse;
extern bool showv, showa;
extern Galaxy glxy;
