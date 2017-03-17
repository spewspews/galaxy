#include "galaxy.h"

Quad* BHTree::getQuad(const Body& b) {
	if(i_ == size_) {
		return nullptr;
	}
	auto& q = quads_[i_++];
	q.setPosMass(b);
	q.c.fill(QB{});
	return &q;
}

void BHTree::calcforce(Body& b, QB qb, double size) {
	for(;;) {
		Vector d;
		double h;
		switch(qb.t) {
		case QB::Type::empty:
			return;
		case QB::Type::body:
			if(qb.b == &b)
				return;
			d = qb.b->p - b.p;
			h = std::hypot(std::hypot(d.x, d.y), ε_);
			b.newa += d * G_ / (h * h * h) * qb.b->mass;
			return;
		case QB::Type::quad:
			d = qb.q->p - b.p;
			h = std::hypot(d.x, d.y);
			if(h != 0.0 && size / h < θ_) {
				h = std::hypot(h, ε_);
				b.newa += d * G_ / (h * h * h) * qb.q->mass;
				return;
			}
			size /= 2;
			calcforce(b, qb.q->c[0], size);
			calcforce(b, qb.q->c[1], size);
			calcforce(b, qb.q->c[2], size);
			qb = qb.q->c[3];
			break; /* calcforce(b, q->q[3], size); */
		}
	}
}

void BHTree::calcforce(Body& b) {
	b.a = b.newa;
	b.newa = {0, 0};
	calcforce(b, root_, limit_);
}

bool BHTree::insert(const Body& nb, double size) {
	if(root_.t == QB::Type::empty) {
		root_.t = QB::Type::body;
		root_.b = &nb;
		return true;
	}
	double qx{0}, qy{0};
	auto qb = &root_;
	for(;;) {
		if(qb->t == QB::Type::body) {
			auto& b = *qb->b;
			auto qxy = b.p.x < qx ? 0 : 1;
			qxy |= b.p.y < qy ? 0 : 2;
			qb->t = QB::Type::quad;
			qb->q = getQuad(b);
			if(qb->q == nullptr)
				return false;
			qb->q->c[qxy].t = QB::Type::body;
			qb->q->c[qxy].b = &b;
		}
		auto& q = *qb->q;
		auto mass = q.mass + nb.mass;
		q.p = (q.p * q.mass + nb.p * nb.mass) / mass;
		q.mass = mass;

		auto qxy = nb.p.x < qx ? 0 : 1;
		qxy |= nb.p.y < qy ? 0 : 2;
		if(q.c[qxy].t == QB::Type::empty) {
			q.c[qxy].t = QB::Type::body;
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
	limit_ = g.limit;
Again:
	root_.t = QB::Type::empty;
	i_ = 0;
	for(auto& b : g.bodies) {
		if(!insert(b, g.limit)) {
			resize();
			goto Again;
		}
	}
}
