# 🎧 Application Audio Capture (Windows Only)

A simple Node.js wrapper around native C++ binaries to list running application windows and capture audio from specific Windows processes using loopback recording.

>   ⚠️ Requirements:
   This package only runs on Windows 10 x64 and later. It uses native binaries and will throw an error on unsupported platforms.

### 🚀 Features

- List all visible application windows and their process IDs.

- Capture raw PCM audio from individual applications using WASAPI loopback.

- Pipe real-time audio data into your JavaScript/TypeScript app.


https://github.com/user-attachments/assets/fc058596-6ea3-4ded-8065-aeb642c5a465


### 📦 Installation

Install using your favourite package manager.
```sh
npm install application-loopback
#OR
bun install application-loopback
#OR any package manager..
```

### 🧠 Usage
1. Get Active Window Titles and Process IDs

```ts
import { getActiveWindowProcessIds, type Window } from "application-loopback";

const windows = await getActiveWindowProcessIds();

windows.forEach((win: Window) => {
  console.log(`PID: ${win.processId}, Title: ${win.title}`);
});
```

2. Start Capturing Audio from a Process

```ts
import { startAudioCapture } from "your-package-name";

startAudioCapture("1234", {
  onData: (chunk: Uint8Array) => {
    console.log("Audio data:", chunk); // Uint8Array
  },
});
```

>    🧠 chunk is a raw PCM audio buffer. You can pipe it to a file, stream it, analyze it, etc.

3. Stop Capturing Audio

```ts
import { stopAudioCapture } from "your-package-name";

const processId = startAudioCapture("1234");
//       ^
//     "1234"

const stopped = stopAudioCapture("1234");

if (stopped) {
  console.log("Audio capture stopped.");
} else {
  console.log("No capture process found for that PID.");
}
```

### 🪟 Why and how?

For my desktop application ([Huginn](https://github.com/WerdoxDev/Huginn)) I needed to capture an application's audio selectively to share in a call just like discord does.

I did a lot of tries with NAPI but windows WASPI was just not having it... That's when it hit me... I can compile a normal C++ application that does the audio capture and basically keep dumping the output to stdout and simply read it from nodejs. Yea that took a couple weeks to figure out 😶

The C++ application is simply a stripped out version from a sample in microsoft's classic samples repo
https://github.com/microsoft/Windows-classic-samples

### 🧪 Example Use Cases

- Build a real-time audio visualizer for specific apps.

- Record browser or game audio selectively.

- Stream audio from only one process instead of the whole system.
