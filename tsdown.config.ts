import { defineConfig } from "tsdown";

export default defineConfig({
  entry: ["js/index.ts"],
  format: ["cjs", "esm"],
  clean: true,
  minify: true,
});
