#include <SDL2/SDL.h>
#include <iostream>
#include <mutex>
#include <random>
#include <sstream>
#include <string>
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
	Point toPoint() const;
};

class Point {
  public:
	int x, y;
	Point() : x{0}, y{0} {}
	Point(SDL_Point& p) : x{p.x}, y{p.y} {}
	Point(int a, int b) : x{a}, y{b} {}

	Point& operator=(const Point&);
	Point& operator/=(int);
	Point operator+(const Point&) const;
	Vector toVector() const;
};

class Body {
	static Uint32 getRandomColor();
	Uint32 color;

  public:
	Vector p, v, a, newa;
	double size, mass;
	Uint8 r, g, b;

	Body()
	    : color{getRandomColor()}, p{}, v{}, a{}, newa{}, size{}, mass{},
	      r{static_cast<Uint8>(color >> 3 & 0xff)},
	      g{static_cast<Uint8>(color >> 2 & 0xff)},
	      b{static_cast<Uint8>(color >> 1 & 0xff)} {}
	void draw() const;
	void drawVel() const;
	void drawAcc() const;
};

class Mouse {
	Uint32 buttons;
	void body();
	void move();
	void setSize(Body&);
	void setVel(Body&);

  public:
	Point p;
	Vector vp;
	void operator()();
	void update();
};

class Galaxy {
  public:
	std::vector<Body> bodies;
	Body& newBody() {
		bodies.push_back({});
		return bodies.back();
	}
	void draw() const;
	Vector center();
};

class Quad;

class QB {
  public:
	enum { body, quad, empty } t;
	union {
		Quad* q;
		Body* b;
	};
	QB() : t{empty} {}
};

class Quad {
  public:
	Vector p;
	double mass;
	QB c[4];
	Quad() {}
	Quad(const Body& b) : p{b.p}, mass{b.mass} {}
};

class BHTree {
	std::vector<Quad> quads;
	QB root;
	bool insert(Body&, double);
	double lim;
	size_t size;
	size_t capacity;
	bool getQuad(Quad& q, const Body& b) {
		if(size == capacity) {
			capacity *= 2;
			quads.reserve(capacity);
			clear();
			return false;
		}
		quads.push_back({b});
		size++;
		q = quads.back();
		return true;
	}

  public:
	BHTree() : quads{5}, lim{10}, capacity{quads.capacity()} {};
	void insert(Galaxy&);
	void clear() {
		quads.clear();
		size = 0;
	}
};

extern SDL_Window* screen;
extern SDL_Renderer* renderer;
extern Mouse mouse;
extern Galaxy glxy;
extern double scale;
extern RandCol randCol;
extern bool showv, showa;
extern std::mutex glxyMutex;
extern Vector offset;
