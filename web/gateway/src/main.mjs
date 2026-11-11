import { readFile, stat } from 'node:fs/promises';
import { createGateway } from './gateway.mjs';
if(process.version!=='v26.11.1')throw new Error('Use pinned Node 26.11.1');
const file=process.argv[2];
if(!file){console.error('Usage: npm start -- path/to/config.local.json');process.exitCode=1;}
else {
 try {
  if((await stat(file)).size>65536)throw new Error('Config too large');
  const config=JSON.parse(await readFile(file,'utf8'));
  const gateway=createGateway(config);
  const address=await gateway.listen();
  console.log(`MU local gateway listening on ws://127.0.0.1:${address.port}/mu/<route>`);
  let stopping=false;
  const stop=async()=>{if(stopping)return;stopping=true;try{await gateway.close();}catch{process.exitCode=1;}};
  process.once('SIGINT',stop);process.once('SIGTERM',stop);
 }catch{console.error('Gateway configuration or startup failed. No config contents were logged.');process.exitCode=1;}
}
