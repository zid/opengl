#include <stdio.h>
#include "shader.h"
#include "png.h"
#include "gl.h"

/* Shaders */
static struct shader *box, *sky;

/* Uniforms */
static GLuint frames;

/* VAO and VBOs */
static GLuint vbo[2];
static GLuint vao;

/* Textures */
static GLuint tex, skytex;

static unsigned int frame_counter;

static const float f[] = {
	-1.0, -1.0,  1.0,
	 1.0, -1.0,  1.0,
	 1.0,  1.0,  1.0,
	-1.0,  1.0,  1.0,
	-1.0, -1.0, -1.0,
	 1.0, -1.0, -1.0,
	 1.0,  1.0, -1.0,
	-1.0,  1.0, -1.0
};

static const unsigned short idx[] = {
	0, 1, 2,
	2, 3, 0,
	1, 5, 6,
	6, 2, 1,
	7, 6, 5,
	5, 4, 7,
	4, 0, 3,
	3, 7, 4,
	4, 5, 1,
	1, 0, 4,
	3, 2, 6,
	6, 7, 3
};

#define F 64.0
#define N 1

static float persp[] = 
{
	0.5625, 0.0, 0, 0.0,
	0.0, 1, 0, 0.0,
	0.0, 0.0, -F/(F-N), -1.0,
	0.0, 0.0, -(F*N)/(F-N), 0
};

static float trans[] = 
{
	1.0, 0.0, 0.0, 0.0,
	0.0, 1.0, 0.0, 0.0,
	0.0, 0.0, 1.0, 0.0,
	0.0, 0.0, -5.0, 1.0
};

static void cubemap_load(void)
{
	int i;

	const char *png_names[] =
	{
		"snek.png",
		"snek.png",
		"white.png",
		"white.png",
		"image0-34.png",
		"image0-34.png"

	};

	const GLenum cube_faces[] =
	{
		GL_TEXTURE_CUBE_MAP_POSITIVE_X,
		GL_TEXTURE_CUBE_MAP_NEGATIVE_X,
		GL_TEXTURE_CUBE_MAP_POSITIVE_Y,
		GL_TEXTURE_CUBE_MAP_NEGATIVE_Y,
		GL_TEXTURE_CUBE_MAP_POSITIVE_Z,
		GL_TEXTURE_CUBE_MAP_NEGATIVE_Z
	};

	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_CUBE_MAP, tex);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_BASE_LEVEL, 0);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAX_LEVEL, 2);
	for(i = 0; i < 6; i++)
	{
		struct png p;
		int format;
		
		p = load_png(png_names[i]);
		format = p.format == 4 ? GL_RGBA : GL_RGB;

		glTexImage2D(cube_faces[i], 0, format, p.w, p.h, 0, format, GL_UNSIGNED_BYTE, p.pixels);

		png_kill(&p);
	}

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		
	glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

	const char *png_names2[] =
	{
		"skybox\\right.png",
		"skybox\\left.png",
		"skybox\\bottom.png",
		"skybox\\top.png",
		"skybox\\front.png",
		"skybox\\back.png"
	};

	glGenTextures(1, &skytex);
	glBindTexture(GL_TEXTURE_CUBE_MAP, skytex);
	for(i = 0; i < 6; i++)
	{
		struct png p;
		int format;
		
		p = load_png(png_names2[i]);
		format = p.format == 4 ? GL_RGBA : GL_RGB;

		glTexImage2D(cube_faces[i], 0, format, p.w, p.h, 0, format, GL_UNSIGNED_BYTE, p.pixels);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		png_kill(&p);
	}
	
}

void load_data(void)
{
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
	glGenBuffers(2, vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
	glBufferData(GL_ARRAY_BUFFER, sizeof f, f, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo[1]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof idx, idx, GL_STATIC_DRAW);
}

void engine_init(void)
{
	glClearColor(0.1, 0.1, 0.1, 0.1);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glEnable(GL_MULTISAMPLE);

	wglSwapIntervalEXT(1);

	load_data();
	box = shader_create("fshade.txt", "vshade.txt");
	glUseProgram(box->program);

	frames = glGetUniformLocation(box->program, "frames");
	glUniformMatrix4fv(glGetUniformLocation(box->program, "persp"), 1, GL_FALSE, persp);
	glUniformMatrix4fv(glGetUniformLocation(box->program, "trans"), 1, GL_FALSE, trans);
	cubemap_load();
	glUniform1i(glGetUniformLocation(box->program, "cubetex"), 0);

	sky = shader_create("fskybox.txt", "vskybox.txt");
	glUseProgram(sky->program);
	glUniform1i(glGetUniformLocation(sky->program, "cubetex"), 0);
}

void engine_reload(void)
{
	shader_reload(&box);
	shader_reload(&sky);
}

void engine(void)
{
	int i;

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	for(i = 0; i < 8; i++)
	{
		glBindVertexArray(vao);
		glUseProgram(box->program);
		glUniform1ui(frames, frame_counter);
		trans[12] = f[i*3+0] * 2.0;
		trans[13] = f[i*3+1] * 2.0;
		trans[14] = f[i*3+2] * 2.0 + -5.0 - frame_counter/20.0;
		glUniformMatrix4fv(glGetUniformLocation(box->program, "trans"), 1, GL_FALSE, trans);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, tex);
		glDrawElements(GL_TRIANGLES, sizeof idx / sizeof idx[0], GL_UNSIGNED_SHORT, 0);
	}
	
	glBindVertexArray(vao);
	glUseProgram(sky->program);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, skytex);
	glDrawElements(GL_TRIANGLES, sizeof idx / sizeof idx[0], GL_UNSIGNED_SHORT, 0);
	
	frame_counter++;
}