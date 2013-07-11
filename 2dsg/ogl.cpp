#include "ogl.h"
#include <cassert>

static GLuint s_texture = 0;
static int s_BindTextureCount = 0;
static bool s_Texture2DEnabled = false;
static int s_Texture2DStateCount = 0;

static int s_VERTEX_ARRAY = 0;
static int s_TEXTURE_COORD_ARRAY = 0;
static int s_COLOR_ARRAY = 0;

static bool s_VERTEX_ARRAY_enabled = false;
static bool s_TEXTURE_COORD_ARRAY_enabled = false;
static bool s_COLOR_ARRAY_enabled = false;

static int s_clientStateCount = 0;

void oglReset()
{
	s_texture = 0;
	s_Texture2DEnabled = false;

	glDisable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, 0);

	s_VERTEX_ARRAY_enabled = false;
	s_TEXTURE_COORD_ARRAY_enabled = false;
    s_COLOR_ARRAY_enabled = false;
	s_VERTEX_ARRAY = 0;
	s_TEXTURE_COORD_ARRAY = 0;
    s_COLOR_ARRAY = 0;
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    glDisableClientState(GL_COLOR_ARRAY);

	resetBindTextureCount();
	resetClientStateCount();
	resetTexture2DStateCount();

    glEnable(GL_BLEND);

#ifndef PREMULTIPLIED_ALPHA
#error PREMULTIPLIED_ALPHA is not defined
#endif

#if PREMULTIPLIED_ALPHA
    glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
#else
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
#endif

    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);	/* sanity set */

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

void oglEnable(GLenum cap)
{
	switch (cap)
	{
	case GL_TEXTURE_2D:
		if (s_Texture2DEnabled == false)
		{
			glEnable(GL_TEXTURE_2D);
			s_Texture2DEnabled = true;
			s_Texture2DStateCount++;
		}
		break;
	default:
		glEnable(cap);
		break;
	}
}

void oglDisable(GLenum cap)
{
	switch (cap)
	{
	case GL_TEXTURE_2D:
		if (s_Texture2DEnabled == true)
		{
			glDisable(GL_TEXTURE_2D);
			s_Texture2DEnabled = false;
			s_Texture2DStateCount++;
		}
		break;
	default:
		glDisable(cap);
		break;
	}
}

void resetTexture2DStateCount()
{
	s_Texture2DStateCount = 0;
}

int getTexture2DStateCount()
{
	return s_Texture2DStateCount;
}
void oglBindTexture(GLenum target, GLuint texture)
{
	if (texture != s_texture)
	{
		s_texture = texture;
		glBindTexture(target, texture);
		s_BindTextureCount++;
	}
}

void oglForceBindTexture(GLenum target, GLuint texture)
{
	glBindTexture(target, texture);
	s_texture = texture;
	s_BindTextureCount++;
}

void resetBindTextureCount()
{
	s_BindTextureCount = 0;
}

int getBindTextureCount()
{
	return s_BindTextureCount;
}

void oglEnableClientState(GLenum array)
{
	switch (array)
	{
		case GL_VERTEX_ARRAY:
			assert(s_VERTEX_ARRAY == 0);
			s_VERTEX_ARRAY++;
			break;
		case GL_TEXTURE_COORD_ARRAY:
			assert(s_TEXTURE_COORD_ARRAY == 0);
			s_TEXTURE_COORD_ARRAY++;
			break;
        case GL_COLOR_ARRAY:
            assert(s_COLOR_ARRAY == 0);
            s_COLOR_ARRAY++;
            break;
		default:
			assert(1);
			break;
	}
}

void oglDisableClientState(GLenum array)
{
	switch (array)
	{
		case GL_VERTEX_ARRAY:
			assert(s_VERTEX_ARRAY == 1);
			s_VERTEX_ARRAY--;
			break;
		case GL_TEXTURE_COORD_ARRAY:
			assert(s_TEXTURE_COORD_ARRAY == 1);
			s_TEXTURE_COORD_ARRAY--;
			break;
        case GL_COLOR_ARRAY:
            assert(s_COLOR_ARRAY == 1);
            s_COLOR_ARRAY--;
            break;
		default:
			assert(1);
			break;
	}
}

