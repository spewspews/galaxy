#include "nbody.h"

Galaxy glxy;
UI ui;

const std::vector<Uint32> RandCol::cols = {
    DWhite,         DRed,         DGreen,         DCyan,
    DMagenta,       DYellow,      DPaleyellow,    DDarkyellow,
    DDarkgreen,     DPalegreen,   DPalebluegreen, DPaleblue,
    DPalegreygreen, DYellowgreen, DGreyblue,      DPalegreyblue,
};

std::ostream& operator<<(std::ostream& os, const Point& p) {
	os << "Point{" << p.x << ", " << p.y << "}";
	return os;
}

std::ostream& operator<<(std::ostream& os, const Vector& v) {
	os << "Vector{" << v.x << ", " << v.y << "}";
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

Vector& Vector::operator=(const Vector& p) {
	x = p.x;
	y = p.y;
	return *this;
}

Vector& Vector::operator+=(const Vector& p) {
	x += p.x;
	y += p.y;
	return *this;
}

Vector& Vector::operator-=(const Vector& p) {
	x -= p.x;
	y -= p.y;
	return *this;
}
Vector& Vector::operator/=(double f) {
	x /= f;
	y /= f;
	return *this;
}

void Galaxy::checkLimit(const Vector& v) {
	auto f = fabs(v.x);
	if(f > limit / 2)
		limit = f * 2;
	f = fabs(v.y);
	if(f > limit / 2)
		limit = f * 2;
}

Vector Galaxy::center() {
	Vector gc, gcv;
	double mass = 0;
	for(auto& b : glxy.bodies) {
		gc += b.p * b.mass;
		gcv += b.v * b.mass;
		mass += b.mass;
	}
	gc /= mass;
	gcv /= mass;
	for(auto& b : glxy.bodies) {
		b.p -= gc;
		b.v -= gcv;
	}
	return gc;
}

void simulate() {
	BHTree tree;
	for(;;) {
		glxy.mutex.lock();
		std::cerr << "top of simulate loop\n";
		ui.draw(glxy);
		tree.insert(glxy);
		glxy.mutex.unlock();
		std::this_thread::sleep_for(std::chrono::microseconds{10});
	}
}

int main() {
	ui.init();
	ui.draw(glxy);
	std::thread simthread{simulate};
	ui.loop();
	std::cerr << "Error: ui loop exited\n";
	exit(1);
}
