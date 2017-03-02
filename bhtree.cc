#include "nbody.h"

Quad* BHTree::getQuad(const Body& b) {
	std::cerr << "BHTree::getQuad: i_ size_ " << i_ << " " << size_ << "\n";
	if(i_ == size_) {
		return nullptr;
	}
	auto& q = quads_[i_++];
	q.setPosMass(b);
	q.clearChild();
	return &q;
}

bool BHTree::insert(const Body& nb, double size) {
	std::cerr << "BHTree::insert entering with &nb nb " << &nb << " " << nb
	          << "\n";
	if(root_.t == QB::empty) {
		root_.t = QB::body;
		root_.b = &nb;
		return true;
	}
	double qx{0}, qy{0};
	auto qb = &root_;
	for(;;) {
		std::cerr << "BHTree::insert qx qy " << qx << " " << qy << "\n";
		if(qb->t == QB::body) {
			auto& b = *qb->b;
			auto qxy = b.p.x < qx ? 0 : 1;
			qxy |= b.p.y < qy ? 0 : 2;
			std::cerr << "BHTree::insert found body b qxy " << b << " " << qxy
			          << "\n";
			qb->t = QB::quad;
			qb->q = getQuad(b);
			if(qb->q == nullptr)
				return false;
			qb->q->c[qxy].t = QB::body;
			qb->q->c[qxy].b = &b;
		}
		auto& q = *qb->q;
		auto mass = q.mass + nb.mass;
		q.p = (q.p * q.mass + nb.p * nb.mass) / mass;
		q.mass = mass;

		auto qxy = nb.p.x < qx ? 0 : 1;
		qxy |= nb.p.y < qy ? 0 : 1;
		std::cerr << "BHTree::insert qxy " << qxy << "\n";
		if(q.c[qxy].t == QB::empty) {
			q.c[qxy].t = QB::body;
			q.c[qxy].b = &nb;
			return true;
		}
		qb = &q.c[qxy];
		size /= 2;
		qx += qxy & 1 ? size / 2 : -size / 2;
		qy += qxy & 2 ? size / 2 : -size / 2;
	}
}

void BHTree::insert(Galaxy& g) {
Again:
	std::cerr << "BHTree::insert(Galaxy&): g.bodies.size() " << g.bodies.size()
	          << "\n";
	root_.t = QB::empty;
	i_ = 0;
	for(auto& b : g.bodies) {
		if(!insert(b, g.limit)) {
			std::cerr << "BHTree::insert(Galaxy&):  need to resize\n";
			resize();
			goto Again;
		}
	}
}
