namespace Math {
	const f32 Pi32 = 3.14159265359f;
	const f64 Pi = 3.14159265358979323846;
	
	f32 toRadians(f32 degrees) { return (( degrees * Math::Pi32 ) / 180.0f); }
	f32 toDegrees(f32 radians) { return ((radians * 180.0f) / Math::Pi32); }
		
	f32 sign(f32 a) { return (a > 0) ? 1 : (a < 0) ? -1 : 0; }
	f32 cos(f32 radians) { return cosf(radians); }
	f32 sin(f32 radians) { return sinf(radians); }
	f32 tan(f32 radians) { return tanf(radians); }
	f32 acos(f32 radians) { return acosf(radians); }
	f32 cot(f32 radians) { return 1.0f / tan(radians); }
	f32 atan2(f32 x, f32 y) { return atan2f(x, y); }
	f32 wrap(f32 a, f32 min, f32 max) { if(a < min) return max; else if (a > max) return min; return a; }
	u32 wrap(u32 a, u32 min, u32 max) {	if(a < min) return max; else if (a > max) return min; return a;	}
	s32 wrap(s32 a, s32 min, s32 max) { if(a < min) return max; else if (a > max) return min; return a; }
	f32 clamp(f32 a, f32 min, f32 max) { if(a < min) return min; else if (a > max) return max; return a; }
	f32 squareRoot(f32 a) { return sqrtf(a); }
	f32 q_inv_squareRoot( f32 number )
	{
		long i;
		f32 x2, y;
		const f32 threehalfs = 1.5F;

		x2 = number * 0.5F;
		y  = number;
		i  = * ( long * ) &y;                       // evil floating point bit level hacking
		i  = 0x5f3759df - ( i >> 1 );               // what the fuck? 
		y  = * ( float * ) &i;
		y  = y * ( threehalfs - ( x2 * y * y ) );   // 1st iteration
	//	y  = y * ( threehalfs - ( x2 * y * y ) );   // 2nd iteration, this can be removed

		return y;
	}
	
	s32 ceilToInt(f32 a) { return (s32)ceilf(a); }
	s32 floorToInt(f32 a) { return (s32)floorf(a); }
	f32 pow(f32 base, f32 exp) { return powf(base, exp); }
	f32 abs(f32 a) { return fabs(a); }
	f32 rmin(f32 a, f32 b) {return fmin(a, b);}
	f32 rmax(f32 a, f32 b) {return fmax(a, b);}
	f32 randFloat() { int r = rand() % 100000; return r * 0.00001f; } // NOTE(nathan): between 0 and 1
	f32 lerp(f32 s, f32 e, f32 t) { return s+(e-s)*t; }
	f32 blerp(f32 c00, f32 c01, f32 c10, f32 c11, f32 tx, f32 ty) { return lerp(lerp(c00, c10, tx), lerp(c01, c11, tx), ty); }
	
};

#define VEC2TEMPLATE(name, type) 																							\
struct name {																												\
	name(type x, type y) { this->x = x; this->y = y; }																		\
	name() { this->x = 0.0f; this->y = 0.0f; }																				\
	name(type x) { this->x = x; this->y = x; }																				\
																															\
	union {																													\
		struct { type x, y; };																								\
		struct { type u, v; };																								\
		struct { type w, h; };																								\
		type xy[2];																											\
	};																														\
																															\
	name operator-() { return name(-x, -y); };																				\
																															\
	name operator*(type i) { return name(x * i, y * i);	}																	\
	name operator/(type i) { return name(x / i, y / i);	}																	\
																															\
	void operator+=(const name &i) { x += i.x; y += i.y; }																	\
	void operator-=(const name &i) { x -= i.x; y -= i.y; }																	\
	void operator*=(const name &i) { x *= i.x; y *= i.y; }																	\
	void operator/=(const name &i) { x /= i.x; y /= i.y; }																	\
																															\
	void operator*=(type i) { x *= i; y *= i; }																				\
	void operator/=(type i) { x /= i; y /= i; }																				\
																															\
	static type length(const name &x) { type result = Math::squareRoot((x.x * x.x) + (x.y * x.y)); return result;	}		\
	static type lengthSquared(const name &x) { type result = (x.x * x.x) + (x.y * x.y); return result;	}		\
	static name abs(const name &x) { return name(Math::abs(x.x), Math::abs(x.y));	}										\
	static name sign(const name &x) { return name(Math::sign(x.x), Math::sign(x.y));	}									\
	static name rPerp(const name &x) { return name(x.y, -x.x); }															\
	static name lPerp(const name &x) { return name(-x.y, x.x); }															\
	static name normalize(const name &x) { type len = length(x); return name(x.x / len, x.y / len); }						\
	static type dot(const name &a, const name &b) { return (a.x * b.x) + (a.y * b.y); }										\
	static name project(name a, name b) { return b * (dot(a, b) / dot(b, b));}												\
	static name right() {return name(1, 0); }																				\
	static name up() {return name(0, 1); }																				\
	static type angle(name a, name b) {return Math::atan2(a.x, a.y) - Math::atan2(b.x, b.y); }														\
	static name rotate(name a, type radians) { type theta = angle(up(), a); theta += radians; return name(Math::sin(theta), Math::cos(theta)); } \
};																															\
																															\
