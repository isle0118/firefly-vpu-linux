#ifndef VPU_API_PRIVATE_H_
#define VPU_API_PRIVATE_H_

#include "vpu_global.h"
#include "vpu_api.h"
#include <sys/types.h>
#include <stdlib.h>
#include <stdio.h>

#define MAX_STREAM_LENGHT  1024*1024  //for encoder out put stream
#define DEFAULT_FRAME_BUFFER_DEPTH 4

/* interlace filed mode support */
typedef enum {
    DEINTER_INPUT_EOS_REACHED   = 0x01,
}DEINTER_FLAG_MAP;

typedef enum DEINTER_CMD {
    DEINT_GET_PRE_PROCESS_COUNT,
    DEINT_GET_FRAME_COUNT,
    DEINT_SET_INPUT_EOS_REACH,
} DEINTER_CMD;

typedef enum DEINTER_STATUS_TYPE {
    DEINTER_NOT_INIT            = -1,
    DEINTER_OK                  = 0,
    DEINTER_INIT_FAIL           = 1,
    DEINTER_ERR_FATAL_THREAD    = 2,
    DEINTER_ERR_OPEN_DEV        = 3,
    DEINTER_ERR_TEST_DEV        = 4,
    DEINTER_ERR_LIST_PRE_DEINT  = 5,
    DEINTER_ERR_ERR_LIST_STREAM = 6,

    DEINTER_ERR_UNKNOW          = 100,
    DEINTER_BUTT,
}DEINTER_STATUS_TYPE;

typedef struct DeinterContext {
    VPU_FRAME  deintBuf;
    VPU_FRAME  originBuf;
    int32_t    poll_flag;
}DeinterContext_t;

typedef enum {
    NEED_FLUSH_DEC_DPB          = 0x01,
    ENABLE_DEINTERLACE_SUPPORT  = 0x02,
    INTERLACE_FIELD_MODE        = 0x04,

}VPU_API_PRIVATE_FLAG_MAP;

// trick: put vpu_mem at head of node and make stream destroy simple
typedef struct {
    uint8_t* buf;
    uint32_t size;
    int64_t timestamp;
    int32_t usePts;
} stream_packet;

typedef enum VPU_CODEC_CMD {
    VPU_GET_STREAM_COUNT,
    VPU_GET_FRAME_COUNT,
} VPU_CODEC_CMD;

typedef struct Stream_describe
{
    uint8_t                 flag[4];
    uint32_t                size;
    uint32_t                timel;
    uint32_t                timeh;
    uint32_t                type;
    uint32_t                slices;
    uint32_t                retFlag;
    uint32_t                res[1];
    uint32_t                slicesize[128];
}Stream_describe;

/* On2 decoder Input structure */
typedef struct On2DecInput
{
    uint8_t *pStream;               /* Pointer to the input */
    uint32_t streamBusAddress;      /* DMA bus address of the input stream */
    uint32_t dataLen;               /* Number of bytes to be decoded         */
    uint32_t picId;                 /* Identifier for the picture to be decoded */
    uint32_t skipNonReference;      /* Flag to enable decoder skip non-reference
                                    * frames to reduce processor load */
    int64_t  timeUs;
    int32_t usePts;
}On2DecInput_t;

/* On2 decoder Output structure */
typedef struct On2DecOutput
{
    uint32_t dataLeft;              /* how many bytes left undecoded */
}On2DecOutput_t;

typedef struct On2M2vTimeAmend {

    int32_t  count;
    int64_t  frameDelayUs;
    int64_t  originTimeUs;
    int64_t  lastTimeUs;
    bool     needReset;
}On2M2vTimeAmend_t;

typedef struct Mpeg2HwStatus {
    int32_t  errFrmNum;
    int32_t  tolerance;
}Mpeg2HwStatus_t;

typedef struct H264HwStatus {
    int32_t  errFrmNum;
    int32_t  tolerance;
}H264HwStatus_t;


typedef struct On2DecExt
{
    int32_t headSend;
    Mpeg2HwStatus_t m2vHwStatus;
    H264HwStatus_t  h264HwStatus;
}On2DecExt_t;

