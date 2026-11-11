// Generated module is served only from dist/generated, never committed as source.
let pending;
export function loadCore() {
  if (!pending) {
    pending = import('../../../generated/frame-reader.mjs')
      .then(({ default: createModule }) => createModule())
      .catch((error) => { pending = undefined; throw error; });
  }
  return pending;
}