void oglDrawArrays(GLenum mode, GLint first, GLsizei count)
{
	if (s_VERTEX_ARRAY)
	{
		if (s_VERTEX_ARRAY_enabled == false)
		{
			s_VERTEX_ARRAY_enabled = true;
			s_clientStateCount++;
			glEnableClientState(GL_VERTEX_ARRAY);
		}
	}
	else
	{
		if (s_VERTEX_ARRAY_enabled == true)
		{
			s_VERTEX_ARRAY_enabled = false;
			s_clientStateCount++;
			glDisableClientState(GL_VERTEX_ARRAY);
		}
	}

	if (s_TEXTURE_COORD_ARRAY)
	{
		if (s_TEXTURE_COORD_ARRAY_enabled == false)
		{
			s_TEXTURE_COORD_ARRAY_enabled = true;
			s_clientStateCount++;
			glEnableClientState(GL_TEXTURE_COORD_ARRAY);
		}
	}
	else
	{
		if (s_TEXTURE_COORD_ARRAY_enabled == true)
		{
			s_TEXTURE_COORD_ARRAY_enabled = false;
			s_clientStateCount++;
			glDisableClientState(GL_TEXTURE_COORD_ARRAY);
		}
	}

    if (s_COLOR_ARRAY)
    {
        if (s_COLOR_ARRAY_enabled == false)
        {
            s_COLOR_ARRAY_enabled = true;
            s_clientStateCount++;
            glEnableClientState(GL_COLOR_ARRAY);
        }
    }
    else
    {
        if (s_COLOR_ARRAY_enabled == true)
        {
            s_COLOR_ARRAY_enabled = false;
            s_clientStateCount++;
            glDisableClientState(GL_COLOR_ARRAY);
        }
    }

	glDrawArrays(mode, first, count);
}



void oglDrawElements(GLenum mode, GLsizei count, GLenum type, const GLvoid *indices)
{
	if (s_VERTEX_ARRAY)
	{
		if (s_VERTEX_ARRAY_enabled == false)
		{
			s_VERTEX_ARRAY_enabled = true;
			s_clientStateCount++;
			glEnableClientState(GL_VERTEX_ARRAY);
		}
	}
	else
	{
		if (s_VERTEX_ARRAY_enabled == true)
		{
			s_VERTEX_ARRAY_enabled = false;
			s_clientStateCount++;
			glDisableClientState(GL_VERTEX_ARRAY);
		}
	}

	if (s_TEXTURE_COORD_ARRAY)
	{
		if (s_TEXTURE_COORD_ARRAY_enabled == false)
		{
			s_TEXTURE_COORD_ARRAY_enabled = true;
			s_clientStateCount++;
			glEnableClientState(GL_TEXTURE_COORD_ARRAY);
		}
	}
	else
	{
		if (s_TEXTURE_COORD_ARRAY_enabled == true)
		{
			s_TEXTURE_COORD_ARRAY_enabled = false;
			s_clientStateCount++;
			glDisableClientState(GL_TEXTURE_COORD_ARRAY);
		}
	}

    if (s_COLOR_ARRAY)
    {
        if (s_COLOR_ARRAY_enabled == false)
        {
            s_COLOR_ARRAY_enabled = true;
            s_clientStateCount++;
            glEnableClientState(GL_COLOR_ARRAY);
        }
    }
    else
    {
        if (s_COLOR_ARRAY_enabled == true)
        {
            s_COLOR_ARRAY_enabled = false;
            s_clientStateCount++;
            glDisableClientState(GL_COLOR_ARRAY);
        }
    }

	glDrawElements(mode, count, type, indices);
}


int getVertexArrayCount()
{
	return s_VERTEX_ARRAY;
}

int getTextureCoordArrayCount()
{
	return s_TEXTURE_COORD_ARRAY;
}

void resetClientStateCount()
{
	s_clientStateCount = 0;
}

int getClientStateCount()
{
	return s_clientStateCount;
}
