import {itemAssetPaths} from '../../client/src/protocol/item-model-bindings.js';
const itemAssets=new Set(itemAssetPaths);
import {uiAssets} from '../../client/src/ui/asset-manifest.js';
import {createHash} from 'node:crypto';
import http from 'node:http';
export function createIdentityHelper({identityProvider,keyProvider,assetProvider,port=9001}){
 if(typeof identityProvider!=='function'||typeof keyProvider!=='function')throw new Error('Missing helper provider');
 let busy=0,requests=0,assetRequests=0,windowStart=Date.now();
 const server=http.createServer(async(req,res)=>{
  res.setHeader('Cache-Control','no-store');res.setHeader('Vary','Origin');
  if(req.headers.origin!=='http://127.0.0.1:8080'||req.headers.host!==`127.0.0.1:${server.address().port}`){res.writeHead(403);res.end();return;}
  res.setHeader('Access-Control-Allow-Origin','http://127.0.0.1:8080');
  let assetPath;try{const url=new URL(req.url,'http://127.0.0.1');if(url.pathname==='/asset'&&[...url.searchParams.keys()].length===1&&url.searchParams.has('path')&&allowedAssetPath(url.searchParams.get('path')))assetPath=url.searchParams.get('path');}catch{}
  if(req.method!=='GET'||(!['/identity','/keys'].includes(req.url)&&!(assetPath&&assetProvider))){res.writeHead(404);res.end();return;}
  if(Date.now()-windowStart>=60000){requests=0;assetRequests=0;windowStart=Date.now();}if(busy>=24||(assetPath?++assetRequests>60000:++requests>3000)){res.setHeader('Retry-After',String(busy>=4?1:Math.max(1,Math.ceil((60000-Date.now()+windowStart)/1000))));res.setHeader('Access-Control-Expose-Headers','Retry-After');res.writeHead(429);res.end();return;}
  busy++;try{
   if(assetPath){const bytes=await assetProvider(assetPath);if(!(bytes instanceof Uint8Array)||bytes.length<1||bytes.length>64*1024*1024)throw new Error('Invalid asset');res.setHeader('Content-Type','application/octet-stream');res.setHeader('X-Asset-SHA256',createHash('sha256').update(bytes).digest('hex'));res.setHeader('Access-Control-Expose-Headers','X-Asset-SHA256');res.end(bytes);return;}
   let value;
   if(req.url==='/identity'){const identity=await identityProvider();if(!/^[0-9A-F]{8}(?:-[0-9A-F]{8}){4}$/.test(identity))throw new Error('Invalid identity');value={identity};}
   else{const keys=await keyProvider();if(!keys||keys.encryption.length!==54||keys.decryption.length!==54)throw new Error('Invalid keys');value={encryption:Buffer.from(keys.encryption).toString('base64'),decryption:Buffer.from(keys.decryption).toString('base64')};}
   res.setHeader('Content-Type','application/json');res.end(JSON.stringify(value));
  }catch{res.writeHead(503);res.end();}finally{busy--;}
 });
 server.requestTimeout=5000;server.headersTimeout=5000;
 return {listen:()=>new Promise((resolve,reject)=>{server.once('error',reject);server.listen(port,'127.0.0.1',()=>resolve(server.address()));}),close:()=>new Promise(resolve=>{server.closeAllConnections();server.close(resolve);})};
}

export function allowedAssetPath(path){if(typeof path!=='string'||path.length>240)return false;if(Object.hasOwn(uiAssets,path)||itemAssets.has(path))return true;return /^(?:Local\/Eng\/Item_eng\.bmd|(?:Item|Player|Skill|Effect|Npc|Monster|Object(?:[1-9]|[1-9][0-9]|1[0-9]{2}|2[0-4][0-9]|25[0-5]))\/[A-Za-z0-9_-][A-Za-z0-9_. -]*\.(?:[bB][mM][dD]|[oO][zZ][jJtT])|World(?:[1-9]|[1-9][0-9]|1[0-9]{2}|2[0-4][0-9]|25[0-5])\/[A-Za-z0-9_-]+\.(?:OZJ|OZT|OZB|map|att|obj))$/.test(path)||/^(?:Npc|Monster)\/(?:[A-Za-z0-9_-]+\/){1,3}[A-Za-z0-9_-][A-Za-z0-9_. -]*\.(?:[bB][mM][dD]|[oO][zZ][jJtT])$/.test(path);}