typedef enum CODEC_CFG_VALUE {
    CFG_VALUE_UNKNOW,
    CFG_VALUE_SUPPORT,
    CFG_VALUE_NOT_SUPPORT
} CODEC_CFG_VALUE;

typedef struct On2CodecConfigure {
    CODEC_TYPE codecType;
    OMX_DEC_VIDEO_CODINGTYPE videoCoding;
    uint8_t old_cfg[10];
    const char* cfg;
    CODEC_CFG_VALUE value;
}On2CodecConfigure_t;

typedef struct CfgFile {
    FILE*       file;
    uint32_t    file_size;
    uint32_t    read_pos;
}CfgFile_t;

typedef struct VpuApiCodecCfg {
    CfgFile_t   cfg_file;
    uint8_t*    buf;
}VpuApiCodecCfg_t;

#ifdef __cplusplus
extern "C"
{
#endif

extern void*         get_class_On2AvcDecoder(void);
extern int          init_class_On2AvcDecoder(void * AvcDecoder, int32_t tsFlag);
extern void      destroy_class_On2AvcDecoder(void * AvcDecoder);
extern int        deinit_class_On2AvcDecoder(void * AvcDecoder);
extern int  dec_oneframe_class_On2AvcDecoder(void * AvcDecoder, uint8_t* aOutBuffer, uint32_t *aOutputLength, uint8_t* aInputBuf, uint32_t* aInBufSize, int64_t *InputTimestamp);
extern int reset_class_On2AvcDecoder(void * AvcDecoder);
extern int flush_oneframe_in_dpb_class_On2AvcDecoder(void *decoder, uint8_t* aOutBuffer, uint32_t *aOutputLength);

extern void*         get_class_On2M4vDecoder(void);
extern void      destroy_class_On2M4vDecoder(void * M4vDecoder);
extern int          init_class_On2M4vDecoder(void * M4vDecoder, VPU_GENERIC *vpug);
extern int        deinit_class_On2M4vDecoder(void * M4vDecoder);
extern int         reset_class_On2M4vDecoder(void * M4vDecoder);
extern int  dec_oneframe_class_On2M4vDecoder(void * M4vDecoder,uint8_t* aOutBuffer, uint32_t *aOutputLength, uint8_t* aInputBuf, uint32_t* aInBufSize);
extern int  flush_oneframe_in_dpb_class_On2M4vDecoder(void *decoder, uint8_t* aOutBuffer, uint32_t *aOutputLength);

extern void*        get_class_On2H263Decoder(void);
extern void     destroy_class_On2H263Decoder(void * H263Decoder);
extern int         init_class_On2H263Decoder(void * H263Decoder, VPU_GENERIC *vpug);
extern int       deinit_class_On2H263Decoder(void * H263Decoder);
extern int        reset_class_On2H263Decoder(void * H263Decoder);
extern int dec_oneframe_class_On2H263Decoder(void * H263Decoder,uint8_t* aOutBuffer, uint32_t *aOutputLength, uint8_t* aInputBuf, uint32_t* aInBufSize);
extern int  flush_oneframe_in_dpb_class_On2H263Decoder(void *decoder, uint8_t* aOutBuffer, uint32_t *aOutputLength);

extern void*         get_class_On2M2vDecoder(void);
extern void      destroy_class_On2M2vDecoder(void * M2vDecoder);
extern int          init_class_On2M2vDecoder(void * M2vDecoder);
extern int        deinit_class_On2M2vDecoder(void * M2vDecoder);
extern int         reset_class_On2M2vDecoder(void * M2vDecoder);
extern int  dec_oneframe_class_On2M2vDecoder(void * M2vDecoder,uint8_t* aOutBuffer, uint32_t *aOutputLength, uint8_t* aInputBuf, uint32_t* aInBufSize);
extern int  get_oneframe_class_On2M2vDecoder(void * M2vDecoder, uint8_t* aOutBuffer, uint32_t* aOutputLength);

extern void*          get_class_On2RvDecoder(void);
extern void       destroy_class_On2RvDecoder(void * RvDecoder);
extern int           init_class_On2RvDecoder(void * RvDecoder);
extern int         deinit_class_On2RvDecoder(void * RvDecoder);
extern int          reset_class_On2RvDecoder(void * RvDecoder);
extern int   dec_oneframe_class_On2RvDecoder(void * RvDecoder,uint8_t* aOutBuffer, uint32_t *aOutputLength, uint8_t* aInputBuf, uint32_t* aInBufSize);
extern void         set_width_Height_class_On2RvDecoder(void * RvDecoder, uint32_t* width, uint32_t* height);

extern void*         get_class_On2Vp8Decoder(void);
extern void      destroy_class_On2Vp8Decoder(void * Vp8Decoder);
extern int          init_class_On2Vp8Decoder(void * Vp8Decoder);
extern int        deinit_class_On2Vp8Decoder(void * Vp8Decoder);
extern int         reset_class_On2Vp8Decoder(void * Vp8Decoder);
extern int  dec_oneframe_class_On2Vp8Decoder(void * Vp8Decoder,uint8_t* aOutBuffer, uint32_t *aOutputLength, uint8_t* aInputBuf, uint32_t* aInBufSize);

extern void*         get_class_On2Vc1Decoder(void);
extern void      destroy_class_On2Vc1Decoder(void * Vc1Decoder);
extern int          init_class_On2Vc1Decoder(void * Vc1Decoder, uint8_t* tmpStrm, uint32_t size,uint32_t extraDataSize);
extern int        deinit_class_On2Vc1Decoder(void * Vc1Decoder);
extern int         reset_class_On2Vc1Decoder(void * Vc1Decoder);
extern int  dec_oneframe_class_On2Vc1Decoder(void * Vc1Decoder, uint8_t* aOutBuffer, uint32_t *aOutputLength, uint8_t* aInputBuf, uint32_t* aInBufSize, int64_t *InputTimestamp);

extern void*         get_class_On2Vp6Decoder(void);
extern void      destroy_class_On2Vp6Decoder(void * Vp6Decoder);
extern int          init_class_On2Vp6Decoder(void * Vp6Decoder, int codecid);
extern int        deinit_class_On2Vp6Decoder(void * Vp6Decoder);
extern int         reset_class_On2Vp6Decoder(void * Vp6Decoder);
extern int  dec_oneframe_class_On2Vp6Decoder(void * Vp6Decoder, uint8_t* aOutBuffer, uint32_t *aOutputLength, uint8_t* aInputBuf, uint32_t* aInBufSize);

extern void*         get_class_On2JpegDecoder(void);
extern void      destroy_class_On2JpegDecoder(void * JpegDecoder);
extern int          init_class_On2JpegDecoder(void * JpegDecoder);
extern int        deinit_class_On2JpegDecoder(void * JpegDecoder);
extern int         reset_class_On2JpegDecoder(void * JpegDecoder);
extern int  dec_oneframe_class_On2JpegDecoder(void * JpegDecoder, unsigned char* aOutBuffer, unsigned int *aOutputLength, unsigned char* aInputBuf, unsigned int* aInBufSize);

extern void* get_class_On2AvcEncoder(void);
extern void  destroy_class_On2AvcEncoder(void * AvcEncoder);
extern int init_class_On2AvcEncoder(void * AvcEncoder, EncParameter_t *aEncOption, uint8_t* aOutBuffer,uint32_t * aOutputLength);
extern int deinit_class_On2AvcEncoder(void * AvcEncoder);
extern int enc_oneframe_class_On2AvcEncoder(void * AvcEncoder, uint8_t* aOutBuffer, uint32_t * aOutputLength, uint8_t* aInputBuf,uint32_t  aInBuffPhy,uint32_t *aInBufSize,uint32_t * aOutTimeStamp, int* aSyncFlag);
extern void set_config_class_On2AvcEncoder(void * AvcEncoder,EncParameter_t *vpug);
extern void get_config_class_On2AvcEncoder(void * AvcEncoder,EncParameter_t *vpug);
extern int set_idrframe_class_On2AvcEncoder(void * AvcEncoder);
extern int set_inputformat_class_On2AvcEncoder(void * AvcEncoder,H264EncPictureType inputFormat);

extern void *  get_class_On2Vp8Encoder(void);
extern void  destroy_class_On2Vp8Encoder(void * Vp8Encoder);
extern int init_class_On2Vp8Encoder(void * Vp8Encoder, EncParameter_t *aEncOption, uint8_t* aOutBuffer,uint32_t* aOutputLength);
extern int deinit_class_On2Vp8Encoder(void * Vp8Encoder);
extern int enc_oneframe_class_On2Vp8Encoder(void * Vp8Encoder, uint8_t* aOutBuffer, uint32_t* aOutputLength,
                                     uint8_t *aInBuffer,uint32_t aInBuffPhy,uint32_t* aInBufSize,uint32_t* aOutTimeStamp, int *aSyncFlag);
extern void *  get_class_On2MjpegEncoder(void);
extern void  destroy_class_On2MjpegEncoder(void * MjpegEncoder);
extern int init_class_On2MjpegEncoder(void * MjpegEncoder, EncParameter_t *aEncOption, unsigned char* aOutBuffer,unsigned int* aOutputLength);
extern int deinit_class_On2MjpegEncoder(void * MjpegEncoder);
extern int enc_oneframe_class_On2MjpegEncoder(void *MjpegEncoder, unsigned char* aOutBuffer, unsigned int* aOutputLength,
                                     unsigned char *aInBuffer,unsigned int aInBuffPhy,unsigned int* aInBufSize,unsigned int* aOutTimeStamp, int *aSyncFlag);
#ifdef __cplusplus
}
#endif

