
#pragma once

#include "MultiBuffer.h"
#include "VoxelCell.h"

#include <glm/gtx/vector_angle.hpp>

#include "SignedDistanceGeometry.h"
#include "SignedDistanceSphere.h"
#include "SignedDistanceBox.h"
#include "SignedDistanceTorus.h"
#include "SignedDistanceTriangle.h"
#include "SignedDistanceEllipsoid.h"

#include <numeric>
#include <memory>
#include <array>
#include <vector>

#include "VoxelInterpolatorNearest.h"
#include "VoxelInterpolatorTrilinear.h"
#include "NodeTransform.h"

#include "VoxelInterpolatorKernel.h"
#include "Interpolation.h"

namespace VoxelGI
{
	template <typename T>
	class VoxelGeometry : public SignedDistanceGeometry<T>
	{
	public:
		enum MipMapLodMode
		{
			MMLM_Disabled,
			MMLM_RayDistance,
			MMLM_Differentials,

			MipMapLodModeCount
		};

	private:
		class VoxelData;

		VoxelData voxelData;

		std::vector<Material> materials;
		std::vector<std::shared_ptr<Texture2DCpu<T>>> textures;

		const char* mipMapLodModeStr[MipMapLodModeCount] =
		{
			"Disabled",
			"RayDistance",
			"Differentials"
		};

		MipMapLodMode mipMapLodMode;

		vec3 coordsAbsToRel(const glm::ivec3& coords)
		{
			vec3 bufferDim((T)voxelData.resolutionX(), (T)voxelData.resolutionY(), (T)voxelData.resolutionZ());

			return ((vec3(coords) + vec3(0.5)) / bufferDim) * (T)2.0 - vec3(1.0);
		}

		template <bool applyTransform, typename S, typename U>
		void computeSDF(S sdfFunc, U uvFunc, const glm::mat4& transRotMatInv, float scaleVal, int materialId)
		{
			for (int z = 0; z < voxelData.resolutionZ(); z++)
				for (int y = 0; y < voxelData.resolutionY(); y++)
					for (int x = 0; x < voxelData.resolutionX(); x++)
					{
						vec3 pos = coordsAbsToRel(glm::ivec3(x, y, z));
						vec3 posLocal;
						T sdf;

						if (applyTransform)
						{
							posLocal = vec3(transRotMatInv * vec4(pos, 1.0)) / (T)scaleVal;
							sdf = sdfFunc(posLocal) * (T)scaleVal;
						}
						else
						{
							posLocal = pos;
							sdf = sdfFunc(posLocal);
						}

						if (sdf <= voxelData.buffer<VoxelData::VA_Sdf, T>()->get(x, y, z))
						{
							voxelData.buffer<VoxelData::VA_Sdf, T>()->get(x, y, z) = sdf;

							//vec2 uv = uvFunc(pos);
							vec2 uv = uvFunc(posLocal);

							if (uv.x < 0.0)
								uv = vec2((T)x / (voxelData.resolutionX() - 1), (T)y / (voxelData.resolutionY() - 1));

							voxelData.buffer<VoxelData::VA_U, T>()->get(x, y, z) = uv.x;
							voxelData.buffer<VoxelData::VA_V, T>()->get(x, y, z) = uv.y;

							voxelData.buffer<VoxelData::VA_MaterialId, unsigned char>()->get(x, y, z) = materialId;
						}
					}
		}

		vec3 projectToObjectAbs(const vec3& relPos) const
		{
			return ((relPos + vec3(1.0)) * (T)0.5) * vec3(getResolution());
		}

		vec3 projectToObjectAbsDiff(const vec3& relPosDiff) const
		{
			return (relPosDiff * (T)0.5) * vec3(voxelData->sizeX(), voxelData->sizeY(), voxelData->sizeZ());
		}

	public:
		template <InterpolationType interpolationType>
		using VoxelInterpolatorLib = VoxelInterpolatorKernel<
			T,
			Interpolation<T>::kernelWidth<interpolationType>(),
			&Interpolation<T>::interpolate<interpolationType>,
			&Interpolation<had::AReal>::interpolate<interpolationType>>;

