struct AABB {
	Vec2 min, max;
	
	Vec2 closestPoint(Vec2 point) {
		f32 x = Math::rmax(min.x, Math::rmin(point.x, max.x));
		f32 y = Math::rmax(min.y, Math::rmin(point.y, max.y));
		return Vec2(x, y);
	}
	
	Vec2 topRight() {
		return Vec2(max.x, min.y);
	}
	
	Vec2 topLeft() {
		return Vec2(min.x, min.y);
	}
	
	Vec2 bottomRight() {
		return Vec2(max.x, max.y);
	}
	
	Vec2 bottomLeft() {
		return Vec2(min.x, max.y);
	}
	
	AABB inflate(f32 r) {
		return {min - r, max + r};
	}
};


#define EPSILON 0.01f

internal_func bool clipLine(int d, const AABB &box, Vec2 v0, Vec2 v1, f32 &f_low, f32 &f_high) {
	f32 f_dim_low, f_dim_high;
	f_dim_low = (box.min.xy[d] - v0.xy[d]) / (v1.xy[d] - v0.xy[d]);
	f_dim_high = (box.max.xy[d] - v0.xy[d]) / (v1.xy[d] - v0.xy[d]);
	
	if(f_dim_high < f_dim_low) Swap(f_dim_high, f_dim_low);
	if(f_dim_high < f_low) return false;
	if(f_dim_low > f_high) return false;
	f_low = Math::rmax(f_dim_low, f_low);
	f_high = Math::rmin(f_dim_high, f_high);
	
	if(f_low > f_high) return false;
	return true;
}

internal_func bool lineAABBIntersects(Vec2 v0, Vec2 v1, const AABB &box, Vec2 &intersection, f32 &fraction) {
	f32 f_low = 0;
	f32 f_high = 1;
	
	if(!clipLine(0, box, v0, v1, f_low, f_high)) return false;	
	if(!clipLine(1, box, v0, v1, f_low, f_high)) return false;	
	
	Vec2 b = v1 - v0;
	intersection = v0 + b * f_low;
	fraction =  f_low;
	return true;	
}

internal_func int lineCircleIntersects(Vec2 p, Vec2 p1, Vec2 s, f32 r, f32 &t, Vec2 &q)
{
	Vec2 d = Vec2::normalize(p1 - p);
	Vec2 m= p - s;
	f32 b = Vec2::dot(m, d);
	f32 c = Vec2::dot(m, m) - r * r;
	// Exit if r’s origin outside s (c > 0) and r pointing away from s (b > 0)
	if (c > 0.0f && b > 0.0f) return 0;
	f32 discr = b*b - c;
	// A negative discriminant corresponds to ray missing sphere
	if (discr < 0.0f) return 0;
	
	// Ray now found to intersect sphere, compute smallest t value of intersection
	f32 tb = (-b - Math::squareRoot(discr));
	f32 l = Vec2::length(p1 - p);
	t = tb / l;
	// If t is negative, ray started inside sphere so clamp t to zero
	if (t < 0.0f) t = 0.0f;
	q = p + d * tb;
	if(t > 1.0f) return 0;
	return 1;
}


internal_func bool sweptCircleAABBIntersects(Vec2 circle_start, Vec2 circle_end, f32 radius,  AABB collider, Vec2 &intersection, f32 &fraction) {
	AABB inflated = collider.inflate(radius);
	// renderAABB(renderer, inflated);
	Vec2 test_intersection;
	f32 test_fraction;
	bool inflated_col = lineAABBIntersects(circle_start, circle_end, inflated, test_intersection, test_fraction);
	if(!inflated_col) return false;
	test_fraction = FLT_MAX;
	fraction = 1.0f;
	Vec2 points[4] = { collider.topLeft(), collider.topRight(), collider.bottomLeft(), collider.bottomRight()};
	for(u32 i = 0; i < 4; i++) {
		// renderEllipsoid(renderer, points[i], radius);
		if(lineCircleIntersects(circle_start, circle_end, points[i], radius, test_fraction, test_intersection) && test_fraction < fraction) {
			fraction = test_fraction;
			intersection = test_intersection;
		}
	}
	
	AABB top = {Vec2(collider.min.x, inflated.min.y), Vec2(collider.max.x, collider.min.y) };
	AABB bottom = {Vec2(collider.min.x, collider.max.y), Vec2(collider.max.x, inflated.max.y) };
	AABB right = {Vec2(collider.max.x, collider.min.y), Vec2(inflated.max.x, collider.max.y) };
	AABB left = {Vec2(inflated.min.x, collider.min.y), Vec2(collider.min.x, collider.max.y) };
	
	AABB boundaries[4] = {
		top,bottom,right,left
	};
	
	for(u32 i = 0; i < 4; i++) {
		// renderAABB(renderer, boundaries[i]);
		if(lineAABBIntersects(circle_start, circle_end, boundaries[i], test_intersection, test_fraction) && test_fraction < fraction) {
			fraction = test_fraction;
			intersection = test_intersection;
		}
	}
	if(fraction < 1.0f) return true;
	else return false;
}

