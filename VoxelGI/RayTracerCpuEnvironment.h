#pragma once

#include "Environment.h"
#include "Shader.h"
#include "SceneNode.h"
#include "RayTracerCpu.h"
#include "Sphere.h"
#include "VoxelGeometry.h"
#include "SignedDistanceSphere.h"
#include "SignedDistanceBox.h"
#include "SignedDistanceTorus.h"
#include "SignedDistanceTriangle.h"
#include "SignedDistanceEllipsoid.h"
#include "SignedDistanceBend.h"
#include "SignedDistanceMesh.h"
#include "MeshLoader.h"

#include "CameraControl.h"

#include <memory>

#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "imgui\imgui.h"

#include "lodepng\lodepng.h"

#include <fstream>
#include <string>

#include "Gui.h"

#include "GLState.h"

// windows only
#define NOMINMAX
#include "Windows.h"

namespace VoxelGI
{
	template <typename T>
	class RayTracerCpuEnvironment : public IEnvironment
	{
	private:
		enum PlotPointMode
		{
			PPM_X,
			PPM_Y,
			PPM_Z,
			PPM_Magnitude
		};

		using Float = T;
		using vec2 = glm::tvec2<T, glm::highp>;
		using vec3 = glm::tvec3<T, glm::highp>;
		using vec4 = glm::tvec4<T, glm::highp>;

		template <template<typename> typename S>
		struct SceneSetup
		{
			RayTracerCpuEnvironment<Float>* env;

			std::shared_ptr<SceneNodeCpu<Float, S<Float>>> node;

			glm::vec4 backgroundColor;
			glm::vec3 camPos;
			int ssaa;

			void init(RayTracerCpuEnvironment<Float>* environment)
			{
				env = environment;
				node = std::make_shared<SceneNodeCpu<Float, VoxelGeometry<Float>>>(*env);

				setDefault();
			}

			void release()
			{
				node.reset();
			}

			void setDefault()
			{
				backgroundColor = glm::vec4(0.0f, 0.0f, 0.5f, 1.0f);
				camPos = glm::vec3(0.0f, 0.0f, 2.0f);
				ssaa = 0;
			}

			void load()
			{
				auto& rayTracer = env->getRayTracer();

				rayTracer.setBackgroundColor(backgroundColor);
				rayTracer.getCamera().setTranslation(camPos);

				rayTracer.clearScene();
				rayTracer.addNode(node);

				rayTracer.setSSAA(ssaa);
			}
		};

		enum SceneType
		{
			ST_Triangle,
			ST_Sphere,
			ST_SDFSphere,
			ST_SDFEllipsoid,
			ST_SDFBox,
			ST_SDFTorus,
			ST_SDFTriangle,
			ST_SDFBend,
			ST_SDFMesh,
			ST_Voxel,
			ST_Voxel2,
			ST_Voxel3,
			ST_VoxelTestScene0,
			ST_VoxelTestScene1,
			ST_VoxelTestScene2,
			ST_VoxelTestScene3,
			ST_VoxelTestScene4,
			ST_VoxelTestScene5,
			ST_VoxelTestScene6,
			ST_VoxelTestScene7,
			ST_VoxelTestScene8,
			ST_VoxelTestScene9,

			SceneTypeCount
		};

		const char* sceneTypeStr[SceneTypeCount] =
		{
			"Triangle",
			"Sphere",
			"SDFSphere",
			"SDFEllipsoid",
			"SDFBox",
			"SDFTorus",
			"SDFTriangle",
			"SDFBend",
			"SDFMesh",
			"Voxel",
			"Voxel2",
			"Voxel3",
			"VoxelTestScene0",
			"VoxelTestScene1",
			"VoxelTestScene2",
			"VoxelTestScene3",
			"VoxelTestScene4",
			"VoxelTestScene5",
			"VoxelTestScene6",
			"VoxelTestScene7",
			"VoxelTestScene8",
			"VoxelTestScene9"
		};

		const std::string imageDir = "images\\";

		SceneType currentSceneType = ST_VoxelTestScene0;

		VoxelGI& voxelGI;

		Shader shader;

