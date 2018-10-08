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

    If, after installing LibRSVG through homebrew you are experiencing issues installing this module, try manually exporting the package config with this command:

    ```bash
    export PKG_CONFIG_PATH=/opt/X11/lib/pkgconfig
    ```

    Then try reinstalling this module. For further information, [see this thread](https://github.com/Homebrew/homebrew/issues/14123).

- Windows:

    Install [msys2](http://www.msys2.org/). Follow the instructions to ensure all packages are up-to-date. 
    
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

```Javascript
const sevruga = require('sevruga');

async function svgTest() {
  const svgStr = fs.readFileSync(`<filename>.svg`, { encoding: 'utf8' }); // returns a string

  const params = { width: 1920, height: 1080 };
  const renderBuf = Buffer.alloc(params.width * params.height * 4); // ARGB 8-bit per component  

  const t = await sevruga.renderSVG(svgStr, renderBuf, params);
  console.log(`Parse: ${t.parseTime}, Render: ${t.renderTime}, Total: ${t.totalTime}`);
}
svgTest()
  .catch(err => console.log(`Sevruga render failed: ${err}`));
```

## Status, support and further development

Contributions can be made via pull requests and will be considered by the author on their merits. Enhancement requests and bug reports should be raised as github issues. For support, please contact [Streampunk Media](http://www.streampunk.media/).

## License

This software is released under the Apache 2.0 license. Copyright 2018 Streampunk Media Ltd.
