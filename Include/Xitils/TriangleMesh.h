#pragma once

#include "Shape.h"
#include "AccelerationStructure.h"

namespace Xitils {

	class TriangleMesh : public Shape {
	public:

		~TriangleMesh(){
			for (auto* tri : triangles) {
				delete tri;
			}
		}

		void setGeometry(const Vector3f* positionData, int positionNum, int* indexData, int indexNum) {
			setPositions(positionData, positionNum);
			setIndices(indexData, indexNum);
			buildBVH();
			calcSurfaceArea();
		}

		void setGeometry(const Vector3f* positionData, int positionNum, const Vector3f* normalData, int normalNum, int* indexData, int indexNum) {
			setPositions(positionData, positionNum);
			setNormals(normalData, normalNum);
			setIndices(indexData, indexNum);
			buildBVH();
			calcSurfaceArea();
		}

		int triangleNum() const { return triangles.size(); }

		Bounds3f bound() const override {
			Bounds3f res;
			for (const auto& p : positions) {
				res = merge(res, p);
			}
			return res;
		}

		float surfaceArea() const override {
			return area;
		}

		float surfaceAreaScaling(const Transform& t)  const override {
			// TODO: ������
			float scaledArea = 0.0f;
			for (const auto& tri : triangles) {
				scaledArea += tri->surfaceArea() * tri->surfaceAreaScaling(t);
			}
			return scaledArea / area;
		}

		bool intersect(const Ray& ray, float* tHit, SurfaceInteraction* isect) const override {
			Ray rayTmp = Ray(ray);
			if (accel->intersect(rayTmp, isect)) {
				isect->shape = this;
				*tHit = rayTmp.tMax;
				return true;
			}
			return false;
		}

		bool intersectAny(const Ray& ray) const override {
			return accel->intersectAny(ray);
		}

		SampledSurface sampleSurface(const std::shared_ptr<Sampler>& sampler, float* pdf) const override {
			// TODO: ���ׂĂ� Triangle ���瓙�m���ŃT���v�����O���Ă��邪�A
			//       �{���͖ʐςɔ�Ⴕ���m���őI�Ԃׂ�
			auto& tri = sampler->select(triangles);
			auto sampled = tri->sampleSurface(sampler, pdf);
			*pdf /= triangleNum();

			SampledSurface res;
			res.shape = this;
			res.primitive = sampled.primitive;
			res.p = sampled.p;
			res.n = sampled.n;
			res.shadingN = sampled.shadingN;
			return res;
		}

		float surfacePDF(const Vector3f& p, const Primitive* primitive) const override {
			// TODO: ��L�𒼂����炱���������
			return primitive->surfacePDF(p) / triangleNum();
		}

	private:
		std::vector<Vector3f> positions;
		std::vector<Vector3f> normals;
		std::vector<int> indices;
		std::vector<Primitive*> triangles;

		std::unique_ptr<AccelerationStructure> accel;

		float area;

		void calcSurfaceArea() {
			area = 0.0f;
			for (const auto& tri : triangles) {
				area += tri->surfaceArea();
			}
		}

		void buildBVH() {
			int faceNum = indices.size() / 3;

			triangles.clear();
			triangles.resize(faceNum);
			
			for (int i = 0; i < faceNum; ++i) {
				triangles[i] = new TriangleIndexed(positions.data(), !normals.empty() ? normals.data() : nullptr, indices.data(), i);
			}

			accel = std::make_unique<AccelerationStructure>(triangles);
		}

		void setPositions(const Vector3f* data, int num){
			positions.clear();
			positions.resize(num);
			memcpy(positions.data(), data, sizeof(Vector3f) * num);
		}

		void setNormals(const Vector3f* data, int num) {
			normals.clear();
			normals.resize(num);
			memcpy(normals.data(), data, sizeof(Vector3f) * num);
		}

		void setIndices(int* data, int num) {
			indices.clear();
			indices.resize(num);
			memcpy(indices.data(), data, sizeof(int) * num);
		}

	};

	class Cube : public TriangleMesh {
	public:

		Cube() {
			std::array<Vector3f,8> positions;
			for (int i = 0; i < 8; ++i) {
				positions[i] = Vector3f( i & 1 ? -0.5f : 0.5f,
										 i & 2 ? -0.5f : 0.5f,
									     i & 4 ? -0.5f : 0.5f);
			}
			std::array<int, 3 * 6 * 2> indices{
				0,1,2, 1,3,2,
				2,3,6, 3,7,6,
				0,4,1, 4,5,1,
				0,2,6, 0,6,4,
				1,5,3, 5,7,3,
				4,6,5, 6,7,5
			};

			setGeometry(positions.data(), positions.size(), indices.data(), indices.size());

		}

	};

	class Plane : public TriangleMesh {
	public:

		Plane() {
			std::array<Vector3f, 4> positions;
			for (int i = 0; i < 4; ++i) {
				positions[i] = Vector3f(i & 1 ? -0.5f : 0.5f,
					i & 2 ? -0.5f : 0.5f,
					0);
			}
			std::array<int, 3 * 2> indices{
				0,2,1, 1,2,3
			};

			setGeometry(positions.data(), positions.size(), indices.data(), indices.size());

		}

	};

}