		Mesh quadMesh;

		RayTracerCpu<Float> rayTracer;

		CameraControl cameraControl;

		std::shared_ptr<SceneNodeCpu<Float, Triangle<Float>>> triNode;
		std::shared_ptr<SceneNodeCpu<Float, Sphere<Float>>> sphereNode;
		std::shared_ptr<SceneNodeCpu<Float, SignedDistanceSphere<Float>>> sdfSphereNode;
		std::shared_ptr<SceneNodeCpu<Float, SignedDistanceEllipsoid<Float>>> sdfEllipsoidNode;
		std::shared_ptr<SceneNodeCpu<Float, SignedDistanceBox<Float>>> sdfBoxNode;
		std::shared_ptr<SceneNodeCpu<Float, SignedDistanceTorus<Float>>> sdfTorusNode;
		std::shared_ptr<SceneNodeCpu<Float, SignedDistanceTriangle<Float>>> sdfTriangleNode;
		std::shared_ptr<SceneNodeCpu<Float, SignedDistanceBend<Float>>> sdfBendNode;
		std::shared_ptr<SceneNodeCpu<Float, SignedDistanceMesh<Float>>> sdfMeshNode;
		std::shared_ptr<SceneNodeCpu<Float, VoxelGeometry<Float>>> voxelNode;
		std::shared_ptr<SceneNodeCpu<Float, VoxelGeometry<Float>>> voxelNode2;
		std::shared_ptr<SceneNodeCpu<Float, VoxelGeometry<Float>>> voxelNode3;

		static constexpr int numVoxelTestScenes = 10;

		SceneSetup<VoxelGeometry> voxelTestSceneSetup[numVoxelTestScenes];

		std::shared_ptr<Texture2DCpu<T>> woodTex1;
		std::shared_ptr<Texture2DCpu<T>> text1Tex = std::make_shared<TextureMip2DCpu<T>>();

		std::vector<std::shared_ptr<Texture2DCpu<T>>> voxelSceneTextures;

		MeshData meshObj;

		bool useDouble = true;
		bool freezeControls = false;

		glm::ivec2 viewResolution = glm::ivec2(512, 512);

		glm::vec2 plotPoint;
		PlotPointMode plotPointMode;

		void screenshot(const std::string& filename, bool screenshotAll, bool openImage = false)
		{
			std::string filepath = imageDir + filename;
			
			unsigned int error;

			if (screenshotAll)
			{
				int width, height;
				std::vector<unsigned char> buffer;

				width = voxelGI.screenWidth();
				height = voxelGI.screenHeight();

				// Make the BYTE array, factor of 3 because it's RBG.
				buffer.resize(3 * width * height);

				glPixelStorei(GL_PACK_ALIGNMENT, 1);
				glReadPixels(0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, buffer.data());

				for (int y = 0; y < height / 2; y++)
				{
					for (int x = 0; x < width; x++)
					{
						int pix1 = 3 * width * y + 3 * x;
						int pix2 = 3 * width * (height - 1 - y) + 3 * x;

						for (int c = 0; c < 3; c++)
						{
							unsigned char temp = buffer[pix1 + c];
							buffer[pix1 + c] = buffer[pix2 + c];
							buffer[pix2 + c] = temp;
						}
					}
				}

				error = lodepng::encode(filepath, buffer, width, height, LCT_RGB);
			}
			else
			{
				error = lodepng::encode(filepath, rayTracer.getFinalBuffer().data(), rayTracer.getFinalBuffer().size().x, rayTracer.getFinalBuffer().size().y, LCT_RGBA);
			}

			if (error)
			{
				std::cout << "encoder error " << error << ": " << lodepng_error_text(error) << std::endl;
				return;
			}

			if (openImage)
			{
				ShellExecuteA(0, 0, filepath.c_str(), 0, 0, SW_SHOW);
			}
		}

		void screenshot(bool screenshotAll, bool openImage = false)
		{
			std::string filename;
			int fileId = 0;

			do
			{
				filename = "screenshot" + std::to_string(fileId) + ".png";
				fileId++;
			} while (std::ifstream(imageDir + filename));

			screenshot(filename, screenshotAll, openImage);
		}

