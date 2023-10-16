
module.exports = (grunt, rootpathprefix) => {
    const _path = rootpathprefix || '../../';
    grunt.initConfig({
        svg_sprite:{
            options: {
                svg: {
                    rootAttributes: {
                        xmlns:'http://www.w3.org/2000/svg',
                        fill: 'none',
                    },
                },
                shape: {
                    id: {
                        separator: ""
                    },
                    transform: [],
                    dimension: {
                        attributes: true
                    }
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
                },
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
                            sprite: `formats.svg`,
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
        },
        replace_allconnect:{
            dist: {
                files:[ {
                    src: [`${_path}res/img/allconnect.svg`],
                    dest: `${_path}res/img/`,
                }],
                options: {
                    replacements: [{
                        pattern: ' fill="#fff"',
                        replacement: ' fill="white"',
                    }]
                }
            }
        }
    });

    grunt.loadNpmTasks('grunt-svg-sprite');
    grunt.registerTask('generate-sprite', ['svg_sprite', 'replace_allconnect']);
}