#version 330 core

out vec4 fragColor;

in vec2 imgPlanePos;

uniform mat4 viewMat;
uniform sampler3D voxelData;

struct Ray 
{
    vec3 origin;
    vec3 dir;
	float t;
} ray;

struct RayDifferential
{
	vec3 dpdx;
	vec3 dpdy;
	vec3 dddx;
	vec3 dddy;
} rayDiff;

vec4 voxel;
ivec3 resolution;

void initRayDiff(vec3 dir)
{
	vec3 right = viewMat[0].xyz;
	vec3 up = viewMat[1].xyz;

	rayDiff.dpdx = vec3(0.0);
	rayDiff.dpdy = vec3(0.0);
	rayDiff.dddx = (dot(dir, dir) * right - dot(dir, right) * dir) / pow(dot(dir, dir), 1.5);
	rayDiff.dddy = (dot(dir, dir) * up - dot(dir, up) * dir) / pow(dot(dir, dir), 1.5);
}

void transferRayDiff(vec3 normal)
{
	float dtdx, dtdy;

	dtdx = -dot(rayDiff.dpdx + ray.t * rayDiff.dddx, normal) / dot(ray.dir, normal);
	dtdy = -dot(rayDiff.dpdy + ray.t * rayDiff.dddy, normal) / dot(ray.dir, normal);

	rayDiff.dpdx = (rayDiff.dpdx + ray.t * rayDiff.dddx) + dtdx * ray.dir;
	rayDiff.dpdy = (rayDiff.dpdy + ray.t * rayDiff.dddy) + dtdy * ray.dir;

	//dddx = dddx;
	//dddy = dddy;
}

vec3 rayPosition()
{
	return ray.origin + ray.dir * ray.t;
}

vec4 lookupVoxel(vec3 pos)
{
	vec3 posBuffer = (pos + vec3(1.0)) * 0.5;
	return texture(voxelData, posBuffer);
}

void boxIntersect(out float tmin, out float tmax)
{
    float tymin, tymax, tzmin, tzmax;

	vec3 rayInvDir = 1.0 / ray.dir;

	int raySign[3] = int[]
	(   
			(rayInvDir.x < 0) ? 1 : 0,
            (rayInvDir.y < 0) ? 1 : 0,
            (rayInvDir.z < 0) ? 1 : 0
    );

	vec3 aabb[2] = vec3[](vec3(-1.0), vec3(1.0));

    tmin = (aabb[raySign[0]].x - ray.origin.x) * rayInvDir.x;
    tmax = (aabb[1-raySign[0]].x - ray.origin.x) * rayInvDir.x;
    tymin = (aabb[raySign[1]].y - ray.origin.y) * rayInvDir.y;
    tymax = (aabb[1-raySign[1]].y - ray.origin.y) * rayInvDir.y;
    tzmin = (aabb[raySign[2]].z - ray.origin.z) * rayInvDir.z;
    tzmax = (aabb[1-raySign[2]].z - ray.origin.z) * rayInvDir.z;
    tmin = max(max(tmin, tymin), tzmin);
    tmax = min(min(tmax, tymax), tzmax);
    // post condition:
    // if tmin > tmax (in the code above this is represented by a return value of INFINITY)
    //     no intersection
    // else
    //     front intersection point = ray.origin + ray.direction * tmin (normally only this point matters)
    //     back intersection point  = ray.origin + ray.direction * tmax
}

bool raymarch(float tmin, float tmax)
{
	ray.t = tmin;

	int maxRaySteps = 1000;
	float sdfEps = 0.001f;

	for (int i = 0; i < maxRaySteps; i++)
	{
		voxel = lookupVoxel(rayPosition());

		if (voxel.a <= sdfEps)
		{
			return true;
		}
		else
		{
			ray.t += voxel.a;// * scale;

			if (ray.t > tmax)
				break;
		}
	}

	return false;
}

void main()
{
	vec3 origin = viewMat[3].xyz;
	vec3 dir = (viewMat * vec4(vec3(imgPlanePos, 1.0), 0.0)).xyz;

	initRayDiff(dir);

	dir = normalize(dir);

	ray = Ray(origin, dir, 0.0);

	float tmin, tmax;

	boxIntersect(tmin, tmax);

	resolution = textureSize(voxelData, 0);

	if (tmin > tmax)
		discard;

	if (!raymarch(tmin, tmax))
		discard;

	transferRayDiff(voxel.xyz);

	fragColor = vec4(abs(rayDiff.dpdx), 1.0);
	//fragColor = vec4(vec3(texelFetch(voxelData, resolution / 4, 0).a), 1.0);
	//fragColor = vec4(voxel.xyz, 0.0);
	//fragColor = vec4(vec3((lookupVoxel(rayPosition())).a), 1.0);
	//fragColor = vec4(rayPosition(), 1.0);
}