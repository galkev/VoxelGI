#pragma once

#include <vector>
#include "SceneNodeCpu.h"
#include "Texture.h"
#include "TextureBuffer.h"
#include "Ray.h"
#include "MultiBuffer.h"
#include "Camera.h"

#include <string>
#include <sstream>
#include <memory>

namespace VoxelGI
{
	class VoxelGI;

	template <typename T>
	std::ostream & operator<<(std::ostream & out, glm::tvec3<T> const & t);

	template <typename T>
	class RayTracerCpu
	{
	public:
		enum RenderMode
		{
			RM_RayDistance,
			RM_SurfaceColor,
			RM_Barycentrics,
			RM_SurfaceNormal,
			RM_ShadingNormal,
			RM_Material,
			RM_UV,
			RM_DPDX,
			RM_DPDY,
			RM_DPDX_DPDY,
			RM_DnDX,
			RM_DnDY,
			RM_DNDX,
			RM_DNDY,
			RM_DUVDX,
			RM_DUVDY,
			RM_DnDX_Subtract,
			RM_DnDY_Subtract,
			RM_DNDX_Subtract,
			RM_DNDY_Subtract,

			RenderModeCount
		};

	private:
		const char* renderModesStr[RenderModeCount] =
		{
			"RayDistance",
			"Surface Color",
			"Barycentrics",
			"Surface Normal",
			"Shading Normal",
			"Material",
			"UV",
			"DPDX",
			"DPDY",
			"DPDX+DPDY",
			"DnDX",
			"DnDY",
			"DNDX",
			"DNDY",
			"DUVDX",
			"DUVDY",
			"DnDX Subtract",
			"DnDY Subtract",
			"DNDX_Subtract",
			"DNDY_Subtract"
		};

		using Float = T;
		using vec2 = glm::tvec2<T, glm::highp>;
		using vec3 = glm::tvec3<T, glm::highp>;
		using vec4 = glm::tvec4<T, glm::highp>;

		template <typename>
		friend class RayTracerCpu;

		VoxelGI& voxelGI;

		std::vector<std::shared_ptr<ISceneNodeCpu<T>>> sceneNodes;

		glm::ivec2 pixelResolution;
		int supersample = 0;

		Buffer2D<vec4> outputBuffer;

		TextureBuffer<2, 4> finalBuffer;
		std::unique_ptr<Texture> tex;

		int currentPass;
		int numPasses;
		int numIntersections;

		vec4 minRenderValue;
		vec4 maxRenderValue;
		vec4 avgRenderValue;
		vec4 centerRenderValue;

		bool continuousRender = false;
		bool renderRequested = false;

		bool minMaxScale = false;
		bool useFiniteDifferentials = false;
		int maxBounces = 0;

		bool traceCenterRayOnly = false;

		float valueScale = 1.0f;
		float valueShift = 0.0f;
		bool valueChannelDisplay[3] = { true, true, true };
		bool absoluteValue = true;

		float renderTime;
		float renderTimeStart;

		Float dx;
		Float dy;

		Light light;

		RenderMode renderMode = RM_SurfaceColor;

		std::stringstream logStream;

		glm::vec4 backgroundColor;

		Camera camera;

		int numThreads = 2;

		bool progressTackingEnabled;
		std::vector<int> threadLinesProgress;

		template <bool reportProgress>
		void trackProgress(int threadId);

		vec4 raytrace(Ray<T>& ray, RenderMode mode);

		vec4 scaleVector01(const vec4& val, const vec4& valMin, const vec4& valMax);
		
		void initRender();
		void postRender();
		void renderPass();

		template <bool reportProgress>
		void renderPart(int threadId, int yStart, int numRows);

		bool isFirstPass();
		bool isSecondPass();
		bool isLastPass();

		void updateBufferSize();

		void blinnPhong(const glm::vec3& pos, const glm::vec3& normal, const Light& light, const Material& mat, const glm::vec3& texVal, vec4& colorOut) const;

	public:
		RayTracerCpu(VoxelGI& engine);
		//RayTracerCpu(IRayTracerCpu* rayTracer);
		~RayTracerCpu();

		VoxelGI& getEngine() const;

		//void moveSetup(IRayTracerCpu* rayTracer);

		//template <typename S>
		//void moveSetup(RayTracerCpu<S>& rayTracer);

		int pixelWidth() const;
		int pixelHeight() const;
		int sampleWidth() const;
		int sampleHeight() const;

		glm::ivec2 getPixelResolution() const;
		glm::ivec2 getSampleResolution() const;

		int samplesPerPixel() const;
		//void setResolution(int width, int height);
		void setPixelResolution(glm::ivec2 res);
	
		void setRenderMode(RenderMode mode);
		void addNode(const std::shared_ptr<ISceneNodeCpu<T>>& sceneNode);
		void render();

		void renderFrame();

		void writeFinalBuffer();
		void updateTexture();

		RenderMode getRenderMode();

		std::shared_ptr<ISceneNodeCpu<T>> nodes(int i);

		bool sceneEmpty();

		void clearScene();

		void requestRender();

		const Texture& getTexture();


		const std::stringstream& getLogStream();

		bool usesDoublePrecision() const;

		void setMinMaxScaleEnable(bool mnScale);
		bool minMaxScaleEnabled();

		void setFiniteDifferentialsEnabled(bool finDif);
		bool finiteDifferentialsEnabled();

		void setBackgroundColor(const glm::vec4& color);
		const glm::vec4& getBackgroundColor();

		bool continuousRenderEnabled();

		const TextureBuffer<2, 4>& getFinalBuffer();
		const Buffer2D<vec4>& getOutputBuffer() const;

		Float getDx() const;
		Float getDy() const;

		static bool rayPlaneIntersect(const Ray<T>& ray, const Plane<T>& plane);

		glm::ivec2 relToAbsCoords(const glm::vec2& coords);

		void drawDebugGui();

		Camera& getCamera();

		void setSSAA(int ssaa);

		void setMaxBounces(int bounces);

		void setProgressTrackingEnable(bool enable);
	};

}