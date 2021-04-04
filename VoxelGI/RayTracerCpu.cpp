
#include <iostream>
#include "RayTracerCpu.h"

#include <glm/gtx/component_wise.hpp>
#include <glm/gtx/string_cast.hpp>

#include <limits>

#include "VoxelGI.h"

#include "imgui\imgui.h"
#include <glm/gtc/type_ptr.hpp>

#include <thread>

#include "Gui.h"

namespace VoxelGI
{
	template <typename T>
	RayTracerCpu<T>::RayTracerCpu(VoxelGI& engine)
		: voxelGI(engine)
	{
		tex = std::make_unique<Texture>();
		std::cout << "RayTracerCpu constructed" << std::endl;

		camera.lookAt(glm::vec3(0.0f, 0.0, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f));

		light.pos = glm::vec3(2.0f, 2.0f, 5.0f);
		light.dir = glm::vec3(0.0f, -1.0f, 0.0f);
		light.ambient = glm::vec3(0.0f);
		light.color = glm::vec3(1.0f);
		light.lightType = Light::LT_Point;
		light.power = 15.0f;
	}

	/*template<typename T>
	RayTracerCpu<T>::RayTracerCpu(IRayTracerCpu * rayTracer)
		: voxelGI(rayTracer->getEngine())
	{
		moveSetup(rayTracer);
	}*/

	template <typename T>
	RayTracerCpu<T>::~RayTracerCpu()
	{
		std::cout << "RayTracerCpu destroyed" << std::endl;
	}

	template<typename T>
	VoxelGI & RayTracerCpu<T>::getEngine() const
	{
		return voxelGI;
	}

	template <typename T>
	typename RayTracerCpu<T>::vec4 RayTracerCpu<T>::raytrace(Ray<T>& ray, RenderMode mode)
	{
		for (int i = 0; i < sceneNodes.size(); i++)
		{
			IGeometry<T>* geometry;

			//if (rayPlaneIntersect(ray, sceneNodes[i]->triangles[0].point(0).pos, sceneNodes[i]->triangles[0].normal(), intersectPoint))
			if (sceneNodes[i]->rayIntersect(ray, geometry))
			{
				bool calcDifferentials = mode != RM_RayDistance;
				ray.transfer(geometry, calcDifferentials);

				vec3 p;
				vec3 dpdx, dpdy;

				p = ray.getPosition();

				Plane<T> plane;
				IGeometry<T>::NormalDifferential dn;
				vec3 duvdx, duvdy;

				RayDifferential<T>& rayDiff = (RayDifferential<T>&)ray;

				if (mode >= RM_DPDX)
				{
					// differentials
					/*if (useFiniteDifferentials)
					{
						RayDifferentialFinite<T> rayDif(ray);
						rayDif.initDifferentials(camera, x, y, dx, dy);

						plane = geometry->plane(p);
						rayDif.transfer(plane);

						dpdx = rayDif.getDpdx();
						dpdy = rayDif.getDpdy();

						//geometry->getDn(p, dpdx, dpdy, dn);
					}
					else
					{*/
						//rayDiff.initDifferentials(camera, x, y);

						//rayDiff.transfer(geometry);

						dpdx = rayDiff.getDpdx();
						dpdy = rayDiff.getDpdy();

						if (mode >= RM_DnDX)
							geometry->getDn(rayDiff, dn);
					//}
				}

				/*Material material;
				geometry->getMaterial(p, material);
				if (material.isMirror && ray.reflectCount < maxBounces)
				{
					ray.reflect(geometry);

					return raytrace(ray, RM_SurfaceColor);
				}*/

				vec3 outputVal;
				vec2 duv;
				vec2 unused;

				switch (mode)
				{
				case RM_RayDistance:
					outputVal = vec3(ray.getDistance());
					break;
				case RM_SurfaceColor:
					//outputVal = vec3(geometry->(p));
					outputVal = geometry->surfaceColor(rayDiff);
					break;
				case RM_SurfaceNormal:
					outputVal = geometry->surfaceNormal(p);
					break;
				case RM_ShadingNormal:
					outputVal = geometry->shadingNormal(p);
					break;
				case RM_Material:
				{
					Material material;
					glm::vec4 texVal;
					vec4 outputVal4;
					geometry->getMaterial(p, material);

					if (material.isMirror && ray.reflectCount < maxBounces)
					{
						ray.reflect(geometry);

						texVal = raytrace(ray, RM_SurfaceColor);
					}
					else
					{
						geometry->getTextureValue(rayDiff, material.texId, texVal);
					}

					blinnPhong(p, geometry->surfaceNormal(p), light, material, texVal, outputVal4);
					outputVal = outputVal4;
					break;
				}
				case RM_UV:
					outputVal = vec3(geometry->getUV(p), 0.0);
					break;
				case RM_DPDX:
					outputVal = dpdx;
					break;
				case RM_DPDY:
					outputVal = dpdy;
					break;
				case RM_DPDX_DPDY:
					outputVal = dpdx + dpdy;
					break;
				case RM_DnDX:
					outputVal = dn.dndx;
					break;
				case RM_DnDY:
					outputVal = dn.dndy;
					break;
				case RM_DNDX:
					outputVal = dn.dNdx;
					break;
				case RM_DNDY:
					outputVal = dn.dNdy;
					break;
				case RM_DUVDX:
					geometry->getDuv(rayDiff, duv, unused);
					outputVal = vec3(duv, 0.0);
					break;
				case RM_DUVDY:
					geometry->getDuv(rayDiff, unused, duv);
					outputVal = vec3(duv, 0.0);
					break;
				}
				
				//outputVal.w = (Float)1.0;

				return vec4(outputVal, 1.0);
			}
		}

		// ray miss
		const vec4 missColor(0.0f, 0.0f, 0.5f, 1.0f);

		if (ray.reflectCount > 0)
			return missColor;
		else
			return vec4(0.0);
	}

