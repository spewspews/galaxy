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

struct UI;
struct Threads;

struct Simulator {
	double dt{0.1}, dt²{dt * dt};
	friend struct Threads;

	Simulator(int nthreads) : nthreads_{nthreads} {}

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
	const int nthreads_;

	void simLoop(Galaxy&, UI&);
	void doPause();
	void doStop();
	void doStop(Threads&);

	void verlet(Galaxy& g, Body& b) const {
		b.p += b.v * dt + b.a * dt² / 2;
		b.v += (b.a + b.newa) * dt / 2;
		g.checkLimit(b.p);
	}

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

struct Threads {
	Threads(const Simulator& sim, Galaxy& glxy, BHTree& tree)
	    : sim_{sim}, n_{sim.nthreads_-1}, gomu_(n_), gocv_(n_), go_(n_, 0),
	      glxy_{glxy}, tree_{tree} {
		for(auto i = 0; i < n_; ++i) {
			auto t = std::thread([this, i] { calcForcesLoop(i); });
			t.detach();
		}
	}

	void stop() {
		die_ = true;
		go();
	}

	void go() {
		running_ = n_;
		for(auto tid = 0; tid < n_; ++tid) {
			gomu_[tid].lock();
			go_[tid] = 1;
			gomu_[tid].unlock();
			gocv_[tid].notify_one();
		}
	}

	void wait() {
		std::unique_lock<std::mutex> lk(runningmu_);
		while(running_ > 0)
			runningcv_.wait(lk);
	}

	void workReady(int tid) {
		std::unique_lock<std::mutex> lk(gomu_[tid]);
		while(!go_[tid])
			gocv_[tid].wait(lk);
		go_[tid] = 0;
	}

	void workDone() {
		runningmu_.lock();
		if(--running_ == 0) {
			runningmu_.unlock();
			runningcv_.notify_one();
		} else
			runningmu_.unlock();
	}

  private:
	const Simulator& sim_;
	const int n_;
	std::vector<std::mutex> gomu_;
	std::vector<std::condition_variable> gocv_;
	std::vector<int> go_;
	std::atomic_bool die_{false};
	std::mutex runningmu_;
	std::condition_variable runningcv_;
	int running_;
	Galaxy& glxy_;
	BHTree& tree_;

	void calcForcesLoop(const int);
};
