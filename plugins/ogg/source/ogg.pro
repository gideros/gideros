QT -= core gui

TARGET = ogg
TEMPLATE = lib

INCLUDEPATH += ../../../Sdk/include ../../../Sdk/include/gideros
INCLUDEPATH += ../../../2dsg
INCLUDEPATH += ../../../2dsg/gfxbackends
INCLUDEPATH += ../../../libgideros
INCLUDEPATH += ../../../libgid/include
INCLUDEPATH += ../../../luabinding
#INCLUDEPATH += ../../../lua/src

XIPH_OGG=libogg-1.3.2
XIPH_THEORA=libtheora-1.1.1
XIPH_VORBIS=libvorbis-1.3.5
XIPH_OPUS=libopus-1.3.1

SOGG_F=bitwise framing
STHEORADEC_F=apiwrapper bitpack decapiwrapper decinfo decode dequant fragment huffdec idct th_info internal quant state
SVORBIS_F=mdct block window synthesis info floor1 floor0 res0 mapping0 registry codebook sharedbook envelope psy bitrate lpc lsp smallft vorbisfile
SVORBIS_F+=analysis vorbisenc

SOPUS_C=bands celt celt_encoder celt_decoder cwrs entcode entdec entenc kiss_fft laplace mathops opus_mdct modes pitch celt_lpc quant_bands rate vq arm/armcpu arm/arm_celt_map
SOPUS_S=CNG code_signs init_decoder decode_core decode_frame decode_parameters decode_indices decode_pulses decoder_set_fs \
	dec_API enc_API encode_indices encode_pulses gain_quant interpolate LP_variable_cutoff NLSF_decode NSQ NSQ_del_dec PLC \
	shell_coder tables_gain tables_LTP tables_NLSF_CB_NB_MB tables_NLSF_CB_WB tables_other tables_pitch_lag tables_pulses_per_block VAD \
	control_audio_bandwidth quant_LTP_gains VQ_WMat_EC HP_variable_cutoff NLSF_encode NLSF_VQ NLSF_unpack NLSF_del_dec_quant process_NLSFs \
	stereo_LR_to_MS stereo_MS_to_LR check_control_input control_SNR init_encoder control_codec A2NLSF ana_filt_bank_1 biquad_alt bwexpander_32 bwexpander \
	debug decode_pitch inner_prod_aligned lin2log log2lin LPC_analysis_filter LPC_inv_pred_gain table_LSF_cos NLSF2A NLSF_stabilize NLSF_VQ_weights_laroia \
	pitch_est_tables resampler resampler_down2_3 resampler_down2 resampler_private_AR2 resampler_private_down_FIR resampler_private_IIR_FIR resampler_private_up2_HQ resampler_rom \
	sigm_Q15 sort sum_sqr_shift stereo_decode_pred stereo_encode_pred stereo_find_predictor stereo_quant_pred LPC_fit
SOPUS_SF+= \
	apply_sine_window_FLP corrMatrix_FLP encode_frame_FLP find_LPC_FLP find_LTP_FLP find_pitch_lags_FLP \
	find_pred_coefs_FLP LPC_analysis_filter_FLP LTP_analysis_filter_FLP LTP_scale_ctrl_FLP noise_shape_analysis_FLP process_gains_FLP \
	regularize_correlations_FLP residual_energy_FLP warped_autocorrelation_FLP wrappers_FLP autocorrelation_FLP burg_modified_FLP \
	bwexpander_FLP energy_FLP inner_product_FLP k2a_FLP LPC_inv_pred_gain_FLP pitch_analysis_core_FLP scale_copy_vector_FLP scale_vector_FLP schur_FLP sort_FLP
#SOPUS_S+=$(addsuffix _FIX,$(addprefix fixed/, \
#	apply_sine_window autocorr burg_modified corrMatrix encode_frame find_LPC find_LTP find_pitch_lags find_pred_coefs k2a k2a_Q16 \
#	LTP_analysis_filter LTP_scale_ctrl noise_shape_analysis pitch_analysis_core process_gains regularize_correlations \
#	residual_energy residual_energy16 schur schur64 vector_ops warped_autocorrelation)) 

SOPUS_G=opus_analysis mapping_matrix mlp_data mlp repacketizer opus_decoder opus_encoder opus opus_projection_encoder opus_projection_decoder
SOPUS_G+=opus_multistream opus_multistream_encoder opus_multistream_decoder
for(f,SOPUS_G):SOPUS_F+=opus/src/$${f}
for(f,SOPUS_C):SOPUS_F+=opus/celt/$${f}
for(f,SOPUS_S):SOPUS_F+=opus/silk/$${f}
for(f,SOPUS_SF):SOPUS_F+=opus/silk/float/$${f}


for(f,SOGG_F):SXIPH+=$$XIPH_OGG/src/$${f}.c
for(f,STHEORADEC_F):SXIPH+=$$XIPH_THEORA/lib/$${f}.c
for(f,SVORBIS_F):SXIPH+=$$XIPH_VORBIS/lib/$${f}.c
for(f,SOPUS_F):SXIPH+=$$XIPH_OPUS/$${f}.c

INCLUDEPATH += Common
INCLUDEPATH += $$XIPH_OGG/include
INCLUDEPATH += $$XIPH_THEORA/include
INCLUDEPATH += $$XIPH_VORBIS/include  $$XIPH_VORBIS/lib
INCLUDEPATH += $$XIPH_OPUS/opus/include $$XIPH_OPUS/opus  $$XIPH_OPUS/opus/silk $$XIPH_OPUS/opus/silk/float

SOURCES += Common/oggbinder.cpp \
Common/OggDec.cpp Common/OggDecVorbis.cpp Common/OggDecTheora.cpp Common/OggDecOpus.cpp \
Common/OggEnc.cpp Common/OggEncVorbis.cpp \
    $$SXIPH

HEADERS += 

LIBS += -L"../../../Sdk/lib/desktop" -llua -lgid -lgideros -lgvfs

DEFINES += GID_LIBRARY HAVE_CONFIG_H

macx {
QMAKE_POST_LINK += install_name_tool -change liblua.1.dylib "@executable_path/../Frameworks/liblua.1.dylib" $(TARGET);
QMAKE_POST_LINK += install_name_tool -change libgid.1.dylib "@executable_path/../Frameworks/libgid.1.dylib" $(TARGET);
QMAKE_POST_LINK += install_name_tool -change libgideros.1.dylib "@executable_path/../Frameworks/libgideros.1.dylib" $(TARGET);
QMAKE_POST_LINK += install_name_tool -change libgvfs.1.dylib "@executable_path/../Frameworks/libgvfs.1.dylib" $(TARGET);
}
