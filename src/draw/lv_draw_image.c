/**
 * @file lv_draw_img.c
 *
 */

/*********************
 *      INCLUDES
 *********************/
#include "lv_draw_image.h"
#include "../display/lv_display.h"
#include "../misc/lv_log.h"
#include "../misc/lv_math.h"
#include "../core/lv_refr.h"
#include "../stdlib/lv_mem.h"
#include "../stdlib/lv_string.h"

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/

/**********************
 *  STATIC VARIABLES
 **********************/

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

void lv_draw_image_dsc_init(lv_draw_image_dsc_t * dsc)
{
    lv_memzero(dsc, sizeof(lv_draw_image_dsc_t));
    dsc->recolor = lv_color_black();
    dsc->opa = LV_OPA_COVER;
    dsc->scale_x = LV_SCALE_NONE;
    dsc->scale_y = LV_SCALE_NONE;
    dsc->antialias = LV_COLOR_DEPTH > 8 ? 1 : 0;
    dsc->base.dsc_size = sizeof(lv_draw_image_dsc_t);
}

void lv_draw_layer(lv_layer_t * layer, const lv_draw_image_dsc_t * dsc, const lv_area_t * coords)
{
    lv_draw_task_t * t = lv_draw_add_task(layer, coords);

    t->draw_dsc = lv_malloc(sizeof(*dsc));
    lv_memcpy(t->draw_dsc, dsc, sizeof(*dsc));
    t->type = LV_DRAW_TASK_TYPE_LAYER;
    t->state = LV_DRAW_TASK_STATE_WAITING;

    lv_layer_t * layer_to_draw = (lv_layer_t *)dsc->src;
    layer_to_draw->all_tasks_added = true;

    lv_draw_finalize_task_creation(layer, t);
}

void lv_draw_image(lv_layer_t * layer, const lv_draw_image_dsc_t * dsc, const lv_area_t * coords)
{
    if(dsc->src == NULL) {
        LV_LOG_WARN("Image draw: src is NULL");
        return;
    }
    if(dsc->opa <= LV_OPA_MIN) return;

    LV_PROFILER_BEGIN;

    lv_draw_image_dsc_t * new_image_dsc = lv_malloc(sizeof(*dsc));
    lv_memcpy(new_image_dsc, dsc, sizeof(*dsc));
    lv_result_t res = lv_image_decoder_get_info(new_image_dsc->src, &new_image_dsc->header);
    if(res != LV_RESULT_OK) {
        LV_LOG_WARN("Couldn't get info about the image");
        lv_free(new_image_dsc);
        return;
    }

    lv_draw_task_t * t = lv_draw_add_task(layer, coords);
    t->draw_dsc = new_image_dsc;
    t->type = LV_DRAW_TASK_TYPE_IMAGE;

    lv_draw_finalize_task_creation(layer, t);
    LV_PROFILER_END;
}

lv_image_src_t lv_image_src_get_type(const void * src)
{
    lv_image_src_t img_src_type = LV_IMAGE_SRC_UNKNOWN;

    if(src == NULL) return img_src_type;
    const uint8_t * u8_p = src;

    /*The first byte shows the type of the image source*/
    if(u8_p[0] >= 0x20 && u8_p[0] <= 0x7F) {
        img_src_type = LV_IMAGE_SRC_FILE; /*If it's an ASCII character then it's file name*/
    }
    else if(u8_p[0] >= 0x80) {
        img_src_type = LV_IMAGE_SRC_SYMBOL; /*Symbols begins after 0x7F*/
    }
    else {
        img_src_type = LV_IMAGE_SRC_VARIABLE; /*`lv_image_dsc_t` is draw to the first byte < 0x20*/
    }

    if(LV_IMAGE_SRC_UNKNOWN == img_src_type) {
        LV_LOG_WARN("unknown image type");
    }

    return img_src_type;
}

/**********************
 *   STATIC FUNCTIONS
 **********************/
