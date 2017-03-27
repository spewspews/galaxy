#include "galaxy.h"
#include "args.h"

void Threads::calcForcesLoop(const int tid) {
	for(;;) {
		if(flags_[tid].val == ThreadFlag::Val::wait) {
			std::unique_lock<std::mutex> lk(flags_[tid].mut);
			while(flags_[tid].val == ThreadFlag::Val::wait)
				flags_[tid].cv.wait(lk);
		}

		if(flags_[tid].val == ThreadFlag::Val::exit) {
			if(--running_ == 0)
				runningcv_.notify_one();
			return;
		}
		flags_[tid].val = ThreadFlag::Val::wait;

		auto nbody = bodies_.size() / (n_ + 1);
		auto start = bodies_.begin() + nbody * tid;
		auto end = start + nbody;
		for(auto& i = start; i < end; ++i)
			tree_.calcforce(*i);

		if(--running_ == 0)
			runningcv_.notify_one();
	}
}

void Simulator::pause(int id) {
	if(pid_ != -1 && id < pid_)
		return;
	pid_ = id;
	if(paused_)
		return;
	pause_ = true;
	std::unique_lock<std::mutex> lk(pausedmut_);
	while(!paused_)
		pausedcv_.wait(lk);
}

void Simulator::unpause(int id) {
	if(!paused_ || id < pid_)
		return;
	pid_ = -1;
	pause_ = false;
	pausecv_.notify_one();
	std::unique_lock<std::mutex> lk(pausedmut_);
	while(paused_)
		pausedcv_.wait(lk);
}

void Simulator::exit() {
	if(paused_) {
		pause_ = false;
		pausecv_.notify_one();
		std::unique_lock<std::mutex> lk(pausedmut_);
		while(paused_)
			pausedcv_.wait(lk);
	}
	exit_ = true;
	if(!exited_) {
		std::unique_lock<std::mutex> lk(exitedmut_);
		while(!exited_)
			exitedcv_.wait(lk);
	}
}

inline void Simulator::doPause() {
	paused_ = true;
	pausedcv_.notify_one();
	std::unique_lock<std::mutex> lk(pausemut_);
	while(pause_)
		pausecv_.wait(lk);
	paused_ = false;
	pausedcv_.notify_one();
}

void Simulator::calcForces(Galaxy& g, BHTree& tree) {
	auto nbody = g.bodies.size() / nthreads_;
	auto i = g.bodies.begin() + nbody * (nthreads_ - 1);
	while(i < g.bodies.end())
		tree.calcforce(*i++);
}

void Simulator::verlet(Galaxy& g) {
	for(auto& b : g.bodies) {
		b.p += b.v * dt + b.a * dt² / 2;
		b.v += (b.a + b.newa) * dt / 2;
		g.checkLimit(b.p);
	}
}

void Simulator::simLoop(Galaxy& g, UI& ui) {
	BHTree tree;
	Threads threads(nthreads_ - 1, g, tree);
	for(;;) {
		if(pause_)
			doPause();
		if(exit_) {
			threads.exit();
			exited_ = true;
			exitedcv_.notify_one();
			return;
		}

		ui.draw(g);

		tree.insert(g);

		threads.go();
		calcForces(g, tree);
		threads.wait();

		verlet(g);
	}
}

void Simulator::simulate(Galaxy& g, UI& ui) {
	auto t = std::thread([this, &g, &ui] { simLoop(g, ui); });
	t.detach();
}

void load(Galaxy& g, UI& ui, Simulator& sim, std::istream& is) {
	for(;;) {
		ReadCmd rc;
		is >> rc;
		Body b;
		switch(rc) {
		case ReadCmd::body:
			if(is >> b) {
				g.bodies.push_back(b);
				g.checkLimit(b.p);
			}
			break;
		case ReadCmd::orig:
			is >> ui.orig_;
			break;
		case ReadCmd::dt:
			is >> sim.dt;
			sim.dt² = sim.dt * sim.dt;
			break;
		case ReadCmd::scale:
			is >> ui.scale_;
			break;
		case ReadCmd::grav:
		case ReadCmd::nocmd:
			if(is.eof()) {
				is.clear();
				return;
			}
			break;
		}
	}
}

namespace flags {

char* argv0;
bool doLoad;
int nthreads;

void usage() {
	std::cerr << "Usage: " << argv0 << " [-i] [-n]\n";
	exit(1);
}

int doFlags(args::Args& args) {
	if(args.get("help") || args.get("h"))
		usage();

	doLoad = args.get("i");
	args.get<int>(nthreads, "n", 4);

	if(args.flags().size() > 0) {
		usage();
		return -1;
	}
	return 0;
}

} // namespace flags

int main(int argc, char** argv) {
	flags::argv0 = argv[0];
	args::Args args(argc, argv);
	if(flags::doFlags(args) == -1)
		exit(1);

	Simulator sim(flags::nthreads);
	UI ui(sim);
	Galaxy glxy;

	if(flags::doLoad) {
		load(glxy, ui, sim, std::cin);
		glxy.center();
	}

	try {
		sim.simulate(glxy, ui);
		ui.loop(glxy);
	} catch(const std::runtime_error& e) {
		std::cerr << "Runtime error: " << e.what() << '\n';
		return 1;
	}
	return 0;
}
