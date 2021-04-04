#version 330 core

in vec2 planePoint;

out vec4 fragColor;

uniform sampler3D voxelData;
//uniform mat4 modelview;
//uniform mat4 normalMat;

uniform vec3 camPos;

/*void useUniforms()
{
	if (modelview[3][3] == 0.0)
	{
		fragColor = vec4(1.0, 0.0, 0.0, 1.0) + normalMat * vec4(1.0);
	}
}*/

/*vec3 getNormal(vec3 pos, float value, float rayStep)
{
    float L = rayStep * 100;
    float E = texture(voxelData, pos + vec3(L,0,0)).a;
    float N = texture(voxelData, pos + vec3(0,L,0)).a;
    float U = texture(voxelData, pos + vec3(0,0,L)).a;
    
	vec3 normal = vec3(E - value, N - value, U - value);

	return normalize((normalMat * vec4(normal, 0.0)).xyz);
}*/

void main()
{
	float z = 0.5f;
	//color = vec3(1,0,0);
	//fragColor = vec4(texture(voxelData, vec3(0.5f, 0.5f, z)).rgb, 1.0);

	vec3 dir = normalize(vec3(planePoint, 0.0) - camPos);

	vec4 value = vec4(0.0);

	float rayStep = 0.003;

	float threshold = 0.45;
	float maxDist = 5.0;

	vec4 color = vec4(0.0);

	vec3 rayStart = (vec4(camPos, 1.0)).xyz;//(modelview * vec4(planePoint, 1.0)).xyz;
	vec3 rayDir = (vec4(dir, 0.0)).xyz;

	//vec3 rayStart = (modelview * vec4(camPos, 1.0)).xyz;//(modelview * vec4(planePoint, 1.0)).xyz;
	//vec3 rayDir = (modelview * vec4(dir, 0.0)).xyz;

	for (float dist = 0.0; dist <= maxDist; dist += rayStep)
	{
		vec3 pos = rayStart + dist * rayDir;

		if (pos.x >= 0.0f && pos.x <= 1.0
		 && pos.y >= 0.0f && pos.y <= 1.0
		 && pos.z >= 0.0f && pos.z <= 1.0)
		{
			value = texture(voxelData, pos);

			if (value.a > threshold)
			{
				color = value;
				//color = vec4(getNormal(pos, value.a, rayStep), 1.0);
				break;
			}
		}
	}

	if (color.a <= threshold)
	{
		color = vec4(0.0, 0.0, 0.0, 1.0);
		//discard;
	}

	fragColor = vec4(color.rgb, 1.0);

	//useUniforms();
}