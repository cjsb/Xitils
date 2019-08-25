#pragma once

#include <random>
#include "Utils.h"

namespace Xitils {

	class _SamplerMersenneTwister;
	class _SamplerXORShift;

	using Sampler = _SamplerXORShift;

	//---------------------------------------------------

	class _Sampler {
	public:
		virtual float randf() {
			return (float)rand() / 0xFFFFFFFF;
		}

		float randf(float max) {
			return randf() * max;
		}

		float randf(float min, float max) {
			float u = randf();
			return (1-u) * min + u * max;
		}

	protected:
		virtual unsigned int rand() = 0;
	};

	class _SamplerMersenneTwister : public _Sampler {
	public:
		
		_SamplerMersenneTwister(int seed) :
			gen(seed),
			dist(0, 0xFFFFFFFF - 1)
		{}

	protected:
		unsigned int rand() override {
			return dist(gen);
		}

	private:
		std::mt19937 gen;
		std::uniform_int_distribution<unsigned int> dist;
	};

	class _SamplerXORShift : public _Sampler {
	public:

		_SamplerXORShift(int seed) :
			x(seed)
		{}

	protected:
		unsigned int rand() override {
			x ^= x << 13;
			x ^= x >> 17;
			x ^= x << 5;
			return (x != 0xFFFFFFFF) ? x : randf(); // 0xFFFFFFFF �͊܂܂Ȃ��悤�ɂ���
		}

	private:
		int x;
	};

}