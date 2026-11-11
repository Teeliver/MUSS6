// Transport-facing API: C++ errors are caught inside WASM before returning to JS.
export function createReader(module) {
  let reader=new module.FrameReader();
  const active=()=>{if(!reader)throw new Error('Reader has been disposed');return reader;};
  const checked=result=>{if(!result.ok)throw new Error(result.error);return result;};
  return {
    feed(bytes){return checked(active().feedResult(bytes)).frames;},
    finish(){checked(active().finishResult());},
    pendingSize(){return active().pendingSize();},
    failed(){return active().failed();},
    delete(){if(reader){reader.delete();reader=undefined;}},
  };
}
