#include "body.h"
#include <SDL2/SDL.h>
#include <array>
#include <atomic>
#include <chrono>
#include <condition_variable>
#include <random>
#include <thread>

void shutdown(int);

struct Point {
	int x, y;

	Point() : x{0}, y{0} {}
	Point(int a, int b) : x{a}, y{b} {}

	Point& operator=(const Point&);

	Point operator+(const Point& p) const { return Point{x + p.x, y + p.y}; }
	Point operator-(const Point& p) const { return Point{x - p.x, y - p.y}; }

	Point operator/(int s) const { return Point{x / s, y / s}; }

	Point& operator+=(const Point&);
	Point& operator-=(const Point&);

	Point& operator/=(int);

	friend std::ostream& operator<<(std::ostream&, const Point&);
	friend std::istream& operator>>(std::istream&, Point&);
};

struct Quad;

struct QB {
	enum { empty, body, quad } t;
	union {
		Quad* q;
		const Body* b;
	};
	QB() : t{empty} {}
};

struct Quad {
	Vector p;
	double mass;
	std::array<QB, 4> c;
	Quad() {}
	void setPosMass(const Body& b) {
		p = b.p;
		mass = b.mass;
	}
	friend std::ostream& operator<<(std::ostream&, const Quad&);
};

struct BHTree {
  public:
	BHTree() : quads_{5}, size_{quads_.size()}, ε_{500}, G_{1}, θ_{1} {};
	void calcforces(Galaxy&);

  private:
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
		if(size_ > 1000000) {
			std::cerr << "Too many quads\n";
			exit(1);
		}
		quads_.resize(size_);
	}
};

struct UI;

struct Simulator {
	double dt, dt²;

	Simulator()
	    : dt{0.1}, dt²{dt * dt}, paused_{false}, pause_{false}, pid_{-1} {};

	void simulate(Galaxy& g, UI& ui);
	void pause(int);
	void unpause(int);
	friend void load(Galaxy&, UI&, Simulator&, std::istream&);

  private:
	std::atomic_bool paused_, pause_;
	std::condition_variable cvp_, cvpd_;
	std::mutex mup_, mupd_;
	std::thread t_;
	int pid_;
	void simLoop(Galaxy& g, UI& ui);
};

struct Pauser {
	Pauser(Simulator& sim, int id) : sim_{sim}, id_{id} { sim_.pause(id_); }
	~Pauser() { sim_.unpause(id_); }

  private:
	Simulator& sim_;
	int id_;
};

struct Mouse {
	Mouse(UI& ui) : ui_{ui} {}
	Point p;
	Vector vp;

	void operator()(Galaxy&);
	void update();

  private:
	Uint32 buttons_;
	UI& ui_;

	void body(Galaxy&);
	void zoom(const Galaxy&);
	void move(const Galaxy&);
	void setSize(Body&, const Galaxy&);
	void setVel(Body&, const Galaxy&);
};

struct UI {
	bool showv, showa;

	UI(Simulator& s)
	    : showv{false}, showa{false}, sim_{s}, mouse_{*this}, scale_{30},
	      paused_{false} {
		init();
	}

	void draw(const Galaxy&) const;
	void draw(const Body&) const;
	void draw(const Body&, const Vector&) const;
	double defaultSize();
	void handleEvents(Galaxy&);
	void loop(Galaxy&);

	friend void load(Galaxy&, UI&, Simulator&, std::istream&);

  private:
	Simulator& sim_;
	friend struct Mouse;
	Mouse mouse_;
	Point orig_;
	double scale_;
	SDL_Window* screen_;
	SDL_Renderer* renderer_;
	bool paused_;

	Vector toVector(const Point&) const;
	Point toPoint(const Vector&) const;
	void center();
	void init();
};
