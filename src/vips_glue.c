#include <stdio.h>
#include <vips/vips.h>

#include "io.h"
#include "sniffing.h"

RESULT rotate_image(VipsImage *in, VipsImage **out, int deg) {
  v_log(INFO, "rotate %3d degree", deg);
  VipsAngle angle;
  switch (deg % 360) {
  case 0:
    angle = VIPS_ANGLE_D0;
    break;
  case 90:
    angle = VIPS_ANGLE_D90;
    break;
  case 180:
    angle = VIPS_ANGLE_D180;
    break;
  case 270:
    angle = VIPS_ANGLE_D270;
    break;
  default:
    return FAIL;
  }

  if (vips_rot(in, out, angle, NULL)) {
    v_vips_err("rotating %d", deg);
    return FAIL;
  }

  return OK;
}

RESULT autorot_image(VipsImage *in, VipsImage **out) {
  v_log(INFO, "autorotate");
  if (vips_autorot(in, out, NULL)) {
    v_vips_err("autorotating");
    return FAIL;
  }
  return OK;
}

RESULT extract_image(VipsImage *in, VipsImage **out, int left, int top,
                     int width, int height) {
  v_log(INFO, "extracting %d %d %d %d\n", left, top, width, height);

  if (left < 0 || top < 0) {
    v_log(ERROR, "extract origin %d,%d is less than zero point", left, top);
    return FAIL;
  }

  if (width < 1 || height < 1) {
    v_log(ERROR, "extract dimensions must both exceed 1 %d %d", width, height);
    return FAIL;
  }


    int img_height = vips_image_get_height(in);
  int img_width = vips_image_get_width(in);

  // TODO: Double check this is correct
  if (img_height < height + top || img_width < width + left) {
    v_log(ERROR, "extract size (%d,%d) is outside image bounds (%d,%d)", width,
          height, img_width + left, img_height + top);
    return FAIL;
  }

  if (vips_extract_area(in, out, left, top, width, height, NULL)) {
    v_vips_err("extracting %d,%d,%d,%d failed", width, height, img_width,
               img_height);
    return FAIL;
  }
  return OK;
}

RESULT gaussblur_image(VipsImage *in, VipsImage **out, double sigma) {
  v_log(INFO, "gaussian blur sigma %f", sigma);
  if (vips_gaussblur(in, out, sigma, NULL)) {
    v_vips_err("gaussian blur sigma %f", sigma);
    return FAIL;
  }
  return OK;
}

RESULT resize_image(VipsImage *in, VipsImage **out, int newHeight, int newWidth,
                    unsigned opts) {
  char *resize_op = NULL;

  if (newHeight < 1) {
      v_log(ERROR, "width must be > 1, not %d", newHeight);
      return FAIL;
  }

  if (newHeight < 1) {
      v_log(ERROR, "height must be > 1, not %d", newWidth);
      return FAIL;
  }

  int curHeight = vips_image_get_height(in);
  int curWidth = vips_image_get_width(in);

  double heightRatio = (double)newHeight / (double)curHeight;
  double widthRatio = (double)newWidth / (double)curWidth;

  double hScale = 0;
  double vScale = 0;

  if (opts & STRETCH) {
    resize_op = "stretch";
    hScale = widthRatio;
    vScale = heightRatio;
  } else if (opts & EXPAND) {
    resize_op = "expand";
    if (heightRatio > widthRatio) {
      hScale = heightRatio;
      vScale = heightRatio;
    } else {
      hScale = widthRatio;
      vScale = widthRatio;
    }
  } else if (opts & RESIZE) {
    resize_op = "resize";
    if (heightRatio < widthRatio) {
      hScale = heightRatio;
      vScale = heightRatio;
    } else {
      hScale = widthRatio;
      vScale = widthRatio;
    }
  } else {
      v_log(ERROR, "unknown resize mode: 0x%x", opts);
      return FAIL;
  }

  if (vips_resize(in, out, hScale, "vscale", vScale, NULL)) {
    v_vips_err("%s (%d,%d)", resize_op, curWidth, curHeight);
    return FAIL;
  }

  int actWidth = vips_image_get_width(*out);
  int actHeight = vips_image_get_height(*out);

  v_log(INFO, "%s (%d,%d) -> (%d,%d)", resize_op, curWidth, curHeight, actWidth,
        actHeight);
  return OK;
}

RESULT export_image(VipsImage *in, void **out, size_t *n, format_t format,
                    int quality) {
  v_log(INFO, "exporting to %s with quality %d", get_format_name(format),
        quality);

  int vips_result = 0;
  switch (format) {
  case JPEG:
    vips_result = vips_jpegsave_buffer(in, out, n, "Q", quality, NULL);
    break;
  case PNG:
    vips_result =
        vips_pngsave_buffer(in, out, n, "Q", quality, "compression", 0, NULL);
    break;
  case WEBP:
    vips_result = vips_webpsave_buffer(in, out, n, "Q", quality, NULL);
    break;
  case TIFF:
    vips_result = vips_tiffsave_buffer(in, out, n, "Q", quality, NULL);
    break;
  case GIF:
    vips_result = vips_magicksave_buffer(in, out, n, "quality", quality,
                                         "format", "gif", NULL);
    break;
  case BMP:
    vips_result = vips_magicksave_buffer(in, out, n, "quality", quality,
                                         "format", "bmp", NULL);
    break;
  default:
    v_log(ERROR, "invalid format selected");
    return FAIL;
  }

  //int width = vips_image_get_width(in);
  //int height = vips_image_get_height(in);
  int height = 0;
  int width = 0;

  if (vips_result) {
    v_vips_err("exporting to %d, %d %s", width, height, get_format_name(format));
  }
  return OK;
}
