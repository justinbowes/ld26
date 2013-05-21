// -- Lib

float r_0_6700(float k) {
	return 1.0;
}

float r_6700_10000(float k) {
	return 6380.0 / k;
}

float r_10000(float k) {
	return 0.313 * k / (k - 5100.0);
}

float g_0_1100(float k) {
	return 0.000005 * k;
}

float g_1100_4000(float k) {
	return 0.000204 * k - 0.164;
}

float g_4000_6700(float k) {
	return -2956.0 / k + 1.39;
}

float g_6700_10000(float k) {
	return 5030.0 / k + 0.196;
}

float g_10000(float k) {
	return 0.436 * k / (k - 3820.0) - 0.0006;
}

float b_0_2086(float k) {
	return 0.0;
}

float b_2086_4000(float k) {
	return 0.00018 * k - 0.378;
}

float b_4000_6700(float k) {
	return 0.000245 * k - 0.64;
}

float b_6700(float k) {
	return 1.0;
}

float i_0_15000(float k) {
    k = max(k, 100.0);
	return 179.3 * pow(k, (-3509.0 / k + 0.001));
}

float i_15000(float k) {
	return 0.00313 * k - 28.04;
}

vec4 blackbody_rgbi(float k) {
	if (k <= 1100.0) return vec4(r_0_6700(k), g_0_1100(k), b_0_2086(k), i_0_15000(k));
	if (k <= 2086.0) return vec4(r_0_6700(k), g_1100_4000(k), b_0_2086(k), i_0_15000(k));
	if (k <= 4000.0) return vec4(r_0_6700(k), g_1100_4000(k), b_2086_4000(k), i_0_15000(k));
	if (k <= 6700.0) return vec4(r_0_6700(k), g_4000_6700(k), b_4000_6700(k), i_0_15000(k));
	if (k <= 10000.0) return vec4(r_6700_10000(k), g_6700_10000(k), b_6700(k), i_0_15000(k));
	if (k <= 15000.0) return vec4(r_10000(k), g_10000(k), b_6700(k), i_0_15000(k));
	return vec4(r_10000(k), g_10000(k), b_6700(k), i_15000(k));
}
