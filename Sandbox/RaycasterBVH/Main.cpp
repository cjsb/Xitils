
#include <Xitils/App.h>
#include <Xitils/Geometry.h>
#include <Xitils/AccelerationStructure.h>
#include <Xitils/TriangleMesh.h>
#include <CinderImGui.h>

using namespace Xitils;
using namespace ci;
using namespace ci::app;
using namespace ci::geom;


struct MyFrameData {
	float initElapsed;
	float frameElapsed;
	Surface surface;
	int triNum;
};

struct MyUIFrameData {
	Vector3f rot;
};

class MyApp : public Xitils::App::XApp<MyFrameData, MyUIFrameData> {
public:
	void onSetup(MyFrameData* frameData, MyUIFrameData* uiFrameData) override;
	void onCleanup(MyFrameData* frameData, MyUIFrameData* uiFrameData) override;
	void onUpdate(MyFrameData& frameData, const MyUIFrameData& uiFrameData) override;
	void onDraw(const MyFrameData& frameData, MyUIFrameData& uiFrameData) override;

private:
	int frameCount = 0;

	gl::TextureRef texture;

	std::shared_ptr<TriangleMesh> teapotMesh;
	std::shared_ptr<Object> teapotObj;
	std::shared_ptr<AccelerationStructure> bvh;
	inline static const glm::ivec2 ImageSize = glm::ivec2(800, 600);
};

void MyApp::onSetup(MyFrameData* frameData, MyUIFrameData* uiFrameData) {
	auto time_start = std::chrono::system_clock::now();

	frameData->surface = Surface(ImageSize.x, ImageSize.y, false);
	frameData->frameElapsed = 0.0f;
	frameData->triNum = 0;

	getWindow()->setTitle("Xitils");
	setWindowSize(ImageSize);
	setFrameRate(60);

	ui::initialize();

	const int subdivision = 100;
	auto teapot = std::make_shared<Teapot>();
	teapot->subdivisions(subdivision);
	auto teapotMeshData = std::make_shared<TriMesh>(*teapot);

	std::vector<Vector3f> positions(teapotMeshData->getNumVertices());
	for (int i = 0; i < positions.size(); ++i) {
		positions[i] = Vector3f(teapotMeshData->getPositions<3>()[i]);
	}
	std::vector<Vector3f> normals(teapotMeshData->getNumVertices());
	for (int i = 0; i < normals.size(); ++i) {
		normals[i] = Vector3f(teapotMeshData->getNormals()[i]);
	}
	std::vector<int> indices(teapotMeshData->getNumTriangles() * 3);
	for (int i = 0; i < indices.size(); i += 3) {
		indices[i + 0] = teapotMeshData->getIndices()[i + 0];
		indices[i + 1] = teapotMeshData->getIndices()[i + 2]; // 時計回りに直す
		indices[i + 2] = teapotMeshData->getIndices()[i + 1];
	}

	teapotMesh = std::make_shared<TriangleMesh>();
	teapotMesh->setGeometry(
		positions.data(), positions.size(),
		normals.data(), normals.size(),
		indices.data(), indices.size()
	);
	teapotObj = std::make_shared<Object>( teapotMesh, Matrix4x4());

	std::vector<Object*> objects;
	objects.push_back(teapotObj.get());

	bvh = std::make_shared<BVH>(objects);

	auto time_end = std::chrono::system_clock::now();
	frameData->initElapsed = std::chrono::duration_cast<std::chrono::milliseconds>(time_end - time_start).count();
}

void MyApp::onCleanup(MyFrameData* frameData, MyUIFrameData* uiFrameData) {

}

void MyApp::onUpdate(MyFrameData& frameData, const MyUIFrameData& uiFrameData) {
	auto time_start = std::chrono::system_clock::now();

	teapotObj->objectToWorld = rotateYXZ(uiFrameData.rot);
	std::vector<Object*> objects;
	objects.push_back(teapotObj.get());
	bvh = std::make_shared<BVH>(objects);

#pragma omp parallel for schedule(dynamic, 1)
	for (int y = 0; y < frameData.surface.getHeight(); ++y) {
#pragma omp parallel for schedule(dynamic, 1)
		for (int x = 0; x < frameData.surface.getWidth(); ++x) {

			Vector3f color(0, 0, 0);

			Vector2f cameraRange;
			cameraRange.x = 4.0f;
			cameraRange.y = cameraRange.x * 3.0f / 4.0f;

			Vector2f cameraOffset(0,0.5f);

			float nx = (float) x / frameData.surface.getWidth();
			float ny = (float) y / frameData.surface.getHeight();

			Xitils::Ray ray;
			ray.o = Vector3f( (nx-0.5f)*cameraRange.x + cameraOffset.x, -(ny-0.5f)*cameraRange.y + cameraOffset.y, 100 );
			ray.d = normalize(Vector3f(0,0,-1));
			
			SurfaceInteraction isect;

			if (bvh->intersect(ray, &isect)) {
				Vector3f dLight = normalize(Vector3f(1, 1, 1));
				color = Vector3f(1.0f, 1.0f, 1.0f) * clamp01(dot(isect.shading.n, dLight));
			}

			ColorA8u colA8u;
			colA8u.r = Xitils::clamp((int)(color.x * 255), 0, 255);
			colA8u.g = Xitils::clamp((int)(color.y * 255), 0, 255);
			colA8u.b = Xitils::clamp((int)(color.z * 255), 0, 255);
			colA8u.a = 255;

			frameData.surface.setPixel(ivec2(x, y), colA8u);
		}
	}

	++frameCount;

	auto time_end = std::chrono::system_clock::now();
	frameData.frameElapsed = std::chrono::duration_cast<std::chrono::milliseconds>(time_end - time_start).count();
	frameData.triNum = teapotMesh->triangleNum();
}

void MyApp::onDraw(const MyFrameData& frameData, MyUIFrameData& uiFrameData) {
	texture = gl::Texture::create(frameData.surface);

	gl::clear(Color::gray(0.5f));
	
	auto windowSize = ci::app::getWindowSize();
	gl::draw(texture, (windowSize - ImageSize) / 2);

	ImGui::Begin("ImGui Window");
	ImGui::Text(("Image Resolution: " + std::to_string(ImageSize.x) + " x " + std::to_string(ImageSize.y)).c_str());
	ImGui::Text(("Elapsed in Initialization: " + std::_Floating_to_string("%.1f", frameData.initElapsed) + " ms").c_str());
	ImGui::Text(("Elapsed per frame: " + std::_Floating_to_string("%.1f", frameData.frameElapsed) + " ms").c_str());
	ImGui::Text(("Triangles: " + std::to_string(frameData.triNum)).c_str());

	float rot[3];
	rot[0] = uiFrameData.rot.x;
	rot[1] = uiFrameData.rot.y;
	rot[2] = uiFrameData.rot.z;
	ImGui::SliderFloat3("Rotation", rot, -180, 180 );
	uiFrameData.rot.x = rot[0];
	uiFrameData.rot.y = rot[1];
	uiFrameData.rot.z = rot[2];

	ImGui::End();
}


XITILS_APP(MyApp)
