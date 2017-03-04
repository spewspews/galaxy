#include "galaxy.h"

static constexpr uint32_t DWhite = 0xFFFFFFFF, DRed = 0xFF0000FF,
                          DGreen = 0x00FF00FF, DBlue = 0x0000FFFF,
                          DCyan = 0x00FFFFFF, DMagenta = 0xFF00FFFF,
                          DYellow = 0xFFFF00FF, DPaleyellow = 0xFFFFAAFF,
                          DDarkyellow = 0xEEEE9EFF, DDarkgreen = 0x448844FF,
                          DPalegreen = 0xAAFFAAFF, DMedgreen = 0x88CC88FF,
                          DPalebluegreen = 0xAAFFFFFF, DPaleblue = 0x0000BBFF,
                          DBluegreen = 0x008888FF, DGreygreen = 0x55AAAAFF,
                          DPalegreygreen = 0x9EEEEEFF,
                          DYellowgreen = 0x99994CFF, DGreyblue = 0x005DBBFF,
                          DPalegreyblue = 0x4993DDFF;

const std::vector<uint32_t> RandCol::cols = {
    DWhite,         DRed,        DGreen,         DCyan,        DMagenta,
    DYellow,        DPaleyellow, DDarkyellow,    DDarkgreen,   DPalegreen,
    DPalebluegreen, DPaleblue,   DPalegreygreen, DYellowgreen, DGreyblue,
    DPalegreyblue,  DBlue,       DBluegreen,     DGreygreen,   DMedgreen};

std::ostream& operator<<(std::ostream& os, const Vector& v) {
	os << "{" << v.x << "," << v.y << "}";
	return os;
}

uint32_t Body::getRandomColor() {
	static RandCol randCol;
	return randCol();
}

std::ostream& operator<<(std::ostream& os, const Body& b) {
	os << "BODY " << b.p << " " << b.v << " " << b.size << "\n";
	return os;
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
	for(auto& b : bodies) {
		gc += b.p * b.mass;
		gcv += b.v * b.mass;
		mass += b.mass;
	}
	gc /= mass;
	gcv /= mass;
	for(auto& b : bodies) {
		b.p -= gc;
		b.v -= gcv;
	}
	return gc;
}
