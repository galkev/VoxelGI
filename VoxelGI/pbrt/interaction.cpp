
/*
    pbrt source code is Copyright(c) 1998-2016
                        Matt Pharr, Greg Humphreys, and Wenzel Jakob.

    This file is part of pbrt.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are
    met:

    - Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.

    - Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
    IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
    TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
    PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
    HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
    SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
    LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
    DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
    THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
    OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

 */


// core/interaction.cpp*
#include "interaction.h"
//#include "transform.h"
//#include "primitive.h"
#include "shape.h"
//#include "light.h"



namespace pbrt {





// SurfaceInteraction Method Definitions
SurfaceInteraction::SurfaceInteraction(
    const Point3f &p, const Vector3f &pError, const Point2f &uv,
    const Vector3f &wo, const Vector3f &dpdu, const Vector3f &dpdv,
    const Normal3f &dndu, const Normal3f &dndv, Float time, const Shape *shape,
    int faceIndex)
    : Interaction(p, Normal3f(Normalize(Cross(dpdu, dpdv))), pError, wo, time,
                  nullptr),
      uv(uv),
      dpdu(dpdu),
      dpdv(dpdv),
      dndu(dndu),
      dndv(dndv),
      shape(shape),
      faceIndex(faceIndex) {
    // Initialize shading geometry from true geometry
    shading.n = n;
    shading.dpdu = dpdu;
    shading.dpdv = dpdv;
    shading.dndu = dndu;
    shading.dndv = dndv;


	/*
    // Adjust normal based on orientation and handedness
    if (shape &&
        (shape->reverseOrientation ^ shape->transformSwapsHandedness)) {
        n *= -1;
        shading.n *= -1;
    }*/
}

inline glm::vec3 Faceforward(const glm::vec3 &n, const glm::vec3 &v) {
	return (Dot(n, v) < 0.f) ? -n : n;
}

void SurfaceInteraction::SetShadingGeometry(const Vector3f &dpdus,
                                            const Vector3f &dpdvs,
                                            const Normal3f &dndus,
                                            const Normal3f &dndvs,
                                            bool orientationIsAuthoritative) {
    // Compute _shading.n_ for _SurfaceInteraction_
    shading.n = Normalize((Normal3f)Cross(dpdus, dpdvs));
    if (shape && (shape->reverseOrientation ^ shape->transformSwapsHandedness))
        shading.n = -shading.n;
    if (orientationIsAuthoritative)
        n = Faceforward(n, shading.n);
    else
        shading.n = Faceforward(shading.n, n);

    // Initialize _shading_ partial derivative values
    shading.dpdu = dpdus;
    shading.dpdv = dpdvs;
    shading.dndu = dndus;
    shading.dndv = dndvs;
}

/*void SurfaceInteraction::ComputeScatteringFunctions(const RayDifferential &ray,
                                                    MemoryArena &arena,
                                                    bool allowMultipleLobes,
                                                    TransportMode mode) {
    ComputeDifferentials(ray);
    primitive->ComputeScatteringFunctions(this, arena, mode,
                                          allowMultipleLobes);
}*/

// Matrix4x4 Method Definitions
bool SolveLinearSystem2x2(const Float A[2][2], const Float B[2], Float *x0,
	Float *x1) {
	Float det = A[0][0] * A[1][1] - A[0][1] * A[1][0];
	if (std::abs(det) < 1e-10f) return false;
	*x0 = (A[1][1] * B[0] - A[0][1] * B[1]) / det;
	*x1 = (A[0][0] * B[1] - A[1][0] * B[0]) / det;
	if (std::isnan(*x0) || std::isnan(*x1)) return false;
	return true;
}

void SurfaceInteraction::ComputeDifferentials(
    const RayDifferential &ray) const {
    if (ray.hasDifferentials) {
        // Estimate screen space change in $\pt{}$ and $(u,v)$

        // Compute auxiliary intersection points with plane
        Float d = Dot(n, Vector3f(p.x, p.y, p.z));
        Float tx =
            -(Dot(n, Vector3f(ray.rxOrigin)) - d) / Dot(n, ray.rxDirection);
        if (std::isinf(tx) || std::isnan(tx)) goto fail;
        Point3f px = ray.rxOrigin + tx * ray.rxDirection;
        Float ty =
            -(Dot(n, Vector3f(ray.ryOrigin)) - d) / Dot(n, ray.ryDirection);
        if (std::isinf(ty) || std::isnan(ty)) goto fail;
        Point3f py = ray.ryOrigin + ty * ray.ryDirection;
        dpdx = px - p;
        dpdy = py - p;

        // Compute $(u,v)$ offsets at auxiliary points

        // Choose two dimensions to use for ray offset computation
        int dim[2];
        if (std::abs(n.x) > std::abs(n.y) && std::abs(n.x) > std::abs(n.z)) {
            dim[0] = 1;
            dim[1] = 2;
        } else if (std::abs(n.y) > std::abs(n.z)) {
            dim[0] = 0;
            dim[1] = 2;
        } else {
            dim[0] = 0;
            dim[1] = 1;
        }

        // Initialize _A_, _Bx_, and _By_ matrices for offset computation
        Float A[2][2] = {{dpdu[dim[0]], dpdv[dim[0]]},
                         {dpdu[dim[1]], dpdv[dim[1]]}};
        Float Bx[2] = {px[dim[0]] - p[dim[0]], px[dim[1]] - p[dim[1]]};
        Float By[2] = {py[dim[0]] - p[dim[0]], py[dim[1]] - p[dim[1]]};
        if (!SolveLinearSystem2x2(A, Bx, &dudx, &dvdx)) dudx = dvdx = 0;
        if (!SolveLinearSystem2x2(A, By, &dudy, &dvdy)) dudy = dvdy = 0;
    } else {
    fail:
        dudx = dvdx = 0;
        dudy = dvdy = 0;
        dpdx = dpdy = Vector3f(0, 0, 0);
    }
}

/*
Spectrum SurfaceInteraction::Le(const Vector3f &w) const {
    const AreaLight *area = primitive->GetAreaLight();
    return area ? area->L(*this, w) : Spectrum(0.f);
}*/

}  // namespace pbrt
