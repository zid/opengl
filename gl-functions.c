
#define ABI __attribute__((stdcall))
#include "gl.h"

/*
 * ABI not declared with 'extern' here.
 * This means we are actually declaring them here.
 */
#include "gl-functions.h"

void gl_init_procs(void)
{
	glBindBuffer = (void *)wglGetProcAddress("glBindBuffer");
	glGenBuffers = (void *)wglGetProcAddress("glGenBuffers");
	glBufferData = (void *)wglGetProcAddress("glBufferData");
	glGenVertexArrays = (void *)wglGetProcAddress("glGenVertexArrays");
	glBindVertexArray = (void *)wglGetProcAddress("glBindVertexArray");
	glVertexAttribPointer = (void *)wglGetProcAddress("glVertexAttribPointer");
	glEnableVertexAttribArray = (void *)wglGetProcAddress("glEnableVertexAttribArray");
	glCreateShader = (void *)wglGetProcAddress("glCreateShader");
	glShaderSource = (void *)wglGetProcAddress("glShaderSource");
	glCompileShader = (void *)wglGetProcAddress("glCompileShader");
	glCreateProgram = (void *)wglGetProcAddress("glCreateProgram");
	glUseProgram = (void *)wglGetProcAddress("glUseProgram");
	glLinkProgram = (void *)wglGetProcAddress("glLinkProgram");
	glAttachShader = (void *)wglGetProcAddress("glAttachShader");
	wglSwapIntervalEXT = (void *)wglGetProcAddress("wglSwapIntervalEXT");
	glGetUniformLocation = (void *)wglGetProcAddress("glGetUniformLocation");
	glUniform1ui = (void *)wglGetProcAddress("glUniform1ui");
	glUniform1i = (void *)wglGetProcAddress("glUniform1i");
	glUniformMatrix4fv = (void *)wglGetProcAddress("glUniformMatrix4fv");
	glDeleteShader = (void *)wglGetProcAddress("glDeleteShader");
	glDeleteProgram = (void *)wglGetProcAddress("glDeleteProgram");
	glActiveTexture = (void *)wglGetProcAddress("glActiveTexture");
	glGenerateMipmap = (void *)wglGetProcAddress("glGenerateMipmap");
	glGetShaderInfoLog = (void *)wglGetProcAddress("glGetShaderInfoLog");
	glGetShaderiv = (void *)wglGetProcAddress("glGetShaderiv");
}
