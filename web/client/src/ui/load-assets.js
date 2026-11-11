import {uiAssets} from './asset-manifest.js';import {fetchAsset} from '../platform/assets/fetch-asset.js';import {createAssetQueue} from '../platform/assets/asset-queue.js';import {inspectTexture} from '../platform/assets/decode-texture.js';import {decodeItemTable,decodeTextTable} from './tables.js';
export async function loadUiAssets(core,signal){const images=new Map(),owned=[];const queue=createAssetQueue(async path=>{const spec=uiAssets[path],response=await fetchAsset('http://127.0.0.1:9001/asset?path='+encodeURIComponent(path),{signal,credentials:'omit',cache:'no-store'});if(!response.ok)throw Error('UI asset '+path);const bytes=new Uint8Array(await response.arrayBuffer()),digest=Array.from(new Uint8Array(await crypto.subtle.digest('SHA-256',bytes)),v=>v.toString(16).padStart(2,'0')).join('');if(bytes.length!==spec.bytes||digest!==spec.sha256)throw Error('UI baseline mismatch '+path);return bytes;});
 try{
  let catalog,texts,next=0,failure;const entries=Object.entries(uiAssets);
  // Bound both fetching and decoding; wait for active workers before cleaning
  // images on failure so no late decoder can add an image after disposal.
  const worker=async()=>{while(!failure&&next<entries.length){const [path,spec]=entries[next++];try{
   const bytes=await queue(path);
   if(spec.name==='Item'){catalog=decodeItemTable(bytes);continue;}
   if(spec.name==='Text'){texts=decodeTextTable(bytes);continue;}
   const raw=inspectTexture(core,path.toUpperCase().endsWith('.OZT')?'OZT':'OZJ',bytes),image=raw.jpeg?await createImageBitmap(new Blob([raw.bytes],{type:'image/jpeg'}),{premultiplyAlpha:'none',colorSpaceConversion:'none'}):await createImageBitmap(new ImageData(new Uint8ClampedArray(raw.bytes),raw.width,raw.height));
   owned.push(image);images.set(spec.name,image);
  }catch(error){failure??=error;}}};
  await Promise.all(Array.from({length:3},worker));if(failure)throw failure;
  return {images,catalog,texts,dispose(){for(const image of owned)image.close();}};
 }catch(error){for(const image of owned)image.close();throw error;}finally{queue.close();}}
