#include <dlfcn.h>  // for dlopen/dlclose
#include "/usr/local/include/hybris/common/dlfcn.h"
#include <stdint.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/mman.h>

#include <unistd.h>
#include "../prebuild/include/rga.h"
#include "/usr/local/include/android/libon2/vpu_api.h"
#include "avcodec.h"
#include "internal.h"
#include "mpegvideo.h"


#include "libswscale/swscale.h"  
#include "libavutil/opt.h"  
#include "libavutil/imgutils.h" 

#include <sys/time.h>
#include <time.h>

#define RTLD_NOW	0x00002
const char *HYBRIS_LIB = "/usr/local/hybris/lib/libhybris-common.so";
const char *RK_DEC_LIB = "librk_on2.so";
const char *RK_VPU_LIB = "libvpu.so";

typedef RK_S32 (*VpuMemFunc)(VPUMemLinear_t *p);
typedef RK_S32 (*VPUMallocFunc)(VPUMemLinear_t *p, RK_U32 size);
typedef RK_S32 (*VpuContextFunc)(VpuCodecContext_t **ctx);
typedef RK_S32 (*VPUFreeSizeFunc)();
typedef void *(*HYBRIS_dlopen)(const char *filename, int flag);
typedef void *(*HYBRIS_dlsym)(void *handle, const char *symbol);
typedef int  *(* HYBRIS_dlclose)(void *handle);


typedef struct _RkVpuDecContext {
    OMX_ON2_VIDEO_CODINGTYPE    video_type;
    VpuCodecContext_t*		ctx;
    DecoderOut_t		decOut;
    VideoPacket_t		demoPkt;
    VpuMemFunc			vpu_mem_link;
    VpuMemFunc			vpu_free_linear;

    VpuContextFunc		vpu_open;
    VpuContextFunc		vpu_close;
    void*			dec_handle;
    void*			vpu_handle;
    void*			hybris_handle;
    HYBRIS_dlopen		hybris_dlopen;
    HYBRIS_dlsym		hybris_dlsym;
    HYBRIS_dlclose		hybris_dlclose;
    int                         rga_fd;
    int                         nal_length_size;
    bool                        header_send_flag;
} RkVpuDecContext;

//-------------------------------------------------------------------------
static bool rkdec_hw_support(enum AVCodecID codec_id, unsigned int codec_tag) {
    switch(codec_id){
        case AV_CODEC_ID_MPEG4:
        case AV_CODEC_ID_H264:
	case AV_CODEC_ID_HEVC:
            return true;
        default:
            return false;
    }
    return false;
}


static bool init_declib(RkVpuDecContext* rkdec_ctx) {

    if (rkdec_ctx->hybris_handle == NULL) {
        rkdec_ctx->hybris_handle = dlopen(HYBRIS_LIB, RTLD_NOW);
    }

    if (rkdec_ctx->hybris_handle == NULL) {
        printf("Failed to open HYBRIS_LIB \n");
	return rkdec_ctx->dec_handle != NULL && rkdec_ctx->vpu_handle != NULL;
    }

    rkdec_ctx->hybris_dlopen = (HYBRIS_dlopen) dlsym(rkdec_ctx->hybris_handle, "hybris_dlopen");

    if(rkdec_ctx->hybris_dlopen==NULL){
	printf("get dlopen error\n");
	return rkdec_ctx->dec_handle != NULL && rkdec_ctx->vpu_handle != NULL;
	}
	
    rkdec_ctx->hybris_dlsym = (HYBRIS_dlsym) dlsym(rkdec_ctx->hybris_handle, "hybris_dlsym");

    if(rkdec_ctx->hybris_dlsym==NULL){
	printf("get dlsym error\n");
	return rkdec_ctx->dec_handle != NULL && rkdec_ctx->vpu_handle != NULL;
	}
    rkdec_ctx->hybris_dlclose = (HYBRIS_dlclose)dlsym(rkdec_ctx->hybris_handle,"hybris_dlclose");
    if(rkdec_ctx->hybris_dlclose==NULL){
	printf("get dlclose error\n");
	return rkdec_ctx->dec_handle != NULL && rkdec_ctx->vpu_handle != NULL;
	}

    if (rkdec_ctx->dec_handle == NULL) {
        rkdec_ctx->dec_handle = rkdec_ctx->hybris_dlopen(RK_DEC_LIB, RTLD_NOW);
    }

    if (rkdec_ctx->dec_handle == NULL) {
        printf("Failed to open RK_DEC_LIB \n");
    }

    if (rkdec_ctx->vpu_handle == NULL) {
        rkdec_ctx->vpu_handle = rkdec_ctx->hybris_dlopen(RK_VPU_LIB, RTLD_NOW);
    }
    if (rkdec_ctx->vpu_handle == NULL) {
        printf("Failed to open RK_VPU_LIB \n");
    }

    return rkdec_ctx->dec_handle != NULL && rkdec_ctx->vpu_handle != NULL;
}

