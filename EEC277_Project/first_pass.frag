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
uniform Sphere    spheres[338];          // Sphere Array
uniform vec3      resolution;            // Viewport resolution (in pixels)
uniform vec3      viewPos;               // View Position
uniform vec3      light_direction;       // Light direction for static/moving light
uniform mat3      rot;
uniform int       iterations;            // Bouncing limit
uniform vec2      cursor;                // mouse pixel coords
uniform bool      withPlane;             // Has a plane or not
uniform bool      canRefract;            // Enable refraction

layout(location = 0) out vec4 color;
layout(location = 1) out vec4 totalRay;

const float epsilon = 1e-3;
const float exposure = 1e-2;
const float gamma = 2.2;
const float intensity = 100.0;
const vec3 ambient = vec3(0.6, 0.8, 1.0) * intensity / gamma;
const float MAX_LEN = 2147483647.0;
const Intersect miss = Intersect(MAX_LEN, vec3(0.0), vec3(0.0), Material(vec3(0.0), vec3(0.0)));
Light light = Light(vec3(1.0, 1.0, 1.0) * intensity, normalize(light_direction)); // Light source, can be fixed or moving
float rayCount = 0; // Ray calculation count for this pixel

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
    if (withPlane) {
        Intersect plane = intersect(ray, Plane(vec3(0, 1, 0), Material(vec3(1.0, 1.0, 1.0), vec3(0.5, 0.5, 0.0))));
        if (length(plane.material.diff_spec_ref)> 0.0) { intersection = plane; }
    }
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
    vec3 color = vec3(0.0);
    vec3 fresnel = vec3(0.0); 
    vec3 fresnel2 = vec3(0.0); 
    vec3 fresnel3 = vec3(0.0); 
    vec3 mask = vec3(1.0);
    vec3 mask2 = vec3(1.0);
    
    for (int i = 0; i <= iterations; ++i) {
        Intersect hit = trace(ray);
        if (length(hit.material.diff_spec_ref)> 0.0) { // If hit
            //----------------------------------------------fresnel for the first hit
            vec3 r0 = hit.material.color * hit.material.diff_spec_ref[1]; // Specular = R0; r0 is for each color
            float hv = clamp(dot(hit.normal, -ray.direction), 0.0, 1.0); // cos(theta)
            fresnel = r0 + (1.0 - r0) * pow(1.0 - hv, 5.0); // Schlick approximation: fresnel = R0 + (1 - R0) * (1 - cos(theta))^5
            mask *= fresnel; // Accumulated color mask
            
            if(canRefract && hit.material.diff_spec_ref[2] > 0.0){ // If refractive

                vec3 enter = ray.origin + hit.len * ray.direction; // enter : where ray hit the sphere
                vec3 refraction_in = refract(ray.direction, hit.normal, hit.material.diff_spec_ref[2]);
                vec3 exit = enter + (dot((hit.center-enter),refraction_in))*refraction_in*2; // exit : where ray exit sphere after refract travel inside
                vec3 refraction_out = refract(refraction_in, (hit.center-exit)/spheres[0].position_r.w, 1/hit.material.diff_spec_ref[2]);

                //--------------------------------------------reflection ray for half transparent sphere (one ray, no iteration)
                vec3 reflection = reflect(ray.direction, hit.normal);
                Ray ray_reflect = Ray(enter + epsilon * reflection, reflection);
                rayCount += rayCount + 1.0f;
                Intersect hit_reflect = trace(ray_reflect);
                if (length(hit_reflect.material.diff_spec_ref) > 0.0) { // If hit

                    if (trace(Ray(ray_reflect.origin + hit_reflect.len * ray_reflect.direction + epsilon * light.direction, light.direction)) == miss) {
                        color += clamp(dot(hit_reflect.normal, light.direction), 0.0, 1.0) * light.color
                        * hit_reflect.material.color * hit_reflect.material.diff_spec_ref[0]
                        * (1.0 - fresnel) * mask;
                        // 1st line : vertical light intensity on surface
                        // 2nd line : diffuse factor for each color
                        // 3rd line : transmittance * old mask
                    }
		    
                    } else {
                        color += mask* ambient;
                    }
                //-------------------------------------------------------------end of reflection ray for half transparent sphere

                //----------------------------------------------fresnel(2) for exiting sphere
                float hv = clamp(dot((hit.center-exit)/spheres[0].position_r.w, -refraction_in), 0.0, 1.0); // cos(theta)
                fresnel2 = r0 + (1.0 - r0) * pow(1.0 - hv, 5.0); // Schlick approximation: fresnel = R0 + (1 - R0) * (1 - cos(theta))^5
                mask *= fresnel2; // Accumulated color mask

                //--------------------------------------------reflection for refracted ray inside the sphere (one ray, no iteration)
                vec3 reflect_inner = reflect(refraction_in, (hit.center-exit)/spheres[0].position_r.w); // inner reflection
                vec3 exit2 = reflect_inner*(dot((hit.center-enter),refraction_in))*2; //point where inner reflection exit sphere // same length as the first refraction
                vec3 refraction_out2 = refract(reflect_inner, (hit.center-exit2)/spheres[0].position_r.w, 1/hit.material.diff_spec_ref[2]);//direction
                
                //----------------------------------------------fresnel(3) for the refracted and reflected ray exiting sphere
                hv = clamp(dot((hit.center-exit2)/spheres[0].position_r.w, -reflect_inner), 0.0, 1.0); // cos(theta)
                fresnel3 = r0 + (1.0 - r0) * pow(1.0 - hv, 5.0); // Schlick approximation: fresnel = R0 + (1 - R0) * (1 - cos(theta))^5
                mask2 = mask * fresnel3; // Accumulated color mask. mask2 specificlly for this single ray
                Ray ray_reflect2 = Ray(exit2 + epsilon * refraction_out2, refraction_out2);
                rayCount += rayCount + 1.0f;
                Intersect hit_reflect2 = trace(ray_reflect2);
                
                if (length(hit_reflect2.material.diff_spec_ref) > 0.0) { // If hit

                    if (trace(Ray(ray_reflect2.origin + hit_reflect2.len * ray_reflect.direction + epsilon * light.direction, light.direction)) == miss) {
                        color += clamp(dot(hit_reflect2.normal, light.direction), 0.0, 1.0) * light.color
                        * hit_reflect2.material.color * hit_reflect2.material.diff_spec_ref[0]
                        * (1.0 - fresnel3) * mask2;
                        // 1st line : vertical light intensity on surface
                        // 2nd line : diffuse factor for each color
                        // 3rd line : transmittance * old mask
                    }
		    
                } else {
                    color += mask2 * ambient;
                }
                //-------------------------------------------------------------end of reflection for refracted ray inside the sphere 

            
                ray = Ray(exit + epsilon * refraction_out, refraction_out);  // next ray: refracted
                rayCount += rayCount + 1.0f;
                color += mask *  fresnel2; // material inner color * transmittance

                if(length(mask) < 0.0003) break;

            } else { // not refractive
                if (trace(Ray(ray.origin + hit.len * ray.direction + epsilon * light.direction, light.direction)) == miss) {
                    color += clamp(dot(hit.normal, light.direction), 0.0, 1.0) * light.color
                    * hit.material.color * hit.material.diff_spec_ref[0]
                    * (1.0 - fresnel) * mask / fresnel;
                    // 1st line : vertical light intensity on surface
                    // 2nd line : diffuse factor for each color
                    // 3rd line : transmittance * old mask
                }
            
                if(length(mask) < 0.0003) break;
            
                vec3 reflection = reflect(ray.direction, hit.normal);
                ray = Ray(ray.origin + hit.len * ray.direction + epsilon * reflection, reflection);// next ray: reflected
                rayCount += rayCount + 1.0f;
            }
            
        } else { // not hit
            
            vec3 spotlight = vec3(1e6) * pow(abs(dot(ray.direction, light.direction)), 250.0);
            color += mask * (ambient + spotlight);   //addtion light 
            // we didn't add this light in the refraction because it would cause too many obvious artifact spots, due to the lack of anti-aliasing
            break;
            
        }
    }
    return color;
}

void mainImage(out vec4 fragColor, out vec4 count, in vec2 fragCoord) {
    vec2 uv = fragCoord.xy / resolution.xy - vec2(0.5);
    uv.x *= resolution.x / resolution.y;
   
//    Ray ray = Ray(viewPos, normalize(mat3(projection * view) * vec3(uv.x, uv.y, 1.0f))); // With projection and view
    Ray ray = Ray(viewPos, rot * normalize(vec3(uv.x, uv.y, -1.0)));
    
    fragColor = vec4(pow(radiance(ray) * exposure, vec3(1.0f / gamma)), 1.0f);
    count = vec4(rayCount / 255.0f, 0.0f, 0.0f, 1.0f); // Put ray calculation count in red color of output vector
}



void main()
{
    mainImage(color, totalRay, gl_FragCoord.xy);
}
