module.exports = function(grunt) {
    var _ = require('lodash'),
        defaultConfig,
        packageFile;


    grunt.loadNpmTasks('grunt-contrib-clean');
    grunt.loadNpmTasks('grunt-contrib-copy');
    grunt.loadNpmTasks('grunt-contrib-less');
    grunt.loadNpmTasks('grunt-contrib-concat');
    grunt.loadNpmTasks('grunt-contrib-htmlmin');
    grunt.loadNpmTasks('grunt-text-replace');
    grunt.loadNpmTasks('grunt-inline');
    grunt.loadNpmTasks('grunt-terser');

    function doRegisterInitializeAppTask(name, appName, configFile) {
        return grunt.registerTask('init-build-' + name, 'Initialize build ' + appName, function(){
            defaultConfig = configFile;
            packageFile = require('./' + defaultConfig);

            if (packageFile)
                grunt.log.ok(appName + ' config loaded successfully'.green);
            else
                grunt.log.error().writeln('Could not load config file'.red);
        });
    }

    grunt.initConfig({
        jshint: {
            options: {
                curly: true,
                eqeqeq: true,
                eqnull: true,
                browser: true,
                globals: {
                    jQuery: true
                },
                force: true
            },
            common: ['../src/*.js']
        }
    });

    grunt.registerTask('desktop-app-extra', function() {
        grunt.initConfig({
            pkg: grunt.file.readJSON(defaultConfig),

            less: {
                production: {
                    options: {
                        compress: true,
                        ieCompat: false
                    },
                    files: {
                        "<%= pkg.desktop.less.files.dest %>": packageFile['desktop']['less']['files']['src']
                    }
                }
            },

            copy: {
                main: {
                    files: packageFile['desktop']['copy']['files']
                }
            },

            concat: {
                options: {
                    stripBanners: true,
                    banner: '/*\n' +
                    ' * Copyright (c) Ascensio System SIA <%= grunt.template.today("yyyy") %>. All rights reserved\n' +
                    ' *\n' +
                    ' * <%= pkg.homepage %> \n' +
                    ' *\n' +
                    ' */\n'
                },
                dist: {
                    files: {
                        "<%= pkg.desktop.concat.files.dest %>" : [packageFile.desktop.concat.files.src]
                    }
                }
            },

            terser: {
                options: {
                    format: {
                        comments: false,
                    },
                    compress: {
                        drop_console: true,
                    },
                },
                langs: {
                    files: {
                        '../deploy/langs.js' : '../locale/*.js'
                    }
                },
                core: {
                    src: '../deploy/build.js',
                    dest: '../deploy/build.min.js',
                },
                dialogconnect: {
                    files: {
                        '../src/dlglogin.min.js' : ['../src/dlglogin.js','../src/dialogconnect.js']
                    }
                },
            },

            htmlmin: {
                dist: {
                    options: {
                        removeComments: true,
                        collapseWhitespace: true,
                        minifyCSS: true
                    },
                    files: {
                        '../deploy/index.html': '../deploy/index.html'
                    }
                }
            },

            inline: {
                dist: {
                    src: '../deploy/index.html',
                    dest: '../deploy/index.html'
                }
            },

            clean: {
                options: {
                    force: true
                },
                files: packageFile['desktop']['clean']
            }
        });
    });

    grunt.registerTask('compile-html', function(){
        if (!grunt.option('external-image')) {
            grunt.config('replace.insert-css', {
                    src: ['../deploy/index.html'],
                    overwrite: true,
                    replacements: [{
                        from: /(\<link[^\<]+stylesheet[^\<]+href="(\w+\.css)\"\>)/,
                        to: function(matchedWord, index, fullText, regexMatches) {
                            var css = grunt.file.read('../deploy/' + regexMatches[1]);
                            return '<style type="text/css">' + css + '</style>';
                        }
                    },{
                        from: /(\<script\s[^\>]+replace[^\>]+"([\w\S\.]+\.js)\"\>\<\/script\>)/g,
                        to: function(matchedWord, index, fullText, regexMatches) {
                            if (!grunt.file.exists('../deploy/' + regexMatches[1])) {
                                grunt.log.error().writeln('file does not exists: ' + regexMatches[1]);
                            } 

                            var script = grunt.file.read('../deploy/' + regexMatches[1]);
                            return '<script>' + script + '</script>';
                        }
                    }]
            });

            grunt.config('clean.files', ['../deploy/*.js','../deploy/*.css','../deploy/locale']);
            grunt.task.run('replace:insert-css', 'clean');
        }
    });


    doRegisterInitializeAppTask('startpage', 'Desktop start page', 'startpage.json');

    grunt.registerTask('deploy-desktop-startpage', ['desktop-app-extra', 'copy', 'less', 'terser:dialogconnect',
        'concat', 'clean', 'inline', 'terser:core', 'terser:langs', 'htmlmin', 'compile-html']);
    grunt.registerTask('default', ['init-build-startpage','deploy-desktop-startpage']);
};