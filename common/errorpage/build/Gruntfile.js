module.exports = function(grunt) {
    grunt.loadNpmTasks('grunt-contrib-clean');
    grunt.loadNpmTasks('grunt-contrib-htmlmin');
    grunt.loadNpmTasks('grunt-text-replace');
    grunt.loadNpmTasks('grunt-svgmin');
    grunt.loadNpmTasks('grunt-terser');

    grunt.registerTask('desktop-app-extra', function() {
        grunt.initConfig({
            terser: {
                options: {
                    format: {
                        comments: false,
                    },
                    compress: {
                        drop_console: true,
                    },
                },
                core: {
                    src: ['../src/locale.js', '../src/code.js'],
                    dest: '../deploy/code.min.js',
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
                        '../deploy/index.html': '../src/index.html.deploy'
                    }
                }
            },

            clean: {
                options: {
                    force: true
                },
                files: ["../deploy/*.png"]
            },

            svgmin: {
                options: {
                    plugins: [
                        {
                            name: 'preset-default',
                            params: {
                                overrides: {
                                    sortAttrs: false,
                                    removeUselessDefs: false,
                                    cleanupIds: false,
                                }
                            }
                        }
                    ]
                },
                dist: {
                    files: [{
                        expand: true,
                        cwd: "../res",
                        src: ['*.svg'],
                        dest: '../deploy',
                    }]
                }
            },
        });
    });

    grunt.registerTask('compile-html', function(){
        if ( !grunt.option('external-image') ) {
            grunt.config('replace.insert-css', {
                    src: ['../deploy/error.html'],
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
                            console.log('replace js ', regexMatches[1]);
                            if (!grunt.file.exists('../deploy/' + regexMatches[1])) {
                                grunt.log.error().writeln('file does not exists: ' + regexMatches[1]);
                            }

                            var script = grunt.file.read('../deploy/' + regexMatches[1]);
                            return '<script>' + script + '</script>';
                        }
                    },{
                        from: /(\<img\s[^\>]+inline-svg[^\>]+"([\w\S\.]+\.svg)\"\>)/g,
                        to: function(matchedWord, index, fullText, regexMatches) {
                            console.log('replace svg ', regexMatches[1]);
                            if (!grunt.file.exists('../deploy/' + regexMatches[1])) {
                                grunt.log.error().writeln('file does not exists: ' + regexMatches[1]);
                            }

                            const svgsource = grunt.file.read('../deploy/' + regexMatches[1]);
                            return svgsource;
                        }
                    }]
            });

            grunt.config('clean.files', ['../deploy/*.js','../deploy/*.css','../deploy/*.svg']);
            grunt.task.run('replace:insert-css', 'clean');
        }
    });

    grunt.registerTask('init-build-startpage', 'Initialize build Desktop connection error page', function(){
        grunt.log.ok('Desktop connection error page config loaded successfully'.green);
    });

    grunt.registerTask('deploy-desktop-startpage', ['desktop-app-extra', 'clean', 'terser', 'htmlmin', 'svgmin', 'compile-html']);
    grunt.registerTask('default', ['init-build-startpage','deploy-desktop-startpage']);
};