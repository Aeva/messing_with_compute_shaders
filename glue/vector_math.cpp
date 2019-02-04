#include <math.h>
#if DEBUG_BUILD
#include <iostream>
#include <iomanip>
#include <string.h>
#endif

#include "gl_boilerplate.h"
#include "vector_math.h"


const double ToRadians = M_PI / 180.0;


inline void CrossProduct(GLfloat Out[3], const GLfloat LHS[3], const GLfloat RHS[3])
{
	Out[0] = LHS[1] * RHS[2] - LHS[2] * RHS[1];
	Out[1] = LHS[2] * RHS[0] - LHS[0] * RHS[2];
	Out[2] = LHS[0] * RHS[1] - LHS[1] * RHS[0];
}


template<int Size>
inline GLfloat DotProduct(const GLfloat LHS[Size], const GLfloat RHS[Size])
{
	GLfloat DotProduct = 0;
	for (int i=0; i<Size; ++i)
	{
		DotProduct += LHS[i] * RHS[i];
	}
	return DotProduct;
}


template<int Size>
inline void Normalize(GLfloat Vector[Size])
{
	const GLfloat Length = sqrt(DotProduct<Size>(Vector, Vector));
	const GLfloat LengthRcp = 1 / Length;
	for (int i=0; i<Size; ++i)
	{
		Vector[i] *= LengthRcp;
	}
}


#define FILL_IDENTITY(Out) \
	Out[0] = 1.0; \
	Out[1] = 0.0; \
	Out[2] = 0.0; \
	Out[3] = 0.0; \
	Out[4] = 0.0; \
	Out[5] = 1.0; \
	Out[6] = 0.0; \
	Out[7] = 0.0; \
	Out[8] = 0.0; \
	Out[9] = 0.0; \
	Out[10] = 1.0; \
	Out[11] = 0.0; \
	Out[12] = 0.0; \
	Out[13] = 0.0; \
	Out[14] = 0.0; \
	Out[15] = 1.0;


void IdentityMatrix(GLfloat Out[16])
{
	FILL_IDENTITY(Out);
}


void MultiplyMatrices(GLfloat Out[16], GLfloat LHS[16], GLfloat RHS[16])
{
	for (int y=0; y<4; ++y)
	{
		for (int x=0; x<4; ++x)
    	{
            Out[x * 4 + y] = \
		    LHS[y +  0] * RHS[x * 4 + 0] + \
		    LHS[y +  4] * RHS[x * 4 + 1] + \
		    LHS[y +  8] * RHS[x * 4 + 2] + \
		    LHS[y + 12] * RHS[x * 4 + 3];
        }
	}
}


void TransposeMatrix(GLfloat Out[16], GLfloat Original[16])
{
	for (int y=0; y<4; ++y)
	{
		for (int x=0; x<4; ++x)
		{
			const int IndexIn = x * 4 + y;
			const int IndexOut = y * 4 + x;
			Out[IndexOut] = Original[IndexIn];
		}
	}
}


void TranslationMatrix(GLfloat Out[16], GLfloat PositionX, GLfloat PositionY, GLfloat PositionZ)
{
	FILL_IDENTITY(Out);
	Out[12] = PositionX;
	Out[13] = PositionY;
	Out[14] = PositionZ;
}


void XRotationMatrix(GLfloat Out[16], GLfloat RotateX)
{
	const GLfloat SinX = sin(RotateX * ToRadians);
	const GLfloat CosX = cos(RotateX * ToRadians);
	FILL_IDENTITY(Out);
	Out[5] = CosX;
	Out[6] = -SinX;
	Out[9] = SinX;
	Out[10] = CosX;
}


void YRotationMatrix(GLfloat Out[16], GLfloat RotateY)
{
	const GLfloat SinY = sin(RotateY * ToRadians);
	const GLfloat CosY = cos(RotateY * ToRadians);
	FILL_IDENTITY(Out);
	Out[0] = CosY;
	Out[2] = SinY;
	Out[8] = -SinY;
	Out[10] = CosY;
}


void ZRotationMatrix(GLfloat Out[16], GLfloat RotateZ)
{
	const GLfloat SinZ = sin(RotateZ * ToRadians);
	const GLfloat CosZ = cos(RotateZ * ToRadians);
	FILL_IDENTITY(Out);
	Out[0] = CosZ;
	Out[1] = SinZ;
	Out[4] = -SinZ;
	Out[5] = CosZ;
}


