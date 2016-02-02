module.exports = function(grunt) {
    var _ = require('lodash'),
        defaultConfig,
        packageFile;


    grunt.loadNpmTasks('grunt-contrib-clean');
    grunt.loadNpmTasks('grunt-contrib-copy');
    grunt.loadNpmTasks('grunt-contrib-uglify');
    grunt.loadNpmTasks('grunt-contrib-less');
    grunt.loadNpmTasks('grunt-contrib-concat');
    // grunt.loadNpmTasks('grunt-contrib-imagemin');
    // grunt.loadNpmTasks('grunt-contrib-cssmin');
    grunt.loadNpmTasks('grunt-text-replace');
    grunt.loadNpmTasks('grunt-usemin');
    // grunt.loadNpmTasks('grunt-mocha');

    function doRegisterTask(name, callbackConfig) {
        return grunt.registerTask(name + '-init', function() {
            var additionalConfig = {},
                initConfig = {};

            if (_.isFunction(callbackConfig)) {
                additionalConfig = callbackConfig.call(this, defaultConfig, packageFile);
            }

            if (!_.isUndefined(packageFile[name]['clean'])) {
                initConfig['clean'] = {
                    options: {
                        force: true
                    },
                    files: packageFile[name]['clean']
                }
            }

            if (!_.isUndefined(packageFile[name]['copy'])) {
                initConfig['copy'] = packageFile[name]['copy'];
            }

            grunt.initConfig(_.assign(initConfig, additionalConfig || {}));
        });
    }

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

    grunt.registerTask('init-config', 'Initialize build script', function() {
        var exec = require('child_process').exec,
            done = this.async(),
            commandsRef = 0;

        function doneTask() {
            if (--commandsRef <= 0) {
                done(true);
            }
        }

        function doCommand(command, callback) {
            commandsRef++;
            exec(command, callback);
        }

        doCommand('hg log -r -1 --template "{node|short}"', function(error, stdout, stderr) {
            if (error) {
                grunt.log.writeln('Error: ' + error);
            } else {
                revisionHash = stdout;
            }

            doneTask();
        });

        doCommand('hg log -r -1 --template "{date|isodate}"', function(error, stdout, stderr) {
            if (error) {
                grunt.log.writeln('Error: ' + error);
            } else {
                revisionTimeStamp = stdout;
            }

            doneTask();
        });
    });

    grunt.initConfig({
        mocha: {
            test: {
                options: {
                    reporter: 'Spec'
                },
                src: [
                    '../test/common/index.html'
                ]
            }
        },

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

    grunt.registerTask('main-app-init', function() {
        grunt.initConfig({
            pkg: grunt.file.readJSON(defaultConfig),

            clean: {
                options: {
                    force: true
                },
                files: packageFile['main']['clean']
            },

            less: {
                production: {
                    options: {
                        compress: true
                    },
                    files: {
                        "<%= pkg.main.less.files.dest %>": packageFile['main']['less']['files']['src']
                    }
                },
                uriPostfix: {
                    options: {
                        compress: true,
                        ieCompat: false
                    },
                    files: {
                        "<%= pkg.main.less.files.dest %>": "<%= pkg.main.less.files.dest %>"
                    }
                }
            },

            requirejs: {
                compile: {
                    options: packageFile['main']['js']['requirejs']['options']
                }
            },

            replace: {
                fixLessUrl: {
                    src: ['<%= pkg.main.less.files.dest %>'],
                    overwrite: true,
                    replacements: packageFile['main']['less']['replacements']
                },
                urlToUri: {
                    src: ['<%= pkg.main.less.files.dest %>'],
                    overwrite: true,
                    replacements: [{
                        from: /url\(([^\)\'\"]+)/g,
                        to: function(matchedWord, index, fullText, regexMatches) {
                            return 'data-uri(\'' + regexMatches + '\'';
                        }
                    },{
                        from: /filter\:\s?alpha\(opacity\s?=\s?[0-9]{1,3}\)\;/g,
                        to: ''
                    }]
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
                    ' * Version: <%= pkg.version %> (build:<%= pkg.build %>, rev:' + revisionHash + ', date:' + revisionTimeStamp + ')\n' +
                    ' */\n'
                },
                dist: {
                    src: [packageFile['main']['js']['requirejs']['options']['out']],
                    dest: packageFile['main']['js']['requirejs']['options']['out']
                }
            },

            imagemin: {
                options: {
                    optimizationLevel: 3
                },
                dynamic: {
                    files: []
                        .concat(packageFile['main']['imagemin']['images-app'])
                        .concat(packageFile['main']['imagemin']['images-common'])
                }
            },

            copy: {
                localization: {
                    files: packageFile['main']['copy']['localization']
                },
                help: {
                    files: packageFile['main']['copy']['help']
                },
                'index-page': {
                    files: packageFile['main']['copy']['index-page']
                }
            }
        });
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

            uglify: {
                options: {
                    mangle: {
                        sort:true
                    },
                    mangleProperties: true,
                    compress: {
                        unused:true,
                        drop_console: true
                    }
                },
                my_target: {
                    files: {
                        '../deploy/login_min.js' : ['../deploy/login.js'],
                        '../deploy/locale_min.js' : ['../deploy/locale.js']
                    }
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
                    src: '../deploy/index.html',
                    overwrite: true,
                    replacements: [{
                        from: /(\<link[^\<]+stylesheet[^\<]+href="(\w+\.css)\"\>)/,
                        to: function(matchedWord, index, fullText, regexMatches) {
                            var css = grunt.file.read('../deploy/' + regexMatches[1]);
                            return '<style type="text/css">' + css + '</style>';
                        }
                    },{
                        from: /(\<script\s[^\>]+replace[^\>]+"([\w\.]+\.js)\"\>\<\/script\>)/g,
                        to: function(matchedWord, index, fullText, regexMatches) {
                            if (!grunt.file.exists('../deploy/' + regexMatches[1])) {
                                grunt.log.error().writeln('file does not exists: ' + regexMatches[1]);
                            } 

                            var script = grunt.file.read('../deploy/' + regexMatches[1]);
                            return '<script>' + script + '</script>';
                        }
                    }]
            });

            grunt.config('clean.files', ['../deploy/*.js','../deploy/*.css']);
            grunt.task.run('replace:insert-css', 'clean');
        }
    });


    grunt.registerTask('deploy-api',                    ['api-init', 'clean', 'copy']);
    grunt.registerTask('deploy-requirejs',              ['requirejs-init', 'clean', 'uglify']);
    // grunt.registerTask('deploy-app-main',               ['main-app-init', 'clean', 'less:production', 'replace:fixLessUrl', 'concat', 'imagemin', 'copy']);


    doRegisterInitializeAppTask('startpage',       'Desktop start page',       'startpage.json');


    grunt.registerTask('deploy-app', 'Deploy application.', function(){
        if (packageFile) {
            if (packageFile['tasks']['deploy'])
                grunt.task.run(packageFile['tasks']['deploy']);
            else
                grunt.log.error().writeln('Not found "deploy" task in configure'.red);
        } else {
            grunt.log.error().writeln('Is not load configure file.'.red);
        }
    });

    grunt.registerTask('deploy-desktop-startpage', ['desktop-app-extra', 'copy', 'less', 'clean', 'uglify', 'compile-html']);

    grunt.registerTask('default', ['init-build-startpage','deploy-desktop-startpage']);
};