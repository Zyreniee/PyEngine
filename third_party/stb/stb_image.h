// stb_image.h - v2.28 - public domain image loader
// For full implementation and documentation, see:
// https://github.com/nothings/stb

#ifndef STB_IMAGE_H
#define STB_IMAGE_H

#ifdef __cplusplus
extern "C" {
#endif

typedef unsigned char stbi_uc;
typedef unsigned short stbi_us;

typedef struct {
  int (*read)(void *user, char *data, int size);
  void (*skip)(void *user, int n);
  int (*eof)(void *user);
} stbi_io_callbacks;

extern unsigned char *stbi_load(char const *filename, int *x, int *y,
                                int *channels_in_file, int desired_channels);
extern unsigned char *stbi_load_from_memory(stbi_uc const *buffer, int len,
                                            int *x, int *y,
                                            int *channels_in_file,
                                            int desired_channels);
extern unsigned char *stbi_load_from_callbacks(stbi_io_callbacks const *clbk,
                                               void *user, int *x, int *y,
                                               int *channels_in_file,
                                               int desired_channels);

extern void stbi_image_free(void *retval_from_stbi_load);

extern int stbi_info(char const *filename, int *x, int *y, int *comp);
extern int stbi_info_from_memory(stbi_uc const *buffer, int len, int *x, int *y,
                                 int *comp);

extern void stbi_set_flip_vertically_on_load(int flag_true_if_should_flip);

extern const char *stbi_failure_reason(void);

#ifdef __cplusplus
}
#endif

#endif // STB_IMAGE_H

#ifdef STB_IMAGE_IMPLEMENTATION

// Minimal stub implementation - in real use, include full stb_image.h
#include <stdlib.h>
#include <string.h>

static const char *stbi__g_failure_reason;

extern const char *stbi_failure_reason(void) { return stbi__g_failure_reason; }

extern void stbi_image_free(void *retval) { free(retval); }

extern void stbi_set_flip_vertically_on_load(int flag) {
  // Stub
}

extern unsigned char *stbi_load(char const *filename, int *x, int *y, int *comp,
                                int req_comp) {
  stbi__g_failure_reason = "stb_image stub - not implemented";
  return NULL;
}

extern unsigned char *stbi_load_from_memory(stbi_uc const *buffer, int len,
                                            int *x, int *y, int *comp,
                                            int req_comp) {
  stbi__g_failure_reason = "stb_image stub - not implemented";
  return NULL;
}

extern int stbi_info(char const *filename, int *x, int *y, int *comp) {
  return 0;
}

#endif // STB_IMAGE_IMPLEMENTATION
