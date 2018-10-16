/* Copyright 2018 Streampunk Media Ltd.

  Licensed under the Apache License, Version 2.0 (the "License");
  you may not use this file except in compliance with the License.
  You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.
*/

#include "sevruga_render.h"
#include <gdk-pixbuf/gdk-pixbuf.h>
#include <librsvg/rsvg.h>

#ifdef _WIN32
#include <cairo-win32.h>
#include <mutex>
static std::mutex m;
#endif

void checkProc(void *p, char *msg)
{
  if (!p)
    printf("Failed to find %s\n", msg);
}

void renderExecute(napi_env env, void* data) {
  renderCarrier* c = (renderCarrier*) data;

  #ifdef _WIN32
  typedef RsvgHandle*(*LPFNDLLRSVGNEW)(const guint8*, gsize, GError**);
  LPFNDLLRSVGNEW rsvg_handle_new_from_data = (LPFNDLLRSVGNEW)GetProcAddress(c->hSVGDLL, "rsvg_handle_new_from_data");
  checkProc(rsvg_handle_new_from_data, "\'rsvg_handle_new_from_data\'");

  typedef gboolean(*LPFNDLLRSVGRENDER)(RsvgHandle*, cairo_t*);
  LPFNDLLRSVGRENDER rsvg_handle_render_cairo = (LPFNDLLRSVGRENDER)GetProcAddress(c->hSVGDLL, "rsvg_handle_render_cairo");
  checkProc(rsvg_handle_render_cairo, "\'rsvg_handle_render_cairo\'");

  typedef gboolean(*LPFNDLLRSVGCLOSE)(RsvgHandle*, GError**);
  LPFNDLLRSVGCLOSE rsvg_handle_close = (LPFNDLLRSVGCLOSE)GetProcAddress(c->hSVGDLL, "rsvg_handle_close");
  checkProc(rsvg_handle_close, "\'rsvg_handle_close\'");
  #endif

  HR_TIME_POINT start = NOW;
  HR_TIME_POINT parseStart = start;

  // parse
  GError* error = NULL;
  RsvgHandle *handle = rsvg_handle_new_from_data((const guint8 *)c->svgSource.c_str(), (gsize)c->svgSource.length(), &error);
  ASYNC_SVG_ERROR;

  c->parseTime = microTime(parseStart);
  HR_TIME_POINT renderStart = NOW;

  // render
  // Win32 optimisation test
  //cairo_surface_t *surface = cairo_win32_surface_create_with_dib(CAIRO_FORMAT_ARGB32, c->width, c->height);
  
  cairo_surface_t *surface = cairo_image_surface_create_for_data(c->renderBuffer, CAIRO_FORMAT_ARGB32, c->width, c->height, c->stride);
  cairo_t* cr = cairo_create(surface);
  {
    #ifdef _WIN32
    // cairo is not thread safe on windows
    std::lock_guard<std::mutex> lk(m);
    #endif

    gboolean success = rsvg_handle_render_cairo(handle, cr);

    if (FALSE == success) {
      c->status = SEVRUGA_ASYNC_FAILURE;
      char errorMsg[200];
      sprintf(errorMsg, "In file %s line %d: %s.",
        __FILE__, __LINE__ - 6, "failed to render SVG via cairo");
      c->errorMsg = std::string(errorMsg);
    }
  }
  cairo_surface_flush(surface);

  if (!c->pngPath.empty()) {
    cairo_status_t status = cairo_surface_write_to_png (surface, c->pngPath.c_str());
    ASYNC_CAIRO_ERROR;
  }

  // Win32 optimisation test
  //unsigned char* renderData = cairo_image_surface_get_data(cairo_win32_surface_get_image(surface));
  //memcpy(c->renderBuffer, renderData, c->renderBufLen);

  cairo_destroy(cr);
  cairo_surface_destroy(surface);

  rsvg_handle_close(handle, &error);

  c->renderTime = microTime(renderStart);
  c->totalTime = microTime(start);
}

void renderComplete(napi_env env, napi_status asyncStatus, void* data) {
  renderCarrier* c = (renderCarrier*) data;

  if (asyncStatus != napi_ok) {
    c->status = asyncStatus;
    c->errorMsg = "Async run of program failed to complete.";
  }
  REJECT_STATUS;

  napi_value result;
  c->status = napi_create_object(env, &result);
  REJECT_STATUS;

  napi_value totalValue;
  c->status = napi_create_int64(env, (int64_t) c->totalTime, &totalValue);
  REJECT_STATUS;
  c->status = napi_set_named_property(env, result, "totalTime", totalValue);
  REJECT_STATUS;

  napi_value parseValue;
  c->status = napi_create_int64(env, (int64_t) c->parseTime, &parseValue);
  REJECT_STATUS;
  c->status = napi_set_named_property(env, result, "parseTime", parseValue);
  REJECT_STATUS;

  napi_value renderValue;
  c->status = napi_create_int64(env, (int64_t) c->renderTime, &renderValue);
  REJECT_STATUS;
  c->status = napi_set_named_property(env, result, "renderTime", renderValue);
  REJECT_STATUS;

  napi_status status;
  status = napi_resolve_deferred(env, c->_deferred, result);
  FLOATING_STATUS;

  tidyCarrier(env, c);
}

