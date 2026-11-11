import { createSurface } from '../platform/rendering/surface.js';
export function startWebGL(canvas,status) {
  const metrics=document.querySelector('#render-metrics');
  const fps=document.querySelector('#fps');
  try {
    const surface=createSurface(canvas,({width,height,dpr})=>{
      status.textContent='WebGL2 sẵn sàng';
      metrics.textContent=`Buffer ${width} × ${height} · DPR ${dpr}`;
    });
    let frame=0,count=0,start=performance.now();
    const tick=now=>{
      surface.draw();count++;
      if(now-start>=1000){fps.textContent=`FPS: ${(count*1000/(now-start)).toFixed(1)}`;count=0;start=now;}
      frame=requestAnimationFrame(tick);
    };
    const visibility=()=>{
      cancelAnimationFrame(frame);count=0;start=performance.now();
      if(!document.hidden)frame=requestAnimationFrame(tick);
      else fps.textContent='FPS: tạm dừng khi tab ẩn';
    };
    document.addEventListener('visibilitychange',visibility);visibility();
    return ()=>{cancelAnimationFrame(frame);document.removeEventListener('visibilitychange',visibility);surface.dispose();};
  } catch {status.textContent='Không tạo được WebGL2';return ()=>{};}
}
