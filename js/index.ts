import binding from "bindings";
import { arch, platform } from "node:os";

export type LoopbackCapture = {
  start: (
    processId: number,
    includeProcessTree: boolean,
    callback: (chunk: Buffer) => void,
  ) => void;
  stop: () => void;
  startSystemAudio: (callback: (chunk: Buffer) => void) => void;
};

export type Addon = {
  LoopbackCapture: {
    new (): LoopbackCapture;
  };
};

if (platform() !== "win32" || arch() !== "x64") {
  console.warn("This package is currently only available for Windows 10 x64 and later");
}

const addon: Addon = binding({
  try: [
    ["module_root", "build", "Release", "loopback_capture_addon.node"],
    ["loopback-capture", "build", "Release", "loopback_capture_addon.node"],
  ],
});

export default { ...addon };
