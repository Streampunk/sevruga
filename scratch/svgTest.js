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

const addon = require('../index.js');
const fs = require('fs');

async function svgTest() {
  const svgStr = fs.readFileSync(`${__dirname}/svg/Test.svg`, { encoding: 'utf8' });

  const params = { width: 1920, height: 1080 };
  const renderBuf = Buffer.alloc(params.width * params.height * 4); // ARGB 8-bit per component
  
  for (let x = 0; x < 10; x++) {
    const t = await addon.renderSVG(svgStr, renderBuf, params);
    console.log(`Parse: ${t.parseTime}, Render: ${t.renderTime}, Total: ${t.totalTime}`);
  }
}
svgTest()
  .catch(err => console.log(`Sevruga render failed: ${err}`));
