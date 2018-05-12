struct StackAllocator {
	private:
		u8 *memory;	
		u8 *top;
		u64 block_size;	
	public:
		typedef u32 Marker;
		
		void initialize(void *block, u64 size) {
			this->memory = (u8 *)block;
			top = this->memory;
			this->block_size = size;
			
		}
		
		Marker getMarker() {
			return (u32)(top - memory);
		}
		
		void freeToMarker(Marker marker) {
			top = memory + marker;
		}
		
		void *alloc(u64 size) {
			Assert((u64)((top + size) - memory) <= block_size);
			void *result = (u8 *)top;
			top += size;
			return (void *)result;
		}	
		
		void clear() {
			top = memory;
		}
		
		void *getTop() { return (void *)top; }
};

struct VertexStack {
	private:
		StackAllocator *fs;
		Vec3 *vertices_start;
		u32 count;
		u32 marker;
	public:
		VertexStack(StackAllocator *frame_stack) {
			marker = frame_stack->getMarker();
			vertices_start = (Vec3 *)frame_stack->getTop();
			fs = frame_stack;
			count = 0;
		}
		
		void push(Vec3 vertex) {
			Vec3 *vp = (Vec3 *)fs->alloc(sizeof(Vec3));
			*vp = vertex;
			count++;
		}
		
		void clear() { // NOTE(nathan): if anyone else uses the stack while this has it, this clear will fuck them up
			fs->freeToMarker(marker);
		}
		
		u64 getSize() {
			return count * sizeof(Vec3);
		}
		
		u32 getCount() {
			return count;
		}
		
		Vec3 *getPointer() {
			return vertices_start;
		}
};