internal_func f32 signed2DTriArea(Vec2 a, Vec2 b, Vec2 c) {
	return (a.x - c.x) * (b.y - c.y) - (a.y - c.y) * (b.x - c.x);
}

internal_func bool segmentIntersect(Vec2 a, Vec2 b, Vec2 c, Vec2 d, f32 &t, Vec2 &p) {
	// Sign of areas correspond to which side of ab points c and d are
	f32 a1 = signed2DTriArea(a, b, d); // Compute winding of abd (+ or -)
	f32 a2 = signed2DTriArea(a, b, c); // To intersect, must have sign opposite of a1
	// If c and d are on different sides of ab, areas have different signs
	if (a1 * a2 < 0.0f) {
		// Compute signs for a and b with respect to segment cd
		f32 a3 = signed2DTriArea(c, d, a); // Compute winding of cda (+ or -)
		// Since area is constant a1 - a2 = a3 - a4, or a4 = a3 + a2 - a1
		// f32 a4 = Signed2DTriArea(c, d, b); // Must have opposite sign of a3
		f32 a4 = a3 + a2 - a1;
		// Points a and b on different sides of cd if areas have different signs
		if (a3 * a4 < 0.0f) {
			// Segments intersect. Find intersection point along L(t) = a + t * (b - a).
			// Given height h1 of an over cd and height h2 of b over cd,
			// t = h1 / (h1 - h2) = (b*h1/2) / (b*h1/2 - b*h2/2) = a3 / (a3 - a4),
			// where b (the base of the triangles cda and cdb, i.e., the length
			// of cd) cancels out.
			t = a3 / (a3 - a4);
			p = a + t * (b - a);
			return true;
		}
	}
	// Segments not intersecting (or collinear)
	return false;
}


struct Line {
	Vec2 start;
	Vec2 end;	
	
	Vec2 closestPoint(Vec2 point) {
		Vec2 ab = end-start;
		// Project c onto ab, computing parameterized position d(t)=a+ t*(b – a)
		f32 t = Vec2::dot(point - start, ab) / Vec2::dot(ab, ab);
		// If outside segment, clamp t (and therefore d) to the closest endpoint
		if (t < 0.0f) t = 0.0f;
		if (t > 1.0f) t = 1.0f;
		// Compute projected position from the clamped t
		Vec2 d = start + ab * t;		
		return d;
	}
};

struct Circle {
	Vec2 center;
	f32 radius;
	
	Vec2 closestPoint(Vec2 point) {
		return center + Vec2::normalize(point - center) * radius;
	}	
	
	bool pointInside(Vec2 point) {
		return Vec2::length(point - center) <= radius;
	}
};

