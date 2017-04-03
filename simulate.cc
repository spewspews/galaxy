#include "galaxy.h"

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
