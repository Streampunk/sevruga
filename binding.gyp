{
	"variables": {
		"GTK_Root%": "c:/msys64/mingw64",
		"conditions": [
			[
				"OS == 'mac'",
				{
					"pkg_env": "/usr/local/bin/pkg-config"
				},
				{
					"pkg_env": ""
				}
			]
		]
	},
  "targets": [
    {
      "target_name": "sevruga",
      "sources": [
        "src/sevruga.cc",
        "src/sevruga_util.cc",
        "src/sevruga_render.cc"
      ],
			"variables": {
				"packages": "librsvg-2.0",
				"conditions": [
          ["OS!='win'", {
            "libraries": "<!(<(pkg_env) pkg-config --libs-only-l <(packages))",
            "ldflags": "<!(<(pkg_env) pkg-config --libs-only-L --libs-only-other <(packages))",
            "cflags": "<!(<(pkg_env) pkg-config --cflags <(packages))"
          }]
				]
			},
      'conditions': [
      	["OS!='mac' and OS!='win'", {
          "cflags": [
            "<@(cflags)",
            "-std=c++0x"
          ],
          "ldflags": [
            "<@(ldflags)"
          ],
          "libraries": [
            "<@(libraries)"
          ]
        }],
				["OS=='mac'", {
          "xcode_settings": {
            "OTHER_CFLAGS": [
              "<@(cflags)"
            ],
            "OTHER_LDFLAGS": [
              "<@(ldflags)"
            ]
          },
          "libraries": [
            "<@(libraries)"
          ]
        }],
        ["OS=='win'", {
          "include_dirs": [
            "<(GTK_Root)\include\gdk-pixbuf-2.0",
            "<(GTK_Root)\include\librsvg-2.0",
            "<(GTK_Root)\include\cairo",
            "<(GTK_Root)\include\glib-2.0",
            "<(GTK_Root)\lib\glib-2.0\include"        
          ],
          "link_settings": {
            "libraries": [ 
              "libcairo.dll.a"
            ],
            "library_dirs": [ 
              "<(GTK_Root)\lib"
            ]
          },
          "msvs_settings": {
						'VCCLCompilerTool': {
							'AdditionalOptions': [
								"/EHsc"
							]
				  	}
					},
          "copies": [
            {
              "destination": "build/Release/",
              "files": [
                "<!@(node -p \"require('fs').readdirSync('<(GTK_Root)/bin').filter(f=>f.match(/.*\.dll/ig)).map(f=>'<(GTK_Root)/bin/'+f).join(' ')\")"
              ]
            }
          ]
        }]
      ]
    }
  ]
}
