/*
 * Copyright (c) Ascensio System SIA, 2016
 *
 */

function Controller()
{
    installer.uninstallationStarted.connect(this, Controller.prototype.uninstallStarted);
    installer.uninstallationFinished.connect(this, Controller.prototype.uninstallFinished);
}

Controller.prototype.PerformInstallationPageCallback = function()
{
}

Controller.prototype.uninstallStarted = function()
{
    console.log("=== uninstallStarted");
    // Post uninstall script will be removed during uninstallation, so need to make backup
    installer.execute("cp", [installer.value("TargetDir") + "/unregister-in-system.sh", "/tmp"]);
}

Controller.prototype.uninstallFinished = function()
{
    console.log("=== uninstallFinished");
    installer.gainAdminRights();
    installer.execute("bash", ["/tmp/unregister-in-system.sh", installer.value("TargetDir"), "Ascensio System SIA"]);
    installer.execute("rm",   ["/tmp/unregister-in-system.sh"]);
}

Controller.prototype.TargetDirectoryPageCallback = function()
{
    var widget = gui.currentPageWidget();
    widget.TargetDirectoryLineEdit.textChanged.connect(this, Controller.prototype.targetChanged);
    Controller.prototype.targetChanged(widget.TargetDirectoryLineEdit.text);
}

Controller.prototype.targetChanged = function (text)
{
    // This code allows overwrite existing directory
    installer.setValue("RemoveTargetDir", true);
    if (text != "" && installer.fileExists(text + "/components.xml"))
    {
        installer.setValue("RemoveTargetDir", false);
    }
}
