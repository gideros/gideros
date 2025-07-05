#include "camerabinder.h"
#include "Shaders.h"
#include "graphicsbase.h"
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <linux/videodev2.h>
#include <jpeglib.h>
#include <jerror.h>
#include <setjmp.h>

#define GL_TEXTURE_EXTERNAL_OES 0x8D65

static const char *VShaderCode = "attribute highp vec3 vVertex;\n"
		"attribute mediump vec2 vTexCoord;\n"
		"uniform highp mat4 vMatrix;\n"
		"uniform highp mat4 tMatrix;\n"
		"varying highp vec2 fTexCoord;\n"
		"\n"
		"void main() {\n"
		"  highp vec4 texc = tMatrix*vec4(vTexCoord,0.0,1.0);\n"
		"  highp vec4 vertex = vec4(vVertex,1.0);\n"
		"  gl_Position = vMatrix*vertex;\n"
		"  fTexCoord=texc.xy;\n"
		"}\n";
static const char *FShaderCode =
		"#extension GL_OES_EGL_image_external : require\n"
				"uniform samplerExternalOES fTexture;\n"
				"varying highp vec2 fTexCoord;\n"
				"void main() {\n"
				" gl_FragColor=texture2D(fTexture, fTexCoord);\n"
				"}\n";

static const ShaderProgram::ConstantDesc camUniforms[] =
		{ { "tMatrix", ShaderProgram::CMATRIX, 1, ShaderProgram::SysConst_None,
		true, 0, NULL },
		 { "rMatrix", ShaderProgram::CMATRIX, 1, ShaderProgram::SysConst_None,
				true, 0, NULL },
		 { "vMatrix", ShaderProgram::CMATRIX, 1,
						ShaderProgram::SysConst_WorldViewProjectionMatrix, true,
						0, NULL }, { "fTexture", ShaderProgram::CTEXTURE, 1,
						ShaderProgram::SysConst_None, false, 0, NULL }, { "",
						ShaderProgram::CFLOAT, 0, ShaderProgram::SysConst_None,
						false, 0, NULL } };
static const ShaderProgram::DataDesc camAttributes[] = { { "vVertex",
		ShaderProgram::DFLOAT, 3, 0, 0 }, { "vColor", ShaderProgram::DUBYTE, 4,
		1, 0 }, { "vTexCoord", ShaderProgram::DFLOAT, 2, 2, 0 }, { "",
		ShaderProgram::DFLOAT, 0, 0, 0 } };

static g_id gid = g_NextId();

void jpegErrorExit ( j_common_ptr cinfo )
{
    char jpegLastErrorMsg[JMSG_LENGTH_MAX];
    /* Create the message */
    ( *( cinfo->err->format_message ) ) ( cinfo, jpegLastErrorMsg );

    /* Jump to the setjmp point */
    throw std::runtime_error( jpegLastErrorMsg ); // or your preffered exception ...
}

static void render_s(int type, void *event, void *udata)
{
	int *b=(int *) event;
	((TextureData *)udata)->id()->updateData(ShaderTexture::FMT_RGB,ShaderTexture::PK_UBYTE,b[0],b[1],(void *)(b+2),ShaderTexture::WRAP_CLAMP,ShaderTexture::FILT_LINEAR);
}

class GCAMERA {
	ShaderBuffer *rdrTgt;
	ShaderProgram *shader;
	TextureData *tex;
	VertexBuffer<unsigned short> indices;
	VertexBuffer<Point2f> vertices;
	VertexBuffer<Point2f> texcoords;
	bool running;
	int fd;
	std::thread thr;
	struct v4l2_capability ccaps;
	size_t buffer_size;
public:
	GCAMERA() {
		running=false;
		fd=-1;

		shader = ShaderEngine::Engine->createShaderProgram(VShaderCode,
				FShaderCode, ShaderProgram::Flag_FromCode, camUniforms,
				camAttributes);
		indices.resize(4);
		vertices.resize(4);
		texcoords.resize(4);

		indices[0] = 0;
		indices[1] = 1;
		indices[2] = 3;
		indices[3] = 2;
		indices.Update();

		texcoords[0] = Point2f(0, 0);
		texcoords[1] = Point2f(1, 0);
		texcoords[2] = Point2f(1, 1);
		texcoords[3] = Point2f(0, 1);
		texcoords.Update();

	}