internal_func bool sweptCircleLineIntersects(Vec2 circle_start, Vec2 circle_end, f32 radius,  Line collider, Vec2 &intersection, f32 &fraction) {
	Vec2 test_intersection;
	f32 test_fraction;
	test_fraction = FLT_MAX;
	fraction = 1.0f;
		// renderEllipsoid(renderer, points[i], radius);
	if(lineCircleIntersects(circle_start, circle_end, collider.start, radius, test_fraction, test_intersection) && test_fraction < fraction) {
		fraction = test_fraction;
		intersection = test_intersection;
	}
	
	if(lineCircleIntersects(circle_start, circle_end, collider.end, radius, test_fraction, test_intersection) && test_fraction < fraction) {
		fraction = test_fraction;
		intersection = test_intersection;
	}
	
	Vec2 n = Vec2::normalize(collider.end - collider.start);
	Vec2 r = Vec2::rPerp(n);
	Vec2 l = Vec2::lPerp(n);
	Vec2 top_left = collider.end + l * radius;
	Vec2 top_right = collider.end + r * radius;
	
	Vec2 bottom_left = collider.start + l * radius;
	Vec2 bottom_right = collider.start + r * radius;
	
	if(segmentIntersect(circle_start, circle_end, bottom_left, top_left, test_fraction, test_intersection) && test_fraction < fraction) {
		fraction = test_fraction;
		intersection = test_intersection;
	}
	
	if(segmentIntersect(circle_start, circle_end, bottom_right, top_right, test_fraction, test_intersection) && test_fraction < fraction) {
		fraction = test_fraction;
		intersection = test_intersection;
	}
	
	if(fraction < 1.0f) return true;
	else return false;
}

internal_func bool sweptCircleCircleIntersects(Vec2 circle_start, Vec2 circle_end, f32 radius,  Circle collider, Vec2 &intersection, f32 &fraction) {
	Vec2 test_intersection;
	f32 test_fraction;
	test_fraction = FLT_MAX;
	fraction = 1.0f;
		// renderEllipsoid(renderer, points[i], radius);
	if(lineCircleIntersects(circle_start, circle_end, collider.center, radius + collider.radius, test_fraction, test_intersection) && test_fraction < fraction) {
		fraction = test_fraction;
		intersection = test_intersection;
	}
	
	if(fraction < 1.0f) return true;
	else return false;
}

struct Plane {
	Vec2 point;
	Vec2 normal;
	
	Vec2 closestPoint(Vec2 p) {
		Vec2 project_point = point + Vec2::project(p - point, Vec2::rPerp(normal));
		return project_point;
	}
	
	f32 distance(Vec2 p) {
		return Vec2::length(p - closestPoint(p));
	}
};

struct Bezier {
	union {
		struct {
			Vec2 a, b, c, d;		
		};
		Vec2 points[4];
	};
	s32 tesselation = 20;
};

struct CollisionStats {
	f32 fraction;
	Plane sliding_plane;	
	Vec2 near_point;
	bool hit;
};

enum class CollisionMask {
	Default = 1, 
	Entity = 1 << 1,
	Zombie = 1 << 2,
};

struct SceneSaveHeader {
	u32 static_aabb_count;
	u32 static_aabb_size;
	u32 static_line_count;
	u32 static_line_size;
	u32 static_circle_count;
	u32 static_circle_size;
	u32 static_bez_count;
	u32 static_bez_size;
	Vec2 player_start;
	u32 data_offset;	
};



struct Scene {
	std::vector<AABB> static_aabbs;
	std::vector<Line> static_lines;
	std::vector<Circle> static_circles;
	std::vector<Bezier> static_beziers;	
	Vec2 player_start;
	
	
	void addStaticBox(AABB box) {
		if(box.min.x > box.max.x) Swap(box.min.x, box.max.x);
		if(box.min.y > box.max.y) Swap(box.min.y, box.max.y);
		static_aabbs.push_back(box);
	}
	
	void addStaticLine(Vec2 start, Vec2 end) {
		static_lines.push_back({start, end});
		
	}
	
	void moveStaticCircle(u32 index, Vec2 new_position) {
		static_circles[index].center = new_position;
	}
	
	u32 addStaticCircle(Vec2 center, f32 radius) {
		static_circles.push_back({center, radius});
		return (u32)static_circles.size()-1;
	}
	
	void addStaticBezier(Vec2 a, Vec2 b, Vec2 c, Vec2 d) {
		static_beziers.push_back({a, b, c, d});
	}
	
