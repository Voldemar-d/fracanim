﻿#include "fracanim.h"
#include "inputparser.h"
#include "fracdraw.h"

namespace fs = std::filesystem;
namespace cr = std::chrono;

void printHelp(char** argv, const bool bFull) {
	fs::path fmain(argv[0]);
	std::cout << "Usage: " << fmain.filename() << " [options]" << '\n';
	if (!bFull) {
		std::cout << "Display help for all options:\n";
		std::cout << fmain.filename() << " -help\n";
		return;
	}
	std::cout << "\noptions can be:" << '\n';
	std::cout << "-help\t\t\tdisplay this help" << '\n';
	std::cout << "-width {N}\t\tset output image width in pixels, 1280 by default" << '\n';
	std::cout << "-height {N}\t\tset output image height in pixels, 720 by default" << '\n';
	std::cout << "-mix\t\t\tmix pixel color with background without pixel/color mapping (turned off by default)" << '\n';
	std::cout << "-outfolder {path}\tset output folder (will be created it doesn't exist) for saving image files" << '\n';
	std::cout << "-steps {N}\t\tset number of output images (animation steps), 1 by default" << '\n';
	std::cout << "-coef1 {v}\t\tset value (float) of coefficient 1, 1.0 by default" << '\n';
	std::cout << "-coef2 {v}\t\tset value (float) of coefficient 2, 0.0 by default" << '\n';
	std::cout << "-coef1end {v}\t\tset ending value (float) of coefficient 1, 2.0 by default" << '\n';
	std::cout << "-coef2end {v}\t\tset ending value (float) of coefficient 2, 0.5 by default" << '\n';
	std::cout << "-threads {N}\t\tset number of running threads: use -threads max to use CPU cores number,\nuse -threads half to use 1/2 CPU cores number (default) or specify a number, e.g. -threads 4" << '\n';
	std::cout << std::thread::hardware_concurrency() << " maximal threads available." << '\n';
	std::cout << '\n' << "Pressing 'q' stops writing image series." << '\n';
}

std::string getFullPath(std::string_view folder) {
	fs::path path(folder);
	if (path.has_root_path()) // already full path
		return std::string(folder);
	auto curpath = fs::absolute(path);
	return curpath.string();
}

