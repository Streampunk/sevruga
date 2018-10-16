/* Copyright 2017 Streampunk Media Ltd.

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

var tape = require('tape');
const sevruga = require('../index.js');
const fs = require('fs');

function checkResult(t, buf) {
  const refBuf = fs.readFileSync(`${__dirname}/raw/blocks.raw`);
  t.ok(refBuf, 'read reference result');
  if (refBuf)
    t.ok(0 == Buffer.compare(buf, refBuf), 'render result matches reference');
}

tape('Test rendering from SVG file', t => {
  const svgRender = async (svgStr, buf, params) => await sevruga.renderSVG(svgStr, buf, params);
  const svgStr = fs.readFileSync(`${__dirname}/svg/blocks.svg`, { encoding: 'utf8' });
  const params = { width: 200, height: 200 };
  const renderBuf = Buffer.alloc(params.width * params.height * 4); // ARGB 8-bit per component
  svgRender(svgStr, renderBuf, params)
    .then(()=> {
      checkResult(t, renderBuf);
      t.end();
    })
    .catch(err => {
      t.fail(err);
      t.end();
    });
});
