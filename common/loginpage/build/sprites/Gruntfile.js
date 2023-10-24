module.exports = function (grunt) {
    require('./sprites')(grunt);

    grunt.registerTask('generate-sprite', ['svg_sprite']);
    grunt.registerTask('default', ['generate-sprite']);
};