name operator+(const name &a, const name &b) { return name(a.x + b.x, a.y + b.y); }											\
name operator-(const name &a, const name &b) { return name(a.x - b.x, a.y - b.y);	}										\
name operator*(const name &a, const name &b) { return name(a.x * b.x, a.y * b.y); }											\
name operator/(const name &a, const name &b) { return name(a.x / b.x, a.y / b.y);	}										\
\


VEC2TEMPLATE(Vec2, f32);
VEC2TEMPLATE(Vec2i, s32);

Vec2 v2iToV2(Vec2i a) { return Vec2((f32)a.x, (f32)a.y);}

struct Vec3 {
	Vec3(f32 x, f32 y, f32 z) { this->x = x; this->y = y; this->z = z;}
	Vec3() { this->x = 0.0f; this->y = 0.0f; this->z = 0.0f; }
	Vec3(Vec2 xy, f32 z) { this->x = xy.x; this->y = xy.y; this->z = z;}
	Vec3(f32 x) { this->x = x; this->y = x; this->z = x;}

	union {
		struct { f32 x, y, z; };
		struct { f32 u, v, w; };
		struct { f32 r, g, b; };
		struct { Vec2 xy; f32 _z; };
		struct { f32 _x; Vec2 yz; };
		struct { Vec2 uv; f32 _w; };
		f32 xyz[3];
	};

	Vec3 operator-() { return Vec3(-x, -y, -z); };
	bool operator==(Vec3 a) {return x == a.x && y == a.y && z == a.z; } 
	Vec3 operator+(const Vec3 &i) { return Vec3(x + i.x, y + i.y, z + i.z); }
	Vec3 operator-(const Vec3 &i) { return Vec3(x - i.x, y - i.y, z - i.z); }
	Vec3 operator*(const Vec3 &i) { return Vec3(x * i.x, y * i.y, z * i.z); }
	Vec3 operator/(const Vec3 &i) { return Vec3(x / i.x, y / i.y, z / i.z); }

	Vec3 operator*(f32 i) { return Vec3(x * i, y * i, z * i); }
	Vec3 operator/(f32 i) { return Vec3(x / i, y / i, z / i); }

	void operator+=(const Vec3 &i) { x += i.x; y += i.y; z += i.z; }
	void operator-=(const Vec3 &i) { x -= i.x; y -= i.y; z -= i.z; }
	void operator*=(const Vec3 &i) { x *= i.x; y *= i.y; z *= i.z; }
	void operator/=(const Vec3 &i) { x /= i.x; y /= i.y; z /= i.z; }

	void operator*=(f32 i) { x *= i; y *= i; z *= i;}
	void operator/=(f32 i) { x /= i; y /= i; z /= i;}