static int rkdec_prepare(AVCodecContext* avctx)
{
    RkVpuDecContext* rkdec_ctx = avctx->priv_data;

    if (!init_declib(rkdec_ctx)) {
        printf("init_decdeclib failed\n");
        return -1;
    }
    rkdec_ctx->vpu_open = (VpuContextFunc) rkdec_ctx->hybris_dlsym(rkdec_ctx->dec_handle, "vpu_open_context");
    rkdec_ctx->vpu_close = (VpuContextFunc) rkdec_ctx->hybris_dlsym(rkdec_ctx->dec_handle, "vpu_close_context");
    rkdec_ctx->vpu_mem_link = (VpuMemFunc) rkdec_ctx->hybris_dlsym(rkdec_ctx->vpu_handle, "VPUMemLink");
    rkdec_ctx->vpu_free_linear = (VpuMemFunc) rkdec_ctx->hybris_dlsym(rkdec_ctx->vpu_handle, "VPUFreeLinear");
	
    if (rkdec_ctx->vpu_open(&rkdec_ctx->ctx) || rkdec_ctx->ctx == NULL) {
        printf("vpu_open_context failed\n");
        return -1;
    }
    rkdec_ctx->ctx->codecType = CODEC_DECODER;
    rkdec_ctx->ctx->videoCoding = rkdec_ctx->video_type;
    rkdec_ctx->ctx->width = avctx->width;
    rkdec_ctx->ctx->height = avctx->height;
    rkdec_ctx->ctx->no_thread = 0;
    rkdec_ctx->ctx->enableparsing = 1;
    if (rkdec_ctx->ctx->init(rkdec_ctx->ctx, avctx->extradata, avctx->extradata_size) != 0) {
        printf("ctx init failed **********************8\n");
        return -1;
    }
#if 1
    rkdec_ctx->rga_fd = open("/dev/rga",O_RDWR,0);
    if (rkdec_ctx->rga_fd < 0) {
        printf("failed to open rga.");
        return false;
    }
#endif
    return 0;
}
static int rkdec_init(AVCodecContext *avctx) {
    RkVpuDecContext* rkdec_ctx = avctx->priv_data;
    if (!rkdec_hw_support(avctx->codec_id, avctx->codec_tag)) {
        return -1;
    }
    memset(rkdec_ctx, 0, sizeof(RkVpuDecContext));
    rkdec_ctx->header_send_flag = false;
    rkdec_ctx->video_type = OMX_ON2_VIDEO_CodingUnused;
    switch(avctx->codec_id){
        case AV_CODEC_ID_MPEG4:
        case AV_CODEC_ID_MSMPEG4V3:
            rkdec_ctx->video_type = OMX_ON2_VIDEO_CodingMPEG4;
            break;
        case AV_CODEC_ID_H264:
            rkdec_ctx->video_type = OMX_ON2_VIDEO_CodingAVC;
            break;
        case AV_CODEC_ID_HEVC:
            rkdec_ctx->video_type = OMX_RK_VIDEO_CodingHEVC;
            break;
        default:
            printf("no support\n");
            break;
    }
	 avctx->pix_fmt = PIX_FMT_NV12;

	rkdec_ctx->ctx = 0;
	rkdec_ctx->dec_handle = 0;
	rkdec_ctx->vpu_handle = 0;
	rkdec_ctx->hybris_handle = 0;
    return rkdec_prepare(avctx);
}

