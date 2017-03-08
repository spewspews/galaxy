#include "galaxy.h"
#include "flags.h"

std::ostream& operator<<(std::ostream& os, const Point& p) {
	os << "{" << p.x << "," << p.y << "}";
	return os;
}

std::istream& operator>>(std::istream& is, Point& p) {
	char c;
	is >> c;
	if(c != '{') {
		is.setstate(std::ios::failbit);
		return is;
	}

	is >> p.x;
	is >> c;
	if(c != ',') {
		is.setstate(std::ios::failbit);
		return is;
	}

	is >> p.y;
	is >> c;
	if(c != '}')
		is.setstate(std::ios::failbit);
	return is;
}

Point& Point::operator=(const Point& p) {
	x = p.x;
	y = p.y;
	return *this;
}

Point& Point::operator/=(int s) {
	x /= s;
	y /= s;
	return *this;
}

Point& Point::operator+=(const Point& p) {
	x += p.x;
	y += p.y;
	return *this;
}

Point& Point::operator-=(const Point& p) {
	x -= p.x;
	y -= p.y;
	return *this;
}

void Simulator::pause() {
	pause_ = true;
	std::unique_lock<std::mutex> lk{mupd_};
	while(!paused_)
		cvpd_.wait(lk);
}

void Simulator::unpause() {
	pause_ = false;
	cvp_.notify_one();
	std::unique_lock<std::mutex> lk{mupd_};
	while(paused_)
		cvpd_.wait(lk);
}

void Simulator::simLoop(Galaxy& g, UI& ui) {
	BHTree tree;
	for(;;) {
		if(pause_) {
			paused_ = true;
			cvpd_.notify_one();
			std::unique_lock<std::mutex> lk{mup_};
			while(pause_)
				cvp_.wait(lk);
			paused_ = false;
			cvpd_.notify_one();
		}
		ui.draw(g);
		tree.calcforces(g);
		for(auto& b : g.bodies) {
			b.p += b.v * dt + b.a * dt² / 2;
			b.v += (b.a + b.newa) * dt / 2;
			g.checkLimit(b.p);
		}
	}
}

void Simulator::simulate(Galaxy& g, UI& ui) {
	t_ = std::thread{[this, &g, &ui] { simLoop(g, ui); }};
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

int main(int argc, char** argv) {
	Galaxy glxy;
	Simulator sim;
	UI ui{sim};

	const flags::args args(argc, argv);

	auto in = std::ref(std::cin);
	auto read = args.get<bool>("i", false);
	auto file = args.get<std::string>("f");
	if(read) {
		load(glxy, ui, sim, in);
		glxy.center();
	}
	sim.simulate(glxy, ui);
	ui.loop(glxy);
	std::cerr << "Error: ui loop exited\n";
	exit(1);
}
