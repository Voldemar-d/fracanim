#pragma once

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
	CFracDraw(bool bMix) {
		m_bUseMap = !bMix;
	}
	~CFracDraw() {
		Stop();
	}
	void DrawSinCosMap(pixColorMap& pcmap, pixBuf& pix, const int w, const int h,
		const double Coef1, const double Coef2) const;
	void DrawSinCosMix(pixBuf& pix, const int w, const int h,
		const double Coef1, const double Coef2) const;
	void SaveSteps(const int nthreads, std::string_view outfolder, const int w, const int h,
		const double dFrom1, const double dTo1, const double dFrom2, const double dTo2, const int nTotal);
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
	void MixPixel(pixBuf& pix, const int x, const int y, const uint32_t col, const int w, const int h, const int mix) const;
	void SaveStep(const int w, const int h, const int nStart, const int nStep, const int nTotal,
		double dFrom1, double dTo1, double dFrom2, double dTo2, std::promise<void> pr);
	void SaveImg(const int w, const int h, const int i, const pixBuf& pix, uint8_t* buf, std::thread& th);
private:
	std::string m_outFolder;
	std::vector<std::thread> m_thr;
	std::atomic_bool m_bStop{ false };
	std::atomic_int m_nCnt{ 0 }, m_nWorking{ 0 };
	bool m_bUseMap = true;
};