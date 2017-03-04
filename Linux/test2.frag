#version 410 core

uniform vec3      iResolution;           // viewport resolution (in pixels)
uniform float     iGlobalTime;           // shader playback time (in seconds)
uniform float     iTimeDelta;            // render time (in seconds)
uniform int       iFrame;                // shader playback frame
uniform float     iChannelTime[4];       // channel playback time (in seconds)
uniform vec3      iChannelResolution[4]; // channel resolution (in pixels)
uniform vec4      iMouse;                // mouse pixel coords. xy: current (if MLB down), zw: click
//uniform samplerXX iChannel0..3;          // input channel. XX = 2D/Cube
uniform vec4      iDate;                 // (year, month, day, time in seconds)
out vec4 color;

float A = 0.15;
float B = 0.50;
float C = 0.10;
float D = 0.20;
float E = 0.02;
float F = 0.30;
vec3 W = vec3(11.2);

vec3 lp = vec3(0.0,0.0,1.0);
vec3 lc = vec3(1.0,0.9,0.8)*20.0;
vec2 mouse;

struct mat{
vec3 col;
vec3 rcol;
float frc;
float frm; 
};
    
struct ray{
vec3 o;
vec3 d;
float m;
};

mat getMat(float n){
  	mat m;   
    
    m.col = vec3(0.8);
    m.rcol= vec3(1.0);
    m.frm = 0.5;
    m.frc = 0.05;
   
    
    if(n < 1.0){
        //n = floor(texture(iChannel0,vec2(n,0.0)).r*10.0)/10.0-0.1;
        
        if(n == 0.1){

        m.col = vec3(1.0,0.0,0.0);

        }else if(n == 0.2){

        m.col = vec3(1.0,1.0,0.0);

        }else if(n == 0.0){

        m.col = vec3(0.1);

        }else if(n == 0.3){

        m.col = vec3(0.0,1.0,0.0);

        }else if(n == 0.4){

        m.col = vec3(0.0,1.0,1.0);

        }else if(n == 0.5){

        m.col = vec3(0.0,0.0,1.0);

        }else if(n == 0.6){

        m.col = vec3(1.0,0.0,1.0);

        }
    }else if(n < -0.1){  
    m.frm = 0.9;
    m.frc = 0.05;
    }else{
        m.frm = 1.0;
         m.frc = 0.45*m.frm;
         m.col = vec3(0.0);

        if(n == 1.0){
            
        m.rcol= vec3(1.0,0.766,0.336);
       
        }else if(n == 1.2){
       
        m.rcol= vec3(0.955,0.637,0.538);
      
        }else if(n >= 1.1){

        m.rcol= vec3(0.972, 0.960, 0.915);  

        }
        
    }
 
    return m;
}


vec3 uncharted2ToneMapping(vec3 x){
	
		
	return ((x*(A*x+C*B)+D*E)/(x*(A*x+B)+D*F))-E/F;
}

vec2 plane( vec3 p)
{
    
    float x = float(int(mod(p.x*5.0,2.0)+1.00));
    float z = float(int(mod(p.z*5.0,2.0)+1.00));
    float check = abs(x-z)*-0.1;
	return vec2(p.y,check);
}

float length8( vec3 p )
{
	p = p*p; p = p*p; p = p*p;
	return pow( p.x + p.y+p.z, 1.0/8.0 );
}


float plane2( vec3 p ,vec3 d)
{
    
	return dot(d,p);
}


float sphere(vec3 p,float r){
    
       return length(p)-r; 
}

float sdTorus( vec3 p, vec2 t )
{
  return length( vec2(length(p.xz)-t.x,p.y) )-t.y;
}

float sdEllipsoid( in vec3 p, in vec3 r )
{
    return (length( p/r ) - 1.0) * min(min(r.x,r.y),r.z);
}

vec3 opTwist( vec3 p )
{
    float  c = cos(10.0*p.y+10.0);
    float  s = sin(10.0*p.y+10.0);
    mat2   m = mat2(c,-s,s,c);
    return vec3(m*p.xz,p.y);
}

float smin( float a, float b, float k )
{
    float res = exp( -k*a ) + exp( -k*b );
    return -log( res )/k;
}