	~GCAMERA() {
		stop();

		delete shader;
	}

	bool isCameraAvailable() {
		return true;
	}

	std::vector<cameraplugin::CameraDesc> availableDevices()
	{
		std::vector<cameraplugin::CameraDesc> cams;
		int camNum=0;
		while (camNum<64) {
			int fd;
			struct v4l2_capability video_cap;
			char fdName[32];
			
			sprintf(fdName,"/dev/video%d",camNum++);

			if((fd = open(fdName, O_RDWR)) == -1)
				break;

			if(ioctl(fd, VIDIOC_QUERYCAP, &video_cap) == -1)
			{
				close(fd);
				continue;
			}
			else {
				cameraplugin::CameraDesc c;
				c.pos=cameraplugin::CameraDesc::POS_UNKNOWN;
				c.name=fdName;
				c.description=(char *)(video_cap.card);
				if (video_cap.device_caps&V4L2_CAP_VIDEO_CAPTURE)
				{
					if (video_cap.device_caps&V4L2_CAP_STREAMING)
						cams.push_back(c);
				}
			}

			close(fd);
		}
		return cams;
	}

	void stop() {
		if (running)
		{
			running=false;
			thr.join();
			delete rdrTgt;			
		}
		if (fd>=0) {
			close(fd);
			fd=-1;
		}
	}

	bool setFlash(int mode) {
		return false;
	}

	bool takePicture() {
		return false; //TODO
	}

	void setOrientation(int angle) {
	}

	cameraplugin::CameraInfo queryCamera(const char *device, int orientation)
	{
		cameraplugin::CameraInfo c;
		return c;
	}

	void start(TextureData *texture,int orientation,int *camwidth,int *camheight,const char *device,int *picWidth,int *picHeight) {
		tex = texture;
		rdrTgt = ShaderEngine::Engine->createRenderTarget(tex->id());
		vertices[0] = Point2f(0, 0);
		vertices[1] = Point2f(tex->width, 0);
		vertices[2] = Point2f(tex->width, tex->height);
		vertices[3] = Point2f(0, tex->height);
		vertices.Update();
		
		fd = open(device?device:"/dev/video0", O_RDWR);

		struct v4l2_format fmt;
		fmt.type=V4L2_BUF_TYPE_VIDEO_CAPTURE;
		fmt.fmt.pix.width=*picWidth;
		fmt.fmt.pix.height=*picHeight;
		fmt.fmt.pix.pixelformat=V4L2_PIX_FMT_MJPEG;
		fmt.fmt.pix.field=V4L2_FIELD_NONE;
		fmt.fmt.pix.bytesperline=(*picWidth)*4;
		fmt.fmt.pix.priv=0;
		
		if((fd>=0)&&((ioctl(fd, VIDIOC_S_FMT, &fmt) == -1)||(ioctl(fd, VIDIOC_QUERYCAP, &ccaps)==-1)))
		{
			close(fd);
			fd=-1;
		}

		if (fd==-1) { 
			*camwidth=0;
			*camheight=0;
			*picWidth=0;
			*picHeight=0;
			delete rdrTgt;
			return;
		}
		

		*camwidth=fmt.fmt.pix.width;
		*camheight=fmt.fmt.pix.height;
		*picWidth=fmt.fmt.pix.width;
		*picHeight=fmt.fmt.pix.height;
		buffer_size=fmt.fmt.pix.sizeimage;
		printf("Select Format: %dx%d (%08lx)\n",fmt.fmt.pix.width,fmt.fmt.pix.height,fmt.fmt.pix.pixelformat);
		
		if ((*camwidth==0)&&(*camheight==0)) {
			delete rdrTgt;
			return;
		}
		/*
	    int x0=0;
	    int x1=1;
	    if (rvals[3]) { x0=1; x1=0; }
	    switch (rvals[2])
		{
		        case 0:
		            texcoords[0] = Point2f(x0, 0);
		            texcoords[1] = Point2f(x1, 0);
		            texcoords[2] = Point2f(x1, 1);
		            texcoords[3] = Point2f(x0, 1);
		            break;
		        case 90:
		            texcoords[0] = Point2f(x1, 0);
		            texcoords[1] = Point2f(x1, 1);
		            texcoords[2] = Point2f(x0, 1);
		            texcoords[3] = Point2f(x0, 0);
		            break;
		        case 180:
		            texcoords[0] = Point2f(x1, 1);
		            texcoords[1] = Point2f(x0, 1);
		            texcoords[2] = Point2f(x0, 0);
		            texcoords[3] = Point2f(x1, 0);
		            break;
		        case 270:
		            texcoords[0] = Point2f(x0, 1);
		            texcoords[1] = Point2f(x0, 0);
		            texcoords[2] = Point2f(x1, 0);
		            texcoords[3] = Point2f(x1, 1);
		            break;
		}
		*/
		texcoords.Update();

		running=true;
		thr=std::thread(&GCAMERA::loop,this);
	}
	
