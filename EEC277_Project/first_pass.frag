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
    vec3 diff_spec_ref;
};

struct Intersect {
    float len;
    vec3 normal;
    vec3 center;
    Material material;
};

struct Sphere {
    vec4 position_r;
    Material material;
};

struct Plane {
    vec3 normal;
    Material material;
};

uniform int       num_spheres;           // Sphere number
uniform Sphere    spheres[325];          // Sphere Array
uniform vec3      resolution;            // Viewport resolution (in pixels)
uniform vec3      viewPos;               // View Position
uniform vec3      light_direction;       // Light direction for static/moving light
uniform int       iterations;            // Bouncing limit
uniform vec4      cursor;                // mouse pixel coords
uniform mat4      projection;            // Projection Matrix
uniform mat4      view;                  // View Matrix


layout(location = 0) out vec4 color;
layout(location = 1) out vec4 totalRay;

const float epsilon = 1e-3;
const float exposure = 1e-2;
const float gamma = 2.2;
const float intensity = 100.0;
const vec3 ambient = vec3(0.6, 0.8, 1.0) * intensity / gamma;
const float MAX_LEN = 2147483647.0;
const Intersect miss = Intersect(MAX_LEN, vec3(0.0), vec3(0.0), Material(vec3(0.0), vec3(0.0)));
Light light = Light(vec3(1.0, 1.0, 1.0) * intensity, normalize(vec3(0.0, 1.0, -3.0))); // Fixed light
//Light light = Light(vec3(1.0, 1.0, 1.0) * intensity, normalize(light_direction)); // Moving light
float rayCount = 0;


Intersect intersect(Ray ray, Sphere sphere) {
    vec3 oc = sphere.position_r.xyz - ray.origin;
    float l = dot(ray.direction, oc);
    float det = pow(l, 2.0) - dot(oc, oc) + pow(sphere.position_r.w, 2.0);
    if (det < 0.0) return miss;
    
    float len = l - sqrt(det);
    if (len < 0.0) len = l + sqrt(det);
    if (len < 0.0) return miss;
    return Intersect(len, (ray.origin + len*ray.direction - sphere.position_r.xyz) / sphere.position_r.w, sphere.position_r.xyz, sphere.material);
}

Intersect intersect(Ray ray, Plane plane) {
    float len = -dot(ray.origin, plane.normal) / dot(ray.direction, plane.normal);
    if (len < 0.0) return miss;
    return Intersect(len, plane.normal, vec3(0.0), plane.material);
}

Intersect trace(Ray ray) {
    Intersect intersection = miss;
    Intersect plane = intersect(ray, Plane(vec3(0, 1, 0), Material(vec3(1.0, 1.0, 1.0), vec3(0.5, 0.5, 0.0))));
    if (plane.material.diff_spec_ref[0] > 0.0 || plane.material.diff_spec_ref[1] > 0.0) { intersection = plane; }
    for (int i = 0; i < num_spheres; i++) {
        if(dot(ray.direction, spheres[i].position_r.xyz - ray.origin) >= 0) { // Prune those spheres at the back of the ray origin
            Intersect sphere = intersect(ray, spheres[i]);
            if ((sphere.material.diff_spec_ref[0] > 0.0 || sphere.material.diff_spec_ref[1] > 0.0)  && sphere.len < intersection.len) // If hit and in front of the last test hit
                intersection = sphere;
        }
    }
    return intersection;
}

