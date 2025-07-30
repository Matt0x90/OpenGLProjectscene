## Initial Scene Inspiration
 <img width="300" height="400" alt="image" src="https://github.com/user-attachments/assets/b853e1b9-eb3c-4ab4-88a3-7a1f8e4d3cf4" />

 
**************************************************************************************************

### Progress Updates

 TODO:
 * Textures & Lighting.
 * Additions: Room extended, ceiling, ceiling trim, ceiling fan, neon sign/wall art, etc.

 First Commit: forgot to put on github before this point.
 
 <img width="400" height="325" alt="image" src="https://github.com/user-attachments/assets/ce8d4637-9d95-49f7-bda7-788bfefa6ebc" />

 Second update: finished lamp
   
 <img width="400" height="325" alt="image" src="https://github.com/user-attachments/assets/d5333e49-71f2-4cf7-a2f9-13d22e2c80fd" />

 <img width="400" height="325" alt="image" src="https://github.com/user-attachments/assets/720ba0c0-7a06-4116-80bb-7f6f605ac54d" />

Third update: Adjusted lamp shade and stool, added arcade. Adjusted room size (needs to be larger still)

<img width="400" height="325" alt="image" src="https://github.com/user-attachments/assets/16b12b4b-6c24-402b-aa68-de01171cc6f5" />

Fourth update:

* Q/E moves up and down. (in addition to WASD)
* Window_Resize_Callback to handle display resizing.
* Mouse_Scroll_Wheel_Callback to handle the scroll wheel adjusting movement speed when looking around. Up = slow, Down = fast.
* Mouse_Button_Callback to allow RMB (right mouse button) to zoom in when pressed and change back to default when pressed again. (This could be easily altered to only do so on LMB (left mouse button), only do so while holding, and to add extra functionality by using both buttons)
* ProcessMousePress inside camera.h, created for Mouse_Button_Callback
* Enabled Orthographic view and setup keybinds to switch between both views: P = perspective, O = orthographic
* Orthographic also has additional angle views: O = front, i = side, U = top
* Console Output showing all control commands.







## Requirements:
* GLEW
* GLFW
* glm

The include folder has everything required -- the project has not yet been updated to use that specific include folder, it's using folders from 2 directories out, the path needs to be changed for includes & linked libraries. 