	template <typename T>
	bool RayTracerCpu<T>::rayPlaneIntersect(const Ray<T> & ray, const Plane<T>& plane)
	{
		ray.t = -glm::dot(plane.normal, ray.origin) / glm::dot(plane.normal, ray.direction);

		return ray.t >= (Float)0;
	}

	template<typename T>
	glm::ivec2 RayTracerCpu<T>::relToAbsCoords(const glm::vec2 & coords)
	{
		return glm::clamp(glm::ivec2((
			glm::vec2(coords.x, -coords.y) + glm::vec2(1.0f)) * 0.5f * glm::vec2(getSampleResolution() - glm::ivec2(1.0))), 
			glm::ivec2(0), getSampleResolution() - glm::ivec2(1.0));
	}

	template<typename T>
	void RayTracerCpu<T>::drawDebugGui()
	{
		if (ImGui::TreeNode("RayTracer"))
		{
			glm::ivec2 res = getPixelResolution();

			if (Gui::Property("Render Resolution", res))
				setPixelResolution(res);

			if (Gui::Property("Supersample", supersample))
				setSSAA(supersample);

			/*ImGui::SameLine();

			if (ImGui::Button("Screen"))
			{
				int screenRes = glm::min(voxelGI.screenWidth(), voxelGI.screenHeight());

				setResolution(glm::ivec2(screenRes));
			}*/

			//ImGui::Checkbox("Min/Max scale", &minMaxScale);

			/*Gui::Property<
				RayTracerCpu,
				bool, 
				&RayTracerCpu::finiteDifferentialsEnabled,
				&RayTracerCpu::setFiniteDifferentialsEnabled>("Test", this);*/

			ImGui::Checkbox("Continous Render", &continuousRender);

			Gui::SameLine();

			Gui::Property("Track Progress", progressTackingEnabled);

			//ImGui::SameLine();

			//ImGui::Checkbox("Finite Dif Differentials", &useFiniteDifferentials);

			Gui::Property("Max bounces", maxBounces);

			Gui::Property("Trace center ray only", traceCenterRayOnly);

			bool doPostProcess = false;

			doPostProcess |= ImGui::ColorEdit4("Background Color", glm::value_ptr(backgroundColor));
			doPostProcess |= ImGui::InputFloat("Value Scale", &valueScale);
			doPostProcess |= ImGui::InputFloat("Value Shift", &valueShift);

			doPostProcess |= Gui::Property("X", valueChannelDisplay[0]); Gui::SameLine();
			doPostProcess |= Gui::Property("Y", valueChannelDisplay[1]); Gui::SameLine();
			doPostProcess |= Gui::Property("Z", valueChannelDisplay[2]); Gui::SameLine();
			doPostProcess |= ImGui::Checkbox("Absolute Value", &absoluteValue);


			int currentMode = getRenderMode();

			ImGui::Combo("Mode", &currentMode, renderModesStr, IM_ARRAYSIZE(renderModesStr));

			setRenderMode((RayTracerCpu<T>::RenderMode)currentMode);

			if (doPostProcess)
			{
				writeFinalBuffer();
			}

			ImGui::TreePop();
		}

		if (ImGui::TreeNode("Camera"))
		{
			glm::vec3 camPos = camera.translation();
			
			if (ImGui::InputFloat3("Position", glm::value_ptr(camPos)))
				camera.setTranslation(camPos);

			ImGui::TreePop();
		}

		if (ImGui::TreeNode("Scene Settings"))
		{
			sceneNodes[0]->drawDebugGui();

			ImGui::TreePop();
		}
	}