		void setupScene(SceneType sceneType)
		{
			currentSceneType = sceneType;

			rayTracer.clearScene();

			switch (currentSceneType)
			{
			case ST_Triangle:
				rayTracer.addNode(triNode);
				break;
			case ST_Sphere:
				rayTracer.addNode(sphereNode);
				break;
			case ST_SDFSphere:
				rayTracer.addNode(sdfSphereNode);
				break;
			case ST_SDFEllipsoid:
				rayTracer.addNode(sdfEllipsoidNode);
				break;
			case ST_SDFBox:
				rayTracer.addNode(sdfBoxNode);
				break;
			case ST_SDFTorus:
				rayTracer.addNode(sdfTorusNode);
				break;
			case ST_SDFTriangle:
				rayTracer.addNode(sdfTriangleNode);
				break;
			case ST_SDFBend:
				rayTracer.addNode(sdfBendNode);
				break;
			case ST_SDFMesh:
				rayTracer.addNode(sdfMeshNode);
				break;
			case ST_Voxel:
				rayTracer.addNode(voxelNode);
				break;
			case ST_Voxel2:
				rayTracer.addNode(voxelNode2);
				break;
			case ST_Voxel3:
				rayTracer.addNode(voxelNode3);
				break;
			default:
				if (currentSceneType >= ST_VoxelTestScene0 && currentSceneType <= ST_VoxelTestScene9)
				{
					setupVoxelTestScene(currentSceneType - ST_VoxelTestScene0, 100, true);
				}
				break;
			}
		}

		void makeEquilateralTriangle(const vec3 & pos, Float a, Vertex<T> & vA, Vertex<T> & vB, Vertex<T> & vC)
		{
			Float h2 = 0.5 * 0.866025 * a;

			vA.pos = pos + vec3(-0.5 * a, -h2, 0.0);
			vB.pos = pos + vec3(0.0, h2, 0.0);
			vC.pos = pos + vec3(0.5 * a, -h2, 0.0);

			vA.uv = vec2(0.0, 0.0);
			vB.uv = vec2(0.5, 1.0);
			vC.uv = vec2(1.0, 0.0);

			//vA.uv = vec2(0.1, 0.1);
			//vB.uv = vec2(0.2, 0.5);
			//vC.uv = vec2(1.0, 0.3);
		}

		void setupVoxelScene()
		{
			return;
			//VoxelGeometry<Float> voxelScene(glm::ivec3(400));
			VoxelGeometry<Float> voxelScene(glm::ivec3(100));

			std::shared_ptr<Texture2DCpu<T>> text1Tex = std::make_shared<TextureMip2DCpu<T>>();
			text1Tex->load("textures/text1.png");

			// test
			//text1Tex->saveMipMaps("images/mipmapgentest/test.png");

			Material material;
			material.isMirror = false;
			material.texId = 0;

			voxelScene.setMaterial(0, material);

			material.isMirror = true;

			voxelScene.setMaterial(1, material);

			//voxelScene.setTexture(0, woodTex1);

			voxelScene.setTexture(0, text1Tex);

			//voxelScene.create(SignedDistanceMesh<T>(meshObj.vertices.data(), meshObj.vertices.size()));
			//voxelScene.create(SignedDistanceSphere<T>(), NodeTransform(glm::vec3(0.3, 0.0, 0.3), glm::vec3(0.0), glm::vec3(0.2)), 0);
			//voxelScene.create(SignedDistanceSphere<T>(), NodeTransform(glm::vec3(-0.3, 0.0, 0.0), glm::vec3(0.0), glm::vec3(0.2)), 1);
			//voxelScene.create(SignedDistanceBend<T>(std::make_shared<SignedDistanceBox<T>>(glm::vec3(0.5f, 0.5f, 0.1f))));
			
			//voxelScene.create(SignedDistanceSphere<T>(), NodeTransform(glm::vec3(0.0, 0.0, -1.0), glm::vec3(0.0), glm::vec3(0.3)), 1);

			voxelScene.create(SignedDistanceBox<T>(glm::vec3(0.8, 0.8, 0.1)), 
				NodeTransform(glm::vec3(0.0, 0.0, 0.0), glm::vec3(0.0), glm::vec3(1.0)), 0);

			//voxelScene.create(SignedDistanceBox<T>(glm::vec3(0.2)),
				//NodeTransform(glm::vec3(0.0, 0.0, 0.7), glm::vec3(0.0), glm::vec3(1.0)), 1);

			//voxelScene.create(SignedDistanceBox<T>(glm::vec3(0.2)),
				//NodeTransform(glm::vec3(0.0, 0.0, -0.7), glm::vec3(0.0), glm::vec3(1.0)), 1);

			//voxelScene.create(SignedDistanceSphere<T>(0.2), 
				//NodeTransform(glm::vec3(0.0, 0.0, -0.7), glm::vec3(0.0), glm::vec3(1.0)), 1);

			voxelScene.create(SignedDistanceBend<T>(std::make_shared<SignedDistanceBox<T>>(glm::vec3(0.3f, 0.3f, 0.25)), 45.0),
				NodeTransform(vec3(0, 0, -1), vec3(0), vec3(1)), 1);

			//voxelScene.create(SignedDistanceBox<T>(glm::vec3(0.2, 0.2, 0.1)),
				//NodeTransform(glm::vec3(0.0, 0.0, 0.8), glm::vec3(0.0), glm::vec3(1.0)), 1);

			voxelScene.setScale(1.0f);

			voxelNode3 = std::make_shared<SceneNodeCpu<Float, VoxelGeometry<Float>>>(*this);
			voxelNode3->addGeometry(voxelScene);
		}

