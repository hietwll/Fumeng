# Fumeng (浮梦)

Fumeng is a hobby project which implements a monte-carlo path tracer from scratch. Fumeng means illusory dream in English.

## Features
### Camera
- [x] Pin-hole camera
- [x] Thin-lens camera (with depth of field)
### Shape
- [x] Sphere
- [x] Rectangle
- [x] Triangle
- [x] Obj mesh
### Material
- [x] Lambert diffuse
- [x] Specular reflection
- [x] Specular transmission
- [x] Disney BSDF
- [ ] Subsurface scatering
- [ ] Volumetric media
### Light
- [x] Directional light
- [x] Area light
- [x] Image-based light (HDR)
### Accelerator
- [x] Built-in BVH
- [x] [Embree](https://www.embree.org/) BVH (opitional, but highly recommended)
### Ray Tracer
- [x] Basic monte-carlo path tracer
- [ ] Bidirectional path tracer
### Sampling
- [x] Multiple importance sampling
### Denoise
- [x] [OIDN](https://www.openimagedenoise.org/) denoise (opitional, but highly recommended)
### Post process
- [x] Tonemapping
### Texture
- [x] Image texture
- [x] Constant texture
- [ ] Checkerboard
### Scene format
- [x] custom description using json


## Scenes
Damaged Helmet:
![pic](./images/DamagedHelmet.png)

DoF Camera:
![pic](./images/DofCamera.png)

Substance Boy:
![pic](./images/SubstanceBoy.png)

Hyperion:
![pic](./images/Hyperion.png)

Storm Trooper:
![pic](./images/StormTrooper.png)

Dining Room:
![pic](./images/DiningRoom.png)

Classroom:
![pic](./images/ClassRoom.png)

