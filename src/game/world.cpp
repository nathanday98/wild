enum class Component : u32 {
	None = 0,
	Transform = 1,
	Sprite = 1 << 1,
};

static Component operator |(Component left, Component right) {
	return (Component)((u32)left | (u32)right);
}

struct Transform {
	Vec3 position;	
};

struct Sprite {
	Vec2 size;
	Vec4 color;	
};

#define MAX_ENTITIES 64
typedef u32 Entity;

struct World {
	Entity entities[MAX_ENTITIES];
	Transform entity_transforms[MAX_ENTITIES];
	Sprite entity_sprites[MAX_ENTITIES];
	
	Entity allocEntity() {
		for(int i = 0; i < MAX_ENTITIES; i++) {
			if(entities[i] == (u32)Component::None) {
				return i;
			}
		}
		
		Assert(1 == 0);
		return MAX_ENTITIES;
	}
	
	bool entityHasComponent(Entity entity, Component comp) {
		return (entities[entity] & (u32)comp) == (u32)comp;
	}
	
	void assignEntityComponent(Entity entity, Component comp) {
		entities[entity] |= (u32)comp;
	}
	
	Transform *getEntityTransform(Entity entity) {
		return &entity_transforms[entity];
	}
	
	Sprite *getEntitySprite(Entity entity) {
		return &entity_sprites[entity];
	}
};