vec2 comp(vec2 a,vec2 b){
    
    return (a.x < b.x) ? a : b;
}

/*vec2 comp(vec2 a,vec2 b){
    
    return vec2(smin(a.x , b.x,32.0),(a.x < b.x) ? a.y : b.y);
}*/

vec2 map(vec3 rp){
    vec2 dis = vec2(50.0);

    
    dis = comp(dis,vec2(plane(rp-vec3(0,-1,0)).xy ));
    dis = comp(dis,vec2(plane2(rp-vec3(0,0.0,-2.0),vec3(0,0,1)),-0.1));
    
    dis = comp(dis,vec2(sphere(rp - vec3(lp),0.1),2.0));
    
    dis = comp(dis,vec2(sdEllipsoid(rp - vec3(0.5,0.,1.0),vec3(0.2,0.2,0.2)),-0.7));
    
    dis = comp(dis,vec2(sphere(rp - vec3(-1.52,-0.5,-1.0),0.5),0.1));
    dis = comp(dis,vec2(sphere(rp - vec3(-0.51,-0.5,-1.0),0.5),0.2));
    dis = comp(dis,vec2(sphere(rp - vec3(0.51,-0.5,-1.0),0.5),0.3));
    dis = comp(dis,vec2(sphere(rp - vec3(1.52,-0.5,-1.0),0.5),0.5));
    
    dis = comp(dis,vec2(sphere(rp - vec3(-1.0,0.5,-1.0),0.5),1.0));
    dis = comp(dis,vec2(sphere(rp - vec3(0.0,0.5,-1.0),0.5),1.1));
    dis = comp(dis,vec2(sphere(rp - vec3(1.0,0.5,-1.0),0.5),1.2));
    
    dis = comp(dis,vec2(sdTorus(rp - vec3(0.0,-0.95,0.0),vec2(0.45,0.05)),-0.1));
    dis = comp(dis, vec2( 0.5*sdTorus( opTwist(rp-vec3(0.0,-0.5, 0.0)),vec2(0.250,0.1)), -0.7 ));
  
    return dis;
}




vec4 castRay(vec3 ro , vec3 rd,float imax){
    
    float i = 0.002;
    vec2 dat;
    
    for(int n = 0; n < 256; n++){
        
        dat.xy = map(ro+rd*i);
        
        if(dat.x < 0.002)return vec4(ro+rd*i,dat.y);
        
        if(i > imax)return vec4(ro+rd*i,-1.1);
        
        i+=max(abs(dat.x),0.002);
    }
    
    return vec4(ro+rd*i,-1.1);
}


vec3 calcNormal(vec3 pos){
    vec2 off = vec2(0.002,0.0);
    
   	vec3 normal = normalize(vec3(
    map(pos+off.xyy).x - map(pos-off.xyy).x,
    map(pos+off.yxy).x - map(pos-off.yxy).x,
    map(pos+off.yyx).x - map(pos-off.yyx).x )); 
    
    return normal;
}

ray castRayR(vec3 ro , vec3 rd,float imax){
    
    float i = 0.002;
    vec2 dat;
    bool pass = false;
    float pre = 1.0;
    
    for(int n = 0; n < 512; n++){
        
        dat.xy = map(ro+rd*i);
        
        
        if(dat.x < 0.002 && dat.y > -0.5)return ray(vec3(ro+rd*i),vec3(rd),dat.y);
        
        if(i > imax)return ray(vec3(ro+rd*i),vec3(rd),-1.1);
        
        if(pre < 0.0 && dat.x > 0.0){
            pass=true;
            ro=ro+rd*i;
            rd=normalize(rd+calcNormal(ro)*1.0/1.5);
            i=0.000;
        }
        
        if(pre > 0.0 && dat.x < 0.0){
            pass=false;
            ro=ro+rd*i;
            rd=normalize(rd-calcNormal(ro)*1.5);
            i=0.000;
        }
        
        i+=max(abs(dat.x),0.002);
        pre=dat.x;
    }
    
    return ray(vec3(ro+rd*i),vec3(rd),-1.1);
}