	public:
		RayTracerCpuEnvironment(VoxelGI& engine)
			: voxelGI(engine),
			rayTracer(voxelGI),
			cameraControl(voxelGI, rayTracer.getCamera())
		{

		}

		~RayTracerCpuEnvironment()
		{
			std::cout << "RayTracerCpuEnvironment destroyed" << std::endl;
		}

		// Inherited via Environment

		virtual void guiUpdate()
		{
			Gui::Begin("Debug");

			if (ImGui::TreeNode("General"))
			{
				if (ImGui::Combo("Scene", (int*)&currentSceneType, sceneTypeStr, IM_ARRAYSIZE(sceneTypeStr)))
					setupScene(currentSceneType);

				Gui::Property("View Resolution", viewResolution);

				ImGui::TreePop();
			}

			rayTracer.drawDebugGui();

			if (ImGui::TreeNode("Plot"))
			{
				ImGui::InputFloat2("Plot point", glm::value_ptr(plotPoint));

				const char* plotModes[] =
				{
					"X",
					"Y",
					"Z",
					"Magnitude",
				};

				ImGui::Combo("Plot Mode", (int*)&plotPointMode, plotModes, IM_ARRAYSIZE(plotModes));

				const Buffer2D<vec4>& outputBuffer = rayTracer.getOutputBuffer();

				std::vector<float> plotDataX, plotDataY;

				glm::ivec2 plotPointAbs = rayTracer.relToAbsCoords(plotPoint);

				outputBuffer.extractValues((Buffer2D<vec4>::DataComponent)plotPointMode, plotPointAbs.y, false, plotDataX);
				outputBuffer.extractValues((Buffer2D<vec4>::DataComponent)plotPointMode, plotPointAbs.x, true, plotDataY);

				ImGui::PlotLines("Value X", plotDataX.data(), plotDataX.size());
				ImGui::PlotLines("Value Y", plotDataY.data(), plotDataY.size());

				ImGui::TreePop();
			}

			//ImGui::PlotLines("Value Y", rayTracer.getPlotYData().data(), rayTracer.getPlotYData().size());

			ImGui::Text(rayTracer.getLogStream().str().c_str());

			//Gui::Property<glm::ivec2>(rayTracer, 0, "Resolution");

			if (ImGui::Button("Render"))
				rayTracer.requestRender();

			ImGui::SameLine();

			if (ImGui::Button("Screenshot All"))
				screenshot(true, voxelGI.getInput().getKey(GLFW_KEY_LEFT_SHIFT));

			ImGui::SameLine();

			if (ImGui::Button("Screenshot Frame"))
				screenshot(false, voxelGI.getInput().getKey(GLFW_KEY_LEFT_SHIFT));

			ImGui::SameLine();

			//if (ImGui::Button("Save Settings"))
			//Gui::SaveNextFrame();

			if (Gui::Button("Clear Console"))
				system("cls");

			ImGui::SameLine();

			if (Gui::Button("Batch Process"))
				batchProcess();

			if (rayTracer.continuousRenderEnabled())
			{
				ImGui::SameLine();
				ImGui::Checkbox("Freeze Controls", &freezeControls);
			}

			Gui::End();
		}

