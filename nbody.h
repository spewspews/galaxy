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
	std::random_device r;
	static const std::vector<Uint32> cols;
	std::default_random_engine eng;
	std::uniform_int_distribution<int> dist;

  public:
	RandCol() : eng{r()}, dist{0, static_cast<int>(cols.size())} {}
	Uint32 operator()() { return dist(eng); }
};

class Point;

class Vector {
  public:
	double x, y;
	Vector& operator=(const Vector& v);
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
	Vector toVector() const;
};

class Body {
  public:
	Vector p, v, a, newa;
	double size, mass;
	Uint32 color;

	Body() : p{}, v{}, a{}, newa{}, size{1}, mass{1}, color{} {}
	void draw(bool, bool) const;
};

class Mouse {
	Uint32 buttons;
	void mkbody();
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
	std::vector<Body> bodies;

  public:
	Body& newBody() {
		bodies.push_back({});
		return bodies.back();
	}
	void draw() const;
};

extern SDL_Window* screen;
extern SDL_Renderer* renderer;
extern Mouse mouse;
extern Point orig;
extern Galaxy glxy;
extern double scale;
extern RandCol randCol;