	CollisionStats sweptQuery(Vec2 pos, f32 radius, Vec2 vel) {
		CollisionStats result = {};
		
		Vec2 intersection_point;
		Vec2 test_intersection_point;
		Vec2 actual_intersection;
		
		f32 fraction = FLT_MAX;
		f32 test_fraction;
		bool intersects = false;
		
		for(int i = 0; i < static_aabbs.size(); i++) {
			AABB collider = static_aabbs[i];	;
			if(sweptCircleAABBIntersects(pos, pos + vel, radius, collider,  test_intersection_point, test_fraction) && test_fraction < fraction) {
				intersects = true;
				intersection_point = test_intersection_point;
				actual_intersection = collider.closestPoint(intersection_point);
				fraction = test_fraction;
				
			}
		}
		
		for(int i = 0; i < static_lines.size(); i++) {
			Line collider = static_lines[i];	;
			if(sweptCircleLineIntersects(pos, pos + vel, radius, collider,  test_intersection_point, test_fraction) && test_fraction < fraction) {
				intersects = true;
				intersection_point = test_intersection_point;
				actual_intersection = collider.closestPoint(intersection_point);
				fraction = test_fraction;
				
			}
		}
		
		for(u32 i = 0; i < static_circles.size(); i++) {
			Circle collider = static_circles[i];	;
			if(sweptCircleCircleIntersects(pos, pos + vel, radius, collider,  test_intersection_point, test_fraction) && test_fraction < fraction) {
				intersects = true;
				intersection_point = test_intersection_point;
				actual_intersection = collider.closestPoint(intersection_point);
				fraction = test_fraction;
				
			}
		}
		for(int c = 0; c < static_beziers.size(); c++) {
			Bezier bez = static_beziers[c];
			s32 tess_detail = bez.tesselation;
			Vec2 previous = Math::cubCurve(bez.a, bez.b, bez.c, bez.d, 0.0f);
			for(int i = 1; i < tess_detail+1; i++) {
				f32 t = (1.0f / (f32)tess_detail) * (f32)i;
				Vec2 p = Math::cubCurve(bez.a, bez.b, bez.c, bez.d, t);
				Vec2 normal = Vec2::lPerp(Vec2::normalize(p - previous));
				Line collider = {previous, p};
				if(sweptCircleLineIntersects(pos, pos + vel, radius, collider,  test_intersection_point, test_fraction) && test_fraction < fraction) {
					intersects = true;
					intersection_point = test_intersection_point;
					actual_intersection = collider.closestPoint(intersection_point);
					fraction = test_fraction;
					
				}
				previous = p;
			}
		}
		
		result.hit = intersects;
		if(!intersects) {
			return result;
		}
		
		
		f32 distance = (Vec2::length(vel) * fraction);
		Vec2 touch_point = pos + vel * fraction;
		f32 short_distance = Math::rmax(distance - EPSILON, 0);
		Vec2 near_point = pos + Vec2::normalize(vel) * short_distance;
		Vec2 surface_normal = Vec2::normalize(intersection_point - actual_intersection);
		result.sliding_plane = {actual_intersection, surface_normal};
		result.fraction = fraction;
		result.near_point = near_point;
		
		return result;
	}
	
	void clear() {
		static_aabbs.clear();
		static_lines.clear();
		static_circles.clear();
		static_beziers.clear();
		player_start = Vec2();
	}
	
	void loadFromFile(FileData *file) {
				
		SceneSaveHeader *new_header = (SceneSaveHeader *)file->contents;
		player_start = new_header->player_start;
		u8 *byte_walker = (u8 *)file->contents;
		byte_walker += new_header->data_offset;
		
		static_aabbs.clear();
		size_t aabb_block_size = new_header->static_aabb_count * new_header->static_aabb_size;
		for(u32 i = 0; i < new_header->static_aabb_count; i++) {
			static_aabbs.push_back(((AABB *)byte_walker)[i]);
		}
		byte_walker += aabb_block_size;
		
		static_lines.clear();
		size_t line_block_size = new_header->static_line_count * new_header->static_line_size;
		for(u32 i = 0; i < new_header->static_line_count; i++) {
			static_lines.push_back(((Line *)byte_walker)[i]);
		}
		byte_walker += line_block_size;
		
		static_circles.clear();
		size_t circle_block_size = new_header->static_circle_count * new_header->static_circle_size;
		for(u32 i = 0; i < new_header->static_circle_count; i++) {
			static_circles.push_back(((Circle *)byte_walker)[i]);
		}
		byte_walker += circle_block_size;
		
		static_beziers.clear();
		size_t bez_block_size = new_header->static_bez_count * new_header->static_bez_size;
		for(u32 i = 0; i < new_header->static_bez_count; i++) {
			static_beziers.push_back(((Bezier *)byte_walker)[i]);
		}
		byte_walker += bez_block_size;
	}

};