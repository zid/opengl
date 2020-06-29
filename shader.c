#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "gl.h"
#include "shader.h"

static char *load_txt(const char *name)
{
	FILE *f;
	size_t len;
	char *s;

	f = fopen(name, "rb");
	if(!f)
		return NULL;

	fseek(f, 0, SEEK_END);
	len = ftell(f);
	rewind(f);

	s = malloc(len+1);
	fread(s, 1, len, f);

	s[len] = 0;

	fclose(f);

	return s;
}

struct shader *shader_create(const char *fpath, const char *vpath)
{
	char *vshade_src, *fshade_src, *buf;
	struct shader *s;
	GLint len;

	s = malloc(sizeof (struct shader));
	if(!s)
		return NULL;


	s->vshade_path = strdup(vpath);
	s->fshade_path = strdup(fpath);

	vshade_src = load_txt(s->vshade_path);

	s->vshade = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(s->vshade, 1, (const char **)&vshade_src, NULL);
	glCompileShader(s->vshade);
	free(vshade_src);
	glGetShaderiv(s->vshade, GL_INFO_LOG_LENGTH, &len);
	buf = malloc(len);
	glGetShaderInfoLog(s->vshade, len, NULL, buf);
	if(len)
		perror(buf);
	free(buf);

	fshade_src = load_txt(s->fshade_path);

	s->fshade = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(s->fshade, 1, (const char **)&fshade_src, NULL);
	glCompileShader(s->fshade);
	free(fshade_src);
	glGetShaderiv(s->fshade, GL_INFO_LOG_LENGTH, &len);
	buf = malloc(len);
	glGetShaderInfoLog(s->fshade, len, NULL, buf);
	if(len)
		perror(buf);
	free(buf);

	s->program = glCreateProgram();

	glAttachShader(s->program, s->vshade);
	glAttachShader(s->program, s->fshade);
	glLinkProgram(s->program);

	return s;
}

void shader_reload(struct shader **s)
{
	char *vpath, *fpath;
	struct shader *old, *new;

	old = *s;

	vpath = old->vshade_path;
	fpath = old->fshade_path;

	glDeleteShader(old->vshade);
	glDeleteShader(old->fshade);
	glDeleteProgram(old->program);

	new = shader_create(vpath, fpath);
	free(vpath);
	free(fpath);
	free(old);

	*s = new;
}