		virtual void update()
		{
			if (rayTracer.continuousRenderEnabled())
				cameraControl.update();

			if (voxelGI.getInput().getMouseButton(GLFW_MOUSE_BUTTON_RIGHT))
			{
				auto mousePos = voxelGI.getInput().getMousePosition();

				int posX = mousePos.x - std::max(voxelGI.screenWidth() - voxelGI.screenHeight(), 0);
				int posY = mousePos.y;

				//std::cout << posX << " " << posY << std::endl;

				auto val = rayTracer.getOutputBuffer().get(posX, posY);

				std::cout << "(" << posX << ", " << posY << ") -> " << val.x << " " << val.y << " " << val.z << std::endl;
			}

			rayTracer.render();
		}

		virtual void render()
		{
			GLPushState<GLState::Viewport> viewportState(
				voxelGI.screenWidth() - voxelGI.screenHeight(), 
				0, 
				voxelGI.screenHeight(), 
				voxelGI.screenHeight());

			glm::vec4 backgroundColor = rayTracer.getBackgroundColor();
			glm::vec2 offset;
			glm::vec2 scale;

			/*if (voxelGI.getAspect() >= 1.0f)
				scale = glm::vec2(1.0f / voxelGI.getAspect(), 1.0f);
			else
				scale = glm::vec2(1.0f, 1.0f / voxelGI.getAspect());

			offset = glm::vec2(1.0f) - scale;*/

			scale = glm::vec2(viewResolution) / glm::vec2(voxelGI.screenHeight(), voxelGI.screenHeight());

			glClearColor(backgroundColor.r, backgroundColor.g, backgroundColor.b, backgroundColor.a);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			shader.bind();
			shader.setUniform("offset", offset);
			shader.setUniform("scale", scale);
			shader.setUniform("renderTex", rayTracer.getTexture(), 0);

			quadMesh.draw();
		}

		virtual Camera & getCamera()
		{
			return Camera();
		}

