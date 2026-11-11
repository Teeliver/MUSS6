import { drawingSize } from './viewport.js';
export function createSurface(canvas, onResize = () => {}) {
  const gl = canvas.getContext('webgl2', { alpha:false, antialias:false, stencil:true });
  if (!gl) throw new Error('WebGL2 unavailable');
  let lost=false;
  const draw=()=>{
    if(lost)return;
    const rect=canvas.getBoundingClientRect();
    if(!rect.width||!rect.height)return;
    const [width,height]=drawingSize(rect.width,rect.height,window.devicePixelRatio||1);
    if(canvas.width!==width)canvas.width=width;
    if(canvas.height!==height)canvas.height=height;
    gl.viewport(0,0,width,height);gl.clearColor(.035,.045,.065,1);gl.clear(gl.COLOR_BUFFER_BIT);
    onResize({width,height,dpr:window.devicePixelRatio||1});
  };
  const onLost=e=>{e.preventDefault();lost=true;};
  const onRestored=()=>{lost=false;draw();};
  const observer=new ResizeObserver(draw);observer.observe(canvas);
  canvas.addEventListener('webglcontextlost',onLost);canvas.addEventListener('webglcontextrestored',onRestored);
  window.addEventListener('resize',draw);draw();
  return {draw,dispose(){observer.disconnect();window.removeEventListener('resize',draw);canvas.removeEventListener('webglcontextlost',onLost);canvas.removeEventListener('webglcontextrestored',onRestored);}};
}
