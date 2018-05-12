#include <windows.h>
#include <core/platform.h>

void Platform::copyFile(char *a, char *b) {
	CopyFile(a, b, FALSE);
}

FileTime Platform::getLastWriteTime(char *filename) {
	FileTime last_write_time = {0};
	WIN32_FIND_DATA find_data;
	HANDLE find_handle = FindFirstFileA(filename, &find_data);
	if(find_handle != INVALID_HANDLE_VALUE) {
		last_write_time.low_date_time = find_data.ftLastWriteTime.dwLowDateTime;
		last_write_time.high_date_time = find_data.ftLastWriteTime.dwHighDateTime;
		FindClose(find_handle);
	}
	
	return last_write_time;
}

u32 Platform::compareFileTime(FileTime *a, FileTime *b) {
	FILETIME f;
	f.dwLowDateTime = a->low_date_time;
	f.dwHighDateTime = a->high_date_time;
	FILETIME g;
	g.dwLowDateTime = b->low_date_time;
	g.dwHighDateTime = b->high_date_time;
	return CompareFileTime(&f, &g);
}