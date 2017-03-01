#include <SDL2/SDL.h>
#include <iostream>
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
	Vector& operator=(const Vector& v);
	Vector operator-(const Vector& v) const;
	Vector& operator+=(const Vector& v);
	Vector& operator-=(const Vector& v);
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
	void draw(bool, bool) const;
	void drawVel() const;
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
	void draw(bool, bool) const;
};

extern SDL_Window* screen;
extern SDL_Renderer* renderer;
extern Mouse mouse;
extern Galaxy glxy;
extern double scale;
extern RandCol randCol;