	struct buffer {
        void   *start;
        size_t  length;
	} *buffers;
	
	void loop() {
		printf("Caps:%08lx\n",ccaps.device_caps);
		
		//Init
		bool valid=false;
		int n_buffers;
		
		struct v4l2_requestbuffers req;
		memset(&req,0,sizeof(req));
		
		if (ccaps.device_caps&V4L2_CAP_STREAMING)		
		{

			req.count = 4;
			req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
			req.memory = V4L2_MEMORY_MMAP;
			
			if (ioctl(fd, VIDIOC_REQBUFS, &req) == -1)
			{
				//try User pointers
				req.memory = V4L2_MEMORY_USERPTR;
				if (ioctl(fd, VIDIOC_REQBUFS, &req) != -1)
					valid=true;
			}
			else
				valid=true;

			if (valid)
			{
				buffers=new buffer[req.count];
				
				for (n_buffers = 0; n_buffers < req.count; ++n_buffers) {
					struct v4l2_buffer buf;
					memset(&buf,0,sizeof(buf));

					buf.type        = V4L2_BUF_TYPE_VIDEO_CAPTURE;
					buf.memory      = req.memory;
					buf.index       = n_buffers;
					
					if (req.memory==V4L2_MEMORY_MMAP) {
						if (ioctl(fd, VIDIOC_QUERYBUF, &buf) == -1)
							break;

						buffers[n_buffers].length = buf.length;
						buffers[n_buffers].start =
								mmap(NULL /* start anywhere */,
									  buf.length,
									  PROT_READ | PROT_WRITE /* required */,
									  MAP_SHARED /* recommended */,
									  fd, buf.m.offset);

						if (MAP_FAILED == buffers[n_buffers].start)
						{
							perror("MMAP");
							break;
						}
						
						ioctl(fd, VIDIOC_QBUF, &buf);
					}
					else
					{
						buffers[n_buffers].length = buffer_size;
						buffers[n_buffers].start = malloc(buffer_size);
						buf.m.userptr = (unsigned long)buffers[n_buffers].start;
						buf.length = buffers[n_buffers].length;

						if (!buffers[n_buffers].start)
							break;
						
						ioctl(fd, VIDIOC_QBUF, &buf);
					}
				}
				enum v4l2_buf_type type;
				type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
				ioctl(fd, VIDIOC_STREAMON, &type);
			}
		}
		
		if (valid) {
			printf("Type:%d nBuffers:%d/%d\n",req.memory,n_buffers,req.count);
		}
		
		while (running) {
			fd_set fds;
			struct timeval tv;
			int r;

			FD_ZERO(&fds);
			FD_SET(fd, &fds);

			/* Timeout. */
			tv.tv_sec = 0;
			tv.tv_usec = 1000000;

			r = select(fd + 1, &fds, NULL, NULL, &tv);
			if (r<=0) continue;
			
			if (valid) {
				struct v4l2_buffer buf;
				memset(&buf,0,sizeof(buf));

				buf.type        = V4L2_BUF_TYPE_VIDEO_CAPTURE;
				buf.memory      = req.memory;
				if (ioctl(fd, VIDIOC_DQBUF, &buf) != -1)
				{
					processImage(buffers[buf.index].start, buf.bytesused);
					ioctl(fd, VIDIOC_QBUF, &buf);
				}				
			}
		}
		
		if (valid) {
			enum v4l2_buf_type type;
			type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
			ioctl(fd, VIDIOC_STREAMOFF, &type);
				
			if (req.memory==V4L2_MEMORY_MMAP) {
				for (int i = 0; i < n_buffers; ++i)
					munmap(buffers[i].start, buffers[i].length);
			}
			else
				for (int i = 0; i < n_buffers; ++i)
					free(buffers[i].start);
			delete[] buffers;
		}
	}

