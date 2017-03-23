#include "body.h"
#include <SDL2/SDL.h>
#include <array>
#include <atomic>
#include <chrono>
#include <condition_variable>
#include <random>
#include <sstream>
#include <thread>

using Point = Linear<int>;

struct Quad;

struct QB {
	enum struct Type { empty, body, quad } t;
	union {
		Quad* q;
		const Body* b;
	};
	QB() : t{Type::empty} {}
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
	BHTree() : quads_{5}, size_{quads_.size()}, ε_{500}, G_{1}, θ_{1} {};
	void insert(Galaxy&);
	void calcforce(Body&);

  private:
	std::vector<Quad> quads_;
	QB root_;
	size_t i_, size_;
	double ε_, G_, θ_, limit_;

	bool insert(const Body&, double);
	Quad* getQuad(const Body&);
	void calcforce(Body&, QB, double);
	void resize() {
		size_ *= 2;
		if(size_ > 10000000) {
			std::cerr << "Too many quads\n";
			exit(1);
		}
		quads_.resize(size_);
	}
};

struct Parallel;

struct CalcThread {
	std::atomic_bool go{false};
	std::condition_variable cv;

	CalcThread(int tid, Parallel& par) : tid_{tid}, par_{par} {}
	CalcThread(CalcThread&& ct) : tid_{ct.tid_}, par_{ct.par_} {}

	void start() {
		auto t = std::thread([this] { loop(); });
		t.detach();
	}

  private:
	const int tid_;
	std::mutex mu_;
	Parallel& par_;

	void loop();
};

struct Parallel {
	const int nthread;
	std::atomic_int running;
	std::atomic_bool die;
	std::condition_variable cvdone;
	BHTree* tree;
	Galaxy& glxy;

	Parallel(int n, Galaxy& g) : nthread{n}, glxy{g} {
		for(auto i = 0; i < nthread; ++i)
			threads_.emplace_back(i, *this);
	}

	void start() {
		for(auto& t : threads_)
			t.start();
	}

	void calc(BHTree& t) {
		tree = &t;
		running = nthread;
		for(auto& t : threads_) {
			t.go = true;
			t.cv.notify_one();
		}
	}

	void stop() {
		die = true;
		for(auto& t : threads_) {
			t.go = true;
			t.cv.notify_one();
		}
	}

	void wait() {
		if(running > 0) {
			std::unique_lock<std::mutex> lk(mudone_);
			while(running > 0)
				cvdone.wait(lk);
		}
	}

  private:
	std::vector<CalcThread> threads_;
	std::mutex mudone_;
};

struct UI;

struct Simulator {
	double dt{0.1}, dt²{dt*dt};

	Simulator(int nthreads, Galaxy& g) : nthreads_{nthreads}, glxy_{g}, par_{nthreads, g} {};

	void simulate(UI&);
	void pause(int);
	void unpause(int);
	void stop();
	friend void load(Galaxy&, UI&, Simulator&, std::istream&);

  private:
	const int nthreads_;
	Galaxy& glxy_;
	std::atomic_bool paused_{false}, pause_{false}, stop_{false}, stopped_{false};
	std::condition_variable cvp_, cvpd_, cvstop_;
	std::mutex mup_, mupd_, mustop_;
	int pid_{-1};
	Parallel par_;

	void simLoop(UI&);
	void calc(BHTree&);
	void goThreads();
	void doStop();
	void doPause();
	void verlet(Body&);
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
	UI(Simulator& s)
	    : showv_{false}, showa_{false}, sim_{s}, mouse_{*this}, scale_{30},
	      paused_{false} {
		init();
	}

	void draw(const Galaxy&) const;
	void draw(const Body&) const;
	void draw(const Body&, const Vector&, double) const;
	void loop(Galaxy&);

	friend void load(Galaxy&, UI&, Simulator&, std::istream&);

  private:
	bool showv_, showa_;
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
	int handleEvents(Galaxy&);
	int keyboard(SDL_Keycode&);
};
