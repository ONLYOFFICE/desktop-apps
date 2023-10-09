module.exports = function (grunt, rootpathprefix) {
    const _path = rootpathprefix || '../../';
    grunt.initConfig({
        svg_sprite:{
            options: {
                svg: {
                    rootAttributes: {
                        //xmlns:'http://www.w3.org/2000/svg',
                    },
                },
                shape: {
                    id: {
                        separator: ""
                    },
                    transform: [{
                        svgo: {
                            plugins: [
                                'removeXMLNS',
                                {
                                    name: "removeAttrs",
                                    params: {
                                        attrs: "(fill|stroke)"
                                    }
                                },
                            ]
                        },
                    }]
                },
                mode: {
                    symbol: {
                    },
                },
            },
            allconnect: {
                src: [`${_path}res/img/connect*.svg`],
                dest: `${_path}res/img/`,
                options: {
                    mode: {
                        symbol: {
                            inline: true,
                            dest: './',
                            sprite: `allconnect.svg`,
                        },
                    },
                }
            },
            allwelcome: {
                src: [`${_path}res/img/welcome*.svg`],
                dest: `${_path}res/img/`,
                options: {
                    mode: {
                        symbol: {
                            inline: true,
                            dest: './',
                            sprite: `allwelcome.svg`,
                        },
                    },
                }
            },
            formats: {
                src: [`${_path}res/img/formats-svg/*.svg`],
                dest: `${_path}res/img/`,
                options: {
                    mode: {
                        symbol: {
                            inline: true,
                            dest: './',
                            sprite: `format.svg`,
                        },
                    },
                }
            },
            common: {
                src: [`${_path}res/img/common-svg/*.svg`],
                dest: `${_path}res/img/`,
                options: {
                    mode: {
                        symbol: {
                            inline: true,
                            dest: './',
                            sprite: `common.svg`,
                        },
                    },
                }
            },
        }
    });

    // Load in `grunt-spritesmith`
    grunt.loadNpmTasks('grunt-svg-sprite');
    grunt.registerTask('generate-sprite', ['svg_sprite']);
    grunt.registerTask('default', ['generate-sprite']);
};
