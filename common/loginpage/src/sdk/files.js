import { command } from "./bridge";

const sdk = window.AscDesktopEditor;

export function createDocument(kind) {
  command("create:new", kind);
}

export function openLocal() {
  command("open:folder", "");
}

export function openFolder(path) {
  command("open:folder", path);
}

export function openRecentFile(model) {
  command("open:recent", {
    id: model.fileid,
    name: model.name,
    path: model.path,
    type: model.type,
    cloud: model.cloud,
    hash: model.hash,
    recovery: false,
  });
}

export function openRecoveryFile(model) {
  command("open:recent", {
    id: model.fileid,
    name: model.name,
    path: model.path,
    type: model.type,
    cloud: model.cloud,
    hash: model.hash,
    recovery: true,
  });
}

export function exploreFile(model) {
  command("files:explore", {
    path: model.path,
    id: model.fileid,
    hash: model.hash,
  });
}

export function checkFiles(hashMap) {
  command("files:check", hashMap);
}

export function dropOfficeFiles() {
  sdk.DropOfficeFiles();
}
