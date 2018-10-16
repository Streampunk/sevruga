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

const sevruga = require('../index.js');
const fs = require('fs');

function svgStreamTest() {
  const svgStr = fs.readFileSync(`${__dirname}/svg/Test.svg`, { encoding: 'utf8' });
  const params = { width: 1920, height: 1080 };
  const svgStream = sevruga.createRenderStream(params);
  svgStream.on('data', buf => {
    const t = buf.timings;
    console.log(`Parse: ${t.parseTime}, Render: ${t.renderTime}, Total: ${t.totalTime}`);
  });
  svgStream.on('error', console.error);

  let i = 10;
  const write = str => {
    var ok = true;
    do {
      i -= 1;
      if (i === 0) {
        // last time!
        svgStream.end(str);
      } else {
        ok = svgStream.write(str, 'utf8');
        // console.log('Write data', ok);
      }
    } while (i > 0 && ok);
    if (i > 0) {
      // had to stop early!
      // write some more once it drains
      // console.log("So draining.");
      svgStream.once('drain', () => write(svgStr));
    }
  };
  write(svgStr);
}
svgStreamTest();
