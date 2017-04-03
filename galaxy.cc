#include "galaxy.h"
#include "args.h"

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
bool doLoad;
int nthreads;

void usage() {
	std::cerr << "Usage: " << argv0 << " [-i] [-n]\n";
	exit(1);
}

int doFlags(args::Args& args) {
	if(args.get("help") || args.get("h"))
		usage();

	doLoad = args.get("i");
	args.get<int>(nthreads, "n", 4);

	if(args.flags().size() > 0) {
		usage();
		return -1;
	}
	return 0;
}

} // namespace flags

int main(int argc, char** argv) {
	flags::argv0 = argv[0];
	args::Args args(argc, argv);
	if(flags::doFlags(args) == -1)
		exit(1);

	Simulator sim(flags::nthreads);
	UI ui(sim);
	Galaxy glxy;

	if(flags::doLoad) {
		load(glxy, ui, sim, std::cin);
		glxy.center();
	}

	try {
		sim.simulate(glxy, ui);
		ui.loop(glxy);
	} catch(const std::runtime_error& e) {
		std::cerr << "Runtime error: " << e.what() << '\n';
		return 1;
	}
	return 0;
}
