/*
 * Copyright (c) Ascensio System SIA 2016
 *
 */

function Component()
{
    // Skipping components selection page because our installer consists only of single component
    installer.setDefaultPageVisible(QInstaller.ComponentSelection, false);
}

Component.prototype.createOperations = function()
{
    // Post installation script
    component.createOperations();
    component.addElevatedOperation("Execute", "/bin/bash", installer.value("TargetDir") + "/register-in-system.sh", installer.value("TargetDir"));
}