import { createSurface } from '../platform/rendering/surface.js';
import { loadCore } from '../platform/wasm/load-core.js';
try {
  createSurface(document.querySelector('canvas'));
  await loadCore();
} catch {
  document.querySelector('#startup-error').hidden=false;
}
