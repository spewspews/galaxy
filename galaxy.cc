#include "galaxy.h"
#include "args.h"

void CalcThread::loop() {
	for(;;) {
		if(!go) {
			std::unique_lock<std::mutex> lk(mu_);
			while(!go)
				cv.wait(lk);
		}
		if(par_.die)
			return;

		go = false;

		auto& bodies = par_.glxy.bodies;
		auto nbody = bodies.size()/par_.nthread;
		auto start = bodies.begin() + nbody*tid_;
		auto end = bodies.begin() + nbody*(tid_+1);

		for(auto& i = start; i < end; ++i)
			par_.tree->calcforce(*i);

		if(--par_.running == 0)
			par_.cvdone.notify_one();
	}
}

void Simulator::pause(int id) {
	if(pid_ != -1 && pid_ > id)
		return;
	pid_ = id;
	if(paused_)
		return;
	pause_ = true;
	std::unique_lock<std::mutex> lk(mupd_);
	while(!paused_)
		cvpd_.wait(lk);
}

void Simulator::unpause(int id) {
	if(!paused_ || pid_ != id)
		return;
	pid_ = -1;
	pause_ = false;
	cvp_.notify_one();
	std::unique_lock<std::mutex> lk(mupd_);
	while(paused_)
		cvpd_.wait(lk);
}

void Simulator::stop() {
	stop_ = true;
	if(!stopped_) {
		std::unique_lock<std::mutex> lk(mustop_);
		while(!stopped_)
			cvstop_.wait(lk);
	}
}

void Simulator::doPause() {
	paused_ = true;
	cvpd_.notify_one();
	std::unique_lock<std::mutex> lk(mup_);
	while(pause_)
		cvp_.wait(lk);
	paused_ = false;
	cvpd_.notify_one();
}

void Simulator::doStop() {
	stopped_ = true;
	cvstop_.notify_one();
}

void Simulator::calc(BHTree& tree) {
	par_.calc(tree);
	auto nbody = glxy_.bodies.size()/(nthreads_+1);
	auto i = glxy_.bodies.begin() + nbody*nthreads_;
	while(i < glxy_.bodies.end())
		tree.calcforce(*i++);
	par_.wait();
}

void Simulator::verlet(Body& b) {
	b.p += b.v * dt + b.a * dt² / 2;
	b.v += (b.a + b.newa) * dt / 2;
	glxy_.checkLimit(b.p);
}

void Simulator::simLoop(UI& ui) {
	BHTree tree;

	par_.start();

	for(;;) {
		if(pause_)
			doPause();
		if(stop_) {
			par_.stop();
			doStop();
			return;
		}
		ui.draw(glxy_);
		tree.insert(glxy_);
		calc(tree);
		for(auto& b : glxy_.bodies)
			verlet(b);
	}
}

void Simulator::simulate(UI& ui) {
	auto t = std::thread([this, &ui] { simLoop(ui); });
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

void usage() {
	std::cerr << "Usage: " << argv0 << " [-i] [-n]\n";
	exit(1);
}

bool doLoad;
int nthreads;

void doFlags(args::Args args) {
	if(args.get("help") || args.get("h"))
		usage();
	args.get<int>(nthreads, "n", 0);
	doLoad = args.get("i");
	if(args.flags().size() > 0) {
		usage();
	}
}

} // namespace flags

int main(int argc, char** argv) {
	flags::argv0 = argv[0];
	args::Args args(argc, argv);
	flags::doFlags(args);

	Galaxy glxy;
	Simulator sim(flags::nthreads, glxy);
	UI ui(sim);

	if(flags::doLoad) {
		load(glxy, ui, sim, std::cin);
		glxy.center();
	}

try {
	sim.simulate(ui);
	ui.loop(glxy);
} catch (const std::runtime_error& e) {
	std::cerr << "Runtime error: " << e.what() << '\n';
	return 1;
}
	return 0;
}