	static f32 dot(const Vec3 &a, const Vec3 &b) { return (a.x * b.x) + (a.y * b.y) + (a.z * b.z); }								
	static Vec3 cross(const Vec3 &a, const Vec3 &b) { return Vec3(a.y*b.z - a.z*b.y, a.z*b.x - a.x*b.z, a.x*b.y - a.y*b.x); }						
	static Vec3 normalize(Vec3 a) { return a / Vec3::length(a); }						
	static f32 lengthSquared(const Vec3 &x) { f32 result = (x.x * x.x) + (x.y * x.y) + (x.z * x.z); return result; }
	static f32 length(const Vec3 &x) { f32 result = Math::squareRoot(lengthSquared(x)); return result; }
	static Vec3 project(Vec3 a, Vec3 b) { return b * (dot(a, b) / dot(b, b));}
	static f32 distanceToPlane(Vec3 point, Vec3 plane_origin, Vec3 plane_normal) { return dot(plane_normal, (point - plane_origin)); }
	static Vec3 lerp(Vec3 v0, Vec3 v1, f32 t) {
		if(v0.x > v1.x) Swap(v0.x, v1.x);
		if(v0.y > v1.y) Swap(v0.y, v1.y);
		if(v0.z > v1.z) Swap(v0.z, v1.z);
		
		Vec3 result = Vec3();
		result.x = Math::lerp(v0.x, v1.x, t);
		result.y = Math::lerp(v0.y, v1.y, t);
		result.z = Math::lerp(v0.z, v1.z, t);
		return result;
	}
	static Vec3 abs(Vec3 a) { return Vec3(Math::abs(a.x), Math::abs(a.y), Math::abs(a.z)); }
	static int maxIndex(Vec3 a) {
		f32 val = 0;
		int index = -1;
		for(int i = 0; i < 3; i++) {
			if(a.xyz[i] > val)  {
				index = i;
				val = a.xyz[i];
			}
		}
		return index;
	}
	
	static Vec3 rmax(Vec3 a, Vec3 b) {
		Vec3 result = Vec3();
		result.x = Math::rmax(a.x, b.x);
		result.y = Math::rmax(a.y, b.y);
		result.z = Math::rmax(a.z, b.z);
		return result;
	}
	
	static Vec3 rmin(Vec3 a, Vec3 b) {
		Vec3 result = Vec3();
		result.x = Math::rmin(a.x, b.x);
		result.y = Math::rmin(a.y, b.y);
		result.z = Math::rmin(a.z, b.z);
		return result;
	}
};

Vec3 operator*(f32 i, Vec3 a) {
	return a * i;
}


Vec3 operator/(f32 i, Vec3 a) {
	return a / i;
}


struct Vec4 {
	Vec4(f32 x, f32 y, f32 z, f32 w) { this->x = x; this->y = y; this->z = z; this->w = w;}
	Vec4() { this->x = 0.0f; this->y = 0.0f; this->z = 0.0f; this->w = 0.0f; }
	Vec4(Vec2 xy, Vec2 zw) { this->xy = xy; this->zw = zw;}
	Vec4(Vec2 xy, f32 z, f32 w) { this->xy = xy; this->z = z; this->w = w;}
	Vec4(Vec3 xyz, f32 w) { this->xyz = xyz; this->w = w;}
	Vec4(f32 x) { this->x = x; this->y = x; this->z = x; this->w = x;}

	union {
		struct { f32 x, y, z, w;};
		struct { f32 r, g, b, a;};
		struct { Vec2 xy; Vec2 zw; };
		struct { Vec3 xyz; f32 _w; };
		f32 xyzw[4];
	};

	Vec4 operator-() { return Vec4(-x, -y, -z, -w); };

	Vec4 operator+(const Vec4 &i) { return Vec4(x + i.x, y + i.y, z + i.z, w + i.w); }
	Vec4 operator-(const Vec4 &i) { return Vec4(x - i.x, y - i.y, z - i.z, w - i.w); }
	Vec4 operator*(const Vec4 &i) { return Vec4(x * i.x, y * i.y, z * i.z, w * i.w); }
	Vec4 operator/(const Vec4 &i) { return Vec4(x / i.x, y / i.y, z / i.z, w / i.w); }

	Vec4 operator*(f32 i) { return Vec4(x * i, y * i, z * i, w * i); }
	Vec4 operator/(f32 i) { return Vec4(x / i, y / i, z / i, w / i); }

	void operator+=(const Vec4 &i) { x += i.x; y += i.y; z += i.z; w += i.z; }
	void operator-=(const Vec4 &i) { x -= i.x; y -= i.y; z -= i.z; w -= i.z; }
	void operator*=(const Vec4 &i) { x *= i.x; y *= i.y; z *= i.z; w *= i.z; }
	void operator/=(const Vec4 &i) { x /= i.x; y /= i.y; z /= i.z; w /= i.z; }

	void operator*=(f32 i) { x *= i; y *= i; z *= i; w *= i; }
	void operator/=(f32 i) { x /= i; y /= i; z /= i; w /= i; }

	static f32 length(const Vec4 &x) { f32 result = Math::squareRoot((x.x * x.x) + (x.y * x.y) + (x.z * x.z) + (x.w * x.w)); return result; }
	