		VoxelGeometry(const glm::ivec3& resolution = glm::ivec3())
			: materials(16), textures(16)
		{
			voxelData.setInterpolator<IT_Cubic>();
			voxelData.setScale(scale);

			if (resolution.x != 0)
				setResolution(resolution);

			defaultMaterial.isMirror = false;
			defaultMaterial.texId = -1;
		}

		~VoxelGeometry() {}

		virtual void setMaterial(int i, const Material& material)
		{
			materials[i] = material;
		}

		virtual void setTexture(int i, const std::shared_ptr<Texture2DCpu<T>>& tex)
		{
			textures[i] = tex;
		}

		int getMaterialId(const vec3& pos) const
		{
			glm::ivec3 cell = glm::clamp(glm::ivec3(glm::round(projectToObjectAbs(projectPosToObject(pos)) - (T)0.5)), 
				glm::ivec3(0), 
				getResolution() - 1);
			
			return voxelData.buffer<VoxelData::VA_MaterialId, unsigned char>()->get(cell.x, cell.y, cell.z);
		}

		virtual void getMaterial(const vec3& position, Material& materialOut) const
		{
			int matId = getMaterialId(position);

			if (matId != 0xff)
				materialOut = materials[matId];
			else
				materialOut = defaultMaterial;
		}

		T selectLod(const std::shared_ptr<Texture2DCpu<T>>& tex, 
			const vec2& duvdx, const vec2& duvdy) const
		{
			T lod;

			switch (mipMapLodMode)
			{
			case MMLM_Disabled:
				lod = (T)0;
				break;
			case MMLM_RayDistance:
				break;
			case MMLM_Differentials:
			{
				vec2 texelPixelRatio = vec2(tex->size()) / vec2(environment->getResolution());

				vec2 duvMax = glm::length2(duvdx) >= glm::length2(duvdy)
					? duvdx
					: duvdy;
				// * 2 ?
				const T additionalScale = (T)1.0;
				lod = std::log2(glm::length(duvMax * texelPixelRatio * additionalScale));
				break;
			}
			}

			return lod;
		}

		bool getTextureValue(const RayDifferential<Float>& rayDiff, int texId, glm::vec4& texValOut) const
		{
			if (texId != 0xff)
			{
				vec2 duvdx, duvdy;
				getDuv(rayDiff, duvdx, duvdy);

				T lod = selectLod(textures[texId], duvdx, duvdy);

				//std::cout << "lod: " << lod << std::endl;

				texValOut = textures[texId]->sample(getUV(rayDiff.getPosition()), lod);
				return true;
			}
			else
				texValOut = glm::vec4(1.0f);
				return false;
		}

		virtual glm::vec4 surfaceColor(const RayDifferential<Float>& rayDiff) const
		{
			vec3 position = rayDiff.getPosition();

			Material mat;
			getMaterial(position, mat);

			if (mat.texId != 0xff && !mat.isMirror)
			{
				vec2 duvdx, duvdy;
				getDuv(rayDiff, duvdx, duvdy);

				T lod = selectLod(textures[mat.texId], duvdx, duvdy);

				//std::cout << "lod: " << lod << std::endl;

				return textures[mat.texId]->sample(getUV(position), lod);
			}
			else
				return glm::vec4(surfaceNormal(position), 1.0);
		}

		void setResolution(int res)
		{
			setResolution(glm::ivec3(res));
		}

		void setResolution(const glm::ivec3& resolution)
		{
			glm::ivec3 res = glm::max(resolution, glm::ivec3(1));

			voxelData.create(res.x, res.y, res.z);
		}

		glm::ivec3 getResolution() const
		{
			return glm::ivec3(voxelData.resolutionX(), voxelData.resolutionY(), voxelData.resolutionZ());
		}

		void create(const SignedDistanceGeometry<T>& geo)
		{
			glm::mat4 transformMat;

			computeSDF<false>(
				[&geo](const vec3& pos) { return geo.getSignedDistance(pos); },
				[&geo](const vec3& pos) { return geo.getUV(pos); },
				transformMat,
				1.0f,
				0xff);
		}

