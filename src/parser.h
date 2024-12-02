/*  ----------------------------------- INFOS
	Parsing various files and format for what we need
	
*/

#ifndef _PARSERH_
#define _PARSERH_
----------------------------- TEXTURES FORMAT & IMAGES
// -------

typedef struct complete_img {
	ui32 x;
	ui32 y;
	ui32 channels_in_file;
	void *memory;
} complete_img;

// todo: finish this
complete_img parse_decode_png(char *location, complete_file *file) {
	complete_img img = {0};
	
	io_file_fullread(location, file);
	int x; 
	int y;
	int channels_in_file;
	img.memory = (void*)stbi_load_from_memory((stbi_uc*)file->memory, file->size, &x, &y, &channels_in_file, 4);
	img.x = (ui32)x;
	img.y = (ui32)y;
	img.channels_in_file = (ui32)channels_in_file;
	return img;
}

#endif /* _PARSERH_ */