	static Vec4 normalize(Vec4 a) { return a / Vec4::length(a); }						
};

struct Mat4 {
	union { f32 data2d[4][4]; f32 data1d[16]; };
	
	Mat4() {
		data2d[0][0] = 1.0f;	data2d[0][1] = 0.0f;	data2d[0][2] = 0.0f;	data2d[0][3] = 0.0f;
		data2d[1][0] = 0.0f;	data2d[1][1] = 1.0f;	data2d[1][2] = 0.0f;	data2d[1][3] = 0.0f;
		data2d[2][0] = 0.0f;	data2d[2][1] = 0.0f;	data2d[2][2] = 1.0f;	data2d[2][3] = 0.0f;
		data2d[3][0] = 0.0f;	data2d[3][1] = 0.0f;	data2d[3][2] = 0.0f;	data2d[3][3] = 1.0f;	
	}
	
	Mat4 operator*(const Mat4 &other) {
		Mat4 result = Mat4();
		for (int i = 0; i < 4; i++) {
			for (int j = 0; j < 4; j++) {
				result.data2d[i][j] = data2d[i][0] * other.data2d[0][j] +
									  data2d[i][1] * other.data2d[1][j] +
									  data2d[i][2] * other.data2d[2][j] +
									  data2d[i][3] * other.data2d[3][j];
			}
		}
		return result;
	}
	
	/*
	
	Vector result;
    for ( int i = 0; i < 4; ++i )
       result[i] = v[0] * m[0][i] + v[1] * m[1][i] + v[2] + m[2][i] + v[3] * m[3][i];
    result[0] = result[0]/result[3];
    result[1] = result[1]/result[3];
    result[2] = result[2]/result[3];
    return result;
    
	*/
	
	static Vec4 transform(const Mat4 &m, const Vec4 &other) {
		Vec4 result = Vec4();
		for(int i = 0; i < 4; i++) {
			f32 r = 0.0f;
			for(int j = 0; j < 4; j++) {
				r += other.xyzw[j] * m.data2d[i][j];
			}
			result.xyzw[i] = r;
		}
		
		// result.x /= result.w;
		// result.y /= result.w;
		// result.z /= result.w;
		
		return result;
	}
	
	static Mat4 ortho(f32 width, f32 height, f32 near_plane, f32 far_plane) {
		Mat4 result = Mat4();
		result.data2d[0][0] = 2.0f / width;
		result.data2d[1][1] = 2.0f / height;
		result.data2d[2][2] = 2.0f / (far_plane - near_plane);
		// result.data2d[2][3] = ((far_plane + near_plane) / (far_plane - near_plane));
		return result;
	}
	
	static Mat4 perspective(f32 fov, f32 aspect_ratio, f32 z_near, f32 z_far)
	{
		
		Mat4 result = Mat4();
	
		f32 tan_half_fov = Math::tan(Math::toRadians(fov * 0.5f));
		
		// scaling
		
		result.data2d[0][0] = (1.0f / tan_half_fov) / aspect_ratio;
		result.data2d[1][1] = 1.0f / tan_half_fov;
		result.data2d[2][2] = z_far / (z_far - z_near);
		
		// translation
		
		result.data2d[3][2] = 1.0f;
		
		// z part
		result.data2d[2][3] = -z_near * z_far / (z_far - z_near);
		
		
		result.data2d[3][3] = 0.0f;
		return result;
	}
	
	static Mat4 invPerspective(const Mat4 &perspective) {
		Mat4 result = Mat4();
		
		f32 d = perspective.data2d[2][3];
		f32 e = perspective.data2d[3][2];
		f32 c = perspective.data2d[2][2];
		
		result.data2d[0][0] = 1.0f / perspective.data2d[0][0];
		result.data2d[1][1] = 1.0f / perspective.data2d[1][1];
		result.data2d[2][2] = 0.0f;
		
		result.data2d[2][3] = 1.0f / d;
		result.data2d[3][3] = -(c / (d * e));
		result.data2d[3][2] = 1.0f / e;
		return result;
		// return invMat(perspective);
	}
	
	static Mat4 translate(Vec3 a) {
		Mat4 result = Mat4();
		result.data2d[0][3] = a.x;
		result.data2d[1][3] = a.y;
		result.data2d[2][3] = a.z;
		return result;
	}
	static Mat4 rotateZ(f32 rads) {
		Mat4 result = Mat4();
		result.data2d[0][0] = Math::cos(rads);
		result.data2d[1][1] = Math::cos(rads);
		result.data2d[0][1] = -Math::sin(rads);
		result.data2d[1][0] = Math::sin(rads);
		return result;
	}
	
