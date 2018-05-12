#ifndef PLATFORM_H
#define PLATFORM_H

#include <engine/std.h>
#include <SDL2/SDL.h>


internal_func void error(char *err, bool sdl_err = true) {
	char *string = 0;
	if(sdl_err)
		string = formatString("%s\n%s", err, SDL_GetError());
	else
		string = formatString("%s", err);
	SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error", string, 0);
}

struct FileData {
	char *contents;
	u64 size;	
};

struct FileTime {
	u32 low_date_time;
	u32 high_date_time;	
};

struct  Platform
{
	static void copyFile(char *a, char *b);
	static FileTime getLastWriteTime(char *file);
	static u32 compareFileTime(FileTime *a, FileTime *b);
	
	static FileData readEntireFile(const char *filename) {
		FileData result = {0};
		
		SDL_RWops *diffuse_file = SDL_RWFromFile(filename, "rb");
		if(diffuse_file) {
			
			result.size = (u64)SDL_RWsize(diffuse_file);
			result.contents = (char *)SDL_malloc(result.size);
			
			SDL_RWread(diffuse_file, result.contents, 1, result.size);
			SDL_RWclose(diffuse_file);
		}
		else {
			error(formatString("Can't read %s", filename));
		}
		
		return result;
	}
	
	static void writeStructureToFile(const char *filename, void *structure, s32 size) {
		SDL_RWops *file = SDL_RWFromFile(filename, "wb");
		if(file) {
			if(SDL_RWwrite(file, structure, 1, size) != size) {
				error("Failed to write structure");
			}
			SDL_RWclose(file);
		} else {
			error("Couldn't open file for writing");
		}
	}
	
	static void *openFileForWriting(const char *filename) {
		SDL_RWops *file = SDL_RWFromFile(filename, "wb");
		void *result = 0;
		if(file) {
			result = file;
		} else {
			error("Couldn't open file for writing");
			result = 0;
		}
		return result;
	}
	
	static void closeOpenFile(void *file) {
		if(file) {
			SDL_RWclose((SDL_RWops *)file);
		}
	}
	
	static void writeToFile(void *file, void *structure, s32 size) {
		if(file) {
			if(SDL_RWwrite((SDL_RWops *)file, structure, 1, size) != size) {
				error("Failed to write structure");
			}
		}
	}

};

#endif // PLATFORM_H