		void create(const SignedDistanceGeometry<T>& geo, const glm::mat4& transRotMat, float scaleVal, int materialId)
		{
			computeSDF<true>(
				[&geo](const vec3& pos) { return geo.getSignedDistance(pos); },
				[&geo](const vec3& pos) { return geo.getUV(pos); },
				glm::inverse(transRotMat),
				scaleVal,
				materialId);
		}

		void create(const SignedDistanceGeometry<T>& geo, const NodeTransform& nodeTransform, int materialId)
		{
			glm::mat4 transMat, rotMat;

			nodeTransform.getTranslationMatrix(transMat);
			nodeTransform.getRotationMatrix(rotMat);

			create(geo, rotMat * transMat, nodeTransform.getScale().x, materialId);
		}

		virtual void copy(IGeometry<T>& geoOut) const
		{
			VoxelGeometry<T>& s = (VoxelGeometry<T>&)geoOut;

			s = *this;
		}

		virtual void getDn(
			const RayDifferential<Float>& rayDiff,
			NormalDifferential& dn) const
		{
			if (shadingNormalType == SN_Differential)
			{
				vec3 pos = projectPosToObject(rayDiff.getPosition());

				vec3 gradientVec;
				glm::tmat3x3<T> hessianMat;

				voxelData.interpolator<VoxelData::VA_Sdf>()->hessian(pos, gradientVec, hessianMat);

				vec3 dtdp = vec3(getResolution()) / (T)scale;

				dn.n = gradientVec * dtdp;
				dn.dndx = (hessianMat * (dtdp * rayDiff.getDpdx())) * dtdp * (T)0.5;
				dn.dndy = (hessianMat * (dtdp * rayDiff.getDpdy())) * dtdp * (T)0.5;

				dn.computeDN(true);
			}
			else if (shadingNormalType == SN_DifferentialInterpolated)
			{
				vec3 pos = projectPosToObject(rayDiff.getPosition());

				//((VoxelInterpolatorTrilinear<T>*)voxelInterpolator.get())->gradientInterpolatedDifferential(pos, rayDiff, dn);

				std::cout << "not differential" << std::endl;
			}
			else if (shadingNormalType == SN_PrecalculatedInterpolated)
			{
				/*dn.n = shadingNormal(rayDiff.getPosition());
				dn.dndx = filterDiff<VA_Normal, vec3>(rayDiff.getPosition(), rayDiff.getDpdx());
				dn.dndy = filterDiff<VA_Normal, vec3>(rayDiff.getPosition(), rayDiff.getDpdy());

				dn.computeDN(true);*/

				std::cout << "not differential" << std::endl;
			}
		}

		virtual void getDuv(
			const RayDifferential<Float>& rayDiff,
			vec2& duvdx,
			vec2& duvdy) const
		{
			vec3 pos = projectPosToObject(rayDiff.getPosition());
			vec3 dtdp = vec3(getResolution()) / (T)scale;

			vec3 gradU = voxelData.interpolator<VoxelData::VA_U>()->gradient(pos) * dtdp;
			vec3 gradV = voxelData.interpolator<VoxelData::VA_V>()->gradient(pos) * dtdp;

			duvdx = vec2(
				glm::dot(gradU, (T)0.5 * rayDiff.getDpdx()),
				glm::dot(gradV, (T)0.5 * rayDiff.getDpdx())
			);

			duvdy = vec2(
				glm::dot(gradU, (T)0.5 * rayDiff.getDpdy()),
				glm::dot(gradV, (T)0.5 * rayDiff.getDpdy())
			);
		}

		virtual vec3 shadingNormal(const vec3& pos) const override
		{
			if (shadingNormalType == SN_Differential)
			{
				return surfaceNormal(pos);
			}
			else if (shadingNormalType == SN_DifferentialInterpolated)
			{
				return vec3();
				//return ((VoxelInterpolatorTrilinear<T>*)voxelInterpolator.get())->gradientInterpolated(projectPosToObject(pos));
			}
			else if (shadingNormalType == SN_PrecalculatedInterpolated)
			{
				return vec3();
				//return glm::normalize(filter<VA_Normal, vec3>(projectPosToObject(pos)));
			}
		}

