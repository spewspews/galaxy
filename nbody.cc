#include "nbody.h"
#include "flags.h"

std::ostream& operator<<(std::ostream& os, const Point& p) {
	os << "Point{" << p.x << ", " << p.y << "}";
	return os;
}

std::ostream& operator<<(std::ostream& os, const Quad& q) {
	os << "Quad{p[" << q.p << "]}";
	return os;
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
	std::unique_lock<std::mutex> lk(mupd_);
	while(!paused_)
		cvpd_.wait(lk);
}

void Simulator::unpause() {
	pause_ = false;
	cvp_.notify_one();
}

void Simulator::simloop(Galaxy& g, UI& ui) {
	BHTree tree;
	for(;;) {
		if(pause_) {
			paused_ = true;
			cvpd_.notify_one();
			std::unique_lock<std::mutex> lk(mup_);
			while(pause_)
				cvp_.wait(lk);
			paused_ = false;
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
	t_ = std::thread{[this, &g, &ui]() { simloop(g, ui); }};
}

void readglxy(Galaxy& g, std::istream& is) {
	auto i = 0;
	for(;;) {
		Body b{0.0};
		is >> b;
		if(is.fail()) {
			if(is.eof()) {
				is.clear();
				break;
			}
			std::cerr << "Body read failed\n";
			is.clear();
			continue;
		}
		i++;
		g.newBody() = b;
		g.checkLimit(b.p);
	}
}

int main(int argc, char** argv) {
	Galaxy glxy;
	Simulator sim;
	UI ui;

	const flags::args args(argc, argv);

	auto read = args.get<bool>("i", false);
	if(read) {
		readglxy(glxy, std::cin);
		glxy.center();
	}
	sim.simulate(glxy, ui);
	ui.loop(glxy, sim);
	std::cerr << "Error: ui loop exited\n";
	exit(1);
}
