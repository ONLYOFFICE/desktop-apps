#!/usr/bin/env python

import os
import sys

SCRIPT_DIR = os.path.dirname(os.path.realpath(__file__))
LOGINPAGE_DIR = os.path.abspath(os.path.join(SCRIPT_DIR, ".."))
ROOT_DIR = os.path.abspath(os.path.join(LOGINPAGE_DIR, "..", "..", ".."))
BUILD_TOOLS_SCRIPTS_DIR = os.path.join(ROOT_DIR, "build_tools", "scripts")
TEMPLATES_PROJECT_DIR = os.path.join(ROOT_DIR, "oforms.onlyoffice.com", "embed")

sys.path.insert(0, BUILD_TOOLS_SCRIPTS_DIR)
import base

# order matters: loginpage itself must be built last, since its build
# depends on addondialog/noconnect deploy artifacts
PROJECTS = [
  os.path.join(LOGINPAGE_DIR, "addondialog"),
  os.path.join(LOGINPAGE_DIR, "noconnect"),
  LOGINPAGE_DIR,
]

def npm_install(directory):
  ret = base.cmd_in_dir(directory, "npm", ["install"], True)
  if 0 != ret:
    ret = base.cmd_in_dir(directory, "npm", ["install", "--verbose"])
  return ret

def npm_build(directory):
  return base.cmd_in_dir(directory, "npm", ["run", "build"])

def build_project(directory):
  base.print_info("Building: " + directory)
  npm_install(directory)
  npm_build(directory)
  return

def build_templates_project(directory):
  base.set_env("EMBED_CMS_URL", "https://templates-update.teamlab.info/dashboard")

  ret = base.cmd_in_dir(directory, "npm", ["run", "install:all"])
  ret = base.cmd_in_dir(directory, "npm", ["run", "build"])

  if ret == 0:
    base.copy_file(directory + "/dist/oforms.js", LOGINPAGE_DIR + "/templatespanel/oforms.js")
    base.copy_file(directory + "/dist/oforms.css", LOGINPAGE_DIR + "/templatespanel/oforms.css")
  return ret

def main():
  base.print_info("Build projects for the desktop's Start page")
  build_templates_project(TEMPLATES_PROJECT_DIR)
  for directory in PROJECTS:
    build_project(directory)
  base.print_info("All projects built successfully")
  return

if __name__ == "__main__":
  main()
