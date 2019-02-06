/*
 * gl2ShaderEngine.cpp
 *
 *  Created on: 5 juin 2015
 *      Author: Nicolas
 */

#include "gtexture.h"
#include "glog.h"
#include "ogl.h"
#include <sstream>
#include "metalShaders.h"

metalShaderEngine::BlendFactor metalShaderEngine::curSFactor;
metalShaderEngine::BlendFactor metalShaderEngine::curDFactor;

const char *metalShaderEngine::getVersion() {
    return "Metal";
}

void metalShaderEngine::resizeFramebuffer(int width,int height)
{
	devWidth = width;
	devHeight = height;
}


void metalShaderEngine::reset(bool reinit) {
	if (reinit) {
//		s_texture = 0;
//		s_depthEnable = 0;
//		s_depthBufferCleared = false;

		currentBuffer = NULL;
		metalShaderProgram::current = NULL;
		metalShaderProgram::resetAll();
	}

    setFramebuffer(NULL);
	ShaderEngine::reset(reinit);
	metalShaderProgram::resetAllUniforms();
//	s_texture = 0;
//	s_depthEnable = 0;
//	s_depthBufferCleared = false;

    curSFactor=BlendFactor::ONE;
    curDFactor=BlendFactor::ONE_MINUS_SRC_ALPHA;
}

extern void pathShadersInit();
extern void pathShadersRelease();

