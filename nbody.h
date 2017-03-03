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

	Vector operator+(const Vector& v) const { return {x + v.x, y + v.y}; };
	Vector operator-(const Vector& v) const { return {x - v.x, y - v.y}; }

	Vector& operator+=(const Vector& v);
	Vector& operator-=(const Vector& v);

	Vector operator*(double m) const { return {x * m, y * m}; }
	Vector operator/(double m) const { return {x / m, y / m}; }

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

	Point operator+(const Point& p) const { return {x + p.x, y + p.y}; }
	Point operator-(const Point& p) const { return {x - p.x, y - p.y}; }

	Point& operator+=(const Point&);

	Point& operator/=(int);

	friend std::ostream& operator<<(std::ostream&, const Point&);
};

class Body {
	static Uint32 getRandomColor();

  public:
	Vector p, v, a, newa;
	double size, mass;
	Uint8 r, g, b;

	Body(double s) : p{}, v{}, a{}, newa{}, size{s}, mass{s * s * s} {
		auto color = getRandomColor();
		r = static_cast<Uint8>(color >> 3 & 0xff);
		g = static_cast<Uint8>(color >> 2 & 0xff);
		b = static_cast<Uint8>(color >> 1 & 0xff);
	}
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
	static constexpr double defaultSize_ = 2.0;

  public:
	double limit;
	std::vector<Body> bodies;
	std::mutex mutex;

	Galaxy() : limit{10} {};

	Body& newBody(double scale) {
		bodies.push_back({defaultSize_ * scale});
		return bodies.back();
	}
	void checkLimit(const Vector&);
	Vector center();
};

class BHTree {
	std::vector<Quad> quads_;
	QB root_;
	size_t i_, size_;
	double ε_, G_, θ_;

	bool insert(const Body&, double);
	void insert(Galaxy&);
	Quad* getQuad(const Body&);
	void calcforces(Body&, QB, double);
	void resize() {
		size_ *= 2;
		quads_.resize(size_);
	}

  public:
	BHTree() : quads_{5}, size_{quads_.size()}, ε_{500}, G_{1}, θ_{1} {};
	void calcforces(Galaxy&);
};

class UI;

class Mouse {
	Uint32 buttons_;
	UI& ui_;

	void body(Galaxy&);
	void move(const Galaxy&);
	void setSize(Body&, const Galaxy&);
	void setVel(Body&, const Galaxy&);

  public:
	Mouse(UI& ui) : ui_{ui} {}
	Point p;
	Vector vp;

	void operator()(Galaxy&);
	void update();
};

class UI {
	friend class Mouse;
	Mouse mouse_;
	Point orig_;
	double scale_;
	SDL_Window* screen_;
	SDL_Renderer* renderer_;

	Point toPoint(const Vector&) const;
	Vector toVector(const Point&) const;

  public:
	bool showv, showa;
	UI() : mouse_{*this}, orig_{0, 0}, scale_{10}, showv{false}, showa{false} {}
	void draw(const Galaxy&) const;
	void draw(const Body&) const;
	void draw(const Body&, const Vector&) const;
	double defaultSize();
	void init();
	void loop(Galaxy& g);
};