static int rkdec_deinit(AVCodecContext *avctx) {
    RkVpuDecContext* rkdec_ctx = avctx->priv_data;
     if(rkdec_ctx->ctx){
          rkdec_ctx->vpu_close(&rkdec_ctx->ctx);
	  rkdec_ctx->ctx = 0;
     } 
    
#if 1
    if (rkdec_ctx->rga_fd > 0) {
        close(rkdec_ctx->rga_fd);
        rkdec_ctx->rga_fd = 0;
    }
#endif
    return 0;
}

static int rkdec_decode_frame(AVCodecContext *avctx/*ctx*/, void *data/*AVFrame*/,
        int *got_frame/*frame count*/, AVPacket *packet/*src*/) {
    int ret = 0;
    struct timeval t_start,t_start1,t_end;
    float timeuse = 0.0;
    float usetime =0.0;
    int retry=0;

    RkVpuDecContext* rkdec_ctx = avctx->priv_data;
    VpuCodecContext_t* ctx = rkdec_ctx->ctx;
    VideoPacket_t* pDemoPkt = &rkdec_ctx->demoPkt;
    DecoderOut_t* pDecOut = &rkdec_ctx->decOut;

    pDemoPkt->data = packet->data;
    pDemoPkt->size = packet->size;
    pDemoPkt->capability = pDemoPkt->size;
    if (packet->pts != AV_NOPTS_VALUE) {
        pDemoPkt->pts = av_q2d(avctx->time_base)*(packet->pts)*1000000ll;
    } else {
        pDemoPkt->pts = av_q2d(avctx->time_base)*(packet->dts)*1000000ll;
    }

        pDemoPkt->pts = VPU_API_NOPTS_VALUE;
	
        pDemoPkt->dts = VPU_API_NOPTS_VALUE;

    memset(pDecOut, 0, sizeof(DecoderOut_t));
    pDecOut->data = (RK_U8 *)(av_malloc)(sizeof(VPU_FRAME));
    if (pDecOut->data == NULL) {
        printf("malloc VPU_FRAME failed \n");
        goto out;
    }
    gettimeofday(&t_start,NULL);
REDECODE:
    memset(pDecOut->data, 0, sizeof(VPU_FRAME));
    pDecOut->size = 0; 
    *got_frame = 0;
        if ((ret = ctx->decode(ctx, pDemoPkt, pDecOut)) == 0) {
	
	if(pDemoPkt->size > 0){
		retry ++;
		usleep(retry*50);
		goto REDECODE;
					
	}
            if (pDecOut->size && pDecOut->data) {
		int decode_width, decode_height;
                AVFrame *frame = data;
                VPU_FRAME *pframe = (VPU_FRAME *) (pDecOut->data);
                rkdec_ctx->vpu_mem_link(&pframe->vpumem);
		decode_width = avctx->width;
		decode_height = avctx->height;
                if ((ret = ff_get_buffer(avctx, frame, 0)) < 0) {

                    printf("Failed to get buffer!!!:%d\n", ret);
                    goto dout;
                }

		 struct rga_req  Rga_Request;
		int act_width = pframe->DisplayWidth;
		int act_height = pframe->DisplayHeight;
		int v_width = pframe->FrameWidth;
		int v_height = pframe->FrameHeight;

		char* virmem = malloc(decode_width*decode_height*3/2);
		memset(&Rga_Request,0x0,sizeof(Rga_Request));
                Rga_Request.src.yrgb_addr = pframe->vpumem.phy_addr;
                Rga_Request.src.uv_addr  = 0;
                Rga_Request.src.v_addr   =      0;
                Rga_Request.src.vir_w = v_width;
                Rga_Request.src.vir_h = v_height;
                Rga_Request.src.format = RK_FORMAT_YCbCr_420_SP;
                Rga_Request.src.act_w = act_width;
                Rga_Request.src.act_h = act_height;
                Rga_Request.src.x_offset = 0;
                Rga_Request.src.y_offset = 0;
	///////////////////////////////////////////////////////////////////////	
                Rga_Request.dst.yrgb_addr = 0;
                Rga_Request.dst.uv_addr  = virmem;
                Rga_Request.dst.v_addr   = 0;
                Rga_Request.dst.vir_w = decode_width;
                Rga_Request.dst.vir_h = decode_height;
                Rga_Request.dst.format = RK_FORMAT_YCbCr_420_SP;
                Rga_Request.dst.act_w = decode_width;
                Rga_Request.dst.act_h = decode_height;
                Rga_Request.dst.x_offset = 0;
                Rga_Request.dst.y_offset = 0;

                Rga_Request.clip.xmin = 0;
                Rga_Request.clip.xmax = v_width  - 1;
                Rga_Request.clip.ymin = 0;
                Rga_Request.clip.ymax = v_height - 1;
		Rga_Request.rotate_mode = 0;
		 Rga_Request.render_mode = bitblt_mode;
        	Rga_Request.rotate_mode = rotate_mode0; // NO_ROTATE
        	Rga_Request.mmu_info.mmu_en = 1;
        	Rga_Request.mmu_info.mmu_flag = 0x21;

    		Rga_Request.PD_mode = 1;
    		Rga_Request.src_trans_mode = 0;

    		Rga_Request.sina = 0;
    		Rga_Request.cosa = 65536;
    		Rga_Request.alpha_rop_flag = 0;

    		Rga_Request.src.endian_mode = 1;
    		Rga_Request.dst.endian_mode = 1;

                if(ret=ioctl(rkdec_ctx->rga_fd, RGA_BLIT_SYNC, &Rga_Request) != 0)
                {
                     printf("RepeaterSource rga RGA_BLIT_SYNC fail ret %d\n",ret);
                    goto dout;
                }

		memcpy(frame->data[0],virmem,decode_width*decode_height);
		memcpy(frame->data[1],virmem+decode_width*decode_height,decode_width*decode_height>>1);
                *got_frame = 1;
dout:
               	rkdec_ctx->vpu_free_linear(&pframe->vpumem);
		free(virmem);
            }
        } else {
            printf("get frame failed ret %d\n", ret);
            goto out;
        }

        ret = 0;
out:
        if (pDecOut->data != NULL) {
            av_free(pDecOut->data);
            pDecOut->data = NULL;
        }
        if (ret < 0) {
            printf("Something wrong during decode!!!\n");
        }
        return (ret < 0) ? ret : packet->size;


    }

    static void rkdec_decode_flush(AVCodecContext *avctx) {
        RkVpuDecContext* rkdec_ctx = avctx->priv_data;
        rkdec_ctx->ctx->flush(rkdec_ctx->ctx);
    }

