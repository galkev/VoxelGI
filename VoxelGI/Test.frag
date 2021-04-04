#version 330 core

out vec4 fragColor;

uniform vec4 p_a, p_b, p_c;
uniform vec4 n_a, n_b, n_c;

vec4 getNormalDifferential()
{
	vec4 bar_a, bar_b, bar_c;
	vec4 l_a, l_b, l_c;

	vec4 dN = vec4(1.0, 0.0, 0.0, 1.0);

	return dN;
}

void useUniforms()
{
	if ((p_a + p_b + p_c + n_a + n_b + n_c).w == -1.0f)
		discard;
}

void main()
{
	fragColor = getNormalDifferential();

	useUniforms();
}