void metalSetupShaders() {
    glog_i("METAL:%s\n", [[metalDevice name] UTF8String]);
	//glog_i("GLSL_VERSION:%s\n", GLCALL glGetString(GL_SHADING_LANGUAGE_VERSION));

	const ShaderProgram::ConstantDesc stdUniforms[] = { { "vMatrix",
			ShaderProgram::CMATRIX, 1,
			ShaderProgram::SysConst_WorldViewProjectionMatrix, true, 0, NULL },
			{ "fColor", ShaderProgram::CFLOAT4, 1,
					ShaderProgram::SysConst_Color, false, 0, NULL }, {
					"fTexture", ShaderProgram::CTEXTURE, 1,
					ShaderProgram::SysConst_None, false, 0, NULL }, { "",
					ShaderProgram::CFLOAT, 0, ShaderProgram::SysConst_None,
					false, 0, NULL } };
	const ShaderProgram::DataDesc stdAttributes[] = { { "vVertex",
			ShaderProgram::DFLOAT, 2, 0, 0 }, { "vColor", ShaderProgram::DUBYTE,
			4, 1, 0 }, { "vTexCoord", ShaderProgram::DFLOAT, 2, 2, 0 }, { "",
			ShaderProgram::DFLOAT, 0, 0, 0 } };
     ShaderProgram::stdBasic = new metalShaderProgram("gidV","gidF", stdUniforms,
			stdAttributes,1,0);
	ShaderProgram::stdColor = new metalShaderProgram("gidCV","gidCF", stdUniforms,
			stdAttributes,3,0);
	ShaderProgram::stdTexture = new metalShaderProgram("gidTV","gidTF",stdUniforms,
			stdAttributes,5,0);
	ShaderProgram::stdTextureAlpha = new metalShaderProgram("gidTV","gidTAF",stdUniforms,
			stdAttributes,5,0);
	ShaderProgram::stdTextureColor = new metalShaderProgram("gidCTV","gidCTF",stdUniforms,
			stdAttributes,7,0);
	ShaderProgram::stdTextureAlphaColor = new metalShaderProgram("gidCTV","gidCTAF", stdUniforms,
			stdAttributes,7,0);
    const ShaderProgram::DataDesc stdAttributes3[] = { { "vVertex",
        ShaderProgram::DFLOAT, 3, 0, 0 }, { "vColor", ShaderProgram::DUBYTE,
            4, 1, 0 }, { "vTexCoord", ShaderProgram::DFLOAT, 2, 2, 0 }, { "",
                ShaderProgram::DFLOAT, 0, 0, 0 } };
    metalShaderProgram::stdBasic3 = new metalShaderProgram("gidV3","gidF", stdUniforms,
                                                     stdAttributes3,1,0);
    metalShaderProgram::stdColor3 = new metalShaderProgram("gidCV3","gidCF", stdUniforms,
                                                     stdAttributes3,3,0);
    metalShaderProgram::stdTexture3 = new metalShaderProgram("gidTV3","gidTF",stdUniforms,
                                                       stdAttributes3,5,0);
    metalShaderProgram::stdTextureColor3 = new metalShaderProgram("gidCTV3","gidCTF",stdUniforms,
                                                            stdAttributes3,7,0);
	const ShaderProgram::ConstantDesc stdPUniforms[] = {
        { "vMatrix",ShaderProgram::CMATRIX,1,ShaderProgram::SysConst_WorldViewProjectionMatrix,true,0, NULL },
        { "vWorldMatrix", ShaderProgram::CMATRIX, 1,ShaderProgram::SysConst_WorldMatrix, true, 0, NULL },
        { "fColor", ShaderProgram::CFLOAT4, 1, ShaderProgram::SysConst_Color, false, 0, NULL },
        { "fTexInfo", ShaderProgram::CFLOAT4, 1, ShaderProgram::SysConst_TextureInfo, false, 0, NULL },
        { "vPSize", ShaderProgram::CFLOAT, 1, ShaderProgram::SysConst_ParticleSize, true, 0, NULL },
        { "fTexture", ShaderProgram::CTEXTURE, 1, ShaderProgram::SysConst_None, false, 0, NULL },
        { "", ShaderProgram::CFLOAT, 0, ShaderProgram::SysConst_None,false, 0, NULL } };
    ShaderProgram::stdParticle = new metalShaderProgram("gidPV","gidPF", stdPUniforms,
                                                        stdAttributes,3,0);
    metalShaderProgram::stdParticleT = new metalShaderProgram("gidPV","gidPTF", stdPUniforms,
                                                        stdAttributes,3,0);

	const ShaderProgram::ConstantDesc stdPSConstants[] = {
		{ "vMatrix",ShaderProgram::CMATRIX,1,ShaderProgram::SysConst_WorldViewProjectionMatrix,true,0,NULL },
		{ "vWorldMatrix",ShaderProgram::CMATRIX,1,ShaderProgram::SysConst_WorldMatrix,true,0,NULL },
        { "fColor", ShaderProgram::CFLOAT4, 1,ShaderProgram::SysConst_Color, false, 0, NULL },
        { "fTexInfo",ShaderProgram::CFLOAT4,1,ShaderProgram::SysConst_TextureInfo,false,0,NULL },
		{ "fTexture",ShaderProgram::CTEXTURE,1,ShaderProgram::SysConst_None,false,0,NULL },
		{ "",ShaderProgram::CFLOAT,0,ShaderProgram::SysConst_None,false,0,NULL }
	};
	const ShaderProgram::DataDesc stdPSAttributes[] = {
		{ "vVertex", ShaderProgram::DFLOAT, 4, 0, 0 },
		{ "vColor", ShaderProgram::DUBYTE, 4, 1, 0 },
		{ "vTexCoord", ShaderProgram::DFLOAT, 2, 2, 0 },
		{ "",ShaderProgram::DFLOAT,0,0,0 }
	};

    ShaderProgram::stdParticles = new metalShaderProgram(
                                                         "gidPSV","gidPSF",stdPSConstants, stdPSAttributes,7,0);
    metalShaderProgram::stdParticlesT = new metalShaderProgram(
                                                         "gidPSV","gidPSTF",stdPSConstants, stdPSAttributes,7,0);
}

ShaderProgram *metalShaderEngine::createShaderProgram(const char *vshader,
		const char *pshader, int flags,
		const ShaderProgram::ConstantDesc *uniforms,
		const ShaderProgram::DataDesc *attributes) {
	return new metalShaderProgram(vshader, pshader, flags, uniforms, attributes);
}

metalShaderEngine::metalShaderEngine(int sw, int sh) {
	devWidth = sw;
	devHeight = sh;
    
    mrce=nil;
    mcq=[metalDevice newCommandQueue];
    [mcq retain];
    mcb=[mcq commandBuffer];
    [mcb retain];
    
    MTLSamplerDescriptor *sd=[MTLSamplerDescriptor new];
    sd.minFilter=MTLSamplerMinMagFilterNearest;
    sd.magFilter=MTLSamplerMinMagFilterNearest;
    tsNC=[metalDevice newSamplerStateWithDescriptor:sd];
    sd.minFilter=MTLSamplerMinMagFilterLinear;
    sd.magFilter=MTLSamplerMinMagFilterLinear;
    tsFC=[metalDevice newSamplerStateWithDescriptor:sd];
    sd.sAddressMode=MTLSamplerAddressModeRepeat;
    sd.tAddressMode=MTLSamplerAddressModeRepeat;
    sd.rAddressMode=MTLSamplerAddressModeRepeat;
    tsFR=[metalDevice newSamplerStateWithDescriptor:sd];
    sd.minFilter=MTLSamplerMinMagFilterNearest;
    sd.magFilter=MTLSamplerMinMagFilterNearest;
    tsNR=[metalDevice newSamplerStateWithDescriptor:sd];
    [tsNC retain];
    [tsFC retain];
    [tsNR retain];
    [tsFR retain];

    metalSetupShaders();
    pathShadersInit();

	reset(true);
}