	template<typename T>
	Camera & RayTracerCpu<T>::getCamera()
	{
		return camera;
	}

	template<typename T>
	void RayTracerCpu<T>::setSSAA(int ssaa)
	{
		supersample = ssaa;
		updateBufferSize();
	}

	template<typename T>
	void RayTracerCpu<T>::setMaxBounces(int bounces)
	{
		maxBounces = bounces;
	}

	template<typename T>
	void RayTracerCpu<T>::setProgressTrackingEnable(bool enable)
	{
		progressTackingEnabled = enable;
	}

	template <typename T>
	typename RayTracerCpu<T>::vec4 RayTracerCpu<T>::scaleVector01(const vec4& val, const vec4& valMin, const vec4& valMax)
	{
		const Float eps = 1e-6;
		vec4 res;

		for (int i = 0; i < 3; i++)
		{
			if (std::abs(valMin[i]) > eps || std::abs(valMax[i]) > eps)
			{
				
				//Float intLength = valMax[i] - valMin[i];
				//res[i] = (val[i] - valMin[i]) / (valMax[i] - valMin[i]);

				//res[i] = (val[i] - minV) / (0.00000000000001);

				Float intLength = valMax[i] - valMin[i];
				Float lowerBounds = valMin[i] / intLength;

				res[i] = (val[i] - valMin[i]) / (valMax[i] - valMin[i]);

				//res[i] = val[i] / intLength - lowerBounds;
			}
			else
				res[i] = 0.0;
		}

		return vec4(vec3(glm::clamp(res, vec4(0.0), vec4(1.0))), 1.0);
	}

	template <typename T>
	void RayTracerCpu<T>::initRender()
	{
		if (isFirstPass())
		{
			avgRenderValue = vec4(0.0f);
			maxRenderValue = vec4(-std::numeric_limits<Float>::infinity());
			minRenderValue = vec4(std::numeric_limits<Float>::infinity());

			renderTimeStart = voxelGI.getTimer().time();

			numIntersections = 0;

			logStream.str(std::string());
		}
	}

	template <typename T>
	void RayTracerCpu<T>::postRender()
	{
		if (isFirstPass())
		{
			int xCenter = sampleWidth() / 2;
			int yCenter = sampleHeight() / 2;

			for (int y = 0; y < sampleHeight(); y++)
			{
				for (int x = 0; x < sampleWidth(); x++)
				{
					vec4 outputVal = outputBuffer(x, y);

					if (outputVal.w > (Float)0.0)
					{
						numIntersections++;

						minRenderValue = glm::min(outputVal, minRenderValue);
						maxRenderValue = glm::max(outputVal, maxRenderValue);
						avgRenderValue += outputVal;

						if (x == xCenter && y == yCenter)
							centerRenderValue = outputVal;
					}
				}
			}

			avgRenderValue /= numIntersections;
		}

		if (isLastPass())
		{
			renderTime = voxelGI.getTimer().time() - renderTimeStart;

			logStream.precision(20);

			logStream
				<< "numIntersections: " << numIntersections << std::endl
				<< "render time: " << renderTime << "s" << std::endl << std::endl
				<< "avgValue: " << vec3(avgRenderValue) << std::endl
				<< "minValue: " << vec3(minRenderValue) << std::endl
				<< "maxValue: " << vec3(maxRenderValue) << std::endl
				<< "center ray value: " << vec3(centerRenderValue) << std::endl;
		}
	

	}

