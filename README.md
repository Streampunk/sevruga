# sevruga
A [Node.JS](https://nodejs.org/) binding for [librsvg](https://github.com/GNOME/librsvg).

### Native bindings

This binding uses the experimental [N-API](https://nodejs.org/dist/latest-v8.x/docs/api/n-api.html) for the development of a C++ addon for Node.JS. Note that N-API will become a core part of Node.JS in version 10 LTS available from October 2018. Until then, a warning about this features being experimental will be printed.

## Installation

### Prerequisites

Install the librsvg library and header files. Usually you have to look for a development package version. You must also have a functioning build tool chain including pkg-config. You can find instructions for different operating systems below:

- Ubuntu:

    ```bash
    sudo apt-get install librsvg2-dev
    ```

- Mac OS X:

    ```bash
    brew install librsvg
    ```

    If, after installing LibRSVG through homebrew you are experiencing not found issues when installing this module, try manually exporting the package config path for any not found package with a command like this:

    ```bash
    export PKG_CONFIG_PATH=/usr/local/opt/zlib/lib/pkgconfig
    ```

    Then try reinstalling this module.

- Windows:

    Install [msys2](http://www.msys2.org/). Follow the instructions to ensure all packages are up-to-date.
    The installer defaults to install at C:\msys64. If you install to a different path you will need to update the GTK_Root variable in binding.gyp.
    
    Install build tools and librsvg via the pacman package manager:

    ```bash
    pacman -S mingw-w64-x86_64-toolchain base-devel
    pacman -S mingw-w64-x86_64-librsvg
    ```

Install [Node.JS](https://nodejs.org/) LTS for your platform.

Install the C++ build tools [node-gyp](https://github.com/nodejs/node-gyp) by following the [installation instructions](https://github.com/nodejs/node-gyp#installation).

### Using sevruga as a dependency

Install sevruga into your project by running the npm install command in its root folder:

    npm install sevruga

Note that the `--save` option is now the default.

### Building sevruga

1. Clone this project with `git clone https://github.com/Streampunk/sevruga.git`
2. Enter the project folder with `cd sevruga`
3. Run `npm install`

## Using sevruga

sevruga exports the function renderSVG that returns a promise that will be resolved when the supplied SVG data has been parsed and rendered to the supplied buffer according to the supplied width and height parameters. Timing parameters for the stages of render are available from the returned object as shown in the example below.

renderSVG requires three parameters:
1. A string containing the svg definition
2. An allocated buffer of sufficient size for the render results.
3. A params object which must include width and height members. An optional pngPath can be provided which if present will cause the output of a png version of the render result at the path provided.

```Javascript
const sevruga = require('sevruga');
const fs = require('fs');

async function svgTest() {
  const svgStr = fs.readFileSync(`${__dirname}/svg/Test.svg`, { encoding: 'utf8' }); // returns a string

  const params = { width: 1920, height: 1080 };
  const renderBuf = Buffer.alloc(params.width * params.height * 4); // ARGB 8-bit per component  

  const t = await sevruga.renderSVG(svgStr, renderBuf, params);
  console.log(`Parse: ${t.parseTime}, Render: ${t.renderTime}, Total: ${t.totalTime}`);
}
svgTest()
  .catch(err => console.log(`Sevruga render failed: ${err}`));
```

A Node.js transform stream implementation is also available. This allows a stream of SVG files to be processed, producing a stream of rendered buffers. This approach supports back-pressure so that there will be a limit on resources consumed.

```Javascript
const sevruga = require('../index.js');
const fs = require('fs');

function svgStreamTest() {
  const svgStr = fs.readFileSync(`${__dirname}/svg/Test.svg`, { encoding: 'utf8' });
  const params = { width: 1920, height: 1080 };
  svgStream = sevruga.createRenderStream(params)
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
  }
  write(svgStr);
}
svgStreamTest();
```

## Status, support and further development

Contributions can be made via pull requests and will be considered by the author on their merits. Enhancement requests and bug reports should be raised as github issues. For support, please contact [Streampunk Media](http://www.streampunk.media/).

## License

This software is released under the Apache 2.0 license. Copyright 2018 Streampunk Media Ltd.
