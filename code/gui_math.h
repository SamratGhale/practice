/* date = September 12th 2022 1:24 am */

#ifndef GUI_MATH_H
#define GUI_MATH_H

union v2{
	struct{
		f32 X, Y;
	};
	f32 E[2];
};

inline f32 AbsoluteValue(f32 Real32){
	f32 Result = (f32)fabs(Real32);
	return (Result);
}

inline v2 operator*(f32 A, v2 B){
	v2 Result;
	Result.X = A*B.X;
	Result.Y = A*B.Y;
	return Result;
}

inline v2 operator*(v2 B, f32 A){
	v2 Result;
	Result.X = A*B.X;
	Result.Y = A*B.Y;
	return Result;
}

inline v2 operator*=(v2 &A, f32 B){
	A = B * A;
	return (A);
}

v2 operator+(v2 A, v2 B){
	v2 Result;
	Result.X = A.X + B.X;
	Result.Y = A.Y + B.Y;
	return Result;
}

inline v2 operator+=(v2 &A, v2 B){
	A =  B + A;
	return (A);
}



v2 operator-(v2 A){
	v2 Result;
	Result.X = -A.X;
	Result.Y = -A.Y;
	return(Result);
}


v2 operator-(v2 A, v2 B){
	v2 Result;
	Result.X = A.X - B.X;
	Result.Y = A.Y - B.Y;
	return Result;
}

inline v2 operator-=(v2 &A, v2 B){
	A = B - A; 
	return (A);
}
inline f32 Square(f32 A){
	f32 Result = A * A;
	return (Result);
}
inline f32 Inner(v2 A, v2 B){
	f32 Result = A.X * B.X + A.Y * B.Y;
	return Result;
}

inline f32 LengthSq(v2 A){
	f32 Result = Inner(A,A);	
	return Result;
}

#endif //GUI_MATH_H
