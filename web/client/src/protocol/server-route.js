// Received address/port is metadata to verify, never a browser-supplied TCP target.
export function selectServerRoute(code,endpoint,routes) {
 if(!Number.isInteger(code)||code<0||code>255)throw new Error('Unsupported selection code');
 const route=Object.hasOwn(routes,code)?routes[code]:undefined;
 if(!route||route.address!==endpoint.address||route.port!==endpoint.port)throw new Error('Unlisted GameServer endpoint');
 if(!/^[a-z0-9-]{1,32}$/.test(route.id))throw new Error('Invalid fixed gateway route');
 return `/mu/${route.id}`;
}