metalShaderEngine::~metalShaderEngine() {
	if (currentBuffer)
		setFramebuffer(NULL);
    
    if (mrce) {
        [mrce endEncoding];
        [mrce release];
        mrce=nil;
    }


    [mcb commit];
    [mcb waitUntilCompleted];
    [mcb release];
    [mcq release];
    
    [tsNC release];
    [tsFC release];
    [tsNR release];
    [tsFR release];
	delete ShaderProgram::stdBasic;
	delete ShaderProgram::stdColor;
	delete ShaderProgram::stdTexture;
	delete ShaderProgram::stdTextureAlpha;
	delete ShaderProgram::stdTextureColor;
    delete ShaderProgram::stdParticle;
    delete ShaderProgram::stdParticles;
    delete metalShaderProgram::stdParticleT;
    delete metalShaderProgram::stdParticlesT;
    delete metalShaderProgram::stdBasic3;
    delete metalShaderProgram::stdColor3;
    delete metalShaderProgram::stdTexture3;
    delete metalShaderProgram::stdTextureColor3;
	pathShadersRelease();
}

ShaderTexture *metalShaderEngine::createTexture(ShaderTexture::Format format,
		ShaderTexture::Packing packing, int width, int height, const void *data,
		ShaderTexture::Wrap wrap, ShaderTexture::Filtering filtering, bool forRT) {
	return new metalShaderTexture(format, packing, width, height, data, wrap,
			filtering, forRT);
}

ShaderBuffer *metalShaderEngine::createRenderTarget(ShaderTexture *texture) {
	return new metalShaderBuffer(texture);
}

void metalShaderEngine::clear(int f) {
    int creq=clearReq;
    if (currentBuffer)
        creq=((metalShaderBuffer *)currentBuffer)->clearReq;
    if ((creq&f)==f) return;
    if (currentBuffer)
        ((metalShaderBuffer *)currentBuffer)->clearReq|=f;
    else
        clearReq|=f;
    if (mrce) {
        [mrce endEncoding];
        [mrce release];
        mrce=nil;
    }
}

id<MTLRenderCommandEncoder> metalShaderEngine::encoder()
{
    if (mrce==nil) {
        int creq=clearReq;
        MTLRenderPassDescriptor *rp=metalFramebuffer;
        if (currentBuffer)
        {
            rp=((metalShaderBuffer *)currentBuffer)->mrpd;
            creq=((metalShaderBuffer *)currentBuffer)->clearReq;
            ((metalShaderBuffer *)currentBuffer)->clearReq=0;
        }
        else
            clearReq=0;
        rp.colorAttachments[0].loadAction=(creq&1)?MTLLoadActionClear:MTLLoadActionLoad;
        rp.depthAttachment.loadAction=(creq&2)?MTLLoadActionClear:MTLLoadActionLoad;
        rp.stencilAttachment.loadAction=(creq&4)?MTLLoadActionClear:MTLLoadActionLoad;
        mrce=[mcb renderCommandEncoderWithDescriptor:rp];
        [mrce retain];
        metalShaderProgram::resetAll();
    }
    return mrce;
}

MTLRenderPassDescriptor *metalShaderEngine::pass()
{
    if (currentBuffer)
        return ((metalShaderBuffer *)currentBuffer)->mrpd;
    else
        return metalFramebuffer;
}

ShaderBuffer *metalShaderEngine::setFramebuffer(ShaderBuffer *fbo) {
    ShaderBuffer *previous = currentBuffer;
    if (currentBuffer!=fbo) {
        if (mrce) {
            [mrce endEncoding];
            [mrce release];
            mrce=nil;
        }
        [mcb commit];
        if (previous) {
            [mcb waitUntilCompleted];
            previous->unbound();
        }
        [mcb release];
        mcb=[mcq commandBuffer];
        [mcb retain];
        currentBuffer = fbo;
    }
	return previous;
}

