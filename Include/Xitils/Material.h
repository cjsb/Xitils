#pragma once

#include "Utils.h"
#include "Vector.h"

namespace Xitils {

	class Material {
	public:
		bool specular;

		// bsdf_cos �̒l��Ԃ�
		// �X�y�L�����̕��̂ł̓f���^�֐��ɂȂ�̂Ŏ������Ȃ�
		virtual Vector3f eval(const Vector3f& wo, const Vector3f& wi, const Vector3f& n) const {
			NOT_IMPLEMENTED;
			return Vector3f();
		}

		// bsdf_cos �̒l��Ԃ��Awi �̃T���v�����O���s��
		// �X�y�L�����̕��̂ł̓f���^�֐��ɂȂ�̂Ŏ������Ȃ�
		virtual Vector3f evalAndSample(const Vector3f& wo, Vector3f* wi, const Vector3f& n, float* pdf, Sampler& sampler) const {
			NOT_IMPLEMENTED;
			return Vector3f();
		}

		// wi �̃T���v�����O�݂̂��s��
		// �X�y�L�����̕��̂ł� pdf ���f���^�֐��ɂȂ�̂Ŏ������Ȃ�
		virtual void sample(const Vector3f& wo, Vector3f* wi, const Vector3f& n, float* pdf, Sampler& sampler) const {
			NOT_IMPLEMENTED;
		}

		// �X�y�L�����̕��̂ł̂ݎ�������
		// BSDF * cos / pdf �̒l��Ԃ�
		virtual Vector3f evalAndSampleSpecular(const Vector3f& wo, Vector3f* wi, const Vector3f& n, Sampler& sampler) const {
			NOT_IMPLEMENTED;
			return Vector3f();
		}

		// �X�y�L�����̕��̂ł̂ݎ�������
		// wi �̃T���v�����O�݂̂��s��
		virtual void sampleSpecular(const Vector3f& wo, Vector3f* wi, const Vector3f& n, Sampler& sampler) const = 0;
	};

	class Diffuse : public Material {
	public:
		Vector3f albedo;

		Diffuse (const Vector3f& albedo):
			albedo(albedo)
		{
			specular = false;
		}

		Vector3f eval(const Vector3f& wo, const Vector3f& wi, const Vector3f& n) const override {
			Vector3f fn = faceForward(n, wo);
			return albedo * fabs(dot(fn, wi)) * dot(wi, n) / M_PI;
		}

		Vector3f evalAndSample(const Vector3f& wo, Vector3f* wi, const Vector3f& n, float* pdf, Sampler& sampler) const override {
			Vector3f fn = faceForward(n, wo);
			*wi = sampleVectorFromCosinedHemiSphere(fn, sampler);
			*pdf = dot(*wi, fn);
			return albedo * fabs(dot(fn, *wi)) * dot(*wi, fn) / M_PI;
		}

		void sample(const Vector3f& wo, Vector3f* wi, const Vector3f& n, float* pdf, Sampler& sampler) const override {
			Vector3f fn = faceForward(n, wo);
			*wi = sampleVectorFromCosinedHemiSphere(fn, sampler);
			*pdf = dot(*wi, fn);
		}
	};

	class Specular : public Material {
	public:
		Vector3f albedo;

		Specular(const Vector3f& albedo) :
			albedo(albedo)
		{
			specular = true;
		}

		Vector3f evalAndSampleSpecular(const Vector3f& wo, Vector3f* wi, const Vector3f& n, Sampler& sampler) const {
			Vector3f fn = faceForward(n, wo);
			*wi = -(wo - 2 * dot(fn, wo) * fn).normalize();
			return albedo;
		}

		void sampleSpecular(const Vector3f& wo, Vector3f* wi, const Vector3f& n, Sampler& sampler) const override {
			Vector3f fn = faceForward(n, wo);
			*wi = -( wo - 2 * dot(fn, wo) * fn ).normalize();
		}
	};


}