vec3 radiance(Ray ray) {
    vec3 color = vec3(0.0), fresnel = vec3(0.0); // Fresnel factor:
    vec3 mask = vec3(1.0);
    
    for (int i = 0; i <= iterations; ++i) {
        Intersect hit = trace(ray);
        if (hit.material.diff_spec_ref[0] > 0.0 || hit.material.diff_spec_ref[1] > 0.0 || hit.material.diff_spec_ref[2]>0.0) { // If hit
            
            vec3 r0 = hit.material.color * hit.material.diff_spec_ref[1]; // Specular = R0; r0 is for each color
            float hv = clamp(dot(hit.normal, -ray.direction), 0.0, 1.0); // cos(theta)
            fresnel = r0 + (1.0 - r0) * pow(1.0 - hv, 5.0); // Schlick approximation: fresnel = R0 + (1 - R0) * (1 - cos(theta))^5
            mask *= fresnel; // Accumulated color mask
            
           if(hit.material.diff_spec_ref[2]>0.0){
		vec3 reflection = reflect(ray.direction, hit.normal);//////reflection ray for half transparent sphere (one ray, no iteration)
		Ray ray_reflect = Ray(ray.origin + hit.len * ray.direction + epsilon * reflection, reflection);
		rayCount += rayCount + 1.0f;
	        Intersect hit_reflect  = trace(ray_reflect);
		if (hit_reflect .material.diff_spec_ref[0] > 0.0 || hit_reflect .material.diff_spec_ref[1] > 0.0 || hit_reflect .material.diff_spec_ref[2]>0.0) { // If hit
		    
		    vec3 r0 = hit_reflect .material.color * hit_reflect .material.diff_spec_ref[1]; // Specular = R0; r0 is for each color
		    float hv = clamp(dot(hit_reflect .normal, -ray_reflect.direction), 0.0, 1.0); // cos(theta)
		    fresnel = r0 + (1.0 - r0) * pow(1.0 - hv, 5.0); // Schlick approximation: fresnel = R0 + (1 - R0) * (1 - cos(theta))^5
		    mask *= fresnel; // Accumulated color mask
		    

			if (trace(Ray(ray_reflect.origin + hit_reflect .len * ray_reflect.direction + epsilon * light.direction, light.direction)) == miss) {
		        color += clamp(dot(hit_reflect .normal, light.direction), 0.0, 1.0) * light.color
		        * hit_reflect .material.color * hit_reflect .material.diff_spec_ref[0]
		        * (1.0 - fresnel) * mask / fresnel;
		    }
		    
		} else {
		    
		    vec3 spotlight = vec3(1e6) * pow(abs(dot(ray.direction, light.direction)), 250.0);
		    color += mask * (ambient + spotlight);		    
		}////////////////////////////////////////////////////////////end of reflection ray for half transparent sphere

	    vec3 enter = ray.origin + hit.len * ray.direction;
            vec3 refraction_in = refract(ray.direction, hit.normal, hit.material.diff_spec_ref[2]);
            vec3 exit = enter + (dot((hit.center-enter),refraction_in))*refraction_in*2; 
            vec3 refraction_out = refract(refraction_in, (hit.center-exit)/0.5, 1/hit.material.diff_spec_ref[2]);
            ray = Ray(exit + epsilon * refraction_out, refraction_out);
            rayCount += rayCount + 1.0f;
	    color += mask * hit.material.color;
                // enter : where ray hit the sphere
                // exit : where ray exit sphere after refract travel inside

            }
            else{ 
		if (trace(Ray(ray.origin + hit.len * ray.direction + epsilon * light.direction, light.direction)) == miss) {
                color += clamp(dot(hit.normal, light.direction), 0.0, 1.0) * light.color
                * hit.material.color * hit.material.diff_spec_ref[0]
                * (1.0 - fresnel) * mask / fresnel;
                // 1st line : vertical light intensity on surface
                // 2nd line : diffuse factor for each color
                // 3rd line : old mask * transmittance
            }
            
            if(length(mask) < 0.0003) break;
            
            vec3 reflection = reflect(ray.direction, hit.normal);
            ray = Ray(ray.origin + hit.len * ray.direction + epsilon * reflection, reflection);
            rayCount += rayCount + 1.0f;
            }

            
        } else {
            
            vec3 spotlight = vec3(1e6) * pow(abs(dot(ray.direction, light.direction)), 250.0);
            color += mask * (ambient + spotlight);
            break;
            
        }
    }
    return color;
}

void mainImage(out vec4 fragColor, out vec4 count, in vec2 fragCoord) {
    vec2 uv = fragCoord.xy / resolution.xy - vec2(0.5);
    uv.x *= resolution.x / resolution.y;
    vec2 mouse = (cursor.xy/resolution.xy*1.3089-vec2(0.65))*vec2(resolution.x/resolution.y,1.0)*2.0;
    mouse.y = max(mouse.y,-0.5);
    mat3 rot = mat3(vec3(sin(mouse.x+3.14159/2.0),0,sin(mouse.x)),vec3(0,1,0),vec3(sin(mouse.x+3.14159),0,sin(mouse.x+3.14159/2.0)));
   

    Ray ray = Ray(viewPos, rot*normalize(vec3(uv.x, uv.y, -1.0)));
    
    fragColor = vec4(pow(radiance(ray) * exposure, vec3(1.0f / gamma)), 1.0f);
    count = vec4(vec3(0.0, 0.0, rayCount / 255.0f), 1.0f);
}



void main()
{
    mainImage(color, totalRay, gl_FragCoord.xy);
}