	static Mat4 rotateY(f32 rads)
	{
		Mat4 result = Mat4();
		result.data2d[0][0] = Math::cos(rads);	
		result.data2d[0][2] = Math::sin(rads);	
		result.data2d[2][0] = -Math::sin(rads);	
		result.data2d[2][2] = Math::cos(rads);	
		return result;
	}

	static Mat4 rotateX(f32 rads)
	{
		Mat4 result = Mat4();
		result.data2d[1][1] = Math::cos(rads);
		result.data2d[1][2] = -Math::sin(rads);
		result.data2d[2][1] = Math::sin(rads);
		result.data2d[2][2] = Math::cos(rads);
		return result;
	}

	
	static Mat4 scale(Vec3 a) {
		Mat4 result = Mat4();
		result.data2d[0][0] = a.x;
		result.data2d[1][1] = a.y;
		result.data2d[2][2] = a.z;
		return result;
	}
	
	static Mat4 inverseScale(const Mat4 &a) {
		Mat4 result = Mat4();
		result.data2d[0][0] = 1.0f / a.data2d[0][0];
		result.data2d[1][1] = 1.0f / a.data2d[1][1];
		result.data2d[2][2] = 1.0f / a.data2d[2][2];
		return result;
	}
	static Mat4 inverseRotation(const Mat4 &a) {
		Mat4 result = a;
		result.data2d[0][1] *= -1.0f;
		result.data2d[1][0] *= -1.0f;
		return result;
	}
	
	static Mat4 inverseTranslation(const Mat4 &a) {
		Mat4 result = a;
		result.data2d[0][3] *= -1.0f;
		result.data2d[1][3] *= -1.0f;
		result.data2d[2][3] *= -1.0f;
		return result;
	}
	
	static Mat4 lookAt(Vec3 position, Vec3 direction, Vec3 up) {
		Mat4 result = Mat4();
		Vec3 cam_side = Vec3::normalize(Vec3::cross(-direction, up));
		Vec3 cam_up = Vec3::cross(cam_side, -direction);
		
		result.data2d[0][0] = cam_side.x;
		result.data2d[0][1] = cam_side.y;
		result.data2d[0][2] = cam_side.z;
		result.data2d[1][0] = cam_up.x;
		result.data2d[1][1] = cam_up.y;
		result.data2d[1][2] = cam_up.z;
		result.data2d[2][0] = direction.x;
		result.data2d[2][1] = direction.y;
		result.data2d[2][2] = direction.z;
		
		result =  result * Mat4::translate(-position);
		
		return result;
	}
	
	static Mat4 transpose(const Mat4 &a) {
		Mat4 result = Mat4();
		
		result.data2d[0][0] = a.data2d[0][0]; result.data2d[1][0] = a.data2d[0][1]; result.data2d[2][0] = a.data2d[0][2]; result.data2d[3][0] = a.data2d[0][3];
		result.data2d[0][1] = a.data2d[1][0]; result.data2d[1][1] = a.data2d[1][1]; result.data2d[2][1] = a.data2d[1][2]; result.data2d[3][1] = a.data2d[1][3];
		result.data2d[0][2] = a.data2d[2][0]; result.data2d[1][2] = a.data2d[2][1]; result.data2d[2][2] = a.data2d[2][2]; result.data2d[3][2] = a.data2d[2][3];
		result.data2d[0][3] = a.data2d[3][0]; result.data2d[1][3] = a.data2d[3][1]; result.data2d[2][3] = a.data2d[3][2]; result.data2d[3][3] = a.data2d[3][3];
		
		return result;
	}
};

namespace Math {
	Vec2 lerp(Vec2 a, Vec2 b, f32 t) { 
		return a + (b - a) * t;
	}
	
	Vec2 quadCurve(Vec2 a, Vec2 b, Vec2 c, f32 t) {
		Vec2 p0 = lerp(a, b, t);
		Vec2 p1 = lerp(b, c, t);
		return lerp(p0, p1, t);
	}
	
	Vec2 cubCurve(Vec2 a, Vec2 b, Vec2 c, Vec2 d, f32 t) {
		Vec2 p0 = quadCurve(a, b, c, t);
		Vec2 p1 = quadCurve(b, c, d, t);
		return lerp(p0, p1, t);
	}
};