	template<typename T>
	void RayTracerCpu<T>::renderPass()
	{
		if (traceCenterRayOnly)
		{
			RayDifferential<T> ray;
			ray.initViewRay(camera, 0, 0);

			outputBuffer(sampleWidth() / 2, sampleHeight() / 2) = raytrace(ray, renderMode);
		}
		else
		{
			threadLinesProgress.assign(std::max(numThreads, 1), 0);

			if (numThreads == 0)
				renderPart<true>(0, 0, sampleHeight());
			else
			{
				int numRowsPerThread = sampleHeight() / numThreads;
				int numRowsFirstThread = numRowsPerThread + (sampleHeight() % numThreads);

				int yStart = 0;
				int yEnd = numRowsFirstThread;

				std::vector<std::thread> threads;

				for (int i = 1; i < numThreads; i++)
				{
					threads.push_back(std::thread(&RayTracerCpu<T>::renderPart<false>, this, i, yStart, yEnd - yStart));
					yStart = yEnd;
					yEnd += numRowsPerThread;
				}

				renderPart<true>(0, yStart, yEnd - yStart);

				for (int i = 0; i < numThreads - 1; i++)
				{
					threads[i].join();
				}
			}
		}
	}

	template<typename T>
	template <bool reportProgress>
	void RayTracerCpu<T>::renderPart(int threadId, int yStart, int numRows)
	{
		for (int y = yStart; y < yStart + numRows; y++)
		{
			for (int x = 0; x < sampleWidth(); x++)
			{
				Float relX = dx * (x - sampleWidth() / 2);
				Float relY = -dy * (y - sampleHeight() / 2);

				RayDifferential<T> ray;
				ray.initViewRay(camera, relX, relY);

				outputBuffer(x, y) = raytrace(ray, renderMode);
			}

			if (progressTackingEnabled)
				trackProgress<reportProgress>(threadId);
		}
	}

	template<typename T>
	template <bool reportProgress>
	void RayTracerCpu<T>::trackProgress(int threadId)
	{
		threadLinesProgress[threadId]++;

		if (reportProgress)
		{
			int linesSum = 0;

			for (int i = 0; i < threadLinesProgress.size(); i++)
				linesSum += threadLinesProgress[i];

			std::cout << 100.0f * linesSum / sampleHeight() << "% ("
				<< linesSum << "/" << sampleHeight() << " lines)" << std::endl;
		}
	}

	template<typename T>
	bool RayTracerCpu<T>::isFirstPass()
	{
		return currentPass == 0;
	}

	template<typename T>
	bool RayTracerCpu<T>::isSecondPass()
	{
		return currentPass == 1;
	}

	template<typename T>
	bool RayTracerCpu<T>::isLastPass()
	{
		return currentPass == numPasses - 1;
	}

	template<typename T>
	void RayTracerCpu<T>::updateBufferSize()
	{
		outputBuffer.resize(sampleWidth(), sampleHeight());
		finalBuffer.resize(pixelWidth(), pixelHeight());
	}

