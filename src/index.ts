import { type ChildProcessWithoutNullStreams, spawn } from "node:child_process";
import { arch, platform } from "node:os";
import path from "node:path";

if (platform() !== "win32" || arch() !== "x64") {
   throw new Error("This package is currently only available for Windows 10 x64 and later");
}

const loopbackBinaryPath = path.resolve(__dirname, "../", "bin", `${platform()}-${arch()}`, "ApplicationLoopback.exe");
const processListBinaryPath = path.resolve(__dirname, "../", "bin", `${platform()}-${arch()}`, "ProcessList.exe");

export type Window = {
   processId: string;
   title: string;
}

/**
 * Retrieves a list of active window process IDs and their titles by spawning an external binary.
 * @returns A promise that resolves to an array of `Window` objects, each containing a `processId` and a `title`.
 * @example
 * const windows = await getActiveWindowProcessIds();
 * windows.forEach(win => {
 *   console.log(win.processId, win.title);
 * });
 */
export async function getActiveWindowProcessIds(): Promise<Window[]> {
   const cppProcess = spawn(processListBinaryPath, { detached: true, stdio: "pipe" });
   cppProcess.stdout.setEncoding("utf8");

   return new Promise<Window[]>((r) => {
      const processes: Window[] = [];

      cppProcess.stdout.on("data", (d: string) => processes.push(
         ...d.split("\n").map(x => {
            const [processId, title] = x.replace("\r", "").split(";");

            if (processId && title) {
               return { processId, title };
            }
            return undefined;
         }).filter(x => x !== undefined))
      )

      cppProcess.stdout.on("close", () => {
         r(processes)
      })
   });
}

const spawnedAudioCaptures: Map<string, ChildProcessWithoutNullStreams> = new Map();

/**
 * Starts capturing audio for a given process ID by spawning an external binary.
 *
 * @param processId - The unique identifier for the process whose audio should be captured.
 * @param options - Configuration options for the audio capture.
 * @param options.onData - Optional callback invoked with audio data as a `Uint8Array` whenever new data is available.
 * @throws {Error} If an audio capture for the specified `processId` is already running.
 * @returns The `processId` for which audio capture has started.
 */
export function startAudioCapture(processId: string, options: { onData?: (data: Uint8Array) => void }) {
   if (spawnedAudioCaptures.has(processId)) {
      throw new Error(`An audio capture with process id of ${processId} is already started`);
   }

   const cppProcess = spawn(`${path.resolve(__dirname, loopbackBinaryPath)}`, [processId], { detached: true, stdio: "pipe" });

   spawnedAudioCaptures.set(processId, cppProcess)

   cppProcess.stdout.on("data", (d) => {
      options.onData?.(d);
   })

   return processId;
}


/**
 * Stops the audio capture process associated with the given process ID.
 * @param processId - The unique identifier of the audio capture process to stop.
 * @returns `true` if the process was found and stopped, otherwise `false`.
 */
export function stopAudioCapture(processId: string): boolean {
   const cppProcess = spawnedAudioCaptures.get(processId);

   if (cppProcess) {
      cppProcess.kill()
      spawnedAudioCaptures.delete(processId);
      return true;
   }

   return false;
}
