const arg = process.argv;
const PLATFORM = arg[2];
const ORG = arg[3];
const PACKAGE = arg[4];

var generateEvb = require('generate-evb');

generateEvb('DesktopEditorsPortable.evb',
            '../../../../build_tools/out/' + PLATFORM + '/' + ORG + '/' + PACKAGE + '/DesktopEditors.exe',
            'portable/DesktopEditorsPortable.exe',
            '../../../../build_tools/out/' + PLATFORM + '/' + ORG + '/' + PACKAGE);
