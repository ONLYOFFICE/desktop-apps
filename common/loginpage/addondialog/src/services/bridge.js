// adapted from loginpage/src/sdk/bridge.js + events.js, trimmed to what this
// dialog needs: sending commands to native and listening for on_native_message.
const sdk = window.AscDesktopEditor ?? { execCommand: () => {} };

const subscribers = {};

sdk.on = function (type, fn) {
  if (!subscribers[type]) subscribers[type] = [];
  subscribers[type].push(fn);
};

sdk.remove = function (type, fn) {
  if (subscribers[type]) {
    subscribers[type] = subscribers[type].filter((f) => f !== fn);
  }
};

sdk.fire = function (type, ...args) {
  (subscribers[type] || []).forEach((fn) => fn(...args));
};

window.on_native_message = function (...args) {
  sdk.fire("on_native_message", ...args);
};

export function command(name, param) {
  sdk.execCommand(name, typeof param === "string" ? param : JSON.stringify(param));
}

export function onNativeMessage(cb) {
  sdk.on("on_native_message", cb);
  return () => sdk.remove("on_native_message", cb);
}
