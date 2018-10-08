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

#ifndef SEVRUGA_RENDER_H
#define SEVRUGA_RENDER_H

#include "node_api.h"
#include "sevruga_util.h"

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

struct renderCarrier : carrier {
  std::string svgSource;
  size_t sourceLength;
  unsigned char* renderBuffer;
  size_t renderBufLen;
  uint32_t width;
  uint32_t height;
  uint32_t stride;
  #ifdef _WIN32
  HINSTANCE hSVGDLL;
  #endif 
  long long parseTime;
  long long renderTime;
};

napi_value renderSVG(napi_env env, napi_callback_info info);

#endif
