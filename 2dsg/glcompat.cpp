#include "glcompat.h"

void glOrthof(GLfloat left, GLfloat right, GLfloat bottom, GLfloat top, GLfloat near_val, GLfloat far_val)
{
	glOrtho(left, right, bottom, top, near_val, far_val);
}

#if 0

#include <PVRTDecompress.h>
#include <PVRTResourceFile.h>
#include <PVRTTexture.h>

void glCompressedTexImage2D(GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border, GLsizei imageSize, const GLvoid *data)
{
	bool Do2bitMode;

	if (internalformat == GL_COMPRESSED_RGB_PVRTC_2BPPV1_IMG || internalformat == GL_COMPRESSED_RGBA_PVRTC_2BPPV1_IMG)
		Do2bitMode = true;
	else if (internalformat == GL_COMPRESSED_RGB_PVRTC_4BPPV1_IMG || internalformat == GL_COMPRESSED_RGBA_PVRTC_4BPPV1_IMG)
		Do2bitMode = false;
	else
		return;

	unsigned char* resultImage = (unsigned char*)malloc(width * height * 4);
	PVRTDecompressPVRTC(data, Do2bitMode, width, height, resultImage);

	glTexImage2D(target, level, GL_RGBA, width, height, border, GL_RGBA, GL_UNSIGNED_BYTE, resultImage);

	free(resultImage);
}

#endif