		void setupVoxelTestScene(int sceneId, int res, bool load = false)
		{
			for (int i = 0; i < numVoxelTestScenes; i++)
				voxelTestSceneSetup[i].release();

			auto& scene = voxelTestSceneSetup[sceneId];
			scene.init(this);

			scene.backgroundColor = glm::vec4(0.0f, 0.0f, 0.5f, 1.0f);

			VoxelGeometry<Float> voxelGrid;

			int stdMaterial = 2;
			int textMaterial = 0;
			int mirrorMaterial = 1;

			voxelGrid.setTexture(0, text1Tex);

			Material material;
			material.isMirror = false;
			material.texId = 0;

			material.diffuse = glm::vec3(1.0f);
			material.specular = glm::vec3(0.5f);
			material.shininess = 40.0f;

			voxelGrid.setMaterial(textMaterial, material);

			material.isMirror = true;
			material.texId = -1;

			voxelGrid.setMaterial(mirrorMaterial, material);

			material.diffuse = glm::vec3(0.0f, 0.0f, 1.0f);
			material.isMirror = false;
			
			voxelGrid.setMaterial(stdMaterial, material);

			voxelGrid.setResolution(res);

			switch (sceneId)
			{
			case 0: //Normal test
				scene.camPos = glm::vec3(0.0f, 0.0f, 0.8f);
				scene.ssaa = 2;
				//voxelGrid.setResolution(32);
				voxelGrid.create(SignedDistanceSphere<T>(0.5f), NodeTransform(), stdMaterial);
				break;
			case 1:
				scene.camPos = glm::vec3(0.0f, 0.0f, 2.0f);
				//voxelGrid.setResolution(100);
				voxelGrid.create(SignedDistanceSphere<T>(0.5f), NodeTransform(glm::vec3(0.0f, 0.0f, 0.0f)), textMaterial);
				break;
			case 2:
				scene.camPos = glm::vec3(0.0f, 0.0f, 0.0f);
				//voxelGrid.setResolution(100);
				voxelGrid.create(SignedDistanceSphere<T>(0.25f), NodeTransform(glm::vec3(0.0f, 0.0f, -0.5f)), mirrorMaterial);
				voxelGrid.create(SignedDistanceBox<T>(glm::vec3(1.0f, 1.0f, 0.5f)), NodeTransform(glm::vec3(0.0f, 0.0f, 0.6f)), textMaterial);
				break;
			case 3:
				scene.camPos = glm::vec3(0.0f, 0.0f, 0.0f);
				//voxelGrid.setResolution(400);
				voxelGrid.create(SignedDistanceBend<T>(std::make_shared<SignedDistanceBox<T>>(glm::vec3(0.25f, 0.25f, 0.2f), 0.05f)),
					NodeTransform(glm::vec3(0.0f, 0.0f, -0.6f)),
					mirrorMaterial);
				voxelGrid.create(SignedDistanceBox<T>(glm::vec3(1.0f, 1.0f, 0.5f)), NodeTransform(glm::vec3(0.0f, 0.0f, 0.6f)), textMaterial);
				break;
			case 4:
				scene.camPos = glm::vec3(0.0f, 0.0f, 1.5f);
				//voxelGrid.setResolution(100);
				voxelGrid.create(SignedDistanceBox<T>(glm::vec3(0.5f)), NodeTransform(), stdMaterial);
				break;
			default:
				return;
			}

			scene.node->addGeometry(voxelGrid);

			if (load)
				scene.load();
		}

		void batchProcess()
		{
			glm::ivec2 screenRes(512, 512);
			int lowRes = 64;
			int highRes = 512;

			std::string sceneName = "spere_mirror";
			int voxelScene = 2;

			rayTracer.setMaxBounces(1);
			//rayTracer.setBackgroundColor(glm::vec4(0.0f, 0.0f, 0.5f, 1.0f));
			rayTracer.setRenderMode(RayTracerCpu<T>::RM_Material);
			rayTracer.setPixelResolution(screenRes);
			rayTracer.setSSAA(0);
			rayTracer.setProgressTrackingEnable(true);

			float batchStartTime = voxelGI.getTimer().time();

			// 1 Low res
			setupVoxelTestScene(voxelScene, lowRes, true);
			auto voxelGrid = (VoxelGeometry<T>*)voxelTestSceneSetup[voxelScene].node->geometry(0);

			// 1.1 Lod Low Linear
			voxelGrid->setMipMapLodMode(VoxelGeometry<T>::MMLM_Differentials);
			voxelGrid->setInterpolator<IT_Linear>();

			rayTracer.renderFrame();
			screenshot(sceneName + "_low_lod_linear.png", false, false);

			// 1.2 Lod Low Cubic
			voxelGrid->setMipMapLodMode(VoxelGeometry<T>::MMLM_Differentials);
			voxelGrid->setInterpolator<IT_Cubic>();

			rayTracer.renderFrame();
			screenshot(sceneName + "_low_lod_cubic.png", false, false);

			// 2 High res
			setupVoxelTestScene(voxelScene, highRes, true);
			voxelGrid = (VoxelGeometry<T>*)voxelTestSceneSetup[voxelScene].node->geometry(0);

			// 2.1 High No Lod
			voxelGrid->setMipMapLodMode(VoxelGeometry<T>::MMLM_Disabled);
			voxelGrid->setInterpolator<IT_Cubic>();

			rayTracer.renderFrame();
			screenshot(sceneName + "_high_nolod_cubic.png", false, false);

			// 2.2 High Lod Linear
			voxelGrid->setMipMapLodMode(VoxelGeometry<T>::MMLM_Differentials);
			voxelGrid->setInterpolator<IT_Linear>();

			rayTracer.renderFrame();
			screenshot(sceneName + "_high_lod_linear.png", false, false);

			// 2.3 High Lod Cubic
			voxelGrid->setMipMapLodMode(VoxelGeometry<T>::MMLM_Differentials);
			voxelGrid->setInterpolator<IT_Cubic>();

			rayTracer.renderFrame();
			screenshot(sceneName + "_high_lod_cubic.png", false, false);

			// 2.4 High Lod Cubic SSAA
			voxelGrid->setMipMapLodMode(VoxelGeometry<T>::MMLM_Differentials);
			voxelGrid->setInterpolator<IT_Cubic>();
			rayTracer.setSSAA(2);

			rayTracer.renderFrame();
			screenshot(sceneName + "_high_lod_cubic_ssaa.png", false, false);

			std::cout << "batch total time: " << voxelGI.getTimer().time() - batchStartTime << "s" << std::endl;
		}

