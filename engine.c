#include <stdio.h>
#include "shader.h"
#include "zpng.h"
#include "gl.h"

enum UNIFORMS
{
	UNIFORM_FRAMES,
	UNIFORM_PERSP,
	UNIFORM_TRANS
};

enum SHADER
{
	SHADER_CUBE,
	SHADER_SKY
};

struct model
{
	int vao;
	size_t faces;
	enum SHADER shader;
	GLuint texture;
	void (*perframe)(struct model *);
	void *data;
};

struct scene
{
	struct model **models;
	size_t model_count;
};

struct vao_info
{
	GLuint vao;
	GLuint vbo[2];
};

/* Cube vertices */
static float cube_verts[] = {
	-1.0, -1.0,  1.0,
	 1.0, -1.0,  1.0,
	 1.0,  1.0,  1.0,
	-1.0,  1.0,  1.0,
	-1.0, -1.0, -1.0,
	 1.0, -1.0, -1.0,
	 1.0,  1.0, -1.0,
	-1.0,  1.0, -1.0
};

/* Cube faces */
static const unsigned short cube_faces[] = {
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

/* Perspective matrix */
#define F 64.0
#define N 1

static float persp[] = 
{
	0.5625, 0.0, 0, 0.0,
	0.0, 1, 0, 0.0,
	0.0, 0.0, -F/(F-N), -1.0,
	0.0, 0.0, -(F*N)/(F-N), 0
};

/* Temp transformation matrix */
static float trans[] = 
{
	1.0, 0.0, 0.0, 0.0,
	0.0, 1.0, 0.0, 0.0,
	0.0, 0.0, 1.0, 0.0,
	0.0, 0.0, -5.0, 1.0
};

static struct scene scene;
static unsigned int frame_counter;

/* If we get multiple models, make this a managed list of our VAOs */
struct vao_info vao_list[1];
struct shader *shader_list[2];

static const char *cube_texnames[] =
{
	"snek.png",
	"snek.png",
	"white.png",
	"white.png",
	"image0-34.png",
	"image0-34.png"
};

const char *skybox_texnames[] =
{
	"skybox\\right.png",
	"skybox\\left.png",
	"skybox\\bottom.png",
	"skybox\\top.png",
	"skybox\\front.png",
	"skybox\\back.png"
};

static void cube_update(struct model *m)
{
	int n = *(int *)m->data;
	trans[12] = cube_verts[n*3-3] * 2.0;
	trans[13] = cube_verts[n*3-2] * 2.0;
	trans[14] = cube_verts[n*3-1] * 2.0 + -6.6;

	glUniformMatrix4fv(UNIFORM_TRANS, 1, GL_FALSE, trans);
	glUniform1ui(UNIFORM_FRAMES, frame_counter);
}

static GLuint cubemap_new(const char *names[])
{
	int i;
	GLuint tex;

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
		
		p = load_png(names[i]);
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
	
	return tex;
}

static struct model *model_new(enum SHADER shader, GLuint texture, int vao, size_t faces, void (*perframe)(struct model *))
{
	struct model *m;

	m = malloc(sizeof (struct model));
	if(!m)
		return NULL;

	m->vao = vao;
	m->faces = faces;
	m->shader = shader;
	m->texture = texture;
	m->perframe = perframe;

	return m;
}

static int vao_new(const float *verts, size_t nverts, const unsigned short *faces, size_t nfaces)
{
	struct vao_info *v;
	GLuint vao, vbo[2];

	glGenVertexArrays(1, &vao);
	glGenBuffers(2, vbo);

	glBindVertexArray(vao);

	/* VBO 0 contains vertex data */
	glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
	glBufferData(GL_ARRAY_BUFFER, nverts * sizeof verts[0], verts, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo[1]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, nfaces * sizeof faces[0], faces, GL_STATIC_DRAW);
	
	/* Hardcoded to 0 until we manage this list properly */
	v = &vao_list[0];
	v->vao = vao;
	v->vbo[0] = vbo[0];
	v->vbo[1] = vbo[1];

	return 0;
}

static void scene_init(struct scene *sc)
{
	struct shader *sky_shader, *cube_shader;
	int i, vao;
	size_t nverts, nfaces;
	GLuint skybox_texture, cube_texture;

	nverts = sizeof cube_verts / sizeof cube_verts[0];
	nfaces = sizeof cube_faces / sizeof cube_faces[0];

	/* The skybox and cubes use the same verts, so make a single vao */
	vao = vao_new(cube_verts, nverts, cube_faces, nfaces);

	/* Cube shader and its uniforms */
	cube_shader = shader_create("fshade.txt", "vshade.txt");
	glUseProgram(cube_shader->program);
	glUniformMatrix4fv(UNIFORM_PERSP, 1, GL_FALSE, persp);
	glUniformMatrix4fv(UNIFORM_TRANS, 1, GL_FALSE, trans);
	shader_list[SHADER_CUBE] = cube_shader;

	/* Skybox shader */
	sky_shader  = shader_create("fskybox.txt", "vskybox.txt");
	shader_list[SHADER_SKY] = sky_shader;

	/* Cubes share a texture, skybox has its own */
	skybox_texture = cubemap_new(skybox_texnames);
	cube_texture   = cubemap_new(cube_texnames);

	/* Hardcoded scene layout */
	sc->model_count = 9;
	sc->models = malloc(sizeof (struct model *[9]));

	/* Model 0 is the skybox */
	sc->models[0] = model_new(SHADER_SKY, skybox_texture, vao, nfaces, NULL);

	/* Model 1-8 are identical in the scene */
	for(i = 1; i <= 8; i++)
	{
		sc->models[i] = model_new(SHADER_CUBE, cube_texture, vao, nfaces, cube_update);
	}
}

void engine_init(void)
{
	glClearColor(0.1, 0.1, 0.1, 0.1);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glEnable(GL_MULTISAMPLE);
	wglSwapIntervalEXT(1);

	scene_init(&scene);
}

void engine_reload(void)
{
	shader_reload(&shader_list[SHADER_CUBE]);
	glUseProgram(shader_list[SHADER_CUBE]->program);
	glUniform1ui(SHADER_CUBE, frame_counter);
	glUniformMatrix4fv(UNIFORM_PERSP, 1, GL_FALSE, persp);

	shader_reload(&shader_list[SHADER_SKY]);
}

void engine(void)
{
	size_t n;

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	for(n = 0; n < scene.model_count; n++)
	{
		struct model *m = scene.models[n];

		glBindVertexArray(vao_list[m->vao].vao);
		glUseProgram(shader_list[m->shader]->program);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, m->texture);

		if(m->perframe)
		{
			m->data = &(int){n};
			m->perframe(m);
		}

		glDrawElements(GL_TRIANGLES, m->faces, GL_UNSIGNED_SHORT, 0);
	}

	frame_counter++;
}