	void nativeEvent(int code, char *data, int size) {
		char *event=(char *)malloc(sizeof(int)+size);
		memcpy(event+sizeof(int),data,size);
		*((int *)event)=size;
		gevent_EnqueueEvent(gid, cameraplugin::callback_s, code, event, 1, NULL);
	}
	
	void processImage(void *img,size_t size) {
		printf("Got %zu bytes\n",size);
	    struct jpeg_decompress_struct cinfo;
		struct jpeg_error_mgr jerr;
		cinfo.err = jpeg_std_error( &jerr );
		jerr.error_exit = jpegErrorExit;
		unsigned char *buf=NULL;

		try {
			jpeg_create_decompress(&cinfo);

			jpeg_mem_src(&cinfo, (unsigned char *) img, size);

			if (jpeg_read_header(&cinfo, TRUE)!=1)
				throw std::runtime_error("Bad header");

			jpeg_start_decompress(&cinfo);
			
			int width = cinfo.output_width;
			int height = cinfo.output_height;
			int pixel_size = cinfo.output_components;


			size_t bmp_size = width * height * pixel_size;
			buf	= (unsigned char*) malloc(bmp_size+sizeof(int)*2);
			((int *)buf)[0]=width;
			((int *)buf)[1]=height;


			if (cinfo.jpeg_color_space == JCS_GRAYSCALE)
				cinfo.out_color_space = JCS_GRAYSCALE;
			else
				cinfo.out_color_space = JCS_RGB;


			while (cinfo.output_scanline < cinfo.output_height)
			{
				JSAMPROW buffer = (JSAMPROW)buf + sizeof(int)*2 + (cinfo.output_scanline * cinfo.output_width * cinfo.output_components);
				jpeg_read_scanlines(&cinfo, &buffer, 1);
			}

			jpeg_finish_decompress(&cinfo);
						
			jpeg_destroy_decompress(&cinfo);

			printf("Proc: Image is %d by %d with %d components", 
				width, height, pixel_size);
			gevent_EnqueueEvent(gid, render_s, 0, buf, 1, tex);
			
		}
		catch ( std::runtime_error & e ) {
			jpeg_destroy_decompress( &cinfo );
		}
	}

