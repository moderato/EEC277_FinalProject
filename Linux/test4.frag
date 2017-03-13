#version 410 core


struct Ray {
    vec3 origin;
    vec3 direction;
};

struct Light {
    vec3 color;
    vec3 direction;
};

struct Material {
    vec3 color;
    float diffuse;
    float specular;
    float refraction;
};

struct Intersect {
    float len;
    vec3 normal;
    Material material;
    vec3 center;
};

struct Sphere {
    float radius;
    vec3 position;
    Material material;
};

struct Plane {
    vec3 normal;
    Material material;
};

uniform int       num_spheres;
uniform Sphere spheres[100];
uniform vec3      iResolution;           // viewport resolution (in pixels)
uniform float     iGlobalTime;           // shader playback time (in seconds)
uniform vec4      iMouse;                // mouse pixel coords. xy: current (if MLB down), zw: click
uniform vec3      viewPos;
uniform vec3      front;
uniform vec4      view;
uniform vec4      projection;
out vec4 color;
vec2 mouse;


const float epsilon = 1e-3;
const int iterations = 4;
const float exposure = 1e-2;
const float gamma = 2.2;
const float intensity = 100.0;
const vec3 ambient = vec3(0.6, 0.8, 1.0) * intensity / gamma;
const Intersect miss = Intersect(2147483647, vec3(0.0), Material(vec3(0.0), 0.0, 0.0 ,0.0), vec3(0.0, 0.0, 0.0));
Light light = Light(vec3(1.0, 1.0, 1.0) * intensity, normalize(vec3(0.0, 1.0, -3.0)));

Intersect intersect(Ray ray, Sphere sphere) {
    vec3 oc = sphere.position - ray.origin;
    float l = dot(ray.direction, oc);
    float det = pow(l, 2.0) - dot(oc, oc) + pow(sphere.radius, 2.0);
    if (det < 0.0) return miss;
    
    float len = l - sqrt(det);
    if (len < 0.0) len = l + sqrt(det);
    if (len < 0.0) return miss;
    return Intersect(len, (ray.origin + len*ray.direction - sphere.position) / sphere.radius, sphere.material, sphere.position);
}

Intersect intersect(Ray ray, Plane plane) {
    float len = -dot(ray.origin, plane.normal) / dot(ray.direction, plane.normal);
    if (len < 0.0) return miss;
    return Intersect(len, plane.normal, plane.material,vec3(0.0, 0.0, 0.0));
}

Intersect trace(Ray ray) {
    Intersect intersection = miss;
    Intersect plane = intersect(ray, Plane(vec3(0, 1, 0), Material(vec3(1.0, 1.0, 1.0), 0.5, 0.5, 0.0)));
    if (plane.material.diffuse > 0.0 || plane.material.specular > 0.0) { intersection = plane; }
    for (int i = 0; i < num_spheres; i++) {
        Intersect sphere = intersect(ray, spheres[i]);
        if ((sphere.material.diffuse > 0.0 || sphere.material.specular > 0.0)  && sphere.len < intersection.len)
            intersection = sphere;
    }

    return intersection;
}

vec3 radiance(Ray ray) {
    vec3 color = vec3(0.0), fresnel = vec3(0.0);
    vec3 mask = vec3(1.0);
    for (int i = 0; i <= 9	; ++i) {
        Intersect hit = trace(ray);
        if (hit.material.diffuse > 0.0 || hit.material.specular > 0.0 || hit.material.refraction > 0.0) {
            
            vec3 r0 = hit.material.color.rgb * hit.material.specular;
            float hv = clamp(dot(hit.normal, -ray.direction), 0.0, 1.0);
            fresnel = r0 + (1.0 - r0) * pow(1.0 - hv, 5.0);
            mask *= fresnel;
            
           


            if(hit.material.refraction>0.0){
	    vec3 enter = ray.origin + hit.len * ray.direction;
            vec3 refraction = refract(ray.direction, hit.normal, hit.material.refraction);
            vec3 exit = enter + (dot((hit.center-enter),refraction))*refraction*2; 
            vec3 refraction2 = refract(refraction, (hit.center-exit)/0.5, 1/hit.material.refraction);
            ray = Ray(exit + epsilon * refraction2/0.5, refraction2);
	    color += mask * ambient ;
            }
            else{ 
		if (trace(Ray(ray.origin + hit.len * ray.direction + epsilon * light.direction, light.direction)) == miss) {
                color += clamp(dot(hit.normal, light.direction), 0.0, 1.0) * light.color
                       * hit.material.color.rgb * hit.material.diffuse
                       * (1.0 - fresnel) * mask / fresnel;
                }
            vec3 reflection = reflect(ray.direction, hit.normal);
            ray = Ray(ray.origin + hit.len * ray.direction + epsilon * reflection, reflection);
            }

            
           
            
        } else {
            
            vec3 spotlight = vec3(1e6) * pow(abs(dot(ray.direction, light.direction)), 300.0);
            color += mask * (ambient+spotlight ); 
	    break;
        }
    }
    return color;
}

void mainImage(out vec4 fragColor, in vec2 fragCoord) {
    vec2 uv    = fragCoord.xy / iResolution.xy - vec2(0.5);
    uv.x *= iResolution.x / iResolution.y;
    mouse = (iMouse.xy/iResolution.xy*1.3089-vec2(0.65))*vec2(iResolution.x/iResolution.y,1.0)*2.0;
    mouse.y = max(mouse.y,-0.5);
    mat3 rot = mat3(vec3(sin(mouse.x+3.14159/2.0),0,sin(mouse.x)),vec3(0,1,0),vec3(sin(mouse.x+3.14159),0,sin(mouse.x+3.14159/2.0)));
   

    Ray ray = Ray(viewPos, rot*normalize(vec3(uv.x, uv.y, -1.0)));
    

    fragColor = vec4(pow(radiance(ray) * exposure, vec3(1.0 / gamma)), 1.0);
}



void main()
{
    mainImage(color, gl_FragCoord.xy);
}

