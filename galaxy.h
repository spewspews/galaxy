#include "body.h"
#include <SDL2/SDL.h>
#include <array>
#include <atomic>
#include <chrono>
#include <condition_variable>
#include <random>
#include <sstream>
#include <thread>

constexpr int nthreads = 4;

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

struct UI;
template <int n>
struct Threads;

struct Simulator {
	double dt{0.1}, dt²{dt * dt};

	void simulate(Galaxy& g, UI& ui);
	void pause(int);
	void unpause(int);
	void stop();
	friend void load(Galaxy&, UI&, Simulator&, std::istream&);

  private:
	std::atomic_bool paused_{false}, pause_{false}, stop_{false},
	    stopped_{false};
	std::condition_variable cvp_, cvpd_, cvstop_;
	std::mutex mup_, mupd_, mustop_;
	int pid_{-1};

	void simLoop(Galaxy&, UI&);
	void doPause();
	void doStop();
	void doStop(Threads<nthreads-1>&);
	void verlet(Galaxy&);
	static void calcForces(Galaxy&, BHTree&);
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

template <int n>
struct Threads {
	void start(Galaxy& g, BHTree &tree) {
		for(auto i = 0; i < n; ++i) {
			auto t = std::thread([this, i, &g, &tree] { calcForcesLoop(i, g, tree); });
			t.detach();
		}
	}

	void stop() {
		die_ = true;
		go();
	}

	void go() {
		running_ = n;
		for(auto i = 0; i < n; ++i) {
			go_[i] = true;
			cv_[i].notify_one();
		}
	}

	void wait() {
		if(running_ > 0) {
			std::unique_lock<std::mutex> lk(mudone_);
			while(running_ > 0)
				cvdone_.wait(lk);
		}
	}

  private:
	std::array<std::mutex, n> mu_;
	std::array<std::condition_variable, n> cv_;
	std::array<std::atomic_bool, n> go_;
	std::atomic_bool die_{false};
	std::mutex mudone_;
	std::condition_variable cvdone_;
	std::atomic_int running_;


	void calcForcesLoop(const int tid, Galaxy& g, BHTree& tree) {
		for(;;) {
			if(!go_[tid]) {
				std::unique_lock<std::mutex> lk(mu_[tid]);
				while(!go_[tid])
					cv_[tid].wait(lk);
			}
			if(die_)
				return;
			go_[tid] = false;
	
			auto nbody = g.bodies.size() / (n+1);
			auto start = g.bodies.begin() + nbody * tid;
			auto end = g.bodies.begin() + nbody * (tid + 1);
			for(auto& i = start; i < end; ++i)
				tree.calcforce(*i);
	
			if(--running_ == 0)
				cvdone_.notify_one();
		}
	}
};
