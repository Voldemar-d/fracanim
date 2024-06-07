#include "fracdraw.h"

namespace fs = std::filesystem;

#define LOBYTE(w) ((uint8_t)(((uint32_t)(w)) & 0xff))

#define GetRValue(rgb) (LOBYTE(rgb))
#define GetGValue(rgb) (LOBYTE(((uint16_t)(rgb)) >> 8))
#define GetBValue(rgb) (LOBYTE((rgb)>>16))

#define RGB(r,g,b) ((uint32_t)(((uint8_t)(r)|((uint16_t)((uint8_t)(g))<<8))|(((uint32_t)(uint8_t)(b))<<16)))

// 'light cyan', 'yellow', 'red', 'orange', 'blue', 'cyan', 'magenta', 'green'
const uint32_t clr[] = {
	RGB(128,255,255),
	RGB(255,255,0),
	RGB(255,0,0),
	RGB(255,128,0),
	RGB(0,255,0),
	RGB(0,255,255),
	RGB(255,128,255),
	RGB(0,0,255),
};

void CFracDraw::DrawSinCos(pixColorMap& pcmap, pixBuf& pix, const int w, const int h,
	const double Coef1, const double Coef2) const
{
	int x, y;
	double fx, fy, d, f;
	srand(1);
	const double fw = w, fh = h, rm = 1.0 / double(RAND_MAX), coef1 = Coef1, coef2 = Coef2;
	uint32_t col = 0;
	for (int j = 0; j < 500; j++) {
		fx = double(rand()) * rm;
		fy = double(rand()) * rm;
		col = clr[rand() % std::size(clr)];
		for (int i = 0; i < 1000; i++) {
			x = int(fx * fw);
			y = int(fy * fh);
			AddPixel(pcmap, x, y, col, w, h);
			d = fx + sin(fy - coef2);
			fx = modf(d, &f);
			d = fy + cos(fx * coef1);
			fy = modf(d, &f);
		}
	}
	DrawPix(pcmap, pix, w, h, 5);
}

void CFracDraw::AddPixel(pixColorMap& pcmap, const int x, const int y, const uint32_t col, const int w, const int h) const
{
	if (x < 0 || y < 0 || x >= w || y >= h)
		return;
	const auto res1 = pcmap.find({ x, y });
	if (pcmap.end() == res1) {
		colorMap cmap; cmap[col]++;
		pcmap.emplace(std::make_pair(x, y), std::move(cmap));
	}
	else {
		auto& cmap = res1->second;
		auto res2 = cmap.find(col);
		if (cmap.end() == res2)
			cmap.emplace(col, 1);
		else
			cmap[col]++;
	}
}

void CFracDraw::DrawPix(pixColorMap& pcmap, pixBuf& pixb, const int w, const int h, const int mix) const
{
	const size_t sz = w * h;
	if (sz != pixb.size()) pixb.resize(sz);
	std::fill(pixb.begin(), pixb.end(), 0);
	int rb, gb, bb, rc, gc, bc, kc, kb;
	uint32_t col = 0;
	for (auto const& pix : pcmap) {
		const auto x = pix.first.first, y = pix.first.second;
		auto& v = pixb[y * w + x];
		const auto& cmap = pix.second;
		for (auto const& clr : cmap) {
			// color weight
			kc = clr.second;
			if (kc > mix) kc = mix;
			// background weight
			kb = mix - kc;
			// RGB of background
			rb = GetRValue(v); gb = GetGValue(v); bb = GetBValue(v);
			// RGB of color
			const auto& c = clr.first;
			rc = GetRValue(c); gc = GetGValue(c); bc = GetBValue(c);
			// mix color and background with weight
			rc = (rc * kc + rb * kb) / mix; if (rc > 255) rc = 255;
			gc = (gc * kc + gb * kb) / mix; if (gc > 255) gc = 255;
			bc = (bc * kc + bb * kb) / mix; if (bc > 255) bc = 255;
			v = RGB(rc, gc, bc);
		}
	}
}

void CFracDraw::Stop() {
	m_bStop = true;
	for (auto& th : m_thr) {
		if (th.joinable())
			th.join();
	}
	m_thr.clear();
}

void CFracDraw::SaveSteps(const int nthreads, std::string_view outfolder, const int w, const int h,
	const int nFrom1, const int nTo1, const int nFrom2, const int nTo2, const int nTotal)
{
	m_outFolder = outfolder;
	m_nCnt = 0;
	const int nStep = nthreads;
	std::cout << "Running " << nStep << " threads, press 'q' to exit:\n";
	m_bStop = false; m_nWorking = 0;
	for (int i = 0; i < nStep; i++) {
		std::promise<void> pr; std::future<void> fut = pr.get_future();
		m_thr.emplace_back(std::thread([this, w, h, i, nStep, nTotal, nFrom1, nTo1, nFrom2, nTo2, &pr]() {
			SaveStep(w, h, i, nStep, nTotal, nFrom1, nTo1, nFrom2, nTo2, std::move(pr));
			}));
		// wait until thread is started
		fut.get();
	}
}

void CFracDraw::SaveStep(const int w, const int h, const int nStart, const int nStep, const int nTotal,
	const int nFrom1, const int nTo1, const int nFrom2, const int nTo2, std::promise<void> pr)
{
	m_nWorking++;
	// signal thread started
	pr.set_value();
	pixColorMap map; pixBuf pix;
	double coef1, coef2;
	int i = nStart;
	const double dRange1 = double(nTo1) - double(nFrom1), dFrom1 = nFrom1,
		dRange2 = double(nTo2) - double(nFrom2), dFrom2 = nFrom2,
		dk = 1.0 / double(nTotal), dm = 1.0 / 1000.0;
	std::thread th;
	std::vector<uint8_t> buf(w * h * 3);
	while (i < nTotal) {
		if (m_bStop)
			break;
		if (i > m_nCnt)
			m_nCnt = i;
		coef1 = (dFrom1 + double(i) * dRange1 * dk) * dm;
		coef2 = (dFrom2 + double(i) * dRange2 * dk) * dm;
		DrawSinCos(map, pix, w, h, coef1, coef2);
		if (th.joinable())
			th.join();
		SaveImg(w, h, i, pix, buf.data(), th);
		map.clear();
		pix.clear();
		i += nStep;
	}
	if (th.joinable())
		th.join();
	if (m_nWorking > 0)
		m_nWorking--;
}

void CFracDraw::SaveImg(const int w, const int h, int i, const pixBuf& pix, uint8_t* buf, std::thread& th) {
	fs::path fpath(m_outFolder);
	std::string fname = std::format("image{:05d}.png", i);
	fpath.append(fname);
	std::promise<void> pr; std::future<void> fut = pr.get_future();
	auto rgba = (uint8_t*)pix.data();
	th = std::thread([this, &fpath, w, h, &rgba, &buf, &pr]() {
		write_png_proc(fpath.string(), w, h, rgba, buf, std::move(pr));
		});
	// wait for start saving image to file
	fut.get();
}
