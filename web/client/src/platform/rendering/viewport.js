// Allocation bound for the diagnostic canvas; not a gameplay camera policy.
export function drawingSize(width, height, dpr) {
  const finite = (n) => Number.isFinite(n) && n > 0;
  if (![width, height, dpr].every(finite)) throw new RangeError('Invalid viewport');
  const scale = Math.min(dpr, 2, 4096 / width, 4096 / height);
  return [Math.max(1, Math.floor(width * scale)), Math.max(1, Math.floor(height * scale))];
}

// HUD keeps its 640:51 aspect and stops growing at 960 CSS pixels on wide windows.
export function gameUiLayout(width,height){if(![width,height].every(n=>Number.isFinite(n)&&n>0))throw new RangeError('Invalid UI viewport');const panelScale=Math.min(width/640,height/480),hudScale=Math.min(panelScale,1.5,width*.9/640),hudWidth=640*hudScale;return {panelScale,hudScale,hudWidth,hudHeight:51*hudScale,left:(width-hudWidth)/2};}