float calcShadow(vec3 sp,float imax){
   vec3 dp = normalize(lp-sp);
    float i2 = 0.1;
    for(int n =0; n < 200;n++){
        
        i2 += max(abs(map(sp+dp*i2).x/1.0) , 0.01);
        
        if(abs(map(sp+dp*i2).x) < 0.01){
            
            if(map(sp+dp*i2).y < -1.5){
                
                vec3 norm = calcNormal(sp+dp*i2);
                float fren =pow(1.0 - max(dot(-dp,norm),0.0),5.0);
     	        fren = mix(0.1,1.0,fren);
            return 1.0-fren;
                
                
            }else if(map(sp+dp*i2).y < 2.0){
        
            return 0.0;
            }else{
                
            return 1.0;
                
            }
            
        }else if( i2 > imax) return 1.0;
            
    }
    
    return 1.0;
}



//from iq's raytracer 
float calcAO( in vec3 pos, in vec3 nor )
{
	float occ = 0.0;
    float sca = 1.0;
    for( int i=0; i<5; i++ )
    {
        float hr = 0.01 + 0.12*float(i)/4.0;
        vec3 aopos =  nor * hr + pos;
        float dd = map( aopos ).x;
        occ += -(dd-hr)*sca;
        sca *= 0.95;
    }
    return clamp( 1.0 - 3.0*occ, 0.1, 1.0 );    
}


vec3 renderR(vec3 ro,vec3 rd){
    vec3 pos = vec3(0,0,0),normal = vec3(0.0,0.0,0.0),color = vec3(0.0,0.0,0.0),ref = vec3(1.0);
    mat mate; 
   	float prec = 0.002;
    vec3 bg = vec3(0.3,0.7,1.0)*1.0;
    float dist = 0.0;
    float cons = 1.0;
    float lin = 0.14;
    float quad = 0.07;
    
	for(int j = 1; j<4; j++){
        vec4 dat = vec4(0.0);
        if(j < 2){
     	ray temp = castRayR(ro,rd,15.0/float(j));
        rd = temp.d;
        dat = vec4(temp.o,temp.m);
        }else{
        dat = castRay(ro,rd,15.0/float(j));
 
        }
     	vec3 pos = dat.xyz;
 	
     	
            
            dist+=length(ro-pos);
            float fog = exp(-dist/10.0);
            float dist1 = dist+length(lp-pos);
           	float atten = 1.0/(cons + lin*dist1 + quad * (dist1 * dist1)); 
            
            if(dat.w >= 2.0){
     	       color+=lc*ref;    
     	       return color;
               
     	    }else if(dat.w <= -1.0){
            
            dist+=length(ro-pos);
  			float fog = exp(-dist/10.0);
   			color+=ref*bg;
   			return color;
                
            }else{
     	         
     	        mate = getMat(dat.w);
                vec3 c = pow(mate.col,vec3(2.2));   
     	        vec3 rc = pow(mate.rcol,vec3(2.2));
                
     	        normal = calcNormal(pos);
     	        vec3 ld = normalize(lp-pos);
                
     	        float fren =pow(1.0 - max(dot(-rd,normal),0.0),5.0);
     	        fren = mix(mate.frc,mate.frm,fren);
                
     	        vec3 lit = (max(dot(ld,normal),0.0))*lc/12.0;
                lit*=calcShadow(pos,15.0/float(j));
                
     	        vec3 dif = c*lit*(1.0-fren)*ref;
                vec3 amb = c*bg/24.0*ref*(1.0-fren)*calcAO(pos,normal);
                
                dif*=atten;
                
     	    	color+= dif + amb;
                
     	        ref*=fren*rc; 
                
     	    }
            
     	    ro=pos;
     		rd=reflect(rd,normal);
     	  	  	
    }
   dist+=length(ro-pos);
   float fog = exp(-dist/10.0);
   color+=ref*bg;
   return color;
    
}

