#pragma once

#ifdef __cplusplus
extern "C" {
#endif

extern "C" void texsynth_inpaint(int w, int h, void *output_ptr, void *image_ptr, bool tiling);

#ifdef __cplusplus
}
#endif