	void nativeRender(int camtex, float *mat) {
		if (!running) return;
		ShaderEngine::Engine->reset();
		ShaderBuffer *oldfbo = ShaderEngine::Engine->setFramebuffer(rdrTgt);
		ShaderEngine::Engine->setViewport(0, 0, tex->width, tex->height);
		Matrix4 projection = ShaderEngine::Engine->setOrthoFrustum(0,
				tex->baseWidth, tex->baseHeight, 0, -1, 1, true);
		ShaderEngine::Engine->setProjection(projection);
		Matrix4 model;
		ShaderEngine::Engine->setModel(model);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_EXTERNAL_OES, camtex);

		shader->setConstant(shader->getConstantByName("tMatrix"),
				ShaderProgram::CMATRIX, 1, mat);
		shader->setData(ShaderProgram::DataVertex, ShaderProgram::DFLOAT, 2,
				&vertices[0], vertices.size(), vertices.modified,
				&vertices.bufferCache);
		shader->setData(ShaderProgram::DataTexture, ShaderProgram::DFLOAT, 2,
				&texcoords[0], texcoords.size(), texcoords.modified,
				&texcoords.bufferCache);
		shader->drawElements(ShaderProgram::TriangleStrip, indices.size(),
				ShaderProgram::DUSHORT, &indices[0], indices.modified,
				&indices.bufferCache);
		vertices.modified = false;
		texcoords.modified = false;
		indices.modified = false;

		ShaderEngine::Engine->setFramebuffer(oldfbo);
	}

};

static GCAMERA *s_gcamera = NULL;

void cameraplugin::init() {
	if (!s_gcamera)
		s_gcamera = new GCAMERA;
}

void cameraplugin::deinit() {
	if (s_gcamera)
	{
		s_gcamera->stop();
		delete s_gcamera;
		s_gcamera = NULL;
	}
}
std::vector<cameraplugin::CameraDesc> cameraplugin::availableDevices()
{
	if (s_gcamera)
		return s_gcamera->availableDevices();
	std::vector<cameraplugin::CameraDesc> cams;
	return cams;
}

bool cameraplugin::isAvailable() {
    if (!s_gcamera) return false;
    return s_gcamera->isCameraAvailable();
}

void cameraplugin::start(Orientation orientation,int *camwidth,int *camheight,const char *device,int *picWidth,int *picHeight) {
	int o=0;
	switch (orientation)
	{
	case ePortrait:
	case eFixed:
		o=0;
		break;
	case eLandscapeLeft:
		o=90;
		break;
	case ePortraitUpsideDown:
		o=180;
		break;
	case eLandscapeRight:
		o=270;
		break;
	}
	s_gcamera->start(cameraplugin::cameraTexture->data,o,camwidth,camheight,device,picWidth,picHeight);
}

void cameraplugin::stop() {
	if (s_gcamera)
		s_gcamera->stop();
}

bool cameraplugin::setFlash(int mode) {
	if (s_gcamera)
		return s_gcamera->setFlash(mode);
	return false;
}

bool cameraplugin::takePicture() {
	if (s_gcamera)
		return s_gcamera->takePicture();
	return false;
}

void cameraplugin::setOrientation(Orientation orientation) {
	int o=0;
	switch (orientation)
	{
	case ePortrait:
	case eFixed:
		o=0;
		break;
	case eLandscapeLeft:
		o=90;
		break;
	case ePortraitUpsideDown:
		o=180;
		break;
	case eLandscapeRight:
		o=270;
		break;
	}
	if (s_gcamera) s_gcamera->setOrientation(o);
}

cameraplugin::CameraInfo cameraplugin::queyCamera(const char *device, Orientation orientation)
{
	int o=0;
	switch (orientation)
	{
	case ePortrait:
	case eFixed:
		o=0;
		break;
	case eLandscapeLeft:
		o=90;
		break;
	case ePortraitUpsideDown:
		o=180;
		break;
	case eLandscapeRight:
		o=270;
		break;
	}
	if (s_gcamera)
		return s_gcamera->queryCamera(device,o);
	cameraplugin::CameraInfo dummy;
	return dummy;
}

