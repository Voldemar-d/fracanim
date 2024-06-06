#include <cmath>
#include <cstdint>
#include <unordered_map>
#include <vector>
#include <atomic>
#include <thread>
#include <string>
#include <format>
#include <filesystem>
#include <iostream>

#include "pngwrite.h"

struct pair_hash
{
	template <class T1, class T2>
	std::size_t operator() (const std::pair<T1, T2>& pair) const {
		return std::hash<T1>()(pair.first) ^ std::hash<T2>()(pair.second);
	}
};

using colorMap = std::unordered_map<uint32_t, int>;
using pixColorMap = std::unordered_map<std::pair<int, int>, colorMap, pair_hash>;
using pixBuf = std::vector<uint32_t>;

class CFracDraw {
public:
	CFracDraw() {}
	~CFracDraw() {
		Stop();
	}
	void DrawSinCos(pixColorMap& pcmap, pixBuf& pix, const int w, const int h,
		const double Coef1, const double Coef2) const;
	void SaveSteps(const int nthreads, std::string_view outfolder, const int w, const int h,
		const int nFrom1, const int nTo1, const int nFrom2, const int nTo2, const int nTotal);
	void Stop();
	bool Finished() const {
		return (m_nWorking < 1);
	}
	int GetCount() const {
		return m_nCnt;
	}
protected:
	void AddPixel(pixColorMap& pcmap, const int x, const int y, const uint32_t col, const int w, const int h) const;
	void DrawPix(pixColorMap& pcmap, pixBuf& pixb, const int w, const int h, const int mix) const;
	void SaveStep(const int w, const int h, int nStart, int nStep, int nTotal, int nFrom1, int nTo1, int nFrom2, int nTo2, std::promise<void> pr);
	void SaveImg(const int w, const int h, int i, const pixBuf& pix, uint8_t* buf, std::thread& th);	
private:
	std::string m_outFolder;
	std::vector<std::thread> m_thr;
	std::atomic_bool m_bStop{ false };
	std::atomic_int m_nCnt{ 0 }, m_nWorking{ 0 };
};