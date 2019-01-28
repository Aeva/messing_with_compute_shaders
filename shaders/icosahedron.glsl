--------------------------------------------------------------------------------

const float Phi = (1.0 + sqrt(5.0)) * 0.5;
const vec3 UnitScale = vec3(1.0/sqrt(pow((Phi + 1.0) / 3.0, 2.0) * 3.0));

#define LR(PhiPart, OnePart) vec3(PhiPart * Phi, 0, OnePart) * UnitScale
#define TB(PhiPart, OnePart) vec3(OnePart, PhiPart * Phi, 0) * UnitScale
#define NF(PhiPart, OnePart) vec3(0, OnePart, PhiPart * Phi) * UnitScale

#define RP LR(1, 1)
#define RM LR(1, -1)
#define LP LR(-1, 1)
#define LM LR(-1, -1)

#define TP TB(1, 1)
#define TM TB(1, -1)
#define BP TB(-1, 1)
#define BM TB(-1, -1)

#define NP NF(1, 1)
#define NM NF(1, -1)
#define FP NF(-1, 1)
#define FM NF(-1, -1)

#define FACE(V1, V2, V3) V1, V2, V3
#define PAIR(V1, V2, V3, V4) FACE(V1, V2, V3), FACE(V1, V3, V4)
#define QUAD(V1, V2, V3, V4, V5, V6) PAIR(V1, V2, V3, V4), PAIR(V5, V1, V4, V6)

const vec3 Icosahedron[60] = {
	QUAD(NP, RP, TP, TM, LP, LM),
    QUAD(NM, RP, NP, LP, BM, LM),
	QUAD(BP, RP, NM, BM, FM, LM),
	QUAD(RM, RP, BP, FM, FP, LM),
	QUAD(TP, RP, RM, FP, TM, LM)
};

#undef LR
#undef TB
#undef NF
#undef RP
#undef RM
#undef LP
#undef LM
#undef TP
#undef TM
#undef BP
#undef BM
#undef NP
#undef NM
#undef FP
#undef FM
#undef FACE
#undef PAIR
#undef QUAD
