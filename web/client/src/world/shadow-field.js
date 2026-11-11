// Terrain-height sampler shared by all world renderers. No CPU shadow mesh copies.
export function createShadowField(gl,terrain,entry){
 const texture=gl.createTexture();if(!texture)throw new Error('Shadow height allocation failed');
 try{const heights=new Float32Array(65536);for(let y=0;y<256;y++)for(let x=0;x<256;x++)heights[y*256+x]=terrain.height(x,y);
 gl.activeTexture(gl.TEXTURE1);gl.bindTexture(gl.TEXTURE_2D,texture);gl.texImage2D(gl.TEXTURE_2D,0,gl.R32F,256,256,0,gl.RED,gl.FLOAT,heights);gl.texParameteri(gl.TEXTURE_2D,gl.TEXTURE_MIN_FILTER,gl.NEAREST);gl.texParameteri(gl.TEXTURE_2D,gl.TEXTURE_MAG_FILTER,gl.NEAREST);gl.texParameteri(gl.TEXTURE_2D,gl.TEXTURE_WRAP_S,gl.CLAMP_TO_EDGE);gl.texParameteri(gl.TEXTURE_2D,gl.TEXTURE_WRAP_T,gl.CLAMP_TO_EDGE);gl.activeTexture(gl.TEXTURE0);
 if(gl.getError()!==gl.NO_ERROR)throw new Error('Shadow height upload failed');let disposed=false;
 return {texture,entry:[entry.x,entry.y,terrain.height(entry.x,entry.y)],sx:entry.map===30?25:20,dispose(){if(!disposed){disposed=true;gl.deleteTexture(texture);}}};
 }catch(error){gl.deleteTexture(texture);gl.activeTexture(gl.TEXTURE0);throw error;}
}
// Reference projection from ZzzBMD.cpp CalcShadowPosition, units converted /100.
export function projectShadow(point,origin,heightAt,sx=20){const z=point[2]-origin[2],denominator=z-40;const x=point[0]+z*(point[0]-origin[0]+sx)/(Math.abs(denominator)<.25?-.25:denominator);return [x,point[1],heightAt(x,point[1])+.05];}