	template<typename T>
	void RayTracerCpu<T>::blinnPhong(const glm::vec3& pos, const glm::vec3& normal, const Light& light, const Material& mat, const glm::vec3& texVal, vec4& colorOut) const
	{
		const float screenGamma = 2.2;

		glm::vec3 lightDir = light.pos - pos;
		float distance = length(lightDir);
		distance = distance * distance;
		lightDir = normalize(lightDir);

		float lambertian = std::max(glm::dot(lightDir, normal), 0.0f);
		float specular = 0.0;

		if (lambertian > 0.0) {

			glm::vec3 viewDir = normalize(-camera.forward());

			// this is blinn phong
			glm::vec3 halfDir = normalize(lightDir + viewDir);
			float specAngle = std::max(dot(halfDir, normal), 0.0f);
			specular = pow(specAngle, mat.shininess);
		}

		glm::vec3 colorLinear = light.ambient +
			mat.diffuse * texVal * lambertian * light.color * light.power / distance +
			mat.specular * specular * light.color * light.power / distance;
		// apply gamma correction (assume ambientColor, diffuseColor and specColor
		// have been linearized, i.e. have no gamma correction in them)
		glm::vec3 colorGammaCorrected = glm::pow(colorLinear, glm::vec3(1.0 / screenGamma));
		// use the gamma corrected color in the fragment
		colorOut = vec4(colorGammaCorrected, 1.0);
	}

	template<typename T>
	int RayTracerCpu<T>::pixelWidth() const
	{
		return pixelResolution.x;
	}

	template<typename T>
	int RayTracerCpu<T>::pixelHeight() const 
	{
		return pixelResolution.y;
	}

	template<typename T>
	int RayTracerCpu<T>::sampleWidth() const
	{
		return pixelWidth() * samplesPerPixel();
	}

	template<typename T>
	int RayTracerCpu<T>::sampleHeight() const
	{
		return pixelHeight() * samplesPerPixel();
	}

	template<typename T>
	glm::ivec2 RayTracerCpu<T>::getPixelResolution() const
	{
		return glm::ivec2(pixelWidth(), pixelHeight());
	}

	template<typename T>
	glm::ivec2 RayTracerCpu<T>::getSampleResolution() const
	{
		return glm::ivec2(sampleWidth(), sampleHeight());
	}

	template<typename T>
	int RayTracerCpu<T>::samplesPerPixel() const
	{
		return std::max(supersample, 1);
	}

	template <typename T>
	void RayTracerCpu<T>::setPixelResolution(glm::ivec2 res)
	{
		pixelResolution = res;

		updateBufferSize();
	}

	/*template <typename T>
	glm::ivec2 RayTracerCpu<T>::getResolution() const
	{
		return glm::ivec2(outputBuffer.sizeX(), outputBuffer.sizeY());
	}*/

	template <typename T>
	void RayTracerCpu<T>::setRenderMode(RenderMode mode)
	{
		renderMode = mode;
	}

	template <typename T>
	void RayTracerCpu<T>::addNode(const std::shared_ptr<ISceneNodeCpu<T>>& sceneNode)
	{
		sceneNodes.push_back(sceneNode);
	}

	template <typename T>
	void RayTracerCpu<T>::renderFrame()
	{
		for (int i = 0; i < sceneNodes.size(); i++)
		{
			sceneNodes[i]->transformGeometry(glm::mat4());
		}

		vec2 imagePlaneExtent = vec2(1.0f, 1.0f);

		dx = 2.0f * imagePlaneExtent.x / sampleWidth();
		dy = 2.0f * imagePlaneExtent.y / sampleHeight();

		numPasses = 1;

		if (minMaxScale)
		{
			numPasses = 2;
		}

		for (currentPass = 0; currentPass < numPasses; currentPass++)
		{
			initRender();

			renderPass();

			postRender();
		}

		writeFinalBuffer();
	}

	template <typename T>
	void RayTracerCpu<T>::render()
	{
		if (!continuousRender && !renderRequested)
			return;

		renderFrame();
		
		renderRequested = false;
	}

