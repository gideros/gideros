/*
 * OggEncVorbis.cpp
 *
 *  Created on: 20 sept. 2019
 *      Author: Nico
 */

#include "OggEncVorbis.h"
#include <math.h>

OggEncVorbis::OggEncVorbis() {
	/* init supporting Vorbis structures needed in header parsing */
	vorbis_info_init(&vi);
	vorbis_comment_init(&vc);
	encodeInited=false;
	stereo=false;
}

OggEncVorbis::~OggEncVorbis() {
	if (encodeInited) {
		vorbis_block_clear(&vb);
		vorbis_dsp_clear(&vd);
	}
	vorbis_comment_clear(&vc);
	vorbis_info_clear(&vi);
}

bool OggEncVorbis::PacketOut(ogg_packet *op)
{
	if (vorbis_bitrate_flushpacket(&vd, op))
		return true;

	if (vorbis_analysis_blockout(&vd, &vb) == 1) {
		/* analysis, assume we want to use bitrate management */
		vorbis_analysis(&vb, NULL);
		vorbis_bitrate_addblock(&vb);
		if (vorbis_bitrate_flushpacket(&vd, op))
			return true;
	}
	return false;
}

bool OggEncVorbis::InitAudio(unsigned int channels, unsigned int rate, float quality, ogg_stream_state *os)
{
	int ret=vorbis_encode_init_vbr(&vi, channels, rate,quality);
	if (ret) return false;
	stereo=(channels==2);

	/* add a comment */
	vorbis_comment_add_tag(&vc, "ENCODER", "Gideros");

	/* set up the analysis state and auxiliary encoding storage */
	vorbis_analysis_init(&vd, &vi);
	vorbis_block_init(&vd, &vb);
	encodeInited=true;

	/* Vorbis streams begin with three headers; the initial header (with
	 most of the codec setup parameters) which is mandated by the Ogg
	 bitstream spec.  The second header holds any comment fields.  The
	 third header holds the bitstream codebook.  We merely need to
	 make the headers, then pass them to libvorbis one at a time;
	 libvorbis handles the additional Ogg bitstream constraints */

	ogg_packet header;
	ogg_packet header_comm;
	ogg_packet header_code;

	vorbis_analysis_headerout(&vd, &vc, &header,
			&header_comm, &header_code);
	ogg_stream_packetin(os, &header); /* automatically placed in its own page */
	ogg_stream_packetin(os, &header_comm);
	ogg_stream_packetin(os, &header_code);
	return true;
}

void OggEncVorbis::WriteAudio(void *buffer,size_t size) {
	if (size == 0) {
		/* end of file.  this can be done implicitly in the mainline,
		 but it's easier to see here in non-clever fashion.
		 Tell the library we're at end of stream so that it can handle
		 the last frame and mark end of stream in the output properly */
		vorbis_analysis_wrote(&vd, 0);

	} else {
		/* data to encode */
		signed char *readbuffer = (signed char *) buffer;
		size_t i;
		/* expose the buffer to submit data */
		float **buffer = vorbis_analysis_buffer(&vd,
				size / (stereo?4:2));

		/* uninterleave samples */
		if (stereo) {
			for (i = 0; i < size / 4; i++) {
				buffer[0][i] = ((readbuffer[i * 4 + 1] << 8)
						| (0x00ff & (int) readbuffer[i * 4])) / 32768.f;
				buffer[1][i] = ((readbuffer[i * 4 + 3] << 8)
						| (0x00ff & (int) readbuffer[i * 4 + 2])) / 32768.f;
			}
		} else {
			for (i = 0; i < size/2 ; i++) {
				buffer[0][i] = ((readbuffer[i * 2 + 1] << 8)
						| (0x00ff & (int) readbuffer[i * 2])) / 32768.f;
			}
		}

		/* tell the library how much we actually submitted */
		vorbis_analysis_wrote(&vd, i);
	}
}
