#ifndef MEMORY_H
#define MEMORY_H

struct MemoryBlock {
	void *memory;
	u64 size;
};

#define MEMORY_STORE_COUNT 3

struct MemoryStore {
	void *memory;
	union {
		struct {
			MemoryBlock game_memory;
			MemoryBlock asset_memory;
			MemoryBlock frame_memory;
			
		};
		MemoryBlock blocks[MEMORY_STORE_COUNT];
	};
	
};

#endif // MEMORY_H