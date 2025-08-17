## OpenGL Scene Project

This project required us to pick a scene to recreate in OpenGL. This involved adapting the project to include more controls, designing the primitives, adding textures, material effects, lighting, etc.


## Project Features
### Keybinds
* W,A,S,D movement (Move camera forward, backward, left, right)
* Additional Q,E movement (Move camera straight up or down)
* Orthographic View angles: O, I, U (Front, Side, Top)
* Perspective Projection View: P
* Middle Mouse Button Scroll changes movement speed (up/down)
* Right Mouse Button Toggles Zoom. (RMB)
* Left Mouse Button Toggles FlashLight. (LMB)
* Console Output for controls/menu
* Utilized the following: OpenGL, GLEW, GLFW, and glm.
* Separated Logic and utilized OOP principles. 
* Added extra unrequired documentation, utilized doc automation tools to help gather information, then modified and created a wiki page with markdown. [Wiki](https://github.com/MatthewTheHall/OpenGLProjectscene/wiki)


## Pictures During Progress

   
Shaders only, no textures:

<img width="400" height="325" alt="image" src="https://github.com/user-attachments/assets/16b12b4b-6c24-402b-aa68-de01171cc6f5" />


Textures added:


<img width="400" height="325" alt="image" src="https://github.com/user-attachments/assets/3f3a9e2e-61dc-4cc9-a26e-f471f48107a2" />


Light and material effects added for final scene:

<img width="400" height="325" alt="image" src="https://github.com/user-attachments/assets/1bbb0cfb-2670-4dd7-85b9-980f000b9b39" />




## Reflection

This OpenGL project helped me understand graphics programming, memory management, and the creation of 3D scenes. We had to learn how to load, bind, and destroy textures. Along with how to create and manipulate primitives through raw geometry data into realistic objects, then applying shader colors, textures, lighting, and material effects for our scene. I learned about how to utilize linear algebra in graphics programming, shader programming with vertex and fragment shaders, and how material and lighting works with shininess, ambient, specular, and diffuse (Phong model).

When I first started out the project and was only adding shader colors and primitives, I was adding each object directly into RenderScene just trying to get the basic scene put together and have everything working, which became too big. I separated this into a modular design process to cleanly move the functionality into methods which can be reused. This allowed me to easily identify which object I was working on and will benefit me or anyone else that works on it in the future. Along with maintaining thorough comment sections describing each part. I also made sure to encorporate any feedback I was given. The modularity design approach is something that can easily be utilized to reuse components across different projects.

During this project, it was made clear how each iteration will introduce variables that can entirely change how I did certain things previously, e.g., shader colors and primitives only to start, followed by adding and deciding on textures for the scene, then material and lighting effects. When I added textures, I was able to remove several primitives in the scene that previously were only used to provide realism within the limitations set on us for the week. Textures allowed me to simulate trim, moulding, wallboards, and wallpaper all for a single wall plane. They also allowed me to simulate the top of the soda can with the pull-tab. I constantly refined my project through the week for each week's section, such as the texture week where I tested out dozens of textures for my scene or created my own with paint & photoshop. Lighting and material effects also took a lot of testing to find acceptable realistic effects.

As I went through the project, I learned more about optimizing performance. For instance, I know I could slightly improve it by reorganizing my drawmesh calls inside the different RenderObject's to reduce redundant Set's (SetShaderTexture & SetShaderMaterial). I've also learned about other approaches I can take, for instance bindless textures or texture arrays will allow me to move beyond the texture limit and utilize mapping as well (emission/specular/diffuse). I've also thought about extending this project and building a basic game using it as a starting point. 

Overall, this project has provided me graphics and visualization knowledge that I can extend into several fields. I'm looking forward to trying out a directx and vulkan project. And I also want to learn more about WebGL and WebGPU. I've also had a lot of interest in game programming, or creating a game engine, and this knowledge helps form a foundation toward that.


## Requirements:
* GLEW
* GLFW
* glm

The include folder has everything required -- the project has not yet been updated to use that specific include folder, it's using folders from 2 directories out, the path needs to be changed for includes & linked libraries. 