typedef struct tag_VPU_API {
    void* (*         get_class_On2Decoder)(void);
    void  (*     destroy_class_On2Decoder)(void *decoder);
    int   (*      deinit_class_On2Decoder)(void *decoder);
    int   (*        init_class_On2Decoder)(void *decoder);
    int   (*        init_class_On2Decoder_M4VH263)(void *decoder, VPU_GENERIC *vpug);
    int   (*        init_class_On2Decoder_VC1)(void *decoder, uint8_t *tmpStrm, uint32_t size,uint32_t extraDataSize);
    int   (*        init_class_On2Decoder_VP6)(void *decoder, int codecid);
    int   (*        init_class_On2Decoder_AVC)(void *decoder,int tsFlag);
    int   (*       reset_class_On2Decoder)(void *decoder);
    int   (*dec_oneframe_class_On2Decoder)(void *decoder, uint8_t* aOutBuffer, uint32_t *aOutputLength, uint8_t* aInputBuf, uint32_t* aInBufSize);
    int   (*dec_oneframe_class_On2Decoder_WithTimeStamp)(void *decoder, uint8_t* aOutBuffer, uint32_t *aOutputLength, uint8_t* aInputBuf, uint32_t* aInBufSize, int64_t* InputTimestamp);
    int   (*get_oneframe_class_On2Decoder)(void *decoder, uint8_t* aOutBuffer, uint32_t* aOutputLength);
    void  (*set_width_Height_class_On2Decoder_RV)(void *decoder, uint32_t* width, uint32_t* height);
    int   (*flush_oneframe_in_dpb_class_On2Decoder)(void *decoder, uint8_t* aOutBuffer, uint32_t *aOutputLength);

	void* (*         get_class_On2Encoder)(void);
    void  (*     destroy_class_On2Encoder)(void *encoder);
    int   (*      deinit_class_On2Encoder)(void *encoder);
    int   (*        init_class_On2Encoder)(void *encoder, EncParameter_t *aEncOption, uint8_t *aOutBuffer, uint32_t* aOutputLength);
    int   (*enc_oneframe_class_On2Encoder)(void *encoder, uint8_t* aOutBuffer, uint32_t * aOutputLength,uint8_t* aInputBuf,uint32_t  aInBuffPhy,uint32_t *aInBufSize,uint32_t * aOutTimeStamp, int* aSyncFlag);
    void  (*enc_getconfig_class_On2Encoder)(void * AvcEncoder,EncParameter_t* vpug);
    void  (*enc_setconfig_class_On2Encoder)(void * AvcEncoder,EncParameter_t* vpug);
    int   (*enc_setInputFormat_class_On2Encoder)(void * AvcEncoder,H264EncPictureType inputFormat);
    int   (*enc_setIdrframe_class_On2Encoder)(void * AvcEncoder);
} VPU_API;

typedef void (*VpuApiInitFactory)(VPU_API *vpu_api, OMX_DEC_VIDEO_CODINGTYPE video_coding_type);

#endif

