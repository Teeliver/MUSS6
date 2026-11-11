// Bounded ring: inserting frame measurements never shifts or grows an array.
export function frameSamples(capacity){
 if(!Number.isInteger(capacity)||capacity<1||capacity>36000)throw new Error('Invalid sample capacity');
 const values=new Float64Array(capacity);let count=0,cursor=0;
 return {
  get length(){return count;},
  push(value){if(!Number.isFinite(value)||value<0)throw new Error('Invalid frame sample');values[cursor]=value;cursor=(cursor+1)%capacity;count=Math.min(count+1,capacity);},
  clear(){count=cursor=0;},
  percentile(fraction){if(!Number.isFinite(fraction)||fraction<=0||fraction>1)throw new Error('Invalid percentile');if(!count)return undefined;const sorted=values.slice(0,count).sort();return sorted[Math.ceil(count*fraction)-1];},
 };
}