#define DECLARE_RKDEC_VIDEO_DECODER(TYPE, CODEC_ID)                     \
    AVCodec ff_##TYPE##_decoder = {                                         \
        .name           = #TYPE,                                            \
        .long_name      = NULL_IF_CONFIG_SMALL(#TYPE " hw decoder"),        \
        .type           = AVMEDIA_TYPE_VIDEO,                               \
        .id             = CODEC_ID,                                         \
        .priv_data_size = sizeof(RkVpuDecContext),                          \
        .init           = rkdec_init,                                       \
        .close          = rkdec_deinit,                                     \
        .decode         = rkdec_decode_frame,                               \
        .capabilities   = CODEC_CAP_DR1 | CODEC_CAP_DELAY,                               \
        .flush          = rkdec_decode_flush,                               \
        .pix_fmts       = (const enum AVPixelFormat[]) {                    \
	    AV_PIX_FMT_NV12,			\
            AV_PIX_FMT_NONE							\
        },									\
    };                                                                      \

        DECLARE_RKDEC_VIDEO_DECODER(mpeg4_rkvpu, AV_CODEC_ID_MPEG4)
        DECLARE_RKDEC_VIDEO_DECODER(h264_rkvpu, AV_CODEC_ID_H264)
	DECLARE_RKDEC_VIDEO_DECODER(hevc_rkvpu, AV_CODEC_ID_HEVC)
