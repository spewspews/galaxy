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

std::array<std::mutex, 3> mu;
std::array<std::condition_variable, 3> cv;
std::array<std::atomic_bool, 3> go;
std::mutex mudone;
std::condition_variable cvdone;
std::atomic_int running;

void calcLoop(Galaxy& g, BHTree& tree, const int tid) {
	for(;;) {
		if(!go[tid]) {
			std::unique_lock<std::mutex> lk(mu[tid]);
			while(!go[tid])
				cv[tid].wait(lk);
		}
		go[tid] = false;

		auto nbody = g.bodies.size()/4;
		auto start = g.bodies.begin() + nbody*tid;
		auto end = g.bodies.begin() + nbody*(tid+1);
		for(auto& i = start; i < end; ++i)
			tree.calcforce(*i);

		--running;
		if(running == 0)
			cvdone.notify_one();
	}
}

void Simulator::simLoop(Galaxy& g, UI& ui) {
	BHTree tree;
	for(auto i = 0; i < 3; i++) {
		auto t = std::thread([&g, &tree, i] { calcLoop(g, tree, i); });
		t.detach();
	}
	for(;;) {
		if(pause_) {
			paused_ = true;
			cvpd_.notify_one();
			std::unique_lock<std::mutex> lk(mup_);
			while(pause_)
				cvp_.wait(lk);
			paused_ = false;
			cvpd_.notify_one();
		}

		ui.draw(g);

		tree.insert(g);

		running = 3;
		for(auto i = 0; i < 3; ++i) {
			go[i] = true;
			cv[i].notify_one();
		}

		auto nbody = g.bodies.size()/4;
		auto i = g.bodies.begin() + nbody*3;
		while(i < g.bodies.end())
			tree.calcforce(*i++);

		if(running > 0) {
			std::unique_lock<std::mutex> lk(mudone);
			while(running > 0)
				cvdone.wait(lk);
		}

		for(auto& b : g.bodies) {
			b.p += b.v * dt + b.a * dt² / 2;
			b.v += (b.a + b.newa) * dt / 2;
			g.checkLimit(b.p);
		}
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

void shutdown(int c) {
	SDL_Quit();
	exit(c);
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

	int n;
	args.get<int>(n, "n", 0);

	Simulator sim;
	UI ui(sim);

	Galaxy glxy;
	if(args.get("i")) {
		load(glxy, ui, sim, std::cin);
		glxy.center();
	}

	if(args.flags().size() > 0)
		usage();

	sim.simulate(glxy, ui);
	ui.loop(glxy);
	std::cerr << "Error: ui loop exited\n";
	shutdown(1);
}