napi_value renderSVG(napi_env env, napi_callback_info info) {
  napi_status status;
  renderCarrier* c = new renderCarrier;

  napi_value args[3];
  size_t argc = 3;
  napi_value programValue;
  status = napi_get_cb_info(env, info, &argc, args, &programValue, nullptr);
  CHECK_STATUS;

  if (argc != 3) {
    status = napi_throw_error(env, nullptr, "Wrong number of arguments. Three expected.");
    return nullptr;
  }

  napi_valuetype t;
  status = napi_typeof(env, args[0], &t);
  if (t != napi_string) {
    status = napi_throw_type_error(env, nullptr, "First argument should be a string - the SVG definition.");
    return nullptr;
  }

  napi_value renderer;
  status = napi_create_object(env, &renderer);
  CHECK_STATUS;

  status = napi_set_named_property(env, renderer, "svgSource", args[0]);
  CHECK_STATUS;

  status = napi_get_value_string_utf8(env, args[0], nullptr, 0, &c->sourceLength);
  CHECK_STATUS;
  char* svgSource = (char*)malloc(c->sourceLength + 1);
  status = napi_get_value_string_utf8(env, args[0], svgSource, c->sourceLength + 1, nullptr);
  CHECK_STATUS;
  c->svgSource = std::string(svgSource);
  delete svgSource;

  bool isBuffer;
  status = napi_is_buffer(env, args[1], &isBuffer);
  CHECK_STATUS;
  if (!isBuffer) {
    status = napi_throw_type_error(env, nullptr, "Second argument should be a buffer for the render result.");
    return nullptr;
  }

  status = napi_get_buffer_info(env, args[1], (void**)&c->renderBuffer, &c->renderBufLen);
  CHECK_STATUS;

  status = napi_typeof(env, args[2], &t);
  CHECK_STATUS;
  if (t != napi_object) {
    status = napi_throw_type_error(env, nullptr, "Third argument must be an object.");
    return nullptr;
  }
  napi_value params = args[2];

  bool hasProp;
  status = napi_has_named_property(env, params, "width", &hasProp);
  CHECK_STATUS;
  if (!hasProp) {
    status = napi_throw_type_error(env, nullptr, "Parameters must have \'width\'.");
    return nullptr;
  }
  status = napi_has_named_property(env, params, "height", &hasProp);
  CHECK_STATUS;
  if (!hasProp) {
    status = napi_throw_type_error(env, nullptr, "Parameters must have \'height\'.");
    return nullptr;
  }

  napi_value widthValue, heightValue;
  status = napi_get_named_property(env, params, "width", &widthValue);
  CHECK_STATUS;
  status = napi_typeof(env, widthValue, &t);
  CHECK_STATUS;
  if (t != napi_number) {
    status = napi_throw_type_error(env, nullptr, "Parameter \'width\' must be a number.");
    return nullptr;
  }
  status = napi_get_named_property(env, params, "height", &heightValue);
  CHECK_STATUS;
  status = napi_typeof(env, heightValue, &t);
  CHECK_STATUS;
  if (t != napi_number) {
    status = napi_throw_type_error(env, nullptr, "Parameter \'height\' must be a number.");
    return nullptr;
  }

  status = napi_set_named_property(env, renderer, "width", widthValue);
  CHECK_STATUS;
  status = napi_set_named_property(env, renderer, "height", heightValue);
  CHECK_STATUS;

  int32_t checkValue;
  status = napi_get_value_int32(env, widthValue, &checkValue);
  CHECK_STATUS;
  if (checkValue < 0) {
    status = napi_throw_range_error(env, nullptr, "Parameter \'width\' cannot be negative.");
    return nullptr;
  }
  status = napi_get_value_int32(env, heightValue, &checkValue);
  CHECK_STATUS;
  if (checkValue < 0) {
    status = napi_throw_range_error(env, nullptr, "Parameter \'height\' cannot be negative.");
    return nullptr;
  }

  status = napi_get_value_uint32(env, widthValue, &c->width);
  CHECK_STATUS;
  status = napi_get_value_uint32(env, heightValue, &c->height);
  CHECK_STATUS;

  c->stride = cairo_format_stride_for_width(CAIRO_FORMAT_ARGB32, c->width);
  if (c->renderBufLen < c->stride * c->height) {
    status = napi_throw_range_error(env, nullptr, "Render buffer is too small");
    return nullptr;
  }

  status = napi_has_named_property(env, params, "pngPath", &hasProp);
  CHECK_STATUS;
  if (hasProp) {
    napi_value pngPathValue;
    status = napi_get_named_property(env, params, "pngPath", &pngPathValue);
    CHECK_STATUS;

    size_t pathLen = 0;
    status = napi_get_value_string_utf8(env, pngPathValue, nullptr, 0, &pathLen);
    CHECK_STATUS;
    pathLen += 1; // null terminator
    
    char *pngPath = (char *)malloc(pathLen);
    status = napi_get_value_string_utf8(env, pngPathValue, pngPath, pathLen, nullptr);
    CHECK_STATUS;
    c->pngPath = std::string(pngPath);
    free(pngPath);
  }

  #ifdef _WIN32
  SetDllDirectoryA(".\\build\\Release");
  c->hSVGDLL = LoadLibraryA("librsvg-2-2.dll");
  if (NULL == c->hSVGDLL)
  {
    printf("Failed to find dll\n");
    return nullptr;
  }
  #endif

  status = napi_create_reference(env, renderer, 1, &c->passthru);
  CHECK_STATUS;

  napi_value promise, resource_name;
  status = napi_create_promise(env, &c->_deferred, &promise);
  CHECK_STATUS;

  status = napi_create_string_utf8(env, "Render", NAPI_AUTO_LENGTH, &resource_name);
  CHECK_STATUS;
  status = napi_create_async_work(env, NULL, resource_name, renderExecute,
    renderComplete, c, &c->_request);
  CHECK_STATUS;
  status = napi_queue_async_work(env, c->_request);
  CHECK_STATUS;

  return promise;
}
