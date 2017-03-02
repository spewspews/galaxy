#include "nbody.h"

bool BHTree::insert(Body& nb, double size) {
	if(root.t == QB::empty) {
		root.t = QB::body;
		root.b = &nb;
		return true;
	}
	Vector q;
	auto qb = &root;
	for(;;) {
		if(qb->t == QB::body) {
			auto& b = *qb->b;
			auto qxy = b.p.x < q.x ? 0 : 1;
			qxy |= b.p.y < q.y ? 0 : 2;
			qb->t = QB::quad;
			if(!newQuad(*qb->q))
				return false;
			qb->q->fromBody(b);
			qb->q->c[qxy].t = QB::body;
			qb->q->c[qxy].b = &b;
		}
		auto& q = *qb->q;
		auto mass = q.mass + nb.mass;
		q.p = (q.p * q.mass + nb.p * nb.mass) / mass;
		q.mass = mass;

		auto qxy = nb.p.x < q.p.x ? 0 : 1;
		qxy |= nb.p.y < q.p.y ? 0 : 1;
		if(q.c[qxy].t == QB::empty) {
			q.c[qxy].t = QB::body;
			q.c[qxy].b = &nb;
			return true;
		}
		qb = &q.c[qxy];
		size /= 2;
		q.p.x += qxy & 1 ? size / 2 : -size / 2;
		q.p.y += qxy & 2 ? size / 2 : -size / 2;
	}
}

void BHTree::insert(Galaxy& g) {
Again:
	for(auto& b : g.bodies) {
		if(!insert(b, lim))
			goto Again;
	}
}
