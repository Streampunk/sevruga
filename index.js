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

const addon = require('bindings')('sevruga');
const { Transform } = require('stream');

const SegfaultHandler = require('segfault-handler');
SegfaultHandler.registerHandler('crash.log'); // With no argument, SegfaultHandler will generate a generic log file name

function createRenderStream(params) {
  return new Transform({
    decodeStrings: params.decodeStrings || false,
    highWaterMark: params.highwaterMark || 16384,
    transform(svgStr, encoding, cb) {
      const renderBuf = Buffer.alloc(params.width * params.height * 4); // ARGB 8-bit per component
      addon.renderSVG(svgStr, renderBuf, params)
        .then(t => {
          renderBuf.timings = t;
          cb(null, renderBuf);
        })
        .catch(cb);
    }
  });
}

function renderSVG(svgStr, renderBuf, params) {
  return addon.renderSVG(svgStr, renderBuf, params);
}

module.exports = { createRenderStream, renderSVG };
