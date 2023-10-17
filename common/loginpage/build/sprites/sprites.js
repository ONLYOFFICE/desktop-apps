
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
            update_status: {
                src: [`${_path}res/img/update_status/*.svg`],
                dest: `${_path}res/img/`,
                options: {
                    mode: {
                        symbol: {
                            inline: true,
                            dest: './',
                            sprite: `update_status.svg`,
                        },
                    },
                }
            },
        },
        replace_allconnect: {                   //when fill =#fff the fill turns orange on the light theme page
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