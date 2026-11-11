// Transport adapter only. All MU cipher/session policy must be supplied explicitly.
export class TransportSession {
  #socket; #reader; #connectTimer; #partialTimer; #state='idle';
  constructor({createSocket,createReader,decodeTransport,encodeTransport,onFrame=()=>{},onState=()=>{},
    timers=globalThis,deadlineMs=5000,maxBytes=262144}) {
    for(const fn of [createSocket,createReader,decodeTransport,encodeTransport,onFrame,onState])
      if(typeof fn!=='function')throw new TypeError('Missing transport dependency');
    if(!Number.isSafeInteger(deadlineMs)||deadlineMs<=0||!Number.isSafeInteger(maxBytes)||maxBytes<=0||maxBytes>262144)
      throw new RangeError('Invalid transport limits');
    this.options={createSocket,createReader,decodeTransport,encodeTransport,onFrame,onState,timers,deadlineMs,maxBytes};
  }
  get state(){return this.#state;}
  connect(url) {
    if(this.#state!=='idle')throw new Error('Create a new session to connect again');
    const target=new URL(url);
    if(!['ws:','wss:'].includes(target.protocol)||target.username||target.password||target.hash)
      throw new Error('Invalid WebSocket URL');
    if(target.protocol==='ws:'&&!['127.0.0.1','localhost','[::1]'].includes(target.hostname))
      throw new Error('Plain WebSocket is restricted to loopback');
    const o=this.options;
    try {
      this.#reader=o.createReader();this.#socket=o.createSocket(target.href);
      this.#socket.binaryType='arraybuffer';this.#transition('connecting');
      if(this.#terminal())return;
      this.#connectTimer=o.timers.setTimeout(()=>this.#fail('connect-timeout'),o.deadlineMs);
      this.#socket.addEventListener('open',()=>{
        if(this.#state!=='connecting')return;
        o.timers.clearTimeout(this.#connectTimer);this.#transition('open');
      });
      this.#socket.addEventListener('message',e=>this.#receive(e.data));
      this.#socket.addEventListener('error',()=>this.#fail('socket-error'));
      this.#socket.addEventListener('close',()=>{
        if(this.#terminal())return;
        try {this.#reader.finish();this.#end('closed','peer-closed');}
        catch {this.#fail('incomplete-or-invalid-stream');}
      });
    } catch {this.#fail('initialization-error');}
  }
  send(bytes) {
    if(this.#state!=='open')throw new Error('Session is not open');
    try {
      this.#check(bytes);
      const encoded=this.options.encodeTransport(bytes.slice());this.#check(encoded);
      if(this.#socket.bufferedAmount+encoded.byteLength>this.options.maxBytes)throw new Error('Send queue exceeded');
      this.#socket.send(encoded.slice().buffer);
    } catch {this.#fail('outbound-error');throw new Error('Transport send failed');}
  }
  close(){if(!this.#terminal())this.#end('closed','local-close');}
  #terminal(){return this.#state==='closed'||this.#state==='failed';}
  #check(bytes){if(!(bytes instanceof Uint8Array)||bytes.byteLength>this.options.maxBytes)throw new Error('Invalid byte buffer');}
  #receive(data) {
    if(this.#state!=='open')return;
    try {
      if(!(data instanceof ArrayBuffer))throw new Error('Binary ArrayBuffer required');
      const input=new Uint8Array(data);this.#check(input);
      const decoded=this.options.decodeTransport(input);this.#check(decoded);
      const frames=this.#reader.feed(decoded);
      if(this.#reader.pendingSize()>this.options.maxBytes)throw new Error('Receive queue exceeded');
      if(!this.#reader.pendingSize()||frames.length){this.options.timers.clearTimeout(this.#partialTimer);this.#partialTimer=undefined;}
      // Keep the original deadline for a partial frame even when more bytes drip in.
      if(this.#reader.pendingSize()&&this.#partialTimer===undefined)
        this.#partialTimer=this.options.timers.setTimeout(()=>this.#fail('partial-frame-timeout'),this.options.deadlineMs);
      for(const frame of frames){if(this.#state!=='open')break;this.options.onFrame(frame);}
    } catch(error) {console.warn('MU transport inbound:',error.message);this.#fail('inbound-error');}
  }
  #transition(state,reason){this.#state=state;this.#notify({state,reason});}
  #notify(event){try{this.options.onState(event);}catch{if(!this.#terminal())this.#fail('state-callback-error');}}
  #fail(reason){if(!this.#terminal())this.#end('failed',reason);}
  #end(state,reason) {
    this.options.timers.clearTimeout(this.#connectTimer);this.options.timers.clearTimeout(this.#partialTimer);
    this.#state=state;
    const reader=this.#reader;this.#reader=undefined;
    try {reader?.delete();} finally {
      try {this.#socket?.close();} finally {this.#notify({state,reason});}
    }
  }
}