void metalShaderEngine::present(id<MTLDrawable> drawable)
{
    if (mrce!=nil) {
        [mrce endEncoding];
        [mrce release];
        mrce=nil;
    }
    [mcb presentDrawable:drawable];
    ShaderBuffer *previous = currentBuffer;
    [mcb commit];
    if (previous) {
        [mcb waitUntilCompleted];
        previous->unbound();
    }
    [mcb release];
}

void metalShaderEngine::newFrame() {
     mcb=[mcq commandBuffer];
    [mcb retain];
    currentBuffer = NULL;
}

extern "C" void metalShaderEnginePresent(id<MTLDrawable> drawable)
{
    metalShaderEngine *e=(metalShaderEngine* ) ShaderEngine::Engine;
    if (e)
        e->present(drawable);
}

extern "C" void metalShaderNewFrame()
{
    metalShaderEngine *e=(metalShaderEngine* ) ShaderEngine::Engine;
    if (e)
        e->newFrame();
}

void metalShaderEngine::setViewport(int x, int y, int width, int height) {
    //Ensure we have a command encoder
    MTLViewport vp;
    vp.originX=x;
    vp.originY=y;
    vp.width=width;
    vp.height=height;
    vp.znear=0.0;
    vp.zfar=1.0;
    [encoder() setViewport:vp];
}

void metalShaderEngine::setModel(const Matrix4 m) {
	ShaderEngine::setModel(m);
}

void metalShaderEngine::setProjection(const Matrix4 p) {
	ShaderEngine::setProjection(p);
}

void metalShaderEngine::adjustViewportProjection(Matrix4 &vp, float width, float height) {
	//vp.scale(1, -1, 1);
	//vp.translate(0, height, 0);
}

static MTLStencilOperation stencilopToMetal(ShaderEngine::StencilOp sf)
{
    switch (sf)
    {
        case ShaderEngine::STENCIL_KEEP: return MTLStencilOperationKeep;
        case ShaderEngine::STENCIL_ZERO: return MTLStencilOperationZero;
        case ShaderEngine::STENCIL_REPLACE: return MTLStencilOperationReplace;
        case ShaderEngine::STENCIL_INCR: return MTLStencilOperationIncrementClamp;
        case ShaderEngine::STENCIL_INCR_WRAP: return MTLStencilOperationIncrementWrap;
        case ShaderEngine::STENCIL_DECR: return MTLStencilOperationDecrementClamp;
        case ShaderEngine::STENCIL_DECR_WRAP: return MTLStencilOperationDecrementWrap;
        case ShaderEngine::STENCIL_INVERT: return MTLStencilOperationInvert;
    }
    return MTLStencilOperationKeep;
}

static MTLCompareFunction stencilfuncToMetal(ShaderEngine::StencilFunc sf)
{
    switch (sf)
    {
        case ShaderEngine::STENCIL_LESS: return MTLCompareFunctionLess;
        case ShaderEngine::STENCIL_EQUAL: return MTLCompareFunctionEqual;
        case ShaderEngine::STENCIL_GREATER: return MTLCompareFunctionGreater;
        case ShaderEngine::STENCIL_LEQUAL: return MTLCompareFunctionLessEqual;
        case ShaderEngine::STENCIL_GEQUAL: return MTLCompareFunctionGreaterEqual;
        case ShaderEngine::STENCIL_ALWAYS: return MTLCompareFunctionAlways;
        case ShaderEngine::STENCIL_NEVER: return MTLCompareFunctionNever;
        case ShaderEngine::STENCIL_NOTEQUAL: return MTLCompareFunctionNotEqual;
    default:
        break;
    }
    return MTLCompareFunctionAlways;
}

