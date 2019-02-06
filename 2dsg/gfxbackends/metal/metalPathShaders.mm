/*
 * gl2PathShaders.cpp
 *
 *  Created on: 1 sept. 2015
 *      Author: Nico
 */

#include "metalShaders.h"


void pathShadersInit()
{
	const ShaderProgram::ConstantDesc pathUniforms[] = {
			{ "mvp",ShaderProgram::CMATRIX, 1,ShaderProgram::SysConst_WorldViewProjectionMatrix, true, 0, NULL },
			{ "xform",ShaderProgram::CMATRIX, 1,ShaderProgram::SysConst_None, true, 0, NULL },
			{ "fColor", ShaderProgram::CFLOAT4, 1,	ShaderProgram::SysConst_Color, false, 0, NULL },
            { "feather", ShaderProgram::CFLOAT, 1, ShaderProgram::SysConst_None, false, 0, NULL },
			{ "fTexture", ShaderProgram::CTEXTURE, 1, ShaderProgram::SysConst_None, false, 0, NULL },
			{ "", ShaderProgram::CFLOAT, 0, ShaderProgram::SysConst_None,false, 0, NULL } };

	const ShaderProgram::DataDesc pathAttributesFillC[] = {
			{ "data0",ShaderProgram::DFLOAT, 4, 0, 0 },
			{ "", ShaderProgram::DFLOAT, 0, 0, 0 } };

	const ShaderProgram::DataDesc pathAttributesStrokeC[] = {
			{ "data0",ShaderProgram::DFLOAT, 4, 0, 0 },
			{ "data1", ShaderProgram::DFLOAT, 4, 1, 0 },
			{ "data2", ShaderProgram::DFLOAT, 4, 2, 0 },
			{ "", ShaderProgram::DFLOAT, 0, 0, 0 } };

	const ShaderProgram::DataDesc pathAttributesStrokeSL[] = {
			{ "data0",ShaderProgram::DFLOAT, 4, 0, 0 },
			{ "", ShaderProgram::DFLOAT, 0, 0, 0 } };

	ShaderProgram::pathShaderFillC = new metalShaderProgram("gidPathFCV","gidPathFCF", pathUniforms,
			pathAttributesFillC,1,0);
	ShaderProgram::pathShaderStrokeC = new metalShaderProgram("gidPathSCV","gidPathSCF", pathUniforms,
			pathAttributesStrokeC,7,48);
    ShaderProgram::pathShaderStrokeLC = new metalShaderProgram("gidPathSLV","gidPathSLF", pathUniforms,
                                                            pathAttributesStrokeSL,1,0);
}

void pathShadersRelease()
{
	delete ShaderProgram::pathShaderFillC;
	delete ShaderProgram::pathShaderStrokeC;
	delete ShaderProgram::pathShaderStrokeLC;
}
