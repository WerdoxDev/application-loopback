import addon from "./";

const mode = process.argv[2]; // "system" or a numeric processId

const capture = new addon.LoopbackCapture();
let totalBytes = 0;

const onData = (chunk: Buffer) => {
  // chunk is a Buffer of raw 16-bit PCM, 2 channels, 48000 Hz.
  totalBytes += chunk.length;
  console.log(`received ${chunk.length} bytes (total: ${totalBytes})`);
};

if (mode === "system") {
  // Capture everything going out through the default playback device.
  capture.startSystemAudio(onData);
} else {
  const targetProcessId = Number(mode);
  if (!targetProcessId) {
    console.error("Usage: node index.js <processId>   OR   node index.js system");
    process.exit(1);
  }
  capture.start(targetProcessId, /* includeProcessTree */ true, onData);
}

process.on("SIGINT", () => {
  capture.stop();
  process.exit(0);
});
