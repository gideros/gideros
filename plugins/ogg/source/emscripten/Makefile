TARGET=Ogg$(PART)$(FLAVOUR)
PARTS=Core Vorbis Theora Opus
INCS= ../../../Sdk/include
INCS += ../../../Sdk/include/gideros
INCS += ../../../2dsg
INCS += ../../../2dsg/gfxbackends
INCS += ../../../libgideros
INCS += ../../../libgid/include
INCS += ../../../luabinding
INCS += ../../../lua/src

XIPH_OGG=libogg-1.3.2
XIPH_THEORA=libtheora-1.1.1
XIPH_VORBIS=libvorbis-1.3.5
XIPH_OPUS=libopus-1.3.1

SOGG_F=bitwise framing
STHEORADEC_F=apiwrapper bitpack decapiwrapper decinfo decode dequant fragment huffdec idct th_info internal quant state
SVORBIS_F=mdct block window synthesis info floor1 floor0 res0 mapping0 registry codebook sharedbook envelope psy bitrate lpc lsp smallft vorbisfile
SVORBIS_E+=analysis vorbisenc
SOPUS_C:=bands celt celt_encoder celt_decoder cwrs entcode entdec entenc kiss_fft laplace mathops opus_mdct modes pitch celt_lpc quant_bands rate vq arm/armcpu arm/arm_celt_map
SOPUS_S:=CNG code_signs init_decoder decode_core decode_frame decode_parameters decode_indices decode_pulses decoder_set_fs \
	dec_API enc_API encode_indices encode_pulses gain_quant interpolate LP_variable_cutoff NLSF_decode NSQ NSQ_del_dec PLC \
	shell_coder tables_gain tables_LTP tables_NLSF_CB_NB_MB tables_NLSF_CB_WB tables_other tables_pitch_lag tables_pulses_per_block VAD \
	control_audio_bandwidth quant_LTP_gains VQ_WMat_EC HP_variable_cutoff NLSF_encode NLSF_VQ NLSF_unpack NLSF_del_dec_quant process_NLSFs \
	stereo_LR_to_MS stereo_MS_to_LR check_control_input control_SNR init_encoder control_codec A2NLSF ana_filt_bank_1 biquad_alt bwexpander_32 bwexpander \
	debug decode_pitch inner_prod_aligned lin2log log2lin LPC_analysis_filter LPC_inv_pred_gain table_LSF_cos NLSF2A NLSF_stabilize NLSF_VQ_weights_laroia \
	pitch_est_tables resampler resampler_down2_3 resampler_down2 resampler_private_AR2 resampler_private_down_FIR resampler_private_IIR_FIR resampler_private_up2_HQ resampler_rom \
	sigm_Q15 sort sum_sqr_shift stereo_decode_pred stereo_encode_pred stereo_find_predictor stereo_quant_pred LPC_fit
SOPUS_SF:= \
	apply_sine_window_FLP corrMatrix_FLP encode_frame_FLP find_LPC_FLP find_LTP_FLP find_pitch_lags_FLP \
	find_pred_coefs_FLP LPC_analysis_filter_FLP LTP_analysis_filter_FLP LTP_scale_ctrl_FLP noise_shape_analysis_FLP process_gains_FLP \
	regularize_correlations_FLP residual_energy_FLP warped_autocorrelation_FLP wrappers_FLP autocorrelation_FLP burg_modified_FLP \
	bwexpander_FLP energy_FLP inner_product_FLP k2a_FLP LPC_inv_pred_gain_FLP pitch_analysis_core_FLP scale_copy_vector_FLP scale_vector_FLP schur_FLP sort_FLP
#SOPUS_S+=$(addsuffix _FIX,$(addprefix fixed/, \
#	apply_sine_window autocorr burg_modified corrMatrix encode_frame find_LPC find_LTP find_pitch_lags find_pred_coefs k2a k2a_Q16 \
#	LTP_analysis_filter LTP_scale_ctrl noise_shape_analysis pitch_analysis_core process_gains regularize_correlations \
#	residual_energy residual_energy16 schur schur64 vector_ops warped_autocorrelation)) 

SOPUS_G:=opus_analysis mapping_matrix mlp_data mlp repacketizer opus_decoder opus_encoder opus opus_projection_encoder opus_projection_decoder
SOPUS_G+=opus_multistream opus_multistream_encoder opus_multistream_decoder
SOPUS_F:=$(addprefix opus/src/,$(SOPUS_G)) $(addprefix opus/celt/,$(SOPUS_C)) $(addprefix opus/silk/,$(SOPUS_S)) $(addprefix opus/silk/float/,$(SOPUS_SF))

ifeq ($(PART),Core)
SXIPH+=$(addprefix $(XIPH_OGG)/src/,$(SOGG_F)) $(addprefix Common/,oggbinder OggDec OggEnc)
LOPTS+=@OggCore.exps
endif
ifeq ($(PART),Theora)
SXIPH+=$(addprefix $(XIPH_THEORA)/lib/,$(STHEORADEC_F)) Common/OggDecTheora
endif
ifeq ($(PART),Vorbis)
FLAVOURS=F NE
SXIPH+=$(addprefix $(XIPH_VORBIS)/lib/,$(SVORBIS_F)) Common/OggDecVorbis
ifeq ($(subst NE,,$(FLAVOUR)),$(FLAVOUR))
SXIPH+=$(addprefix $(XIPH_VORBIS)/lib/,$(SVORBIS_E)) Common/OggEncVorbis
endif
endif
ifeq ($(PART),Opus)
#FLAVOURS=F NE
CFLGS+= -DHAVE_CONFIG_H
SXIPH+=$(addprefix $(XIPH_OPUS)/,$(SOPUS_F)) Common/OggDecOpus
ifeq ($(subst NE,,$(FLAVOUR)),$(FLAVOUR))
SXIPH+=$(addprefix $(XIPH_OPUS)/,$(SOPUS_E))
endif
endif

INCS += Common
INCS += $(XIPH_OGG)/include
INCS += $(XIPH_THEORA)/include
INCS += $(XIPH_VORBIS)/include $(XIPH_VORBIS)/lib
INCS += $(XIPH_OPUS)/opus/include $(XIPH_OPUS)/opus  $(XIPH_OPUS)/opus/silk $(XIPH_OPUS)/opus/silk/float

CFLGS+=-DPART_$(PART) -DHAVE_CONFIG_H

SRCS+= $(SXIPH)

ifeq ($(PART),)
allparts: $(addsuffix .part,$(PARTS))

%.part:
	PART=$* $(MAKE) 
endif

include ../../../../emscripten/Plugin.mk