		virtual void init() override
		{
			shader.createFromFile("RayTracerCpu.vert", "RayTracerCpu.frag");

			quadMesh.create(MP_Quad);

			//camera.setTranslation(glm::vec3(0.0f, 0.0f, 5.0f));
			//camera.setProjection(45.0f, (float)voxelGI.screenWidth() / voxelGI.screenHeight(), 0.05f, 1000.0f);

			//int res = glm::min(voxelGI.screenWidth(), voxelGI.screenHeight());
			int res = 64;

			rayTracer.setPixelResolution(glm::ivec2(res));
			//rayTracer.setBackgroundColor(glm::vec4(0.0f, 0.0f, 0.5f, 1.0f));
			rayTracer.setBackgroundColor(glm::vec4(0.0f, 0.0f, 0.5f, 1.0f));

			/*
			B
			/ \
			A---C
			*/

			woodTex1 = static_cast<std::shared_ptr<Texture2DCpu<T>>>(std::make_shared<TextureMip2DCpu<T>>());
			woodTex1->load("textures/wood1_512.png");

			//woodTex1->saveMipMaps("images/mipmaptest/test.png");

			std::vector<int> shapeOffsets;
			MeshLoader::load("Data/Meshes/cube.obj", meshObj, shapeOffsets);
			//MeshLoader::load("Data/Meshes/sphere.obj", meshObj, shapeOffsets);


			Vertex<T> vA, vB, vC;

			/*vA.pos = glm::vec3(-1.0f, -1.0f, 0.0f);
			vB.pos = glm::vec3(0.0f, 1.0f, 0.0f);
			vC.pos = glm::vec3(1.0f, -1.0f, 0.0f);*/

			makeEquilateralTriangle(vec3(0, 0, 0), 2.0, vA, vB, vC);

			vA.normal = glm::vec3(1.0f, 0.0f, 0.0f);
			vB.normal = glm::vec3(0.0f, 1.0f, 0.0f);
			vC.normal = glm::vec3(0.0f, 0.0f, 1.0f);

			vA.normal = glm::normalize(vA.normal);
			vB.normal = glm::normalize(vB.normal);
			vC.normal = glm::normalize(vC.normal);

			triNode = std::make_shared<SceneNodeCpu<Float, Triangle<Float>>>(*this);

			triNode->addGeometry(Triangle<Float>(vA, vB, vC));
			//triNode->transform().setTranslation(glm::vec3(0.0f, 0.0f, -2.0f));

			sphereNode = std::make_shared<SceneNodeCpu<Float, Sphere<Float>>>(*this);
			//sphereNode->transform().setTranslation(glm::vec3(0.9f, 0.0f, 0.0f));

			sphereNode->addGeometry(Sphere<Float>(glm::vec3(0.0, 0.0, 0.0), 1.0f));

			//SignedDistanceAnalytic<Float> sdfAnalytic;
			/*
			//VoxelGeometry<Float> voxelSphereGeo(glm::ivec3(100));
			VoxelGeometry<Float> voxelSphereGeo(glm::ivec3(50));
			//voxelGeo.createSphere(1.0f);
			//voxelGeo.createBox(glm::vec3(1.0f));
			//voxelGeo.createTorus(glm::vec2(0.7f, 0.25f));
			voxelSphereGeo.create(SignedDistanceSphere<T>(0.5f));
			//voxelSphereGeo.create(SignedDistanceSphere<T>(1.0f));
			voxelSphereGeo.setScale(2.0f);
			//voxelGeo.create(SignedDistanceBox<T>(glm::vec3(0.7f, 0.5f, 0.3f)));
			//voxelSphereGeo.create(SignedDistanceTorus<T>(glm::vec2(0.7f, 0.25f)));

			voxelSphereGeo.setTexture(0, woodTex1);

			voxelNode = std::make_shared<SceneNodeCpu<Float, VoxelGeometry<Float>>>(*this);

			voxelNode->addGeometry(voxelSphereGeo);
			// voxelNode->transform().setTranslation(glm::vec3(0.0f, 0.0f, -2.0f));

			voxelNode2 = std::make_shared<SceneNodeCpu<Float, VoxelGeometry<Float>>>(*this);

			VoxelGeometry<Float> voxelTriangle(glm::ivec3(50));
			voxelTriangle.create(SignedDistanceTriangle<T>(Triangle<Float>(vA, vB, vC)));

			voxelTriangle.setTexture(0, woodTex1);

			voxelNode2->addGeometry(voxelTriangle);

			setupVoxelScene();*/

			text1Tex->load("textures/text1.png");

			//for (int i = 0; i < 10; i++)
				//setupVoxelTestScene(i);

			sdfTriangleNode = std::make_shared<SceneNodeCpu<Float, SignedDistanceTriangle<T>>>(*this);
			sdfTriangleNode->addGeometry(SignedDistanceTriangle<T>(Triangle<T>(vA, vB, vC)));

			sdfMeshNode = std::make_shared<SceneNodeCpu<Float, SignedDistanceMesh<T>>>(*this);
			sdfMeshNode->addGeometry(SignedDistanceMesh<T>(meshObj.vertices.data(), meshObj.vertices.size()));

			sdfBendNode = std::make_shared<SceneNodeCpu<Float, SignedDistanceBend<T>>>(*this);
			sdfBendNode->addGeometry(SignedDistanceBend<T>(std::make_shared<SignedDistanceBox<T>>(glm::vec3(0.5f, 0.5f, 0.1f))));

			sdfSphereNode = std::make_shared<SceneNodeCpu<Float, SignedDistanceSphere<T>>>(*this);
			//sdfSphereNode->transform().setTranslation(glm::vec3(0.9f, 0.0f, 0.0f));
			sdfSphereNode->addGeometry(SignedDistanceSphere<T>(1.0f));

			sdfEllipsoidNode = std::make_shared<SceneNodeCpu<Float, SignedDistanceEllipsoid<T>>>(*this);
			//sdfEllipsoidNode->addGeometry(SignedDistanceEllipsoid<T>(vec3(1.0f, 0.5f, 1.0f)));
			sdfEllipsoidNode->addGeometry(SignedDistanceEllipsoid<T>(vec3(0.25f, 0.25f, 0.25f)));

			sdfBoxNode = std::make_shared<SceneNodeCpu<Float, SignedDistanceBox<T>>>(*this);
			sdfBoxNode->addGeometry(SignedDistanceBox<T>(glm::vec3(0.7f, 0.5f, 0.3f)));

			sdfTorusNode = std::make_shared<SceneNodeCpu<Float, SignedDistanceTorus<T>>>(*this);
			sdfTorusNode->addGeometry(SignedDistanceTorus<T>(glm::vec2(0.7f, 0.25f)));

			//setupScene(ST_Triangle);
			setupScene(currentSceneType);
		}

		virtual glm::ivec2 getResolution() const
		{
			return rayTracer.getSampleResolution();
			//return rayTracer.getPixelResolution();
		}

		RayTracerCpu<Float>& getRayTracer()
		{
			return rayTracer;
		}
	};
}