vec3 render(vec3 ro,vec3 rd){
    vec3 pos = vec3(0,0,0),normal = vec3(0.0,0.0,0.0),color = vec3(0.0,0.0,0.0),ref = vec3(1.0);
    mat mate; 
   	float prec = 0.002;
    vec3 bg = vec3(0.3,0.7,1.0)*1.0;
    float dist = 0.0;
    float cons = 1.0;
    float lin = 0.14;
    float quad = 0.07;
    
	for(int j = 1; j<7; j++){
   
     	vec4 dat = castRay(ro,rd,15.0/float(j));
     	vec3 pos = dat.xyz;
 	
     	
            
            dist+=length(ro-pos);
            float fog = exp(-dist/10.0);
            float dist1 = dist+length(lp-pos);
           	float atten = 1.0/(cons + lin*dist1 + quad * (dist1 * dist1)); 
            
            if(dat.w >= 2.0){
     	       color+=lc*ref;    
     	       return color;
               
     	    }else if(dat.w < -1.0){
            
            dist+=length(ro-pos);
  			float fog = exp(-dist/10.0);
   			color+=ref*bg;
   			return color;
                
            }else if(dat.w < -0.5 && j < 3){
                
             
     	        mate = getMat(dat.w);
                vec3 c = pow(mate.col,vec3(2.2));   
     	        vec3 rc = pow(mate.rcol,vec3(2.2));
                
     	        normal = calcNormal(pos);
     	        vec3 ld = normalize(lp-pos);
                
     	        float fren =pow(1.0 - max(dot(-rd,normal),0.0),5.0);
     	        fren = mix(mate.frc,mate.frm,fren);
               
                
     	        vec3 dif = renderR(ro,rd)*(1.0-fren)*ref;
                vec3 amb = c*bg/24.0*ref*(1.0-fren)*calcAO(pos,normal);
                
                dif*=atten;
                
     	    	color+= dif;
                
     	        ref*=fren*rc; 
                
            }else{
     	         
     	        mate = getMat(dat.w);
                vec3 c = pow(mate.col,vec3(2.2));   
     	        vec3 rc = pow(mate.rcol,vec3(2.2));
                
     	        normal = calcNormal(pos);
     	        vec3 ld = normalize(lp-pos);
                
     	        float fren =pow(1.0 - max(dot(-rd,normal),0.0),5.0);
     	        fren = mix(mate.frc,mate.frm,fren);
                
     	        vec3 lit = (max(dot(ld,normal),0.0))*lc/12.0;
                lit*=calcShadow(pos,15.0/float(j));
                
     	        vec3 dif = c*lit*(1.0-fren)*ref;
                vec3 amb = c*bg/24.0*ref*(1.0-fren)*calcAO(pos,normal);
                
                dif*=atten;
                
     	    	color+= dif + amb;
                
     	        ref*=fren*rc; 
                
     	    }
            
     	    ro=pos;
     		rd=reflect(rd,normal);
     	  	  	
    }
   dist+=length(ro-pos);
   float fog = exp(-dist/10.0);
   color+=ref*bg;
   return color;
    
}


void mainImage( out vec4 fragColor, in vec2 fragCoord )
{
	vec2 uv = (vec2(-1.0)+2.0*(fragCoord.xy / iResolution.xy)) * vec2(iResolution.x/iResolution.y,1.0);
    
    
    mouse = (iMouse.xy/iResolution.xy*2.0-vec2(1.0))*vec2(iResolution.x/iResolution.y,1.0)*2.0;
    mouse.y = max(mouse.y,-0.5);
    
    mat3 rot = mat3(vec3(sin(mouse.x+3.14159/2.0),0,sin(mouse.x)),
                    vec3(0,1,0),
                    vec3(sin(mouse.x+3.14159),0,sin(mouse.x+3.14159/2.0)));
    
    //float p0 = texture(iChannel0,vec2(0.0,0.0)).g;
    
    lp=vec3(0.0,mouse.y,0.5);
    
    vec3 ro = vec3(0.0,0.0,1.9)+vec3(0.0,0.0,0.0);
    vec3 rd = normalize(vec3(uv,-1.0));
   	rd = rot*rd;
	ro = rot*ro;
    
    
    vec3 color = render(ro,rd);

    vec3 curr = uncharted2ToneMapping(color.xyz*5.0);
	vec3 whiteScale = 1.0/uncharted2ToneMapping(W); 
	vec3 mapped = curr*whiteScale;
	
	mapped = pow(mapped, vec3(1.0 / 2.2));
	
	fragColor =  vec4(mapped,1.0) ;
}

void main()
{
    mainImage(color, gl_FragCoord.xy);
//    color = vec4(0.99f, 0.99f, 0.99f, 1.0f);
}