		virtual T getSignedDistance(const vec3& pos) const
		{
			return voxelData.interpolator<VoxelData::VA_Sdf>()->filter(pos);
		}

		virtual vec2 getUV(const vec3& pos) const
		{
			return vec2(
				voxelData.interpolator<VoxelData::VA_U>()->filter(projectPosToObject(pos)),
				voxelData.interpolator<VoxelData::VA_V>()->filter(projectPosToObject(pos))
			);
		}

		virtual vec3 voxelSize() const
		{
			return (Float)2.0 / vec3(getResolution());
		}

		virtual void setScale(float s)
		{
			scale = s;
			voxelData.setScale(scale);
		}

		virtual vec3 getGradient(const vec3& pos) const
		{
			if (shadingNormalAsGradient)
				return shadingNormal(projectPosToWorld(pos));
			else
			{
				vec3 dtdp = vec3(getResolution()) / (T)scale;
				return voxelData.interpolator<VoxelData::VA_Sdf>()->gradient(pos) * dtdp;
			}
		}

		virtual SdfType getSdfType() const
		{
			return ST_Voxel;
		}

		static struct DebugGuiCache
		{
			int newType = 0;
			glm::ivec3 res = glm::ivec3(50, 50, 50);

			std::unique_ptr<SignedDistanceGeometry<T>> sdg = std::make_unique<SignedDistanceSphere<T>>();;
		} debugGuiCache;

		bool shadingNormalAsGradient;
		int interpolationMode = 1;

		enum ShadingNormalType
		{
			SN_Differential,
			SN_DifferentialInterpolated,
			SN_PrecalculatedInterpolated
		} shadingNormalType = SN_Differential;

		void setMipMapLodMode(MipMapLodMode mmMode)
		{
			mipMapLodMode = mmMode;
		}

		template <InterpolationType interpolationType>
		void setInterpolator()
		{
			interpolationMode = interpolationType;
			voxelData.setInterpolator<interpolationType>();
		}

		virtual void drawSdfDebugGui() override
		{
			Gui::Property("Shading as Surface Normal", shadingNormalAsGradient);
			Gui::Property("Shading Normal Type", shadingNormalType, {"Differential", "DifferentialInterpolated", "PrecalculatedInterpolated"});

			if (Gui::Property("Interpolation", interpolationMode,
				{
					"Linear",
					"Cubic"
				}))
			{
				switch (interpolationMode)
				{
				case 0:
					setInterpolator<IT_Linear>();
					break;
				case 1:
					setInterpolator<IT_Cubic>();
					break;
				}
			}

			Gui::Property<MipMapLodModeCount>("Lod Select", mipMapLodMode, mipMapLodModeStr);

			Gui::Text("New Data");

			Gui::Property("Resolution", debugGuiCache.res);

			if (Gui::Property("Content", debugGuiCache.newType,
				{
				"Sphere",
				"Ellipsoid",
				"Box",
				"Torus",
				"Triangle"
				}))
			{
				switch (debugGuiCache.newType)
				{
				case 0:
					debugGuiCache.sdg = std::make_unique<SignedDistanceSphere<T>>();
					break;
				case 1:
					debugGuiCache.sdg = std::make_unique<SignedDistanceEllipsoid<T>>();
					break;
				case 2:
					debugGuiCache.sdg = std::make_unique<SignedDistanceBox<T>>();
					break;
				case 3:
					debugGuiCache.sdg = std::make_unique<SignedDistanceTorus<T>>();
					break;
				case 4:
					debugGuiCache.sdg = std::make_unique<SignedDistanceTriangle<T>>();
					break;
				}
			}

			debugGuiCache.sdg->drawSdfDebugGui();

			if (Gui::Button("Create"))
			{
				setResolution(debugGuiCache.res);
				create(*debugGuiCache.sdg);
			}
		}
	};

	template <typename T>
	class VoxelGeometry<T>::VoxelData
	{
	public:
		enum VoxelAttribute
		{
			VA_Sdf,
			VA_U,
			VA_V,
			VA_MaterialId
		};

		static constexpr int attrCount = 4;

