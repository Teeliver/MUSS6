let pending;
export function loadMapCatalog(){return pending??=fetch(new URL('./map-catalog.json',import.meta.url),{cache:'no-store'}).then(response=>{if(!response.ok)throw new Error('Không nạp được danh mục map');return response.json();}).catch(error=>{pending=undefined;throw error;});}

let resources;export function loadResourceCatalog(){return resources??=fetch(new URL('./resource-catalog.json',import.meta.url),{cache:'no-store'}).then(response=>{if(!response.ok)throw new Error('Resource catalog unavailable');return response.json();}).catch(error=>{resources=undefined;throw error;});}
