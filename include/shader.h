#ifndef SHADER_H
#define SHADER_H

#include <gl/gl.h>

struct shader
{
	GLuint vshade, fshade;
	GLuint program;
	char *vshade_path, *fshade_path;
	GLint uniforms[];
};

struct shader *shader_create(const char *fpath, const char *vpath);
void shader_reload(struct shader **shader);
#endif