int main(int argc, char* argv[])
{
	if (argc < 2) {
		printHelp(argv, false);
		return 0;
	}
	std::vector<std::string> all_args;
	if (argc > 1)
		all_args.assign(argv + 1, argv + argc);
	InputParser input(all_args);

	if (input.cmdOptionExists("-help")) {
		printHelp(argv, true);
		return 0;
	}

	std::string outfolder;
	if (input.cmdOptionExists("-outfolder")) {
		const auto& param = input.getThisOption();
		outfolder = getFullPath(param);
	}
	else
		outfolder = getFullPath(".");
	if (!fs::exists(outfolder) && !fs::create_directory(outfolder)) {
		std::cout << "Failed to create output folder: " << outfolder << '\n';
		return -1;
	}

	int width = 1280, height = 720;
	if (input.cmdOptionExists("-width")) {
		const auto& param = input.getThisOption();
		width = atoi(param.c_str());
	}
	if (input.cmdOptionExists("-height")) {
		const auto& param = input.getThisOption();
		height = atoi(param.c_str());
	}
	if (width < 2 || height < 2) {
		std::cout << "Width and height must be not less than 2 pixels\n";
		return -1;
	}
	const bool bMix = input.cmdOptionExists("-mix");

	int nsteps = 1;
	if (input.cmdOptionExists("-steps")) {
		const auto& param = input.getThisOption();
		nsteps = atoi(param.c_str());
	}
	if (nsteps < 1) nsteps = 1;

	const int nmaxthreads = std::thread::hardware_concurrency();
	int nthreads = 0;
	if (input.cmdOptionExists("-threads")) {
		const auto& param = input.getThisOption();
		if (param == "max")
			nthreads = nmaxthreads;
		else if (param == "half")
			nthreads = nmaxthreads / 2;
		else
			nthreads = atoi(param.c_str());
	}
	if (nthreads < 1)
		nthreads = nmaxthreads / 2;
	else if (nthreads > nmaxthreads)
		nthreads = nmaxthreads;
	if (nthreads > nsteps)
		nthreads = nsteps;

	double coef1 = 1.0, coef2 = 0.0, coef1end = 2.0, coef2end = 0.5;
	if (input.cmdOptionExists("-coef1")) {
		const auto& param = input.getThisOption();
		coef1 = atof(param.c_str());
	}
	if (input.cmdOptionExists("-coef1end")) {
		const auto& param = input.getThisOption();
		coef1end = atof(param.c_str());
	}
	if (input.cmdOptionExists("-coef2")) {
		const auto& param = input.getThisOption();
		coef2 = atof(param.c_str());
	}
	if (input.cmdOptionExists("-coef2end")) {
		const auto& param = input.getThisOption();
		coef2end = atof(param.c_str());
	}

	auto funcStop = []() {
#ifdef _WIN32
		if (_kbhit() && 'q' == _getch())
			return true;
#else
		auto kbhit = []() {
			struct termios oldt, newt;
			int ch, oldf;
			tcgetattr(STDIN_FILENO, &oldt);
			newt = oldt;
			newt.c_lflag &= ~(ICANON | ECHO);
			tcsetattr(STDIN_FILENO, TCSANOW, &newt);
			oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
			fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);
			ch = getchar();
			tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
			fcntl(STDIN_FILENO, F_SETFL, oldf);
			if (ch != EOF) {
				ungetc(ch, stdin);
				return std::make_pair(true, ch);
			}
			return std::make_pair(true, ch);
		};

		const auto [hit, ch] = kbhit();
		if (hit && 'q' == ch) return true;
#endif
		return false;
	};

	if (nsteps < 2) {
		fs::path fpath(outfolder);
		fpath.append("image.png");
		std::cout << "Generating image (" << (bMix ? "mix color" : "use pixel/color mapping") << ")...\n";
		CFracDraw frac;
		pixBuf pix;
		if (bMix)
			frac.DrawSinCosMix(pix, width, height, coef1, coef2);
		else {
			pixColorMap pcmap;
			frac.DrawSinCosMap(pcmap, pix, width, height, coef1, coef2);
		}
		std::vector<uint8_t> buf;
		write_png_file(fpath.string().c_str(), width, height, (uint8_t*)pix.data(), buf);
		std::cout << "Saved " << width << " x " << height << " image: " << fpath.string() << '\n';
	}
	else {
		std::cout << "Generating and saving " << nsteps << " images (" << width << " x " << height << ") to:\n";
		std::cout << outfolder << '\n';
		if (bMix)
			std::cout << "Mixing colors with background\n";
		else
			std::cout << "Using pixel/color mapping for mixing with background\n";
		bool bBreak = false;
		const auto tstart = cr::steady_clock::now();
		{
			CFracDraw frac(bMix); int cnt = -1;
			frac.SaveSteps(nthreads, outfolder, width, height, coef1, coef1end, coef2, coef2end, nsteps);
			for (;;) {
				if (funcStop()) {
					bBreak = true;
					break;
				}
				std::this_thread::sleep_for(cr::milliseconds{ 500 });
				if (frac.Finished())
					break;
				if (frac.GetCount() != cnt) {
					cnt = frac.GetCount();
					std::cout << "Saving image: " << cnt + 1 << "/" << nsteps << '\r';
				}
			}
			if (bBreak)
				std::cout << "\nFinishing...\n";
		}
		if (bBreak)
			std::cout << "Aborted by user.\n";
		else {
			const auto dur = cr::duration_cast<cr::milliseconds>(cr::steady_clock::now() - tstart);
			const auto sdur = std::format("{:%H:%M:%S}", dur);
			std::cout << "Finished saving " << nsteps << " images, time: " << sdur << '\n';
		}
	}

	return 0;
}