	template<typename T>
	void RayTracerCpu<T>::writeFinalBuffer()
	{
		for (int y = 0; y < finalBuffer.size().y; y++)
		{
			for (int x = 0; x < finalBuffer.size().x; x++)
			{
				vec4 finalVal;

				for (int ySample = 0; ySample < samplesPerPixel(); ySample++)
				{
					for (int xSample = 0; xSample < samplesPerPixel(); xSample++)
					{
						vec4 outputVal = outputBuffer(
							x * samplesPerPixel() + xSample,
							y * samplesPerPixel() + ySample);

						if (outputVal.w != (Float)0.0)
							finalVal += outputVal;
						else
							finalVal += backgroundColor;
					}
				}

				finalVal /= samplesPerPixel() * samplesPerPixel();

				if (finalVal.w == (Float)0.0)
				{
					finalVal = backgroundColor;
				}
				else
				{
					if (absoluteValue)
						finalVal = glm::abs(finalVal);

					finalVal = vec4
					(
						valueChannelDisplay[0] ? finalVal.x : 0.0,
						valueChannelDisplay[1] ? finalVal.y : 0.0,
						valueChannelDisplay[2] ? finalVal.z : 0.0,
						finalVal.w
					);

					finalVal = (finalVal + vec4(valueShift)) * (Float)valueScale;

					finalVal = glm::clamp(finalVal, vec4(0.0f), vec4(1.0f));
				}

				finalBuffer.setPixel(glm::ivec2(x, y), finalVal);
			}
		}

		updateTexture();
	}

	template<typename T>
	void RayTracerCpu<T>::updateTexture()
	{
		tex->createFromBuffer(finalBuffer);
	}

	template<typename T>
	typename RayTracerCpu<T>::RenderMode RayTracerCpu<T>::getRenderMode()
	{
		return renderMode;
	}

	template <typename T>
	std::shared_ptr<ISceneNodeCpu<T>> RayTracerCpu<T>::nodes(int i)
	{
		return sceneNodes[i];
	}

	template<typename T>
	bool RayTracerCpu<T>::sceneEmpty()
	{
		return sceneNodes.empty();
	}

	template<typename T>
	void RayTracerCpu<T>::clearScene()
	{
		sceneNodes.clear();
	}

	template <typename T>
	void RayTracerCpu<T>::requestRender()
	{
		renderRequested = true;
	}

	template <typename T>
	const Texture& RayTracerCpu<T>::getTexture()
	{
		return *tex;
	}

	template <typename T>
	const std::stringstream & RayTracerCpu<T>::getLogStream()
	{
		return logStream;
	}

	template<typename T>
	bool RayTracerCpu<T>::usesDoublePrecision() const
	{
		return false;
	}

	template<typename T>
	void RayTracerCpu<T>::setMinMaxScaleEnable(bool mnScale)
	{
		minMaxScale = mnScale;
	}

	template<typename T>
	bool RayTracerCpu<T>::minMaxScaleEnabled()
	{
		return minMaxScale;
	}

	template<typename T>
	void RayTracerCpu<T>::setFiniteDifferentialsEnabled(bool finDif)
	{
		useFiniteDifferentials = finDif;
	}

	template<typename T>
	bool RayTracerCpu<T>::finiteDifferentialsEnabled()
	{
		return useFiniteDifferentials;
	}

	template<typename T>
	void RayTracerCpu<T>::setBackgroundColor(const glm::vec4 & color)
	{
		backgroundColor = color;
	}

	template<typename T>
	const glm::vec4 & RayTracerCpu<T>::getBackgroundColor()
	{
		return backgroundColor;
	}

	template<typename T>
	bool RayTracerCpu<T>::continuousRenderEnabled()
	{
		return continuousRender;
	}

	template<typename T>
	const TextureBuffer<2, 4>& RayTracerCpu<T>::getFinalBuffer()
	{
		return finalBuffer;
	}

	template<typename T>
	const Buffer2D<typename RayTracerCpu<T>::vec4>& RayTracerCpu<T>::getOutputBuffer() const
	{
		return outputBuffer;
	}

	template<typename T>
	typename RayTracerCpu<T>::Float RayTracerCpu<T>::getDx() const
	{
		return dx;
	}

	template<typename T>
	typename RayTracerCpu<T>::Float RayTracerCpu<T>::getDy() const
	{
		return dy;
	}

	template<>
	bool RayTracerCpu<double>::usesDoublePrecision() const
	{
		return true;
	}

	template class RayTracerCpu<float>;
	template class RayTracerCpu<double>;

	template <typename T>
	std::ostream & operator<<(std::ostream & out, glm::tvec3<T> const & t)
	{
		out << "(" << t.x << ", " << t.y << ", " << t.z << ")";
		return out;
	}
}