void ScaleMatrix(GLfloat Out[16], GLfloat ScaleX, GLfloat ScaleY, GLfloat ScaleZ)
{
	FILL_IDENTITY(Out);
	Out[0] = ScaleX;
	Out[5] = ScaleY;
	Out[10] = ScaleZ;
}


void ViewMatrix(GLfloat Out[16], const GLfloat Origin[3], const GLfloat Focus[3], const GLfloat Up[3])
{
	GLfloat AxisZ[3] = {
		Origin[0] - Focus[0],
		Origin[1] - Focus[1],
		Origin[2] - Focus[2]	
	};
	Normalize<3>(AxisZ);
	
	GLfloat AxisX[3];
	CrossProduct(AxisX, Up, AxisZ);
	Normalize<3>(AxisX);

	GLfloat AxisY[3];
	CrossProduct(AxisY, AxisZ, AxisX);
	Normalize<3>(AxisY);

	for (int i=0; i<3; ++i)
	{
		const int Offset = i * 4;
		Out[Offset + 0] = AxisX[i];
		Out[Offset + 1] = AxisY[i];
		Out[Offset + 2] = AxisZ[i];
		Out[Offset + 3] = 0;
	}

	Out[12] = DotProduct<3>(AxisX, Origin) * -1;
	Out[13] = DotProduct<3>(AxisY, Origin) * -1;
	Out[14] = DotProduct<3>(AxisZ, Origin) * -1;
	Out[15] = 1;
}


void ViewMatrix(GLfloat Out[16], const GLfloat Origin[3], const GLfloat Focus[3])
{
	const GLfloat Up[3] = { 0, 0, 1 };
	ViewMatrix(Out, Origin, Focus, Up);
}


void OrthographicMatrix(GLfloat Out[16])
{
}


void PerspectiveMatrix(GLfloat Out[16])
{
	const GLfloat FieldOfView = 1.0 / (45.0 * 0.0174533 * 0.5); // Radians
	const GLfloat AspectRatio = (GLfloat)ScreenWidth / (GLfloat)ScreenHeight;
	const GLfloat NearPlane = 0.1; // Is this a reasonable default?
	const GLfloat FarPlane = 1000;
	const GLfloat DepthRange = FarPlane - NearPlane;

#if DEBUG_BUILD
	std::cout << "width: " << ScreenWidth << "\n";
	std::cout << "height: " << ScreenHeight << "\n";
	std::cout << "fov: " << FieldOfView << "\n";
	std::cout << "aspect: " << AspectRatio << "\n";
	std::cout << "near: " << NearPlane << "\n";
	std::cout << "far: " << FarPlane << "\n";
#endif

	const GLfloat ScaleX = (1 / AspectRatio) * FieldOfView;
	const GLfloat ScaleY = FieldOfView;
	const GLfloat ScaleZ = FarPlane / DepthRange;
	const GLfloat OffsetZ = -(FarPlane + NearPlane) / DepthRange;

	for (int i=0; i<16; ++i)
	{
		Out[i] = 0;	
	}
	Out[0] = ScaleX;
	Out[5] = ScaleY;
	Out[10] = ScaleZ;
	Out[11] = 1.0;
	Out[14] = OffsetZ;
}


void PrintMatrix(GLfloat Matrix[16], const char* MatrixName)
{
#if DEBUG_BUILD
	const char* Prefix = "────┤ ";
	const char* Suffix = " ├";
	const int NameLen = strlen(MatrixName);
	const int PrefixLen = 6; // hardcoding these because lol unicode
	const int SuffixLen = 2;
	const int TableWidth = 55;
	const int Remainder = TableWidth - PrefixLen - NameLen - SuffixLen;
	std::cout << " ╭" << Prefix << MatrixName << Suffix;
	for (int i=0; i<Remainder; ++i)
	{
		std::cout << "─";
	}
	std::cout << "╮\n";

#define COL(x) std::right << std::setw(11) << std::setfill(' ') << Matrix[x]
	std::cout << " │ " << COL(0) << "   " << COL(4) << "   " << COL(8) << "   " << COL(12) << " │\n"
	          << " │ " << COL(1) << "   " << COL(5) << "   " << COL(9) << "   " << COL(13) << " │\n"
	          << " │ " << COL(2) << "   " << COL(6) << "   " << COL(10) << "   " << COL(14) << " │\n"
	          << " │ " << COL(3) << "   " << COL(7) << "   " << COL(11) << "   " << COL(15) << " │\n"
	          << " ╰───────────────────────────────────────────────────────╯\n";
#undef COL
#endif
}

#undef FILL_IDENTITY