		class VoxelBuffer
		{
		private:
			template <typename U>
			void makeBuffer(std::shared_ptr<Buffer3D<U>>& buf)
			{
				buf = std::make_shared<Buffer3D<U>>();
			}

		public:
			std::shared_ptr<Buffer3D<T>> sdf;
			std::shared_ptr<Buffer3D<T>> u;
			std::shared_ptr<Buffer3D<T>> v;
			std::shared_ptr<Buffer3D<unsigned char>> materialId;

			VoxelBuffer()
			{
				makeBuffer(sdf);
				makeBuffer(u);
				makeBuffer(v);
				makeBuffer(materialId);
			}

			std::shared_ptr<IBuffer3D> operator[](int attr) const
			{
				switch (attr)
				{
				case VA_Sdf:
					return std::dynamic_pointer_cast<IBuffer3D>(sdf);
				case VA_U:
					return std::dynamic_pointer_cast<IBuffer3D>(u);
				case VA_V:
					return std::dynamic_pointer_cast<IBuffer3D>(v);
				case VA_MaterialId:
					return std::dynamic_pointer_cast<IBuffer3D>(materialId);
				}
			}

			template <int attr, typename U>
			const std::shared_ptr<Buffer3D<U>>& get() const
			{
				static_assert(false, "template error");
			}

			template <>
			const std::shared_ptr<Buffer3D<T>>& get<0>() const
			{
				return sdf;
			}

			template <>
			const std::shared_ptr<Buffer3D<T>>& get<1>() const
			{
				return u;
			}

			template <>
			const std::shared_ptr<Buffer3D<T>>& get<2>() const
			{
				return v;
			}

			template <>
			const std::shared_ptr<Buffer3D<unsigned char>>& get<3>() const
			{
				return materialId;
			}
		};

	private:
		
		VoxelBuffer voxelBuffer;
		std::shared_ptr<VoxelInterpolator<T>> voxelInterpolator[attrCount];
		

		T scale;

		std::vector<Material> materials;

		void assignInterpolator(int i, const std::shared_ptr<IBuffer3D>& buf)
		{
			if (voxelInterpolator[i] && buf)
			{
				auto bufSpec = std::dynamic_pointer_cast<Buffer3D<T>>(buf);

				if (bufSpec)
				{
					voxelInterpolator[i]->setData(bufSpec);
					voxelInterpolator[i]->setScale(scale);
				}
			}
		}

	public:
		template <VoxelAttribute A, typename U>
		const std::shared_ptr<Buffer3D<U>>& buffer() const
		{
			return voxelBuffer.get<A, U>();
		}

		template <VoxelAttribute A>
		const std::shared_ptr<VoxelInterpolator<T>>& interpolator() const
		{
			return voxelInterpolator[(int)A];
		}

		void create(int sizeX, int sizeY, int sizeZ)
		{
			for (int i = 0; i < attrCount; i++)
			{
				voxelBuffer[i]->resize(sizeX, sizeY, sizeZ);

				assignInterpolator(i, voxelBuffer[i]);
			}

			voxelBuffer.sdf->fill(std::numeric_limits<T>::infinity());
		}

		template <InterpolationType interpolationType>
		void setInterpolator()
		{
			setInterpolator<VoxelInterpolatorLib<interpolationType>>();
		}

		template <typename I>
		void setInterpolator()
		{
			for (int i = 0; i < attrCount; i++)
			{
				voxelInterpolator[i] = std::make_shared<I>();
				
				assignInterpolator(i, voxelBuffer[i]);
			}
		}

		void setScale(T s)
		{
			scale = s;

			for (int i = 0; i < attrCount; i++)
			{
				voxelInterpolator[i]->setScale(scale);
			}
		}

		int resolutionX() const
		{
			return voxelBuffer.sdf->sizeX();
		}

		int resolutionY() const
		{
			return voxelBuffer.sdf->sizeY();
		}

		int resolutionZ() const
		{
			return voxelBuffer.sdf->sizeZ();
		}
	};


	template <typename T>
	typename VoxelGeometry<T>::DebugGuiCache VoxelGeometry<T>::debugGuiCache;

}