void metalShaderEngine::setDepthStencil(DepthStencil state)
{
	dsCurrent=state;
    if (currentBuffer&&(state.dTest||(state.sFunc!=ShaderEngine::STENCIL_DISABLE)))
    currentBuffer->needDepthStencil();
    clear((state.dClear?2:0)|(state.sClear?4:0));
    MTLDepthStencilDescriptor *mdsd=[MTLDepthStencilDescriptor new];
    mdsd.depthWriteEnabled=state.dTest;
    mdsd.depthCompareFunction=state.dTest?MTLCompareFunctionLess:MTLCompareFunctionAlways;
    mdsd.frontFaceStencil.stencilCompareFunction=stencilfuncToMetal(state.sFunc);
    mdsd.backFaceStencil.stencilCompareFunction=stencilfuncToMetal(state.sFunc);
    mdsd.frontFaceStencil.stencilFailureOperation=stencilopToMetal(state.sFail);
    mdsd.backFaceStencil.stencilFailureOperation=stencilopToMetal(state.sFail);
    mdsd.frontFaceStencil.depthStencilPassOperation=stencilopToMetal(state.dPass);
    mdsd.backFaceStencil.depthStencilPassOperation=stencilopToMetal(state.dPass);
    mdsd.frontFaceStencil.depthFailureOperation=stencilopToMetal(state.dFail);
    mdsd.backFaceStencil.depthFailureOperation=stencilopToMetal(state.dFail);
    mdsd.frontFaceStencil.readMask=state.sMask;
    mdsd.backFaceStencil.readMask=state.sMask;
    mdsd.frontFaceStencil.writeMask=state.sWMask;
    mdsd.backFaceStencil.writeMask=state.sWMask;
    id<MTLDepthStencilState> mds=[metalDevice newDepthStencilStateWithDescriptor:mdsd];
    [mdsd release];
    [encoder() setDepthStencilState:mds];
    [encoder() setStencilReferenceValue:state.sRef];
    //[encoder() setStencilStoreAction:state.sFunc==(ShaderEngine::STENCIL_DISABLE)?MTLStoreActionDontCare: MTLStoreActionStore];    
}

void metalShaderEngine::clearColor(float r, float g, float b, float a) {
    //Needs to be done before any other command, incl. viewport
    pass().colorAttachments[0].clearColor=MTLClearColorMake(r,g,b,a);
    clear(1);
}

void metalShaderEngine::bindTexture(int num, ShaderTexture *texture) {
    id<MTLSamplerState> smp=tsNC;
    if (((metalShaderTexture *)texture)->filter==ShaderTexture::FILT_LINEAR) {
        smp=(((metalShaderTexture *)texture)->wrap==ShaderTexture::WRAP_REPEAT)?tsFR:tsFC;
    }
    else if (((metalShaderTexture *)texture)->wrap==ShaderTexture::WRAP_REPEAT)
        smp=tsNR;
    [encoder() setFragmentTexture:((metalShaderTexture *)texture)->mtex atIndex:num];
    [encoder() setFragmentSamplerState:smp atIndex:num];
}

void metalShaderEngine::setClip(int x, int y, int w, int h) {
    MTLScissorRect sr;
    if ((w < 0) || (h < 0)) {
        sr.x=0;
        sr.y=0;
        sr.width=pass().colorAttachments[0].texture.width;
        sr.height=pass().colorAttachments[0].texture.height;
    }
	else {
        sr.x=x;
        sr.y=y;
        sr.width=w;
        sr.height=h;
	}
    [encoder() setScissorRect:sr];
}

void metalShaderEngine::setBlendFunc(BlendFactor sfactor, BlendFactor dfactor) {
    curSFactor=sfactor;
    curDFactor=dfactor;
}

void metalShaderEngine::getProperties(std::map<std::string,std::string> &props)
{
	props["shader_compiler"]="1";
    props["renderer"]=[[metalDevice name] UTF8String];
}

ShaderEngine *createMetalShaderEngine(int sw,int sh) {
    if (metalDevice)
        return new metalShaderEngine(sw,sh);
    return NULL;
}

ShaderProgram *metalShaderEngine::getDefault(StandardProgram id,int variant)
{
    if (variant==STDPV_TEXTURED)
        switch (id) {
            case STDP_PARTICLE: return metalShaderProgram::stdParticleT;
            case STDP_PARTICLES: return metalShaderProgram::stdParticlesT;
            default: break;
        }
    if (variant==STDPV_3D)
        switch (id) {
            case STDP_BASIC: return metalShaderProgram::stdBasic3;
            case STDP_COLOR: return metalShaderProgram::stdColor3;
            case STDP_TEXTURE: return metalShaderProgram::stdTexture3;
            case STDP_TEXTURECOLOR: return metalShaderProgram::stdTextureColor3;
            default: break;
        }
    return ShaderEngine::getDefault(id,variant);
}
