import {startSound} from '../audio/game-audio.js';
import {startOfflineUi} from '../ui/offline-ui.js';
import {startTerrainCheck} from '../world/offline-check.js';
import {startLiveServers} from './live-servers.js';
import {createPreviewMenu} from '../preview/menu.js';
import { startWebGL } from '../diagnostics/webgl.js';
import { checkFraming } from '../diagnostics/framing.js';
import { loadCore } from '../platform/wasm/load-core.js';
import { startPreview } from '../preview/start-preview.js';
const fatal=message=>{let el=document.querySelector('#mu-fatal');if(!el){el=document.createElement('output');el.id='mu-fatal';el.style.cssText='position:fixed;left:8px;right:8px;bottom:8px;z-index:99;padding:8px;background:#3a0d0d;color:#ffb3b3;border:1px solid #a33;font:12px monospace;white-space:pre-wrap';document.body.append(el);}el.textContent=String(message);};
window.addEventListener('error',event=>fatal('Lỗi: '+(event.error?.stack||event.message||'unknown')));
window.addEventListener('unhandledrejection',event=>fatal('Promise lỗi: '+(event.reason?.stack||event.reason?.message||event.reason||'unknown')));
const stopSound=startSound();
const stopDiagnostics=startWebGL(document.querySelector('canvas'), document.querySelector('#status'));
let stopPreview,stopMenu,stopLive;
window.addEventListener('pagehide',()=>{stopSound();stopDiagnostics();stopPreview?.();stopMenu?.();stopLive?.();},{once:true});
const coreStatus = document.querySelector('#core-status');
if (location.protocol === 'file:') {
  coreStatus.textContent = 'Hãy dùng npm run serve và mở http://127.0.0.1:8080; file:// không dùng để kiểm tra module WASM.';
} else {
  try {
    const core = await loadCore();
    const result = checkFraming(core);
    coreStatus.textContent = `WASM: đạt · ${result.frames} loại frame / ${result.splitCases} split`;
    if(new URLSearchParams(location.search).get('scene')==='ui-check'){stopDiagnostics();stopLive=await startOfflineUi(core);}
    else if(new URLSearchParams(location.search).get('scene')==='terrain-check'){stopDiagnostics();stopLive=await startTerrainCheck(core);}
    else if(new URLSearchParams(location.search).get('scene')==='servers'){stopDiagnostics();document.querySelector('#fps').textContent='FPS: scene 3D chưa nạp';document.querySelector('#status').textContent='Kiểm thử kết nối ConnectServer thật';stopLive=await startLiveServers(core);}
    else if(new URLSearchParams(location.search).get('preview')==='models'){
      stopDiagnostics();stopMenu=await createPreviewMenu();
      try{stopPreview=await startPreview(document.querySelector('canvas'),core,message=>{document.querySelector('#status').textContent=message;});}
      catch(error){document.querySelector('#status').textContent=`Preview lỗi: ${error.message}`;}
    }
  } catch (error) {
    coreStatus.textContent = `WASM chưa sẵn sàng: ${error instanceof Error ? error.message : 'Không nạp hoặc chạy được core'}.`;
  }
}
