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
};

struct Intersect {
    float len;
    vec3 normal;
    Material material;
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

uniform int       num_spheres;           // Sphere number
uniform Sphere    spheres[280];          // Sphere Array
uniform vec3      iResolution;           // Viewport resolution (in pixels)
uniform vec3      viewPos;               // View Position
uniform vec3      light_direction;       // Light direction for static/moving light
uniform int       iterations;             // Bouncing limit

uniform mat4      projection;            // Projection Matrix
uniform mat4      view;                  // View Matrix

out vec4 color;

const float epsilon = 1e-3;
const float exposure = 1e-2;
const float gamma = 2.2;
const float intensity = 100.0;
const vec3 ambient = vec3(0.6, 0.8, 1.0) * intensity / gamma;
const float MAX_LEN = 2147483647.0;
const Intersect miss = Intersect(MAX_LEN, vec3(0.0), Material(vec3(0.0), 0.0, 0.0));
//Light light = Light(vec3(1.0, 1.0, 1.0) * intensity, normalize(vec3(0.0, 1.0, -3.0)));
Light light = Light(vec3(1.0, 1.0, 1.0) * intensity, normalize(light_direction));

Intersect intersect(Ray ray, Sphere sphere) {
    vec3 oc = sphere.position - ray.origin;
    float l = dot(ray.direction, oc);
    float det = pow(l, 2.0) - dot(oc, oc) + pow(sphere.radius, 2.0);
    if (det < 0.0) return miss;
    
    float len = l - sqrt(det);
    if (len < 0.0) len = l + sqrt(det);
    if (len < 0.0) return miss;
    return Intersect(len, (ray.origin + len*ray.direction - sphere.position) / sphere.radius, sphere.material);
}

Intersect intersect(Ray ray, Plane plane) {
    float len = -dot(ray.origin, plane.normal) / dot(ray.direction, plane.normal);
    if (len < 0.0) return miss;
    return Intersect(len, plane.normal, plane.material);
}

Intersect trace(Ray ray) {
    Intersect intersection = miss;
    Intersect plane = intersect(ray, Plane(vec3(0, 1, 0), Material(vec3(1.0, 1.0, 1.0), 0.5, 0.5)));
    if (plane.material.diffuse > 0.0 || plane.material.specular > 0.0) { intersection = plane; }
    for (int i = 0; i < num_spheres; i++) {
        if(dot(ray.direction, spheres[i].position - ray.origin) >= 0) { // Prune those spheres at the back of the ray origin
            Intersect sphere = intersect(ray, spheres[i]);
            if ((sphere.material.diffuse > 0.0 || sphere.material.specular > 0.0)  && sphere.len < intersection.len) // If hit and in front of the last test hit
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
        if (hit.material.diffuse > 0.0 || hit.material.specular > 0.0) { // If hit
            
            vec3 r0 = hit.material.color.rgb * hit.material.specular;
            float hv = clamp(dot(hit.normal, -ray.direction), 0.0, 1.0); // cos(theta)
            fresnel = r0 + (1.0 - r0) * pow(1.0 - hv, 5.0); // Schlick approximation: fresnel = R0 + (1 - R0) * (1 - cos(theta))^5
            mask *= fresnel;
            
            if (trace(Ray(ray.origin + hit.len * ray.direction + epsilon * light.direction, light.direction)) == miss) {
                color += clamp(dot(hit.normal, light.direction), 0.0, 1.0) * light.color
                * hit.material.color.rgb * hit.material.diffuse
                * (1.0 - fresnel) * mask / fresnel;
            }
            
            vec3 reflection = reflect(ray.direction, hit.normal);
            ray = Ray(ray.origin + hit.len * ray.direction + epsilon * reflection, reflection);
            
        } else {
            
            vec3 spotlight = vec3(1e6) * pow(abs(dot(ray.direction, light.direction)), 250.0);
            color += mask * (ambient + spotlight);
            break;
            
        }
    }
    return color;
}

void mainImage(out vec4 fragColor, in vec2 fragCoord) {
    vec2 uv = fragCoord.xy / iResolution.xy - vec2(0.5);
    uv.x *= iResolution.x / iResolution.y;
//    Ray ray = Ray(viewPos, normalize(mat3(projection * view) * vec3(uv.x, uv.y, 1.0f)));
    Ray ray = Ray(viewPos, normalize(vec3(uv.x, uv.y, -1.0f)));

    fragColor = vec4(pow(radiance(ray) * exposure, vec3(1.0 / gamma)), 1.0);
}



void main()
{
    mainImage(color, gl_FragCoord.xy);
}
