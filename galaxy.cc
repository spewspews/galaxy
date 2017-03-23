#include "galaxy.h"
#include "args.h"

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

void Simulator::doStop(Threads<nthreads-1>& thr) {
	thr.stop();
	stopped_ = true;
	cvstop_.notify_one();
}

void Simulator::calcForces(Galaxy& g, BHTree& tree) {
	auto nbody = g.bodies.size() / nthreads;
	auto i = g.bodies.begin() + nbody * (nthreads-1);
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
	Threads<nthreads-1> thr;
	thr.start(g, tree);
	for(;;) {
		if(pause_) 
			doPause();
		if(stop_) {
			doStop(thr);
			return;
		}

		ui.draw(g);

		tree.insert(g);

		thr.go();
		calcForces(g, tree);
		thr.wait();

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

char* argv0;

void usage() {
	std::cerr << "Usage: " << argv0 << " [-i] [-n]\n";
	exit(1);
}

int main(int argc, char** argv) {
	argv0 = argv[0];
	args::Args args(argc, argv);
	if(args.get("help") || args.get("h"))
		usage();

	Simulator sim;
	UI ui(sim);
	Galaxy glxy;
	if(args.get("i")) {
		load(glxy, ui, sim, std::cin);
		glxy.center();
	}
	if(args.flags().size() > 0)
		usage();

	try {
		sim.simulate(glxy, ui);
		ui.loop(glxy);
	} catch(const std::runtime_error& e) {
		std::cerr << "Runtime error: " << e.what() << '\n';
		return 1;
	}
	return 0;
}
